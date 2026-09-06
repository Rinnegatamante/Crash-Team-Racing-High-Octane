#include <common.h>
#include "platform/native_leaderboard.h"

#if CTR_NATIVE_HAS_LEADERBOARD
#include "platform/native_log.h"
#include "platform/native_network.h"
#include "platform/native_user_id.h"
#include "platform/native_pc_account.h"
#if defined(_WIN32) && !defined(__vita__)
#include "platform/native_http_win32.h"
#endif

#include <SDL3/SDL.h>
#if defined(__vita__)
#include <curl/curl.h>
#include <openssl/sha.h>
#include <psp2/io/fcntl.h>
#endif
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#ifndef CTR_NATIVE_LEADERBOARD_GET_URL
#define CTR_NATIVE_LEADERBOARD_GET_URL "https://www.rinnegatamante.eu/ctr/get_leaderboard.php"
#endif
#ifndef CTR_NATIVE_LEADERBOARD_UPLOAD_URL
#if defined(__vita__)
#define CTR_NATIVE_LEADERBOARD_UPLOAD_URL "https://www.rinnegatamante.eu/ctr/upload_record.php"
#else
#define CTR_NATIVE_LEADERBOARD_UPLOAD_URL "https://www.rinnegatamante.eu/ctr/api/upload_record.php"
#endif
#endif
#ifndef CTR_NATIVE_LEADERBOARD_ME_URL
#define CTR_NATIVE_LEADERBOARD_ME_URL "https://www.rinnegatamante.eu/ctr/api/me.php"
#endif
#ifndef CTR_NATIVE_LEADERBOARD_LINK_VITA_URL
#define CTR_NATIVE_LEADERBOARD_LINK_VITA_URL "https://www.rinnegatamante.eu/ctr/api/link_vita.php"
#endif

enum { NATIVE_LEADERBOARD_JOB_QUEUE_SIZE = 8, NATIVE_LEADERBOARD_HTTP_MAX_JSON = 512 * 1024, NATIVE_LEADERBOARD_HTTP_MAX_GHOST = 300 * 1024, NATIVE_LEADERBOARD_CLIENT_VERSION_SIZE = 96 };
enum NativeLeaderboardJobType { NATIVE_LEADERBOARD_JOB_NONE = 0, NATIVE_LEADERBOARD_JOB_REFRESH, NATIVE_LEADERBOARD_JOB_UPLOAD, NATIVE_LEADERBOARD_JOB_GHOST, NATIVE_LEADERBOARD_JOB_VERIFY_ACCOUNT, NATIVE_LEADERBOARD_JOB_LINK_VITA };

struct NativeLeaderboardUpload
{
    u16 trackId;
    u16 characterId;
    char nickname[NATIVE_LEADERBOARD_NICKNAME_SIZE];
    u32 raceTimeMs;
    u32 lapTimeMs;
    u32 relicTimeMs;
    b32 raceBest;
    b32 lapBest;
    b32 relicBest;
    void *ghostData;
    int ghostSize;
};

struct NativeLeaderboardJob { int type; u64 recordId; struct NativeLeaderboardUpload upload; };
struct NativeLeaderboardHttpBuffer { u8 *data; size_t size; size_t capacity; size_t limit; };
struct NativeLeaderboardGhostHeaders { char sha256[65]; };

struct NativeLeaderboardContext
{
    SDL_Mutex *mutex;
    SDL_Condition *condition;
    SDL_Thread *thread;
    b32 initialized;
    b32 shutdown;
    b32 workerBusy;
    char clientVersion[NATIVE_LEADERBOARD_CLIENT_VERSION_SIZE];
    struct NativeLeaderboardJob jobs[NATIVE_LEADERBOARD_JOB_QUEUE_SIZE];
    int jobRead;
    int jobWrite;
    int jobCount;
    struct NativeLeaderboardTrack tracks[NATIVE_LEADERBOARD_TRACK_COUNT];
    b32 hasCache;
    u64 cacheTimestampMs;
    b32 refreshInFlight;
    b32 vitaLinkInFlight;
    int ghostState;
    void *ghostData;
    int ghostSize;
    struct NativeLeaderboardUpload pendingUpload;
    b32 pendingUploadValid;
};

static struct NativeLeaderboardContext s_nativeLeaderboard;
struct NativeLeaderboardJson { const char *p; const char *end; };

static b32 NativeLeaderboard_InitClientVersion(void)
{
#if defined(__vita__)
    static const char hex[] = "0123456789ABCDEF";
    u8 buffer[16 * 1024];
    u8 digest[SHA256_DIGEST_LENGTH];
    SHA256_CTX context;
    SceUID fd = sceIoOpen("app0:/eboot.bin", SCE_O_RDONLY, 0);
    if (fd < 0)
    {
        Platform_LogError("[CTR Leaderboard] Failed to open app0:/eboot.bin for client fingerprint: 0x%08X\n", fd);
        return false;
    }

    if (SHA256_Init(&context) != 1)
    {
        sceIoClose(fd);
        Platform_LogError("[CTR Leaderboard] Failed to initialize eboot SHA-256\n");
        return false;
    }

    for (;;)
    {
        int bytesRead = sceIoRead(fd, buffer, sizeof(buffer));
        if (bytesRead < 0)
        {
            sceIoClose(fd);
            Platform_LogError("[CTR Leaderboard] Failed to read app0:/eboot.bin for client fingerprint: 0x%08X\n", bytesRead);
            return false;
        }
        if (bytesRead == 0)
        {
            break;
        }
        if (SHA256_Update(&context, buffer, (size_t)bytesRead) != 1)
        {
            sceIoClose(fd);
            Platform_LogError("[CTR Leaderboard] Failed to update eboot SHA-256\n");
            return false;
        }
    }
    sceIoClose(fd);

    if (SHA256_Final(digest, &context) != 1)
    {
        Platform_LogError("[CTR Leaderboard] Failed to finalize eboot SHA-256\n");
        return false;
    }

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        s_nativeLeaderboard.clientVersion[i * 2] = hex[digest[i] >> 4];
        s_nativeLeaderboard.clientVersion[i * 2 + 1] = hex[digest[i] & 0xf];
    }
    s_nativeLeaderboard.clientVersion[sizeof(s_nativeLeaderboard.clientVersion) - 1] = '\0';
    return true;
#else
    snprintf(s_nativeLeaderboard.clientVersion, sizeof(s_nativeLeaderboard.clientVersion), "PC:%s:%s", CTR_NATIVE_VERSION, CTR_NATIVE_BUILD_ID);
    return true;
#endif
}

static void NativeLeaderboard_FreeUpload(struct NativeLeaderboardUpload *upload)
{
    if (upload == NULL) return;
    free(upload->ghostData);
    memset(upload, 0, sizeof(*upload));
}

static size_t NativeLeaderboard_WriteCallback(void *contents, size_t size, size_t count, void *userData)
{
    struct NativeLeaderboardHttpBuffer *buffer = userData;
    size_t bytes = size * count;
    if ((bytes == 0) || (buffer == NULL)) return bytes;
    if ((buffer->size + bytes) > buffer->limit) return 0;

    size_t needed = buffer->size + bytes + 1;
    if (needed > buffer->capacity)
    {
        size_t capacity = buffer->capacity != 0 ? buffer->capacity : 4096;
        while (capacity < needed) capacity *= 2;
        if (capacity > buffer->limit + 1) capacity = buffer->limit + 1;
        u8 *newData = realloc(buffer->data, capacity);
        if (newData == NULL) return 0;
        buffer->data = newData;
        buffer->capacity = capacity;
    }

    memcpy(buffer->data + buffer->size, contents, bytes);
    buffer->size += bytes;
    buffer->data[buffer->size] = 0;
    return bytes;
}

