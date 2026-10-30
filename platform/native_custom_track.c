#include <common.h>
#include <platform/native_assets.h>
#include <platform/native_custom_track.h>
#include <platform/native_path.h>
#include <platform/native_renderer.h>

#if defined(__vita__)
#include <platform/native_adhoc.h>
#endif

#if defined(_WIN32)
#include <platform/native_win32.h>
#else
#include <dirent.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NATIVE_CUSTOM_TRACK_MAGIC 0x54525443u
#define NATIVE_CUSTOM_TRACK_VERSION 1u
#define NATIVE_CUSTOM_TRACK_PATH_MAX 1024
#define NATIVE_CUSTOM_TRACK_DIR "mods/customtracks"

struct NativeCustomTrackDiskAsset
{
	u32 offset;
	u32 size;
};

struct NativeCustomTrackDiskHeader
{
	u32 magic;
	u16 version;
	u16 headerSize;
	s16 levelID;
	u16 flags;
	char name[64];
	char author[64];
	// world range the minimap image spans (x, z) and its size in HUD pixels
	s16 mapWorldX0;
	s16 mapWorldX1;
	s16 mapWorldZ0;
	s16 mapWorldZ1;
	s16 mapWidth;
	s16 mapHeight;
	struct NativeCustomTrackDiskAsset assets[NATIVE_CUSTOM_TRACK_ASSET_COUNT];
};

struct NativeCustomTrackBigHeader
{
	struct BigHeader header;
	struct BigEntry entries[NATIVE_CUSTOM_TRACK_ASSET_COUNT];
};

struct NativeCustomTrackEntry
{
	char path[NATIVE_CUSTOM_TRACK_PATH_MAX];
	char previewPath[NATIVE_CUSTOM_TRACK_PATH_MAX];
	struct NativeCustomTrackDiskHeader disk;
	struct NativeCustomTrackBigHeader bigfile;
	b32 previewTried;
	u32 previewTexture;
	int previewWidth;
	int previewHeight;
	b32 mapTried;
	u32 mapTexture;
};

struct NativeCustomTrackTextureTask
{
	u32 texture;
	int width;
	int height;
	const u8 *pixels;
};

CTR_STATIC_ASSERT(sizeof(struct NativeCustomTrackDiskHeader) == 0xb0);

global_variable struct NativeCustomTrackEntry s_nativeCustomTracks[NATIVE_CUSTOM_TRACK_MAX];
global_variable int s_nativeCustomTrackCount;
global_variable int s_nativeCustomTrackSelection = -1;
global_variable struct UIMapSpawnMetadata s_nativeCustomTrackRaceMap;

internal int NativeCustomTrack_HasExtension(const char *name, const char *extension)
{
	size_t nameLen = strlen(name);
	size_t extensionLen = strlen(extension);
	if (nameLen < extensionLen)
		return 0;

	for (size_t i = 0; i < extensionLen; i++)
	{
		char a = name[nameLen - extensionLen + i];
		char b = extension[i];
		if ((a >= 'A') && (a <= 'Z')) a = (char)(a + ('a' - 'A'));
		if ((b >= 'A') && (b <= 'Z')) b = (char)(b + ('a' - 'A'));
		if (a != b) return 0;
	}

	return 1;
}

