#include <common.h>
#include <platform/native_assets.h>
#include <platform/native_custom_racer.h>
#include <platform/native_disc_image.h>
#include <platform/native_gpu.h>
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

#define NATIVE_CUSTOM_RACER_MAGIC 0x52525443u
#define NATIVE_CUSTOM_RACER_VERSION 2u
#define NATIVE_CUSTOM_RACER_PATH_MAX 1024
#define NATIVE_CUSTOM_RACER_DIR "mods/customracers"

enum
{
	NATIVE_CUSTOM_RACER_PODIUM_COUNT = 3,
};

struct NativeCustomRacerDiskAsset
{
	u32 offset;
	u32 size;
};

struct NativeCustomRacerDiskVoiceFile
{
	u8 fileNumber;
	u8 reserved[3];
	struct NativeCustomRacerDiskAsset asset;
};

struct NativeCustomRacerDiskHeader
{
	u32 magic;
	u16 version;
	u16 headerSize;
	s16 templateCharacterID;
	s16 engineClass;
	u32 flags;
	u8 sourceHash[32];
	char name[64];
	char author[64];
	struct NativeCustomRacerDiskAsset assets[NATIVE_CUSTOM_RACER_ASSET_COUNT];
	u32 voiceFileCount;
	struct NativeCustomRacerDiskVoiceFile voiceFiles[NATIVE_CUSTOM_RACER_MAX_VOICE_FILES];
};

struct NativeCustomRacerBigHeader
{
	struct BigHeader header;
	struct BigEntry entries[NATIVE_CUSTOM_RACER_ASSET_COUNT];
};

struct NativeCustomRacerEntry
{
	char path[NATIVE_CUSTOM_RACER_PATH_MAX];
	struct NativeCustomRacerDiskHeader disk;
	struct NativeCustomRacerBigHeader bigfile;
	void *dramStorage[NATIVE_CUSTOM_RACER_ASSET_COUNT];
	void *previewStorage;
	struct Model *previewModel;
	u8 *voiceXnf;
	u32 voiceXnfSize;
	u32 portraitTexture;
	s16 portraitWidth;
	s16 portraitHeight;
};

struct NativeCustomRacerRetailPortrait
{
	u32 texture;
	s16 width;
	s16 height;
	RECT16 textureRect;
	u16 *textureWords;
	RECT16 clutRect;
	u16 *palette;
};

struct NativeCustomRacerTextureTask
{
	u32 texture;
	int width;
	int height;
	const u8 *pixels;
};

global_variable struct NativeCustomRacerEntry s_nativeCustomRacers[NATIVE_CUSTOM_RACER_MAX];
global_variable int s_nativeCustomRacerCount;
global_variable s16 s_nativeCustomRacerDriverSelection[LOAD_CHARACTER_ID_COUNT] = {-1, -1, -1, -1, -1, -1, -1, -1};
global_variable void *s_nativeCustomRacerDriverModelStorage[LOAD_CHARACTER_ID_COUNT];
global_variable s16 s_nativeCustomRacerPodiumSelection[NATIVE_CUSTOM_RACER_PODIUM_COUNT] = {-1, -1, -1};
global_variable void *s_nativeCustomRacerPodiumModelStorage[NATIVE_CUSTOM_RACER_PODIUM_COUNT];
global_variable s16 s_nativeCustomRacerSharedVram = -1;
global_variable struct NativeCustomRacerRetailPortrait s_nativeCustomRacerRetailPortraits[16];
global_variable u8 *s_nativeCustomRacerRetailSharedVram;
global_variable u32 s_nativeCustomRacerRetailSharedVramSize;
global_variable int s_nativeCustomRacerVoiceCharacterID = -1;

internal int NativeCustomRacer_ReadAsset(struct NativeCustomRacerEntry *racer, int assetIndex, void *destination);

internal int NativeCustomRacer_DriverIndexForModelTarget(void **target)
{
	if (target == NULL)
		return -1;

	for (int playerIndex = 0; playerIndex < LOAD_DRIVER_MODEL_EXTRA_COUNT; playerIndex++)
	{
		if (target == &data.driverModelExtras[playerIndex].fileBase)
			return playerIndex;
	}
	for (int driverIndex = 0; driverIndex < LOAD_CHARACTER_ID_COUNT; driverIndex++)
	{
		if (target == &s_nativeCustomRacerDriverModelStorage[driverIndex])
			return driverIndex;
	}
	return -1;
}

internal int NativeCustomRacer_SaveRetailPortraitVram(struct NativeCustomRacerRetailPortrait *portrait,
	const struct Icon *templateIcon, const u16 *vram)
{
	if ((portrait == NULL) || (templateIcon == NULL) || (vram == NULL))
		return 0;

	const int iconWidth = (int)templateIcon->texLayout.u1 - (int)templateIcon->texLayout.u0;
	const int iconHeight = (int)templateIcon->texLayout.v2 - (int)templateIcon->texLayout.v0;
	const int mode = (templateIcon->texLayout.tpage >> 7) & 3;
	const int pageX = (templateIcon->texLayout.tpage & 0xf) << 6;
	const int pageY = (templateIcon->texLayout.tpage & 0x10) ? 0x100 : 0;
	const int pixelsPerWord = (mode == 0) ? 4 : ((mode == 1) ? 2 : 1);
	if ((iconWidth <= 0) || (iconHeight <= 0) || (mode == 3))
		return 0;

	const int firstWord = templateIcon->texLayout.u0 / pixelsPerWord;
	const int lastWord = ((int)templateIcon->texLayout.u0 + iconWidth + pixelsPerWord - 1) / pixelsPerWord;
	const int wordWidth = lastWord - firstWord;
	if ((wordWidth <= 0) || (pageX + firstWord < 0) || (pageX + firstWord + wordWidth > VRAM_WIDTH) ||
	    (pageY + templateIcon->texLayout.v0 < 0) || (pageY + templateIcon->texLayout.v0 + iconHeight > VRAM_HEIGHT))
		return 0;

	u16 *textureWords = (u16 *)malloc((size_t)wordWidth * (size_t)iconHeight * sizeof(u16));
	if (textureWords == NULL)
		return 0;

	for (int y = 0; y < iconHeight; y++)
	{
		memcpy(&textureWords[y * wordWidth],
		       &vram[(pageY + templateIcon->texLayout.v0 + y) * VRAM_WIDTH + pageX + firstWord],
		       (size_t)wordWidth * sizeof(u16));
	}

	u16 *palette = NULL;
	RECT16 clutRect = {0};
	if (mode <= 1)
	{
		const int paletteWidth = mode == 0 ? 16 : 256;
		const int clutX = (templateIcon->texLayout.clut & 0x3f) << 4;
		const int clutY = templateIcon->texLayout.clut >> 6;
		if ((clutX < 0) || (clutY < 0) || (clutX + paletteWidth > VRAM_WIDTH) || (clutY >= VRAM_HEIGHT))
		{
			free(textureWords);
			return 0;
		}

		palette = (u16 *)malloc((size_t)paletteWidth * sizeof(u16));
		if (palette == NULL)
		{
			free(textureWords);
			return 0;
		}
		memcpy(palette, &vram[clutY * VRAM_WIDTH + clutX], (size_t)paletteWidth * sizeof(u16));
		clutRect = (RECT16){(s16)clutX, (s16)clutY, (s16)paletteWidth, 1};
	}

	free(portrait->textureWords);
	free(portrait->palette);
	portrait->textureWords = textureWords;
	portrait->textureRect = (RECT16){(s16)(pageX + firstWord), (s16)(pageY + templateIcon->texLayout.v0), (s16)wordWidth, (s16)iconHeight};
	portrait->palette = palette;
	portrait->clutRect = clutRect;
	return 1;
}

