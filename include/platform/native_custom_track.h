#ifndef PLATFORM_NATIVE_CUSTOM_TRACK_H
#define PLATFORM_NATIVE_CUSTOM_TRACK_H

#define NATIVE_CUSTOM_TRACK_MAX 256

enum NativeCustomTrackFlags
{
	NATIVE_CUSTOM_TRACK_FLAG_BATTLE = 1u << 0,
	NATIVE_CUSTOM_TRACK_FLAG_NO_AI_PATHS = 1u << 1,
};

enum NativeCustomTrackAsset
{
	NATIVE_CUSTOM_TRACK_ASSET_VRM = 0,
	NATIVE_CUSTOM_TRACK_ASSET_LEV,
	NATIVE_CUSTOM_TRACK_ASSET_COUNT,
};

struct BigHeader;

int NativeCustomTrack_Scan(void);
int NativeCustomTrack_IsEnabled(void);
int NativeCustomTrack_GetCount(void);
const char *NativeCustomTrack_GetName(int index);
int NativeCustomTrack_GetLevelID(int index);
int NativeCustomTrack_HasAIPaths(int index);

void NativeCustomTrack_Select(int index);
int NativeCustomTrack_GetSelection(void);
int NativeCustomTrack_IsActive(void);
struct BigHeader *NativeCustomTrack_GetLevelBigHeader(int levelID);
int NativeCustomTrack_IsBigHeader(const struct BigHeader *bigfile);
int NativeCustomTrack_ReadAsset(const struct BigHeader *bigfile, int assetIndex, void *destination);

#endif