static size_t NativeLeaderboard_HeaderCallback(char *data, size_t size, size_t count, void *userData)
{
    struct NativeLeaderboardGhostHeaders *headers = userData;
    size_t bytes = size * count;
    static const char prefix[] = "X-Ghost-SHA256:";
    if ((headers != NULL) && (bytes > sizeof(prefix) - 1))
    {
        b32 match = true;
        for (size_t i = 0; i < sizeof(prefix) - 1; i++)
        {
            if (tolower((unsigned char)data[i]) != tolower((unsigned char)prefix[i])) { match = false; break; }
        }
        if (match)
        {
            const char *p = data + sizeof(prefix) - 1;
            const char *end = data + bytes;
            while ((p < end) && isspace((unsigned char)*p)) p++;
            int length = 0;
            while ((p < end) && (length < 64) && isxdigit((unsigned char)*p)) headers->sha256[length++] = (char)toupper((unsigned char)*p++);
            headers->sha256[length] = '\0';
        }
    }
    return bytes;
}

#if defined(__vita__)
static void NativeLeaderboard_SetCommonCurlOptions(CURL *curl)
{
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 8L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "CTR-Native/" CTR_NATIVE_VERSION);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
}

static b32 NativeLeaderboard_HttpGet(const char *url, struct NativeLeaderboardHttpBuffer *buffer, struct NativeLeaderboardGhostHeaders *headers)
{
    CURL *curl = curl_easy_init();
    if (curl == NULL) return false;
    NativeLeaderboard_SetCommonCurlOptions(curl);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NativeLeaderboard_WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
    if (headers != NULL)
    {
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, NativeLeaderboard_HeaderCallback);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, headers);
    }
    CURLcode result = curl_easy_perform(curl);
    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_cleanup(curl);
    return (result == CURLE_OK) && (status == 200);
}

static b32 NativeLeaderboard_HashSha256(const void *data, size_t size, u8 digest[32])
{
    return SHA256(data, size, digest) != NULL;
}
#else
static b32 NativeLeaderboard_BufferAppend(struct NativeLeaderboardHttpBuffer *buffer, const void *data, size_t size)
{
    if ((buffer == NULL) || (data == NULL) || (size == 0)) return size == 0;
    if ((buffer->size + size) > buffer->limit) return false;
    size_t needed = buffer->size + size + 1;
    if (needed > buffer->capacity)
    {
        size_t capacity = buffer->capacity != 0 ? buffer->capacity : 4096;
        while (capacity < needed) capacity *= 2;
        if (capacity > buffer->limit + 1) capacity = buffer->limit + 1;
        u8 *newData = (u8 *)realloc(buffer->data, capacity);
        if (newData == NULL) return false;
        buffer->data = newData;
        buffer->capacity = capacity;
    }
    memcpy(buffer->data + buffer->size, data, size);
    buffer->size += size;
    buffer->data[buffer->size] = 0;
    return true;
}

static b32 NativeLeaderboard_HttpGet(const char *url, struct NativeLeaderboardHttpBuffer *buffer, struct NativeLeaderboardGhostHeaders *headers)
{
    NativeWinHttpResponse response;
    if (!NativeWinHttp_Request("GET", url, NULL, NULL, NULL, 0, buffer->limit, &response))
    {
        NativeWinHttp_FreeResponse(&response);
        return false;
    }
    free(buffer->data);
    buffer->data = response.data;
    buffer->size = response.size;
    buffer->capacity = response.size + 1;
    response.data = NULL;
    if (headers != NULL) snprintf(headers->sha256, sizeof(headers->sha256), "%s", response.ghostSha256);
    NativeWinHttp_FreeResponse(&response);
    return true;
}

static b32 NativeLeaderboard_HashSha256(const void *data, size_t size, u8 digest[32])
{
    return NativeWinHttp_Sha256(data, size, digest) != 0;
}
#endif

static void NativeLeaderboard_JsonWhitespace(struct NativeLeaderboardJson *json)
{
    while ((json->p < json->end) && isspace((unsigned char)*json->p)) json->p++;
}

static b32 NativeLeaderboard_JsonChar(struct NativeLeaderboardJson *json, char c)
{
    NativeLeaderboard_JsonWhitespace(json);
    if ((json->p >= json->end) || (*json->p != c)) return false;
    json->p++;
    return true;
}

static b32 NativeLeaderboard_JsonString(struct NativeLeaderboardJson *json, char *out, int outSize)
{
    NativeLeaderboard_JsonWhitespace(json);
    if ((json->p >= json->end) || (*json->p != '"')) return false;
    json->p++;
    int length = 0;
    while (json->p < json->end)
    {
        char c = *json->p++;
        if (c == '"')
        {
            if ((out != NULL) && (outSize > 0)) out[length < outSize ? length : outSize - 1] = '\0';
            return true;
        }
        if (c == '\\')
        {
            if (json->p >= json->end) return false;
            char e = *json->p++;
            if (e == 'u') { if ((json->end - json->p) < 4) return false; json->p += 4; c = '?'; }
            else if (e == 'n') c = '\n'; else if (e == 'r') c = '\r'; else if (e == 't') c = '\t';
            else if (e == 'b') c = '\b'; else if (e == 'f') c = '\f'; else if ((e == '"') || (e == '\\') || (e == '/')) c = e; else return false;
        }
        if ((out != NULL) && (outSize > 1) && (length < outSize - 1)) out[length] = c;
        length++;
    }
    return false;
}

static b32 NativeLeaderboard_JsonUInt64(struct NativeLeaderboardJson *json, u64 *out)
{
    NativeLeaderboard_JsonWhitespace(json);
    if ((json->p >= json->end) || !isdigit((unsigned char)*json->p)) return false;
    u64 value = 0;
    while ((json->p < json->end) && isdigit((unsigned char)*json->p)) value = value * 10 + (u64)(*json->p++ - '0');
    if (out != NULL) *out = value;
    return true;
}

static b32 NativeLeaderboard_JsonLiteral(struct NativeLeaderboardJson *json, const char *literal)
{
    NativeLeaderboard_JsonWhitespace(json);
    size_t length = strlen(literal);
    if (((size_t)(json->end - json->p) < length) || (memcmp(json->p, literal, length) != 0)) return false;
    json->p += length;
    return true;
}

static b32 NativeLeaderboard_JsonSkipValue(struct NativeLeaderboardJson *json);

static b32 NativeLeaderboard_JsonSkipObject(struct NativeLeaderboardJson *json)
{
    if (!NativeLeaderboard_JsonChar(json, '{')) return false;
    NativeLeaderboard_JsonWhitespace(json);
    if ((json->p < json->end) && (*json->p == '}')) { json->p++; return true; }
    for (;;)
    {
        if (!NativeLeaderboard_JsonString(json, NULL, 0) || !NativeLeaderboard_JsonChar(json, ':') || !NativeLeaderboard_JsonSkipValue(json)) return false;
        NativeLeaderboard_JsonWhitespace(json);
        if ((json->p < json->end) && (*json->p == '}')) { json->p++; return true; }
        if (!NativeLeaderboard_JsonChar(json, ',')) return false;
    }
}

static b32 NativeLeaderboard_JsonSkipArray(struct NativeLeaderboardJson *json)
{
    if (!NativeLeaderboard_JsonChar(json, '[')) return false;
    NativeLeaderboard_JsonWhitespace(json);
    if ((json->p < json->end) && (*json->p == ']')) { json->p++; return true; }
    for (;;)
    {
        if (!NativeLeaderboard_JsonSkipValue(json)) return false;
        NativeLeaderboard_JsonWhitespace(json);
        if ((json->p < json->end) && (*json->p == ']')) { json->p++; return true; }
        if (!NativeLeaderboard_JsonChar(json, ',')) return false;
    }
}

static b32 NativeLeaderboard_JsonSkipValue(struct NativeLeaderboardJson *json)
{
    NativeLeaderboard_JsonWhitespace(json);
    if (json->p >= json->end) return false;
    if (*json->p == '{') return NativeLeaderboard_JsonSkipObject(json);
    if (*json->p == '[') return NativeLeaderboard_JsonSkipArray(json);
    if (*json->p == '"') return NativeLeaderboard_JsonString(json, NULL, 0);
    if (*json->p == 't') return NativeLeaderboard_JsonLiteral(json, "true");
    if (*json->p == 'f') return NativeLeaderboard_JsonLiteral(json, "false");
    if (*json->p == 'n') return NativeLeaderboard_JsonLiteral(json, "null");
    if ((*json->p == '-') || isdigit((unsigned char)*json->p))
    {
        if (*json->p == '-') json->p++;
        while ((json->p < json->end) && (isdigit((unsigned char)*json->p) || (*json->p == '.') || (*json->p == 'e') || (*json->p == 'E') || (*json->p == '+') || (*json->p == '-'))) json->p++;
        return true;
    }
    return false;
}

