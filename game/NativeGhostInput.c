#include <common.h>
#include <platform/native_memcard.h>

enum
{
    NATIVE_GHOST_INPUT_MAGIC = 0x3152474e,
    NATIVE_GHOST_INPUT_VERSION = 1,
    NATIVE_GHOST_INPUT_MAX_FRAMES = 32768,
    NATIVE_GHOST_INPUT_META_BUTTONS = BTN_START | BTN_SELECT,
    NATIVE_GHOST_INPUT_FLAG_60FPS = 1 << 0,
    NATIVE_GHOST_INPUT_FLAG_TIMING_METADATA = 1 << 15,
    NATIVE_GHOST_INPUT_TIMER_PHASE_MASK = 7,
};

struct NativeGhostInputHeader
{
    u32 magic;
    u16 version;
    u16 headerSize;
    u16 trackID;
    u16 characterID;
    u32 frameCount;
    u16 frameSize;
    u16 flags;
    u32 totalTimeMS;
    u32 reserved[2];
};

struct NativeGhostInputFrame
{
    u32 buttonsHeld;
    u8 stickLX;
    u8 stickLY;
    u16 elapsedTimeMS;
};

CTR_STATIC_ASSERT(sizeof(struct NativeGhostInputHeader) == 0x20);
CTR_STATIC_ASSERT(sizeof(struct NativeGhostInputFrame) == 0x8);

int gNativeGhostReplayMode = 0;
int gNativeGhostReplayFpsOverride = -1;

static struct NativeGhostInputFrame s_nativeGhostInputFrames[NATIVE_GHOST_INPUT_MAX_FRAMES];
static struct NativeGhostInputFrame s_nativeGhostInputPending;
static u32 s_nativeGhostInputFrameCount;
static u32 s_nativeGhostInputPlaybackIndex;
static u32 s_nativeGhostInputTotalTimeMS;
static u16 s_nativeGhostInputTrackID;
static u16 s_nativeGhostInputCharacterID;
static u16 s_nativeGhostInputRecordingFlags;
static u32 s_nativeGhostInputStartTimerPhase;
static u32 s_nativeGhostInputPlaybackTimerPhase;
static b32 s_nativeGhostInputPlaybackTimerPhasePending;
static b32 s_nativeGhostInputRecording;
static b32 s_nativeGhostInputRecordingInvalid;
static b32 s_nativeGhostInputPendingValid;
static b32 s_nativeGhostInputPlaybackActive;
static b32 s_nativeGhostInputExternalLoaded;
static char s_nativeGhostInputSelectedName[0x40];

static b32 NativeGhostInput_HeaderUses60Fps(const struct NativeGhostInputHeader *header)
{
    if ((header->flags & NATIVE_GHOST_INPUT_FLAG_TIMING_METADATA) == 0)
    {
        return false;
    }

    return (header->flags & NATIVE_GHOST_INPUT_FLAG_60FPS) != 0;
}


static b32 NativeGhostInput_ValidateHeader(const struct NativeGhostInputHeader *header)
{
    if ((header->magic != NATIVE_GHOST_INPUT_MAGIC) ||
        (header->version != NATIVE_GHOST_INPUT_VERSION) ||
        (header->headerSize != sizeof(struct NativeGhostInputHeader)) ||
        (header->frameSize != sizeof(struct NativeGhostInputFrame)) ||
        (header->frameCount == 0) ||
        (header->frameCount > NATIVE_GHOST_INPUT_MAX_FRAMES))
    {
        return false;
    }

    return true;
}

int NativeGhostInput_GetGhostFps(const char *ghostName)
{
    struct NativeGhostInputHeader header;

    if ((ghostName == NULL) || (ghostName[0] == '\0'))
    {
        return 0;
    }

    if (NativeMemcard_ReadReplayData(0, ghostName, &header, sizeof(header), 0) != NATIVE_MEMCARD_OK)
    {
        return 0;
    }

    if (!NativeGhostInput_ValidateHeader(&header))
    {
        return 0;
    }

    int expectedSize = header.headerSize + (header.frameCount * header.frameSize);
    if (NativeMemcard_ReplaySize(0, ghostName) < expectedSize)
    {
        return 0;
    }

    return NativeGhostInput_HeaderUses60Fps(&header) ? 60 : 30;
}

