#include <macros.h>
#include <platform/native_assets.h>
#include <platform/native_cd.h>
#include <platform/native_disc_image.h>
#include <platform/native_path.h>
#include <psx/libcd.h>

#include <SDL3/SDL.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

// NOTE(aalhendi): Native exports the retail Cd* API. Extracted host files stay
// as dev/modding overrides; missing files can fall back to assets/ctr-u.bin.

#define NATIVE_CD_SECTOR_WORDS   512
#define NATIVE_CD_MAX_OPEN_FILES 8
#define NATIVE_CD_SECTOR_SIZE    0x800

enum NativeCDFileSource
{
	NATIVE_CD_FILE_NONE,
	NATIVE_CD_FILE_HOST,
	NATIVE_CD_FILE_DISC,
};

struct NativeCDOpenFile
{
	int source;
	FILE *hostFile;
	struct NativeDiscImageFile discFile;
};

struct NativeCDReadWorker
{
	SDL_Mutex *mutex;
	SDL_Condition *condition;
	SDL_Thread *thread;
	b32 quit;
	b32 pending;
	b32 busy;
	b32 complete;
	b32 success;
	s32 fileIndex;
	s32 firstSector;
	s32 sectorCount;
	void *destination;
	CdlCB callback;
};

global_variable s32 s_cdDebugLevel;
global_variable s32 s_cdLastCom;
global_variable CdlCB s_cdReadyCallback;
global_variable CdlCB s_cdSyncCallback;
global_variable u32 s_cdSectorData[NATIVE_CD_SECTOR_WORDS];
global_variable CdlCB s_nativeCdReadCallback;
global_variable struct NativeCDOpenFile s_nativeCdFiles[NATIVE_CD_MAX_OPEN_FILES];
global_variable s32 s_nativeCdFileCount;
global_variable s32 s_nativeCdCurrentFile;
global_variable s32 s_nativeCdCurrentSector;
global_variable struct NativeCDReadWorker s_nativeCdReadWorker;

int boolDecodeXaDuringVsyncCallback;

internal s32 NativeCD_NormalizeFilename(char *dst, s32 dstCount, const char *src)
{
	NativeStr8 filename = NativeStr8_FromCString(src);
	size_t i;

	if ((dstCount <= 0) || (src == NULL))
	{
		return 0;
	}

	for (i = 0; i < filename.len; i++)
	{
		if (filename.ptr[i] == ';')
		{
			filename.len = i;
			break;
		}
	}

	return NativePath_NormalizeSlashes(dst, (size_t)dstCount, filename);
}

internal NativeStr8 NativeCD_PathAfterRoot(NativeStr8 filename)
{
	if ((filename.len != 0) && NativePath_IsSeparator(filename.ptr[0]))
	{
		return NativeStr8_Skip(filename, 1);
	}

	return filename;
}

internal s32 NativeCD_OpenFile(const char *filename, s32 *outSize)
{
	char normalized[256];
	char rootlessPath[256];
	NativeStr8 rootless;
	FILE *file;
	struct NativeDiscImageFile discFile;
	s32 fileIndex;
	long fileSize;

	if (s_nativeCdFileCount >= NATIVE_CD_MAX_OPEN_FILES)
	{
		return -1;
	}

	if (NativeCD_NormalizeFilename(normalized, sizeof(normalized), filename) == 0)
	{
		return -1;
	}

	rootless = NativeCD_PathAfterRoot(NativeStr8_FromCString(normalized));

	file = NativeAssets_OpenHostStr8(rootless, "rb");
	if ((file == NULL) && NativeStr8_CopyToCString(rootlessPath, sizeof(rootlessPath), rootless))
	{
		file = fopen(rootlessPath, "rb");
	}

	if (file == NULL)
	{
		if (!NativeStr8_CopyToCString(rootlessPath, sizeof(rootlessPath), rootless) || !NativeDiscImage_FindFile(rootlessPath, &discFile))
		{
			return -1;
		}

		fileIndex = s_nativeCdFileCount++;
		memset(&s_nativeCdFiles[fileIndex], 0, sizeof(s_nativeCdFiles[fileIndex]));
		s_nativeCdFiles[fileIndex].source = NATIVE_CD_FILE_DISC;
		s_nativeCdFiles[fileIndex].discFile = discFile;
		*outSize = (s32)discFile.size;
		return fileIndex;
	}

	if (fseek(file, 0, SEEK_END) != 0)
	{
		fclose(file);
		return -1;
	}

	fileSize = ftell(file);
	if (fileSize < 0)
	{
		fclose(file);
		return -1;
	}

	if (fseek(file, 0, SEEK_SET) != 0)
	{
		fclose(file);
		return -1;
	}

	fileIndex = s_nativeCdFileCount++;
	memset(&s_nativeCdFiles[fileIndex], 0, sizeof(s_nativeCdFiles[fileIndex]));
	s_nativeCdFiles[fileIndex].source = NATIVE_CD_FILE_HOST;
	s_nativeCdFiles[fileIndex].hostFile = file;
	*outSize = (s32)fileSize;
	return fileIndex;
}

