#define _CRT_SECURE_NO_WARNINGS
#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <io.h>
#include "platform/native_win32.h"
#else
#include <unistd.h>
#endif

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#define _EnterCriticalSection(x)
#define EnterCriticalSection(x)
#define ExitCriticalSection()

#include "platform/native_assets.h"
#include "platform/native_log.h"
#include "platform/native_memory.h"
#include "platform/native_perf.h"
#include "platform/native_replay_scheduler.h"
#include "platform/native_savestate.h"
#include "platform/native_adhoc.h"
#include "platform/native_leaderboard.h"
#include "platform/native_network.h"
#include "platform/native_user_id.h"

#ifdef __vita__
#include <vitasdk.h>
#include <dirent.h>
int _newlib_heap_size_user = 256 * 1024 * 1024;
#if 0
int __real_mkdir(const char *fname, mode_t mode);
int __wrap_mkdir(const char *fname, mode_t mode) {
	sceClibPrintf("mkdir %s\n", fname);
	char patched_fname[256];
	sprintf(patched_fname, "ux0:data/ctr/%s", fname);
	return __real_mkdir(patched_fname, mode);
}
FILE *__real_fopen(char *fname, char *mode);
FILE *__wrap_fopen(char *fname, char *mode) {
	sceClibPrintf("fopen %s\n", fname);
	char patched_fname[256];
	sprintf(patched_fname, "ux0:data/ctr/%s", fname);
	return __real_fopen(patched_fname, mode);
}
int __real_unlink(const char *fname);
int __wrap_unlink(const char *fname) {
	sceClibPrintf("unlink %s\n", fname);
	char patched_fname[256];
	sprintf(patched_fname, "ux0:data/ctr/%s", fname);
	return __real_unlink(patched_fname);
}
DIR *__real_opendir(const char *fname);
DIR *__wrap_opendir(const char *fname) {
	sceClibPrintf("opendir %s\n", fname);
	char patched_fname[256];
	sprintf(patched_fname, "ux0:data/ctr/%s", fname);
	return __real_opendir(patched_fname);
}
#endif
#endif

#include <platform.h>

int gNativeRelicRaceMode = 0;
int gNativeRelicRaceResultTier = -1;

#include "game/game_unity.h"

#include "game/zGlobal_RDATA.c"
#include "game/zGlobal_DATA.c"
#include "game/zGlobal_SDATA.c"

#undef RECT

#include "platform/native_disc_image.c"
#include "platform/native_assets.c"
#include "platform/native_audio.c"
#include "platform/native_memory.c"
#include "platform/native_checkpoint.c"
#include "platform/native_checkpoint_file.c"
#include "platform/native_cd.c"
#include "platform/native_gpu_links.c"
#include "platform/native_gpu.c"
#include "platform/native_gte_core.c"
#include "platform/native_glad.c"
#include "platform/native_input.c"
#include "platform/native_network.c"
#include "platform/native_pc_account.c"
#include "platform/native_user_id.c"
#include "platform/native_leaderboard.c"
#include "platform/native_adhoc.c"
#include "platform/native_inline_c.c"
#include "platform/native_libapi.c"
#include "platform/native_libetc.c"
#include "platform/native_libgte.c"
#include "platform/native_libgpu.c"
#include "platform/native_libpad.c"
#include "platform/native_libspu.c"
#include "platform/native_log.c"
#include "platform/native_memcard.c"
#include "platform/native_memcard_adapter.c"
#include "platform/native_perf.c"
#include "platform/native_platform.c"
#include "platform/native_replay_scheduler.c"
#include "platform/native_renderer.c"
#include "platform/native_savestate.c"
#include "platform/native_state.c"
#include "platform/native_str.c"

#ifndef CC
#if defined(__GNUC__)
#if _WIN32
#ifndef __clang__
#define CC "MINGW-GCC"
#else
#define CC "MINGW-CLANG"
#endif
#else
#ifndef __clang__
#define CC "GCC"
#else
#define CC "CLANG"
#endif
#endif
#elif defined(_MSC_VER)
#define CC "MSVC"
#else
#define CC "Unknown"
#endif
#endif

