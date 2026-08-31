/*
 * Derived from REDRIVER2/PsyCross MIT source:
 * externals/PsyCross/src/psx/LIBGPU.C
 * See THIRD_PARTY_NOTICES.md for copyright and license details.
 */

#include <psx/libgte.h>
#include <psx/libetc.h>
#include <psx/libgpu.h>

#include <platform/native_renderer.h>
#include <platform/native_gpu.h>
#include <platform/native_perf.h>
#include <gpu.h>
#include <platform.h>

#include <string.h>

// NOTE(aalhendi): Native libgpu preserves the retail-shaped PsyQ GPU facade
// while keeping the game-facing GPU symbols under ctr-native ownership. The
// primitive parser and GL backend live behind this native platform boundary.

int g_dbg_emulatorPaused = 0;
void (*drawsync_callback)(void) = NULL;

#ifdef __vita__
typedef struct
{
	RECT16 rect;
	u8 r;
	u8 g;
	u8 b;
} NativeLibGpuClearTask;

typedef struct
{
	RECT16 rect;
	u16 *pixels;
	int dstX;
	int dstY;
} NativeLibGpuVramTask;

internal void NativeLibGpu_BackendClearTask(void *arg)
{
	NativeLibGpuClearTask *task = (NativeLibGpuClearTask *)arg;
	NativeRenderer_ClearVRAM(task->rect.x, task->rect.y, task->rect.w, task->rect.h, task->r, task->g, task->b);
	NativeRenderer_Clear(task->rect.x, task->rect.y, task->rect.w, task->rect.h, task->r, task->g, task->b);
}

internal void NativeLibGpu_BackendLoadTask(void *arg)
{
	NativeLibGpuVramTask *task = (NativeLibGpuVramTask *)arg;
	NativeRenderer_CopyVRAM(task->pixels, 0, 0, task->rect.w, task->rect.h, task->rect.x, task->rect.y);
}

internal void NativeLibGpu_BackendMoveTask(void *arg)
{
	NativeLibGpuVramTask *task = (NativeLibGpuVramTask *)arg;
	NativeRenderer_CopyVRAM(NULL, task->rect.x, task->rect.y, task->rect.w, task->rect.h, task->dstX, task->dstY);
}

internal void NativeLibGpu_BackendReadTask(void *arg)
{
	NativeLibGpuVramTask *task = (NativeLibGpuVramTask *)arg;
	NativeRenderer_ReadVRAM(task->pixels, task->rect.x, task->rect.y, task->rect.w, task->rect.h);
}

internal void NativeLibGpu_BackendUpdateVRAMTask(void *arg)
{
	(void)arg;
	NativeRenderer_UpdateVRAM();
}
#endif

int ClearImage(RECT16 *rect, uint8_t r, uint8_t g, uint8_t b)
{
#ifdef __vita__
	NativeLibGpuClearTask task = {*rect, r, g, b};
	NativeGpu_RunBackendTaskSync(NativeLibGpu_BackendClearTask, &task);
#else
	NativeRenderer_ClearVRAM(rect->x, rect->y, rect->w, rect->h, r, g, b);
	NativeRenderer_Clear(rect->x, rect->y, rect->w, rect->h, r, g, b);
#endif
	return 0;
}

int ClearImage2(RECT16 *rect, uint8_t r, uint8_t g, uint8_t b)
{
	return ClearImage(rect, r, g, b);
}

int DrawSync(int mode)
{
	(void)mode;

#ifdef __vita__
	NativeGpu_SyncBackend();
	if (NativeGpu_HasPendingSplits())
	{
		NativeGpu_ForceSynchronousFrame();
		NativeGpu_FlushFrontendSplitsSync();
	}
#else
	NativeRenderer_UpdateVRAM();
#endif
#ifndef __vita__
	if (NativeGpu_HasPendingSplits())
	{
		DrawAllSplits();
	}
#endif
	// NOTE(penta3): Real PS1 DrawSync only waits for the GPU; it never copies the
	// framebuffer back into VRAM. We do the same: no per-frame readback here. The
	// on-demand consumers that actually sample the framebuffer pull it when needed
	// (StoreImage/ElimBG pause grab, MoveImage, save-state capture).

	if (drawsync_callback != NULL)
	{
		drawsync_callback();
	}

	return 0;
}

int LoadImage(RECT16 *rect, void *p)
{
#ifdef __vita__
	NativeLibGpuVramTask task = {*rect, (u16 *)p, 0, 0};
	NativeGpu_RunBackendTaskSync(NativeLibGpu_BackendLoadTask, &task);
#else
	NativeRenderer_CopyVRAM((unsigned short *)p, 0, 0, rect->w, rect->h, rect->x, rect->y);
#endif
	return 0;
}

