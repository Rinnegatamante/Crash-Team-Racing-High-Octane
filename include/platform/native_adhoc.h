#ifndef NATIVE_ADHOC_H
#define NATIVE_ADHOC_H

#include "platform/native_input.h"

enum NativeAdhocStatus
{
	NATIVE_ADHOC_STATUS_IDLE = 0,
	NATIVE_ADHOC_STATUS_CONNECTING,
	NATIVE_ADHOC_STATUS_SEARCHING,
	NATIVE_ADHOC_STATUS_WAITING,
	NATIVE_ADHOC_STATUS_SYNCHRONIZING,
	NATIVE_ADHOC_STATUS_CONNECTED,
	NATIVE_ADHOC_STATUS_ERROR,
};

struct NativeAdhocTimingState
{
	u64 rootCounterValue;
	u64 rootCounterBase;
	s32 vblankCount;
};

void NativeAdhoc_StartHost(void);
void NativeAdhoc_StartClient(void);
void NativeAdhoc_Cancel(void);
void NativeAdhoc_Update(void);
void NativeAdhoc_Shutdown(void);
int NativeAdhoc_BeginGameFrame(void);
int NativeAdhoc_IsSessionActive(void);
int NativeAdhoc_IsTimingControlled(void);
int NativeAdhoc_IsMenuInputBlocked(void);
int NativeAdhoc_IsCommonDialogActive(void);
int NativeAdhoc_HostNeedsState(void);
int NativeAdhoc_HostStateReady(void);
int NativeAdhoc_ConsumeGameplayFailure(void);
enum NativeAdhocStatus NativeAdhoc_GetStatus(void);

void NativeAdhocTiming_Capture(struct NativeAdhocTimingState *state);
void NativeAdhocTiming_Restore(const struct NativeAdhocTimingState *state);

#endif
