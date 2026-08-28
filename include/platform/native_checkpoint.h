#ifndef PLATFORM_NATIVE_CHECKPOINT_H
#define PLATFORM_NATIVE_CHECKPOINT_H

#include <macros.h>

int NativeCheckpoint_GetSize(void);
unsigned int NativeCheckpoint_GetFormatVersion(void);
int NativeCheckpoint_Capture(void *dst, int dstSize);
int NativeCheckpoint_Restore(const void *src, int srcSize);
#if defined(CTR_INTERNAL)
void NativeCheckpoint_OnMempackArenaReset(void);
void NativeCheckpoint_RegisterPointerSlot(void *slot);
#endif

#endif