internal void NativeCustomRacer_BackendCreateTexture(void *arg)
{
	struct NativeCustomRacerTextureTask *task = (struct NativeCustomRacerTextureTask *)arg;
	task->texture = NativeRenderer_CreateStreamingTexture(task->width, task->height);
	if (task->texture != 0)
		NativeRenderer_UpdateStreamingTexture(task->texture, task->width, task->height, task->pixels);
}

internal u32 NativeCustomRacer_CreateTexture(int width, int height, const u8 *pixels)
{
	if ((width <= 0) || (height <= 0) || (pixels == NULL))
		return 0;

	struct NativeCustomRacerTextureTask task = {0, width, height, pixels};
#if defined(__vita__)
	NativeGpu_RunBackendTaskSync(NativeCustomRacer_BackendCreateTexture, &task);
#else
	NativeCustomRacer_BackendCreateTexture(&task);
#endif
	return task.texture;
}

internal void NativeCustomRacer_PsxColorToRgba(u16 color, u8 *rgba)
{
	if (color == 0)
	{
		rgba[0] = 0;
		rgba[1] = 0;
		rgba[2] = 0;
		rgba[3] = 0;
		return;
	}

	const u8 r = (u8)(color & 0x1f);
	const u8 g = (u8)((color >> 5) & 0x1f);
	const u8 b = (u8)((color >> 10) & 0x1f);
	rgba[0] = (u8)((r << 3) | (r >> 2));
	rgba[1] = (u8)((g << 3) | (g >> 2));
	rgba[2] = (u8)((b << 3) | (b >> 2));
	rgba[3] = 0xff;
}

internal int NativeCustomRacer_DecodePortrait(const u16 *vram, const struct Icon *icon, u8 **rgbaOut, int *widthOut, int *heightOut)
{
	if ((vram == NULL) || (icon == NULL) || (rgbaOut == NULL) || (widthOut == NULL) || (heightOut == NULL))
		return 0;

	const int width = (int)icon->texLayout.u1 - (int)icon->texLayout.u0;
	const int height = (int)icon->texLayout.v2 - (int)icon->texLayout.v0;
	const int mode = (icon->texLayout.tpage >> 7) & 3;
	const int pageX = (icon->texLayout.tpage & 0xf) << 6;
	const int pageY = (icon->texLayout.tpage & 0x10) ? 0x100 : 0;
	const int clutX = (icon->texLayout.clut & 0x3f) << 4;
	const int clutY = icon->texLayout.clut >> 6;
	if ((width <= 0) || (height <= 0) || (width > 255) || (height > 255) || (mode == 3))
		return 0;

	u8 *rgba = (u8 *)malloc((size_t)width * (size_t)height * 4u);
	if (rgba == NULL)
		return 0;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			const int u = (int)icon->texLayout.u0 + x;
			const int v = (int)icon->texLayout.v0 + y;
			u16 color;

			if (mode == 0)
			{
				const u16 packed = vram[(pageY + v) * VRAM_WIDTH + pageX + (u >> 2)];
				const int paletteIndex = (packed >> ((u & 3) * 4)) & 0xf;
				color = vram[clutY * VRAM_WIDTH + clutX + paletteIndex];
			}
			else if (mode == 1)
			{
				const u16 packed = vram[(pageY + v) * VRAM_WIDTH + pageX + (u >> 1)];
				const int paletteIndex = (packed >> ((u & 1) * 8)) & 0xff;
				color = vram[clutY * VRAM_WIDTH + clutX + paletteIndex];
			}
			else
			{
				color = vram[(pageY + v) * VRAM_WIDTH + pageX + u];
			}

			NativeCustomRacer_PsxColorToRgba(color, &rgba[((size_t)y * (size_t)width + (size_t)x) * 4u]);
		}
	}

	*rgbaOut = rgba;
	*widthOut = width;
	*heightOut = height;
	return 1;
}

internal int NativeCustomRacer_ApplyVramFileToBuffer(const u8 *fileData, u32 fileSize, u16 *vram)
{
	if ((fileData == NULL) || (fileSize < sizeof(struct VramHeader)) || (vram == NULL))
		return 0;

	const u8 *cursor = fileData;
	const u8 *end = fileData + fileSize;
	if (*(const u32 *)cursor != 0x20u)
	{
		const struct VramHeader *vh = (const struct VramHeader *)cursor;
		const size_t pixelBytes = (size_t)(u16)vh->rect.w * (size_t)(u16)vh->rect.h * sizeof(u16);
		if (((const u8 *)VRAMHEADER_GETPIXLES(vh) + pixelBytes > end) ||
		    (vh->rect.x < 0) || (vh->rect.y < 0) || (vh->rect.x + vh->rect.w > VRAM_WIDTH) || (vh->rect.y + vh->rect.h > VRAM_HEIGHT))
			return 0;
		for (int y = 0; y < vh->rect.h; y++)
			memcpy(&vram[(vh->rect.y + y) * VRAM_WIDTH + vh->rect.x], &((const u16 *)VRAMHEADER_GETPIXLES(vh))[y * vh->rect.w], (size_t)vh->rect.w * sizeof(u16));
		return 1;
	}

	cursor += sizeof(u32);
	while (cursor + sizeof(u32) <= end)
	{
		const u32 size = *(const u32 *)cursor;
		if (size == 0)
			return 1;
		cursor += sizeof(u32);
		if ((size < sizeof(struct VramHeader)) || (cursor + (size & ~3u) > end))
			return 0;

		const struct VramHeader *vh = (const struct VramHeader *)cursor;
		const size_t pixelBytes = (size_t)(u16)vh->rect.w * (size_t)(u16)vh->rect.h * sizeof(u16);
		if (((const u8 *)VRAMHEADER_GETPIXLES(vh) + pixelBytes > cursor + (size & ~3u)) ||
		    (vh->rect.x < 0) || (vh->rect.y < 0) || (vh->rect.x + vh->rect.w > VRAM_WIDTH) || (vh->rect.y + vh->rect.h > VRAM_HEIGHT))
			return 0;
		for (int y = 0; y < vh->rect.h; y++)
			memcpy(&vram[(vh->rect.y + y) * VRAM_WIDTH + vh->rect.x], &((const u16 *)VRAMHEADER_GETPIXLES(vh))[y * vh->rect.w], (size_t)vh->rect.w * sizeof(u16));
		cursor += size & ~3u;
	}
	return 0;
}

internal int NativeCustomRacer_UploadVramFile(const u8 *fileData, u32 fileSize)
{
	if ((fileData == NULL) || (fileSize < sizeof(struct VramHeader)))
		return 0;

	const u8 *cursor = fileData;
	const u8 *end = fileData + fileSize;
	if (*(const u32 *)cursor != 0x20u)
	{
		const struct VramHeader *vh = (const struct VramHeader *)cursor;
		const size_t pixelBytes = (size_t)(u16)vh->rect.w * (size_t)(u16)vh->rect.h * sizeof(u16);
		if (((const u8 *)VRAMHEADER_GETPIXLES(vh) + pixelBytes > end) ||
		    (vh->rect.x < 0) || (vh->rect.y < 0) || (vh->rect.x + vh->rect.w > VRAM_WIDTH) || (vh->rect.y + vh->rect.h > VRAM_HEIGHT))
			return 0;
		RECT16 rect = {(s16)vh->rect.x, (s16)vh->rect.y, (s16)vh->rect.w, (s16)vh->rect.h};
		LoadImage(&rect, VRAMHEADER_GETPIXLES(vh));
		return 1;
	}

	cursor += sizeof(u32);
	while (cursor + sizeof(u32) <= end)
	{
		const u32 size = *(const u32 *)cursor;
		if (size == 0)
			return 1;
		cursor += sizeof(u32);
		if ((size < sizeof(struct VramHeader)) || (cursor + (size & ~3u) > end))
			return 0;

		const struct VramHeader *vh = (const struct VramHeader *)cursor;
		const size_t pixelBytes = (size_t)(u16)vh->rect.w * (size_t)(u16)vh->rect.h * sizeof(u16);
		if (((const u8 *)VRAMHEADER_GETPIXLES(vh) + pixelBytes > cursor + (size & ~3u)) ||
		    (vh->rect.x < 0) || (vh->rect.y < 0) || (vh->rect.x + vh->rect.w > VRAM_WIDTH) || (vh->rect.y + vh->rect.h > VRAM_HEIGHT))
			return 0;
		RECT16 rect = {(s16)vh->rect.x, (s16)vh->rect.y, (s16)vh->rect.w, (s16)vh->rect.h};
		LoadImage(&rect, VRAMHEADER_GETPIXLES(vh));
		cursor += size & ~3u;
	}
	return 0;
}