internal s32 NativeCD_SearchFile(CdlFILE *loc, const char *filename)
{
	char normalized[256];
	char rootlessPath[256];
	NativeStr8 rootless;
	s32 fileIndex;
	s32 fileSize;
	s32 encodedPos;

	if ((loc == NULL) || (filename == NULL))
	{
		return 0;
	}

	fileIndex = NativeCD_OpenFile(filename, &fileSize);
	if (fileIndex < 0)
	{
		return 0;
	}

	if (NativeCD_NormalizeFilename(normalized, sizeof(normalized), filename) == 0)
	{
		return 0;
	}

	rootless = NativeCD_PathAfterRoot(NativeStr8_FromCString(normalized));
	if (!NativeStr8_CopyToCString(rootlessPath, sizeof(rootlessPath), rootless))
	{
		return 0;
	}

	encodedPos = fileIndex << 24;

	memcpy(&loc->pos, &encodedPos, sizeof(encodedPos));
	loc->size = fileSize;
	memset(loc->name, 0, sizeof(loc->name));
	size_t nameLen = rootless.len;
	if (nameLen >= sizeof(loc->name))
	{
		nameLen = sizeof(loc->name) - 1;
	}
	memcpy(loc->name, rootless.ptr, nameLen);

	return 1;
}

internal s32 NativeCD_PosToInt(const CdlLOC *pos)
{
	s32 value;

	memcpy(&value, pos, sizeof(value));
	return value;
}

internal void NativeCD_IntToPos(s32 value, CdlLOC *pos)
{
	memcpy(pos, &value, sizeof(value));
}

internal s32 NativeCD_SetLoc(const CdlLOC *pos)
{
	s32 encodedPos;
	s32 fileIndex;
	s32 sector;

	encodedPos = NativeCD_PosToInt(pos);
	fileIndex = (encodedPos >> 24) & 0xff;
	sector = encodedPos & 0xffffff;

	if ((fileIndex < 0) || (fileIndex >= s_nativeCdFileCount) || (s_nativeCdFiles[fileIndex].source == NATIVE_CD_FILE_NONE))
	{
		return 0;
	}

	s_nativeCdCurrentFile = fileIndex;
	s_nativeCdCurrentSector = sector;
	return 1;
}

internal s32 NativeCD_ReadSectorsAt(s32 fileIndex, s32 firstSector, s32 sectors, void *dst)
{
	u64 byteOffset;
	size_t byteCount;

	if ((fileIndex < 0) || (fileIndex >= s_nativeCdFileCount) || (s_nativeCdFiles[fileIndex].source == NATIVE_CD_FILE_NONE) || (firstSector < 0) ||
	    (sectors <= 0) || (dst == NULL))
	{
		return 0;
	}

	byteCount = (size_t)sectors * NATIVE_CD_SECTOR_SIZE;

	if (s_nativeCdFiles[fileIndex].source == NATIVE_CD_FILE_DISC)
	{
		return NativeDiscImage_ReadDataSectors(&s_nativeCdFiles[fileIndex].discFile, (u32)firstSector, (u32)sectors, dst);
	}

	byteOffset = (u64)(u32)firstSector * NATIVE_CD_SECTOR_SIZE;
	if ((byteOffset > (u64)LONG_MAX) || (fseek(s_nativeCdFiles[fileIndex].hostFile, (long)byteOffset, SEEK_SET) != 0))
	{
		return 0;
	}

	return fread(dst, 1, byteCount, s_nativeCdFiles[fileIndex].hostFile) == byteCount;
}

