/*
 * Derived from REDRIVER2/PsyCross MIT source:
 * externals/PsyCross/src/gpu/PsyX_GPU.cpp
 * See THIRD_PARTY_NOTICES.md for copyright and license details.
 */

#include <macros.h>
#include "platform/native_gpu.h"

#include <platform.h>

#include <SDL3/SDL.h>

#include "platform/native_log.h"
#include "platform/native_perf.h"
#include "platform/native_renderer.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __vita__
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/threadmgr/lw_cond.h>
#include <psp2/kernel/threadmgr/lw_mutex.h>
#include <vitaGL.h>
#endif

void Platform_PollHostEvents(void);
extern int g_cfg_bilinearFiltering;
extern int g_dbg_emulatorPaused;
extern int g_dbg_polygonSelected;

#define NATIVE_GPU_LOG(fmt, ...)   Platform_Log("[CTR GPU] " fmt, __VA_ARGS__)
#define NATIVE_GPU_ERROR(fmt, ...) Platform_LogError("[CTR GPU] [%s] - " fmt, __func__, __VA_ARGS__)

// NOTE(aalhendi): Little-endian tag `CTRG` = CTR native GPU snapshot.
#define NATIVE_GPU_STATE_MAGIC     0x47525443
#define NATIVE_GPU_STATE_VERSION   1

#define GET_TPAGE_BLEND(tpage)     ((BlendMode)(((tpage >> 5) & 3) + 1))

#define GET_TPAGE_DITHER(tpage)    ((tpage >> 9) & 0x1)

#define GET_CLUT_X(clut)           ((clut & 0x3F) << 4)
#define GET_CLUT_Y(clut)           (clut >> 6)

internal TexFormat GetTPageFormat(int tpage)
{
	const int mode = (tpage >> 7) & 0x3;

	// NOTE(aalhendi): ctr-native local divergence from upstream PsyCross. PS1
	// mode 3 is reserved; TF_32_BIT_RGBA is only for explicit native override
	// textures, not raw retail TPAGE mode bits.
	return mode == 3 ? TF_16_BIT : (TexFormat)mode;
}

internal short GetTPageBase(int tpage)
{
	const u16 page = (u16)tpage;

	// NOTE(aalhendi): ctr-native local divergence for CTR retail emitters. The
	// shader wants xPage + yPage * 16, not raw draw-mode bits.
	return (s16)((page & 0xf) | ((page & 0x10) ? 0x10 : 0));
}

internal s16 NativeGpu_SignExtend11(u32 value)
{
	value &= 0x7ff;
	return (s16)((value ^ 0x400) - 0x400);
}

DISPENV activeDispEnv;
DRAWENV activeDrawEnv;
int g_GPUDisabledState = 0;

typedef struct
{
	DRAWENV drawenv;
	DISPENV dispenv;

	BlendMode blendMode;

	TexFormat texFormat;
	TextureID textureId;

	int drawPrimMode;
#ifdef __vita__
	RECT16 clearRect;
	u8 clearR;
	u8 clearG;
	u8 clearB;
#endif
	bool psxTexturedSemiTrans;
	bool psxTextureOutputSTP;
	bool psxDrawMaskSet;
#ifdef __vita__
	bool psxTextureFullyOpaque;
	bool p4CacheEligible;
	s16 p4Page;
	u16 p4Clut;
#endif
	u8 psxSemiTransPassMask;

	u16 startVertex;
	u16 numVerts;

	const char *debugText;
} GPUDrawSplit;

#define MAX_DRAW_SPLITS 4096

typedef struct
{
	const char *currentSplitDebugText;
	TextureID overrideTexture;
	int overrideTextureWidth;
	int overrideTextureHeight;

	int drawPrimMode;
	bool psxDrawMaskSet;
	bool framebufferFeedbackRunActive;
#ifdef __vita__
	u32 primitiveOrder;
	u16 currentPrimitiveOrder;
#endif

#ifdef __vita__
	GrVertex *vertexBuffer;
	GPUDrawSplit *splits;
#else
	GrVertex vertexBuffer[MAX_VERTEX_BUFFER_SIZE];
	GPUDrawSplit splits[MAX_DRAW_SPLITS];
#endif
	int vertexIndex;
	int splitIndex;
} NativeGpuState;

#ifdef __vita__
typedef struct
{
	GrVertex vertices[MAX_VERTEX_BUFFER_SIZE];
	GPUDrawSplit splits[MAX_DRAW_SPLITS];
	DRAWENV drawEnv;
	DISPENV dispEnv;
	int vertexCount;
	int splitCount;
} NativeGpuFramePacket;

#define NATIVE_GPU_FRAME_PACKET_COUNT 2

global_variable NativeGpuFramePacket s_gpuFramePackets[NATIVE_GPU_FRAME_PACKET_COUNT] __attribute__((aligned(64)));
global_variable int s_gpuFrontendPacketIndex;
global_variable int s_gpuFrontendFramePrepared;
global_variable int s_gpuSynchronousFrame;
global_variable NativeGpuFramePacket *s_gpuBackendPacket;
global_variable NativeGpuBackendTaskFn s_gpuBackendTask;
global_variable void *s_gpuBackendTaskArg;
global_variable DRAWENV s_gpuBackendDrawEnv;
global_variable DISPENV s_gpuBackendDispEnv;
global_variable DRAWENV s_gpuBackendTaskDrawEnv;
global_variable DISPENV s_gpuBackendTaskDispEnv;
global_variable SceKernelLwMutexWork s_gpuBackendMutex;
global_variable SceKernelLwCondWork s_gpuBackendCond;
global_variable SceUID s_gpuBackendThread = -1;
global_variable int s_gpuBackendBusy;
global_variable int s_gpuBackendWorkReady;
global_variable int s_gpuBackendInitComplete;
global_variable volatile int s_gpuBackendExit;
global_variable int s_gpuBackendEnabled;
global_variable int s_gpuBackendInitWidth;
global_variable int s_gpuBackendInitHeight;
global_variable int s_gpuBackendInitSucceeded;
#endif

global_variable GrVertex *s_gpuDrawVertices;
global_variable GPUDrawSplit *s_gpuDrawSplits;
global_variable int s_gpuDrawVertexCount;
global_variable int s_gpuDrawSplitCount;
global_variable NativeGpuState s_gpu;

internal void NativeGpu_DrawPreparedFrame(GrVertex *vertices, GPUDrawSplit *splits, int vertexCount, int splitCount);

void NativeGpu_ResetOrderDepth(void)
{
#ifdef __vita__
	s_gpu.primitiveOrder = 0;
	s_gpu.currentPrimitiveOrder = 0;
#endif
}

#ifdef __vita__
internal void NativeGpu_BindFrontendPacket(void)
{
	NativeGpuFramePacket *packet = &s_gpuFramePackets[s_gpuFrontendPacketIndex];
	s_gpu.vertexBuffer = packet->vertices;
	s_gpu.splits = packet->splits;
}

internal void NativeGpu_BackendLock(void)
{
	sceKernelLockLwMutex(&s_gpuBackendMutex, 1, NULL);
}

internal void NativeGpu_BackendUnlock(void)
{
	sceKernelUnlockLwMutex(&s_gpuBackendMutex, 1);
}

internal void NativeGpu_BackendWaitIdleLocked(void)
{
	while (s_gpuBackendBusy)
	{
		sceKernelWaitLwCond(&s_gpuBackendCond, NULL);
	}
}

internal void NativeGpu_BackendApplyEnv(const DRAWENV *drawEnv, const DISPENV *dispEnv)
{
	s_gpuBackendDrawEnv = *drawEnv;
	s_gpuBackendDispEnv = *dispEnv;
}

typedef struct
{
	GrVertex *vertices;
	GPUDrawSplit *splits;
	int vertexCount;
	int splitCount;
} NativeGpuBackendDrawTask;

internal void NativeGpu_BackendBeginSceneTask(void *arg)
{
	(void)arg;
	Platform_BeginScene();
}

internal void NativeGpu_BackendEndSceneTask(void *arg)
{
	(void)arg;
	Platform_EndScene();
}

internal void NativeGpu_BackendDrawTaskMain(void *arg)
{
	NativeGpuBackendDrawTask *task = (NativeGpuBackendDrawTask *)arg;
	NativeGpu_DrawPreparedFrame(task->vertices, task->splits, task->vertexCount, task->splitCount);
}

typedef struct
{
	int x;
	int y;
	int w;
	int h;
	u8 r;
	u8 g;
	u8 b;
} NativeGpuBackendRectTask;

internal void NativeGpu_BackendStoreFrameBufferTask(void *arg)
{
	NativeGpuBackendRectTask *task = (NativeGpuBackendRectTask *)arg;
	NativeRenderer_StoreFrameBuffer(task->x, task->y, task->w, task->h);
}

internal void NativeGpu_BackendClearTask(void *arg)
{
	NativeGpuBackendRectTask *task = (NativeGpuBackendRectTask *)arg;
	NativeRenderer_Clear(task->x, task->y, task->w, task->h, task->r, task->g, task->b);
}

internal void NativeGpu_BackendSyncVRAMToCPUTask(void *arg)
{
	NativeGpuBackendRectTask *task = (NativeGpuBackendRectTask *)arg;
	NativeRenderer_SyncVRAMToCPU(task->x, task->y, task->w, task->h);
}

internal int NativeGpu_BackendThreadMain(SceSize args, void *argp)
{
	(void)args;
	(void)argp;
	vglSetShaderCachePath("ux0:data/ctr/shader_cache");
	vglUseLowPrecision(GL_TRUE);
	vglSetupRuntimeShaderCompiler(SHARK_OPT_UNSAFE, GL_TRUE, GL_TRUE, GL_TRUE);
	(void)vglInitExtended(0, s_gpuBackendInitWidth, s_gpuBackendInitHeight, 4 * 1024 * 1024, SCE_GXM_MULTISAMPLE_4X);
	NativeGpu_BackendLock();
	s_gpuBackendInitSucceeded = 1;
	s_gpuBackendInitComplete = 1;
	sceKernelSignalLwCondAll(&s_gpuBackendCond);
	for (;;)
	{
		while (!s_gpuBackendWorkReady && !s_gpuBackendExit)
		{
			sceKernelWaitLwCond(&s_gpuBackendCond, NULL);
		}
		if (s_gpuBackendExit)
		{
			break;
		}
		NativeGpuFramePacket *packet = s_gpuBackendPacket;
		NativeGpuBackendTaskFn task = s_gpuBackendTask;
		void *taskArg = s_gpuBackendTaskArg;
		if (packet != NULL)
		{
			NativeGpu_BackendApplyEnv(&packet->drawEnv, &packet->dispEnv);
		}
		else
		{
			NativeGpu_BackendApplyEnv(&s_gpuBackendTaskDrawEnv, &s_gpuBackendTaskDispEnv);
		}
		s_gpuBackendWorkReady = 0;
		NativeGpu_BackendUnlock();
		if (packet != NULL)
		{
			Platform_BeginScene();
			NativeGpu_DrawPreparedFrame(packet->vertices, packet->splits, packet->vertexCount, packet->splitCount);
			Platform_EndScene();
		}
		else if (task != NULL)
		{
			task(taskArg);
		}
		NativeGpu_BackendLock();
		s_gpuBackendPacket = NULL;
		s_gpuBackendTask = NULL;
		s_gpuBackendTaskArg = NULL;
		s_gpuBackendBusy = 0;
		sceKernelSignalLwCondAll(&s_gpuBackendCond);
	}
	NativeGpu_BackendUnlock();
	sceKernelExitThread(0);
	return 0;
}

#endif

int NativeGpu_InitBackend(int width, int height)
{
#ifdef __vita__
	if (s_gpuBackendEnabled)
	{
		return 1;
	}
	s_gpuFrontendPacketIndex = 0;
	s_gpuFrontendFramePrepared = 0;
	s_gpuSynchronousFrame = 0;
	s_gpuBackendPacket = NULL;
	s_gpuBackendTask = NULL;
	s_gpuBackendTaskArg = NULL;
	s_gpuBackendBusy = 0;
	s_gpuBackendWorkReady = 0;
	s_gpuBackendInitComplete = 0;
	s_gpuBackendExit = 0;
	s_gpuBackendInitWidth = width;
	s_gpuBackendInitHeight = height;
	s_gpuBackendInitSucceeded = 0;
	NativeGpu_BindFrontendPacket();
	if (sceKernelCreateLwMutex(&s_gpuBackendMutex, "CTR Render Lock", 0, 0, NULL) < 0)
	{
		return 0;
	}
	if (sceKernelCreateLwCond(&s_gpuBackendCond, "CTR Render Cond", 0, &s_gpuBackendMutex, NULL) < 0)
	{
		sceKernelDeleteLwMutex(&s_gpuBackendMutex);
		return 0;
	}
	s_gpuBackendThread = sceKernelCreateThread("CTR Renderer", NativeGpu_BackendThreadMain, 0x10000100, 0x40000, 0, 0, NULL);
	if (s_gpuBackendThread < 0)
	{
		sceKernelDeleteLwCond(&s_gpuBackendCond);
		sceKernelDeleteLwMutex(&s_gpuBackendMutex);
		return 0;
	}
	if (sceKernelStartThread(s_gpuBackendThread, 0, NULL) < 0)
	{
		sceKernelDeleteThread(s_gpuBackendThread);
		s_gpuBackendThread = -1;
		sceKernelDeleteLwCond(&s_gpuBackendCond);
		sceKernelDeleteLwMutex(&s_gpuBackendMutex);
		return 0;
	}
	NativeGpu_BackendLock();
	while (!s_gpuBackendInitComplete)
	{
		sceKernelWaitLwCond(&s_gpuBackendCond, NULL);
	}
	const int initSucceeded = s_gpuBackendInitSucceeded;
	NativeGpu_BackendUnlock();
	if (!initSucceeded)
	{
		sceKernelWaitThreadEnd(s_gpuBackendThread, NULL, NULL);
		sceKernelDeleteThread(s_gpuBackendThread);
		s_gpuBackendThread = -1;
		sceKernelDeleteLwCond(&s_gpuBackendCond);
		sceKernelDeleteLwMutex(&s_gpuBackendMutex);
		return 0;
	}
	s_gpuBackendEnabled = 1;
	return 1;
#else
	(void)width;
	(void)height;
	return 1;
#endif
}

void NativeGpu_SyncBackend(void)
{
#ifdef __vita__
	if (!s_gpuBackendEnabled)
	{
		return;
	}
	NativeGpu_BackendLock();
	NativeGpu_BackendWaitIdleLocked();
	NativeGpu_BackendUnlock();
#endif
}

void NativeGpu_RunBackendTaskSync(NativeGpuBackendTaskFn task, void *arg)
{
#ifdef __vita__
	if (task == NULL)
	{
		return;
	}
	if (!s_gpuBackendEnabled)
	{
		task(arg);
		return;
	}
	NativeGpu_BackendLock();
	NativeGpu_BackendWaitIdleLocked();
	s_gpuBackendPacket = NULL;
	s_gpuBackendTask = task;
	s_gpuBackendTaskArg = arg;
	s_gpuBackendTaskDrawEnv = activeDrawEnv;
	s_gpuBackendTaskDispEnv = activeDispEnv;
	s_gpuBackendBusy = 1;
	s_gpuBackendWorkReady = 1;
	sceKernelSignalLwCond(&s_gpuBackendCond);
	NativeGpu_BackendWaitIdleLocked();
	NativeGpu_BackendUnlock();
#else
	if (task != NULL)
	{
		task(arg);
	}
#endif
}

void NativeGpu_SyncVRAMToCPU(int x, int y, int w, int h)
{
#ifdef __vita__
	NativeGpuBackendRectTask task = {x, y, w, h, 0, 0, 0};
	NativeGpu_RunBackendTaskSync(NativeGpu_BackendSyncVRAMToCPUTask, &task);
#else
	NativeRenderer_SyncVRAMToCPU(x, y, w, h);
#endif
}

void NativeGpu_ShutdownBackend(void)
{
#ifdef __vita__
	if (!s_gpuBackendEnabled)
	{
		return;
	}
	NativeGpu_BackendLock();
	NativeGpu_BackendWaitIdleLocked();
	s_gpuBackendExit = 1;
	sceKernelSignalLwCond(&s_gpuBackendCond);
	NativeGpu_BackendUnlock();
	sceKernelWaitThreadEnd(s_gpuBackendThread, NULL, NULL);
	sceKernelDeleteThread(s_gpuBackendThread);
	s_gpuBackendThread = -1;
	sceKernelDeleteLwCond(&s_gpuBackendCond);
	sceKernelDeleteLwMutex(&s_gpuBackendMutex);
	s_gpuBackendEnabled = 0;
#endif
}