internal int NativeCustomTrack_ReadHeader(const char *path, struct NativeCustomTrackEntry *track)
{
	FILE *file = fopen(path, "rb");
	long fileSize;
	if (file == NULL)
		return 0;

	memset(track, 0, sizeof(*track));
	if ((fseek(file, 0, SEEK_END) != 0) || ((fileSize = ftell(file)) < (long)sizeof(track->disk)) ||
	    (fseek(file, 0, SEEK_SET) != 0) || (fread(&track->disk, 1, sizeof(track->disk), file) != sizeof(track->disk)))
	{
		fclose(file);
		return 0;
	}
	fclose(file);

	if ((track->disk.magic != NATIVE_CUSTOM_TRACK_MAGIC) || (track->disk.version != NATIVE_CUSTOM_TRACK_VERSION) ||
	    (track->disk.headerSize != sizeof(track->disk)) || (track->disk.levelID < 0) || (track->disk.levelID > LAB_BASEMENT))
	{
		return 0;
	}

	for (int i = 0; i < NATIVE_CUSTOM_TRACK_ASSET_COUNT; i++)
	{
		u64 offset = track->disk.assets[i].offset;
		u64 size = track->disk.assets[i].size;
		b32 optional = (i == NATIVE_CUSTOM_TRACK_ASSET_MAP);
		if ((size == 0) ? !optional : ((offset < sizeof(track->disk)) || (offset + size > (u64)fileSize)))
			return 0;
	}
	// (the map is drawn with 8 bit texture coordinates)
	if ((track->disk.mapWidth <= 0) || (track->disk.mapWidth > 255) || (track->disk.mapHeight <= 1) || (track->disk.mapHeight > 255) ||
	    (track->disk.mapWorldX1 == track->disk.mapWorldX0) || (track->disk.mapWorldZ1 == track->disk.mapWorldZ0))
	{
		track->disk.assets[NATIVE_CUSTOM_TRACK_ASSET_MAP].size = 0;
	}

	track->disk.name[sizeof(track->disk.name) - 1] = '\0';
	track->disk.author[sizeof(track->disk.author) - 1] = '\0';
	size_t pathLen = strlen(path);
	if ((track->disk.name[0] == '\0') || (pathLen >= sizeof(track->path)))
		return 0;
	strcpy(track->path, path);
	// picture: the package's name with .png
	strcpy(track->previewPath, path);
	strcpy(track->previewPath + pathLen - strlen(".ctrt"), ".png");

	track->bigfile.header.cdpos = -1;
	track->bigfile.header.numEntry = NATIVE_CUSTOM_TRACK_ASSET_COUNT;
	for (int i = 0; i < NATIVE_CUSTOM_TRACK_ASSET_COUNT; i++)
	{
		track->bigfile.entries[i].offset = (int)track->disk.assets[i].offset;
		track->bigfile.entries[i].size = (int)track->disk.assets[i].size;
	}
	return 1;
}

internal int NativeCustomTrack_Compare(const void *a, const void *b)
{
	const struct NativeCustomTrackEntry *ta = a;
	const struct NativeCustomTrackEntry *tb = b;
	return strcmp(ta->disk.name, tb->disk.name);
}

internal void NativeCustomTrack_AddPath(const char *path)
{
	if (s_nativeCustomTrackCount >= NATIVE_CUSTOM_TRACK_MAX)
		return;
	if (!NativeCustomTrack_ReadHeader(path, &s_nativeCustomTracks[s_nativeCustomTrackCount]))
	{
		fprintf(stderr, "[CTR Native] Ignoring invalid custom track package: %s\n", path);
		return;
	}
	s_nativeCustomTrackCount++;
}

int NativeCustomTrack_Scan(void)
{
	char directory[NATIVE_CUSTOM_TRACK_PATH_MAX];
	s_nativeCustomTrackCount = 0;
	s_nativeCustomTrackSelection = -1;

	if (!NativePath_Join(directory, sizeof(directory), NativeStr8_FromCString(NativeAssets_GetBaseDir()), NATIVE_STR8_LIT(NATIVE_CUSTOM_TRACK_DIR)))
		return 0;

#if defined(_WIN32)
	{
		char searchPath[NATIVE_CUSTOM_TRACK_PATH_MAX];
		WIN32_FIND_DATAA findData;
		HANDLE findHandle;

		if (!NativePath_Join(searchPath, sizeof(searchPath), NativeStr8_FromCString(directory), NATIVE_STR8_LIT("*")))
			return 0;
		findHandle = FindFirstFileA(searchPath, &findData);
		if (findHandle == INVALID_HANDLE_VALUE)
			return 0;

		do
		{
			char path[NATIVE_CUSTOM_TRACK_PATH_MAX];
			if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
				continue;
			if (!NativeCustomTrack_HasExtension(findData.cFileName, ".ctrt"))
				continue;
			if (NativePath_Join(path, sizeof(path), NativeStr8_FromCString(directory), NativeStr8_FromCString(findData.cFileName)))
				NativeCustomTrack_AddPath(path);
		} while (FindNextFileA(findHandle, &findData));

		FindClose(findHandle);
	}
#else
	{
		DIR *dir = opendir(directory);
		struct dirent *entry;
		if (dir == NULL)
			return 0;

		while ((entry = readdir(dir)) != NULL)
		{
			char path[NATIVE_CUSTOM_TRACK_PATH_MAX];
			if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0))
				continue;
			if (!NativeCustomTrack_HasExtension(entry->d_name, ".ctrt"))
				continue;
			if (NativePath_Join(path, sizeof(path), NativeStr8_FromCString(directory), NativeStr8_FromCString(entry->d_name)))
				NativeCustomTrack_AddPath(path);
		}
		closedir(dir);
	}
