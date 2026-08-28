#ifndef PLATFORM_NATIVE_ADHOC_H
#define PLATFORM_NATIVE_ADHOC_H

struct GameTracker;
struct PlatformInputPadSnapshot;
struct PushBuffer;

enum NativeAdhocRole
{
	NATIVE_ADHOC_ROLE_NONE = 0,
	NATIVE_ADHOC_ROLE_HOST = 1,
	NATIVE_ADHOC_ROLE_CLIENT = 2,
};

enum NativeAdhocStatus
{
	NATIVE_ADHOC_STATUS_OFF = 0,
	NATIVE_ADHOC_STATUS_DIALOG,
	NATIVE_ADHOC_STATUS_WAITING,
	NATIVE_ADHOC_STATUS_CONNECTED,
	NATIVE_ADHOC_STATUS_ERROR,
};

int NativeAdhoc_IsSupported(void);
int NativeAdhoc_Begin(int role);
void NativeAdhoc_Shutdown(void);
void NativeAdhoc_Update(void);
void NativeAdhoc_WaitForFrame(void);
void NativeAdhoc_ProcessPadSnapshots(struct PlatformInputPadSnapshot *pads, int count);
int NativeAdhoc_PrepareRaceLoad(struct GameTracker *gGT);
int NativeAdhoc_EnforcePreparedRaceConfig(struct GameTracker *gGT);
void NativeAdhoc_NotifyLevelReady(struct GameTracker *gGT);
int NativeAdhoc_BeginSimulationFrame(struct GameTracker *gGT);
void NativeAdhoc_EndSimulationFrame(struct GameTracker *gGT);
void NativeAdhoc_BeginRenderFrame(struct GameTracker *gGT);
void NativeAdhoc_EndRenderFrame(void);
int NativeAdhoc_IsSingleViewRenderActive(void);
struct PushBuffer *NativeAdhoc_GetRenderPushBuffer(void);
int NativeAdhoc_IsActive(void);
int NativeAdhoc_IsConnected(void);
int NativeAdhoc_IsSimulationActive(void);
int NativeAdhoc_IsDialogRunning(void);
int NativeAdhoc_ShouldPresentDriver(int driverID);
int NativeAdhoc_ShouldReturnToMainMenu(void);
void NativeAdhoc_AcknowledgeReturnToMainMenu(void);
int NativeAdhoc_ShouldDrawConnectionLostNotice(void);
int NativeAdhoc_GetRole(void);
int NativeAdhoc_GetStatus(void);
int NativeAdhoc_GetLocalPlayerIndex(void);
int NativeAdhoc_GetRemotePlayerIndex(void);
u32 NativeAdhoc_GetSimulationFrame(void);
const char *NativeAdhoc_GetStatusText(void);

#endif