static b32 NativeLeaderboard_ParseEntry(struct NativeLeaderboardJson *json, struct NativeLeaderboardEntry *entry)
{
    memset(entry, 0, sizeof(*entry));
    if (!NativeLeaderboard_JsonChar(json, '{')) return false;
    for (;;)
    {
        NativeLeaderboard_JsonWhitespace(json);
        if ((json->p < json->end) && (*json->p == '}'))
        {
            json->p++;
            return entry->recordId != 0 && entry->timeMs != 0;
        }

        char key[32];
        if (!NativeLeaderboard_JsonString(json, key, sizeof(key)) || !NativeLeaderboard_JsonChar(json, ':')) return false;
        if (strcmp(key, "id") == 0)
        {
            if (!NativeLeaderboard_JsonUInt64(json, &entry->recordId)) return false;
        }
        else if (strcmp(key, "user_id") == 0)
        {
            if (!NativeLeaderboard_JsonString(json, entry->userId, sizeof(entry->userId))) return false;
        }
        else if (strcmp(key, "nickname") == 0)
        {
            if (!NativeLeaderboard_JsonString(json, entry->nickname, sizeof(entry->nickname))) return false;
        }
        else if (strcmp(key, "character_id") == 0)
        {
            u64 value;
            if (!NativeLeaderboard_JsonUInt64(json, &value) || value >= 0x10) return false;
            entry->characterId = (u16)value;
        }
        else if (strcmp(key, "time_ms") == 0)
        {
            u64 value;
            if (!NativeLeaderboard_JsonUInt64(json, &value) || value > 0xffffffffu) return false;
            entry->timeMs = (u32)value;
        }
        else if (strcmp(key, "has_ghost") == 0)
        {
            if (NativeLeaderboard_JsonLiteral(json, "true")) entry->hasGhost = true;
            else if (NativeLeaderboard_JsonLiteral(json, "false")) entry->hasGhost = false;
            else return false;
        }
        else if (!NativeLeaderboard_JsonSkipValue(json)) return false;

        NativeLeaderboard_JsonWhitespace(json);
        if ((json->p < json->end) && (*json->p == '}')) continue;
        if (!NativeLeaderboard_JsonChar(json, ',')) return false;
    }
}

static b32 NativeLeaderboard_ParseEntryArray(struct NativeLeaderboardJson *json, struct NativeLeaderboardEntry *entries, int maxEntries, int *outCount)
{
    if (!NativeLeaderboard_JsonChar(json, '[')) return false;
    int count = 0;
    NativeLeaderboard_JsonWhitespace(json);
    if ((json->p < json->end) && (*json->p == ']')) { json->p++; *outCount = 0; return true; }
    for (;;)
    {
        struct NativeLeaderboardEntry entry;
        if (!NativeLeaderboard_ParseEntry(json, &entry)) return false;
        if (count < maxEntries) entries[count++] = entry;
        NativeLeaderboard_JsonWhitespace(json);
        if ((json->p < json->end) && (*json->p == ']')) { json->p++; *outCount = count; return true; }
        if (!NativeLeaderboard_JsonChar(json, ',')) return false;
    }
}

static b32 NativeLeaderboard_ParseTrack(struct NativeLeaderboardJson *json, struct NativeLeaderboardTrack tracks[NATIVE_LEADERBOARD_TRACK_COUNT])
{
    int trackId = -1;
    struct NativeLeaderboardTrack track;
    memset(&track, 0, sizeof(track));
    if (!NativeLeaderboard_JsonChar(json, '{')) return false;
    for (;;)
    {
        NativeLeaderboard_JsonWhitespace(json);
        if ((json->p < json->end) && (*json->p == '}'))
        {
            json->p++;
            if ((trackId >= 0) && (trackId < NATIVE_LEADERBOARD_TRACK_COUNT)) tracks[trackId] = track;
            return trackId >= 0;
        }

        char key[32];
        if (!NativeLeaderboard_JsonString(json, key, sizeof(key)) || !NativeLeaderboard_JsonChar(json, ':')) return false;
        if (strcmp(key, "track_id") == 0)
        {
            u64 value;
            if (!NativeLeaderboard_JsonUInt64(json, &value) || value >= NATIVE_LEADERBOARD_TRACK_COUNT) return false;
            trackId = (int)value;
        }
        else if (strcmp(key, "race") == 0)
        {
            if (!NativeLeaderboard_ParseEntryArray(json, track.race, NATIVE_LEADERBOARD_RACE_COUNT, &track.raceCount)) return false;
        }
        else if (strcmp(key, "lap") == 0)
        {
            struct NativeLeaderboardEntry lap[1];
            int count = 0;
            if (!NativeLeaderboard_ParseEntryArray(json, lap, 1, &count)) return false;
            if (count > 0) { track.lap = lap[0]; track.hasLap = true; }
        }
        else if (strcmp(key, "relic") == 0)
        {
            if (!NativeLeaderboard_ParseEntryArray(json, track.relic, NATIVE_LEADERBOARD_RELIC_COUNT, &track.relicCount)) return false;
        }
        else if (!NativeLeaderboard_JsonSkipValue(json)) return false;
        NativeLeaderboard_JsonWhitespace(json);
        if ((json->p < json->end) && (*json->p == '}')) continue;
        if (!NativeLeaderboard_JsonChar(json, ',')) return false;
    }
}

static b32 NativeLeaderboard_ParseTracks(struct NativeLeaderboardJson *json, struct NativeLeaderboardTrack tracks[NATIVE_LEADERBOARD_TRACK_COUNT])
{
    if (!NativeLeaderboard_JsonChar(json, '[')) return false;
    NativeLeaderboard_JsonWhitespace(json);
    if ((json->p < json->end) && (*json->p == ']')) { json->p++; return true; }
    for (;;)
    {
        if (!NativeLeaderboard_ParseTrack(json, tracks)) return false;
        NativeLeaderboard_JsonWhitespace(json);
        if ((json->p < json->end) && (*json->p == ']')) { json->p++; return true; }
        if (!NativeLeaderboard_JsonChar(json, ',')) return false;
    }
}

static b32 NativeLeaderboard_ParseJson(const char *data, size_t size, struct NativeLeaderboardTrack tracks[NATIVE_LEADERBOARD_TRACK_COUNT])
{
    memset(tracks, 0, sizeof(struct NativeLeaderboardTrack) * NATIVE_LEADERBOARD_TRACK_COUNT);
    struct NativeLeaderboardJson json = {data, data + size};
    if (!NativeLeaderboard_JsonChar(&json, '{')) return false;
    b32 ok = false;
    b32 sawTracks = false;
    for (;;)
    {
        NativeLeaderboard_JsonWhitespace(&json);
        if ((json.p < json.end) && (*json.p == '}')) { json.p++; return ok && sawTracks; }
        char key[32];
        if (!NativeLeaderboard_JsonString(&json, key, sizeof(key)) || !NativeLeaderboard_JsonChar(&json, ':')) return false;
        if (strcmp(key, "ok") == 0)
        {
            if (NativeLeaderboard_JsonLiteral(&json, "true")) ok = true;
            else if (NativeLeaderboard_JsonLiteral(&json, "false")) ok = false;
            else return false;
        }
        else if (strcmp(key, "tracks") == 0)
        {
            if (!NativeLeaderboard_ParseTracks(&json, tracks)) return false;
            sawTracks = true;
        }
        else if (!NativeLeaderboard_JsonSkipValue(&json)) return false;
        NativeLeaderboard_JsonWhitespace(&json);
        if ((json.p < json.end) && (*json.p == '}')) continue;
        if (!NativeLeaderboard_JsonChar(&json, ',')) return false;
    }
}