#endif

	if (s_nativeCustomTrackCount > 1)
		qsort(s_nativeCustomTracks, (size_t)s_nativeCustomTrackCount, sizeof(s_nativeCustomTracks[0]), NativeCustomTrack_Compare);

	if (s_nativeCustomTrackCount != 0)
		printf("[CTR Native] Found %d custom track package(s) in %s\n", s_nativeCustomTrackCount, directory);

	return s_nativeCustomTrackCount;
}

internal struct NativeCustomTrackEntry *NativeCustomTrack_Get(int index)
{
	return ((index >= 0) && (index < s_nativeCustomTrackCount)) ? &s_nativeCustomTracks[index] : NULL;
}

int NativeCustomTrack_IsEnabled(void)
{
#if defined(__vita__)
	if (NativeAdhoc_IsActive())
		return 0;
#endif
	return s_nativeCustomTrackCount > 0;
}

int NativeCustomTrack_GetCount(void)
{
	return s_nativeCustomTrackCount;
}

const char *NativeCustomTrack_GetName(int index)
{
	struct NativeCustomTrackEntry *track = NativeCustomTrack_Get(index);
	return (track != NULL) ? track->disk.name : "";
}

const char *NativeCustomTrack_GetAuthor(int index)
{
	struct NativeCustomTrackEntry *track = NativeCustomTrack_Get(index);
	return (track != NULL) ? track->disk.author : "";
}

int NativeCustomTrack_GetLevelID(int index)
{
	struct NativeCustomTrackEntry *track = NativeCustomTrack_Get(index);
	return (track != NULL) ? track->disk.levelID : -1;
}

int NativeCustomTrack_IsBattle(int index)
{
	struct NativeCustomTrackEntry *track = NativeCustomTrack_Get(index);
	return (track != NULL) && ((track->disk.flags & NATIVE_CUSTOM_TRACK_FLAG_BATTLE) != 0);
}

int NativeCustomTrack_HasAIPaths(int index)
{
	struct NativeCustomTrackEntry *track = NativeCustomTrack_Get(index);
	return (track != NULL) && ((track->disk.flags & NATIVE_CUSTOM_TRACK_FLAG_NO_AI_PATHS) == 0);
}

internal void NativeCustomTrack_BackendCreateTexture(void *arg)
{
	struct NativeCustomTrackTextureTask *task = (struct NativeCustomTrackTextureTask *)arg;
	task->texture = NativeRenderer_CreateStreamingTexture(task->width, task->height);
	if (task->texture != 0)
		NativeRenderer_UpdateStreamingTexture(task->texture, task->width, task->height, task->pixels);
}

// A texture from PNG bytes; 0 if they don't decode.
internal u32 NativeCustomTrack_CreateTextureFromPNG(const void *data, int size, int *width, int *height)
{
	u8 *pixels = NativeRenderer_DecodePNG(data, size, width, height);
	if (pixels == NULL)
		return 0;

	struct NativeCustomTrackTextureTask task = {0, *width, *height, pixels};
#if defined(__vita__)
	NativeGpu_RunBackendTaskSync(NativeCustomTrack_BackendCreateTexture, &task);
#else
	NativeCustomTrack_BackendCreateTexture(&task);
#endif
	NativeRenderer_FreeImage(pixels);
	return task.texture;
}

// size 0: the whole file
internal void *NativeCustomTrack_ReadFile(const char *path, u32 offset, u32 size, int *sizeOut)
{
	FILE *file = fopen(path, "rb");
	if (file == NULL)
		return NULL;
	if (size == 0)
	{
		long fileSize = (fseek(file, 0, SEEK_END) == 0) ? ftell(file) : -1;
		size = (fileSize > 0) ? (u32)fileSize : 0;
	}
	void *data = (size != 0) ? malloc(size) : NULL;
	if ((data == NULL) || (fseek(file, (long)offset, SEEK_SET) != 0) || (fread(data, 1, size, file) != size))
	{
		free(data);
		data = NULL;
	}
	fclose(file);
	*sizeOut = (int)size;
	return data;
}

