#include <common.h>
#include <platform/native_assets.h>
#include <platform/native_custom_track.h>
#include <platform/native_path.h>

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
	struct NativeCustomTrackDiskHeader disk;
	struct NativeCustomTrackBigHeader bigfile;
};

CTR_STATIC_ASSERT(sizeof(struct NativeCustomTrackDiskHeader) == 0x9c);
CTR_STATIC_ASSERT(NATIVE_MENU_STRING_CUSTOM_TRACK_LAST - NATIVE_MENU_STRING_CUSTOM_TRACK_0 + 1 == NATIVE_CUSTOM_TRACK_MAX);

global_variable struct NativeCustomTrackEntry s_nativeCustomTracks[NATIVE_CUSTOM_TRACK_MAX];
global_variable int s_nativeCustomTrackCount;
global_variable int s_nativeCustomTrackSelection = -1;

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
		if ((size == 0) || (offset < sizeof(track->disk)) || (offset + size > (u64)fileSize))
			return 0;
	}

	track->disk.name[sizeof(track->disk.name) - 1] = '\0';
	track->disk.author[sizeof(track->disk.author) - 1] = '\0';
	if ((track->disk.name[0] == '\0') || (strlen(path) >= sizeof(track->path)))
		return 0;
	strcpy(track->path, path);

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
	return ((index >= 0) && (index < s_nativeCustomTrackCount)) ? s_nativeCustomTracks[index].disk.name : "";
}

int NativeCustomTrack_GetLevelID(int index)
{
	return ((index >= 0) && (index < s_nativeCustomTrackCount)) ? s_nativeCustomTracks[index].disk.levelID : -1;
}

int NativeCustomTrack_HasAIPaths(int index)
{
	return ((index >= 0) && (index < s_nativeCustomTrackCount)) &&
	       ((s_nativeCustomTracks[index].disk.flags & NATIVE_CUSTOM_TRACK_FLAG_NO_AI_PATHS) == 0);
}

void NativeCustomTrack_Select(int index)
{
	s_nativeCustomTrackSelection = ((index >= 0) && (index < s_nativeCustomTrackCount)) ? index : -1;
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