static b32 NativeLeaderboard_ParseAccountJson(const char *data, size_t size, char *username, int usernameSize, char *publicId, int publicIdSize)
{
    struct NativeLeaderboardJson json = {data, data + size};
    b32 ok = false;
    b32 sawUsername = false;
    b32 sawPublicId = false;
    if (!NativeLeaderboard_JsonChar(&json, '{')) return false;
    for (;;)
    {
        NativeLeaderboard_JsonWhitespace(&json);
        if ((json.p < json.end) && (*json.p == '}')) return ok && sawUsername && sawPublicId;
        char key[32];
        if (!NativeLeaderboard_JsonString(&json, key, sizeof(key)) || !NativeLeaderboard_JsonChar(&json, ':')) return false;
        if (strcmp(key, "ok") == 0)
        {
            if (NativeLeaderboard_JsonLiteral(&json, "true")) ok = true;
            else if (NativeLeaderboard_JsonLiteral(&json, "false")) ok = false;
            else return false;
        }
        else if (strcmp(key, "username") == 0)
        {
            if (!NativeLeaderboard_JsonString(&json, username, usernameSize)) return false;
            sawUsername = true;
        }
        else if (strcmp(key, "public_id") == 0)
        {
            if (!NativeLeaderboard_JsonString(&json, publicId, publicIdSize)) return false;
            sawPublicId = true;
        }
        else if (!NativeLeaderboard_JsonSkipValue(&json)) return false;
        NativeLeaderboard_JsonWhitespace(&json);
        if ((json.p < json.end) && (*json.p == '}')) continue;
        if (!NativeLeaderboard_JsonChar(&json, ',')) return false;
    }
}

#if defined(_WIN32) && !defined(__vita__)
static void NativeLeaderboard_ProcessAccountVerify(void)
{
    const char *token = NativePcAccount_GetToken();
    if (token == NULL) return;
    char verifyBody[128];
    int verifyBodySize = snprintf(verifyBody, sizeof(verifyBody), "account_key=%s", token);
    if ((verifyBodySize <= 0) || ((size_t)verifyBodySize >= sizeof(verifyBody))) return;

    NativeWinHttpResponse response;
    memset(&response, 0, sizeof(response));
    if (!NativeWinHttp_Request("POST", CTR_NATIVE_LEADERBOARD_ME_URL, token,
                               "application/x-www-form-urlencoded", verifyBody, (size_t)verifyBodySize,
                               16 * 1024, &response))
    {
        Platform_Log("[CTR Account] Account key verification failed http=%ld\n", response.status);
        NativeWinHttp_FreeResponse(&response);
        return;
    }
    char username[18] = {0};
    char publicId[24] = {0};
    if (NativeLeaderboard_ParseAccountJson((const char *)response.data, response.size, username, sizeof(username), publicId, sizeof(publicId)))
    {
        NativePcAccount_SetVerifiedIdentity(username, publicId);
        Platform_Log("[CTR Account] Verified PC account %s (%s)\n", username, publicId);
    }
    else
    {
        Platform_Log("[CTR Account] Invalid /api/me response\n");
    }
    NativeWinHttp_FreeResponse(&response);
}
#endif

#if defined(__vita__)
#define NATIVE_LEADERBOARD_VITA_LINK_KEY_PATH "ux0:data/ctr/highoctane.key"
#define NATIVE_LEADERBOARD_KEY_HEADER "CTR-HIGH-OCTANE-PC-KEY-V1"

static b32 NativeLeaderboard_IsAccountTokenValid(const char *token)
{
    if ((token == NULL) || (strlen(token) != 43)) return false;
    for (const char *p = token; *p != '\0'; p++)
    {
        if (!(isalnum((unsigned char)*p) || (*p == '-') || (*p == '_'))) return false;
    }
    return true;
}

static b32 NativeLeaderboard_VitaLinkKeyExists(void)
{
    SceUID fd = sceIoOpen(NATIVE_LEADERBOARD_VITA_LINK_KEY_PATH, SCE_O_RDONLY, 0);
    if (fd < 0) return false;
    sceIoClose(fd);
    return true;
}

static b32 NativeLeaderboard_LoadVitaLinkKey(char token[96])
{
    token[0] = '\0';
    SceUID fd = sceIoOpen(NATIVE_LEADERBOARD_VITA_LINK_KEY_PATH, SCE_O_RDONLY, 0);
    if (fd < 0) return false;

    char contents[1024];
    int bytesRead = sceIoRead(fd, contents, sizeof(contents) - 1);
    sceIoClose(fd);
    if ((bytesRead <= 0) || (bytesRead >= (int)sizeof(contents))) return false;
    contents[bytesRead] = '\0';

    char *cursor = contents;
    char *lineEnd = cursor;
    while ((*lineEnd != '\0') && (*lineEnd != '\r') && (*lineEnd != '\n')) lineEnd++;
    char saved = *lineEnd;
    *lineEnd = '\0';
    b32 headerOk = strcmp(cursor, NATIVE_LEADERBOARD_KEY_HEADER) == 0;
    *lineEnd = saved;
    if (!headerOk) return false;

    cursor = lineEnd;
    while ((*cursor == '\r') || (*cursor == '\n')) cursor++;
    while (*cursor != '\0')
    {
        lineEnd = cursor;
        while ((*lineEnd != '\0') && (*lineEnd != '\r') && (*lineEnd != '\n')) lineEnd++;
        saved = *lineEnd;
        *lineEnd = '\0';
        if (strncmp(cursor, "token=", 6) == 0)
        {
            snprintf(token, 96, "%s", cursor + 6);
            *lineEnd = saved;
            break;
        }
        *lineEnd = saved;
        cursor = lineEnd;
        while ((*cursor == '\r') || (*cursor == '\n')) cursor++;
    }

    if (!NativeLeaderboard_IsAccountTokenValid(token))
    {
        memset(token, 0, 96);
        return false;
    }
    return true;
}

static void NativeLeaderboard_ProcessVitaLink(void)
{
    if (!NativeNetwork_IsInternetConnected())
    {
        Platform_Log("[CTR Account] Vita account link deferred: internet unavailable\n");
        return;
    }

    char token[96];
    if (!NativeLeaderboard_LoadVitaLinkKey(token))
    {
        Platform_LogWarn("[CTR Account] Invalid %s\n", NATIVE_LEADERBOARD_VITA_LINK_KEY_PATH);
        return;
    }

    u8 hash[NATIVE_USER_ID_HASH_SIZE];
    if (!NativeUserId_CopyHash(hash))
    {
        memset(token, 0, sizeof(token));
        Platform_LogWarn("[CTR Account] Cannot read Vita user identity\n");
        return;
    }

    static const char hex[] = "0123456789ABCDEF";
    char hashHex[NATIVE_USER_ID_HASH_SIZE * 2 + 1];
    for (int i = 0; i < NATIVE_USER_ID_HASH_SIZE; i++)
    {
        hashHex[i * 2] = hex[hash[i] >> 4];
        hashHex[i * 2 + 1] = hex[hash[i] & 0xf];
    }
    hashHex[sizeof(hashHex) - 1] = '\0';
    memset(hash, 0, sizeof(hash));

    struct curl_httppost *form = NULL;
    struct curl_httppost *last = NULL;
    curl_formadd(&form, &last, CURLFORM_COPYNAME, "account_key", CURLFORM_COPYCONTENTS, token, CURLFORM_END);
    curl_formadd(&form, &last, CURLFORM_COPYNAME, "vita_user_hash", CURLFORM_COPYCONTENTS, hashHex, CURLFORM_END);
    curl_formadd(&form, &last, CURLFORM_COPYNAME, "client_version", CURLFORM_COPYCONTENTS, s_nativeLeaderboard.clientVersion, CURLFORM_END);

    CURL *curl = curl_easy_init();
    if (curl == NULL)
    {
        curl_formfree(form);
        memset(token, 0, sizeof(token));
        return;
    }

    struct NativeLeaderboardHttpBuffer response;
    memset(&response, 0, sizeof(response));
    response.limit = 16 * 1024;
    NativeLeaderboard_SetCommonCurlOptions(curl);
    curl_easy_setopt(curl, CURLOPT_URL, CTR_NATIVE_LEADERBOARD_LINK_VITA_URL);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, form);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NativeLeaderboard_WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    CURLcode result = curl_easy_perform(curl);
    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);

    char username[18] = {0};
    char publicId[24] = {0};
    b32 linked = (result == CURLE_OK) && (status >= 200) && (status < 300) &&
                 NativeLeaderboard_ParseAccountJson((const char *)response.data, response.size,
                                                    username, sizeof(username), publicId, sizeof(publicId));
    if (linked)
    {
        sceIoRemove(NATIVE_LEADERBOARD_VITA_LINK_KEY_PATH);
        Platform_Log("[CTR Account] Vita linked to %s (%s)\n", username, publicId);
    }
    else
    {
        Platform_Log("[CTR Account] Vita account link failed curl=%d http=%ld\n", (int)result, status);
    }

    free(response.data);
    curl_easy_cleanup(curl);
    curl_formfree(form);
    memset(token, 0, sizeof(token));
}
#endif