void NativeGpu_BeginFrontendFrame(void)
{
#ifdef __vita__
	NativeGpu_BindFrontendPacket();
	NativeGpuFramePacket *packet = &s_gpuFramePackets[s_gpuFrontendPacketIndex];
	packet->drawEnv = activeDrawEnv;
	packet->dispEnv = activeDispEnv;
	s_gpuFrontendFramePrepared = 1;
	s_gpuSynchronousFrame = 0;
#endif
	NativeGpu_ResetOrderDepth();
	ClearSplits();
}

void NativeGpu_SetFrontendDrawEnv(const DRAWENV *env)
{
#ifdef __vita__
	if (s_gpuFrontendFramePrepared && env != NULL)
	{
		s_gpuFramePackets[s_gpuFrontendPacketIndex].drawEnv = *env;
	}
#else
	(void)env;
#endif
}

void NativeGpu_SetFrontendDispEnv(const DISPENV *env)
{
#ifdef __vita__
	if (s_gpuFrontendFramePrepared && env != NULL)
	{
		s_gpuFramePackets[s_gpuFrontendPacketIndex].dispEnv = *env;
	}
#else
	(void)env;
#endif
}

const DRAWENV *NativeGpu_GetRenderDrawEnv(void)
{
#ifdef __vita__
	return s_gpuBackendEnabled ? &s_gpuBackendDrawEnv : &activeDrawEnv;
#else
	return &activeDrawEnv;
#endif
}

const DISPENV *NativeGpu_GetRenderDispEnv(void)
{
#ifdef __vita__
	return s_gpuBackendEnabled ? &s_gpuBackendDispEnv : &activeDispEnv;
#else
	return &activeDispEnv;
#endif
}

void NativeGpu_ForceSynchronousFrame(void)
{
#ifdef __vita__
	if (!s_gpuSynchronousFrame)
	{
		NativeGpu_RunBackendTaskSync(NativeGpu_BackendBeginSceneTask, NULL);
		s_gpuSynchronousFrame = 1;
	}
#endif
}

void NativeGpu_FlushFrontendSplitsSync(void)
{
#ifdef __vita__
	if (!NativeGpu_HasPendingSplits())
	{
		return;
	}
	NativeGpuBackendDrawTask task;
	task.vertices = s_gpu.vertexBuffer;
	task.splits = s_gpu.splits;
	task.vertexCount = s_gpu.vertexIndex;
	task.splitCount = s_gpu.splitIndex;
	NativeGpu_RunBackendTaskSync(NativeGpu_BackendDrawTaskMain, &task);
	ClearSplits();
#else
	DrawAllSplits();
#endif
}

void NativeGpu_FinishSynchronousFrame(void)
{
#ifdef __vita__
	if (!s_gpuSynchronousFrame)
	{
		return;
	}
	NativeGpu_FlushFrontendSplitsSync();
	NativeGpu_RunBackendTaskSync(NativeGpu_BackendEndSceneTask, NULL);
	s_gpuSynchronousFrame = 0;
	s_gpuFrontendFramePrepared = 0;
#endif
}

int NativeGpu_IsSynchronousFrame(void)
{
#ifdef __vita__
	return s_gpuSynchronousFrame;
#else
	return 1;
#endif
}

int NativeGpu_IsFrontendFrameActive(void)
{
#ifdef __vita__
	return s_gpuFrontendFramePrepared;
#else
	return 0;
#endif
}

int NativeGpu_SubmitFrontendFrame(void)
{
#ifdef __vita__
	if (!s_gpuFrontendFramePrepared)
	{
		return 0;
	}

	NativeGpuFramePacket *packet = &s_gpuFramePackets[s_gpuFrontendPacketIndex];
	packet->vertexCount = s_gpu.vertexIndex;
	packet->splitCount = s_gpu.splitIndex;

	if (!s_gpuBackendEnabled)
	{
		Platform_BeginScene();
		if (s_gpu.splitIndex > 0)
		{
			DrawAllSplits();
		}
		else
		{
			ClearSplits();
		}
		Platform_EndScene();
		s_gpuFrontendFramePrepared = 0;
		s_gpuSynchronousFrame = 0;
		return 1;
	}

	if (s_gpuSynchronousFrame)
	{
		NativeGpu_FinishSynchronousFrame();
		return 1;
	}

	NativeGpu_BackendLock();
	NativeGpu_BackendWaitIdleLocked();
	s_gpuBackendPacket = packet;
	s_gpuBackendTask = NULL;
	s_gpuBackendTaskArg = NULL;
	s_gpuBackendBusy = 1;
	s_gpuBackendWorkReady = 1;
	s_gpuFrontendPacketIndex ^= 1;
	NativeGpu_BindFrontendPacket();
	ClearSplits();
	s_gpuFrontendFramePrepared = 0;
	s_gpuSynchronousFrame = 0;
	sceKernelSignalLwCond(&s_gpuBackendCond);
	NativeGpu_BackendUnlock();
	return 1;
#else
	return 0;
#endif
}
struct NativeGpuSnapshot
{
	u32 magic;
	u32 version;
	u32 size;
	DRAWENV drawEnv;
	DISPENV dispEnv;
	s32 gpuDisabledState;
	s32 psxDrawMaskSet;
	u16 vram[VRAM_WIDTH * VRAM_HEIGHT];
};

#ifdef __vita__
typedef struct
{
	void *data;
	int size;
	int result;
} NativeGpuBackendVRAMStateTask;

internal void NativeGpu_BackendCaptureVRAMStateTask(void *arg)
{
	NativeGpuBackendVRAMStateTask *task = (NativeGpuBackendVRAMStateTask *)arg;
	task->result = NativeRenderer_CaptureVRAMState(task->data, task->size);
}

internal void NativeGpu_BackendRestoreVRAMStateTask(void *arg)
{
	NativeGpuBackendVRAMStateTask *task = (NativeGpuBackendVRAMStateTask *)arg;
	task->result = NativeRenderer_RestoreVRAMState(task->data, task->size);
}
#endif

int NativeGpu_HasPendingSplits(void)
{
	return s_gpu.splitIndex > 0;
}

internal void NativeGpu_SetVertexOrderDepth(GrVertex *vertex, int count)
{
#ifdef __vita__
	for (int i = 0; i < count; i++)
	{
		vertex[i].orderDepth = s_gpu.currentPrimitiveOrder;
	}
#else
	(void)vertex;
	(void)count;
#endif
}

void ClearSplits(void)
{
	s_gpu.currentSplitDebugText = NULL;
#ifdef __vita__
	if (s_gpu.vertexBuffer == NULL || s_gpu.splits == NULL)
	{
		NativeGpu_BindFrontendPacket();
	}
#endif
	s_gpu.vertexIndex = 0;
	s_gpu.splitIndex = 0;
	s_gpu.splits[0].texFormat = (TexFormat)0xFFFF;
	s_gpu.splits[0].psxTexturedSemiTrans = false;
	s_gpu.splits[0].psxTextureOutputSTP = false;
	s_gpu.splits[0].psxDrawMaskSet = false;
#ifdef __vita__
	s_gpu.splits[0].psxTextureFullyOpaque = false;
	s_gpu.splits[0].p4CacheEligible = false;
	s_gpu.splits[0].p4Page = -1;
	s_gpu.splits[0].p4Clut = 0;
#endif
	s_gpu.splits[0].psxSemiTransPassMask = 0;
	s_gpu.framebufferFeedbackRunActive = false;
}

int NativeGpu_GetStateSize(void)
{
	return (int)sizeof(struct NativeGpuSnapshot);
}

int NativeGpu_CaptureState(void *dst, int dstSize)
{
	struct NativeGpuSnapshot *snapshot = (struct NativeGpuSnapshot *)dst;

	if ((dst == NULL) || (dstSize < (int)sizeof(*snapshot)))
	{
		return 0;
	}
	if (NativeRenderer_GetVRAMStateSize() != (int)sizeof(snapshot->vram))
	{
		return 0;
	}

	memset(snapshot, 0, sizeof(*snapshot));
	snapshot->magic = NATIVE_GPU_STATE_MAGIC;
	snapshot->version = NATIVE_GPU_STATE_VERSION;
	snapshot->size = sizeof(*snapshot);
	snapshot->drawEnv = activeDrawEnv;
	snapshot->dispEnv = activeDispEnv;
	snapshot->gpuDisabledState = g_GPUDisabledState;
	snapshot->psxDrawMaskSet = s_gpu.psxDrawMaskSet;

#ifdef __vita__
	NativeGpuBackendVRAMStateTask task = {snapshot->vram, sizeof(snapshot->vram), 0};
	NativeGpu_RunBackendTaskSync(NativeGpu_BackendCaptureVRAMStateTask, &task);
	return task.result;
#else
	return NativeRenderer_CaptureVRAMState(snapshot->vram, sizeof(snapshot->vram));
#endif
}

int NativeGpu_RestoreState(const void *src, int srcSize)
{
	const struct NativeGpuSnapshot *snapshot = (const struct NativeGpuSnapshot *)src;

	if ((src == NULL) || (srcSize < (int)sizeof(*snapshot)))
	{
		return 0;
	}
	if ((snapshot->magic != NATIVE_GPU_STATE_MAGIC) || (snapshot->version != NATIVE_GPU_STATE_VERSION) || (snapshot->size != sizeof(*snapshot)))
	{
		return 0;
	}
	if ((snapshot->gpuDisabledState < 0) || (snapshot->gpuDisabledState > 1) || (snapshot->psxDrawMaskSet < 0) || (snapshot->psxDrawMaskSet > 1))
	{
		return 0;
	}
	if (NativeRenderer_GetVRAMStateSize() != (int)sizeof(snapshot->vram))
	{
		return 0;
	}
#ifdef __vita__
	NativeGpuBackendVRAMStateTask task = {(void *)snapshot->vram, sizeof(snapshot->vram), 0};
	NativeGpu_RunBackendTaskSync(NativeGpu_BackendRestoreVRAMStateTask, &task);
	if (!task.result)
	{
		return 0;
	}
#else
	if (!NativeRenderer_RestoreVRAMState(snapshot->vram, sizeof(snapshot->vram)))
	{
		return 0;
	}
#endif

	activeDrawEnv = snapshot->drawEnv;
	activeDispEnv = snapshot->dispEnv;
	g_GPUDisabledState = snapshot->gpuDisabledState;
	s_gpu.psxDrawMaskSet = snapshot->psxDrawMaskSet;
	ClearSplits();
	return 1;
}

internal void DrawEnvDimensionsInt(int *width, int *height)
{
	if (activeDrawEnv.dfe)
	{
		*width = activeDispEnv.disp.w;
		*height = activeDispEnv.disp.h;
	}
	else
	{
		*width = activeDrawEnv.clip.w;
		*height = activeDrawEnv.clip.h;
	}
}

void DrawEnvOffset(float *ofsX, float *ofsY)
{
	if (activeDrawEnv.dfe)
	{
		int w, h;
		DrawEnvDimensionsInt(&w, &h);

		if (w <= 0)
		{
			w = 1;
		}

		// NOTE(aalhendi): Convert PS1 VRAM-page draw offsets into display-relative host-screen offsets.
		// CTR alternates draw pages at y=0 and y=0x128; using raw modulo VRAM offsets shifts every other native frame vertically.
		*ofsX = activeDrawEnv.ofs[0] - activeDispEnv.disp.x;
		*ofsY = activeDrawEnv.ofs[1] - activeDispEnv.disp.y;
	}
	else
	{
		*ofsX = activeDrawEnv.ofs[0] - activeDrawEnv.clip.x;
		*ofsY = activeDrawEnv.ofs[1] - activeDrawEnv.clip.y;
	}
}

void LineSwapSourceVerts(VERTTYPE **p0, VERTTYPE **p1, u8 **c0, u8 **c1)
{
	// swap line coordinates for left-to-right and up-to-bottom direction
	if (((*p0)[0] > (*p1)[0]) || ((*p0)[1] > (*p1)[1] && (*p0)[0] == (*p1)[0]))
	{
		VERTTYPE *tmp = *p0;
		*p0 = *p1;
		*p1 = tmp;

		u8 *tmpCol = *c0;
		*c0 = *c1;
		*c1 = tmpCol;
	}
}

void MakeLineArray(GrVertex *vertex, VERTTYPE *p0, VERTTYPE *p1)
{
	const VERTTYPE dx = p1[0] - p0[0];
	const VERTTYPE dy = p1[1] - p0[1];

	float ofsX, ofsY;
	DrawEnvOffset(&ofsX, &ofsY);

	memset(vertex, 0, sizeof(GrVertex) * 4);
	NativeGpu_SetVertexOrderDepth(vertex, 4);

	if (dx > abs((s16)dy))
	{ // horizontal
		vertex[0].x = p0[0] + ofsX;
		vertex[0].y = p0[1] + ofsY;

		vertex[1].x = p1[0] + ofsX + 1;
		vertex[1].y = p1[1] + ofsY;

		vertex[2].x = vertex[1].x;
		vertex[2].y = vertex[1].y + 1;

		vertex[3].x = vertex[0].x;
		vertex[3].y = vertex[0].y + 1;
	}
	else
	{ // vertical
		vertex[0].x = p0[0] + ofsX;
		vertex[0].y = p0[1] + ofsY;

		vertex[1].x = p1[0] + ofsX;
		vertex[1].y = p1[1] + ofsY + 1;

		vertex[2].x = vertex[1].x + 1;
		vertex[2].y = vertex[1].y;

		vertex[3].x = vertex[0].x + 1;
		vertex[3].y = vertex[0].y;
	} // TODO diagonal line alignment
}

void MakeVertexTriangle(GrVertex *vertex, VERTTYPE *p0, VERTTYPE *p1, VERTTYPE *p2)
{
	assert(p0);
	assert(p1);
	assert(p2);

	float ofsX, ofsY;
	DrawEnvOffset(&ofsX, &ofsY);

	memset(vertex, 0, sizeof(GrVertex) * 3);
	NativeGpu_SetVertexOrderDepth(vertex, 3);

	vertex[0].x = p0[0] + ofsX;
	vertex[0].y = p0[1] + ofsY;

	vertex[1].x = p1[0] + ofsX;
	vertex[1].y = p1[1] + ofsY;

	vertex[2].x = p2[0] + ofsX;
	vertex[2].y = p2[1] + ofsY;
}

void MakeVertexQuad(GrVertex *vertex, VERTTYPE *p0, VERTTYPE *p1, VERTTYPE *p2, VERTTYPE *p3)
{
	assert(p0);
	assert(p1);
	assert(p2);
	assert(p3);

	float ofsX, ofsY;
	DrawEnvOffset(&ofsX, &ofsY);

	memset(vertex, 0, sizeof(GrVertex) * 4);
	NativeGpu_SetVertexOrderDepth(vertex, 4);

	vertex[0].x = p0[0] + ofsX;
	vertex[0].y = p0[1] + ofsY;

	vertex[1].x = p1[0] + ofsX;
	vertex[1].y = p1[1] + ofsY;

	vertex[2].x = p2[0] + ofsX;
	vertex[2].y = p2[1] + ofsY;

	vertex[3].x = p3[0] + ofsX;
	vertex[3].y = p3[1] + ofsY;
}

void MakeVertexRect(GrVertex *vertex, VERTTYPE *p0, s16 w, s16 h)
{
	assert(p0);

	float ofsX, ofsY;
	DrawEnvOffset(&ofsX, &ofsY);

	memset(vertex, 0, sizeof(GrVertex) * 4);
	NativeGpu_SetVertexOrderDepth(vertex, 4);

	vertex[0].x = p0[0] + ofsX;
	vertex[0].y = p0[1] + ofsY;

	vertex[1].x = vertex[0].x;
	vertex[1].y = vertex[0].y + h;

	vertex[2].x = vertex[0].x + w;
	vertex[2].y = vertex[0].y + h;

	vertex[3].x = vertex[0].x + w;
	vertex[3].y = vertex[0].y;
}