CTR_STATIC_ASSERT(sizeof(struct NativeCustomRacerDiskHeader) == 0x24c);

internal int NativeCustomRacer_HasExtension(const char *name, const char *extension)
{
	size_t nameLen;
	size_t extensionLen;

	if ((name == NULL) || (extension == NULL))
		return 0;

	nameLen = strlen(name);
	extensionLen = strlen(extension);
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

internal int NativeCustomRacer_ReadHeader(const char *path, struct NativeCustomRacerEntry *racer)
{
	FILE *file;
	long fileSize;

	file = fopen(path, "rb");
	if (file == NULL)
		return 0;

	if (fseek(file, 0, SEEK_END) != 0)
	{
		fclose(file);
		return 0;
	}
	fileSize = ftell(file);
	if ((fileSize < (long)sizeof(racer->disk)) || (fseek(file, 0, SEEK_SET) != 0))
	{
		fclose(file);
		return 0;
	}

	memset(racer, 0, sizeof(*racer));
	if (fread(&racer->disk, 1, sizeof(racer->disk), file) != sizeof(racer->disk))
	{
		fclose(file);
		return 0;
	}
	fclose(file);

	if ((racer->disk.magic != NATIVE_CUSTOM_RACER_MAGIC) ||
	    (racer->disk.version != NATIVE_CUSTOM_RACER_VERSION) ||
	    (racer->disk.headerSize != sizeof(racer->disk)) ||
	    (racer->disk.templateCharacterID < 0) || (racer->disk.templateCharacterID >= 16) ||
	    (racer->disk.engineClass < -1) || (racer->disk.engineClass >= NUM_CLASSES))
	{
		return 0;
	}

	for (int i = 0; i < NATIVE_CUSTOM_RACER_ASSET_COUNT; i++)
	{
		u64 offset = racer->disk.assets[i].offset;
		u64 size = racer->disk.assets[i].size;
		if ((size != 0) && ((offset < sizeof(racer->disk)) || (offset + size > (u64)fileSize)))
		{
			return 0;
		}
	}
	if (racer->disk.voiceFileCount > NATIVE_CUSTOM_RACER_MAX_VOICE_FILES)
		return 0;
	for (u32 i = 0; i < racer->disk.voiceFileCount; i++)
	{
		const u64 offset = racer->disk.voiceFiles[i].asset.offset;
		const u64 size = racer->disk.voiceFiles[i].asset.size;
		if ((size == 0) || (offset < sizeof(racer->disk)) || (offset + size > (u64)fileSize))
			return 0;
	}

	racer->disk.name[sizeof(racer->disk.name) - 1] = '\0';
	racer->disk.author[sizeof(racer->disk.author) - 1] = '\0';
	if (racer->disk.name[0] == '\0')
		return 0;

	if (strlen(path) >= sizeof(racer->path))
		return 0;
	strcpy(racer->path, path);

	racer->bigfile.header.cdpos = -1;
	racer->bigfile.header.numEntry = NATIVE_CUSTOM_RACER_ASSET_COUNT;
	for (int i = 0; i < NATIVE_CUSTOM_RACER_ASSET_COUNT; i++)
	{
		racer->bigfile.entries[i].offset = (int)racer->disk.assets[i].offset;
		racer->bigfile.entries[i].size = (int)racer->disk.assets[i].size;
	}
	return 1;
}

internal int NativeCustomRacer_Compare(const void *a, const void *b)
{
	const struct NativeCustomRacerEntry *ra = a;
	const struct NativeCustomRacerEntry *rb = b;
	return strcmp(ra->disk.name, rb->disk.name);
}

internal void NativeCustomRacer_AddPath(const char *path)
{
	struct NativeCustomRacerEntry racer;
	if (s_nativeCustomRacerCount >= NATIVE_CUSTOM_RACER_MAX)
		return;
	if (!NativeCustomRacer_ReadHeader(path, &racer))
	{
		fprintf(stderr, "[CTR Native] Ignoring invalid custom racer package: %s\n", path);
		return;
	}
	s_nativeCustomRacers[s_nativeCustomRacerCount++] = racer;
}

internal int NativeCustomRacer_CacheRetailSharedVramFromAssets(void)
{
	if (s_nativeCustomRacerRetailSharedVram != NULL)
		return 1;

	const size_t headerSize = (size_t)LOAD_BIGFILE_HEADER_SECTORS * LOAD_CD_DATA_SECTOR_SIZE;
	u8 *headerData = (u8 *)malloc(headerSize);
	u8 *vramData = NULL;
	u32 vramSize = 0;
	if (headerData == NULL)
		return 0;

	FILE *hostBigfile = NativeAssets_OpenHostBigfile("rb");
	if (hostBigfile != NULL)
	{
		if (fread(headerData, 1, headerSize, hostBigfile) != headerSize)
			goto host_fail;

		struct BigHeader *header = (struct BigHeader *)headerData;
		if ((header->numEntry <= BI_SHAREDMPKVRM) ||
		    (sizeof(*header) + (size_t)header->numEntry * sizeof(struct BigEntry) > headerSize))
			goto host_fail;

		struct BigEntry *entries = BIG_GETENTRY(header);
		struct BigEntry *entry = &entries[BI_SHAREDMPKVRM];
		if ((entry->offset < 0) || (entry->size < (int)sizeof(struct VramHeader)))
			goto host_fail;

		vramSize = (u32)entry->size;
		vramData = (u8 *)malloc(vramSize);
		if ((vramData == NULL) ||
		    (fseek(hostBigfile, (long)((u32)entry->offset << LOAD_CD_DATA_SECTOR_SHIFT), SEEK_SET) != 0) ||
		    (fread(vramData, 1, vramSize, hostBigfile) != vramSize))
		{
			free(vramData);
			vramData = NULL;
			goto host_fail;
		}

		fclose(hostBigfile);
	}
	else
	{
		struct NativeDiscImageFile discBigfile;
		if (!NativeDiscImage_FindFile("BIGFILE.BIG", &discBigfile) ||
		    !NativeDiscImage_ReadDataSectors(&discBigfile, 0, LOAD_BIGFILE_HEADER_SECTORS, headerData))
			goto fail;

		struct BigHeader *header = (struct BigHeader *)headerData;
		if ((header->numEntry <= BI_SHAREDMPKVRM) ||
		    (sizeof(*header) + (size_t)header->numEntry * sizeof(struct BigEntry) > headerSize))
			goto fail;

		struct BigEntry *entries = BIG_GETENTRY(header);
		struct BigEntry *entry = &entries[BI_SHAREDMPKVRM];
		if ((entry->offset < 0) || (entry->size < (int)sizeof(struct VramHeader)))
			goto fail;

		vramSize = (u32)entry->size;
		const u32 sectorCount = (vramSize + LOAD_CD_DATA_SECTOR_ROUND_MASK) >> LOAD_CD_DATA_SECTOR_SHIFT;
		const size_t readSize = (size_t)sectorCount * LOAD_CD_DATA_SECTOR_SIZE;
		vramData = (u8 *)malloc(readSize);
		if ((vramData == NULL) ||
		    !NativeDiscImage_ReadDataSectors(&discBigfile, (u32)entry->offset, sectorCount, vramData))
		{
			free(vramData);
			vramData = NULL;
			goto fail;
		}
	}

	free(headerData);
	s_nativeCustomRacerRetailSharedVram = vramData;
	s_nativeCustomRacerRetailSharedVramSize = vramSize;
	printf("[CTR Native] Cached retail shared racer VRAM (%u bytes)\n", vramSize);
	return 1;

host_fail:
	fclose(hostBigfile);
fail:
	free(vramData);
	free(headerData);
	return 0;
}

int NativeCustomRacer_Scan(void)
{
	char directory[NATIVE_CUSTOM_RACER_PATH_MAX];
	s_nativeCustomRacerCount = 0;
	NativeCustomRacer_ClearDriverSelections();
	NativeCustomRacer_ClearPodiumSelections();

	if (!NativePath_Join(directory, sizeof(directory), NativeStr8_FromCString(NativeAssets_GetBaseDir()), NATIVE_STR8_LIT(NATIVE_CUSTOM_RACER_DIR)))
		return 0;

#if defined(_WIN32)
	{
		char searchPath[NATIVE_CUSTOM_RACER_PATH_MAX];
		WIN32_FIND_DATAA findData;
		HANDLE findHandle;

		if (!NativePath_Join(searchPath, sizeof(searchPath), NativeStr8_FromCString(directory), NATIVE_STR8_LIT("*")))
			return 0;
		findHandle = FindFirstFileA(searchPath, &findData);
		if (findHandle == INVALID_HANDLE_VALUE)
			return 0;

		do
		{
			char path[NATIVE_CUSTOM_RACER_PATH_MAX];
			if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
				continue;
			if (!NativeCustomRacer_HasExtension(findData.cFileName, ".ctrr"))
				continue;
			if (NativePath_Join(path, sizeof(path), NativeStr8_FromCString(directory), NativeStr8_FromCString(findData.cFileName)))
				NativeCustomRacer_AddPath(path);
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
			char path[NATIVE_CUSTOM_RACER_PATH_MAX];
			if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0))
				continue;
			if (!NativeCustomRacer_HasExtension(entry->d_name, ".ctrr"))
				continue;
			if (NativePath_Join(path, sizeof(path), NativeStr8_FromCString(directory), NativeStr8_FromCString(entry->d_name)))
				NativeCustomRacer_AddPath(path);
		}
		closedir(dir);
	}