u32 NativeCustomTrack_GetPreviewTexture(int index, int *width, int *height)
{
	struct NativeCustomTrackEntry *track = NativeCustomTrack_Get(index);
	if (track == NULL)
		return 0;

	// (looked for once a session: none stays none)
	if (!track->previewTried)
	{
		track->previewTried = true;
		int size = 0;
		void *data = NativeCustomTrack_ReadFile(track->previewPath, 0, 0, &size);
		if (data != NULL)
		{
			track->previewTexture = NativeCustomTrack_CreateTextureFromPNG(data, size, &track->previewWidth, &track->previewHeight);
			free(data);
		}
	}

	*width = track->previewWidth;
	*height = track->previewHeight;
	return track->previewTexture;
}

internal u32 NativeCustomTrack_GetMapTexture(struct NativeCustomTrackEntry *track)
{
	if (!track->mapTried)
	{
		track->mapTried = true;
		const struct NativeCustomTrackDiskAsset *asset = &track->disk.assets[NATIVE_CUSTOM_TRACK_ASSET_MAP];
		int size = 0;
		void *data = (asset->size != 0) ? NativeCustomTrack_ReadFile(track->path, asset->offset, asset->size, &size) : NULL;
		if (data != NULL)
		{
			int width = 0;
			int height = 0;
			track->mapTexture = NativeCustomTrack_CreateTextureFromPNG(data, size, &width, &height);
			free(data);
		}
	}
	return track->mapTexture;
}

int NativeCustomTrack_HasMap(int index, int *width, int *height)
{
	struct NativeCustomTrackEntry *track = NativeCustomTrack_Get(index);
	if ((track == NULL) || (track->disk.assets[NATIVE_CUSTOM_TRACK_ASSET_MAP].size == 0))
		return 0;
	*width = track->disk.mapWidth;
	*height = track->disk.mapHeight;
	return 1;
}

internal void NativeCustomTrack_LinkPrim(u32 *ot, void *prim, u32 words)
{
	u32 *tag = (u32 *)prim;
	*tag = CtrGpu_PackOTTag(*ot, words << 24);
	*ot = (u32)CtrGpu_PrimToOTLink24(prim);
}

void NativeCustomTrack_DrawMap(int index, int posX, int posY, struct PrimMem *primMem, u32 *ot, u32 colorID, struct UIMap *wideMap)
{
	struct NativeCustomTrackEntry *track = NativeCustomTrack_Get(index);
	int width;
	int height;
	if (!NativeCustomTrack_HasMap(index, &width, &height))
		return;
	u32 texture = NativeCustomTrack_GetMapTexture(track);
	if (texture == 0)
		return;

	// colours and blending as UI_Map_DrawMap draws the stock maps
	u32 color = UI_MAP_NEUTRAL_COLOR;
	u32 transparency = colorID;
	if (colorID == UI_MAP_COLOR_MODE_BLACK)
	{
		color = 0;
		transparency = 0;
	}
	if (colorID == UI_MAP_COLOR_MODE_BLUE)
	{
		color = UI_MAP_BLUE_OUTLINE_COLOR;
		transparency = 0;
	}

	int leftX = posX - width;
	int rightX = posX;
#if CTR_NATIVE_WIDESCREEN
	// (as UI_Map_DrawMap_ExtraFunc: scaled round the map's world origin, where
	// the race HUD's icons are scaled round)
	if (wideMap != NULL)
	{
		int centerX = 0;
		int centerY = 0;
		UI_Map_GetIconPos(wideMap, &centerX, &centerY);
		leftX = centerX + CTR_WIDESCREEN_SCALE_X(leftX - centerX);
		rightX = centerX + CTR_WIDESCREEN_SCALE_X(rightX - centerX);
	}
#else
	(void)wideMap;
#endif

	// linked in reverse: drawn as texture, polygon, texture reset
	DR_PSYX_TEX *resetTexture = (DR_PSYX_TEX *)primMem->cursor;
	POLY_FT4 *p = (POLY_FT4 *)(resetTexture + 1);
	DR_PSYX_TEX *setTexture = (DR_PSYX_TEX *)(p + 1);
	primMem->cursor = setTexture + 1;

	SetPsyXTexture(resetTexture, 0, 0, 0);
	NativeCustomTrack_LinkPrim(ot, resetTexture, 2);

	// (the colour word's top byte is the primitive code: colour first)
	CtrGpu_WriteColorCode(&p->r0, color);
	setPolyFT4(p);
	p->code |= UI_MAP_SEMI_TRANS_CODE_BIT;
	p->x0 = (s16)leftX;
	p->y0 = (s16)(posY - height);
	p->x1 = (s16)rightX;
	p->y1 = (s16)(posY - height);
	p->x2 = (s16)leftX;
	p->y2 = (s16)posY;
	p->x3 = (s16)rightX;
	p->y3 = (s16)posY;
	p->u0 = 0;
	p->v0 = 0;
	p->u1 = (u8)width;
	p->v1 = 0;
	p->u2 = 0;
	p->v2 = (u8)height;
	p->u3 = (u8)width;
	p->v3 = (u8)height;
	p->clut = 0;
	p->tpage = getTPage(TEXPAGE_COLOR_15BIT, transparency, 0, 0);
	NativeCustomTrack_LinkPrim(ot, p, 9);

	SetPsyXTexture(setTexture, texture, width, height);
	NativeCustomTrack_LinkPrim(ot, setTexture, 2);
}