internal int SDLCALL NativeCD_ReadWorkerThread(void *unused)
{
	(void)unused;
	SDL_SetCurrentThreadPriority(SDL_THREAD_PRIORITY_LOW);

	for (;;)
	{
		s32 fileIndex;
		s32 firstSector;
		s32 sectorCount;
		void *destination;
		b32 success;

		SDL_LockMutex(s_nativeCdReadWorker.mutex);
		while (!s_nativeCdReadWorker.quit && !s_nativeCdReadWorker.pending)
		{
			SDL_WaitCondition(s_nativeCdReadWorker.condition, s_nativeCdReadWorker.mutex);
		}

		if (s_nativeCdReadWorker.quit)
		{
			SDL_UnlockMutex(s_nativeCdReadWorker.mutex);
			break;
		}

		fileIndex = s_nativeCdReadWorker.fileIndex;
		firstSector = s_nativeCdReadWorker.firstSector;
		sectorCount = s_nativeCdReadWorker.sectorCount;
		destination = s_nativeCdReadWorker.destination;
		s_nativeCdReadWorker.pending = 0;
		s_nativeCdReadWorker.busy = 1;
		SDL_UnlockMutex(s_nativeCdReadWorker.mutex);

		success = NativeCD_ReadSectorsAt(fileIndex, firstSector, sectorCount, destination);

		SDL_LockMutex(s_nativeCdReadWorker.mutex);
		s_nativeCdReadWorker.busy = 0;
		s_nativeCdReadWorker.success = success;
		s_nativeCdReadWorker.complete = 1;
		SDL_SignalCondition(s_nativeCdReadWorker.condition);
		SDL_UnlockMutex(s_nativeCdReadWorker.mutex);
	}

	return 0;
}

internal s32 NativeCD_ReadWorkerInit(void)
{
	memset(&s_nativeCdReadWorker, 0, sizeof(s_nativeCdReadWorker));
	s_nativeCdReadWorker.mutex = SDL_CreateMutex();
	s_nativeCdReadWorker.condition = SDL_CreateCondition();
	if ((s_nativeCdReadWorker.mutex == NULL) || (s_nativeCdReadWorker.condition == NULL))
	{
		SDL_DestroyCondition(s_nativeCdReadWorker.condition);
		SDL_DestroyMutex(s_nativeCdReadWorker.mutex);
		memset(&s_nativeCdReadWorker, 0, sizeof(s_nativeCdReadWorker));
		return 0;
	}

	s_nativeCdReadWorker.thread = SDL_CreateThread(NativeCD_ReadWorkerThread, "CTR CD Reader", NULL);
	if (s_nativeCdReadWorker.thread == NULL)
	{
		SDL_DestroyCondition(s_nativeCdReadWorker.condition);
		SDL_DestroyMutex(s_nativeCdReadWorker.mutex);
		memset(&s_nativeCdReadWorker, 0, sizeof(s_nativeCdReadWorker));
		return 0;
	}

	return 1;
}

void NativeCD_Shutdown(void)
{
	s32 i;

	if (s_nativeCdReadWorker.mutex != NULL)
	{
		SDL_LockMutex(s_nativeCdReadWorker.mutex);
		s_nativeCdReadWorker.quit = 1;
		SDL_SignalCondition(s_nativeCdReadWorker.condition);
		SDL_UnlockMutex(s_nativeCdReadWorker.mutex);
		SDL_WaitThread(s_nativeCdReadWorker.thread, NULL);
		SDL_DestroyCondition(s_nativeCdReadWorker.condition);
		SDL_DestroyMutex(s_nativeCdReadWorker.mutex);
		memset(&s_nativeCdReadWorker, 0, sizeof(s_nativeCdReadWorker));
	}

	for (i = 0; i < s_nativeCdFileCount; i++)
	{
		if (s_nativeCdFiles[i].hostFile != NULL)
		{
			fclose(s_nativeCdFiles[i].hostFile);
		}
	}

	memset(s_nativeCdFiles, 0, sizeof(s_nativeCdFiles));
	s_nativeCdFileCount = 0;
	s_nativeCdCurrentFile = -1;
	s_nativeCdCurrentSector = 0;
}