#endif

	if (s_nativeCustomRacerCount > 1)
		qsort(s_nativeCustomRacers, (size_t)s_nativeCustomRacerCount, sizeof(s_nativeCustomRacers[0]), NativeCustomRacer_Compare);

	if (s_nativeCustomRacerCount != 0)
	{
		printf("[CTR Native] Found %d custom racer package(s) in %s\n", s_nativeCustomRacerCount, directory);
		if (!NativeCustomRacer_CacheRetailSharedVramFromAssets())
			fprintf(stderr, "[CTR Native] Warning: failed to cache retail shared racer VRAM; custom AI textures may be unavailable until it is loaded normally.\n");
	}

	return s_nativeCustomRacerCount;
}

int NativeCustomRacer_GetCount(void)
{
	return s_nativeCustomRacerCount;
}


const char *NativeCustomRacer_GetName(int index)
{
	return (index >= 0 && index < s_nativeCustomRacerCount) ? s_nativeCustomRacers[index].disk.name : "";
}


int NativeCustomRacer_GetTemplateCharacterID(int index)
{
	return (index >= 0 && index < s_nativeCustomRacerCount) ? s_nativeCustomRacers[index].disk.templateCharacterID : CRASH_BANDICOOT;
}

int NativeCustomRacer_GetEngineClass(int index)
{
	if (index < 0 || index >= s_nativeCustomRacerCount)
		return -1;
	return s_nativeCustomRacers[index].disk.engineClass;
}

struct Model *NativeCustomRacer_GetPreviewModel(int index)
{
	if ((index < 0) || (index >= s_nativeCustomRacerCount))
		return NULL;

	struct NativeCustomRacerEntry *racer = &s_nativeCustomRacers[index];
	if (racer->previewModel != NULL)
		return racer->previewModel;

	const u32 size = racer->disk.assets[NATIVE_CUSTOM_RACER_ASSET_MODEL_HI].size;
	if (size < LOAD_MODEL_FILE_HEADER_BYTES)
		return NULL;

	u8 *fileBuf = (u8 *)malloc(size);
	if ((fileBuf == NULL) || !NativeCustomRacer_ReadAsset(racer, NATIVE_CUSTOM_RACER_ASSET_MODEL_HI, fileBuf))
	{
		free(fileBuf);
		return NULL;
	}

	const int ptrMapOffset = *(int *)&fileBuf[0];
	u8 *realFileBuf = &fileBuf[LOAD_MODEL_FILE_HEADER_BYTES];
	if (ptrMapOffset >= 0)
	{
		if ((u32)ptrMapOffset >= size - LOAD_MODEL_FILE_HEADER_BYTES)
		{
			free(fileBuf);
			return NULL;
		}
		struct DramPointerMap *dpm = (struct DramPointerMap *)&realFileBuf[ptrMapOffset];
		LOAD_RunPtrMap((char *)realFileBuf, (int *)DRAM_GETOFFSETS(dpm), dpm->numBytes >> 2);
	}

	racer->previewStorage = fileBuf;
	racer->previewModel = (struct Model *)realFileBuf;
	return racer->previewModel;
}

u32 NativeCustomRacer_GetPortraitTexture(int index, const struct Icon *templateIcon, int *width, int *height)
{
	if ((index < 0) || (index >= s_nativeCustomRacerCount) || (templateIcon == NULL))
		return 0;

	struct NativeCustomRacerEntry *racer = &s_nativeCustomRacers[index];
	if (racer->portraitTexture != 0)
	{
		if (width != NULL) *width = racer->portraitWidth;
		if (height != NULL) *height = racer->portraitHeight;
		return racer->portraitTexture;
	}

	const u32 vrmSize = racer->disk.assets[NATIVE_CUSTOM_RACER_ASSET_SHARED_VRM].size;
	if (vrmSize < sizeof(struct VramHeader))
		return 0;

	u8 *vrm = (u8 *)malloc(vrmSize);
	u16 *vram = (u16 *)calloc((size_t)VRAM_WIDTH * (size_t)VRAM_HEIGHT, sizeof(u16));
	u8 *rgba = NULL;
	int portraitWidth = 0;
	int portraitHeight = 0;
	if ((vrm == NULL) || (vram == NULL) ||
	    !NativeCustomRacer_ReadAsset(racer, NATIVE_CUSTOM_RACER_ASSET_SHARED_VRM, vrm) ||
	    !NativeCustomRacer_ApplyVramFileToBuffer(vrm, vrmSize, vram) ||
	    !NativeCustomRacer_DecodePortrait(vram, templateIcon, &rgba, &portraitWidth, &portraitHeight))
	{
		free(rgba);
		free(vram);
		free(vrm);
		return 0;
	}

	racer->portraitTexture = NativeCustomRacer_CreateTexture(portraitWidth, portraitHeight, rgba);
	if (racer->portraitTexture != 0)
	{
		racer->portraitWidth = (s16)portraitWidth;
		racer->portraitHeight = (s16)portraitHeight;
	}

	free(rgba);
	free(vram);
	free(vrm);
	if (width != NULL) *width = racer->portraitWidth;
	if (height != NULL) *height = racer->portraitHeight;
	return racer->portraitTexture;
}