void MakeTexcoordQuad(GrVertex *vertex, u8 *uv0, u8 *uv1, u8 *uv2, u8 *uv3, s16 page, s16 clut, u8 dither)
{
	assert(uv0);
	assert(uv1);
	assert(uv2);
	assert(uv3);

	const u8 bright = 2;
	const short texPage = GetTPageBase(page);

	vertex[0].u = uv0[0];
	vertex[0].v = uv0[1];
	vertex[0].bright = bright;
	vertex[0].dither = dither;
	vertex[0].page = texPage;
	vertex[0].clut = clut;

	vertex[1].u = uv1[0];
	vertex[1].v = uv1[1];
	vertex[1].bright = bright;
	vertex[1].dither = dither;
	vertex[1].page = texPage;
	vertex[1].clut = clut;

	vertex[2].u = uv2[0];
	vertex[2].v = uv2[1];
	vertex[2].bright = bright;
	vertex[2].dither = dither;
	vertex[2].page = texPage;
	vertex[2].clut = clut;

	vertex[3].u = uv3[0];
	vertex[3].v = uv3[1];
	vertex[3].bright = bright;
	vertex[3].dither = dither;
	vertex[3].page = texPage;
	vertex[3].clut = clut;
	/*
	if (g_cfg_bilinearFiltering)
	{
	    vertex[0].tcx = -1;
	    vertex[0].tcy = -1;

	    vertex[1].tcx = -1;
	    vertex[1].tcy = -1;

	    vertex[2].tcx = -1;
	    vertex[2].tcy = -1;

	    vertex[3].tcx = -1;
	    vertex[3].tcy = -1;
	}*/
}

void MakeTexcoordTriangle(GrVertex *vertex, u8 *uv0, u8 *uv1, u8 *uv2, s16 page, s16 clut, u8 dither)
{
	assert(uv0);
	assert(uv1);
	assert(uv2);

	const u8 bright = 2;
	const short texPage = GetTPageBase(page);

	vertex[0].u = uv0[0];
	vertex[0].v = uv0[1];
	vertex[0].bright = bright;
	vertex[0].dither = dither;
	vertex[0].page = texPage;
	vertex[0].clut = clut;

	vertex[1].u = uv1[0];
	vertex[1].v = uv1[1];
	vertex[1].bright = bright;
	vertex[1].dither = dither;
	vertex[1].page = texPage;
	vertex[1].clut = clut;

	vertex[2].u = uv2[0];
	vertex[2].v = uv2[1];
	vertex[2].bright = bright;
	vertex[2].dither = dither;
	vertex[2].page = texPage;
	vertex[2].clut = clut;
	/*
	if (g_cfg_bilinearFiltering)
	{
	    vertex[0].tcx = -1;
	    vertex[0].tcy = -1;

	    vertex[1].tcx = -1;
	    vertex[1].tcy = -1;

	    vertex[2].tcx = -1;
	    vertex[2].tcy = -1;

	    vertex[3].tcx = -1;
	    vertex[3].tcy = -1;
	}*/
}

void MakeTexcoordRect(GrVertex *vertex, u8 *uv, s16 page, s16 clut, s16 w, s16 h)
{
	assert(uv);

	const u8 bright = 2;
	const u8 dither = 0;
	const short texPage = GetTPageBase(page);
	const int endU = (int)uv[0] + w;
	const int endV = (int)uv[1] + h;
	const bool wrapsU = endU == 256;
	const bool wrapsV = endV == 256;
	const s8 texelCenterOffset = g_cfg_bilinearFiltering ? -1 : 0;
	const u8 encodedEndU = wrapsU ? 255 : (u8)endU;
	const u8 encodedEndV = wrapsV ? 255 : (u8)endV;

	vertex[0].u = uv[0];
	vertex[0].v = uv[1];
	vertex[0].bright = bright;
	vertex[0].dither = dither;
	vertex[0].page = texPage;
	vertex[0].clut = clut;
	vertex[0].tcx = texelCenterOffset;
	vertex[0].tcy = texelCenterOffset;

	vertex[1].u = uv[0];
	vertex[1].v = encodedEndV;
	vertex[1].bright = bright;
	vertex[1].dither = dither;
	vertex[1].page = texPage;
	vertex[1].clut = clut;
	vertex[1].tcx = texelCenterOffset;
	vertex[1].tcy = texelCenterOffset + (wrapsV ? 2 : 0);

	vertex[2].u = encodedEndU;
	vertex[2].v = encodedEndV;
	vertex[2].bright = bright;
	vertex[2].dither = dither;
	vertex[2].page = texPage;
	vertex[2].clut = clut;
	vertex[2].tcx = texelCenterOffset + (wrapsU ? 2 : 0);
	vertex[2].tcy = texelCenterOffset + (wrapsV ? 2 : 0);

	vertex[3].u = encodedEndU;
	vertex[3].v = uv[1];
	vertex[3].bright = bright;
	vertex[3].dither = dither;
	vertex[3].page = texPage;
	vertex[3].clut = clut;
	vertex[3].tcx = texelCenterOffset + (wrapsU ? 2 : 0);
	vertex[3].tcy = texelCenterOffset;
}

void MakeTexcoordLineZero(GrVertex *vertex, u8 dither)
{
	const u8 bright = 1;

	vertex[0].u = 0;
	vertex[0].v = 0;
	vertex[0].bright = bright;
	vertex[0].dither = dither;
	vertex[0].page = 0;
	vertex[0].clut = 0;

	vertex[1].u = 0;
	vertex[1].v = 0;
	vertex[1].bright = bright;
	vertex[1].dither = dither;
	vertex[1].page = 0;
	vertex[1].clut = 0;

	vertex[2].u = 0;
	vertex[2].v = 0;
	vertex[2].bright = bright;
	vertex[2].dither = dither;
	vertex[2].page = 0;
	vertex[2].clut = 0;

	vertex[3].u = 0;
	vertex[3].v = 0;
	vertex[3].bright = bright;
	vertex[3].dither = dither;
	vertex[3].page = 0;
	vertex[3].clut = 0;
}

void MakeTexcoordTriangleZero(GrVertex *vertex, u8 dither)
{
	const u8 bright = 1;

	vertex[0].u = 0;
	vertex[0].v = 0;
	vertex[0].bright = bright;
	vertex[0].dither = dither;
	vertex[0].page = 0;
	vertex[0].clut = 0;

	vertex[1].u = 0;
	vertex[1].v = 0;
	vertex[1].bright = bright;
	vertex[1].dither = dither;
	vertex[1].page = 0;
	vertex[1].clut = 0;

	vertex[2].u = 0;
	vertex[2].v = 0;
	vertex[2].bright = bright;
	vertex[2].dither = dither;
	vertex[2].page = 0;
	vertex[2].clut = 0;
}

void MakeTexcoordQuadZero(GrVertex *vertex, u8 dither)
{
	const u8 bright = 1;

	vertex[0].u = 0;
	vertex[0].v = 0;
	vertex[0].bright = bright;
	vertex[0].dither = dither;
	vertex[0].page = 0;
	vertex[0].clut = 0;

	vertex[1].u = 0;
	vertex[1].v = 0;
	vertex[1].bright = bright;
	vertex[1].dither = dither;
	vertex[1].page = 0;
	vertex[1].clut = 0;

	vertex[2].u = 0;
	vertex[2].v = 0;
	vertex[2].bright = bright;
	vertex[2].dither = dither;
	vertex[2].page = 0;
	vertex[2].clut = 0;

	vertex[3].u = 0;
	vertex[3].v = 0;
	vertex[3].bright = bright;
	vertex[3].dither = dither;
	vertex[3].page = 0;
	vertex[3].clut = 0;
}

void MakeColourNoShade(GrVertex *vertex, int n)
{
	--n;
	while (n >= 0)
	{
		vertex[n].r = 128;
		vertex[n].g = 128;
		vertex[n].b = 128;
		vertex[n].a = 255;
		--n;
	}
}

void MakeColourLine(GrVertex *vertex, bool shadeTexOn, u8 *col0, u8 *col1)
{
	if (!shadeTexOn)
	{
		MakeColourNoShade(vertex, 4);
		return;
	}
	assert(col0);
	assert(col1);

	vertex[0].r = col0[0];
	vertex[0].g = col0[1];
	vertex[0].b = col0[2];
	vertex[0].a = 255;

	vertex[1].r = col1[0];
	vertex[1].g = col1[1];
	vertex[1].b = col1[2];
	vertex[1].a = 255;

	vertex[2].r = col1[0];
	vertex[2].g = col1[1];
	vertex[2].b = col1[2];
	vertex[2].a = 255;

	vertex[3].r = col0[0];
	vertex[3].g = col0[1];
	vertex[3].b = col0[2];
	vertex[3].a = 255;
}

void MakeColourTriangle(GrVertex *vertex, bool shadeTexOn, u8 *col0, u8 *col1, u8 *col2)
{
	if (!shadeTexOn)
	{
		MakeColourNoShade(vertex, 3);
		return;
	}

	assert(col0);
	assert(col1);
	assert(col2);

	vertex[0].r = col0[0];
	vertex[0].g = col0[1];
	vertex[0].b = col0[2];
	vertex[0].a = 255;

	vertex[1].r = col1[0];
	vertex[1].g = col1[1];
	vertex[1].b = col1[2];
	vertex[1].a = 255;

	vertex[2].r = col2[0];
	vertex[2].g = col2[1];
	vertex[2].b = col2[2];
	vertex[2].a = 255;
}

void MakeColourQuad(GrVertex *vertex, bool shadeTexOn, u8 *col0, u8 *col1, u8 *col2, u8 *col3)
{
	if (!shadeTexOn)
	{
		MakeColourNoShade(vertex, 4);
		return;
	}

	assert(col0);
	assert(col1);
	assert(col2);
	assert(col3);

	vertex[0].r = col0[0];
	vertex[0].g = col0[1];
	vertex[0].b = col0[2];
	vertex[0].a = 255;

	vertex[1].r = col1[0];
	vertex[1].g = col1[1];
	vertex[1].b = col1[2];
	vertex[1].a = 255;

	vertex[2].r = col2[0];
	vertex[2].g = col2[1];
	vertex[2].b = col2[2];
	vertex[2].a = 255;

	vertex[3].r = col3[0];
	vertex[3].g = col3[1];
	vertex[3].b = col3[2];
	vertex[3].a = 255;
}

internal void TriangulateQuadVertices(GrVertex *vertex)
{
	/*
	Triangulate like this:

	v0--v1
	|  / |
	| /  |
	v2--v3

	NOTE: v2 swapped with v3 during primitive parsing but it not shown here
	*/

	vertex[4] = vertex[3];
	vertex[5] = vertex[2];
	vertex[2] = vertex[3];
	vertex[3] = vertex[1];
}

void TriangulateQuad()
{
	TriangulateQuadVertices(&s_gpu.vertexBuffer[s_gpu.vertexIndex]);
}

internal int NativeGpu_EmitTexturedSprite(VERTTYPE *position, u8 *uv, s16 page, s16 clut, s16 width, s16 height, bool shadeTexOn, u8 *color)
{
	int emittedVertices = 0;
	int yOffset = 0;

	while (yOffset < height)
	{
		const int texV = ((int)uv[1] + yOffset) & 255;
		const int remainingH = height - yOffset;
		const int partH = remainingH < 256 - texV ? remainingH : 256 - texV;
		int xOffset = 0;

		while (xOffset < width)
		{
			const int texU = ((int)uv[0] + xOffset) & 255;
			const int remainingW = width - xOffset;
			const int partW = remainingW < 256 - texU ? remainingW : 256 - texU;
			VERTTYPE partPosition[2] = {(VERTTYPE)(position[0] + xOffset), (VERTTYPE)(position[1] + yOffset)};
			u8 partUv[2] = {(u8)texU, (u8)texV};
			GrVertex *firstVertex = &s_gpu.vertexBuffer[s_gpu.vertexIndex + emittedVertices];

			MakeVertexRect(firstVertex, partPosition, (s16)partW, (s16)partH);
			MakeTexcoordRect(firstVertex, partUv, page, clut, (s16)partW, (s16)partH);
			MakeColourQuad(firstVertex, shadeTexOn, color, color, color, color);
			TriangulateQuadVertices(firstVertex);
			emittedVertices += 6;
			xOffset += partW;
		}

		yOffset += partH;
	}

	return emittedVertices;
}

//------------------------------------------------------------------------------------------------------------------------

internal bool NativeGpu_RectOverlaps(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh)
{
	return (aw > 0) && (ah > 0) && (bw > 0) && (bh > 0) && (ax < bx + bw) && (bx < ax + aw) && (ay < by + bh) && (by < ay + ah);
}

internal bool NativeGpu_TPageOverlapsActiveDrawPage(int tpage)
{
	const int pageX = (tpage & 0xf) << 6;
	const int pageY = (tpage & 0x10) ? 0x100 : 0;
	const int pageW = 0x100;
	const int pageH = 0x100;

	if (!activeDrawEnv.dfe)
	{
		return false;
	}

	if (GetTPageFormat(tpage) != TF_16_BIT)
	{
		return false;
	}

	return NativeGpu_RectOverlaps(pageX, pageY, pageW, pageH, activeDrawEnv.clip.x, activeDrawEnv.clip.y, activeDrawEnv.clip.w, activeDrawEnv.clip.h);
}

internal void NativeGpu_PrepareFramebufferFeedback(int tpage)
{
	if (!NativeGpu_TPageOverlapsActiveDrawPage(tpage))
	{
		return;
	}

	if (s_gpu.framebufferFeedbackRunActive)
	{
		return;
	}

	// NOTE(aalhendi): PS1 can draw into VRAM and immediately texture from that
	// same draw page. Native batches primitives, so screen-feedback effects
	// like heat warp need an explicit barrier before their framebuffer-sampling
	// polygons consume the VRAM texture.
#ifdef __vita__
	NativeGpu_ForceSynchronousFrame();
	NativeGpu_FlushFrontendSplitsSync();
#else
	if (NativeGpu_HasPendingSplits())
	{
		DrawAllSplits();
	}
#endif

#ifdef __vita__
	NativeGpuBackendRectTask task = {activeDrawEnv.clip.x, activeDrawEnv.clip.y, activeDrawEnv.clip.w, activeDrawEnv.clip.h, 0, 0, 0};
	NativeGpu_RunBackendTaskSync(NativeGpu_BackendStoreFrameBufferTask, &task);
#else
	NativeRenderer_StoreFrameBuffer(activeDrawEnv.clip.x, activeDrawEnv.clip.y, activeDrawEnv.clip.w, activeDrawEnv.clip.h);
#endif
	s_gpu.framebufferFeedbackRunActive = true;
}

#ifdef __vita__
internal bool NativeGpu_AppendClearSplit(const RECT16 *rect, u8 r, u8 g, u8 b)
{
	GPUDrawSplit *current = &s_gpu.splits[s_gpu.splitIndex];
	current->numVerts = s_gpu.vertexIndex - current->startVertex;
	if (s_gpu.splitIndex + 1 >= MAX_DRAW_SPLITS)
	{
		return false;
	}
	GPUDrawSplit *split = &s_gpu.splits[++s_gpu.splitIndex];
	memset(split, 0, sizeof(*split));
	split->drawPrimMode = 2;
	split->drawenv = activeDrawEnv;
	split->dispenv = activeDispEnv;
	split->clearRect = *rect;
	split->clearR = r;
	split->clearG = g;
	split->clearB = b;
	split->startVertex = s_gpu.vertexIndex;
	return true;
}
#endif