struct UIMap *NativeCustomTrack_GetRaceMap(int posX, int posY)
{
	int width;
	int height;
	if (!NativeCustomTrack_IsActive() || !NativeCustomTrack_HasMap(s_nativeCustomTrackSelection, &width, &height) ||
	    (NativeCustomTrack_GetMapTexture(&s_nativeCustomTracks[s_nativeCustomTrackSelection]) == 0))
	{
		return NULL;
	}

	// UI_Map_GetIconPos, mode 0: screen x from world x, screen y from world z
	// (north up), its origin at world 0
	const struct NativeCustomTrackDiskHeader *disk = &s_nativeCustomTracks[s_nativeCustomTrackSelection].disk;
	struct UIMap *map = &s_nativeCustomTrackRaceMap.map;
	int left = posX - width;
	int top = posY - height;
	map->worldStartX = disk->mapWorldX0;
	map->worldEndX = disk->mapWorldX1;
	map->worldStartY = disk->mapWorldZ1; // (z decreasing down the image)
	map->worldEndY = disk->mapWorldZ0;
	map->iconSizeX = (s16)width;
	map->iconSizeY = (s16)(height / 2); // (GetIconPos doubles it)
	map->iconStartX = (s16)(left - (s32)disk->mapWorldX0 * width / (disk->mapWorldX1 - disk->mapWorldX0));
	map->iconStartY = (s16)(top + (s32)disk->mapWorldZ1 * height / (disk->mapWorldZ1 - disk->mapWorldZ0) + UI_MAP_ICON_Y_OFFSET);
	map->mode = UI_MAP_MODE_0_DEGREES;
	s_nativeCustomTrackRaceMap.topHalfMode = 0;
	return map;
}

void NativeCustomTrack_Select(int index)
{
	s_nativeCustomTrackSelection = (NativeCustomTrack_Get(index) != NULL) ? index : -1;
}

int NativeCustomTrack_GetSelection(void)
{
	return s_nativeCustomTrackSelection;
}

// The selection replaces its host level only in the modes the track list
// offers it (never in Adventure, cups or the attract demo).
int NativeCustomTrack_IsActive(void)
{
	struct GameTracker *gGT = sdata->gGT;
	if ((s_nativeCustomTrackSelection < 0) || (gGT == NULL) || (gGT->boolDemoMode != 0))
		return 0;
	if (((gGT->gameMode1 & (ADVENTURE_MODE | ADVENTURE_ARENA)) != 0) || ((gGT->gameMode2 & CUP_ANY_KIND) != 0))
		return 0;
	return s_nativeCustomTracks[s_nativeCustomTrackSelection].disk.levelID == (s16)gGT->levelID;
}

struct BigHeader *NativeCustomTrack_GetLevelBigHeader(int levelID)
{
	if (!NativeCustomTrack_IsActive() || (s_nativeCustomTracks[s_nativeCustomTrackSelection].disk.levelID != levelID))
		return NULL;
	return &s_nativeCustomTracks[s_nativeCustomTrackSelection].bigfile.header;
}

internal struct NativeCustomTrackEntry *NativeCustomTrack_FindByBigHeader(const struct BigHeader *bigfile)
{
	if (bigfile == NULL)
		return NULL;
	for (int i = 0; i < s_nativeCustomTrackCount; i++)
	{
		if (bigfile == &s_nativeCustomTracks[i].bigfile.header)
			return &s_nativeCustomTracks[i];
	}
	return NULL;
}

