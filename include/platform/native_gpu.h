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
void ClearSplits(void);
void DrawAllSplits(void);
void ParsePrimitivesLinkedList(u32 *p, int singlePrimitive);
int NativeGpu_GetStateSize(void);
int NativeGpu_CaptureState(void *dst, int dstSize);
int NativeGpu_RestoreState(const void *src, int srcSize);

#endif