u32 NativeCustomRacer_GetRetailPortraitTexture(int templateCharacterID, const struct Icon *templateIcon, int *width, int *height)
{
	if ((templateCharacterID < 0) || (templateCharacterID >= 16) || (templateIcon == NULL))
		return 0;

	struct NativeCustomRacerRetailPortrait *portrait = &s_nativeCustomRacerRetailPortraits[templateCharacterID];
	if (portrait->texture != 0)
	{
		if (width != NULL) *width = portrait->width;
		if (height != NULL) *height = portrait->height;
		return portrait->texture;
	}

	// Prefer the untouched retail BI_SHAREDMPKVRM captured by LOAD_VramFileCallback.
	if ((s_nativeCustomRacerRetailSharedVram != NULL) &&
	    (s_nativeCustomRacerRetailSharedVramSize >= sizeof(struct VramHeader)))
	{
		u16 *retailVram = (u16 *)calloc((size_t)VRAM_WIDTH * (size_t)VRAM_HEIGHT, sizeof(u16));
		u8 *retailRgba = NULL;
		int retailWidth = 0;
		int retailHeight = 0;
		if ((retailVram != NULL) &&
		    NativeCustomRacer_ApplyVramFileToBuffer(s_nativeCustomRacerRetailSharedVram,
		                                             s_nativeCustomRacerRetailSharedVramSize, retailVram) &&
		    NativeCustomRacer_DecodePortrait(retailVram, templateIcon, &retailRgba, &retailWidth, &retailHeight))
		{
			NativeCustomRacer_SaveRetailPortraitVram(portrait, templateIcon, retailVram);
			portrait->texture = NativeCustomRacer_CreateTexture(retailWidth, retailHeight, retailRgba);
			if (portrait->texture != 0)
			{
				portrait->width = (s16)retailWidth;
				portrait->height = (s16)retailHeight;
			}
		}
		free(retailRgba);
		free(retailVram);
		if (portrait->texture != 0)
		{
			if (width != NULL) *width = portrait->width;
			if (height != NULL) *height = portrait->height;
			return portrait->texture;
		}
	}

	const int iconWidth = (int)templateIcon->texLayout.u1 - (int)templateIcon->texLayout.u0;
	const int iconHeight = (int)templateIcon->texLayout.v2 - (int)templateIcon->texLayout.v0;
	const int mode = (templateIcon->texLayout.tpage >> 7) & 3;
	const int pageX = (templateIcon->texLayout.tpage & 0xf) << 6;
	const int pageY = (templateIcon->texLayout.tpage & 0x10) ? 0x100 : 0;
	const int pixelsPerWord = (mode == 0) ? 4 : ((mode == 1) ? 2 : 1);
	if ((iconWidth <= 0) || (iconHeight <= 0) || (mode == 3))
		return 0;

	const int firstWord = templateIcon->texLayout.u0 / pixelsPerWord;
	const int lastWord = ((int)templateIcon->texLayout.u0 + iconWidth + pixelsPerWord - 1) / pixelsPerWord;
	const int wordWidth = lastWord - firstWord;
	u16 *vram = (u16 *)calloc((size_t)VRAM_WIDTH * (size_t)VRAM_HEIGHT, sizeof(u16));
	u16 *textureWords = (u16 *)malloc((size_t)wordWidth * (size_t)iconHeight * sizeof(u16));
	u16 *palette = NULL;
	u8 *rgba = NULL;
	int portraitWidth = 0;
	int portraitHeight = 0;
	if ((vram == NULL) || (textureWords == NULL))
		goto retail_fail;

	RECT16 textureRect;
	textureRect.x = (s16)(pageX + firstWord);
	textureRect.y = (s16)(pageY + templateIcon->texLayout.v0);
	textureRect.w = (s16)wordWidth;
	textureRect.h = (s16)iconHeight;
	StoreImage(&textureRect, (u32 *)textureWords);
	for (int y = 0; y < iconHeight; y++)
		memcpy(&vram[(textureRect.y + y) * VRAM_WIDTH + textureRect.x], &textureWords[y * wordWidth], (size_t)wordWidth * sizeof(u16));

	if (mode <= 1)
	{
		const int paletteWidth = mode == 0 ? 16 : 256;
		const int clutX = (templateIcon->texLayout.clut & 0x3f) << 4;
		const int clutY = templateIcon->texLayout.clut >> 6;
		palette = (u16 *)malloc((size_t)paletteWidth * sizeof(u16));
		if (palette == NULL)
			goto retail_fail;
		RECT16 clutRect = {(s16)clutX, (s16)clutY, (s16)paletteWidth, 1};
		StoreImage(&clutRect, (u32 *)palette);
		memcpy(&vram[clutY * VRAM_WIDTH + clutX], palette, (size_t)paletteWidth * sizeof(u16));
	}
	NativeCustomRacer_SaveRetailPortraitVram(portrait, templateIcon, vram);

	if (!NativeCustomRacer_DecodePortrait(vram, templateIcon, &rgba, &portraitWidth, &portraitHeight))
		goto retail_fail;

	portrait->texture = NativeCustomRacer_CreateTexture(portraitWidth, portraitHeight, rgba);
	if (portrait->texture != 0)
	{
		portrait->width = (s16)portraitWidth;
		portrait->height = (s16)portraitHeight;
	}

	free(rgba);
	free(palette);
	free(textureWords);
	free(vram);
	if (width != NULL) *width = portrait->width;
	if (height != NULL) *height = portrait->height;
	return portrait->texture;

retail_fail:
	free(rgba);
	free(palette);
	free(textureWords);
	free(vram);
	return 0;
}

void NativeCustomRacer_CaptureRetailSharedVram(const void *fileData, u32 fileSize)
{
	if ((fileData == NULL) || (fileSize < sizeof(struct VramHeader)) ||
	    (s_nativeCustomRacerRetailSharedVram != NULL))
		return;

	u8 *copy = (u8 *)malloc(fileSize);
	if (copy == NULL)
		return;

	memcpy(copy, fileData, fileSize);
	s_nativeCustomRacerRetailSharedVram = copy;
	s_nativeCustomRacerRetailSharedVramSize = fileSize;
}

internal void NativeCustomRacer_CacheRetailTemplatePortraits(void)
{
	if ((sdata == NULL) || (sdata->gGT == NULL))
		return;

	struct GameTracker *gGT = sdata->gGT;
	u32 cachedTemplates = 0;
	for (int racerIndex = 0; racerIndex < s_nativeCustomRacerCount; racerIndex++)
	{
		const int characterID = s_nativeCustomRacers[racerIndex].disk.templateCharacterID;
		const u32 characterBit = 1u << characterID;
		if ((cachedTemplates & characterBit) != 0)
			continue;

		cachedTemplates |= characterBit;
		const int iconID = data.MetaDataCharacters[characterID].iconID;
		struct Icon *icon = gGT->ptrIcons[iconID];
		if (icon != NULL)
			NativeCustomRacer_GetRetailPortraitTexture(characterID, icon, NULL, NULL);
	}
}

internal void NativeCustomRacer_RestoreRetailTemplatePortraitsVram(void)
{
	if ((sdata == NULL) || (sdata->gGT == NULL))
		return;

	struct GameTracker *gGT = sdata->gGT;
	u32 restoredTemplates = 0;
	for (int racerIndex = 0; racerIndex < s_nativeCustomRacerCount; racerIndex++)
	{
		const int characterID = s_nativeCustomRacers[racerIndex].disk.templateCharacterID;
		const u32 characterBit = 1u << characterID;
		if ((restoredTemplates & characterBit) != 0)
			continue;
		restoredTemplates |= characterBit;

		const int iconID = data.MetaDataCharacters[characterID].iconID;
		struct Icon *icon = gGT->ptrIcons[iconID];
		if (icon == NULL)
			continue;

		NativeCustomRacer_GetRetailPortraitTexture(characterID, icon, NULL, NULL);
		struct NativeCustomRacerRetailPortrait *portrait = &s_nativeCustomRacerRetailPortraits[characterID];
		if (portrait->textureWords != NULL)
			LoadImage(&portrait->textureRect, portrait->textureWords);
		if (portrait->palette != NULL)
			LoadImage(&portrait->clutRect, portrait->palette);
	}
}

