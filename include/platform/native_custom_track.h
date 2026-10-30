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
	NATIVE_CUSTOM_TRACK_ASSET_MAP,
	NATIVE_CUSTOM_TRACK_ASSET_COUNT,
};

struct BigHeader;
struct PrimMem;
struct UIMap;

int NativeCustomTrack_Scan(void);
int NativeCustomTrack_IsEnabled(void);
int NativeCustomTrack_GetCount(void);
const char *NativeCustomTrack_GetName(int index);
const char *NativeCustomTrack_GetAuthor(int index);
int NativeCustomTrack_GetLevelID(int index);
int NativeCustomTrack_IsBattle(int index);
int NativeCustomTrack_HasAIPaths(int index);

// Track select picture (<package>.png next to the package), 0 when it has none.
u32 NativeCustomTrack_GetPreviewTexture(int index, int *width, int *height);

// Minimap: white track shape, drawn like the stock maps (UI_Map_DrawMap colour
// modes) with its bottom right corner at posX, posY. wideMap: the UIMap the
// race HUD places icons with (widescreen scales the map around its origin),
// NULL on menus.
int NativeCustomTrack_HasMap(int index, int *width, int *height);
void NativeCustomTrack_DrawMap(int index, int posX, int posY, struct PrimMem *primMem, u32 *ot, u32 colorID, struct UIMap *wideMap);
// UIMap placing the race HUD's icons on the active track's minimap drawn at
// posX, posY, or NULL.
struct UIMap *NativeCustomTrack_GetRaceMap(int posX, int posY);

void NativeCustomTrack_Select(int index);
int NativeCustomTrack_GetSelection(void);
int NativeCustomTrack_IsActive(void);
struct BigHeader *NativeCustomTrack_GetLevelBigHeader(int levelID);
int NativeCustomTrack_IsBigHeader(const struct BigHeader *bigfile);
int NativeCustomTrack_ReadAsset(const struct BigHeader *bigfile, int assetIndex, void *destination);

// --capture-track-previews: boots into every custom track without a picture,
// takes one (no karts, HUD or particles) and quits.
void NativeCustomTrack_Capture_Init(int argc, char *argv[]);
int NativeCustomTrack_Capture_BootLevel(int defaultLevel);
int NativeCustomTrack_Capture_Active(void);
void NativeCustomTrack_Capture_Tick(void);
int NativeCustomTrack_Capture_WantsFrame(void);
void NativeCustomTrack_Capture_OnFrame(const u8 *rgba, int width, int height);

#endif
