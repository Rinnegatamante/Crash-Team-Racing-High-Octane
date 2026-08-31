/*
 * Derived from REDRIVER2/PsyCross MIT source:
 * externals/PsyCross/src/gpu/PsyX_GPU.h
 * See THIRD_PARTY_NOTICES.md for copyright and license details.
 */

#ifndef NATIVE_GPU_H
#define NATIVE_GPU_H

#include <macros.h>
#include <psx/libgte.h>
#include <psx/libgpu.h>

extern DISPENV activeDispEnv;
extern DRAWENV activeDrawEnv;
extern int g_GPUDisabledState;

int NativeGpu_HasPendingSplits(void);
void NativeGpu_ResetOrderDepth(void);
void NativeGpu_BeginFrontendFrame(void);
void NativeGpu_SetFrontendDrawEnv(const DRAWENV *env);
void NativeGpu_SetFrontendDispEnv(const DISPENV *env);
const DRAWENV *NativeGpu_GetRenderDrawEnv(void);
const DISPENV *NativeGpu_GetRenderDispEnv(void);
int NativeGpu_SubmitFrontendFrame(void);
typedef void (*NativeGpuBackendTaskFn)(void *arg);
void NativeGpu_RunBackendTaskSync(NativeGpuBackendTaskFn task, void *arg);
void NativeGpu_SyncBackend(void);
void NativeGpu_FlushFrontendSplitsSync(void);
void NativeGpu_FinishSynchronousFrame(void);
void NativeGpu_SyncVRAMToCPU(int x, int y, int w, int h);
int NativeGpu_InitBackend(int width, int height);
void NativeGpu_ShutdownBackend(void);
void NativeGpu_ForceSynchronousFrame(void);
int NativeGpu_IsSynchronousFrame(void);
int NativeGpu_IsFrontendFrameActive(void);
void ClearSplits(void);
void DrawAllSplits(void);
void ParsePrimitivesLinkedList(u32 *p, int singlePrimitive);
int NativeGpu_GetStateSize(void);
int NativeGpu_CaptureState(void *dst, int dstSize);
int NativeGpu_RestoreState(const void *src, int srcSize);

#endif
