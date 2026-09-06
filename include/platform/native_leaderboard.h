#ifndef CTR_NATIVE_LEADERBOARD_H
#define CTR_NATIVE_LEADERBOARD_H

#include <common.h>

enum
{
    NATIVE_LEADERBOARD_TRACK_COUNT = 18,
    NATIVE_LEADERBOARD_RACE_COUNT = 5,
    NATIVE_LEADERBOARD_NICKNAME_SIZE = 18,
    NATIVE_LEADERBOARD_PUBLIC_USER_ID_SIZE = 24,
    NATIVE_LEADERBOARD_CACHE_TTL_MS = 120000,
};

enum NativeLeaderboardTransferState
{
    NATIVE_LEADERBOARD_TRANSFER_IDLE = 0,
    NATIVE_LEADERBOARD_TRANSFER_PENDING,
    NATIVE_LEADERBOARD_TRANSFER_READY,
    NATIVE_LEADERBOARD_TRANSFER_FAILED,
};

struct NativeLeaderboardEntry
{
    u64 recordId;
    char userId[NATIVE_LEADERBOARD_PUBLIC_USER_ID_SIZE];
    char nickname[NATIVE_LEADERBOARD_NICKNAME_SIZE];
    u16 characterId;
    u32 timeMs;
    b32 hasGhost;
};

struct NativeLeaderboardTrack
{
    struct NativeLeaderboardEntry race[NATIVE_LEADERBOARD_RACE_COUNT];
    int raceCount;
    struct NativeLeaderboardEntry lap;
    b32 hasLap;
};

int NativeLeaderboard_Init(void);
void NativeLeaderboard_Shutdown(void);
int NativeLeaderboard_IsNetworkIdle(void);
int NativeLeaderboard_IsInternetConnected(void);
int NativeLeaderboard_RequestNetworkRestoreProbe(void);
int NativeLeaderboard_RequestRefresh(void);
int NativeLeaderboard_HasCache(void);
int NativeLeaderboard_IsRefreshing(void);
int NativeLeaderboard_CopyTrack(int trackId, struct NativeLeaderboardTrack *outTrack);
int NativeLeaderboard_RequestGhost(u64 recordId);
int NativeLeaderboard_GetGhostState(void);
int NativeLeaderboard_TakeGhost(void **data, int *size);
void NativeLeaderboard_StageTimeTrialRecord(u16 trackId, u16 characterId, const char *nickname, u32 raceTimeMs, u32 lapTimeMs, b32 raceBest, b32 lapBest);
void NativeLeaderboard_CommitPendingUpload(void);
void NativeLeaderboard_ClearPendingUpload(void);

#endif
