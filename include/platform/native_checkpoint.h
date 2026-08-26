#ifndef PLATFORM_NATIVE_CHECKPOINT_H
#define PLATFORM_NATIVE_CHECKPOINT_H

#include <macros.h>

int NativeCheckpoint_GetSize(void);
int NativeCheckpoint_Capture(void *dst, int dstSize);
int NativeCheckpoint_Restore(const void *src, int srcSize);
int NativeCheckpoint_GetGameplaySize(void);
int NativeCheckpoint_CaptureGameplay(void *dst, int dstSize);
int NativeCheckpoint_RestoreGameplay(const void *src, int srcSize);
#if defined(CTR_INTERNAL)
void NativeCheckpoint_OnMempackArenaReset(void);
void NativeCheckpoint_RegisterPointerSlot(void *slot);
#endif

#endif