int NativeCustomTrack_IsBigHeader(const struct BigHeader *bigfile)
{
	return NativeCustomTrack_FindByBigHeader(bigfile) != NULL;
}

int NativeCustomTrack_ReadAsset(const struct BigHeader *bigfile, int assetIndex, void *destination)
{
	struct NativeCustomTrackEntry *track = NativeCustomTrack_FindByBigHeader(bigfile);
	if ((track == NULL) || (assetIndex < 0) || (assetIndex >= NATIVE_CUSTOM_TRACK_ASSET_COUNT) || (destination == NULL))
		return 0;

	const u32 offset = track->disk.assets[assetIndex].offset;
	const u32 size = track->disk.assets[assetIndex].size;
	FILE *file = fopen(track->path, "rb");
	if (file == NULL)
		return 0;
	const int ok = (fseek(file, (long)offset, SEEK_SET) == 0) && (fread(destination, 1, size, file) == size);
	fclose(file);

	printf("[CTR Native] Loading custom track %s asset=%d size=%u\n", track->disk.name, assetIndex, size);
	return ok;
}

// ---- --capture-track-previews ----------------------------------------------
// Each custom track without a picture is loaded in turn (one player, no bots);
// once the countdown is over and its traffic lights have left, the presented frame
// is cropped to the track select preview's shape and saved next to the package.

#define NATIVE_CUSTOM_TRACK_CAPTURE_SETTLE_FRAMES 90
#define NATIVE_CUSTOM_TRACK_CAPTURE_GIVE_UP_FRAMES 1800
#define NATIVE_CUSTOM_TRACK_PREVIEW_WIDTH 512
#define NATIVE_CUSTOM_TRACK_PREVIEW_HEIGHT 214 // (the preview frame's 170 x 71)

global_variable struct
{
	b32 active;
	s16 queue[NATIVE_CUSTOM_TRACK_MAX];
	int count;
	int current;
	int frames;
	int settled;
	b32 wantFrame;
} s_nativeCustomTrackCapture;

void NativeCustomTrack_Capture_Init(int argc, char *argv[])
{
	b32 requested = false;
	for (int i = 1; i < argc; i++)
	{
		requested |= (strcmp(argv[i], "--capture-track-previews") == 0);
	}
	if (!requested)
		return;

#if defined(__vita__) || defined(__EMSCRIPTEN__)
	fprintf(stderr, "[CTR Native] --capture-track-previews is not supported on this platform\n");
	exit(1);
#else
	for (int i = 0; i < s_nativeCustomTrackCount; i++)
	{
		FILE *file = fopen(s_nativeCustomTracks[i].previewPath, "rb");
		if (file != NULL)
		{
			fclose(file);
			continue;
		}
		s_nativeCustomTrackCapture.queue[s_nativeCustomTrackCapture.count++] = (s16)i;
	}
	printf("[CTR Native] Capturing %d custom track picture(s)\n", s_nativeCustomTrackCapture.count);
	if (s_nativeCustomTrackCapture.count == 0)
		exit(0);
	s_nativeCustomTrackCapture.active = true;
#endif
}

int NativeCustomTrack_Capture_Active(void)
{
	return s_nativeCustomTrackCapture.active;
}

internal int NativeCustomTrack_Capture_SelectCurrent(void)
{
	int index = s_nativeCustomTrackCapture.queue[s_nativeCustomTrackCapture.current];
	NativeCustomTrack_Select(index);
	sdata->gGT->numPlyrNextGame = 1;
	s_nativeCustomTrackCapture.frames = 0;
	s_nativeCustomTrackCapture.settled = 0;
	printf("[CTR Native] Capturing '%s' (%d of %d)\n", s_nativeCustomTracks[index].disk.name, s_nativeCustomTrackCapture.current + 1,
	       s_nativeCustomTrackCapture.count);
	fflush(stdout);
	return s_nativeCustomTracks[index].disk.levelID;
}

// (StateZero: straight into the first track instead of the intro)
int NativeCustomTrack_Capture_BootLevel(int defaultLevel)
{
	return s_nativeCustomTrackCapture.active ? NativeCustomTrack_Capture_SelectCurrent() : defaultLevel;
}

internal void NativeCustomTrack_Capture_Next(void)
{
	if (++s_nativeCustomTrackCapture.current >= s_nativeCustomTrackCapture.count)
	{
		printf("[CTR Native] Captured %d custom track picture(s)\n", s_nativeCustomTrackCapture.count);
		fflush(stdout);
		exit(0);
	}
	MainRaceTrack_RequestLoad((s16)NativeCustomTrack_Capture_SelectCurrent());
}