b32 NativeGhostInput_IsModernGhost(const char *ghostName)
{
    return NativeGhostInput_GetGhostFps(ghostName) != 0;
}

void NativeGhostInput_ClearSelection(void)
{
    s_nativeGhostInputSelectedName[0] = '\0';
    s_nativeGhostInputPlaybackActive = false;
    s_nativeGhostInputPlaybackIndex = 0;
    s_nativeGhostInputPlaybackTimerPhasePending = false;
    s_nativeGhostInputExternalLoaded = false;
    gNativeGhostReplayFpsOverride = -1;
}

b32 NativeGhostInput_SelectGhost(const char *ghostName, u16 trackID, u16 characterID)
{
    struct NativeGhostInputHeader header;

    NativeGhostInput_ClearSelection();

    if (NativeMemcard_ReadReplayData(0, ghostName, &header, sizeof(header), 0) != NATIVE_MEMCARD_OK)
    {
        return false;
    }

    if (!NativeGhostInput_ValidateHeader(&header) ||
        (header.trackID != trackID) ||
        (header.characterID != characterID))
    {
        return false;
    }

    snprintf(s_nativeGhostInputSelectedName, sizeof(s_nativeGhostInputSelectedName), "%s", ghostName);
    gNativeGhostReplayFpsOverride = NativeGhostInput_HeaderUses60Fps(&header) ? 1 : 0;
    return true;
}

void NativeGhostInput_StartRecording(void)
{
    struct GameTracker *gGT = sdata->gGT;
    struct Driver *driver = gGT->drivers[0];

    gNativeGhostReplayFpsOverride = -1;
    s_nativeGhostInputRecording = true;
    s_nativeGhostInputRecordingInvalid = false;
    s_nativeGhostInputPendingValid = false;
    s_nativeGhostInputPlaybackActive = false;
    s_nativeGhostInputFrameCount = 0;
    s_nativeGhostInputPlaybackIndex = 0;
    s_nativeGhostInputTotalTimeMS = 0;
    s_nativeGhostInputTrackID = gGT->levelID;
    s_nativeGhostInputCharacterID = data.characterIDs[driver->driverID];
    s_nativeGhostInputRecordingFlags = NATIVE_GHOST_INPUT_FLAG_TIMING_METADATA;
    if (CTR_NATIVE_60FPS_ACTIVE)
    {
        s_nativeGhostInputRecordingFlags |= NATIVE_GHOST_INPUT_FLAG_60FPS;
    }
    s_nativeGhostInputStartTimerPhase = 0;
}

void NativeGhostInput_DiscardRecording(void)
{
    s_nativeGhostInputRecording = false;
    s_nativeGhostInputRecordingInvalid = true;
    s_nativeGhostInputPendingValid = false;
    s_nativeGhostInputFrameCount = 0;
    s_nativeGhostInputTotalTimeMS = 0;
}

void NativeGhostInput_StopRecording(void)
{
    s_nativeGhostInputRecording = false;
    s_nativeGhostInputPendingValid = false;
}

static void NativeGhostInput_SetReplayPad(struct GamepadSystem *gGamepads)
{
    struct GamepadBuffer *pad = &gGamepads->gamepad[0];
    u32 physicalHeld = pad->buttonsHeldCurrFrame & NATIVE_GHOST_INPUT_META_BUTTONS;
    u32 physicalPrev = pad->buttonsHeldPrevFrame & NATIVE_GHOST_INPUT_META_BUTTONS;
    u32 replayHeld = 0;
    u32 replayPrev = 0;
    u8 stickLX = 0x80;
    u8 stickLY = 0x80;

    if (s_nativeGhostInputPlaybackActive && (s_nativeGhostInputPlaybackIndex < s_nativeGhostInputFrameCount))
    {
        struct NativeGhostInputFrame *frame = &s_nativeGhostInputFrames[s_nativeGhostInputPlaybackIndex];
        replayHeld = frame->buttonsHeld;
        stickLX = frame->stickLX;
        stickLY = frame->stickLY;

        if (s_nativeGhostInputPlaybackIndex != 0)
        {
            replayPrev = s_nativeGhostInputFrames[s_nativeGhostInputPlaybackIndex - 1].buttonsHeld;
        }
    }

    pad->buttonsHeldPrevFrame = replayPrev | physicalPrev;
    pad->buttonsHeldCurrFrame = replayHeld | physicalHeld;
    pad->buttonsTapped = ~pad->buttonsHeldPrevFrame & pad->buttonsHeldCurrFrame;
    pad->buttonsReleased = pad->buttonsHeldPrevFrame & ~pad->buttonsHeldCurrFrame;
    pad->stickLX = stickLX;
    pad->stickLY = stickLY;
    pad->stickLX_dontUse1 = stickLX;
    pad->stickLY_dontUse1 = stickLY;
    pad->stickRX = 0x80;
    pad->stickRY = 0x80;

    gGamepads->anyoneHeldCurr = pad->buttonsHeldCurrFrame;
    gGamepads->anyoneTapped = pad->buttonsTapped;
    gGamepads->anyoneReleased = pad->buttonsReleased;
    gGamepads->anyoneHeldPrev = pad->buttonsHeldPrevFrame;
}