internal void AddSplit(bool semiTrans, bool textured, bool framebufferFeedback, s16 clut)
{
	int tpage = activeDrawEnv.tpage;

#ifdef __vita__
	if (s_gpu.primitiveOrder < 0xffffu)
	{
		s_gpu.primitiveOrder++;
	}
	s_gpu.currentPrimitiveOrder = (u16)s_gpu.primitiveOrder;
#endif

	if (framebufferFeedback)
	{
		NativeGpu_PrepareFramebufferFeedback(tpage);
	}
	else
	{
		s_gpu.framebufferFeedbackRunActive = false;
	}

	GPUDrawSplit *curSplit = &s_gpu.splits[s_gpu.splitIndex];

	BlendMode blendMode = semiTrans ? GET_TPAGE_BLEND(tpage) : BM_NONE;
	TexFormat texFormat = GetTPageFormat(tpage);
	TextureID textureId = textured ? NativeRenderer_GetVRAMTexture() : NativeRenderer_GetWhiteTexture();
	bool psxTexturedSemiTrans = semiTrans && textured && s_gpu.overrideTexture == 0;
	// NOTE(aalhendi): PS1 framebuffer bit 15 follows sampled texture STP for
	// textured draws unless E6 forces it. Recursive screen-copy effects depend
	// on this bit surviving after the blended textured pass.
	bool psxTextureOutputSTP = textured && s_gpu.overrideTexture == 0;

	if (textured && s_gpu.overrideTexture != 0)
	{
		// override texture format, zero tpage
		texFormat = TF_32_BIT_RGBA;
		textureId = s_gpu.overrideTexture;
		psxTexturedSemiTrans = false;
	}

#ifdef __vita__
	const bool p4CacheEligible = textured && s_gpu.overrideTexture == 0 && texFormat == TF_4_BIT && !framebufferFeedback;
	const s16 p4Page = p4CacheEligible ? GetTPageBase(tpage) : -1;
	const u16 p4Clut = p4CacheEligible ? (u16)clut : 0;
#endif

	// FIXME: compare drawing environment too?
	if (!psxTexturedSemiTrans && curSplit->blendMode == blendMode && curSplit->texFormat == texFormat && curSplit->textureId == textureId &&
	    curSplit->drawPrimMode == s_gpu.drawPrimMode && curSplit->psxTexturedSemiTrans == psxTexturedSemiTrans &&
	    curSplit->psxTextureOutputSTP == psxTextureOutputSTP && curSplit->psxDrawMaskSet == s_gpu.psxDrawMaskSet &&
#ifdef __vita__
	    curSplit->p4CacheEligible == p4CacheEligible &&
	    (!p4CacheEligible || (curSplit->p4Page == p4Page && curSplit->p4Clut == p4Clut)) &&
#endif
	    curSplit->drawenv.clip.x == activeDrawEnv.clip.x && curSplit->drawenv.clip.y == activeDrawEnv.clip.y &&
	    curSplit->drawenv.clip.w == activeDrawEnv.clip.w && curSplit->drawenv.clip.h == activeDrawEnv.clip.h && curSplit->drawenv.dfe == activeDrawEnv.dfe &&
	    curSplit->debugText == s_gpu.currentSplitDebugText)
	{
		return;
	}

	curSplit->numVerts = s_gpu.vertexIndex - curSplit->startVertex;

	if (s_gpu.splitIndex + 1 >= MAX_DRAW_SPLITS)
	{
		NATIVE_GPU_ERROR("%s\n", "MAX_DRAW_SPLITS reached (too many blend modes, texture formats, drawEnv clip rects, dfe switches), expect rendering errors");
		return;
	}

	GPUDrawSplit *split = &s_gpu.splits[++s_gpu.splitIndex];
	split->blendMode = blendMode;
	split->texFormat = texFormat;
	split->textureId = textureId;
	split->drawPrimMode = s_gpu.drawPrimMode;
	split->psxTexturedSemiTrans = psxTexturedSemiTrans;
	split->psxTextureOutputSTP = psxTextureOutputSTP;
	split->psxDrawMaskSet = s_gpu.psxDrawMaskSet;
#ifdef __vita__
	split->psxTextureFullyOpaque = false;
	split->p4CacheEligible = p4CacheEligible;
	split->p4Page = p4Page;
	split->p4Clut = p4Clut;
#endif
	split->psxSemiTransPassMask = psxTexturedSemiTrans ? 3 : 0;
	split->drawenv = activeDrawEnv;
	split->dispenv = activeDispEnv;
	split->debugText = s_gpu.currentSplitDebugText;

	split->drawenv.tw.w = s_gpu.overrideTextureWidth;
	split->drawenv.tw.h = s_gpu.overrideTextureHeight;

	split->startVertex = s_gpu.vertexIndex;
	split->numVerts = 0;
}

internal void NativeGpu_SetSplitShaderState(const GPUDrawSplit *split, int semiTransPass, BlendMode blendMode, bool offscreen)
{
	TextureID texture = split->textureId;
	int cachedP4 = 0;
#ifdef __vita__
	if (split->p4CacheEligible)
	{
		const TextureID cachedTexture = NativeRenderer_GetCachedP4Texture(split->p4Page, split->p4Clut);
		if (cachedTexture != 0)
		{
			texture = cachedTexture;
			cachedP4 = 1;
		}
	}
#endif
	if (semiTransPass == 3)
	{
		NativeRenderer_SetMixedSTPBlendMode(blendMode);
	}
	else
	{
		NativeRenderer_SetBlendMode(blendMode);
	}
	NativeRenderer_SetTexture(texture, split->texFormat, semiTransPass, blendMode,
	                          texture != NativeRenderer_GetWhiteTexture(),
#ifdef __vita__
	                          split->psxTextureFullyOpaque,
#else
	                          false,
#endif
	                          cachedP4);
	if (split->texFormat == TF_32_BIT_RGBA)
	{
		NativeRenderer_SetOverrideTextureSize(split->drawenv.tw.w, split->drawenv.tw.h);
	}
	NativeRenderer_SetPSXDrawMaskSet(split->psxDrawMaskSet);
	NativeRenderer_SetPSXTextureOutputSTP(split->psxTextureOutputSTP);
	NativeRenderer_SetProjection(&split->drawenv.clip, &split->dispenv, offscreen);
}

#ifdef __vita__
typedef enum
{
	NATIVE_GPU_PASS_OPAQUE,
	NATIVE_GPU_PASS_DISCARD,
	NATIVE_GPU_PASS_BLEND,
	NATIVE_GPU_PASS_BLEND_DISCARD,
	NATIVE_GPU_PASS_COUNT,
} NativeGpuPassCategory;

struct NativeGpuSplitBounds
{
	s32 minX;
	s32 minY;
	s32 maxX;
	s32 maxY;
};

internal bool NativeGpu_GetSplitBounds(const GPUDrawSplit *split, struct NativeGpuSplitBounds *bounds);

typedef struct
{
	int domainFirstSplit;
	int representativeSplit;
	int lastMemberSplit;
	int semiTransPass;
	NativeGpuPassCategory category;
} NativeGpuDepthBatch;

typedef struct
{
	u32 generation;
	u32 hash;
	int batchIndex;
} NativeGpuDepthHashEntry;

#define NATIVE_GPU_DEPTH_HASH_CAPACITY 8192

typedef struct
{
	bool valid;
	NativeGpuPassCategory category;
	int semiTransPass;
} NativeGpuDepthPassInfo;

typedef struct
{
	int splitIndex;
	int indegree;
	bool scheduled;
	NativeGpuPassCategory category;
	struct NativeGpuSplitBounds bounds;
} NativeGpuBlendNode;

typedef struct
{
	int representativeSplit;
	int startVertex;
	int numVerts;
	int semiTransPass;
	bool active;
} NativeGpuBlendBatch;

global_variable NativeGpuDepthBatch s_gpuDepthBatches[MAX_DRAW_SPLITS];
global_variable int s_gpuDepthBatchCount;
global_variable NativeGpuDepthPassInfo s_gpuDepthPassInfo[MAX_DRAW_SPLITS];
global_variable int s_gpuDepthNextSplit[MAX_DRAW_SPLITS];
global_variable NativeGpuDepthHashEntry s_gpuDepthHashTable[NATIVE_GPU_DEPTH_HASH_CAPACITY];
global_variable u32 s_gpuDepthHashGeneration = 1;
global_variable NativeGpuBlendNode s_gpuBlendNodes[MAX_DRAW_SPLITS];

internal NativeGpuPassCategory NativeGpu_GetPassCategory(const GPUDrawSplit *split, BlendMode blendMode)
{
	const bool mayDiscard = split->textureId != NativeRenderer_GetWhiteTexture() && split->texFormat != TF_32_BIT_RGBA && !split->psxTextureFullyOpaque;
	if (blendMode == BM_NONE)
	{
		return mayDiscard ? NATIVE_GPU_PASS_DISCARD : NATIVE_GPU_PASS_OPAQUE;
	}
	return mayDiscard ? NATIVE_GPU_PASS_BLEND_DISCARD : NATIVE_GPU_PASS_BLEND;
}

internal bool NativeGpu_CanUseMixedSTPPass(const GPUDrawSplit *split)
{
	return split->psxTexturedSemiTrans && split->psxSemiTransPassMask == 3 && !split->drawPrimMode && !split->psxDrawMaskSet &&
	       split->blendMode != BM_SUBTRACT;
}

internal bool NativeGpu_GetDepthPassInfo(const GPUDrawSplit *split, NativeGpuPassCategory *category, int *semiTransPass)
{
	if ((split->drawenv.clip.w <= 0) || (split->drawenv.clip.h <= 0))
	{
		return false;
	}

	if (split->psxTexturedSemiTrans)
	{
		if (NativeGpu_CanUseMixedSTPPass(split))
		{
			return false;
		}
		if ((split->psxSemiTransPassMask & 1) == 0)
		{
			return false;
		}
		*category = NativeGpu_GetPassCategory(split, BM_NONE);
		*semiTransPass = 1;
		return true;
	}

	if (split->blendMode != BM_NONE)
	{
		return false;
	}

	*category = NativeGpu_GetPassCategory(split, BM_NONE);
	*semiTransPass = 0;
	return true;
}

internal bool NativeGpu_DepthPassStateCompatible(const GPUDrawSplit *first, int firstSemiTransPass, const GPUDrawSplit *second,
                                                  int secondSemiTransPass)
{
	if (firstSemiTransPass != secondSemiTransPass || first->textureId != second->textureId || first->texFormat != second->texFormat ||
	    first->drawPrimMode != second->drawPrimMode || first->psxTextureOutputSTP != second->psxTextureOutputSTP ||
	    first->psxDrawMaskSet != second->psxDrawMaskSet || first->psxTextureFullyOpaque != second->psxTextureFullyOpaque ||
	    first->p4CacheEligible != second->p4CacheEligible ||
	    (first->p4CacheEligible && (first->p4Page != second->p4Page || first->p4Clut != second->p4Clut)) ||
	    first->debugText != second->debugText || first->drawenv.dfe != second->drawenv.dfe)
	{
		return false;
	}

	if (first->drawenv.clip.x != second->drawenv.clip.x || first->drawenv.clip.y != second->drawenv.clip.y ||
	    first->drawenv.clip.w != second->drawenv.clip.w || first->drawenv.clip.h != second->drawenv.clip.h ||
	    first->dispenv.disp.x != second->dispenv.disp.x || first->dispenv.disp.y != second->dispenv.disp.y ||
	    first->dispenv.disp.w != second->dispenv.disp.w || first->dispenv.disp.h != second->dispenv.disp.h ||
	    first->dispenv.isinter != second->dispenv.isinter)
	{
		return false;
	}

	if (first->texFormat == TF_32_BIT_RGBA &&
	    (first->drawenv.tw.w != second->drawenv.tw.w || first->drawenv.tw.h != second->drawenv.tw.h))
	{
		return false;
	}

	return true;
}

#endif

internal void NativeGpu_DrawSplitRangePass(const GPUDrawSplit *split, int semiTransPass, BlendMode blendMode, bool depthWrite, int startVertex,
                                           int numVerts)
{
	if (split->debugText)
	{
		NativeRenderer_PushDebugLabel(split->debugText);
	}

	const bool drawOnScreen = split->drawenv.dfe;
	if ((split->drawenv.clip.w <= 0) || (split->drawenv.clip.h <= 0))
	{
		// NOTE(aalhendi): VS/Battle end previews can shrink a losing viewport
		// into an empty retail draw area. Empty clips should consume no pixels,
		// and must not leak stale native offscreen/scissor state.
		NativeRenderer_SetupClipMode(&split->drawenv.clip, &split->dispenv, drawOnScreen);
		NativeRenderer_SetOffscreenState(&split->drawenv.clip, 0);
		if (split->debugText)
		{
			NativeRenderer_PopDebugLabel();
		}
		return;
	}

	NativeRenderer_SetStencilMode(split->drawPrimMode); // draw with mask 0x16

	NativeRenderer_SetupClipMode(&split->drawenv.clip, &split->dispenv, drawOnScreen);
	NativeRenderer_SetOffscreenState(&split->drawenv.clip, !drawOnScreen);
	NativeGpu_SetSplitShaderState(split, semiTransPass, blendMode, !drawOnScreen);
	NativeRenderer_SetDepthState(1, depthWrite);
	NativeRenderer_DrawTriangles(startVertex, numVerts / 3);

	if (split->debugText)
	{
		NativeRenderer_PopDebugLabel();
	}
}

internal void NativeGpu_DrawSplitPass(const GPUDrawSplit *split, int semiTransPass, BlendMode blendMode, bool depthWrite)
{
	NativeGpu_DrawSplitRangePass(split, semiTransPass, blendMode, depthWrite, split->startVertex, split->numVerts);
}

void DrawSplit(const GPUDrawSplit *split)
{
#ifdef __vita__
	if (split->drawPrimMode == 2)
	{
		const DRAWENV savedDrawEnv = s_gpuBackendDrawEnv;
		const DISPENV savedDispEnv = s_gpuBackendDispEnv;
		NativeGpu_BackendApplyEnv(&split->drawenv, &split->dispenv);
		NativeRenderer_Clear(split->clearRect.x, split->clearRect.y, split->clearRect.w, split->clearRect.h, split->clearR, split->clearG, split->clearB);
		NativeGpu_BackendApplyEnv(&savedDrawEnv, &savedDispEnv);
		return;
	}
#endif

	if (split->psxTexturedSemiTrans)
	{
#ifdef __vita__
		if (NativeGpu_CanUseMixedSTPPass(split))
		{
			NativeGpu_DrawSplitPass(split, 3, split->blendMode, true);
			return;
		}
#endif
		// NOTE(aalhendi): CTR native renderer divergence from upstream PsyCross.
		// PS1 textured ABE only blends texels whose sampled 16-bit color has STP
		// set; non-STP texels remain opaque. Native split state is per draw,
		// so draw this primitive-sized split twice with shader-side STP masks.
		if ((split->psxSemiTransPassMask & 1) != 0)
		{
			NativeGpu_DrawSplitPass(split, 1, BM_NONE, true);
		}

		if ((split->psxSemiTransPassMask & 2) != 0)
		{
			NativeGpu_DrawSplitPass(split, 2, split->blendMode, false);
		}
	}
	else
	{
		NativeGpu_DrawSplitPass(split, 0, split->blendMode, split->blendMode == BM_NONE);
	}
}

#ifdef __vita__
internal bool NativeGpu_SplitsShareReorderDomain(const GPUDrawSplit *first, const GPUDrawSplit *second)
{
	if (first->drawPrimMode || second->drawPrimMode || first->drawenv.dfe != second->drawenv.dfe)
	{
		return false;
	}

	if (!first->drawenv.dfe)
	{
		return first->drawenv.clip.x == second->drawenv.clip.x && first->drawenv.clip.y == second->drawenv.clip.y &&
		       first->drawenv.clip.w == second->drawenv.clip.w && first->drawenv.clip.h == second->drawenv.clip.h;
	}

	return true;
}

internal u32 NativeGpu_DepthHashMix(u32 hash, u32 value)
{
	hash ^= value;
	hash *= 16777619u;
	return hash;
}

internal u32 NativeGpu_GetDepthStateHash(const GPUDrawSplit *split, int semiTransPass)
{
	u32 hash = 2166136261u;
	hash = NativeGpu_DepthHashMix(hash, (u32)semiTransPass);
	hash = NativeGpu_DepthHashMix(hash, (u32)split->textureId);
	hash = NativeGpu_DepthHashMix(hash, (u32)split->texFormat);
	hash = NativeGpu_DepthHashMix(hash, (u32)split->drawPrimMode);
	hash = NativeGpu_DepthHashMix(hash, (u32)split->psxTextureOutputSTP);
	hash = NativeGpu_DepthHashMix(hash, (u32)split->psxDrawMaskSet);
	hash = NativeGpu_DepthHashMix(hash, (u32)split->psxTextureFullyOpaque);
	hash = NativeGpu_DepthHashMix(hash, (u32)split->p4CacheEligible);
	if (split->p4CacheEligible)
	{
		hash = NativeGpu_DepthHashMix(hash, (u32)split->p4Page);
		hash = NativeGpu_DepthHashMix(hash, (u32)split->p4Clut);
	}
	hash = NativeGpu_DepthHashMix(hash, (u32)(uintptr_t)split->debugText);
	hash = NativeGpu_DepthHashMix(hash, (u32)split->drawenv.dfe);
	hash = NativeGpu_DepthHashMix(hash, (u32)(u16)split->drawenv.clip.x | ((u32)(u16)split->drawenv.clip.y << 16));
	hash = NativeGpu_DepthHashMix(hash, (u32)(u16)split->drawenv.clip.w | ((u32)(u16)split->drawenv.clip.h << 16));
	hash = NativeGpu_DepthHashMix(hash, (u32)(u16)split->dispenv.disp.x | ((u32)(u16)split->dispenv.disp.y << 16));
	hash = NativeGpu_DepthHashMix(hash, (u32)(u16)split->dispenv.disp.w | ((u32)(u16)split->dispenv.disp.h << 16));
	hash = NativeGpu_DepthHashMix(hash, (u32)split->dispenv.isinter);
	if (split->texFormat == TF_32_BIT_RGBA)
	{
		hash = NativeGpu_DepthHashMix(hash, (u32)(u16)split->drawenv.tw.w | ((u32)(u16)split->drawenv.tw.h << 16));
	}
	return hash;
}

