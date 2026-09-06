#include "platform/native_discord.h"

#if defined(_WIN32) && !defined(__vita__)

#include "platform/native_log.h"
#include "platform/native_win32.h"

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NATIVE_DISCORD_APP_ID "1546144938495049798"
#define NATIVE_DISCORD_LARGE_IMAGE "high_octane"
#define NATIVE_DISCORD_LARGE_TEXT "Crash Team Racing: High Octane"
#define NATIVE_DISCORD_RETRY_MS 5000u
#define NATIVE_DISCORD_MAX_PAYLOAD 16384u

enum NativeDiscordOpcode
{
    NATIVE_DISCORD_OP_HANDSHAKE = 0,
    NATIVE_DISCORD_OP_FRAME = 1,
    NATIVE_DISCORD_OP_CLOSE = 2,
    NATIVE_DISCORD_OP_PING = 3,
    NATIVE_DISCORD_OP_PONG = 4,
};

struct NativeDiscordContext
{
    HANDLE pipe;
    u64 nextRetryMs;
    u64 activityStartEpoch;
    u32 nonce;
    b32 initialized;
    b32 forcePresence;
    b32 wasGameplay;
    char lastDetails[128];
    char lastState[128];
};

static struct NativeDiscordContext s_nativeDiscord;

static void NativeDiscord_Disconnect(void)
{
    if ((s_nativeDiscord.pipe != NULL) && (s_nativeDiscord.pipe != INVALID_HANDLE_VALUE))
    {
        CloseHandle(s_nativeDiscord.pipe);
    }
    s_nativeDiscord.pipe = INVALID_HANDLE_VALUE;
    s_nativeDiscord.forcePresence = true;
}

static b32 NativeDiscord_WriteAll(const void *bytes, u32 size)
{
    const u8 *cursor = (const u8 *)bytes;
    while (size != 0)
    {
        DWORD written = 0;
        if (!WriteFile(s_nativeDiscord.pipe, cursor, size, &written, NULL) || (written == 0))
        {
            NativeDiscord_Disconnect();
            return false;
        }
        cursor += written;
        size -= written;
    }
    return true;
}

static b32 NativeDiscord_SendPayload(u32 opcode, const void *payload, u32 payloadSize)
{
    if ((s_nativeDiscord.pipe == INVALID_HANDLE_VALUE) || (payloadSize > NATIVE_DISCORD_MAX_PAYLOAD))
    {
        return false;
    }

    u32 header[2];
    header[0] = opcode;
    header[1] = payloadSize;
    return NativeDiscord_WriteAll(header, sizeof(header)) &&
           ((payloadSize == 0) || NativeDiscord_WriteAll(payload, payloadSize));
}

static b32 NativeDiscord_SendJson(u32 opcode, const char *json)
{
    return NativeDiscord_SendPayload(opcode, json, (u32)strlen(json));
}