void NativeGhostInput_ProcessGamepad(struct GamepadSystem *gGamepads)
{
    if ((gNativeGhostReplayMode != 0) && s_nativeGhostInputPlaybackActive)
    {
        if ((s_nativeGhostInputPlaybackIndex >= s_nativeGhostInputFrameCount) ||
            ((sdata->gGT->gameMode1 & END_OF_RACE) != 0))
        {
            s_nativeGhostInputPlaybackActive = false;
            return;
        }

        if ((sdata->gGT->gameMode1 & PAUSE_ALL) == 0)
        {
            NativeGhostInput_SetReplayPad(gGamepads);
        }
        return;
    }


    if (!s_nativeGhostInputRecording || (gGamepads->numGamepadsConnected <= 0))
    {
        return;
    }

    struct GamepadBuffer *pad = &gGamepads->gamepad[0];
    s_nativeGhostInputPending.buttonsHeld = pad->buttonsHeldCurrFrame & ~NATIVE_GHOST_INPUT_META_BUTTONS;
    s_nativeGhostInputPending.stickLX = (u8)pad->stickLX;
    s_nativeGhostInputPending.stickLY = (u8)pad->stickLY;
    s_nativeGhostInputPending.elapsedTimeMS = 0;
    s_nativeGhostInputPendingValid = true;
}

void NativeGhostInput_ProcessFrameTiming(s32 *elapsedTimeMS)
{
    if (gNativeGhostReplayMode != 0)
    {
        if (s_nativeGhostInputPlaybackActive && (s_nativeGhostInputPlaybackIndex < s_nativeGhostInputFrameCount))
        {
            if (s_nativeGhostInputPlaybackTimerPhasePending)
            {
                u32 timer = (u32)sdata->gGT->timer;
                timer = (timer & ~((u32)NATIVE_GHOST_INPUT_TIMER_PHASE_MASK)) | s_nativeGhostInputPlaybackTimerPhase;
                sdata->gGT->timer = (s32)timer;
                s_nativeGhostInputPlaybackTimerPhasePending = false;
            }

            *elapsedTimeMS = s_nativeGhostInputFrames[s_nativeGhostInputPlaybackIndex].elapsedTimeMS;
#if defined(__vita__)
            if (CTR_NATIVE_60FPS_ACTIVE)
            {
                s32 replayVBlanks = (*elapsedTimeMS + 15) / 16;
                if (replayVBlanks < 1)
                {
                    replayVBlanks = 1;
                }
                sdata->vsyncTillFlip = replayVBlanks;
            }
#endif
            s_nativeGhostInputPlaybackIndex++;
        }
        return;
    }

    if (!s_nativeGhostInputRecording || !s_nativeGhostInputPendingValid)
    {
        return;
    }

    if (((sdata->gGT->gameMode1 & DEBUG_MENU) != 0) ||
        ((sdata->gGT->gameMode1 & GAME_MODE_TIME_TRIAL_GAMEPLAY_MASK) != TIME_TRIAL))
    {
        s_nativeGhostInputRecordingInvalid = true;
        s_nativeGhostInputPendingValid = false;
        return;
    }

    if (s_nativeGhostInputFrameCount >= NATIVE_GHOST_INPUT_MAX_FRAMES)
    {
        s_nativeGhostInputRecordingInvalid = true;
        s_nativeGhostInputPendingValid = false;
        return;
    }

    if (s_nativeGhostInputFrameCount == 0)
    {
        s_nativeGhostInputStartTimerPhase = (u32)sdata->gGT->timer & NATIVE_GHOST_INPUT_TIMER_PHASE_MASK;
    }

    s_nativeGhostInputPending.elapsedTimeMS = (u16)*elapsedTimeMS;
    s_nativeGhostInputFrames[s_nativeGhostInputFrameCount++] = s_nativeGhostInputPending;
    s_nativeGhostInputTotalTimeMS += (u32)*elapsedTimeMS;
    s_nativeGhostInputPendingValid = false;
}