static b32 NativeLeaderboard_Enqueue(struct NativeLeaderboardJob *job)
{
    if (!s_nativeLeaderboard.initialized || (job == NULL)) return false;
    SDL_LockMutex(s_nativeLeaderboard.mutex);
    if (s_nativeLeaderboard.jobCount >= NATIVE_LEADERBOARD_JOB_QUEUE_SIZE)
    {
        SDL_UnlockMutex(s_nativeLeaderboard.mutex);
        return false;
    }
    s_nativeLeaderboard.jobs[s_nativeLeaderboard.jobWrite] = *job;
    s_nativeLeaderboard.jobWrite = (s_nativeLeaderboard.jobWrite + 1) % NATIVE_LEADERBOARD_JOB_QUEUE_SIZE;
    s_nativeLeaderboard.jobCount++;
    SDL_SignalCondition(s_nativeLeaderboard.condition);
    SDL_UnlockMutex(s_nativeLeaderboard.mutex);
    return true;
}

#if defined(__vita__)
static void NativeLeaderboard_RequestVitaLinkIfPresent(void)
{
    if (!s_nativeLeaderboard.initialized || !NativeLeaderboard_VitaLinkKeyExists()) return;

    SDL_LockMutex(s_nativeLeaderboard.mutex);
    if (s_nativeLeaderboard.vitaLinkInFlight)
    {
        SDL_UnlockMutex(s_nativeLeaderboard.mutex);
        return;
    }
    s_nativeLeaderboard.vitaLinkInFlight = true;
    SDL_UnlockMutex(s_nativeLeaderboard.mutex);

    struct NativeLeaderboardJob linkJob;
    memset(&linkJob, 0, sizeof(linkJob));
    linkJob.type = NATIVE_LEADERBOARD_JOB_LINK_VITA;
    if (!NativeLeaderboard_Enqueue(&linkJob))
    {
        SDL_LockMutex(s_nativeLeaderboard.mutex);
        s_nativeLeaderboard.vitaLinkInFlight = false;
        SDL_UnlockMutex(s_nativeLeaderboard.mutex);
    }
}
#endif

static void NativeLeaderboard_ProcessRefresh(void)
{
    struct NativeLeaderboardHttpBuffer buffer;
    memset(&buffer, 0, sizeof(buffer));
    buffer.limit = NATIVE_LEADERBOARD_HTTP_MAX_JSON;
    struct NativeLeaderboardTrack tracks[NATIVE_LEADERBOARD_TRACK_COUNT];
    b32 success = NativeLeaderboard_HttpGet(CTR_NATIVE_LEADERBOARD_GET_URL, &buffer, NULL) &&
                  NativeLeaderboard_ParseJson((const char *)buffer.data, buffer.size, tracks);

    SDL_LockMutex(s_nativeLeaderboard.mutex);
    if (success)
    {
        memcpy(s_nativeLeaderboard.tracks, tracks, sizeof(tracks));
        s_nativeLeaderboard.hasCache = true;
        s_nativeLeaderboard.cacheTimestampMs = SDL_GetTicks();
    }
    s_nativeLeaderboard.refreshInFlight = false;
    SDL_UnlockMutex(s_nativeLeaderboard.mutex);
    free(buffer.data);
}

#if defined(__vita__)
static void NativeLeaderboard_AddFormText(struct curl_httppost **first, struct curl_httppost **last, const char *name, const char *value)
{
    curl_formadd(first, last, CURLFORM_COPYNAME, name, CURLFORM_COPYCONTENTS, value, CURLFORM_END);
}
#endif

#if defined(_WIN32) && !defined(__vita__)
static b32 NativeLeaderboard_MultipartAppendText(struct NativeLeaderboardHttpBuffer *body, const char *boundary,
                                                 const char *name, const char *value)
{
    char header[512];
    int count = snprintf(header, sizeof(header), "--%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
                         boundary, name, value != NULL ? value : "");
    return (count > 0) && ((size_t)count < sizeof(header)) && NativeLeaderboard_BufferAppend(body, header, (size_t)count);
}

static b32 NativeLeaderboard_MultipartAppendFile(struct NativeLeaderboardHttpBuffer *body, const char *boundary,
                                                 const char *name, const char *filename, const void *data, size_t size)
{
    char header[512];
    int count = snprintf(header, sizeof(header),
                         "--%s\r\nContent-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n"
                         "Content-Type: application/octet-stream\r\n\r\n",
                         boundary, name, filename);
    if ((count <= 0) || ((size_t)count >= sizeof(header))) return false;
    return NativeLeaderboard_BufferAppend(body, header, (size_t)count) &&
           NativeLeaderboard_BufferAppend(body, data, size) &&
           NativeLeaderboard_BufferAppend(body, "\r\n", 2);
}
#endif

static void NativeLeaderboard_ProcessUpload(struct NativeLeaderboardUpload *upload)
{
    if (!NativeNetwork_IsInternetConnected()) { NativeLeaderboard_FreeUpload(upload); return; }

    char trackText[16], characterText[16], raceText[16], lapText[16], relicText[16];
    snprintf(trackText, sizeof(trackText), "%u", upload->trackId);
    snprintf(characterText, sizeof(characterText), "%u", upload->characterId);
    snprintf(raceText, sizeof(raceText), "%u", upload->raceTimeMs);
    snprintf(lapText, sizeof(lapText), "%u", upload->lapTimeMs);
    snprintf(relicText, sizeof(relicText), "%u", upload->relicTimeMs);

#if defined(__vita__)
    u8 hash[NATIVE_USER_ID_HASH_SIZE];
    if (!NativeUserId_CopyHash(hash)) { NativeLeaderboard_FreeUpload(upload); return; }
    static const char hex[] = "0123456789ABCDEF";
    char hashHex[NATIVE_USER_ID_HASH_SIZE * 2 + 1];
    for (int i = 0; i < NATIVE_USER_ID_HASH_SIZE; i++)
    {
        hashHex[i * 2] = hex[hash[i] >> 4];
        hashHex[i * 2 + 1] = hex[hash[i] & 0xf];
    }
    hashHex[sizeof(hashHex) - 1] = '\0';
    memset(hash, 0, sizeof(hash));

    struct curl_httppost *form = NULL;
    struct curl_httppost *last = NULL;
    NativeLeaderboard_AddFormText(&form, &last, "user_hash", hashHex);
    NativeLeaderboard_AddFormText(&form, &last, "nickname", upload->nickname);
    NativeLeaderboard_AddFormText(&form, &last, "track_id", trackText);
    NativeLeaderboard_AddFormText(&form, &last, "character_id", characterText);
    NativeLeaderboard_AddFormText(&form, &last, "client_version", s_nativeLeaderboard.clientVersion);
    if (upload->raceBest)
    {
        NativeLeaderboard_AddFormText(&form, &last, "race_time_ms", raceText);
        curl_formadd(&form, &last,
                     CURLFORM_COPYNAME, "ghost",
                     CURLFORM_BUFFER, "record.ngr",
                     CURLFORM_BUFFERPTR, upload->ghostData,
                     CURLFORM_BUFFERLENGTH, (long)upload->ghostSize,
                     CURLFORM_CONTENTTYPE, "application/octet-stream",
                     CURLFORM_END);
    }
    if (upload->lapBest) NativeLeaderboard_AddFormText(&form, &last, "lap_time_ms", lapText);
    if (upload->relicBest)
    {
        NativeLeaderboard_AddFormText(&form, &last, "relic_time_ms", relicText);
        curl_formadd(&form, &last,
                     CURLFORM_COPYNAME, "ghost",
                     CURLFORM_BUFFER, "record.ngr",
                     CURLFORM_BUFFERPTR, upload->ghostData,
                     CURLFORM_BUFFERLENGTH, (long)upload->ghostSize,
                     CURLFORM_CONTENTTYPE, "application/octet-stream",
                     CURLFORM_END);
    }

    CURL *curl = curl_easy_init();
    if (curl != NULL)
    {
        struct NativeLeaderboardHttpBuffer response;
        memset(&response, 0, sizeof(response));
        response.limit = 64 * 1024;
        NativeLeaderboard_SetCommonCurlOptions(curl);
        curl_easy_setopt(curl, CURLOPT_URL, CTR_NATIVE_LEADERBOARD_UPLOAD_URL);
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, form);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NativeLeaderboard_WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        CURLcode result = curl_easy_perform(curl);
        long status = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        if ((result != CURLE_OK) || (status < 200) || (status >= 300))
            Platform_Log("[CTR Leaderboard] upload failed curl=%d http=%ld\n", (int)result, status);
        free(response.data);
        curl_easy_cleanup(curl);
    }
    curl_formfree(form);