int NativeCustomRacer_GetVoiceTrack(int categoryID, int xaID, int *channelFilter, int *numSectors,
                                    const char **packagePath, u64 *assetOffset, u32 *assetSize)
{
	// Character speech uses custom XA data; music and extra XA stay retail.
	if ((categoryID != 2) || (xaID < 0) || !NativeCustomRacer_IsRosterEnabled())
		return 0;

	int racerIndex = -1;
	if (s_nativeCustomRacerVoiceCharacterID >= 0)
	{
		for (int driverIndex = 0; driverIndex < LOAD_CHARACTER_ID_COUNT; driverIndex++)
		{
			const int selected = NativeCustomRacer_GetDriverSelection(driverIndex);
			if ((selected >= 0) &&
			    (NativeCustomRacer_GetTemplateCharacterID(selected) == s_nativeCustomRacerVoiceCharacterID))
			{
				racerIndex = selected;
				break;
			}
		}
		if (racerIndex < 0)
		{
			return 0;
		}
	}
	else
	{
		racerIndex = NativeCustomRacer_GetPlayerSelection(0);
	}
	if ((racerIndex < 0) || (racerIndex >= s_nativeCustomRacerCount))
		return 0;
	struct NativeCustomRacerEntry *racer = &s_nativeCustomRacers[racerIndex];

	if (racer->voiceXnf == NULL)
	{
		const u32 size = racer->disk.assets[NATIVE_CUSTOM_RACER_ASSET_VOICE_XNF].size;
		if (size < 0x44)
			return 0;
		racer->voiceXnf = (u8 *)malloc(size);
		if ((racer->voiceXnf == NULL) || !NativeCustomRacer_ReadAsset(racer, NATIVE_CUSTOM_RACER_ASSET_VOICE_XNF, racer->voiceXnf))
		{
			free(racer->voiceXnf);
			racer->voiceXnf = NULL;
			return 0;
		}
		racer->voiceXnfSize = size;
	}

	const u8 *xnf = racer->voiceXnf;
	const u32 xnfSize = racer->voiceXnfSize;
	if ((xnfSize < 0x44) || (CTR_ReadU32LE(&xnf[0]) != 0x464e4958u) ||
	    (CTR_ReadU32LE(&xnf[4]) != 102u) || (CTR_ReadU32LE(&xnf[8]) != 3u))
		return 0;

	const int numXasTotal = (int)CTR_ReadU32LE(&xnf[0x0c]);
	const int numTracksTotal = (int)CTR_ReadU32LE(&xnf[0x10]);
	const int numSongs = (int)CTR_ReadU32LE(&xnf[0x2c + categoryID * 4]);
	const int firstSongIndex = (int)CTR_ReadU32LE(&xnf[0x38 + categoryID * 4]);
	if ((numXasTotal < 0) || (numTracksTotal < 0) || (xaID >= numSongs))
		return 0;

	const u32 entryOffset = 0x44u + (u32)numXasTotal * 4u;
	const int entryIndex = firstSongIndex + xaID;
	if ((entryIndex < 0) || (entryIndex >= numTracksTotal) ||
	    (entryOffset + ((u32)entryIndex + 1u) * 4u > xnfSize))
		return 0;

	const u8 *entry = &xnf[entryOffset + (u32)entryIndex * 4u];
	const int fileNumber = entry[1];
	const int sectors = (s16)((u16)entry[2] | ((u16)entry[3] << 8));
	if (sectors <= 0)
		return 0;

	for (u32 i = 0; i < racer->disk.voiceFileCount; i++)
	{
		if (racer->disk.voiceFiles[i].fileNumber != fileNumber)
			continue;
		if (channelFilter != NULL) *channelFilter = entry[0];
		if (numSectors != NULL) *numSectors = sectors;
		if (packagePath != NULL) *packagePath = racer->path;
		if (assetOffset != NULL) *assetOffset = racer->disk.voiceFiles[i].asset.offset;
		if (assetSize != NULL) *assetSize = racer->disk.voiceFiles[i].asset.size;
		return 1;
	}

	return 0;
}

void NativeCustomRacer_SetActiveVoiceCharacter(int characterID)
{
	s_nativeCustomRacerVoiceCharacterID = ((characterID >= 0) && (characterID < 16)) ? characterID : -1;
}

int NativeCustomRacer_IsRosterEnabled(void)
{
#if defined(__vita__)
	if (NativeAdhoc_IsActive())
		return 0;
#endif
	return s_nativeCustomRacerCount > 0;
}

int NativeCustomRacer_DisablesRecords(void)
{
	// Time Trial and Relic Race records are invalid when P1 is custom.
	return NativeCustomRacer_GetPlayerSelection(0) >= 0;
}

internal struct NativeCustomRacerEntry *NativeCustomRacer_FindByBigHeader(const struct BigHeader *bigfile)
{
	if (bigfile == NULL)
		return NULL;
	for (int i = 0; i < s_nativeCustomRacerCount; i++)
	{
		if (bigfile == &s_nativeCustomRacers[i].bigfile.header)
			return &s_nativeCustomRacers[i];
	}
	return NULL;
}

int NativeCustomRacer_IsBigHeader(const struct BigHeader *bigfile)
{
	return NativeCustomRacer_FindByBigHeader(bigfile) != NULL;
}

internal int NativeCustomRacer_ReadAsset(struct NativeCustomRacerEntry *racer, int assetIndex, void *destination)
{
	if ((racer == NULL) || (assetIndex < 0) || (assetIndex >= NATIVE_CUSTOM_RACER_ASSET_COUNT) || (destination == NULL))
		return 0;
	const u32 offset = racer->disk.assets[assetIndex].offset;
	const u32 size = racer->disk.assets[assetIndex].size;
	if ((offset == 0) || (size == 0) || (offset > 0x7fffffffu))
		return 0;

	FILE *file = fopen(racer->path, "rb");
	if (file == NULL)
		return 0;
	const int ok = (fseek(file, (long)offset, SEEK_SET) == 0) && (fread(destination, 1, size, file) == size);
	fclose(file);
	return ok;
}

int NativeCustomRacer_LoadQueueSlot(struct LoadQueueSlot *slot)
{
	if (slot == NULL)
		return 0;
	struct NativeCustomRacerEntry *racer = NativeCustomRacer_FindByBigHeader(slot->ptrBigfileCdPos_UNUSED);
	if ((racer == NULL) || (slot->subfileIndex >= NATIVE_CUSTOM_RACER_ASSET_COUNT))
		return 0;

	const int assetIndex = (int)slot->subfileIndex;
	const u32 size = racer->disk.assets[assetIndex].size;
	if (size == 0)
	{
		sdata->queueReady = 1;
		return 0;
	}

	void **setPointerTarget = NULL;
	void *destination = slot->ptrDestination;
	if ((slot->type_UNUSED == LT_DRAM) && (slot->callbackFuncPtr == LOAD_QUEUE_CALLBACK_SET_POINTER))
	{
		setPointerTarget = (void **)slot->ptrDestination;
		destination = NULL;
	}
	if (destination == NULL)
	{
		destination = malloc(size);
		if (destination == NULL)
		{
			fprintf(stderr, "[CTR Native] Failed to allocate %u bytes for custom racer asset %d\n", size, assetIndex);
			sdata->queueReady = 1;
			return 0;
		}
	}

	if (!NativeCustomRacer_ReadAsset(racer, assetIndex, destination))
	{
		fprintf(stderr, "[CTR Native] Failed to read custom racer asset %d from %s\n", assetIndex, racer->path);
		free(destination);
		sdata->queueReady = 1;
		return 0;
	}

	printf("[CTR Native] Loading custom racer %s asset=%d size=%u\n", racer->disk.name, assetIndex, size);
	slot->ptrDestination = destination;
	slot->size_UNUSED = size;
	slot->flags &= ~LT_MEMPACK;

	if (slot->type_UNUSED == LT_DRAM)
	{
			const int driverModelIndex = (assetIndex == NATIVE_CUSTOM_RACER_ASSET_MODEL_HI)
				? NativeCustomRacer_DriverIndexForModelTarget(setPointerTarget)
				: -1;
			if (driverModelIndex >= 0)
			{
				free(s_nativeCustomRacerDriverModelStorage[driverModelIndex]);
				s_nativeCustomRacerDriverModelStorage[driverModelIndex] = destination;
		}
		else
		{
			if (racer->dramStorage[assetIndex] != NULL)
				free(racer->dramStorage[assetIndex]);
			racer->dramStorage[assetIndex] = destination;
		}
		LOAD_DramFileCallback(slot);
		if (setPointerTarget != NULL)
			*setPointerTarget = destination;
		return 1;
	}
	if (slot->type_UNUSED == LT_VRAM)
	{
		LOAD_VramFileCallback(slot);
		if (assetIndex == NATIVE_CUSTOM_RACER_ASSET_SHARED_VRM)
		{
			NativeCustomRacer_RestoreRetailTemplatePortraitsVram();
		}
		return 1;
	}

	free(destination);
	slot->ptrDestination = NULL;
	sdata->queueReady = 1;
	return 0;
}