int LoadImage2(RECT16 *rect, void *p)
{
	LoadImage(rect, p);
#ifdef __vita__
	NativeGpu_RunBackendTaskSync(NativeLibGpu_BackendUpdateVRAMTask, NULL);
#else
	NativeRenderer_UpdateVRAM();
#endif
	return 0;
}

int MoveImage(RECT16 *rect, int x, int y)
{
#ifdef __vita__
	NativeLibGpuVramTask task = {*rect, NULL, x, y};
	NativeGpu_RunBackendTaskSync(NativeLibGpu_BackendMoveTask, &task);
#else
	NativeRenderer_CopyVRAM(NULL, rect->x, rect->y, rect->w, rect->h, x, y);
#endif
	return 0;
}

int StoreImage(RECT16 *rect, uint32_t *p)
{
#ifdef __vita__
	NativeLibGpuVramTask task = {*rect, (u16 *)p, 0, 0};
	NativeGpu_RunBackendTaskSync(NativeLibGpu_BackendReadTask, &task);
#else
	NativeRenderer_ReadVRAM((unsigned short *)p, rect->x, rect->y, rect->w, rect->h);
#endif
	return 0;
}

int StoreImage2(RECT16 *rect, uint32_t *p)
{
	return StoreImage(rect, p);
}

int ResetGraph(int mode)
{
	if (mode == 0)
	{
		g_GPUDisabledState = 0;
		ClearImage(&activeDrawEnv.clip, 0, 0, 0);
		ClearSplits();
#ifdef __vita__
		NativeGpu_FinishSynchronousFrame();
#else
		Platform_EndScene();
#endif
	}
	else if (mode == 1)
	{
		ClearSplits();
#ifdef __vita__
		NativeGpu_FinishSynchronousFrame();
#else
		Platform_EndScene();
#endif
	}

	return 0;
}

int SetGraphDebug(int level)
{
	(void)level;
	return 0;
}

uint32_t *ClearOTag(uint32_t *ot, int n)
{
	OT_TAG *ptag_list;

	if (n == 0)
	{
		return NULL;
	}

	ptag_list = (OT_TAG *)ot;

	termPrim(&ptag_list[n - 1]);
	setlen(&ptag_list[n - 1], 0);

	for (int i = n - 2; i >= 0; --i)
	{
		setaddr(&ptag_list[i], &ptag_list[i + 1]);
		setlen(&ptag_list[i], 0);
	}

	return NULL;
}

uint32_t *ClearOTagR(uint32_t *ot, int n)
{
	OT_TAG *ptag_list;

	if (n == 0)
	{
		return NULL;
	}

	ptag_list = (OT_TAG *)ot;

	termPrim(ptag_list);
	setlen(ptag_list, 0);

	for (int i = 1; i < n; ++i)
	{
		setaddr(&ptag_list[i], &ptag_list[i - 1]);
		setlen(&ptag_list[i], 0);
	}

	return NULL;
}

void SetDispMask(int mask)
{
	g_GPUDisabledState = (mask == 0);
}

DISPENV *GetDispEnv(DISPENV *env)
{
	memcpy(env, &activeDispEnv, sizeof(DISPENV));
	return env;
}

DISPENV *PutDispEnv(DISPENV *env)
{
	memcpy(&activeDispEnv, env, sizeof(DISPENV));
	NativeGpu_SetFrontendDispEnv(env);
	return 0;
}

DISPENV *SetDefDispEnv(DISPENV *env, int x, int y, int w, int h)
{
	env->disp.x = x;
	env->disp.y = y;
	env->disp.w = w;
	env->disp.h = h;

	env->screen.x = 0;
	env->screen.y = 0;
	env->screen.w = 0;
	env->screen.h = 0;

	env->isrgb24 = 0;
	env->isinter = 0;
	env->pad1 = 0;
	env->pad0 = 0;

	return 0;
}

DRAWENV *PutDrawEnv(DRAWENV *env)
{
	memcpy(&activeDrawEnv, env, sizeof(DRAWENV));
	NativeGpu_SetFrontendDrawEnv(env);
	return 0;
}

DRAWENV *SetDefDrawEnv(DRAWENV *env, int x, int y, int w, int h)
{
	env->clip.x = x;
	env->clip.y = y;
	env->clip.w = w;
	env->clip.h = h;

	env->tw.x = 0;
	env->tw.y = 0;
	env->tw.w = 0;
	env->tw.h = 0;
	env->r0 = 0;
	env->g0 = 0;
	env->b0 = 0;
	env->dtd = 1;

	if (GetVideoMode() == MODE_NTSC)
	{
		env->dfe = h < 289 ? 1 : 0;
	}
	else
	{
		env->dfe = h < 257 ? 1 : 0;
	}

	env->ofs[0] = x;
	env->ofs[1] = y;
	env->tpage = 10;
	env->isbg = 0;

	return env;
}