#ifndef CTR_NATIVE_VERSION
#define CTR_NATIVE_VERSION "0.0.0-dev"
#endif

#ifndef CTR_NATIVE_BUILD_ID
#define CTR_NATIVE_BUILD_ID "unknown"
#endif

static int NativeConsole_ShouldPauseOnError(void)
{
#if defined(_WIN32)
	DWORD consoleProcesses[2];
	DWORD consoleProcessCount;

	if (GetConsoleWindow() == NULL)
		return 0;

	consoleProcessCount = GetConsoleProcessList(consoleProcesses, (DWORD)(sizeof(consoleProcesses) / sizeof(consoleProcesses[0])));
	return (consoleProcessCount == 1) && (consoleProcesses[0] == GetCurrentProcessId());
#else
	return 0;
#endif
}

static s32 NativeConsole_Return(const u32 result)
{
	if ((result != 0) && NativeConsole_ShouldPauseOnError())
	{
		fflush(stdout);
		fflush(stderr);
		fprintf(stderr, "\n[CTR Native] Press Enter to close this window...");
		fflush(stderr);

		while (getchar() != '\n' && !feof(stdin))
		{
		}
	}

	return (s32)result;
}

// TODO(aalhendi): just make an argparser?
static int NativeArg_IsVersion(const char *arg)
{
	return (arg != NULL) && ((strcmp(arg, "--version") == 0) || (strcmp(arg, "-v") == 0));
}

extern s32 s_nativeLanguageChosen; // Flag if language has been selected on first boot
extern int gNativeMirrorModeEnabled;
int gNative60FpsEnabled = 0;
int gNativeForce30Fps = 0;
#ifndef __vita__
int gNativeAntiAliasingEnabled = 1;
int gNativeBorderlessEnabled = 0;
#endif
int cfg_language = 2; // Default: PAL UK language

static const char *NativeConfig_GetPath(void)
{
#ifdef __vita__
	return "ux0:data/ctr/config.ini";
#else
	return "config.ini";
#endif
}

void load_config(void)
{
	char buffer[30];
	int value;
	FILE *config = fopen(NativeConfig_GetPath(), "r");
	if (config)
	{
		while (EOF != fscanf(config, "%[^=]=%d\n", buffer, &value))
		{
			if (strcmp("language", buffer) == 0)
			{
				cfg_language = value;
				s_nativeLanguageChosen = 1;
			}
			else if (strcmp("mirror_mode", buffer) == 0)
			{
				gNativeMirrorModeEnabled = (value != 0);
			}
			else if (strcmp("60fps", buffer) == 0)
			{
				gNative60FpsEnabled = (value != 0);
			}
#ifndef __vita__
			else if (strcmp("anti_aliasing", buffer) == 0)
			{
				gNativeAntiAliasingEnabled = (value != 0);
			}
			else if (strcmp("borderless", buffer) == 0)
			{
				gNativeBorderlessEnabled = (value != 0);
			}
#endif
		}
		fclose(config);
	}
}

void save_config(void)
{
	FILE *config = fopen(NativeConfig_GetPath(), "w+");
	if (config != NULL)
	{
		fprintf(config, "%s=%d\n", "language", cfg_language);
		fprintf(config, "%s=%d\n", "mirror_mode", gNativeMirrorModeEnabled != 0);
		fprintf(config, "%s=%d\n", "60fps", gNative60FpsEnabled != 0);
#ifndef __vita__
		fprintf(config, "%s=%d\n", "anti_aliasing", gNativeAntiAliasingEnabled != 0);
		fprintf(config, "%s=%d\n", "borderless", gNativeBorderlessEnabled != 0);
#endif
		fclose(config);
	}
}

#ifdef __vita__
#include <pthread.h>
void *real_main(void *argv);

int main(int argc, char *argv[])
{
	pthread_t t;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 0x400000);
	pthread_create(&t, &attr, real_main, NULL);

	return sceKernelExitDeleteThread(0);
}