void NativeCustomRacer_FinishQueueSlot(struct LoadQueueSlot *slot)
{
	if ((slot == NULL) || !NativeCustomRacer_IsBigHeader(slot->ptrBigfileCdPos_UNUSED))
		return;
	if ((slot->type_UNUSED == LT_VRAM) && (slot->ptrDestination != NULL))
	{
		free(slot->ptrDestination);
		slot->ptrDestination = NULL;
	}
}

int NativeCustomRacer_QueueDriverModel(int driverIndex, void **destination)
{
	if (!NativeCustomRacer_IsRosterEnabled())
		return 0;

	const int racerIndex = NativeCustomRacer_GetDriverSelection(driverIndex);
	if ((racerIndex < 0) || (racerIndex >= s_nativeCustomRacerCount) ||
	    (s_nativeCustomRacers[racerIndex].disk.assets[NATIVE_CUSTOM_RACER_ASSET_MODEL_HI].size == 0))
	{
		return 0;
	}
	if ((destination == NULL) && (driverIndex >= 0) && (driverIndex < LOAD_CHARACTER_ID_COUNT))
		destination = &s_nativeCustomRacerDriverModelStorage[driverIndex];

	LOAD_AppendQueue(&s_nativeCustomRacers[racerIndex].bigfile.header, LT_GETADDR,
	                 NATIVE_CUSTOM_RACER_ASSET_MODEL_HI, destination, LOAD_QUEUE_CALLBACK_SET_POINTER);
	return 1;
}

internal int NativeCustomRacer_LoadModelNow(int racerIndex, void **storageSlot, void **destination, int modelIDOverride)
{
	if (!NativeCustomRacer_IsRosterEnabled())
		return 0;
	if ((racerIndex < 0) || (racerIndex >= s_nativeCustomRacerCount) || (storageSlot == NULL))
		return 0;

	struct NativeCustomRacerEntry *racer = &s_nativeCustomRacers[racerIndex];
	const u32 size = racer->disk.assets[NATIVE_CUSTOM_RACER_ASSET_MODEL_HI].size;
	if (size == 0)
		return 0;

	void *storage = malloc(size);
	if (storage == NULL)
		return 0;
	if (!NativeCustomRacer_ReadAsset(racer, NATIVE_CUSTOM_RACER_ASSET_MODEL_HI, storage))
	{
		free(storage);
		return 0;
	}

	struct LoadQueueSlot slot = {0};
	slot.ptrBigfileCdPos_UNUSED = &racer->bigfile.header;
	slot.type_UNUSED = LT_DRAM;
	slot.subfileIndex = NATIVE_CUSTOM_RACER_ASSET_MODEL_HI;
	slot.ptrDestination = storage;
	slot.size_UNUSED = size;
	slot.callbackFuncPtr = LOAD_QUEUE_CALLBACK_SET_POINTER;

	const int oldQueueReady = sdata->queueReady;
	LOAD_DramFileCallback(&slot);
	sdata->queueReady = oldQueueReady;

	if (modelIDOverride >= 0)
	{
		struct Model *model = (struct Model *)((u8 *)storage + LOAD_MODEL_FILE_HEADER_BYTES);
		model->id = (s16)modelIDOverride;
	}

	free(*storageSlot);
	*storageSlot = storage;
	if (destination != NULL)
		*destination = storage;
	return 1;
}

int NativeCustomRacer_LoadDriverModelNow(int driverIndex, void **destination)
{
	if ((driverIndex < 0) || (driverIndex >= LOAD_CHARACTER_ID_COUNT))
		return 0;

	const int racerIndex = NativeCustomRacer_GetDriverSelection(driverIndex);
	if (!NativeCustomRacer_LoadModelNow(racerIndex, &s_nativeCustomRacerDriverModelStorage[driverIndex], destination, -1))
		return 0;

	printf("[CTR Native] Loaded custom racer %s immediately for driver %d size=%u\n",
	       s_nativeCustomRacers[racerIndex].disk.name, driverIndex,
	       s_nativeCustomRacers[racerIndex].disk.assets[NATIVE_CUSTOM_RACER_ASSET_MODEL_HI].size);
	return 1;
}

int NativeCustomRacer_LoadPodiumModelNow(int podiumRank, int danceModelID, void **destination)
{
	if ((podiumRank < 0) || (podiumRank >= NATIVE_CUSTOM_RACER_PODIUM_COUNT) || (danceModelID <= 0))
		return 0;

	const int racerIndex = NativeCustomRacer_GetPodiumSelection(podiumRank);
	if (!NativeCustomRacer_LoadModelNow(racerIndex, &s_nativeCustomRacerPodiumModelStorage[podiumRank], destination, danceModelID))
		return 0;

	printf("[CTR Native] Loaded custom racer %s for podium rank %d model=%d\n",
	       s_nativeCustomRacers[racerIndex].disk.name, podiumRank, danceModelID);
	return 1;
}

int NativeCustomRacer_QueueSelectedModel(int playerIndex, void **destination)
{
	if ((playerIndex < 0) || (playerIndex >= 4))
		return 0;
	return NativeCustomRacer_QueueDriverModel(playerIndex, destination);
}

struct Model *NativeCustomRacer_GetLoadedDriverModel(int driverIndex)
{
	if ((driverIndex < 0) || (driverIndex >= LOAD_CHARACTER_ID_COUNT) || (s_nativeCustomRacerDriverModelStorage[driverIndex] == NULL))
		return NULL;
	return (struct Model *)((u8 *)s_nativeCustomRacerDriverModelStorage[driverIndex] + LOAD_MODEL_FILE_HEADER_BYTES);
}

struct Model *NativeCustomRacer_GetLoadedPlayerModel(int playerIndex)
{
	if ((playerIndex < 0) || (playerIndex >= 4))
		return NULL;
	return NativeCustomRacer_GetLoadedDriverModel(playerIndex);
}

void NativeCustomRacer_QueueSharedVramForSelections(struct BigHeader *retailBigfile)
{
	int racerIndex = -1;
	if (NativeCustomRacer_IsRosterEnabled() && sdata != NULL && sdata->gGT != NULL)
	{
		for (int playerIndex = 0; playerIndex < sdata->gGT->numPlyrNextGame; playerIndex++)
		{
			const int selected = NativeCustomRacer_GetPlayerSelection(playerIndex);
			if ((selected >= 0) && (selected < s_nativeCustomRacerCount) &&
			    (s_nativeCustomRacers[selected].disk.assets[NATIVE_CUSTOM_RACER_ASSET_SHARED_VRM].size != 0))
			{
				racerIndex = selected;
				break;
			}
		}
	}

	if (racerIndex >= 0)
	{
		if (s_nativeCustomRacerSharedVram != racerIndex)
		{
			if (s_nativeCustomRacerSharedVram < 0)
				NativeCustomRacer_CacheRetailTemplatePortraits();

			LOAD_AppendQueue(&s_nativeCustomRacers[racerIndex].bigfile.header, LT_VRAM,
			                 NATIVE_CUSTOM_RACER_ASSET_SHARED_VRM, NULL, NULL);
			s_nativeCustomRacerSharedVram = (s16)racerIndex;
		}
	}
	else if ((s_nativeCustomRacerSharedVram >= 0) && (retailBigfile != NULL))
	{
		LOAD_AppendQueue(retailBigfile, LT_VRAM, BI_SHAREDMPKVRM, NULL, NULL);
		s_nativeCustomRacerSharedVram = -1;
	}
}