int NativeGhostInput_GetSerializedRecordingSize(void)
{
    if (s_nativeGhostInputRecordingInvalid || (s_nativeGhostInputFrameCount == 0))
    {
        return 0;
    }

    return sizeof(struct NativeGhostInputHeader) + (int)(s_nativeGhostInputFrameCount * sizeof(struct NativeGhostInputFrame));
}

b32 NativeGhostInput_SerializeRecording(void *dst, int dstSize)
{
    int required = NativeGhostInput_GetSerializedRecordingSize();
    if ((dst == NULL) || (required <= 0) || (dstSize < required))
    {
        return false;
    }

    struct NativeGhostInputHeader header;
    memset(&header, 0, sizeof(header));
    header.magic = NATIVE_GHOST_INPUT_MAGIC;
    header.version = NATIVE_GHOST_INPUT_VERSION;
    header.headerSize = sizeof(header);
    header.trackID = s_nativeGhostInputTrackID;
    header.characterID = s_nativeGhostInputCharacterID;
    header.frameCount = s_nativeGhostInputFrameCount;
    header.frameSize = sizeof(struct NativeGhostInputFrame);
    header.flags = s_nativeGhostInputRecordingFlags;
    header.totalTimeMS = s_nativeGhostInputTotalTimeMS;
    header.reserved[0] = s_nativeGhostInputStartTimerPhase;

    memcpy(dst, &header, sizeof(header));
    memcpy((u8 *)dst + sizeof(header), s_nativeGhostInputFrames, s_nativeGhostInputFrameCount * sizeof(struct NativeGhostInputFrame));
    return true;
}

b32 NativeGhostInput_LoadSerializedGhost(const void *src, int size, u16 expectedTrackID, u16 expectedCharacterID)
{
    if ((src == NULL) || (size < (int)sizeof(struct NativeGhostInputHeader)))
    {
        return false;
    }

    struct NativeGhostInputHeader header;
    memcpy(&header, src, sizeof(header));
    if (!NativeGhostInput_ValidateHeader(&header) ||
        (header.trackID != expectedTrackID) ||
        (header.characterID != expectedCharacterID))
    {
        return false;
    }

    int expectedSize = header.headerSize + (int)(header.frameCount * header.frameSize);
    if (size != expectedSize)
    {
        return false;
    }

    NativeGhostInput_ClearSelection();
    memcpy(s_nativeGhostInputFrames, (const u8 *)src + header.headerSize, header.frameCount * sizeof(struct NativeGhostInputFrame));
    s_nativeGhostInputFrameCount = header.frameCount;
    s_nativeGhostInputPlaybackIndex = 0;
    s_nativeGhostInputTotalTimeMS = header.totalTimeMS;
    s_nativeGhostInputTrackID = header.trackID;
    s_nativeGhostInputCharacterID = header.characterID;
    s_nativeGhostInputRecordingFlags = header.flags;
    s_nativeGhostInputStartTimerPhase = header.reserved[0] & NATIVE_GHOST_INPUT_TIMER_PHASE_MASK;
    s_nativeGhostInputExternalLoaded = true;
    gNativeGhostReplayFpsOverride = NativeGhostInput_HeaderUses60Fps(&header) ? 1 : 0;
    return true;
}