// once a frame, before the HUD
void NativeCustomTrack_Capture_Tick(void)
{
	struct GameTracker *gGT = sdata->gGT;
	if (!s_nativeCustomTrackCapture.active || (sdata->Loading.stage != LOAD_IDLE) || (gGT->level1 == NULL) || s_nativeCustomTrackCapture.wantFrame)
		return;

	// no karts in the picture
	for (int i = 0; i < 8; i++)
	{
		if ((gGT->drivers[i] != NULL) && (gGT->drivers[i]->instSelf != NULL))
			gGT->drivers[i]->instSelf->flags |= HIDE_MODEL;
	}

	// (after the countdown: its traffic lights gone)
	s_nativeCustomTrackCapture.frames++;
	if (((gGT->hudFlags & HUD_FLAG_RACE_HUD) != 0) && ((gGT->gameMode1 & START_OF_RACE) == 0))
		s_nativeCustomTrackCapture.settled++;
	if ((s_nativeCustomTrackCapture.settled >= NATIVE_CUSTOM_TRACK_CAPTURE_SETTLE_FRAMES) ||
	    (s_nativeCustomTrackCapture.frames >= NATIVE_CUSTOM_TRACK_CAPTURE_GIVE_UP_FRAMES))
	{
		s_nativeCustomTrackCapture.wantFrame = true;
	}
}

// NativeRenderer_SwapWindow, before the swap
int NativeCustomTrack_Capture_WantsFrame(void)
{
	return s_nativeCustomTrackCapture.active && s_nativeCustomTrackCapture.wantFrame;
}

// the presented area, RGBA rows bottom-up
void NativeCustomTrack_Capture_OnFrame(const u8 *rgba, int width, int height)
{
#if !defined(__vita__) && !defined(__EMSCRIPTEN__)
	const int outW = NATIVE_CUSTOM_TRACK_PREVIEW_WIDTH;
	const int outH = NATIVE_CUSTOM_TRACK_PREVIEW_HEIGHT;
	struct NativeCustomTrackEntry *track = &s_nativeCustomTracks[s_nativeCustomTrackCapture.queue[s_nativeCustomTrackCapture.current]];

	// the middle of the frame in the preview's shape, box filtered down
	int cropW = width;
	int cropH = width * outH / outW;
	if (cropH > height)
	{
		cropH = height;
		cropW = height * outW / outH;
	}
	int cropX = (width - cropW) / 2;
	int cropY = (height - cropH) / 2;
	SDL_Surface *surface = SDL_CreateSurface(outW, outH, SDL_PIXELFORMAT_RGBA32);
	if (surface != NULL)
	{
		u8 *out = (u8 *)surface->pixels;
		for (int y = 0; y < outH; y++)
		{
			int y0 = cropY + y * cropH / outH;
			int y1 = cropY + (y + 1) * cropH / outH;
			y1 = (y1 > y0) ? y1 : y0 + 1;
			for (int x = 0; x < outW; x++)
			{
				int x0 = cropX + x * cropW / outW;
				int x1 = cropX + (x + 1) * cropW / outW;
				x1 = (x1 > x0) ? x1 : x0 + 1;
				u32 sum[3] = {0, 0, 0};
				u32 n = 0;
				for (int sy = y0; sy < y1; sy++)
				{
					const u8 *row = rgba + (size_t)(height - 1 - sy) * width * 4;
					for (int sx = x0; sx < x1; sx++)
					{
						sum[0] += row[sx * 4 + 0];
						sum[1] += row[sx * 4 + 1];
						sum[2] += row[sx * 4 + 2];
						n++;
					}
				}
				u8 *px = out + (size_t)y * surface->pitch + x * 4;
				px[0] = (u8)(sum[0] / n);
				px[1] = (u8)(sum[1] / n);
				px[2] = (u8)(sum[2] / n);
				px[3] = 0xff;
			}
		}
		if (!SDL_SavePNG(surface, track->previewPath))
			fprintf(stderr, "[CTR Native] Failed to save %s: %s\n", track->previewPath, SDL_GetError());
		SDL_DestroySurface(surface);
	}
#else
	(void)rgba;
	(void)width;
	(void)height;
#endif
	s_nativeCustomTrackCapture.wantFrame = false;
	NativeCustomTrack_Capture_Next();
}