void NativeCD_PumpCallbacks(void)
{
	CdlCB callback = NULL;
	b32 success = 0;

	if (s_nativeCdReadWorker.mutex == NULL)
	{
		return;
	}

	SDL_LockMutex(s_nativeCdReadWorker.mutex);
	if (s_nativeCdReadWorker.complete)
	{
		success = s_nativeCdReadWorker.success;
		callback = s_nativeCdReadWorker.callback;
		s_nativeCdReadWorker.complete = 0;
		s_nativeCdReadWorker.callback = NULL;
	}
	SDL_UnlockMutex(s_nativeCdReadWorker.mutex);

	if (callback != NULL)
	{
		callback(success ? CdlComplete : CdlDiskError, NULL);
	}
}

internal void NativeCD_SetLastCom(int com)
{
	s_cdLastCom = com;
}

int NativeCD_Init(void)
{
	NativeCD_Shutdown();

	s_cdDebugLevel = 0;
	s_cdLastCom = 0;
	s_cdReadyCallback = NULL;
	s_cdSyncCallback = NULL;
	s_nativeCdReadCallback = NULL;
	memset(s_nativeCdFiles, 0, sizeof(s_nativeCdFiles));
	s_nativeCdFileCount = 0;
	s_nativeCdCurrentFile = -1;
	s_nativeCdCurrentSector = 0;
	memset(s_cdSectorData, 0, sizeof(s_cdSectorData));
	NativeCD_ReadWorkerInit();
	return 0;
}

int CdInit(void)
{
	s_cdLastCom = 0;
	s_nativeCdCurrentFile = -1;
	s_nativeCdCurrentSector = 0;
	memset(s_cdSectorData, 0, sizeof(s_cdSectorData));
	return 1;
}

int CdSetDebug(int level)
{
	s32 old = s_cdDebugLevel;
	s_cdDebugLevel = level;
	return old;
}

int CdLastCom(void)
{
	return s_cdLastCom;
}

CdlCB CdReadyCallback(CdlCB func)
{
	CdlCB old = s_cdReadyCallback;
	s_cdReadyCallback = func;
	return old;
}

CdlCB CdSyncCallback(CdlCB func)
{
	CdlCB old = s_cdSyncCallback;
	s_cdSyncCallback = func;
	return old;
}

int CdGetSector(void *madr, int size)
{
	u32 byteCount;

	if ((madr == NULL) || (size <= 0))
	{
		return 1;
	}

	byteCount = (u32)size * sizeof(u32);
	if (byteCount > sizeof(s_cdSectorData))
	{
		byteCount = sizeof(s_cdSectorData);
	}

	memcpy(madr, s_cdSectorData, byteCount);
	return 1;
}

CdlCB CdReadCallback(CdlCB func)
{
	CdlCB old;

	if (s_nativeCdReadWorker.mutex != NULL)
	{
		SDL_LockMutex(s_nativeCdReadWorker.mutex);
	}
	old = s_nativeCdReadCallback;
	s_nativeCdReadCallback = func;
	if (s_nativeCdReadWorker.mutex != NULL)
	{
		SDL_UnlockMutex(s_nativeCdReadWorker.mutex);
	}
	return old;
}

int CdPosToInt(CdlLOC *p)
{
	return NativeCD_PosToInt(p);
}

CdlLOC *CdIntToPos(int val, CdlLOC *p)
{
	NativeCD_IntToPos(val, p);
	return p;
}

CdlFILE *CdSearchFile(CdlFILE *loc, char *filename)
{
	if (NativeCD_SearchFile(loc, filename) == 0)
	{
		return NULL;
	}

	return loc;
}