static void NativeDiscord_TryConnect(void)
{
    if (s_nativeDiscord.pipe != INVALID_HANDLE_VALUE)
    {
        return;
    }

    u64 now = SDL_GetTicks();
    if (now < s_nativeDiscord.nextRetryMs)
    {
        return;
    }
    s_nativeDiscord.nextRetryMs = now + NATIVE_DISCORD_RETRY_MS;

    for (int pipeIndex = 0; pipeIndex < 10; pipeIndex++)
    {
        char pipeName[64];
        snprintf(pipeName, sizeof(pipeName), "\\\\?\\pipe\\discord-ipc-%d", pipeIndex);
        HANDLE pipe = CreateFileA(pipeName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (pipe == INVALID_HANDLE_VALUE)
        {
            continue;
        }

        s_nativeDiscord.pipe = pipe;
        char handshake[128];
        snprintf(handshake, sizeof(handshake), "{\"v\":1,\"client_id\":\"%s\"}", NATIVE_DISCORD_APP_ID);
        if (!NativeDiscord_SendJson(NATIVE_DISCORD_OP_HANDSHAKE, handshake))
        {
            continue;
        }

        s_nativeDiscord.forcePresence = true;
        Platform_Log("[CTR Discord] Connected to Discord IPC\n");
        return;
    }
}

static void NativeDiscord_Pump(void)
{
    while (s_nativeDiscord.pipe != INVALID_HANDLE_VALUE)
    {
        DWORD available = 0;
        if (!PeekNamedPipe(s_nativeDiscord.pipe, NULL, 0, NULL, &available, NULL))
        {
            NativeDiscord_Disconnect();
            return;
        }
        if (available < 8)
        {
            return;
        }

        u32 header[2];
        DWORD headerBytes = 0;
        DWORD totalAvailable = 0;
        if (!PeekNamedPipe(s_nativeDiscord.pipe, header, sizeof(header), &headerBytes, &totalAvailable, NULL) ||
            (headerBytes != sizeof(header)))
        {
            NativeDiscord_Disconnect();
            return;
        }

        u32 payloadSize = header[1];
        if (payloadSize > NATIVE_DISCORD_MAX_PAYLOAD)
        {
            NativeDiscord_Disconnect();
            return;
        }
        if (totalAvailable < (DWORD)(sizeof(header) + payloadSize))
        {
            return;
        }

        u8 packet[sizeof(header) + NATIVE_DISCORD_MAX_PAYLOAD];
        DWORD bytesRead = 0;
        if (!ReadFile(s_nativeDiscord.pipe, packet, sizeof(header) + payloadSize, &bytesRead, NULL) ||
            (bytesRead != sizeof(header) + payloadSize))
        {
            NativeDiscord_Disconnect();
            return;
        }

        u32 opcode = ((u32 *)packet)[0];
        if (opcode == NATIVE_DISCORD_OP_PING)
        {
            if (!NativeDiscord_SendPayload(NATIVE_DISCORD_OP_PONG, packet + sizeof(header), payloadSize))
            {
                return;
            }
        }
        else if (opcode == NATIVE_DISCORD_OP_CLOSE)
        {
            NativeDiscord_Disconnect();
            return;
        }
    }
}

static void NativeDiscord_JsonEscape(const char *src, char *dst, int dstSize)
{
    int out = 0;
    if (dstSize <= 0)
    {
        return;
    }

    while ((src != NULL) && (*src != '\0') && (out < dstSize - 1))
    {
        unsigned char c = (unsigned char)*src++;
        if ((c == '"') || (c == '\\'))
        {
            if (out + 2 >= dstSize) break;
            dst[out++] = '\\';
            dst[out++] = (char)c;
        }
        else if (c >= 0x20)
        {
            dst[out++] = (char)c;
        }
    }
    dst[out] = '\0';
}

static const char *NativeDiscord_GetModeName(const struct GameTracker *gGT)
{
    if (gNativeBossFightMode != 0) return "Boss Fight";
    if (gNativeGhostReplayMode != 0) return "Ghost Replay";
    if ((gNativeRelicRaceMode != 0) || ((gGT->gameMode1 & RELIC_RACE) != 0)) return "Relic Race";
    if ((gGT->gameMode1 & TIME_TRIAL) != 0) return "Time Trial";
    if ((gGT->gameMode1 & BATTLE_MODE) != 0) return "Battle";
    if ((gGT->gameMode1 & ADVENTURE_MODE) != 0) return "Adventure";
    if ((gGT->gameMode1 & ARCADE_MODE) != 0) return "Arcade";
    if (gGT->numPlyrCurrGame > 1) return "VS";
    return "Race";
}

static const char *NativeDiscord_GetTrackName(const struct GameTracker *gGT)
{
    if ((gGT->levelID < DINGO_CANYON) || (gGT->levelID > ROCKY_ROAD) || (sdata->lngStrings == NULL))
    {
        return NULL;
    }

    int nameIndex = data.metaDataLEV[gGT->levelID].name_LNG;
    if (nameIndex < 0)
    {
        return NULL;
    }
    return sdata->lngStrings[nameIndex];
}

static const char *NativeDiscord_GetCharacterName(const struct GameTracker *gGT)
{
    if ((gGT->numPlyrCurrGame < 1) || (gGT->drivers[0] == NULL) || (sdata->lngStrings == NULL))
    {
        return NULL;
    }

    int driverId = gGT->drivers[0]->driverID;
    if ((driverId < 0) || (driverId >= 8))
    {
        return NULL;
    }
    int characterId = data.characterIDs[driverId];
    if ((characterId < 0) || (characterId >= 16))
    {
        return NULL;
    }

    int nameIndex = data.MetaDataCharacters[characterId].name_LNG_long;
    if (nameIndex < 0)
    {
        return NULL;
    }
    return sdata->lngStrings[nameIndex];
}

static void NativeDiscord_BuildPresence(char details[128], char state[128], b32 *gameplay)
{
    *gameplay = false;
    snprintf(details, 128, "%s", "Starting High Octane");
    state[0] = '\0';

    if ((sdata == NULL) || (sdata->gGT == NULL))
    {
        return;
    }

    struct GameTracker *gGT = sdata->gGT;
    if ((gGT->levelID == MAIN_MENU_LEVEL) || ((gGT->gameMode1 & MAIN_MENU) != 0))
    {
        snprintf(details, 128, "%s", "Main Menu");
        snprintf(state, 128, "%s", "Ready to race");
        return;
    }

    if ((gGT->gameMode1 & GAME_CUTSCENE) != 0)
    {
        snprintf(details, 128, "%s", "Watching a cutscene");
        return;
    }

    const char *mode = NativeDiscord_GetModeName(gGT);
    const char *track = NativeDiscord_GetTrackName(gGT);
    const char *character = NativeDiscord_GetCharacterName(gGT);
    if (track != NULL)
    {
        snprintf(details, 128, "%s - %s", mode, track);
        *gameplay = true;
    }
    else
    {
        snprintf(details, 128, "%s", mode);
    }

    if ((character != NULL) && (gGT->numPlyrCurrGame == 1))
    {
        snprintf(state, 128, "Playing as %s", character);
    }
    else if (gGT->numPlyrCurrGame > 1)
    {
        snprintf(state, 128, "%d Players", gGT->numPlyrCurrGame);
    }
}

static void NativeDiscord_SendPresence(const char *details, const char *state, b32 gameplay)
{
    char escapedDetails[256];
    char escapedState[256];
    NativeDiscord_JsonEscape(details, escapedDetails, sizeof(escapedDetails));
    NativeDiscord_JsonEscape(state, escapedState, sizeof(escapedState));

    if (gameplay && (!s_nativeDiscord.wasGameplay || (strcmp(details, s_nativeDiscord.lastDetails) != 0)))
    {
        s_nativeDiscord.activityStartEpoch = (u64)time(NULL);
    }
    else if (!gameplay)
    {
        s_nativeDiscord.activityStartEpoch = 0;
    }
    s_nativeDiscord.wasGameplay = gameplay;

    char timestampJson[96] = {0};
    if (gameplay && (s_nativeDiscord.activityStartEpoch != 0))
    {
        snprintf(timestampJson, sizeof(timestampJson), ",\"timestamps\":{\"start\":%llu}",
                 (unsigned long long)s_nativeDiscord.activityStartEpoch);
    }

    char stateJson[320] = {0};
    if (escapedState[0] != '\0')
    {
        snprintf(stateJson, sizeof(stateJson), ",\"state\":\"%s\"", escapedState);
    }

    char json[2048];
    u32 nonce = ++s_nativeDiscord.nonce;
    snprintf(json, sizeof(json),
             "{\"cmd\":\"SET_ACTIVITY\",\"args\":{\"pid\":%lu,\"activity\":{\"details\":\"%s\"%s%s,"
             "\"assets\":{\"large_image\":\"%s\",\"large_text\":\"%s\"}}},\"nonce\":\"%u\"}",
             (unsigned long)GetCurrentProcessId(), escapedDetails, stateJson, timestampJson,
             NATIVE_DISCORD_LARGE_IMAGE, NATIVE_DISCORD_LARGE_TEXT, nonce);

    if (NativeDiscord_SendJson(NATIVE_DISCORD_OP_FRAME, json))
    {
        snprintf(s_nativeDiscord.lastDetails, sizeof(s_nativeDiscord.lastDetails), "%s", details);
        snprintf(s_nativeDiscord.lastState, sizeof(s_nativeDiscord.lastState), "%s", state);
        s_nativeDiscord.forcePresence = false;
    }
}

void NativeDiscord_Init(void)
{
    memset(&s_nativeDiscord, 0, sizeof(s_nativeDiscord));
    s_nativeDiscord.pipe = INVALID_HANDLE_VALUE;
    s_nativeDiscord.initialized = true;
    s_nativeDiscord.forcePresence = true;
    NativeDiscord_TryConnect();
}

void NativeDiscord_Update(void)
{
    if (!s_nativeDiscord.initialized)
    {
        return;
    }

    NativeDiscord_TryConnect();
    if (s_nativeDiscord.pipe == INVALID_HANDLE_VALUE)
    {
        return;
    }

    NativeDiscord_Pump();
    if (s_nativeDiscord.pipe == INVALID_HANDLE_VALUE)
    {
        return;
    }

    char details[128];
    char state[128];
    b32 gameplay = false;
    NativeDiscord_BuildPresence(details, state, &gameplay);

    if (s_nativeDiscord.forcePresence ||
        (strcmp(details, s_nativeDiscord.lastDetails) != 0) ||
        (strcmp(state, s_nativeDiscord.lastState) != 0) ||
        (gameplay != s_nativeDiscord.wasGameplay))
    {
        NativeDiscord_SendPresence(details, state, gameplay);
    }
}

void NativeDiscord_Shutdown(void)
{
    if (!s_nativeDiscord.initialized)
    {
        return;
    }

    if (s_nativeDiscord.pipe != INVALID_HANDLE_VALUE)
    {
        char json[256];
        u32 nonce = ++s_nativeDiscord.nonce;
        snprintf(json, sizeof(json),
                 "{\"cmd\":\"SET_ACTIVITY\",\"args\":{\"pid\":%lu,\"activity\":null},\"nonce\":\"%u\"}",
                 (unsigned long)GetCurrentProcessId(), nonce);
        NativeDiscord_SendJson(NATIVE_DISCORD_OP_FRAME, json);
    }

    NativeDiscord_Disconnect();
    memset(&s_nativeDiscord, 0, sizeof(s_nativeDiscord));
}

#else

void NativeDiscord_Init(void) {}
void NativeDiscord_Update(void) {}
void NativeDiscord_Shutdown(void) {}

#endif