b32 NativeGhostInput_SaveRecordingForGhost(const char *ghostName)
{
    struct NativeGhostInputHeader header;

    if (s_nativeGhostInputRecordingInvalid || (s_nativeGhostInputFrameCount == 0))
    {
        NativeMemcard_RemoveReplay(0, ghostName);
        return false;
    }

    memset(&header, 0, sizeof(header));
    header.magic = NATIVE_GHOST_INPUT_MAGIC;
    header.version = NATIVE_GHOST_INPUT_VERSION;
    header.headerSize = sizeof(header);
    header.trackID = s_nativeGhostInputTrackID;
    header.characterID = s_nativeGhostInputCharacterID;
    header.frameCount = s_nativeGhostInputFrameCount;
    header.frameSize = sizeof(struct NativeGhostInputFrame);
    header.flags = s_nativeGhostInputRecordingFlags;
    header.totalTimeMS = s_nativeGhostInputTotalTimeMS;
    header.reserved[0] = s_nativeGhostInputStartTimerPhase;

    return NativeMemcard_WriteReplayData(0, ghostName, &header, sizeof(header), s_nativeGhostInputFrames,
                                         s_nativeGhostInputFrameCount * sizeof(struct NativeGhostInputFrame)) == NATIVE_MEMCARD_OK;
}

void NativeGhostInput_RemoveForGhost(const char *ghostName)
{
    NativeMemcard_RemoveReplay(0, ghostName);
}

b32 NativeGhostInput_BeginPlayback(void)
{
    struct NativeGhostInputHeader header;
    int expectedSize;
    int actualSize;

    s_nativeGhostInputPlaybackActive = false;
    s_nativeGhostInputPlaybackIndex = 0;

    if (s_nativeGhostInputExternalLoaded)
    {
        if ((s_nativeGhostInputTrackID != sdata->gGT->levelID) || (s_nativeGhostInputCharacterID != data.characterIDs[0]))
        {
            return false;
        }

        b32 use60Fps = (s_nativeGhostInputRecordingFlags & NATIVE_GHOST_INPUT_FLAG_TIMING_METADATA) &&
                       (s_nativeGhostInputRecordingFlags & NATIVE_GHOST_INPUT_FLAG_60FPS);
        gNativeGhostReplayFpsOverride = use60Fps ? 1 : 0;
        s_nativeGhostInputPlaybackTimerPhase = s_nativeGhostInputStartTimerPhase & NATIVE_GHOST_INPUT_TIMER_PHASE_MASK;
        s_nativeGhostInputPlaybackTimerPhasePending = use60Fps;
        s_nativeGhostInputPlaybackActive = true;
        return true;
    }

    if (s_nativeGhostInputSelectedName[0] == '\0')
    {
        return false;
    }

    if (NativeMemcard_ReadReplayData(0, s_nativeGhostInputSelectedName, &header, sizeof(header), 0) != NATIVE_MEMCARD_OK)
    {
        return false;
    }

    if (!NativeGhostInput_ValidateHeader(&header) ||
        (header.trackID != sdata->gGT->levelID) ||
        (header.characterID != data.characterIDs[0]))
    {
        return false;
    }

    expectedSize = header.headerSize + (header.frameCount * header.frameSize);
    actualSize = NativeMemcard_ReplaySize(0, s_nativeGhostInputSelectedName);
    if (actualSize < expectedSize)
    {
        return false;
    }

    if (NativeMemcard_ReadReplayData(0, s_nativeGhostInputSelectedName, s_nativeGhostInputFrames,
                                     header.frameCount * sizeof(struct NativeGhostInputFrame), header.headerSize) != NATIVE_MEMCARD_OK)
    {
        return false;
    }

    s_nativeGhostInputFrameCount = header.frameCount;
    s_nativeGhostInputTotalTimeMS = header.totalTimeMS;
    b32 use60Fps = NativeGhostInput_HeaderUses60Fps(&header);
    gNativeGhostReplayFpsOverride = use60Fps ? 1 : 0;
    if (use60Fps && ((header.flags & NATIVE_GHOST_INPUT_FLAG_TIMING_METADATA) != 0))
    {
        s_nativeGhostInputPlaybackTimerPhase = header.reserved[0] & NATIVE_GHOST_INPUT_TIMER_PHASE_MASK;
        s_nativeGhostInputPlaybackTimerPhasePending = true;
    }
    else
    {
        s_nativeGhostInputPlaybackTimerPhasePending = false;
    }
    s_nativeGhostInputPlaybackActive = true;
    return true;
}