void *real_main(void *_argv)
{
	scePowerSetArmClockFrequency(444);
	scePowerSetBusClockFrequency(222);
	scePowerSetGpuClockFrequency(222);
	scePowerSetGpuXbarClockFrequency(166);
	sceAppUtilInit(&(SceAppUtilInitParam){}, &(SceAppUtilBootParam){});
	SceCommonDialogConfigParam cmnDlgCfgParam;
	sceCommonDialogConfigParamInit(&cmnDlgCfgParam);
	sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_LANG, (int *)&cmnDlgCfgParam.language);
	sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_ENTER_BUTTON, (int *)&cmnDlgCfgParam.enterButtonAssign);
	sceCommonDialogSetConfigParam(&cmnDlgCfgParam);
	
	load_config();
	sceIoMkdir("ux0:data/ctr/shader_cache", 0777);
	char **argv = _argv;
	int argc = 0;
#else
int main(int argc, char *argv[])
{
#endif
	for (int argIndex = 1; argIndex < argc; argIndex++)
	{
		if (NativeArg_IsVersion(argv[argIndex]))
		{
			printf("CTR Native %s (%s)\n", CTR_NATIVE_VERSION, CTR_NATIVE_BUILD_ID);
			return 0;
		}
	}

	printf("[CTR Native] Starting...\n");
	fflush(stdout);

#ifdef __vita__
	const char *sdlBasePath = "ux0:data/ctr";
#else
	const char *sdlBasePath = SDL_GetBasePath();
#endif
	printf("[CTR Native] SDL base path: %s\n", sdlBasePath ? sdlBasePath : "(null)");
	fflush(stdout);

	if (!NativeAssets_Init(sdlBasePath))
	{
		fprintf(stderr, "[CTR Native] Failed to initialize asset paths.\n");
		return NativeConsole_Return(1);
	}

	printf("[CTR Native] Version: %s (%s)\n", CTR_NATIVE_VERSION, CTR_NATIVE_BUILD_ID);
	printf("[CTR Native] Built with: " CC "\n");
	printf("[CTR Native] Base: %s\n", NativeAssets_GetBaseDir());
	printf("[CTR Native] Assets: %s\n", NativeAssets_GetAssetDir());
	fflush(stdout);

	if (chdir(NativeAssets_GetBaseDir()) != 0)
	{
		fprintf(stderr, "[CTR Native] Failed to enter base directory: %s\n", NativeAssets_GetBaseDir());
		return NativeConsole_Return(1);
	}

#ifndef __vita__
	load_config();
#endif

	if (!NativeAssets_Validate())
	{
		return NativeConsole_Return(1);
	}

#if defined(CTR_INTERNAL)
	if (NativeReplayScheduler_PrepareReportFromArgs(argc, argv) != 0)
	{
		return NativeConsole_Return(1);
	}
#endif

#if defined(__vita__)
	printf("[CTR Native] High Octane widescreen 960x544\n");
	Platform_Init("Crash Team Racing: High Octane", 960, 544);
#elif CTR_NATIVE_WIDESCREEN
	printf("[CTR Native] High Octane widescreen 1280x720\n");
	Platform_Init("Crash Team Racing: High Octane", 1280, 720);
#elif defined(USE_16BY9)
	printf("[CTR Native] Widescreen\n");
	Platform_Init("Crash Team Racing: High Octane", 1280, 720);
#else
	printf("[CTR Native] 4:3\n");
	Platform_Init("Crash Team Racing: High Octane", 800, 600);
#endif

#if defined(CTR_INTERNAL)
	if (NativePerf_ConfigureFromArgs(argc, argv) != 0)
	{
		Platform_LogFlush();
		Platform_Shutdown();
		return NativeConsole_Return(1);
	}
#endif

	Platform_InitScratchpad();
	Platform_RepairResidentPointers(0);

#if defined(CTR_INTERNAL)
	if (NativeReplayScheduler_ConfigureFromArgs(argc, argv) != 0)
	{
		Platform_LogFlush();
		Platform_Shutdown();
		return NativeConsole_Return(1);
	}
#else
	(void)argc;
	(void)argv;
#endif

	const int result = CTR_Main();

	Platform_Shutdown();
	return NativeConsole_Return(result);
}