#else
    const char *token = NativePcAccount_GetToken();
    if (token == NULL)
    {
        Platform_Log("[CTR Leaderboard] PC record not uploaded: highoctane.key is missing or invalid\n");
        NativeLeaderboard_FreeUpload(upload);
        return;
    }

    static const char boundary[] = "----------------CTRHighOctanePC926";
    struct NativeLeaderboardHttpBuffer body;
    memset(&body, 0, sizeof(body));
    body.limit = NATIVE_LEADERBOARD_HTTP_MAX_GHOST + 16 * 1024;
    b32 ok = NativeLeaderboard_MultipartAppendText(&body, boundary, "account_key", token) &&
             NativeLeaderboard_MultipartAppendText(&body, boundary, "platform", "pc") &&
             NativeLeaderboard_MultipartAppendText(&body, boundary, "nickname", upload->nickname) &&
             NativeLeaderboard_MultipartAppendText(&body, boundary, "track_id", trackText) &&
             NativeLeaderboard_MultipartAppendText(&body, boundary, "character_id", characterText) &&
             NativeLeaderboard_MultipartAppendText(&body, boundary, "client_version", s_nativeLeaderboard.clientVersion);
    if (ok && upload->raceBest)
        ok = NativeLeaderboard_MultipartAppendText(&body, boundary, "race_time_ms", raceText) &&
             NativeLeaderboard_MultipartAppendFile(&body, boundary, "ghost", "record.ngr", upload->ghostData, (size_t)upload->ghostSize);
    if (ok && upload->lapBest)
        ok = NativeLeaderboard_MultipartAppendText(&body, boundary, "lap_time_ms", lapText);
    if (ok && upload->relicBest)
        ok = NativeLeaderboard_MultipartAppendText(&body, boundary, "relic_time_ms", relicText) &&
             NativeLeaderboard_MultipartAppendFile(&body, boundary, "ghost", "record.ngr", upload->ghostData, (size_t)upload->ghostSize);
    if (ok)
    {
        char footer[128];
        int footerSize = snprintf(footer, sizeof(footer), "--%s--\r\n", boundary);
        ok = (footerSize > 0) && NativeLeaderboard_BufferAppend(&body, footer, (size_t)footerSize);
    }

    if (ok)
    {
        char contentType[128];
        snprintf(contentType, sizeof(contentType), "multipart/form-data; boundary=%s", boundary);
        NativeWinHttpResponse response;
        memset(&response, 0, sizeof(response));
        if (!NativeWinHttp_Request("POST", CTR_NATIVE_LEADERBOARD_UPLOAD_URL, token,
                                   contentType, body.data, body.size, 64 * 1024, &response))
        {
            Platform_Log("[CTR Leaderboard] PC upload failed http=%ld\n", response.status);
        }
        NativeWinHttp_FreeResponse(&response);
    }
    free(body.data);
#endif
    NativeLeaderboard_FreeUpload(upload);
}

static void NativeLeaderboard_ProcessGhost(u64 recordId)
{
    char url[256];
    snprintf(url, sizeof(url), "%s?ghost=%llu", CTR_NATIVE_LEADERBOARD_GET_URL, (unsigned long long)recordId);
    struct NativeLeaderboardHttpBuffer buffer;
    memset(&buffer, 0, sizeof(buffer));
    buffer.limit = NATIVE_LEADERBOARD_HTTP_MAX_GHOST;
    struct NativeLeaderboardGhostHeaders headers;
    memset(&headers, 0, sizeof(headers));
    b32 success = NativeNetwork_IsInternetConnected() && NativeLeaderboard_HttpGet(url, &buffer, &headers);

    if (success && (headers.sha256[0] != '\0'))
    {
        u8 digest[32];
        char digestHex[65];
        static const char hex[] = "0123456789ABCDEF";
        if (!NativeLeaderboard_HashSha256(buffer.data, buffer.size, digest)) success = false;
        for (int i = 0; success && (i < 32); i++)
        {
            digestHex[i * 2] = hex[digest[i] >> 4];
            digestHex[i * 2 + 1] = hex[digest[i] & 0xf];
        }
        digestHex[sizeof(digestHex) - 1] = '\0';
        success = strcmp(digestHex, headers.sha256) == 0;
    }

    SDL_LockMutex(s_nativeLeaderboard.mutex);
    free(s_nativeLeaderboard.ghostData);
    s_nativeLeaderboard.ghostData = NULL;
    s_nativeLeaderboard.ghostSize = 0;
    if (success)
    {
        s_nativeLeaderboard.ghostData = buffer.data;
        s_nativeLeaderboard.ghostSize = (int)buffer.size;
        s_nativeLeaderboard.ghostState = NATIVE_LEADERBOARD_TRANSFER_READY;
        buffer.data = NULL;
    }
    else s_nativeLeaderboard.ghostState = NATIVE_LEADERBOARD_TRANSFER_FAILED;
    SDL_UnlockMutex(s_nativeLeaderboard.mutex);
    free(buffer.data);
}

static int NativeLeaderboard_Worker(void *unused)
{
    (void)unused;
    for (;;)
    {
        struct NativeLeaderboardJob job;
        memset(&job, 0, sizeof(job));
        SDL_LockMutex(s_nativeLeaderboard.mutex);
        while (!s_nativeLeaderboard.shutdown && (s_nativeLeaderboard.jobCount == 0)) SDL_WaitCondition(s_nativeLeaderboard.condition, s_nativeLeaderboard.mutex);
        if (s_nativeLeaderboard.shutdown) { SDL_UnlockMutex(s_nativeLeaderboard.mutex); break; }
        job = s_nativeLeaderboard.jobs[s_nativeLeaderboard.jobRead];
        memset(&s_nativeLeaderboard.jobs[s_nativeLeaderboard.jobRead], 0, sizeof(struct NativeLeaderboardJob));
        s_nativeLeaderboard.jobRead = (s_nativeLeaderboard.jobRead + 1) % NATIVE_LEADERBOARD_JOB_QUEUE_SIZE;
        s_nativeLeaderboard.jobCount--;
        s_nativeLeaderboard.workerBusy = true;
        SDL_UnlockMutex(s_nativeLeaderboard.mutex);

        if (job.type == NATIVE_LEADERBOARD_JOB_REFRESH) NativeLeaderboard_ProcessRefresh();
        else if (job.type == NATIVE_LEADERBOARD_JOB_UPLOAD) NativeLeaderboard_ProcessUpload(&job.upload);
        else if (job.type == NATIVE_LEADERBOARD_JOB_GHOST) NativeLeaderboard_ProcessGhost(job.recordId);
#if defined(_WIN32) && !defined(__vita__)
        else if (job.type == NATIVE_LEADERBOARD_JOB_VERIFY_ACCOUNT) NativeLeaderboard_ProcessAccountVerify();
#endif
#if defined(__vita__)
        else if (job.type == NATIVE_LEADERBOARD_JOB_LINK_VITA) NativeLeaderboard_ProcessVitaLink();
#endif
        else NativeLeaderboard_FreeUpload(&job.upload);

        SDL_LockMutex(s_nativeLeaderboard.mutex);
        s_nativeLeaderboard.workerBusy = false;
#if defined(__vita__)
        if (job.type == NATIVE_LEADERBOARD_JOB_LINK_VITA) s_nativeLeaderboard.vitaLinkInFlight = false;
#endif
        SDL_UnlockMutex(s_nativeLeaderboard.mutex);
    }
    return 0;
}