internal u32 NativeGpu_BeginDepthHashGeneration(void)
{
	s_gpuDepthHashGeneration++;
	if (s_gpuDepthHashGeneration == 0)
	{
		memset(s_gpuDepthHashTable, 0, sizeof(s_gpuDepthHashTable));
		s_gpuDepthHashGeneration = 1;
	}
	return s_gpuDepthHashGeneration;
}

internal bool NativeGpu_BuildDepthBatches(void)
{
	s_gpuDepthBatchCount = 0;
	if (s_gpuDrawSplitCount <= 0)
	{
		return true;
	}

	memset(s_gpuDepthNextSplit, 0xff, sizeof(s_gpuDepthNextSplit));
	for (int i = 1; i <= s_gpuDrawSplitCount; i++)
	{
		NativeGpuDepthPassInfo *info = &s_gpuDepthPassInfo[i];
		info->valid = NativeGpu_GetDepthPassInfo(&s_gpuDrawSplits[i], &info->category, &info->semiTransPass);
	}

	int splitIndex = 1;
	while (splitIndex <= s_gpuDrawSplitCount)
	{
		GPUDrawSplit *first = &s_gpuDrawSplits[splitIndex];
		const int firstSplit = splitIndex;
		int lastSplit = firstSplit;
		if (!first->drawPrimMode)
		{
			while (lastSplit + 1 <= s_gpuDrawSplitCount && NativeGpu_SplitsShareReorderDomain(first, &s_gpuDrawSplits[lastSplit + 1]))
			{
				lastSplit++;
			}
		}

		if (!first->drawPrimMode)
		{
			for (int categoryIndex = NATIVE_GPU_PASS_OPAQUE; categoryIndex <= NATIVE_GPU_PASS_DISCARD; categoryIndex++)
			{
				const NativeGpuPassCategory wantedCategory = (NativeGpuPassCategory)categoryIndex;
				const u32 generation = NativeGpu_BeginDepthHashGeneration();
				for (int i = firstSplit; i <= lastSplit; i++)
				{
					const NativeGpuDepthPassInfo *info = &s_gpuDepthPassInfo[i];
					if (!info->valid || info->category != wantedCategory)
					{
						continue;
					}

					GPUDrawSplit *split = &s_gpuDrawSplits[i];
					const u32 hash = NativeGpu_GetDepthStateHash(split, info->semiTransPass);
					u32 slot = hash & (NATIVE_GPU_DEPTH_HASH_CAPACITY - 1);
					for (u32 probe = 0; probe < NATIVE_GPU_DEPTH_HASH_CAPACITY; probe++)
					{
						NativeGpuDepthHashEntry *entry = &s_gpuDepthHashTable[slot];
						if (entry->generation != generation)
						{
							if (s_gpuDepthBatchCount >= MAX_DRAW_SPLITS)
							{
								return false;
							}
							const int batchIndex = s_gpuDepthBatchCount++;
							NativeGpuDepthBatch *batch = &s_gpuDepthBatches[batchIndex];
							batch->domainFirstSplit = firstSplit;
							batch->representativeSplit = i;
							batch->lastMemberSplit = i;
							batch->semiTransPass = info->semiTransPass;
							batch->category = info->category;
							entry->generation = generation;
							entry->hash = hash;
							entry->batchIndex = batchIndex;
							break;
						}

						if (entry->hash == hash)
						{
							NativeGpuDepthBatch *batch = &s_gpuDepthBatches[entry->batchIndex];
							const GPUDrawSplit *representative = &s_gpuDrawSplits[batch->representativeSplit];
							if (NativeGpu_DepthPassStateCompatible(representative, batch->semiTransPass, split, info->semiTransPass))
							{
								s_gpuDepthNextSplit[batch->lastMemberSplit] = i;
								batch->lastMemberSplit = i;
								break;
							}
						}
						slot = (slot + 1) & (NATIVE_GPU_DEPTH_HASH_CAPACITY - 1);
					}
				}
			}
		}

		splitIndex = lastSplit + 1;
	}
	return true;
}

internal void NativeGpu_DrawDepthBatch(const NativeGpuDepthBatch *batch)
{
	const GPUDrawSplit *split = &s_gpuDrawSplits[batch->representativeSplit];
	if (split->debugText)
	{
		NativeRenderer_PushDebugLabel(split->debugText);
	}

	const bool drawOnScreen = split->drawenv.dfe;
	NativeRenderer_SetStencilMode(split->drawPrimMode);
	NativeRenderer_SetupClipMode(&split->drawenv.clip, &split->dispenv, drawOnScreen);
	NativeRenderer_SetOffscreenState(&split->drawenv.clip, !drawOnScreen);
	NativeGpu_SetSplitShaderState(split, batch->semiTransPass, BM_NONE, !drawOnScreen);
	NativeRenderer_SetDepthState(1, true);

	for (int memberIndex = batch->representativeSplit; memberIndex > 0; memberIndex = s_gpuDepthNextSplit[memberIndex])
	{
		const GPUDrawSplit *member = &s_gpuDrawSplits[memberIndex];
		NativeRenderer_DrawTriangles(member->startVertex, member->numVerts / 3);
	}

	if (split->debugText)
	{
		NativeRenderer_PopDebugLabel();
	}
}

internal void NativeGpu_DrawBlendPass(const GPUDrawSplit *split)
{
	if (split->psxTexturedSemiTrans)
	{
		if (NativeGpu_CanUseMixedSTPPass(split))
		{
			NativeGpu_DrawSplitPass(split, 3, split->blendMode, true);
			return;
		}
		if ((split->psxSemiTransPassMask & 2) != 0)
		{
			NativeGpu_DrawSplitPass(split, 2, split->blendMode, false);
		}
		return;
	}

	if (split->blendMode != BM_NONE)
	{
		NativeGpu_DrawSplitPass(split, 0, split->blendMode, false);
	}
}

internal bool NativeGpu_GetBlendPassInfo(const GPUDrawSplit *split, NativeGpuPassCategory *category, int *semiTransPass)
{
	if ((split->drawenv.clip.w <= 0) || (split->drawenv.clip.h <= 0))
	{
		return false;
	}

	if (split->psxTexturedSemiTrans)
	{
		if (NativeGpu_CanUseMixedSTPPass(split))
		{
			*category = NativeGpu_GetPassCategory(split, split->blendMode);
			*semiTransPass = 3;
			return true;
		}
		if ((split->psxSemiTransPassMask & 2) == 0)
		{
			return false;
		}
		*category = NativeGpu_GetPassCategory(split, split->blendMode);
		*semiTransPass = 2;
		return true;
	}

	if (split->blendMode == BM_NONE)
	{
		return false;
	}

	*category = NativeGpu_GetPassCategory(split, split->blendMode);
	*semiTransPass = 0;
	return true;
}

internal bool NativeGpu_BoundsOverlap(const struct NativeGpuSplitBounds *a, const struct NativeGpuSplitBounds *b)
{
	return !(a->maxX < b->minX || b->maxX < a->minX || a->maxY < b->minY || b->maxY < a->minY);
}

internal bool NativeGpu_BlendScissorStateCompatible(const GPUDrawSplit *first, const GPUDrawSplit *second)
{
	return first->drawenv.clip.x == second->drawenv.clip.x && first->drawenv.clip.y == second->drawenv.clip.y &&
	       first->drawenv.clip.w == second->drawenv.clip.w && first->drawenv.clip.h == second->drawenv.clip.h &&
	       first->dispenv.disp.x == second->dispenv.disp.x && first->dispenv.disp.y == second->dispenv.disp.y &&
	       first->dispenv.disp.w == second->dispenv.disp.w && first->dispenv.disp.h == second->dispenv.disp.h &&
	       first->dispenv.isinter == second->dispenv.isinter;
}

internal bool NativeGpu_BlendPassStateCompatible(const GPUDrawSplit *first, int firstSemiTransPass, const GPUDrawSplit *second,
                                                  int secondSemiTransPass)
{
	if (firstSemiTransPass != secondSemiTransPass || first->blendMode != second->blendMode || first->textureId != second->textureId ||
	    first->texFormat != second->texFormat || first->psxTextureOutputSTP != second->psxTextureOutputSTP ||
	    first->psxDrawMaskSet != second->psxDrawMaskSet || first->psxTextureFullyOpaque != second->psxTextureFullyOpaque ||
	    first->p4CacheEligible != second->p4CacheEligible ||
	    (first->p4CacheEligible && (first->p4Page != second->p4Page || first->p4Clut != second->p4Clut)) ||
	    first->debugText != second->debugText || first->drawenv.dfe != second->drawenv.dfe)
	{
		return false;
	}

	if (!NativeGpu_BlendScissorStateCompatible(first, second))
	{
		return false;
	}

	if (first->texFormat == TF_32_BIT_RGBA &&
	    (first->drawenv.tw.w != second->drawenv.tw.w || first->drawenv.tw.h != second->drawenv.tw.h))
	{
		return false;
	}

	return true;
}

internal void NativeGpu_FlushBlendBatch(NativeGpuBlendBatch *batch)
{
	if (!batch->active)
	{
		return;
	}

	const GPUDrawSplit *split = &s_gpuDrawSplits[batch->representativeSplit];
	NativeGpu_DrawSplitRangePass(split, batch->semiTransPass, split->blendMode, batch->semiTransPass == 3, batch->startVertex, batch->numVerts);
	batch->active = false;
}

internal void NativeGpu_AppendBlendBatch(NativeGpuBlendBatch *batch, int splitIndex, int semiTransPass)
{
	const GPUDrawSplit *split = &s_gpuDrawSplits[splitIndex];
	if (batch->active)
	{
		const GPUDrawSplit *representative = &s_gpuDrawSplits[batch->representativeSplit];
		if (batch->startVertex + batch->numVerts == split->startVertex &&
		    NativeGpu_BlendPassStateCompatible(representative, batch->semiTransPass, split, semiTransPass))
		{
			batch->numVerts += split->numVerts;
			return;
		}

		NativeGpu_FlushBlendBatch(batch);
	}

	batch->representativeSplit = splitIndex;
	batch->startVertex = split->startVertex;
	batch->numVerts = split->numVerts;
	batch->semiTransPass = semiTransPass;
	batch->active = true;
}

internal int NativeGpu_BlendCandidateScore(const NativeGpuBlendNode *candidate, const NativeGpuBlendNode *previous)
{
	if (previous == NULL)
	{
		return 0;
	}

	const GPUDrawSplit *candidateSplit = &s_gpuDrawSplits[candidate->splitIndex];
	const GPUDrawSplit *previousSplit = &s_gpuDrawSplits[previous->splitIndex];
	int score = 0;

	if (candidate->category == previous->category)
	{
		score += 1 << 24;
	}
	if (NativeGpu_BlendScissorStateCompatible(candidateSplit, previousSplit))
	{
		score += 1 << 20;
	}
	if (candidateSplit->p4CacheEligible && previousSplit->p4CacheEligible && candidateSplit->p4Page == previousSplit->p4Page &&
	    candidateSplit->p4Clut == previousSplit->p4Clut)
	{
		score += 1 << 18;
	}
	if (candidateSplit->blendMode == previousSplit->blendMode)
	{
		score += 1 << 16;
	}
	if (candidateSplit->psxTexturedSemiTrans == previousSplit->psxTexturedSemiTrans)
	{
		score += 1 << 14;
	}
	if (candidateSplit->textureId == previousSplit->textureId)
	{
		score += 1 << 12;
	}
	if (candidateSplit->texFormat == previousSplit->texFormat)
	{
		score += 1 << 10;
	}
	if (candidateSplit->psxTextureOutputSTP == previousSplit->psxTextureOutputSTP &&
	    candidateSplit->psxDrawMaskSet == previousSplit->psxDrawMaskSet)
	{
		score += 1 << 8;
	}

	return score;
}