void SetDrawEnv(DR_ENV *dr_env, DRAWENV *env)
{
	dr_env->code[0] = ((env->clip.y & 0x3FF) << 10) | (env->clip.x & 0x3FF) | 0xE3000000;
	dr_env->code[1] = (((env->clip.y + env->clip.h - 1) & 0x3FF) << 10) | ((env->clip.x + env->clip.w - 1) & 0x3FF) | 0xE4000000;
	dr_env->code[2] = ((env->ofs[1] & 0x7FF) << 11) | (env->ofs[0] & 0x7FF) | 0xE5000000;
	dr_env->code[3] = 32 * (((256 - env->tw.h) >> 3) & 0x1F) | (((256 - env->tw.w) >> 3) & 0x1F) | (((env->tw.y >> 3) & 0x1F) << 15) |
	                  (((env->tw.x >> 3) & 0x1F) << 10) | 0xE2000000;
	dr_env->code[4] = ((env->dtd != 0) << 9) | ((env->dfe != 0) << 10) | (env->tpage & 0x1FF) | 0xE1000000;

	setlen(dr_env, 5);
}

void SetDrawMove(DR_MOVE *p, RECT16 *rect, int x, int y)
{
	char len = 5;

	if (rect->w == 0 || rect->h == 0)
	{
		len = 0;
	}

	p->code[0] = 0x1000000;
	p->code[1] = 0x80000000;
	p->code[2] = ((u32)(u16)rect->x) | ((u32)(u16)rect->y << 16);
	p->code[3] = (y << 0x10) | (x & 0xffffU);
	p->code[4] = ((u32)(u16)rect->w) | ((u32)(u16)rect->h << 16);

	setlen(p, len);
}

void SetPsyXTexture(DR_PSYX_TEX *p, uint32_t grTextureId, int width, int height)
{
	p->code[0] = 0xB1000000u | (grTextureId & 0x00ffffffu);
	p->code[1] = ((uint32_t)(height & 0x0fff) << 16) | (uint32_t)(width & 0x0fff);
	setlen(p, 2);
}

uint32_t DrawSyncCallback(void (*func)(void))
{
	drawsync_callback = func;
	return 0;
}

void DrawOTag(void *p)
{
	NativePerf_BeginScope(NATIVE_PERF_BUCKET_DRAW_OTAG);
	do
	{
		if (g_GPUDisabledState)
		{
			ClearSplits();
			NativePerf_EndScope(NATIVE_PERF_BUCKET_DRAW_OTAG);
			return;
		}

#ifdef __vita__
		if (NativeGpu_IsFrontendFrameActive())
		{
			ParsePrimitivesLinkedList((uint32_t *)p, 0);
			if (NativeGpu_IsSynchronousFrame() && NativeGpu_HasPendingSplits())
			{
				NativeGpu_FlushFrontendSplitsSync();
			}
		}
		else
		{
			NativeGpu_SyncBackend();
			const int newScene = !NativeGpu_IsSynchronousFrame();
			NativeGpu_ForceSynchronousFrame();
			if (newScene)
			{
				NativeGpu_ResetOrderDepth();
				ClearSplits();
			}
			ParsePrimitivesLinkedList((uint32_t *)p, 0);
			NativeGpu_FlushFrontendSplitsSync();
		}
#else
		if (Platform_BeginScene())
		{
			NativeGpu_ResetOrderDepth();
			ClearSplits();
		}

		ParsePrimitivesLinkedList((uint32_t *)p, 0);
		DrawAllSplits();
#endif
	} while (g_dbg_emulatorPaused);
	NativePerf_EndScope(NATIVE_PERF_BUCKET_DRAW_OTAG);
}

void DrawPrim(void *p)
{
	if (g_GPUDisabledState)
	{
		ClearSplits();
		return;
	}

#ifdef __vita__
	NativeGpu_SyncBackend();
	NativeGpu_ForceSynchronousFrame();
#else
	if (Platform_BeginScene())
	{
		NativeGpu_ResetOrderDepth();
		ClearSplits();
	}
#endif

	ParsePrimitivesLinkedList((uint32_t *)p, 1);
#ifdef __vita__
	NativeGpu_FlushFrontendSplitsSync();
#endif
}

void AddPrim(void *ot, void *p)
{
	u32 otTag = CTR_GPU_ReadTagWord(ot);
	u32 primTag = CTR_GPU_ReadTagWord(p);

	CTR_GPU_WriteTagWord(p, CtrGpu_PackOTTag(otTag, primTag & 0xff000000u));
	CTR_GPU_WriteTagWord(ot, CtrGpu_PrimToOTLink24(p));
}