internal void NativeCustomRacer_ApplyVramPatches(u64 selectedMask, int uploadRetailBase, int restoreRetailPortraits)
{
	if (!NativeCustomRacer_IsRosterEnabled() || (s_nativeCustomRacerRetailSharedVram == NULL) ||
	    (s_nativeCustomRacerRetailSharedVramSize < sizeof(struct VramHeader)) || (selectedMask == 0))
	{
		return;
	}

	const size_t vramWordCount = (size_t)VRAM_WIDTH * (size_t)VRAM_HEIGHT;
	u16 *retailVram = (u16 *)calloc(vramWordCount, sizeof(u16));
	u16 *customVram = (u16 *)calloc(vramWordCount, sizeof(u16));
	u16 patchRow[VRAM_WIDTH];
	if ((retailVram == NULL) || (customVram == NULL) ||
	    !NativeCustomRacer_ApplyVramFileToBuffer(s_nativeCustomRacerRetailSharedVram,
	                                             s_nativeCustomRacerRetailSharedVramSize, retailVram) ||
	    (uploadRetailBase && !NativeCustomRacer_UploadVramFile(s_nativeCustomRacerRetailSharedVram, s_nativeCustomRacerRetailSharedVramSize)))
	{
		free(customVram);
		free(retailVram);
		return;
	}

	for (int racerIndex = 0; racerIndex < s_nativeCustomRacerCount; racerIndex++)
	{
		if ((selectedMask & ((u64)1 << racerIndex)) == 0)
			continue;

		struct NativeCustomRacerEntry *racer = &s_nativeCustomRacers[racerIndex];
		const u32 vrmSize = racer->disk.assets[NATIVE_CUSTOM_RACER_ASSET_SHARED_VRM].size;
		u8 *vrm = (u8 *)malloc(vrmSize);
		memset(customVram, 0, vramWordCount * sizeof(u16));
		if ((vrm == NULL) || !NativeCustomRacer_ReadAsset(racer, NATIVE_CUSTOM_RACER_ASSET_SHARED_VRM, vrm) ||
		    !NativeCustomRacer_ApplyVramFileToBuffer(vrm, vrmSize, customVram))
		{
			free(vrm);
			continue;
		}

		for (int y = 0; y < VRAM_HEIGHT; y++)
		{
			int x = 0;
			while (x < VRAM_WIDTH)
			{
				const size_t rowOffset = (size_t)y * VRAM_WIDTH;
				while ((x < VRAM_WIDTH) && (customVram[rowOffset + x] == retailVram[rowOffset + x]))
					x++;
				if (x >= VRAM_WIDTH)
					break;

				const int startX = x;
				while ((x < VRAM_WIDTH) && (customVram[rowOffset + x] != retailVram[rowOffset + x]))
					x++;

				RECT16 rect = {(s16)startX, (s16)y, (s16)(x - startX), 1};
				memcpy(patchRow, &customVram[rowOffset + startX], (size_t)rect.w * sizeof(u16));
				LoadImage(&rect, (u32 *)patchRow);
			}
		}
		free(vrm);
	}

	if (restoreRetailPortraits)
		NativeCustomRacer_RestoreRetailTemplatePortraitsVram();
	free(customVram);
	free(retailVram);
}

void NativeCustomRacer_ApplyDriverVramPatches(void)
{
	u64 selectedMask = 0;
	for (int driverIndex = 0; driverIndex < LOAD_CHARACTER_ID_COUNT; driverIndex++)
	{
		const int racerIndex = NativeCustomRacer_GetDriverSelection(driverIndex);
		if ((racerIndex >= 0) && (racerIndex < s_nativeCustomRacerCount) &&
		    (s_nativeCustomRacers[racerIndex].disk.assets[NATIVE_CUSTOM_RACER_ASSET_SHARED_VRM].size != 0))
		{
			selectedMask |= (u64)1 << racerIndex;
		}
	}
	NativeCustomRacer_ApplyVramPatches(selectedMask, 1, 1);
}

void NativeCustomRacer_ApplyPodiumVramPatches(void)
{
	u64 selectedMask = 0;
	for (int podiumRank = 0; podiumRank < NATIVE_CUSTOM_RACER_PODIUM_COUNT; podiumRank++)
	{
		const int racerIndex = NativeCustomRacer_GetPodiumSelection(podiumRank);
		if ((racerIndex >= 0) && (racerIndex < s_nativeCustomRacerCount) &&
		    (s_nativeCustomRacers[racerIndex].disk.assets[NATIVE_CUSTOM_RACER_ASSET_SHARED_VRM].size != 0))
		{
			selectedMask |= (u64)1 << racerIndex;
		}
	}
	NativeCustomRacer_ApplyVramPatches(selectedMask, 0, 0);
}

void NativeCustomRacer_ClearPlayerSelections(void)
{
	for (int i = 0; i < 4; i++)
		s_nativeCustomRacerDriverSelection[i] = -1;
}

void NativeCustomRacer_SetPlayerSelection(int playerIndex, int racerIndex)
{
	if (playerIndex < 0 || playerIndex >= 4)
		return;
	NativeCustomRacer_SetDriverSelection(playerIndex, racerIndex);
}

int NativeCustomRacer_GetPlayerSelection(int playerIndex)
{
	return (playerIndex >= 0 && playerIndex < 4) ? NativeCustomRacer_GetDriverSelection(playerIndex) : -1;
}

void NativeCustomRacer_ClearDriverSelections(void)
{
	for (int i = 0; i < LOAD_CHARACTER_ID_COUNT; i++)
		s_nativeCustomRacerDriverSelection[i] = -1;
}

void NativeCustomRacer_SetDriverSelection(int driverIndex, int racerIndex)
{
	if (driverIndex < 0 || driverIndex >= LOAD_CHARACTER_ID_COUNT)
		return;
	s_nativeCustomRacerDriverSelection[driverIndex] =
		(racerIndex >= 0 && racerIndex < s_nativeCustomRacerCount) ? (s16)racerIndex : -1;
}

int NativeCustomRacer_GetDriverSelection(int driverIndex)
{
	return (driverIndex >= 0 && driverIndex < LOAD_CHARACTER_ID_COUNT) ? s_nativeCustomRacerDriverSelection[driverIndex] : -1;
}

void NativeCustomRacer_ClearPodiumSelections(void)
{
	for (int podiumRank = 0; podiumRank < NATIVE_CUSTOM_RACER_PODIUM_COUNT; podiumRank++)
	{
		s_nativeCustomRacerPodiumSelection[podiumRank] = -1;
		free(s_nativeCustomRacerPodiumModelStorage[podiumRank]);
		s_nativeCustomRacerPodiumModelStorage[podiumRank] = NULL;
	}
}

void NativeCustomRacer_SetPodiumSelection(int podiumRank, int racerIndex)
{
	if ((podiumRank < 0) || (podiumRank >= NATIVE_CUSTOM_RACER_PODIUM_COUNT))
		return;
	s_nativeCustomRacerPodiumSelection[podiumRank] =
		(racerIndex >= 0 && racerIndex < s_nativeCustomRacerCount) ? (s16)racerIndex : -1;
}

int NativeCustomRacer_GetPodiumSelection(int podiumRank)
{
	return (podiumRank >= 0 && podiumRank < NATIVE_CUSTOM_RACER_PODIUM_COUNT) ? s_nativeCustomRacerPodiumSelection[podiumRank] : -1;
}