int NativeLeaderboard_Init(void)
{
    if (s_nativeLeaderboard.initialized) return 1;
    memset(&s_nativeLeaderboard, 0, sizeof(s_nativeLeaderboard));
    if (!NativeLeaderboard_InitClientVersion()) return 0;
#if defined(__vita__)
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) return 0;
#else
    NativePcAccount_Init();
#endif
    s_nativeLeaderboard.mutex = SDL_CreateMutex();
    s_nativeLeaderboard.condition = SDL_CreateCondition();
    if ((s_nativeLeaderboard.mutex == NULL) || (s_nativeLeaderboard.condition == NULL))
    {
        NativeLeaderboard_Shutdown();
        return 0;
    }
    s_nativeLeaderboard.initialized = true;
    s_nativeLeaderboard.thread = SDL_CreateThread(NativeLeaderboard_Worker, "CTR Leaderboard", NULL);
    if (s_nativeLeaderboard.thread == NULL)
    {
        NativeLeaderboard_Shutdown();
        return 0;
    }
#if defined(_WIN32) && !defined(__vita__)
    if (NativePcAccount_IsAvailable())
    {
        struct NativeLeaderboardJob verifyJob;
        memset(&verifyJob, 0, sizeof(verifyJob));
        verifyJob.type = NATIVE_LEADERBOARD_JOB_VERIFY_ACCOUNT;
        NativeLeaderboard_Enqueue(&verifyJob);
    }
#endif
#if defined(__vita__)
    NativeLeaderboard_RequestVitaLinkIfPresent();
#endif
    return 1;
}

void NativeLeaderboard_Shutdown(void)
{
#if defined(__vita__)
    b32 curlWasInitialized = s_nativeLeaderboard.initialized || (s_nativeLeaderboard.mutex != NULL) || (s_nativeLeaderboard.condition != NULL);
#endif
    if (s_nativeLeaderboard.mutex != NULL)
    {
        SDL_LockMutex(s_nativeLeaderboard.mutex);
        s_nativeLeaderboard.shutdown = true;
        if (s_nativeLeaderboard.condition != NULL) SDL_SignalCondition(s_nativeLeaderboard.condition);
        SDL_UnlockMutex(s_nativeLeaderboard.mutex);
    }
    if (s_nativeLeaderboard.thread != NULL) SDL_WaitThread(s_nativeLeaderboard.thread, NULL);
    for (int i = 0; i < NATIVE_LEADERBOARD_JOB_QUEUE_SIZE; i++) NativeLeaderboard_FreeUpload(&s_nativeLeaderboard.jobs[i].upload);
    NativeLeaderboard_FreeUpload(&s_nativeLeaderboard.pendingUpload);
    free(s_nativeLeaderboard.ghostData);
    if (s_nativeLeaderboard.condition != NULL) SDL_DestroyCondition(s_nativeLeaderboard.condition);
    if (s_nativeLeaderboard.mutex != NULL) SDL_DestroyMutex(s_nativeLeaderboard.mutex);
#if defined(__vita__)
    if (curlWasInitialized) curl_global_cleanup();
#endif
    memset(&s_nativeLeaderboard, 0, sizeof(s_nativeLeaderboard));
}

int NativeLeaderboard_IsNetworkIdle(void)
{
    if (!s_nativeLeaderboard.initialized || (s_nativeLeaderboard.mutex == NULL)) return 1;
    SDL_LockMutex(s_nativeLeaderboard.mutex);
    int idle = !s_nativeLeaderboard.workerBusy && (s_nativeLeaderboard.jobCount == 0);
    SDL_UnlockMutex(s_nativeLeaderboard.mutex);
    return idle;
}
int NativeLeaderboard_IsInternetConnected(void)
{
    return NativeNetwork_IsInternetConnected();
}

int NativeLeaderboard_RequestNetworkRestoreProbe(void)
{
    if (!s_nativeLeaderboard.initialized) return 0;

    SDL_LockMutex(s_nativeLeaderboard.mutex);
    if (s_nativeLeaderboard.refreshInFlight)
    {
        SDL_UnlockMutex(s_nativeLeaderboard.mutex);
        return 1;
    }
    s_nativeLeaderboard.refreshInFlight = true;
    SDL_UnlockMutex(s_nativeLeaderboard.mutex);

    struct NativeLeaderboardJob job;
    memset(&job, 0, sizeof(job));
    job.type = NATIVE_LEADERBOARD_JOB_REFRESH;
    if (!NativeLeaderboard_Enqueue(&job))
    {
        SDL_LockMutex(s_nativeLeaderboard.mutex);
        s_nativeLeaderboard.refreshInFlight = false;
        SDL_UnlockMutex(s_nativeLeaderboard.mutex);
        return 0;
    }
    return 1;
}

int NativeLeaderboard_RequestRefresh(void)
{
    if (!s_nativeLeaderboard.initialized || !NativeNetwork_IsInternetConnected()) return 0;
#if defined(__vita__)
    NativeLeaderboard_RequestVitaLinkIfPresent();
#endif
    SDL_LockMutex(s_nativeLeaderboard.mutex);
    u64 now = SDL_GetTicks();
    b32 cacheFresh = s_nativeLeaderboard.hasCache && ((now - s_nativeLeaderboard.cacheTimestampMs) < NATIVE_LEADERBOARD_CACHE_TTL_MS);
    if (cacheFresh || s_nativeLeaderboard.refreshInFlight)
    {
        SDL_UnlockMutex(s_nativeLeaderboard.mutex);
        return 1;
    }
    s_nativeLeaderboard.refreshInFlight = true;
    SDL_UnlockMutex(s_nativeLeaderboard.mutex);
    struct NativeLeaderboardJob job;
    memset(&job, 0, sizeof(job));
    job.type = NATIVE_LEADERBOARD_JOB_REFRESH;
    if (!NativeLeaderboard_Enqueue(&job))
    {
        SDL_LockMutex(s_nativeLeaderboard.mutex);
        s_nativeLeaderboard.refreshInFlight = false;
        SDL_UnlockMutex(s_nativeLeaderboard.mutex);
        return 0;
    }
    return 1;
}

int NativeLeaderboard_HasCache(void)
{
    if (!s_nativeLeaderboard.initialized) return 0;
    SDL_LockMutex(s_nativeLeaderboard.mutex);
    int result = s_nativeLeaderboard.hasCache;
    SDL_UnlockMutex(s_nativeLeaderboard.mutex);
    return result;
}

int NativeLeaderboard_IsRefreshing(void)
{
    if (!s_nativeLeaderboard.initialized) return 0;
    SDL_LockMutex(s_nativeLeaderboard.mutex);
    int result = s_nativeLeaderboard.refreshInFlight;
    SDL_UnlockMutex(s_nativeLeaderboard.mutex);
    return result;
}

int NativeLeaderboard_CopyTrack(int trackId, struct NativeLeaderboardTrack *outTrack)
{
    if (!s_nativeLeaderboard.initialized || (outTrack == NULL) || (trackId < 0) || (trackId >= NATIVE_LEADERBOARD_TRACK_COUNT)) return 0;
    SDL_LockMutex(s_nativeLeaderboard.mutex);
    if (!s_nativeLeaderboard.hasCache) { SDL_UnlockMutex(s_nativeLeaderboard.mutex); return 0; }
    *outTrack = s_nativeLeaderboard.tracks[trackId];
    SDL_UnlockMutex(s_nativeLeaderboard.mutex);
    return 1;
}