internal void NativeGpu_DrawScheduledBlendPasses(int firstSplit, int lastSplit)
{
	int nodeCount = 0;
	for (int splitIndex = firstSplit; splitIndex <= lastSplit; splitIndex++)
	{
		NativeGpuPassCategory category;
		int semiTransPass;
		if (!NativeGpu_GetBlendPassInfo(&s_gpuDrawSplits[splitIndex], &category, &semiTransPass))
		{
			continue;
		}

		NativeGpuBlendNode *node = &s_gpuBlendNodes[nodeCount];
		if (!NativeGpu_GetSplitBounds(&s_gpuDrawSplits[splitIndex], &node->bounds))
		{
			continue;
		}

		node->splitIndex = splitIndex;
		node->indegree = 0;
		node->scheduled = false;
		node->category = category;
		nodeCount++;
	}

	NativeGpuBlendBatch blendBatch;
	memset(&blendBatch, 0, sizeof(blendBatch));
	if (nodeCount > 256)
	{
		for (int nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
		{
			NativeGpuPassCategory ignoredCategory;
			int semiTransPass;
			const int splitIndex = s_gpuBlendNodes[nodeIndex].splitIndex;
			if (NativeGpu_GetBlendPassInfo(&s_gpuDrawSplits[splitIndex], &ignoredCategory, &semiTransPass))
			{
				NativeGpu_AppendBlendBatch(&blendBatch, splitIndex, semiTransPass);
			}
		}
		NativeGpu_FlushBlendBatch(&blendBatch);
		return;
	}

	for (int firstNode = 0; firstNode < nodeCount; firstNode++)
	{
		for (int secondNode = firstNode + 1; secondNode < nodeCount; secondNode++)
		{
			if (NativeGpu_BoundsOverlap(&s_gpuBlendNodes[firstNode].bounds, &s_gpuBlendNodes[secondNode].bounds))
			{
				s_gpuBlendNodes[secondNode].indegree++;
			}
		}
	}
	NativeGpuBlendNode *previous = NULL;
	for (int outputIndex = 0; outputIndex < nodeCount; outputIndex++)
	{
		int bestNode = -1;
		int bestScore = -1;
		for (int nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
		{
			NativeGpuBlendNode *candidate = &s_gpuBlendNodes[nodeIndex];
			if (candidate->scheduled || candidate->indegree != 0)
			{
				continue;
			}

			const int score = NativeGpu_BlendCandidateScore(candidate, previous);
			if (bestNode < 0 || score > bestScore ||
			    (score == bestScore && candidate->splitIndex < s_gpuBlendNodes[bestNode].splitIndex))
			{
				bestNode = nodeIndex;
				bestScore = score;
			}
		}

		if (bestNode < 0)
		{
			NativeGpu_FlushBlendBatch(&blendBatch);
			NATIVE_GPU_ERROR("%s\n", "blend dependency scheduler reached an invalid cycle");
			for (int nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
			{
				if (!s_gpuBlendNodes[nodeIndex].scheduled)
				{
					NativeGpu_DrawBlendPass(&s_gpuDrawSplits[s_gpuBlendNodes[nodeIndex].splitIndex]);
					s_gpuBlendNodes[nodeIndex].scheduled = true;
				}
			}
			return;
		}

		NativeGpuBlendNode *selected = &s_gpuBlendNodes[bestNode];
		NativeGpuPassCategory ignoredCategory;
		int semiTransPass;
		if (!NativeGpu_GetBlendPassInfo(&s_gpuDrawSplits[selected->splitIndex], &ignoredCategory, &semiTransPass))
		{
			NativeGpu_FlushBlendBatch(&blendBatch);
			NATIVE_GPU_ERROR("%s\n", "scheduled blend node lost its pass metadata");
			return;
		}
		NativeGpu_AppendBlendBatch(&blendBatch, selected->splitIndex, semiTransPass);
		selected->scheduled = true;

		for (int nodeIndex = bestNode + 1; nodeIndex < nodeCount; nodeIndex++)
		{
			NativeGpuBlendNode *dependent = &s_gpuBlendNodes[nodeIndex];
			if (!dependent->scheduled && NativeGpu_BoundsOverlap(&selected->bounds, &dependent->bounds))
			{
				assert(dependent->indegree > 0);
				dependent->indegree--;
			}
		}

		previous = selected;
	}

	NativeGpu_FlushBlendBatch(&blendBatch);
}

internal void NativeGpu_DrawReorderDomain(int firstSplit, int lastSplit, int *depthBatchIndex)
{
	while (*depthBatchIndex < s_gpuDepthBatchCount && s_gpuDepthBatches[*depthBatchIndex].domainFirstSplit == firstSplit)
	{
		NativeGpu_DrawDepthBatch(&s_gpuDepthBatches[*depthBatchIndex]);
		(*depthBatchIndex)++;
	}

	NativeGpu_DrawScheduledBlendPasses(firstSplit, lastSplit);
}

internal void NativeGpu_DrawReorderedSplits(void)
{
	int depthBatchIndex = 0;
	int splitIndex = 1;
	while (splitIndex <= s_gpuDrawSplitCount)
	{
		GPUDrawSplit *first = &s_gpuDrawSplits[splitIndex];
		if (first->drawPrimMode)
		{
			DrawSplit(first);
			splitIndex++;
			continue;
		}

		const int firstSplit = splitIndex;
		int lastSplit = firstSplit;
		while (lastSplit + 1 <= s_gpuDrawSplitCount && NativeGpu_SplitsShareReorderDomain(first, &s_gpuDrawSplits[lastSplit + 1]))
		{
			lastSplit++;
		}

		NativeGpu_DrawReorderDomain(firstSplit, lastSplit, &depthBatchIndex);
		splitIndex = lastSplit + 1;
	}
}
#endif

#ifdef __vita__
internal bool NativeGpu_GetSplitBounds(const GPUDrawSplit *split, struct NativeGpuSplitBounds *bounds)
{
	if (split->numVerts == 0)
	{
		return false;
	}

	const GrVertex *first = &s_gpuDrawVertices[split->startVertex];
	bounds->minX = bounds->maxX = first->x;
	bounds->minY = bounds->maxY = first->y;
	for (u32 i = 1; i < split->numVerts; i++)
	{
		const GrVertex *vertex = &s_gpuDrawVertices[split->startVertex + i];
		if (vertex->x < bounds->minX)
		{
			bounds->minX = vertex->x;
		}
		if (vertex->x > bounds->maxX)
		{
			bounds->maxX = vertex->x;
		}
		if (vertex->y < bounds->minY)
		{
			bounds->minY = vertex->y;
		}
		if (vertex->y > bounds->maxY)
		{
			bounds->maxY = vertex->y;
		}
	}

	return true;
}

internal bool NativeGpu_SemiTransSplitsCanMerge(const GPUDrawSplit *first, const GPUDrawSplit *second)
{
	if (!first->psxTexturedSemiTrans || !second->psxTexturedSemiTrans || first->startVertex + first->numVerts != second->startVertex ||
	    (u32)first->numVerts + second->numVerts > 0xffffu)
	{
		return false;
	}

	if (first->blendMode != second->blendMode || first->texFormat != second->texFormat || first->textureId != second->textureId ||
	    first->drawPrimMode != second->drawPrimMode || first->psxTextureOutputSTP != second->psxTextureOutputSTP ||
	    first->psxDrawMaskSet != second->psxDrawMaskSet || first->p4CacheEligible != second->p4CacheEligible ||
	    (first->p4CacheEligible && (first->p4Page != second->p4Page || first->p4Clut != second->p4Clut)) || first->debugText != second->debugText ||
	    memcmp(&first->drawenv, &second->drawenv, sizeof(first->drawenv)) != 0 || memcmp(&first->dispenv, &second->dispenv, sizeof(first->dispenv)) != 0)
	{
		return false;
	}

	struct NativeGpuSplitBounds firstBounds;
	struct NativeGpuSplitBounds secondBounds;
	if (!NativeGpu_GetSplitBounds(first, &firstBounds) || !NativeGpu_GetSplitBounds(second, &secondBounds))
	{
		return false;
	}

	// Preserve exact primitive ordering whenever their covered regions can
	// overlap. Disjoint primitives may share the same opaque/STP draw pair.
	return firstBounds.maxX <= secondBounds.minX || secondBounds.maxX <= firstBounds.minX || firstBounds.maxY <= secondBounds.minY ||
	       secondBounds.maxY <= firstBounds.minY;
}

internal void NativeGpu_CoalesceNonOverlappingSemiTransSplits(void)
{
	int outputIndex = 0;
	for (int inputIndex = 1; inputIndex <= s_gpuDrawSplitCount; inputIndex++)
	{
		GPUDrawSplit split = s_gpuDrawSplits[inputIndex];
		if (outputIndex > 0 && NativeGpu_SemiTransSplitsCanMerge(&s_gpuDrawSplits[outputIndex], &split))
		{
			s_gpuDrawSplits[outputIndex].numVerts = (u16)(s_gpuDrawSplits[outputIndex].numVerts + split.numVerts);
			continue;
		}

		outputIndex++;
		if (outputIndex != inputIndex)
		{
			s_gpuDrawSplits[outputIndex] = split;
		}
	}
	s_gpuDrawSplitCount = outputIndex;
}

internal void NativeGpu_ClassifyOpaqueTextureSplits(void)
{
	for (int splitIndex = 1; splitIndex <= s_gpuDrawSplitCount; splitIndex++)
	{
		GPUDrawSplit *split = &s_gpuDrawSplits[splitIndex];
		split->psxTextureFullyOpaque = false;
		split->psxSemiTransPassMask = split->psxTexturedSemiTrans ? 3 : 0;
		if (split->numVerts == 0)
		{
			continue;
		}

		if (split->textureId == NativeRenderer_GetWhiteTexture())
		{
			continue;
		}
		if (!split->psxTextureOutputSTP)
		{
			// Native RGBA override textures already use a shader without discard.
			continue;
		}
		if (split->psxTexturedSemiTrans)
		{
			if (split->texFormat == TF_4_BIT || split->texFormat == TF_8_BIT)
			{
				u8 paletteProperties = 0;
				for (u32 vertexIndex = split->startVertex; vertexIndex + 2 < (u32)split->startVertex + split->numVerts; vertexIndex += 3)
				{
					const GrVertex *vertex = &s_gpuDrawVertices[vertexIndex];
					paletteProperties |= NativeRenderer_GetPaletteProperties(split->texFormat, vertex->clut);
					if (paletteProperties == (NATIVE_PALETTE_HAS_TRANSPARENT | NATIVE_PALETTE_HAS_OPAQUE | NATIVE_PALETTE_HAS_STP))
					{
						break;
					}
				}

				if ((paletteProperties & NATIVE_PALETTE_HAS_OPAQUE) == 0)
				{
					split->psxSemiTransPassMask &= ~1;
				}
				if ((paletteProperties & NATIVE_PALETTE_HAS_STP) == 0)
				{
					split->psxSemiTransPassMask &= ~2;
				}
				if ((split->psxSemiTransPassMask == 1 || split->psxSemiTransPassMask == 2) &&
				    (paletteProperties & NATIVE_PALETTE_HAS_TRANSPARENT) == 0)
				{
					split->psxTextureFullyOpaque = true;
				}
			}
			continue;
		}

		if (split->texFormat != TF_4_BIT && split->texFormat != TF_8_BIT)
		{
			continue;
		}

		bool allOpaque = true;
		for (u32 vertexIndex = split->startVertex; vertexIndex + 2 < (u32)split->startVertex + split->numVerts; vertexIndex += 3)
		{
			const GrVertex *vertex = &s_gpuDrawVertices[vertexIndex];
			const u8 paletteProperties = NativeRenderer_GetPaletteProperties(split->texFormat, vertex->clut);
			if ((paletteProperties & NATIVE_PALETTE_HAS_TRANSPARENT) != 0)
			{
				allOpaque = false;
				break;
			}
		}

		split->psxTextureFullyOpaque = allOpaque;
	}
}
#endif

internal void SetPSXMaskState(u32 code)
{
	s_gpu.psxDrawMaskSet = (code & 1) != 0;
}

//
// Draws all polygons after AggregatePTAG
//
internal void NativeGpu_DrawPreparedFrame(GrVertex *vertices, GPUDrawSplit *splits, int vertexCount, int splitCount)
{
	s_gpuDrawVertices = vertices;
	s_gpuDrawSplits = splits;
	s_gpuDrawVertexCount = vertexCount;
	s_gpuDrawSplitCount = splitCount;
	NativePerf_BeginScope(NATIVE_PERF_BUCKET_DRAW_ALL_SPLITS);
	// CPU-originated LoadImage, MoveImage, and fill commands are GPU-visible
	// before the next draw batch, matching PS1 command ordering.
	NativeRenderer_UpdateVRAM();
#ifdef CTR_INTERNAL
	if (g_dbg_emulatorPaused)
	{
		for (int i = 0; i < 3; i++)
		{
			GrVertex *vert = &s_gpuDrawVertices[g_dbg_polygonSelected + i];
			vert->r = 255;
			vert->g = 0;
			vert->b = 0;

			NATIVE_GPU_LOG("%s\n", "==========================================");
			NATIVE_GPU_LOG("POLYGON: %d\n", g_dbg_polygonSelected);
			NATIVE_GPU_LOG("X: %d Y: %d\n", vert->x, vert->y);
			NATIVE_GPU_LOG("U: %d V: %d\n", vert->u, vert->v);
			NATIVE_GPU_LOG("TP: %d CLT: %d\n", vert->page, vert->clut);

			NATIVE_GPU_LOG("%s\n", "==========================================");
		}

		Platform_PollHostEvents();
	}
#endif

#ifdef __vita__
	NativeGpu_CoalesceNonOverlappingSemiTransSplits();
	NativeGpu_ClassifyOpaqueTextureSplits();
	const bool depthBatchingReady = NativeGpu_BuildDepthBatches();
	NativeRenderer_UpdateVertexBuffer(s_gpuDrawVertices, s_gpuDrawVertexCount);
#else
	// next code ideally should be called before EndScene
	NativeRenderer_UpdateVertexBuffer(s_gpuDrawVertices, s_gpuDrawVertexCount);
#endif

#ifdef __vita__
	if (depthBatchingReady)
	{
		NativeGpu_DrawReorderedSplits();
	}
	else
	{
		for (int i = 1; i <= s_gpuDrawSplitCount; i++)
		{
			DrawSplit(&s_gpuDrawSplits[i]);
		}
	}
#else
	for (int i = 1; i <= s_gpuDrawSplitCount; i++)
	{
		DrawSplit(&s_gpuDrawSplits[i]);
	}
#endif


	NativePerf_EndScope(NATIVE_PERF_BUCKET_DRAW_ALL_SPLITS);
}

void DrawAllSplits(void)
{
	NativeGpu_DrawPreparedFrame(s_gpu.vertexBuffer, s_gpu.splits, s_gpu.vertexIndex, s_gpu.splitIndex);
	ClearSplits();
}

// forward declarations
int ParsePrimitive(P_TAG *polyTag);
int ParseTaglessPrimitive(u32 *command);

internal bool NativeGpu_IsValidOTLink(uintptr_t link)
{
	if (NativeGpuLinks_IsRegisteredHostPointer((const void *)link))
	{
		return (link & (sizeof(u32) - 1)) == 0;
	}

	return false;
}

internal u32 NativeGpu_ReadPacketWordForLog(uintptr_t packet, int wordIndex)
{
	const struct PlatformMempackArena *arena = Platform_GetMempackArena();
	const uintptr_t word = packet + (uintptr_t)wordIndex * sizeof(u32);
	const uintptr_t end = word + sizeof(u32);

	if ((NativeGpuLinks_IsRegisteredHostRange((const void *)word, sizeof(u32))) && ((word & (sizeof(u32) - 1)) == 0))
	{
		return *(const u32 *)word;
	}

	if ((word < (uintptr_t)arena->base) || (end > (uintptr_t)arena->endOfMemory) || ((word & (sizeof(u32) - 1)) != 0))
	{
		return 0xffffffffu;
	}

	return *(const u32 *)word;
}

internal void NativeGpu_FormatPointerRegion(char *dst, size_t dstSize, uintptr_t ptr)
{
	if ((sdata == NULL) || (sdata->gGT == NULL))
	{
		snprintf(dst, dstSize, "no-gGT");
		return;
	}

	struct GameTracker *gGT = sdata->gGT;

	for (int playerIndex = 0; playerIndex < 4; playerIndex++)
	{
		struct PushBuffer *pb = &gGT->pushBuffer[playerIndex];
		const uintptr_t start = (uintptr_t)pb->ptrOT;
		const uintptr_t end = (uintptr_t)pb->renderBucketOTRangeEnd;
		if ((start != 0) && (end != 0) && (ptr >= start) && (ptr <= end))
		{
			snprintf(dst, dstSize, "pb%d.ot+0x%zx", playerIndex, (size_t)(ptr - start));
			return;
		}
	}

	struct PushBuffer *uiPB = &gGT->pushBuffer_UI;
	const uintptr_t uiStart = (uintptr_t)uiPB->ptrOT;
	const uintptr_t uiEnd = (uintptr_t)uiPB->renderBucketOTRangeEnd;
	if ((uiStart != 0) && (uiEnd != 0) && (ptr >= uiStart) && (ptr <= uiEnd))
	{
		snprintf(dst, dstSize, "ui.ot+0x%zx", (size_t)(ptr - uiStart));
		return;
	}

	for (int dbIndex = 0; dbIndex < 2; dbIndex++)
	{
		struct DB *db = &gGT->db[dbIndex];
		const uintptr_t primStart = (uintptr_t)db->primMem.start;
		const uintptr_t primEnd = (uintptr_t)db->primMem.end;
		if ((primStart != 0) && (ptr >= primStart) && (ptr < primEnd))
		{
			snprintf(dst, dstSize, "db%d.prim+0x%zx", dbIndex, (size_t)(ptr - primStart));
			return;
		}

		const uintptr_t otStart = (uintptr_t)db->otMem.start;
		const uintptr_t otEnd = (uintptr_t)db->otMem.end;
		if ((otStart != 0) && (ptr >= otStart) && (ptr < otEnd))
		{
			snprintf(dst, dstSize, "db%d.ot+0x%zx", dbIndex, (size_t)(ptr - otStart));
			return;
		}
	}

	snprintf(dst, dstSize, "unknown");
}

void ParsePrimitivesLinkedList(u32 *p, int singlePrimitive)
{
	if (!p)
	{
		return;
	}

	NativePerf_BeginScope(NATIVE_PERF_BUCKET_DRAW_OTAG_PARSE);

#ifdef CTR_NATIVE
	if (!singlePrimitive && !NativeGpuLinks_IsRegisteredHostPointer(p) && !isendprim(p))
	{
		char packetRegion[64];
		NativeGpu_FormatPointerRegion(packetRegion, sizeof(packetRegion), (uintptr_t)p);
		NATIVE_GPU_ERROR("unregistered linked DrawOTag packet: packet=%p region=%s addr=%06x len=%d code=%02x words=%08x %08x %08x %08x\n", (void *)p,
		                 packetRegion, getaddr(p), getlen(p), getcode(p), NativeGpu_ReadPacketWordForLog((uintptr_t)p, 0),
		                 NativeGpu_ReadPacketWordForLog((uintptr_t)p, 1), NativeGpu_ReadPacketWordForLog((uintptr_t)p, 2),
		                 NativeGpu_ReadPacketWordForLog((uintptr_t)p, 3));
		NativePerf_EndScope(NATIVE_PERF_BUCKET_DRAW_OTAG_PARSE);
		return;
	}
#endif

	// setup single primitive flag (needed for AddSplits)
	s_gpu.drawPrimMode = singlePrimitive;

	if (singlePrimitive)
	{
		P_TAG *polyTag = (P_TAG *)p;
		ParsePrimitive(polyTag);

		GPUDrawSplit *lastSplit = &s_gpu.splits[s_gpu.splitIndex];
		lastSplit->numVerts = s_gpu.vertexIndex - lastSplit->startVertex;
	}
	else
	{
		// walk OT_TAG linked list
		u8 *basePacket = (u8 *)p;
		while (true)
		{
			const int tagLength = getlen(basePacket);
			if (tagLength > 0)
			{
				if (tagLength > 32)
				{
					char packetRegion[64];
					NativeGpu_FormatPointerRegion(packetRegion, sizeof(packetRegion), (uintptr_t)basePacket);
					NATIVE_GPU_ERROR("got invalid tag length %d, code %d packet=%p region=%s words=%08x %08x %08x %08x\n", tagLength,
					                 ((P_TAG *)basePacket)->code, (void *)basePacket, packetRegion, NativeGpu_ReadPacketWordForLog((uintptr_t)basePacket, 0),
					                 NativeGpu_ReadPacketWordForLog((uintptr_t)basePacket, 1), NativeGpu_ReadPacketWordForLog((uintptr_t)basePacket, 2),
					                 NativeGpu_ReadPacketWordForLog((uintptr_t)basePacket, 3));
					break;
				}

				u8 *currentPacket = basePacket;
				u8 *endPacket = basePacket + (tagLength + P_LEN) * sizeof(u32);
				int primLength = 0;
				if (currentPacket < endPacket)
				{
					primLength = ParsePrimitive((P_TAG *)currentPacket);
					currentPacket += (primLength + P_LEN) * sizeof(u32);
				}

				while (currentPacket < endPacket)
				{
					primLength = ParseTaglessPrimitive((u32 *)currentPacket);
					currentPacket += primLength * sizeof(u32);
				}

				if (currentPacket != endPacket)
				{
					char packetRegion[64];
					NativeGpu_FormatPointerRegion(packetRegion, sizeof(packetRegion), (uintptr_t)basePacket);
					NATIVE_GPU_ERROR("did not output valid primitive or ptag length is not valid (diff=%d packet=%p region=%s words=%08x %08x %08x %08x)\n",
					                 endPacket - currentPacket, (void *)basePacket, packetRegion, NativeGpu_ReadPacketWordForLog((uintptr_t)basePacket, 0),
					                 NativeGpu_ReadPacketWordForLog((uintptr_t)basePacket, 1), NativeGpu_ReadPacketWordForLog((uintptr_t)basePacket, 2),
					                 NativeGpu_ReadPacketWordForLog((uintptr_t)basePacket, 3));
				}
			}

			GPUDrawSplit *lastSplit = &s_gpu.splits[s_gpu.splitIndex];
			lastSplit->numVerts = s_gpu.vertexIndex - lastSplit->startVertex;

			if (isendprim(basePacket))
			{
				break;
			}

			u8 *nextPacket = nextPrim(basePacket);
			if (!NativeGpu_IsValidOTLink((uintptr_t)nextPacket))
			{
				char packetRegion[64];
				char nextRegion[64];
				NativeGpu_FormatPointerRegion(packetRegion, sizeof(packetRegion), (uintptr_t)basePacket);
				NativeGpu_FormatPointerRegion(nextRegion, sizeof(nextRegion), (uintptr_t)nextPacket);
				NATIVE_GPU_ERROR("invalid OT link: packet=%p region=%s addr=%06x next=%p nextRegion=%s len=%d code=%02x words=%08x %08x %08x %08x\n",
				                 (void *)basePacket, packetRegion, getaddr(basePacket), (void *)nextPacket, nextRegion, getlen(basePacket), getcode(basePacket),
				                 NativeGpu_ReadPacketWordForLog((uintptr_t)basePacket, 0), NativeGpu_ReadPacketWordForLog((uintptr_t)basePacket, 1),
				                 NativeGpu_ReadPacketWordForLog((uintptr_t)basePacket, 2), NativeGpu_ReadPacketWordForLog((uintptr_t)basePacket, 3));
				break;
			}

			basePacket = nextPacket;
		}
	}

	NativePerf_EndScope(NATIVE_PERF_BUCKET_DRAW_OTAG_PARSE);
}

internal inline int IsNull(POLY_FT3 *poly)
{
	return poly->x0 == -1 && poly->y0 == -1 && poly->x1 == -1 && poly->y1 == -1 && poly->x2 == -1 && poly->y2 == -1;
}

internal int ProcessFlatLines(P_TAG *polyTag)
{
	const bool shadeTexOn = true;
	const bool semiTrans = (polyTag->code & 2);
	const int primSubType = polyTag->code & 0x0C;

	switch (primSubType)
	{
	case 0x0:
	{
		LINE_F2 *poly = (LINE_F2 *)polyTag;

		AddSplit(semiTrans, false, false, 0);

		VERTTYPE *p0 = &poly->x0;
		VERTTYPE *p1 = &poly->x1;
		u8 *c0 = &poly->r0;
		u8 *c1 = c0;

		GrVertex *firstVertex = &s_gpu.vertexBuffer[s_gpu.vertexIndex];
		LineSwapSourceVerts(&p0, &p1, &c0, &c1);
		MakeLineArray(firstVertex, p0, p1);
		MakeTexcoordLineZero(firstVertex, 0);
		MakeColourLine(firstVertex, shadeTexOn, c0, c1);

		TriangulateQuad();

		s_gpu.vertexIndex += 6;

		return 3;
	}
	case 0x8: // TODO (unused)
	{
		LINE_F3 *poly = (LINE_F3 *)polyTag;

		AddSplit(semiTrans, false, false, 0);

		{
			VERTTYPE *p0 = &poly->x0;
			VERTTYPE *p1 = &poly->x1;
			u8 *c0 = &poly->r0;
			u8 *c1 = c0;

			GrVertex *firstVertex = &s_gpu.vertexBuffer[s_gpu.vertexIndex];
			LineSwapSourceVerts(&p0, &p1, &c0, &c1);
			MakeLineArray(firstVertex, p0, p1);
			MakeTexcoordLineZero(firstVertex, 0);
			MakeColourLine(firstVertex, shadeTexOn, c0, c1);

			TriangulateQuad();

			s_gpu.vertexIndex += 6;
		}

		{
			VERTTYPE *p0 = &poly->x1;
			VERTTYPE *p1 = &poly->x2;
			u8 *c0 = &poly->r0;
			u8 *c1 = c0;

			GrVertex *firstVertex = &s_gpu.vertexBuffer[s_gpu.vertexIndex];
			LineSwapSourceVerts(&p0, &p1, &c0, &c1);
			MakeLineArray(firstVertex, p0, p1);
			MakeTexcoordLineZero(firstVertex, 0);
			MakeColourLine(firstVertex, shadeTexOn, c0, c1);

			TriangulateQuad();

			s_gpu.vertexIndex += 6;
		}

		return 5;
	}
	case 0xc:
	{
		LINE_F4 *poly = (LINE_F4 *)polyTag;

		AddSplit(semiTrans, false, false, 0);

		{
			VERTTYPE *p0 = &poly->x0;
			VERTTYPE *p1 = &poly->x1;
			u8 *c0 = &poly->r0;
			u8 *c1 = c0;

			GrVertex *firstVertex = &s_gpu.vertexBuffer[s_gpu.vertexIndex];
			LineSwapSourceVerts(&p0, &p1, &c0, &c1);
			MakeLineArray(firstVertex, p0, p1);
			MakeTexcoordLineZero(firstVertex, 0);
			MakeColourLine(firstVertex, shadeTexOn, c0, c1);

			TriangulateQuad();

			s_gpu.vertexIndex += 6;
		}

		{
			VERTTYPE *p0 = &poly->x1;
			VERTTYPE *p1 = &poly->x2;
			u8 *c0 = &poly->r0;
			u8 *c1 = c0;

			GrVertex *firstVertex = &s_gpu.vertexBuffer[s_gpu.vertexIndex];
			LineSwapSourceVerts(&p0, &p1, &c0, &c1);
			MakeLineArray(firstVertex, p0, p1);
			MakeTexcoordLineZero(firstVertex, 0);
			MakeColourLine(firstVertex, shadeTexOn, c0, c1);

			TriangulateQuad();

			s_gpu.vertexIndex += 6;
		}

		{
			VERTTYPE *p0 = &poly->x2;
			VERTTYPE *p1 = &poly->x3;
			u8 *c0 = &poly->r0;
			u8 *c1 = c0;

			GrVertex *firstVertex = &s_gpu.vertexBuffer[s_gpu.vertexIndex];
			LineSwapSourceVerts(&p0, &p1, &c0, &c1);
			MakeLineArray(firstVertex, p0, p1);
			MakeTexcoordLineZero(firstVertex, 0);
			MakeColourLine(firstVertex, shadeTexOn, c0, c1);

			TriangulateQuad();

			s_gpu.vertexIndex += 6;
		}

		return 6;
	}
	}
	return 0;
}

internal int ProcessGouraudLines(P_TAG *polyTag)
{
	const bool shadeTexOn = true;
	const bool semiTrans = (polyTag->code & 2);
	const int primSubType = polyTag->code & 0x0C;

	switch (primSubType)
	{
	case 0x0:
	{
		LINE_G2 *poly = (LINE_G2 *)polyTag;

		AddSplit(semiTrans, false, false, 0);

		VERTTYPE *p0 = &poly->x0;
		VERTTYPE *p1 = &poly->x1;
		u8 *c0 = &poly->r0;
		u8 *c1 = &poly->r1;

		GrVertex *firstVertex = &s_gpu.vertexBuffer[s_gpu.vertexIndex];
		LineSwapSourceVerts(&p0, &p1, &c0, &c1);
		MakeLineArray(firstVertex, p0, p1);
		MakeTexcoordLineZero(firstVertex, 0);
		MakeColourLine(firstVertex, shadeTexOn, c0, c1);

		TriangulateQuad();

		s_gpu.vertexIndex += 6;

		return 4;
	}
	case 0x8:
	{
		// TODO: LINE_G3
		return 7;
	}
	case 0xC:
	{
		// TODO: LINE_G4
		return 9;
	}
	}
	return 0;
}

internal int ProcessFlatPoly(P_TAG *polyTag)
{
	const bool shadeTexOn = (polyTag->code & 1) == 0;
	const bool semiTrans = (polyTag->code & 2);
	const int primSubType = polyTag->code & 0x0C;

	switch (primSubType)
	{
	case 0x0:
	{
		POLY_F3 *poly = (POLY_F3 *)polyTag;

		AddSplit(semiTrans, false, false, 0);

		GrVertex *firstVertex = &s_gpu.vertexBuffer[s_gpu.vertexIndex];
		MakeVertexTriangle(firstVertex, &poly->x0, &poly->x1, &poly->x2);
		MakeTexcoordTriangleZero(firstVertex, 0);
		MakeColourTriangle(firstVertex, shadeTexOn, &poly->r0, &poly->r0, &poly->r0);

		s_gpu.vertexIndex += 3;

		return 4;
	}
	case 0x4:
	{
		POLY_FT3 *poly = (POLY_FT3 *)polyTag;
		activeDrawEnv.tpage = poly->tpage;

		// It is an official hack from SCE devs to not use DR_TPAGE and instead use null polygon
		if (!IsNull(poly))
		{
			AddSplit(semiTrans, true, NativeGpu_TPageOverlapsActiveDrawPage(poly->tpage), poly->clut);

			GrVertex *firstVertex = &s_gpu.vertexBuffer[s_gpu.vertexIndex];
			MakeVertexTriangle(firstVertex, &poly->x0, &poly->x1, &poly->x2);
			MakeTexcoordTriangle(firstVertex, &poly->u0, &poly->u1, &poly->u2, poly->tpage, poly->clut,
			                     GET_TPAGE_DITHER(activeDrawEnv.tpage) || activeDrawEnv.dtd);
			MakeColourTriangle(firstVertex, shadeTexOn, &poly->r0, &poly->r0, &poly->r0);

			s_gpu.vertexIndex += 3;
		}
		return 7;
	}
	case 0x8:
	{
		POLY_F4 *poly = (POLY_F4 *)polyTag;

		AddSplit(semiTrans, false, false, 0);

		GrVertex *firstVertex = &s_gpu.vertexBuffer[s_gpu.vertexIndex];
		MakeVertexQuad(firstVertex, &poly->x0, &poly->x1, &poly->x3, &poly->x2);
		MakeTexcoordQuadZero(firstVertex, 0);
		MakeColourQuad(firstVertex, shadeTexOn, &poly->r0, &poly->r0, &poly->r0, &poly->r0);

		TriangulateQuad();

		s_gpu.vertexIndex += 6;
		return 5;
	}
	case 0xC:
	{
		POLY_FT4 *poly = (POLY_FT4 *)polyTag;
		activeDrawEnv.tpage = poly->tpage;

		AddSplit(semiTrans, true, NativeGpu_TPageOverlapsActiveDrawPage(poly->tpage), poly->clut);

		GrVertex *firstVertex = &s_gpu.vertexBuffer[s_gpu.vertexIndex];
		MakeVertexQuad(firstVertex, &poly->x0, &poly->x1, &poly->x3, &poly->x2);
		MakeTexcoordQuad(firstVertex, &poly->u0, &poly->u1, &poly->u3, &poly->u2, poly->tpage, poly->clut,
		                 GET_TPAGE_DITHER(activeDrawEnv.tpage) || activeDrawEnv.dtd);
		MakeColourQuad(firstVertex, shadeTexOn, &poly->r0, &poly->r0, &poly->r0, &poly->r0);

		TriangulateQuad();

		s_gpu.vertexIndex += 6;

		return 9;
	}
	}
	return 0;
}

internal int ProcessGouraudPoly(P_TAG *polyTag)
{
	const bool shadeTexOn = true;
	const bool semiTrans = (polyTag->code & 2);
	const int primSubType = polyTag->code & 0x0C;

	switch (primSubType)
	{
	case 0x0:
	{
		POLY_G3 *poly = (POLY_G3 *)polyTag;

		AddSplit(semiTrans, false, false, 0);

		GrVertex *firstVertex = &s_gpu.vertexBuffer[s_gpu.vertexIndex];
		MakeVertexTriangle(firstVertex, &poly->x0, &poly->x1, &poly->x2);
		MakeTexcoordTriangleZero(firstVertex, 1);
		MakeColourTriangle(firstVertex, shadeTexOn, &poly->r0, &poly->r1, &poly->r2);

		s_gpu.vertexIndex += 3;

		return 6;
	}
	case 0x4:
	{
		POLY_GT3 *poly = (POLY_GT3 *)polyTag;
		activeDrawEnv.tpage = poly->tpage;

		AddSplit(semiTrans, true, NativeGpu_TPageOverlapsActiveDrawPage(poly->tpage), poly->clut);

		GrVertex *firstVertex = &s_gpu.vertexBuffer[s_gpu.vertexIndex];
		MakeVertexTriangle(firstVertex, &poly->x0, &poly->x1, &poly->x2);
		MakeTexcoordTriangle(firstVertex, &poly->u0, &poly->u1, &poly->u2, poly->tpage, poly->clut, GET_TPAGE_DITHER(activeDrawEnv.tpage) || activeDrawEnv.dtd);
		MakeColourTriangle(firstVertex, shadeTexOn, &poly->r0, &poly->r1, &poly->r2);

		s_gpu.vertexIndex += 3;

		return 9;
	}
	case 0x8:
	{
		POLY_G4 *poly = (POLY_G4 *)polyTag;

		AddSplit(semiTrans, false, false, 0);

		GrVertex *firstVertex = &s_gpu.vertexBuffer[s_gpu.vertexIndex];
		MakeVertexQuad(firstVertex, &poly->x0, &poly->x1, &poly->x3, &poly->x2);
		MakeTexcoordQuadZero(firstVertex, 1);
		MakeColourQuad(firstVertex, shadeTexOn, &poly->r0, &poly->r1, &poly->r3, &poly->r2);

		TriangulateQuad();

		s_gpu.vertexIndex += 6;

		return 8;
	}
	case 0xC:
	{
		POLY_GT4 *poly = (POLY_GT4 *)polyTag;
		activeDrawEnv.tpage = poly->tpage;

		AddSplit(semiTrans, true, NativeGpu_TPageOverlapsActiveDrawPage(poly->tpage), poly->clut);

		GrVertex *firstVertex = &s_gpu.vertexBuffer[s_gpu.vertexIndex];
		MakeVertexQuad(firstVertex, &poly->x0, &poly->x1, &poly->x3, &poly->x2);
		MakeTexcoordQuad(firstVertex, &poly->u0, &poly->u1, &poly->u3, &poly->u2, poly->tpage, poly->clut,
		                 GET_TPAGE_DITHER(activeDrawEnv.tpage) || activeDrawEnv.dtd);
		MakeColourQuad(firstVertex, shadeTexOn, &poly->r0, &poly->r1, &poly->r3, &poly->r2);

		TriangulateQuad();

		s_gpu.vertexIndex += 6;

		return 12;
	}
	}
	return 0;
}

internal int ProcessTileAndSprt(P_TAG *polyTag)
{
	// NOTE: TILE does not support switching shadeTex on real PSX
	const bool shadeTexOn = (polyTag->code & 1) == 0;
	const bool semiTrans = (polyTag->code & 2);

	switch (polyTag->code & 0xFD)
	{
	case 0x60:
	{
		TILE *poly = (TILE *)polyTag;

		AddSplit(semiTrans, false, false, 0);

		GrVertex *firstVertex = &s_gpu.vertexBuffer[s_gpu.vertexIndex];
		MakeVertexRect(firstVertex, &poly->x0, poly->w, poly->h);
		MakeTexcoordQuadZero(firstVertex, 0);
		MakeColourQuad(firstVertex, shadeTexOn, &poly->r0, &poly->r0, &poly->r0, &poly->r0);

		TriangulateQuad();

		s_gpu.vertexIndex += 6;

		return 3;
	}
	case 0x64:
	{
		SPRT *poly = (SPRT *)polyTag;

		AddSplit(semiTrans, true, NativeGpu_TPageOverlapsActiveDrawPage(activeDrawEnv.tpage), poly->clut);
		s_gpu.vertexIndex += NativeGpu_EmitTexturedSprite(&poly->x0, &poly->u0, activeDrawEnv.tpage, poly->clut, poly->w, poly->h, shadeTexOn, &poly->r0);

		return 4;
	}
	case 0x68:
	{
		TILE_1 *poly = (TILE_1 *)polyTag;

		AddSplit(semiTrans, false, false, 0);

		GrVertex *firstVertex = &s_gpu.vertexBuffer[s_gpu.vertexIndex];
		MakeVertexRect(firstVertex, &poly->x0, 1, 1);
		MakeTexcoordQuadZero(firstVertex, 0);
		MakeColourQuad(firstVertex, true, &poly->r0, &poly->r0, &poly->r0, &poly->r0);

		TriangulateQuad();

		s_gpu.vertexIndex += 6;

		return 2;
	}
	case 0x70:
	{
		TILE_8 *poly = (TILE_8 *)polyTag;

		AddSplit(semiTrans, false, false, 0);

		GrVertex *firstVertex = &s_gpu.vertexBuffer[s_gpu.vertexIndex];
		MakeVertexRect(firstVertex, &poly->x0, 8, 8);
		MakeTexcoordQuadZero(firstVertex, 0);
		MakeColourQuad(firstVertex, true, &poly->r0, &poly->r0, &poly->r0, &poly->r0);

		TriangulateQuad();

		s_gpu.vertexIndex += 6;

		return 2;
	}
	case 0x74:
	{
		SPRT_8 *poly = (SPRT_8 *)polyTag;

		AddSplit(semiTrans, true, NativeGpu_TPageOverlapsActiveDrawPage(activeDrawEnv.tpage), poly->clut);
		s_gpu.vertexIndex += NativeGpu_EmitTexturedSprite(&poly->x0, &poly->u0, activeDrawEnv.tpage, poly->clut, 8, 8, shadeTexOn, &poly->r0);

		return 3;
	}
	case 0x78:
	{
		TILE_16 *poly = (TILE_16 *)polyTag;

		AddSplit(semiTrans, false, false, 0);

		GrVertex *firstVertex = &s_gpu.vertexBuffer[s_gpu.vertexIndex];
		MakeVertexRect(firstVertex, &poly->x0, 16, 16);
		MakeTexcoordQuadZero(firstVertex, 0);
		MakeColourQuad(firstVertex, true, &poly->r0, &poly->r0, &poly->r0, &poly->r0);

		TriangulateQuad();

		s_gpu.vertexIndex += 6;

		return 2;
	}
	case 0x7C:
	{
		SPRT_16 *poly = (SPRT_16 *)polyTag;

		AddSplit(semiTrans, true, NativeGpu_TPageOverlapsActiveDrawPage(activeDrawEnv.tpage), poly->clut);
		s_gpu.vertexIndex += NativeGpu_EmitTexturedSprite(&poly->x0, &poly->u0, activeDrawEnv.tpage, poly->clut, 16, 16, shadeTexOn, &poly->r0);

		return 3;
	}
	}
	return 0;
}

internal int ProcessDrawEnv(P_TAG *polyTag)
{
	const u32 *codePtr = (u32 *)&polyTag->pad0;
	int processedLongs = 0;
	bool fullDrawEnvPacket = false;
	for (int i = 0; i < polyTag->len; ++i)
	{
		const u32 code = codePtr[i];
		const int primType = code >> 24 & 0xF0;
		const int primSubType = code >> 24 & 0x0F;

		// NOTE(aalhendi): CTR can pack draw-env commands, tagless geometry,
		// and more draw-env commands into one OT entry. Stop at the first
		// non-E command so ParseTaglessPrimitive owns the geometry payload.
		if (primType != 0xE0)
		{
			return processedLongs;
		}

		switch (primSubType)
		{
		case 0x1:
		{
			// DR_TPAGE
			activeDrawEnv.tpage = (code & 0x1FF);
			activeDrawEnv.dtd = (code >> 9) & 1;
			// NOTE(aalhendi): Standalone DR_TPAGE packets use the same E1 word
			// for blend changes; only full DRAWENV packets retarget native
			// on-screen/offscreen rendering.
			if (fullDrawEnvPacket)
			{
				activeDrawEnv.dfe = (code >> 10) & 1;
			}
			break;
		}
		case 0x2:
		{
			// DR_TWIN
			activeDrawEnv.tw.w = (code & 0x1F);
			activeDrawEnv.tw.h = ((code >> 5) & 0x1F);
			activeDrawEnv.tw.x = ((code >> 10) & 0x1F);
			activeDrawEnv.tw.y = ((code >> 15) & 0x1F);
			break;
		}
		case 0x3:
		{
			// DR_AREA
			activeDrawEnv.clip.x = code & 1023;
			activeDrawEnv.clip.y = (code >> 10) & 1023;
			fullDrawEnvPacket = true;
			break;
		}
		case 0x4:
		{
			// DR_AREA (second part)
			activeDrawEnv.clip.w = code & 1023;
			activeDrawEnv.clip.h = (code >> 10) & 1023;

			activeDrawEnv.clip.w = activeDrawEnv.clip.w - activeDrawEnv.clip.x + 1;
			activeDrawEnv.clip.h = activeDrawEnv.clip.h - activeDrawEnv.clip.y + 1;
			fullDrawEnvPacket = true;
			break;
		}
		case 0x5:
		{
			// DR_OFFSET
			activeDrawEnv.ofs[0] = NativeGpu_SignExtend11(code);
			activeDrawEnv.ofs[1] = NativeGpu_SignExtend11(code >> 11);
			fullDrawEnvPacket = true;
			break;
		}
		case 0x6:
		{
			SetPSXMaskState(code);
			break;
		}
		case 0:
			// NOTE(aalhendi): ctr-native local divergence for CTR OTs. A zero
			// word can be terminal draw-env padding, or the tag word for the
			// next primitive packed into the same OT entry.
			// return processedLongs;
			if (i + 1 != polyTag->len)
			{
				return processedLongs;
			}
			break;
		}
		++processedLongs;
	}

	return processedLongs;
}

internal void ProcessDrawEnvCommand(u32 code)
{
	const int primSubType = code >> 24 & 0x0F;

	switch (primSubType)
	{
	case 0x1:
		activeDrawEnv.tpage = (code & 0x1FF);
		activeDrawEnv.dtd = (code >> 9) & 1;
		break;
	case 0x2:
		activeDrawEnv.tw.w = (code & 0x1F);
		activeDrawEnv.tw.h = ((code >> 5) & 0x1F);
		activeDrawEnv.tw.x = ((code >> 10) & 0x1F);
		activeDrawEnv.tw.y = ((code >> 15) & 0x1F);
		break;
	case 0x3:
		activeDrawEnv.clip.x = code & 1023;
		activeDrawEnv.clip.y = (code >> 10) & 1023;
		break;
	case 0x4:
		activeDrawEnv.clip.w = code & 1023;
		activeDrawEnv.clip.h = (code >> 10) & 1023;
		activeDrawEnv.clip.w = activeDrawEnv.clip.w - activeDrawEnv.clip.x + 1;
		activeDrawEnv.clip.h = activeDrawEnv.clip.h - activeDrawEnv.clip.y + 1;
		break;
	case 0x5:
		activeDrawEnv.ofs[0] = NativeGpu_SignExtend11(code);
		activeDrawEnv.ofs[1] = NativeGpu_SignExtend11(code >> 11);
		break;
	case 0x6:
		SetPSXMaskState(code);
		break;
	}
}

internal int ProcessPsyXPrims(P_TAG *polyTag)
{
	const int primSubType = polyTag->code & 0x0F;

	switch (primSubType)
	{
	case 0x01:
	{
		DR_PSYX_TEX *psytex = (DR_PSYX_TEX *)polyTag;
		s_gpu.overrideTexture = psytex->code[0] & 0xFFFFFF;
		s_gpu.overrideTextureWidth = psytex->code[1] & 0xFFF;
		s_gpu.overrideTextureHeight = psytex->code[1] >> 16 & 0xFFF;
		return 2;
	}
	case 0x02:
	{
		// [A] Psy-X custom debug marker packet
		DR_PSYX_DBGMARKER *psydbg = (DR_PSYX_DBGMARKER *)polyTag;
		s_gpu.currentSplitDebugText = psydbg->text;
		return 2;
	}
	}

	return 0;
}

// Processes primitive
// returns processed primitive primLength in longs
int ParsePrimitive(P_TAG *polyTag)
{
	const int primType = polyTag->code & 0xF0;

	int primLength = 0;
	bool handledZeroLength = false;

	switch (primType)
	{
	case 0x00:
	{
		const int primSubType = polyTag->code & 0x0F;
		const u32 *codePtr = (u32 *)&polyTag->pad0;
		// NOTE(aalhendi): ctr-native local divergence. CTR RenderWeather can
		// emit a retail length-2 zero packet when weather is enabled but the
		// level has no fill-mode payload. The PSX consumes it by tag length;
		// the native parser must advance past it too.
		if (polyTag->len == 2 && codePtr[0] == 0 && codePtr[1] == 0)
		{
			primLength = 2;
		}
		else if (polyTag->len == 0 && *(u32 *)polyTag == 0)
		{
			// CTR ghost transparency packets include raw GPU NOP words between
			// draw-mode changes and triangle commands. They consume exactly one
			// command word; ParsePrimitivesLinkedList adds P_LEN to the return.
			handledZeroLength = true;
		}
		else if (primSubType == 0x0)
		{
			primLength = 3;
		}
		else if (primSubType == 0x1)
		{
			DR_MOVE *drmove = (DR_MOVE *)polyTag;
			const u32 rectPos = drmove->code[2];
			const u32 rectSize = drmove->code[4];

			const int y = drmove->code[3] >> 0x10 & 0xFFFF;
			const int x = drmove->code[3] & 0xFFFF;

			RECT16 rect;
			rect.x = (s16)(rectPos & 0xffff);
			rect.y = (s16)(rectPos >> 16);
			rect.w = (s16)(rectSize & 0xffff);
			rect.h = (s16)(rectSize >> 16);

#ifdef __vita__
			NativeGpu_ForceSynchronousFrame();
			NativeGpu_FlushFrontendSplitsSync();
#else
			if (NativeGpu_HasPendingSplits())
			{
				DrawAllSplits();
			}
#endif
			MoveImage(&rect, x, y);
			primLength = 5;
		}
		else if (primSubType == 0x2)
		{
			// NOTE(aalhendi): ctr-native local divergence. CTR emits retail
			// FILL packets in OTs; the old PsyCross parser did not consume them, which caused
			// zero-length primitive spam.
			TILE *fill = (TILE *)polyTag;
			RECT16 rect;

			rect.x = fill->x0;
			rect.y = fill->y0;
			rect.w = fill->w;
			rect.h = fill->h;

			if (activeDrawEnv.dfe && rect.x >= activeDrawEnv.clip.x && rect.y >= activeDrawEnv.clip.y &&
			    rect.x + rect.w <= activeDrawEnv.clip.x + activeDrawEnv.clip.w && rect.y + rect.h <= activeDrawEnv.clip.y + activeDrawEnv.clip.h)
			{
#ifdef __vita__
				if (!NativeGpu_AppendClearSplit(&rect, fill->r0, fill->g0, fill->b0))
				{
					NativeGpu_ForceSynchronousFrame();
					NativeGpu_FlushFrontendSplitsSync();
					NativeGpuBackendRectTask task = {rect.x, rect.y, rect.w, rect.h, fill->r0, fill->g0, fill->b0};
					NativeGpu_RunBackendTaskSync(NativeGpu_BackendClearTask, &task);
				}
#else
				if (NativeGpu_HasPendingSplits())
				{
					DrawAllSplits();
				}
				NativeRenderer_Clear(rect.x, rect.y, rect.w, rect.h, fill->r0, fill->g0, fill->b0);
#endif
			}
			else
			{
#ifdef __vita__
				NativeGpu_ForceSynchronousFrame();
				NativeGpu_FlushFrontendSplitsSync();
#else
				if (NativeGpu_HasPendingSplits())
				{
					DrawAllSplits();
				}
#endif
				ClearImage(&rect, fill->r0, fill->g0, fill->b0);
			}
			primLength = 3;
		}
		break;
	}
	case 0x20:
		// Flat polygons
		primLength = ProcessFlatPoly(polyTag);
		break;
	case 0x30:
		// Gouraud shaded polygons
		primLength = ProcessGouraudPoly(polyTag);
		break;
	case 0x40:
		// Flat (single colour) Lines
		primLength = ProcessFlatLines(polyTag);
		break;
	case 0x50:
		// Gouraud lines
		primLength = ProcessGouraudLines(polyTag);
		break;
	case 0x60:
	case 0x70:
		// TILE and SPRT
		primLength = ProcessTileAndSprt(polyTag);
		break;
	case 0xA0:
		// DR_LOAD
		{
			DR_LOAD *drload = (DR_LOAD *)polyTag;
			const u32 rectPos = drload->code[1];
			const u32 rectSize = drload->code[2];

			RECT16 rect;
			rect.x = (s16)(rectPos & 0xffff);
			rect.y = (s16)(rectPos >> 16);
			rect.w = (s16)(rectSize & 0xffff);
			rect.h = (s16)(rectSize >> 16);

#ifdef __vita__
			if (NativeGpu_HasPendingSplits())
			{
				NativeGpu_ForceSynchronousFrame();
				NativeGpu_FlushFrontendSplitsSync();
			}
#endif
			LoadImage(&rect, (uint32_t *)drload->p);

			// TODO(aalhendi): Audit whether CTR ever appends additional GPU
			// commands after a DR_LOAD payload in the same packet.
		}
		primLength = getlen(polyTag);
		break;
	case 0xB0:
		// [A] Psy-X custom primitives
		primLength = ProcessPsyXPrims(polyTag);
		break;
	case 0xE0:
		// Draw Env setup
		primLength = ProcessDrawEnv(polyTag);
		break;
		// default:
		//	NATIVE_GPU_ERROR("got %0x primitive\n", primType);
	}

	if (primLength == 0 && !handledZeroLength)
	{
		NATIVE_GPU_ERROR("Unhandled zero length %0x primitive\n", primType);
	}

	return primLength;
}

int ParseTaglessPrimitive(u32 *command)
{
	const u32 code = *command;
	const int primType = (code >> 24) & 0xF0;

	if (code == 0)
	{
		return 1;
	}

	if (primType == 0xE0)
	{
		ProcessDrawEnvCommand(code);
		return 1;
	}

	P_TAG *polyTag = (P_TAG *)(command - P_LEN);
	int primLength = ParsePrimitive(polyTag);

	if (primLength == 0)
	{
		NATIVE_GPU_ERROR("Unhandled tagless primitive %08x\n", code);
		return 1;
	}

	return primLength;
}