int CdControl(uint8_t com, uint8_t *param, uint8_t *result)
{
	(void)result;

	NativeCD_SetLastCom(com);

	if ((com == CdlSetloc) && (param != NULL))
	{
		if (s_nativeCdReadWorker.mutex != NULL)
		{
			SDL_LockMutex(s_nativeCdReadWorker.mutex);
		}
		NativeCD_SetLoc((const CdlLOC *)param);
		if (s_nativeCdReadWorker.mutex != NULL)
		{
			SDL_UnlockMutex(s_nativeCdReadWorker.mutex);
		}
	}

	if ((com == CdlSetmode) && (param != NULL))
	{
		if (param[0] == 0xE8)
		{
			boolDecodeXaDuringVsyncCallback = 1;
		}

		if (param[0] == CdlModeSpeed)
		{
			boolDecodeXaDuringVsyncCallback = 0;
		}
	}

	return 1;
}

int CdRead(int sectors, uint32_t *buf, int mode)
{
	s32 fileIndex;
	s32 firstSector;
	CdlCB callback;
	b32 success;

	(void)mode;

	if ((sectors <= 0) || (buf == NULL))
	{
		return 0;
	}

	if (s_nativeCdReadWorker.mutex == NULL)
	{
		success = NativeCD_ReadSectorsAt(s_nativeCdCurrentFile, s_nativeCdCurrentSector, sectors, buf);
		if (success)
		{
			s_nativeCdCurrentSector += sectors;
		}
		if (s_nativeCdReadCallback != NULL)
		{
			s_nativeCdReadCallback(success ? CdlComplete : CdlDiskError, NULL);
		}
		return success;
	}

	SDL_LockMutex(s_nativeCdReadWorker.mutex);
	if (s_nativeCdReadWorker.pending || s_nativeCdReadWorker.busy || s_nativeCdReadWorker.complete)
	{
		SDL_UnlockMutex(s_nativeCdReadWorker.mutex);
		return 0;
	}

	fileIndex = s_nativeCdCurrentFile;
	firstSector = s_nativeCdCurrentSector;
	callback = s_nativeCdReadCallback;
	if ((fileIndex < 0) || (fileIndex >= s_nativeCdFileCount) || (s_nativeCdFiles[fileIndex].source == NATIVE_CD_FILE_NONE))
	{
		SDL_UnlockMutex(s_nativeCdReadWorker.mutex);
		return 0;
	}

	s_nativeCdCurrentSector += sectors;
	s_nativeCdReadWorker.fileIndex = fileIndex;
	s_nativeCdReadWorker.firstSector = firstSector;
	s_nativeCdReadWorker.sectorCount = sectors;
	s_nativeCdReadWorker.destination = buf;
	s_nativeCdReadWorker.callback = callback;
	s_nativeCdReadWorker.pending = 1;
	SDL_SignalCondition(s_nativeCdReadWorker.condition);
	SDL_UnlockMutex(s_nativeCdReadWorker.mutex);
	return 1;
}

int CdReadSync(int mode, uint8_t *result)
{
	b32 success;
	CdlCB callback;

	if (s_nativeCdReadWorker.mutex == NULL)
	{
		return 0;
	}

	SDL_LockMutex(s_nativeCdReadWorker.mutex);
	if (mode != 0)
	{
		int busy = s_nativeCdReadWorker.pending || s_nativeCdReadWorker.busy;
		SDL_UnlockMutex(s_nativeCdReadWorker.mutex);
		return busy;
	}

	while (s_nativeCdReadWorker.pending || s_nativeCdReadWorker.busy)
	{
		SDL_WaitCondition(s_nativeCdReadWorker.condition, s_nativeCdReadWorker.mutex);
	}

	if (!s_nativeCdReadWorker.complete)
	{
		SDL_UnlockMutex(s_nativeCdReadWorker.mutex);
		return 0;
	}

	success = s_nativeCdReadWorker.success;
	callback = s_nativeCdReadWorker.callback;
	s_nativeCdReadWorker.complete = 0;
	s_nativeCdReadWorker.callback = NULL;
	SDL_UnlockMutex(s_nativeCdReadWorker.mutex);

	if (result != NULL)
	{
		result[0] = success ? CdlComplete : CdlDiskError;
	}
	if (callback != NULL)
	{
		callback(success ? CdlComplete : CdlDiskError, NULL);
	}

	return success ? 0 : CdlDiskError;
}