int NativeLeaderboard_RequestGhost(u64 recordId)
{
    if (!s_nativeLeaderboard.initialized || !NativeNetwork_IsInternetConnected() || (recordId == 0)) return 0;
    SDL_LockMutex(s_nativeLeaderboard.mutex);
    if (s_nativeLeaderboard.ghostState == NATIVE_LEADERBOARD_TRANSFER_PENDING) { SDL_UnlockMutex(s_nativeLeaderboard.mutex); return 0; }
    free(s_nativeLeaderboard.ghostData);
    s_nativeLeaderboard.ghostData = NULL;
    s_nativeLeaderboard.ghostSize = 0;
    s_nativeLeaderboard.ghostState = NATIVE_LEADERBOARD_TRANSFER_PENDING;
    SDL_UnlockMutex(s_nativeLeaderboard.mutex);
    struct NativeLeaderboardJob job;
    memset(&job, 0, sizeof(job));
    job.type = NATIVE_LEADERBOARD_JOB_GHOST;
    job.recordId = recordId;
    if (!NativeLeaderboard_Enqueue(&job))
    {
        SDL_LockMutex(s_nativeLeaderboard.mutex);
        s_nativeLeaderboard.ghostState = NATIVE_LEADERBOARD_TRANSFER_FAILED;
        SDL_UnlockMutex(s_nativeLeaderboard.mutex);
        return 0;
    }
    return 1;
}

int NativeLeaderboard_GetGhostState(void)
{
    if (!s_nativeLeaderboard.initialized) return NATIVE_LEADERBOARD_TRANSFER_FAILED;
    SDL_LockMutex(s_nativeLeaderboard.mutex);
    int result = s_nativeLeaderboard.ghostState;
    SDL_UnlockMutex(s_nativeLeaderboard.mutex);
    return result;
}

int NativeLeaderboard_TakeGhost(void **data, int *size)
{
    if ((data == NULL) || (size == NULL) || !s_nativeLeaderboard.initialized) return 0;
    SDL_LockMutex(s_nativeLeaderboard.mutex);
    if (s_nativeLeaderboard.ghostState != NATIVE_LEADERBOARD_TRANSFER_READY) { SDL_UnlockMutex(s_nativeLeaderboard.mutex); return 0; }
    *data = s_nativeLeaderboard.ghostData;
    *size = s_nativeLeaderboard.ghostSize;
    s_nativeLeaderboard.ghostData = NULL;
    s_nativeLeaderboard.ghostSize = 0;
    s_nativeLeaderboard.ghostState = NATIVE_LEADERBOARD_TRANSFER_IDLE;
    SDL_UnlockMutex(s_nativeLeaderboard.mutex);
    return 1;
}

void NativeLeaderboard_ClearPendingUpload(void)
{
    NativeLeaderboard_FreeUpload(&s_nativeLeaderboard.pendingUpload);
    s_nativeLeaderboard.pendingUploadValid = false;
}

void NativeLeaderboard_StageTimeTrialRecord(u16 trackId, u16 characterId, const char *nickname, u32 raceTimeMs, u32 lapTimeMs, b32 raceBest, b32 lapBest)
{
    NativeLeaderboard_ClearPendingUpload();
    if (!s_nativeLeaderboard.initialized || (!raceBest && !lapBest) || (trackId >= NATIVE_LEADERBOARD_TRACK_COUNT)) return;
    struct NativeLeaderboardUpload *upload = &s_nativeLeaderboard.pendingUpload;
    upload->trackId = trackId;
    upload->characterId = characterId;
    upload->raceTimeMs = raceTimeMs;
    upload->lapTimeMs = lapTimeMs;
    upload->raceBest = raceBest;
    upload->lapBest = lapBest;
    snprintf(upload->nickname, sizeof(upload->nickname), "%s", nickname != NULL ? nickname : "");
    if (raceBest)
    {
        int size = NativeGhostInput_GetSerializedRecordingSize();
        if (size > 0)
        {
            upload->ghostData = malloc((size_t)size);
            if ((upload->ghostData != NULL) && NativeGhostInput_SerializeRecording(upload->ghostData, size))
            {
                upload->ghostSize = size;
            }
            else
            {
                free(upload->ghostData);
                upload->ghostData = NULL;
            }
        }

        if (upload->ghostData == NULL)
        {
            upload->raceBest = false;
        }
    }

    if (!upload->raceBest && !upload->lapBest)
    {
        NativeLeaderboard_ClearPendingUpload();
        return;
    }
    s_nativeLeaderboard.pendingUploadValid = true;
}

void NativeLeaderboard_StageRelicRaceRecord(u16 trackId, u16 characterId, const char *nickname, u32 relicTimeMs, b32 relicBest)
{
    NativeLeaderboard_ClearPendingUpload();
    if (!s_nativeLeaderboard.initialized || !relicBest || (trackId >= NATIVE_LEADERBOARD_TRACK_COUNT)) return;

    struct NativeLeaderboardUpload *upload = &s_nativeLeaderboard.pendingUpload;
    upload->trackId = trackId;
    upload->characterId = characterId;
    upload->relicTimeMs = relicTimeMs;
    upload->relicBest = true;
    snprintf(upload->nickname, sizeof(upload->nickname), "%s", nickname != NULL ? nickname : "");

    int size = NativeGhostInput_GetSerializedRecordingSize();
    if (size > 0)
    {
        upload->ghostData = malloc((size_t)size);
        if ((upload->ghostData != NULL) && NativeGhostInput_SerializeRecording(upload->ghostData, size))
        {
            upload->ghostSize = size;
        }
        else
        {
            free(upload->ghostData);
            upload->ghostData = NULL;
        }
    }

    if (upload->ghostData == NULL)
    {
        NativeLeaderboard_ClearPendingUpload();
        return;
    }

    s_nativeLeaderboard.pendingUploadValid = true;
}

void NativeLeaderboard_CommitPendingUpload(void)
{
    if (!s_nativeLeaderboard.pendingUploadValid) return;
    struct NativeLeaderboardJob job;
    memset(&job, 0, sizeof(job));
    job.type = NATIVE_LEADERBOARD_JOB_UPLOAD;
    job.upload = s_nativeLeaderboard.pendingUpload;
    memset(&s_nativeLeaderboard.pendingUpload, 0, sizeof(s_nativeLeaderboard.pendingUpload));
    s_nativeLeaderboard.pendingUploadValid = false;
    if (!NativeLeaderboard_Enqueue(&job)) NativeLeaderboard_FreeUpload(&job.upload);
}

#else

int NativeLeaderboard_Init(void) { return 1; }
void NativeLeaderboard_Shutdown(void) {}
int NativeLeaderboard_IsNetworkIdle(void) { return 1; }
int NativeLeaderboard_IsInternetConnected(void) { return 0; }
int NativeLeaderboard_RequestNetworkRestoreProbe(void) { return 0; }
int NativeLeaderboard_RequestRefresh(void) { return 0; }
int NativeLeaderboard_HasCache(void) { return 0; }
int NativeLeaderboard_IsRefreshing(void) { return 0; }
int NativeLeaderboard_CopyTrack(int trackId, struct NativeLeaderboardTrack *outTrack)
{
    (void)trackId;
    (void)outTrack;
    return 0;
}
int NativeLeaderboard_RequestGhost(u64 recordId)
{
    (void)recordId;
    return 0;
}
int NativeLeaderboard_GetGhostState(void) { return NATIVE_LEADERBOARD_TRANSFER_IDLE; }
int NativeLeaderboard_TakeGhost(void **data, int *size)
{
    if (data != NULL) *data = NULL;
    if (size != NULL) *size = 0;
    return 0;
}
void NativeLeaderboard_StageTimeTrialRecord(u16 trackId, u16 characterId, const char *nickname, u32 raceTimeMs, u32 lapTimeMs, b32 raceBest, b32 lapBest)
{
    (void)trackId;
    (void)characterId;
    (void)nickname;
    (void)raceTimeMs;
    (void)lapTimeMs;
    (void)raceBest;
    (void)lapBest;
}
void NativeLeaderboard_StageRelicRaceRecord(u16 trackId, u16 characterId, const char *nickname, u32 relicTimeMs, b32 relicBest)
{
    (void)trackId;
    (void)characterId;
    (void)nickname;
    (void)relicTimeMs;
    (void)relicBest;
}
void NativeLeaderboard_CommitPendingUpload(void) {}
void NativeLeaderboard_ClearPendingUpload(void) {}

#endif
