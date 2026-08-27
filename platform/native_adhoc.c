#include <common.h>

#include "platform/native_adhoc.h"
#include "platform/native_checkpoint.h"
#include "platform/native_input.h"
#include "platform/native_log.h"
#include "platform/native_renderer.h"

#include <stdlib.h>
#include <string.h>

#if defined(__vita__)

#include <psp2/common_dialog.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/net/adhoc_matching.h>
#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <psp2/netcheck_dialog.h>
#include <psp2/sysmodule.h>

#ifndef CTR_NATIVE_VERSION
#define CTR_NATIVE_VERSION "0.0.0-dev"
#endif

#ifndef CTR_NATIVE_BUILD_ID
#define CTR_NATIVE_BUILD_ID "unknown"
#endif

#ifndef SCE_NET_ADHOC_MATCHING_EVENT_HELLO
#define SCE_NET_ADHOC_MATCHING_EVENT_HELLO       1
#define SCE_NET_ADHOC_MATCHING_EVENT_REQUEST     2
#define SCE_NET_ADHOC_MATCHING_EVENT_LEAVE       3
#define SCE_NET_ADHOC_MATCHING_EVENT_DENY        4
#define SCE_NET_ADHOC_MATCHING_EVENT_CANCEL      5
#define SCE_NET_ADHOC_MATCHING_EVENT_ACCEPT      6
#define SCE_NET_ADHOC_MATCHING_EVENT_ESTABLISHED 7
#define SCE_NET_ADHOC_MATCHING_EVENT_TIMEOUT     8
#define SCE_NET_ADHOC_MATCHING_EVENT_ERROR       9
#define SCE_NET_ADHOC_MATCHING_EVENT_BYE         10
#endif

#define NATIVE_ADHOC_NET_MEMORY_SIZE          (256 * 1024)
#define NATIVE_ADHOC_MATCHING_POOL_SIZE       (128 * 1024)
#define NATIVE_ADHOC_MATCHING_STACK_SIZE      (NATIVE_ADHOC_MATCHING_POOL_SIZE + (4 * 1024))
#define NATIVE_ADHOC_MATCHING_PORT            1
#define NATIVE_ADHOC_STREAM_PORT              27183
#define NATIVE_ADHOC_MATCHING_INTERVAL_US     (1000 * 1000)
#define NATIVE_ADHOC_MATCHING_RETRY_COUNT     120
#define NATIVE_ADHOC_MATCHING_PRIORITY        0x10000100
#define NATIVE_ADHOC_BOOTSTRAP_CHUNK_SIZE     (64 * 1024)
#define NATIVE_ADHOC_SNAPSHOT_MAX_SIZE        (64 * 1024 * 1024)
#define NATIVE_ADHOC_HELLO_SIZE               24
#define NATIVE_ADHOC_SNAPSHOT_HEADER_SIZE     56
#define NATIVE_ADHOC_CONTROL_SIZE             24
#define NATIVE_ADHOC_INPUT_SIZE               40
#define NATIVE_ADHOC_PROTOCOL_VERSION         2
#define NATIVE_ADHOC_WIRE_MAGIC               0x43545241u
#define NATIVE_ADHOC_WIRE_HELLO               1
#define NATIVE_ADHOC_WIRE_SNAPSHOT            2
#define NATIVE_ADHOC_WIRE_READY               3
#define NATIVE_ADHOC_WIRE_START               4
#define NATIVE_ADHOC_WIRE_INPUT               5
#define NATIVE_ADHOC_WIRE_LOAD_READY          6
#define NATIVE_ADHOC_SNAPSHOT_INITIAL         1
#define NATIVE_ADHOC_SNAPSHOT_RESYNC          2
#define NATIVE_ADHOC_MATCHING_TIMEOUT_US      (120ULL * 1000 * 1000)
#define NATIVE_ADHOC_CONNECT_TIMEOUT_US       (15ULL * 1000 * 1000)
#define NATIVE_ADHOC_BOOTSTRAP_TIMEOUT_US     (30ULL * 1000 * 1000)
#define NATIVE_ADHOC_CONTROL_TIMEOUT_US       (15ULL * 1000 * 1000)
#define NATIVE_ADHOC_FRAME_TIMEOUT_US         (5ULL * 1000 * 1000)
#define NATIVE_ADHOC_LOAD_TIMEOUT_US          (120ULL * 1000 * 1000)
#define NATIVE_ADHOC_IDLE_DELAY_US            1000
#define NATIVE_ADHOC_SHUTDOWN_DIALOG_STEPS    240
#define NATIVE_ADHOC_SHUTDOWN_DIALOG_DELAY_US 8000

#define NATIVE_ADHOC_MATCH_FLAG_CANDIDATE   (1u << 0)
#define NATIVE_ADHOC_MATCH_FLAG_REQUEST     (1u << 1)
#define NATIVE_ADHOC_MATCH_FLAG_REJECT      (1u << 2)
#define NATIVE_ADHOC_MATCH_FLAG_ESTABLISHED (1u << 3)
#define NATIVE_ADHOC_MATCH_FLAG_LOST        (1u << 4)
#define NATIVE_ADHOC_MATCH_FLAG_ERROR       (1u << 5)

CTR_STATIC_ASSERT(sizeof(struct PlatformInputPadSnapshot) == 12);
CTR_STATIC_ASSERT(NATIVE_ADHOC_INPUT_SIZE == 28 + sizeof(struct PlatformInputPadSnapshot));

extern int gNativeMirrorModeEnabled;
extern int gNativeGhostReplayMode;
extern int gNativeBossFightMode;
extern int gNativeBossFightBossID;

enum NativeAdhocRole
{
	NATIVE_ADHOC_ROLE_NONE = 0,
	NATIVE_ADHOC_ROLE_HOST = 1,
	NATIVE_ADHOC_ROLE_CLIENT = 2,
};

enum NativeAdhocState
{
	NATIVE_ADHOC_STATE_IDLE = 0,
	NATIVE_ADHOC_STATE_DIALOG,
	NATIVE_ADHOC_STATE_DIALOG_ABORTING,
	NATIVE_ADHOC_STATE_MATCHING,
	NATIVE_ADHOC_STATE_HOST_ACCEPT,
	NATIVE_ADHOC_STATE_CLIENT_CONNECT,
	NATIVE_ADHOC_STATE_HELLO,
	NATIVE_ADHOC_STATE_HOST_WAIT_STATE,
	NATIVE_ADHOC_STATE_HOST_SEND_HEADER,
	NATIVE_ADHOC_STATE_HOST_SEND_SNAPSHOT,
	NATIVE_ADHOC_STATE_HOST_WAIT_READY,
	NATIVE_ADHOC_STATE_HOST_SEND_START,
	NATIVE_ADHOC_STATE_CLIENT_RECV_HEADER,
	NATIVE_ADHOC_STATE_CLIENT_RECV_SNAPSHOT,
	NATIVE_ADHOC_STATE_CLIENT_SEND_READY,
	NATIVE_ADHOC_STATE_CLIENT_RECV_START,
	NATIVE_ADHOC_STATE_ACTIVE,
	NATIVE_ADHOC_STATE_LOAD_LOCAL,
	NATIVE_ADHOC_STATE_LOAD_READY,
	NATIVE_ADHOC_STATE_ERROR,
};

static u8 s_nativeAdhocNetMemory[NATIVE_ADHOC_NET_MEMORY_SIZE] __attribute__((aligned(64)));
static u8 s_nativeAdhocMatchingPool[NATIVE_ADHOC_MATCHING_POOL_SIZE] __attribute__((aligned(64)));

static enum NativeAdhocRole s_nativeAdhocRole;
static enum NativeAdhocState s_nativeAdhocState;
static enum NativeAdhocStatus s_nativeAdhocStatus;
static int s_nativeAdhocNetModuleLoaded;
static int s_nativeAdhocMatchingModuleLoaded;
static int s_nativeAdhocNetInitialized;
static int s_nativeAdhocNetCtlInitialized;
static int s_nativeAdhocMatchingInitialized;
static int s_nativeAdhocDialogActive;
static int s_nativeAdhocDialogCancelRequested;
static int s_nativeAdhocDialogFailurePending;
static int s_nativeAdhocMatchingId = -1;
static int s_nativeAdhocListener = -1;
static int s_nativeAdhocSocket = -1;
static int s_nativeAdhocTargetSelected;
static int s_nativeAdhocGameplayFailure;
static int s_nativeAdhocSessionSynchronized;
static int s_nativeAdhocSavedMirrorMode;
static int s_nativeAdhocNativeModeSaved;
static int s_nativeAdhocLoadStableTicks;
static int s_nativeAdhocFramePending;
static u32 s_nativeAdhocEpoch;
static u32 s_nativeAdhocPendingEpoch;
static u32 s_nativeAdhocFrame;
static u32 s_nativeAdhocSnapshotReason;
static u16 s_nativeAdhocSnapshotLevelId;
static u64 s_nativeAdhocDeadlineUs;
static u64 s_nativeAdhocProgressTimeoutUs;
static SceNetInAddr s_nativeAdhocPeerAddress;

static volatile u32 s_nativeAdhocMatchingFlags;
static volatile u32 s_nativeAdhocCandidateAddress;
static volatile u32 s_nativeAdhocRequestAddress;
static volatile u32 s_nativeAdhocRejectAddress;
static volatile u32 s_nativeAdhocEstablishedAddress;

static u8 s_nativeAdhocHelloSend[NATIVE_ADHOC_HELLO_SIZE];
static u8 s_nativeAdhocHelloRecv[NATIVE_ADHOC_HELLO_SIZE];
static u8 s_nativeAdhocSnapshotHeader[NATIVE_ADHOC_SNAPSHOT_HEADER_SIZE];
static u8 s_nativeAdhocControlSend[NATIVE_ADHOC_CONTROL_SIZE];
static u8 s_nativeAdhocControlRecv[NATIVE_ADHOC_CONTROL_SIZE];
static u8 s_nativeAdhocFrameSend[NATIVE_ADHOC_INPUT_SIZE];
static u8 s_nativeAdhocFrameRecv[NATIVE_ADHOC_INPUT_SIZE];
static u32 s_nativeAdhocSendOffset;
static u32 s_nativeAdhocRecvOffset;
static u32 s_nativeAdhocFrameSendOffset;
static u32 s_nativeAdhocFrameRecvOffset;
static u32 s_nativeAdhocFrameFingerprint;
static u8 *s_nativeAdhocSnapshot;
static u32 s_nativeAdhocSnapshotSize;
static u32 s_nativeAdhocSnapshotCrc;
static struct PlatformInputPadSnapshot s_nativeAdhocLocalInput;
static struct NativeAdhocTimingState s_nativeAdhocTiming;

static void NativeAdhoc_WriteU16(u8 *dst, u16 value)
{
	dst[0] = (u8)(value >> 8);
	dst[1] = (u8)value;
}

static void NativeAdhoc_WriteU32(u8 *dst, u32 value)
{
	dst[0] = (u8)(value >> 24);
	dst[1] = (u8)(value >> 16);
	dst[2] = (u8)(value >> 8);
	dst[3] = (u8)value;
}

static void NativeAdhoc_WriteU64(u8 *dst, u64 value)
{
	NativeAdhoc_WriteU32(dst, (u32)(value >> 32));
	NativeAdhoc_WriteU32(dst + 4, (u32)value);
}

static u16 NativeAdhoc_ReadU16(const u8 *src)
{
	return (u16)(((u16)src[0] << 8) | src[1]);
}

static u32 NativeAdhoc_ReadU32(const u8 *src)
{
	return ((u32)src[0] << 24) | ((u32)src[1] << 16) | ((u32)src[2] << 8) | src[3];
}

static u64 NativeAdhoc_ReadU64(const u8 *src)
{
	return ((u64)NativeAdhoc_ReadU32(src) << 32) | NativeAdhoc_ReadU32(src + 4);
}

static u32 NativeAdhoc_Crc32(const void *data, u32 size)
{
	const u8 *bytes = (const u8 *)data;
	u32 crc = 0xffffffffu;

	for (u32 i = 0; i < size; i++)
	{
		crc ^= bytes[i];
		for (u32 bit = 0; bit < 8; bit++)
		{
			const u32 mask = 0u - (crc & 1u);
			crc = (crc >> 1) ^ (0xedb88320u & mask);
		}
	}

	return ~crc;
}

static u32 NativeAdhoc_Fnv1a(u32 hash, const void *data, size_t size)
{
	const u8 *bytes = (const u8 *)data;

	for (size_t i = 0; i < size; i++)
	{
		hash ^= bytes[i];
		hash *= 16777619u;
	}
	return hash;
}

static u32 NativeAdhoc_CompatibilityHash(void)
{
	const u32 values[] =
	{
		BUILD,
		NATIVE_ADHOC_PROTOCOL_VERSION,
		sizeof(struct PlatformInputPadSnapshot),
		sizeof(struct NativeAdhocTimingState),
	};
	u32 hash = 2166136261u;

	hash = NativeAdhoc_Fnv1a(hash, CTR_NATIVE_VERSION, strlen(CTR_NATIVE_VERSION));
	hash = NativeAdhoc_Fnv1a(hash, CTR_NATIVE_BUILD_ID, strlen(CTR_NATIVE_BUILD_ID));
	hash = NativeAdhoc_Fnv1a(hash, values, sizeof(values));
	return hash;
}

static int NativeAdhoc_IsNetError(int result, u32 error)
{
	return (u32)result == error;
}

static u64 NativeAdhoc_Now(void)
{
	return sceKernelGetProcessTimeWide();
}

static void NativeAdhoc_SetDeadline(u64 timeoutUs)
{
	s_nativeAdhocProgressTimeoutUs = timeoutUs;
	s_nativeAdhocDeadlineUs = timeoutUs == 0 ? 0 : NativeAdhoc_Now() + timeoutUs;
}

static void NativeAdhoc_TouchProgress(void)
{
	if (s_nativeAdhocProgressTimeoutUs != 0)
	{
		s_nativeAdhocDeadlineUs = NativeAdhoc_Now() + s_nativeAdhocProgressTimeoutUs;
	}
}

static int NativeAdhoc_DeadlineExpired(void)
{
	return (s_nativeAdhocDeadlineUs != 0) && (NativeAdhoc_Now() >= s_nativeAdhocDeadlineUs);
}

static enum NativeAdhocRole NativeAdhoc_RemoteRole(void)
{
	return s_nativeAdhocRole == NATIVE_ADHOC_ROLE_HOST ? NATIVE_ADHOC_ROLE_CLIENT : NATIVE_ADHOC_ROLE_HOST;
}

static void NativeAdhoc_BuildHello(u8 *hello, enum NativeAdhocRole role)
{
	memset(hello, 0, NATIVE_ADHOC_HELLO_SIZE);
	NativeAdhoc_WriteU32(hello, NATIVE_ADHOC_WIRE_MAGIC);
	NativeAdhoc_WriteU16(hello + 4, NATIVE_ADHOC_PROTOCOL_VERSION);
	hello[6] = (u8)role;
	hello[7] = NATIVE_ADHOC_HELLO_SIZE;
	NativeAdhoc_WriteU32(hello + 8, BUILD);
	NativeAdhoc_WriteU16(hello + 12, sizeof(struct PlatformInputPadSnapshot));
	NativeAdhoc_WriteU16(hello + 14, sizeof(struct NativeAdhocTimingState));
	NativeAdhoc_WriteU32(hello + 16, NativeAdhoc_CompatibilityHash());
	NativeAdhoc_WriteU32(hello + 20, NativeAdhoc_Crc32(hello, 20));
}

static int NativeAdhoc_ValidateHello(const void *data, int size, enum NativeAdhocRole expectedRole)
{
	const u8 *hello = (const u8 *)data;

	return (data != NULL) &&
	       (size == NATIVE_ADHOC_HELLO_SIZE) &&
	       (NativeAdhoc_ReadU32(hello) == NATIVE_ADHOC_WIRE_MAGIC) &&
	       (NativeAdhoc_ReadU16(hello + 4) == NATIVE_ADHOC_PROTOCOL_VERSION) &&
	       (hello[6] == expectedRole) &&
	       (hello[7] == NATIVE_ADHOC_HELLO_SIZE) &&
	       (NativeAdhoc_ReadU32(hello + 8) == BUILD) &&
	       (NativeAdhoc_ReadU16(hello + 12) == sizeof(struct PlatformInputPadSnapshot)) &&
	       (NativeAdhoc_ReadU16(hello + 14) == sizeof(struct NativeAdhocTimingState)) &&
	       (NativeAdhoc_ReadU32(hello + 16) == NativeAdhoc_CompatibilityHash()) &&
	       (NativeAdhoc_ReadU32(hello + 20) == NativeAdhoc_Crc32(hello, 20));
}

static void NativeAdhoc_MatchingCallback(int id, int event, SceNetInAddr *address, SceSize optionSize, void *option)
{
	(void)id;

	if (event == SCE_NET_ADHOC_MATCHING_EVENT_ERROR)
	{
		__atomic_fetch_or(&s_nativeAdhocMatchingFlags, NATIVE_ADHOC_MATCH_FLAG_ERROR, __ATOMIC_RELEASE);
		return;
	}
	if (address == NULL)
	{
		return;
	}

	switch (event)
	{
	case SCE_NET_ADHOC_MATCHING_EVENT_HELLO:
		if ((s_nativeAdhocRole == NATIVE_ADHOC_ROLE_CLIENT) &&
		    NativeAdhoc_ValidateHello(option, (int)optionSize, NATIVE_ADHOC_ROLE_HOST))
		{
			__atomic_store_n(&s_nativeAdhocCandidateAddress, address->s_addr, __ATOMIC_RELEASE);
			__atomic_fetch_or(&s_nativeAdhocMatchingFlags, NATIVE_ADHOC_MATCH_FLAG_CANDIDATE, __ATOMIC_RELEASE);
		}
		break;

	case SCE_NET_ADHOC_MATCHING_EVENT_REQUEST:
		if ((s_nativeAdhocRole == NATIVE_ADHOC_ROLE_HOST) &&
		    NativeAdhoc_ValidateHello(option, (int)optionSize, NATIVE_ADHOC_ROLE_CLIENT))
		{
			__atomic_store_n(&s_nativeAdhocRequestAddress, address->s_addr, __ATOMIC_RELEASE);
			__atomic_fetch_or(&s_nativeAdhocMatchingFlags, NATIVE_ADHOC_MATCH_FLAG_REQUEST, __ATOMIC_RELEASE);
		}
		else
		{
			__atomic_store_n(&s_nativeAdhocRejectAddress, address->s_addr, __ATOMIC_RELEASE);
			__atomic_fetch_or(&s_nativeAdhocMatchingFlags, NATIVE_ADHOC_MATCH_FLAG_REJECT, __ATOMIC_RELEASE);
		}
		break;

	case SCE_NET_ADHOC_MATCHING_EVENT_ESTABLISHED:
		__atomic_store_n(&s_nativeAdhocEstablishedAddress, address->s_addr, __ATOMIC_RELEASE);
		__atomic_fetch_or(&s_nativeAdhocMatchingFlags, NATIVE_ADHOC_MATCH_FLAG_ESTABLISHED, __ATOMIC_RELEASE);
		break;

	case SCE_NET_ADHOC_MATCHING_EVENT_DENY:
	case SCE_NET_ADHOC_MATCHING_EVENT_LEAVE:
	case SCE_NET_ADHOC_MATCHING_EVENT_CANCEL:
	case SCE_NET_ADHOC_MATCHING_EVENT_TIMEOUT:
	case SCE_NET_ADHOC_MATCHING_EVENT_BYE:
		__atomic_fetch_or(&s_nativeAdhocMatchingFlags, NATIVE_ADHOC_MATCH_FLAG_LOST, __ATOMIC_RELEASE);
		break;
	}
}

static void NativeAdhoc_CloseSocket(int *socket)
{
	if (*socket < 0)
	{
		return;
	}

	sceNetSocketAbort(*socket, 0);
	sceNetShutdown(*socket, SCE_NET_SHUT_RDWR);
	sceNetSocketClose(*socket);
	*socket = -1;
}

static void NativeAdhoc_StopMatching(void)
{
	if (s_nativeAdhocMatchingId < 0)
	{
		return;
	}

	sceNetAdhocMatchingStop(s_nativeAdhocMatchingId);
	sceNetAdhocMatchingDelete(s_nativeAdhocMatchingId);
	s_nativeAdhocMatchingId = -1;
	__atomic_store_n(&s_nativeAdhocMatchingFlags, 0, __ATOMIC_RELEASE);
}

static void NativeAdhoc_FreeSnapshot(void)
{
	free(s_nativeAdhocSnapshot);
	s_nativeAdhocSnapshot = NULL;
	s_nativeAdhocSnapshotSize = 0;
	s_nativeAdhocSnapshotCrc = 0;
}

static void NativeAdhoc_ForceDeterministicModes(void)
{
	gNativeMirrorModeEnabled = 0;
	gNativeGhostReplayMode = 0;
	gNativeBossFightMode = 0;
	gNativeBossFightBossID = 0;
	NativeGhostInput_ClearSelection();
}

static void NativeAdhoc_SaveNativeModes(void)
{
	if (!s_nativeAdhocNativeModeSaved)
	{
		s_nativeAdhocSavedMirrorMode = gNativeMirrorModeEnabled;
		s_nativeAdhocNativeModeSaved = 1;
	}
	NativeAdhoc_ForceDeterministicModes();
}

static void NativeAdhoc_RestoreNativeModes(void)
{
	if (s_nativeAdhocNativeModeSaved)
	{
		gNativeMirrorModeEnabled = s_nativeAdhocSavedMirrorMode;
		s_nativeAdhocNativeModeSaved = 0;
	}
	gNativeGhostReplayMode = 0;
	gNativeBossFightMode = 0;
	gNativeBossFightBossID = 0;
	NativeGhostInput_ClearSelection();
}

static void NativeAdhoc_ClearInputOverride(void)
{
	Platform_InputClearInstalledPadSnapshots();
	Platform_InputUpdate();
}

static void NativeAdhoc_CleanupNetwork(void)
{
	NativeAdhoc_CloseSocket(&s_nativeAdhocSocket);
	NativeAdhoc_CloseSocket(&s_nativeAdhocListener);
	NativeAdhoc_StopMatching();
	NativeAdhoc_FreeSnapshot();

	if (s_nativeAdhocMatchingInitialized)
	{
		sceNetAdhocMatchingTerm();
		s_nativeAdhocMatchingInitialized = 0;
	}

	if (s_nativeAdhocNetCtlInitialized)
	{
		int state = SCE_NETCTL_STATE_CONNECTED;

		sceNetCtlAdhocDisconnect();
		for (int attempt = 0; attempt < 100; attempt++)
		{
			sceNetCtlCheckCallback();
			if ((sceNetCtlAdhocGetState(&state) >= 0) && (state == SCE_NETCTL_STATE_DISCONNECTED))
			{
				break;
			}
			sceKernelDelayThread(10000);
		}

		sceNetCtlTerm();
		s_nativeAdhocNetCtlInitialized = 0;
	}

	if (s_nativeAdhocNetInitialized)
	{
		sceNetTerm();
		s_nativeAdhocNetInitialized = 0;
	}

	if (s_nativeAdhocMatchingModuleLoaded)
	{
		sceSysmoduleUnloadModule(SCE_SYSMODULE_NET_ADHOC_MATCHING);
		s_nativeAdhocMatchingModuleLoaded = 0;
	}

	if (s_nativeAdhocNetModuleLoaded)
	{
		sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
		s_nativeAdhocNetModuleLoaded = 0;
	}
}

static void NativeAdhoc_ResetRuntime(void)
{
	s_nativeAdhocRole = NATIVE_ADHOC_ROLE_NONE;
	s_nativeAdhocState = NATIVE_ADHOC_STATE_IDLE;
	s_nativeAdhocStatus = NATIVE_ADHOC_STATUS_IDLE;
	s_nativeAdhocDialogActive = 0;
	s_nativeAdhocDialogCancelRequested = 0;
	s_nativeAdhocDialogFailurePending = 0;
	s_nativeAdhocMatchingId = -1;
	s_nativeAdhocListener = -1;
	s_nativeAdhocSocket = -1;
	s_nativeAdhocTargetSelected = 0;
	s_nativeAdhocSessionSynchronized = 0;
	s_nativeAdhocLoadStableTicks = 0;
	s_nativeAdhocFramePending = 0;
	s_nativeAdhocEpoch = 0;
	s_nativeAdhocPendingEpoch = 0;
	s_nativeAdhocFrame = 0;
	s_nativeAdhocSnapshotReason = 0;
	s_nativeAdhocSnapshotLevelId = 0;
	s_nativeAdhocSendOffset = 0;
	s_nativeAdhocRecvOffset = 0;
	s_nativeAdhocFrameSendOffset = 0;
	s_nativeAdhocFrameRecvOffset = 0;
	s_nativeAdhocDeadlineUs = 0;
	s_nativeAdhocProgressTimeoutUs = 0;
	memset(&s_nativeAdhocPeerAddress, 0, sizeof(s_nativeAdhocPeerAddress));
	memset(&s_nativeAdhocTiming, 0, sizeof(s_nativeAdhocTiming));
	__atomic_store_n(&s_nativeAdhocMatchingFlags, 0, __ATOMIC_RELEASE);
	__atomic_store_n(&s_nativeAdhocCandidateAddress, 0, __ATOMIC_RELEASE);
	__atomic_store_n(&s_nativeAdhocRequestAddress, 0, __ATOMIC_RELEASE);
	__atomic_store_n(&s_nativeAdhocRejectAddress, 0, __ATOMIC_RELEASE);
	__atomic_store_n(&s_nativeAdhocEstablishedAddress, 0, __ATOMIC_RELEASE);
}

static void NativeAdhoc_Fail(const char *operation, int result)
{
	const int gameplayFailure = s_nativeAdhocSessionSynchronized;

	Platform_LogError("[CTR Adhoc] %s failed: 0x%08x\n", operation, (u32)result);
	s_nativeAdhocGameplayFailure |= gameplayFailure;

	if (s_nativeAdhocDialogActive)
	{
		if (!s_nativeAdhocDialogCancelRequested)
		{
			sceNetCheckDialogAbort();
			s_nativeAdhocDialogCancelRequested = 1;
		}
		s_nativeAdhocDialogFailurePending = 1;
		s_nativeAdhocState = NATIVE_ADHOC_STATE_DIALOG_ABORTING;
		s_nativeAdhocStatus = NATIVE_ADHOC_STATUS_ERROR;
		return;
	}

	NativeAdhoc_CleanupNetwork();
	NativeAdhoc_ClearInputOverride();
	NativeAdhoc_RestoreNativeModes();
	s_nativeAdhocState = NATIVE_ADHOC_STATE_ERROR;
	s_nativeAdhocStatus = NATIVE_ADHOC_STATUS_ERROR;
	s_nativeAdhocRole = NATIVE_ADHOC_ROLE_NONE;
	s_nativeAdhocFramePending = 0;
}

static int NativeAdhoc_InitNetwork(void)
{
	SceNetInitParam netParam;
	int result;

	result = sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
	if (result < 0)
	{
		NativeAdhoc_Fail("sceSysmoduleLoadModule(SCE_SYSMODULE_NET)", result);
		return 0;
	}
	s_nativeAdhocNetModuleLoaded = 1;

	result = sceSysmoduleLoadModule(SCE_SYSMODULE_NET_ADHOC_MATCHING);
	if (result < 0)
	{
		NativeAdhoc_Fail("sceSysmoduleLoadModule(SCE_SYSMODULE_NET_ADHOC_MATCHING)", result);
		return 0;
	}
	s_nativeAdhocMatchingModuleLoaded = 1;

	memset(&netParam, 0, sizeof(netParam));
	memset(s_nativeAdhocNetMemory, 0, sizeof(s_nativeAdhocNetMemory));
	netParam.memory = s_nativeAdhocNetMemory;
	netParam.size = sizeof(s_nativeAdhocNetMemory);
	netParam.flags = 0;

	result = sceNetInit(&netParam);
	if (result < 0)
	{
		NativeAdhoc_Fail("sceNetInit", result);
		return 0;
	}
	s_nativeAdhocNetInitialized = 1;

	result = sceNetCtlInit();
	if (result < 0)
	{
		NativeAdhoc_Fail("sceNetCtlInit", result);
		return 0;
	}
	s_nativeAdhocNetCtlInitialized = 1;
	return 1;
}

static int NativeAdhoc_InitMatching(void)
{
	int result;

	memset(s_nativeAdhocMatchingPool, 0, sizeof(s_nativeAdhocMatchingPool));
	result = sceNetAdhocMatchingInit(sizeof(s_nativeAdhocMatchingPool), s_nativeAdhocMatchingPool);
	if (result < 0)
	{
		NativeAdhoc_Fail("sceNetAdhocMatchingInit", result);
		return 0;
	}
	s_nativeAdhocMatchingInitialized = 1;
	return 1;
}

static int NativeAdhoc_StartDialog(void)
{
	SceNetCheckDialogParam param;
	static const char communicationId[9] = {'C', 'T', 'R', '0', '0', '0', '0', '0', '4'};
	int result;

	sceNetCheckDialogParamInit(&param);
	param.mode = SCE_NETCHECK_DIALOG_MODE_ADHOC_CONN;
	memcpy(param.npCommunicationId.data, communicationId, sizeof(communicationId));
	param.npCommunicationId.term = '\0';
	param.npCommunicationId.num = 0;

	result = sceNetCheckDialogInit(&param);
	if (result < 0)
	{
		NativeAdhoc_Fail("sceNetCheckDialogInit", result);
		return 0;
	}

	s_nativeAdhocDialogActive = 1;
	s_nativeAdhocDialogCancelRequested = 0;
	s_nativeAdhocDialogFailurePending = 0;
	s_nativeAdhocState = NATIVE_ADHOC_STATE_DIALOG;
	s_nativeAdhocStatus = NATIVE_ADHOC_STATUS_CONNECTING;
	return 1;
}

static int NativeAdhoc_StartMatching(void)
{
	const int host = s_nativeAdhocRole == NATIVE_ADHOC_ROLE_HOST;
	const SceNetAdhocMatchingMode mode = host ? SCE_NET_ADHOC_MATCHING_MODE_PARENT : SCE_NET_ADHOC_MATCHING_MODE_CHILD;
	const unsigned int helloInterval = host ? NATIVE_ADHOC_MATCHING_INTERVAL_US : 0;
	const unsigned int keepAliveInterval = host ? NATIVE_ADHOC_MATCHING_INTERVAL_US : 0;
	int result;

	__atomic_store_n(&s_nativeAdhocMatchingFlags, 0, __ATOMIC_RELEASE);
	s_nativeAdhocTargetSelected = 0;

	result = sceNetAdhocMatchingCreate(
	    mode,
	    2,
	    NATIVE_ADHOC_MATCHING_PORT,
	    2048,
	    helloInterval,
	    keepAliveInterval,
	    NATIVE_ADHOC_MATCHING_RETRY_COUNT,
	    NATIVE_ADHOC_MATCHING_INTERVAL_US,
	    NativeAdhoc_MatchingCallback);
	if (result < 0)
	{
		NativeAdhoc_Fail("sceNetAdhocMatchingCreate", result);
		return 0;
	}
	s_nativeAdhocMatchingId = result;

	NativeAdhoc_BuildHello(s_nativeAdhocHelloSend, s_nativeAdhocRole);
	result = sceNetAdhocMatchingStart(
	    s_nativeAdhocMatchingId,
	    NATIVE_ADHOC_MATCHING_PRIORITY,
	    NATIVE_ADHOC_MATCHING_STACK_SIZE,
	    SCE_KERNEL_THREAD_CPU_AFFINITY_MASK_DEFAULT,
	    host ? NATIVE_ADHOC_HELLO_SIZE : 0,
	    host ? s_nativeAdhocHelloSend : NULL);
	if (result < 0)
	{
		NativeAdhoc_Fail("sceNetAdhocMatchingStart", result);
		return 0;
	}

	s_nativeAdhocState = NATIVE_ADHOC_STATE_MATCHING;
	s_nativeAdhocStatus = host ? NATIVE_ADHOC_STATUS_WAITING : NATIVE_ADHOC_STATUS_SEARCHING;
	NativeAdhoc_SetDeadline(NATIVE_ADHOC_MATCHING_TIMEOUT_US);
	return 1;
}

static int NativeAdhoc_SetNonblocking(int socket, int nonblocking)
{
	return sceNetSetsockopt(socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_NBIO, &nonblocking, sizeof(nonblocking));
}

static int NativeAdhoc_SetTcpNoDelay(int socket)
{
	int noDelay = 1;
	return sceNetSetsockopt(socket, SCE_NET_IPPROTO_TCP, SCE_NET_TCP_NODELAY, &noDelay, sizeof(noDelay));
}

static int NativeAdhoc_CreateSocket(void)
{
	SceNetSockaddrIn address;
	int result;

	if (s_nativeAdhocRole == NATIVE_ADHOC_ROLE_HOST)
	{
		s_nativeAdhocListener = sceNetSocket("ctr-adhoc-listener", SCE_NET_AF_INET, SCE_NET_SOCK_STREAM_P2P, 0);
		if (s_nativeAdhocListener < 0)
		{
			NativeAdhoc_Fail("sceNetSocket(listener)", s_nativeAdhocListener);
			return 0;
		}

		result = NativeAdhoc_SetNonblocking(s_nativeAdhocListener, 1);
		if (result < 0)
		{
			NativeAdhoc_Fail("sceNetSetsockopt(listener NBIO)", result);
			return 0;
		}

		memset(&address, 0, sizeof(address));
		address.sin_len = sizeof(address);
		address.sin_family = SCE_NET_AF_INET;
		address.sin_port = sceNetHtons(NATIVE_ADHOC_STREAM_PORT);
		address.sin_addr.s_addr = SCE_NET_INADDR_ANY;
		address.sin_vport = sceNetHtons(SCE_NET_ADHOC_PORT);

		result = sceNetBind(s_nativeAdhocListener, (const SceNetSockaddr *)&address, sizeof(address));
		if (result < 0)
		{
			NativeAdhoc_Fail("sceNetBind", result);
			return 0;
		}

		result = sceNetListen(s_nativeAdhocListener, 1);
		if (result < 0)
		{
			NativeAdhoc_Fail("sceNetListen", result);
			return 0;
		}

		s_nativeAdhocState = NATIVE_ADHOC_STATE_HOST_ACCEPT;
	}
	else
	{
		s_nativeAdhocSocket = sceNetSocket("ctr-adhoc-client", SCE_NET_AF_INET, SCE_NET_SOCK_STREAM_P2P, 0);
		if (s_nativeAdhocSocket < 0)
		{
			NativeAdhoc_Fail("sceNetSocket(client)", s_nativeAdhocSocket);
			return 0;
		}

		result = NativeAdhoc_SetNonblocking(s_nativeAdhocSocket, 1);
		if (result < 0)
		{
			NativeAdhoc_Fail("sceNetSetsockopt(client NBIO)", result);
			return 0;
		}

		s_nativeAdhocState = NATIVE_ADHOC_STATE_CLIENT_CONNECT;
	}

	s_nativeAdhocStatus = NATIVE_ADHOC_STATUS_SYNCHRONIZING;
	NativeAdhoc_SetDeadline(NATIVE_ADHOC_CONNECT_TIMEOUT_US);
	return 1;
}

static void NativeAdhoc_StreamConnected(void)
{
	int result;

	NativeAdhoc_CloseSocket(&s_nativeAdhocListener);
	NativeAdhoc_StopMatching();

	result = NativeAdhoc_SetNonblocking(s_nativeAdhocSocket, 1);
	if (result < 0)
	{
		NativeAdhoc_Fail("sceNetSetsockopt(stream NBIO)", result);
		return;
	}
	result = NativeAdhoc_SetTcpNoDelay(s_nativeAdhocSocket);
	if (result < 0)
	{
		NativeAdhoc_Fail("sceNetSetsockopt(TCP_NODELAY)", result);
		return;
	}

	NativeAdhoc_BuildHello(s_nativeAdhocHelloSend, s_nativeAdhocRole);
	memset(s_nativeAdhocHelloRecv, 0, sizeof(s_nativeAdhocHelloRecv));
	s_nativeAdhocSendOffset = 0;
	s_nativeAdhocRecvOffset = 0;
	s_nativeAdhocState = NATIVE_ADHOC_STATE_HELLO;
	s_nativeAdhocStatus = NATIVE_ADHOC_STATUS_SYNCHRONIZING;
	NativeAdhoc_SetDeadline(NATIVE_ADHOC_CONNECT_TIMEOUT_US);
}

static int NativeAdhoc_StepSend(const void *data, u32 size, u32 *offset)
{
	const u8 *bytes = (const u8 *)data;
	u32 remaining;
	u32 chunk;
	int result;

	if (*offset >= size)
	{
		return 1;
	}

	remaining = size - *offset;
	chunk = remaining > NATIVE_ADHOC_BOOTSTRAP_CHUNK_SIZE ? NATIVE_ADHOC_BOOTSTRAP_CHUNK_SIZE : remaining;
	result = sceNetSend(s_nativeAdhocSocket, bytes + *offset, chunk, SCE_NET_MSG_DONTWAIT);
	if (result > 0)
	{
		*offset += (u32)result;
		NativeAdhoc_TouchProgress();
		return *offset == size;
	}
	if (result == 0)
	{
		NativeAdhoc_Fail("stream send closed", result);
		return -1;
	}
	if (NativeAdhoc_IsNetError(result, SCE_NET_ERROR_EAGAIN) ||
	    NativeAdhoc_IsNetError(result, SCE_NET_ERROR_EWOULDBLOCK) ||
	    NativeAdhoc_IsNetError(result, SCE_NET_ERROR_EINTR))
	{
		return 0;
	}

	NativeAdhoc_Fail("sceNetSend", result);
	return -1;
}

static int NativeAdhoc_StepRecv(void *data, u32 size, u32 *offset)
{
	u8 *bytes = (u8 *)data;
	u32 remaining;
	u32 chunk;
	int result;

	if (*offset >= size)
	{
		return 1;
	}

	remaining = size - *offset;
	chunk = remaining > NATIVE_ADHOC_BOOTSTRAP_CHUNK_SIZE ? NATIVE_ADHOC_BOOTSTRAP_CHUNK_SIZE : remaining;
	result = sceNetRecv(s_nativeAdhocSocket, bytes + *offset, chunk, SCE_NET_MSG_DONTWAIT);
	if (result > 0)
	{
		*offset += (u32)result;
		NativeAdhoc_TouchProgress();
		return *offset == size;
	}
	if (result == 0)
	{
		NativeAdhoc_Fail("stream receive closed", result);
		return -1;
	}
	if (NativeAdhoc_IsNetError(result, SCE_NET_ERROR_EAGAIN) ||
	    NativeAdhoc_IsNetError(result, SCE_NET_ERROR_EWOULDBLOCK) ||
	    NativeAdhoc_IsNetError(result, SCE_NET_ERROR_EINTR))
	{
		return 0;
	}

	NativeAdhoc_Fail("sceNetRecv", result);
	return -1;
}

static void NativeAdhoc_BuildControl(u8 *control, u16 type, u32 epoch, u32 value)
{
	memset(control, 0, NATIVE_ADHOC_CONTROL_SIZE);
	NativeAdhoc_WriteU32(control, NATIVE_ADHOC_WIRE_MAGIC);
	NativeAdhoc_WriteU16(control + 4, NATIVE_ADHOC_PROTOCOL_VERSION);
	NativeAdhoc_WriteU16(control + 6, type);
	NativeAdhoc_WriteU16(control + 8, NATIVE_ADHOC_CONTROL_SIZE);
	control[10] = (u8)s_nativeAdhocRole;
	NativeAdhoc_WriteU32(control + 12, epoch);
	NativeAdhoc_WriteU32(control + 16, value);
	NativeAdhoc_WriteU32(control + 20, NativeAdhoc_Crc32(control, 20));
}

static int NativeAdhoc_ValidateControl(const u8 *control, u16 type, u32 epoch, u32 value)
{
	return (NativeAdhoc_ReadU32(control) == NATIVE_ADHOC_WIRE_MAGIC) &&
	       (NativeAdhoc_ReadU16(control + 4) == NATIVE_ADHOC_PROTOCOL_VERSION) &&
	       (NativeAdhoc_ReadU16(control + 6) == type) &&
	       (NativeAdhoc_ReadU16(control + 8) == NATIVE_ADHOC_CONTROL_SIZE) &&
	       (control[10] == NativeAdhoc_RemoteRole()) &&
	       (control[11] == 0) &&
	       (NativeAdhoc_ReadU32(control + 12) == epoch) &&
	       (NativeAdhoc_ReadU32(control + 16) == value) &&
	       (NativeAdhoc_ReadU32(control + 20) == NativeAdhoc_Crc32(control, 20));
}

static int NativeAdhoc_CheckLink(void)
{
	int state = SCE_NETCTL_STATE_DISCONNECTED;
	int result = sceNetCtlAdhocGetState(&state);

	if (result < 0)
	{
		NativeAdhoc_Fail("sceNetCtlAdhocGetState", result);
		return 0;
	}
	if (state != SCE_NETCTL_STATE_CONNECTED)
	{
		NativeAdhoc_Fail("ad hoc link disconnected", state);
		return 0;
	}
	return 1;
}

static int NativeAdhoc_IsGameStable(void)
{
	return (sdata != NULL) &&
	       (sdata->gGT != NULL) &&
	       (sdata->mainGameState == 3) &&
	       (sdata->Loading.stage == LOAD_IDLE);
}

static void NativeAdhoc_EndSessionNormally(void)
{
	NativeAdhoc_CleanupNetwork();
	NativeAdhoc_ClearInputOverride();
	NativeAdhoc_RestoreNativeModes();
	if ((sdata != NULL) && (sdata->gGT != NULL))
	{
		sdata->gGT->numPlyrNextGame = 1;
	}
	NativeAdhoc_ResetRuntime();
}

static void NativeAdhoc_Activate(void)
{
	s_nativeAdhocEpoch = s_nativeAdhocPendingEpoch;
	s_nativeAdhocFrame = 0;
	s_nativeAdhocFramePending = 0;
	s_nativeAdhocFrameSendOffset = 0;
	s_nativeAdhocFrameRecvOffset = 0;
	s_nativeAdhocLoadStableTicks = 0;
	s_nativeAdhocSessionSynchronized = 1;
	s_nativeAdhocState = NATIVE_ADHOC_STATE_ACTIVE;
	s_nativeAdhocStatus = NATIVE_ADHOC_STATUS_CONNECTED;
	NativeAdhoc_SetDeadline(0);
	NativeAdhoc_ClearInputOverride();
	NativeAdhoc_ForceDeterministicModes();
}

static int NativeAdhoc_CaptureSnapshot(u32 reason, u32 targetEpoch)
{
	const int size = NativeCheckpoint_GetSize();
	int captured;

	if ((size <= 0) || (size > NATIVE_ADHOC_SNAPSHOT_MAX_SIZE))
	{
		NativeAdhoc_Fail("NativeCheckpoint_GetSize", size);
		return 0;
	}

	NativeAdhoc_FreeSnapshot();
	s_nativeAdhocSnapshot = (u8 *)malloc((size_t)size);
	if (s_nativeAdhocSnapshot == NULL)
	{
		NativeAdhoc_Fail("snapshot allocation", -1);
		return 0;
	}

	captured = NativeCheckpoint_Capture(s_nativeAdhocSnapshot, size);
	if (!captured)
	{
		NativeAdhoc_Fail("NativeCheckpoint_Capture", captured);
		return 0;
	}

	s_nativeAdhocSnapshotReason = reason;
	s_nativeAdhocPendingEpoch = targetEpoch;
	s_nativeAdhocSnapshotLevelId = (u16)sdata->gGT->levelID;
	s_nativeAdhocSnapshotSize = (u32)size;
	s_nativeAdhocSnapshotCrc = NativeAdhoc_Crc32(s_nativeAdhocSnapshot, s_nativeAdhocSnapshotSize);
	NativeAdhocTiming_Capture(&s_nativeAdhocTiming);

	memset(s_nativeAdhocSnapshotHeader, 0, sizeof(s_nativeAdhocSnapshotHeader));
	NativeAdhoc_WriteU32(s_nativeAdhocSnapshotHeader, NATIVE_ADHOC_WIRE_MAGIC);
	NativeAdhoc_WriteU16(s_nativeAdhocSnapshotHeader + 4, NATIVE_ADHOC_PROTOCOL_VERSION);
	NativeAdhoc_WriteU16(s_nativeAdhocSnapshotHeader + 6, NATIVE_ADHOC_WIRE_SNAPSHOT);
	NativeAdhoc_WriteU16(s_nativeAdhocSnapshotHeader + 8, NATIVE_ADHOC_SNAPSHOT_HEADER_SIZE);
	s_nativeAdhocSnapshotHeader[10] = (u8)reason;
	s_nativeAdhocSnapshotHeader[11] = NATIVE_ADHOC_ROLE_HOST;
	NativeAdhoc_WriteU32(s_nativeAdhocSnapshotHeader + 12, targetEpoch);
	NativeAdhoc_WriteU32(s_nativeAdhocSnapshotHeader + 16, s_nativeAdhocSnapshotSize);
	NativeAdhoc_WriteU32(s_nativeAdhocSnapshotHeader + 20, s_nativeAdhocSnapshotCrc);
	NativeAdhoc_WriteU32(s_nativeAdhocSnapshotHeader + 24, (u32)s_nativeAdhocTiming.vblankCount);
	NativeAdhoc_WriteU64(s_nativeAdhocSnapshotHeader + 28, s_nativeAdhocTiming.rootCounterValue);
	NativeAdhoc_WriteU64(s_nativeAdhocSnapshotHeader + 36, s_nativeAdhocTiming.rootCounterBase);
	NativeAdhoc_WriteU32(s_nativeAdhocSnapshotHeader + 44, BUILD);
	NativeAdhoc_WriteU16(s_nativeAdhocSnapshotHeader + 48, s_nativeAdhocSnapshotLevelId);
	NativeAdhoc_WriteU16(s_nativeAdhocSnapshotHeader + 50, 0);
	NativeAdhoc_WriteU32(s_nativeAdhocSnapshotHeader + 52, NativeAdhoc_Crc32(s_nativeAdhocSnapshotHeader, 52));

	s_nativeAdhocSendOffset = 0;
	s_nativeAdhocRecvOffset = 0;
	s_nativeAdhocState = NATIVE_ADHOC_STATE_HOST_SEND_HEADER;
	s_nativeAdhocStatus = NATIVE_ADHOC_STATUS_SYNCHRONIZING;
	NativeAdhoc_SetDeadline(NATIVE_ADHOC_BOOTSTRAP_TIMEOUT_US);
	return 1;
}

static int NativeAdhoc_ValidateSnapshotHeader(void)
{
	const u32 expectedEpoch = s_nativeAdhocSnapshotReason == NATIVE_ADHOC_SNAPSHOT_INITIAL ? 1 : s_nativeAdhocEpoch + 1;

	if ((NativeAdhoc_ReadU32(s_nativeAdhocSnapshotHeader) != NATIVE_ADHOC_WIRE_MAGIC) ||
	    (NativeAdhoc_ReadU16(s_nativeAdhocSnapshotHeader + 4) != NATIVE_ADHOC_PROTOCOL_VERSION) ||
	    (NativeAdhoc_ReadU16(s_nativeAdhocSnapshotHeader + 6) != NATIVE_ADHOC_WIRE_SNAPSHOT) ||
	    (NativeAdhoc_ReadU16(s_nativeAdhocSnapshotHeader + 8) != NATIVE_ADHOC_SNAPSHOT_HEADER_SIZE) ||
	    (s_nativeAdhocSnapshotHeader[10] != s_nativeAdhocSnapshotReason) ||
	    (s_nativeAdhocSnapshotHeader[11] != NATIVE_ADHOC_ROLE_HOST) ||
	    (NativeAdhoc_ReadU32(s_nativeAdhocSnapshotHeader + 12) != expectedEpoch) ||
	    (NativeAdhoc_ReadU32(s_nativeAdhocSnapshotHeader + 44) != BUILD) ||
	    (NativeAdhoc_ReadU16(s_nativeAdhocSnapshotHeader + 50) != 0) ||
	    (NativeAdhoc_ReadU32(s_nativeAdhocSnapshotHeader + 52) != NativeAdhoc_Crc32(s_nativeAdhocSnapshotHeader, 52)))
	{
		return 0;
	}

	s_nativeAdhocPendingEpoch = NativeAdhoc_ReadU32(s_nativeAdhocSnapshotHeader + 12);
	s_nativeAdhocSnapshotSize = NativeAdhoc_ReadU32(s_nativeAdhocSnapshotHeader + 16);
	s_nativeAdhocSnapshotCrc = NativeAdhoc_ReadU32(s_nativeAdhocSnapshotHeader + 20);
	s_nativeAdhocTiming.vblankCount = (s32)NativeAdhoc_ReadU32(s_nativeAdhocSnapshotHeader + 24);
	s_nativeAdhocTiming.rootCounterValue = NativeAdhoc_ReadU64(s_nativeAdhocSnapshotHeader + 28);
	s_nativeAdhocTiming.rootCounterBase = NativeAdhoc_ReadU64(s_nativeAdhocSnapshotHeader + 36);
	s_nativeAdhocSnapshotLevelId = NativeAdhoc_ReadU16(s_nativeAdhocSnapshotHeader + 48);
	return (s_nativeAdhocSnapshotSize > 0) && (s_nativeAdhocSnapshotSize <= NATIVE_ADHOC_SNAPSHOT_MAX_SIZE);
}

static void NativeAdhoc_UpdateDialog(void)
{
	SceNetCheckDialogResult dialogResult;
	const SceCommonDialogStatus status = sceNetCheckDialogGetStatus();
	const int failurePending = s_nativeAdhocDialogFailurePending;
	const int cancelRequested = s_nativeAdhocDialogCancelRequested;
	int result;

	if ((status == SCE_COMMON_DIALOG_STATUS_NONE) || (status == SCE_COMMON_DIALOG_STATUS_RUNNING))
	{
		return;
	}
	if (status != SCE_COMMON_DIALOG_STATUS_FINISHED)
	{
		NativeAdhoc_Fail("sceNetCheckDialogGetStatus", status);
		return;
	}

	memset(&dialogResult, 0, sizeof(dialogResult));
	result = sceNetCheckDialogGetResult(&dialogResult);
	if (result < 0)
	{
		s_nativeAdhocDialogFailurePending = 1;
		Platform_LogError("[CTR Adhoc] sceNetCheckDialogGetResult failed: 0x%08x\n", (u32)result);
	}

	result = sceNetCheckDialogTerm();
	s_nativeAdhocDialogActive = 0;
	if (result < 0)
	{
		s_nativeAdhocDialogFailurePending = 1;
		Platform_LogError("[CTR Adhoc] sceNetCheckDialogTerm failed: 0x%08x\n", (u32)result);
	}

	if (failurePending || s_nativeAdhocDialogFailurePending)
	{
		NativeAdhoc_CleanupNetwork();
		NativeAdhoc_RestoreNativeModes();
		s_nativeAdhocState = NATIVE_ADHOC_STATE_ERROR;
		s_nativeAdhocStatus = NATIVE_ADHOC_STATUS_ERROR;
		s_nativeAdhocRole = NATIVE_ADHOC_ROLE_NONE;
		return;
	}

	if (cancelRequested || (dialogResult.result != SCE_COMMON_DIALOG_RESULT_OK))
	{
		NativeAdhoc_CleanupNetwork();
		NativeAdhoc_RestoreNativeModes();
		NativeAdhoc_ResetRuntime();
		return;
	}

	if (NativeAdhoc_InitMatching())
	{
		NativeAdhoc_StartMatching();
	}
}

static void NativeAdhoc_UpdateMatching(void)
{
	u32 flags = __atomic_exchange_n(&s_nativeAdhocMatchingFlags, 0, __ATOMIC_ACQ_REL);
	SceNetInAddr address;
	int result;

	if (flags != 0)
	{
		NativeAdhoc_TouchProgress();
	}
	if ((flags & (NATIVE_ADHOC_MATCH_FLAG_ERROR | NATIVE_ADHOC_MATCH_FLAG_LOST)) != 0)
	{
		NativeAdhoc_Fail("ad hoc matching peer lost", (int)flags);
		return;
	}

	if ((flags & NATIVE_ADHOC_MATCH_FLAG_REJECT) != 0)
	{
		address.s_addr = __atomic_load_n(&s_nativeAdhocRejectAddress, __ATOMIC_ACQUIRE);
		sceNetAdhocMatchingCancelTarget(s_nativeAdhocMatchingId, &address);
	}

	if ((s_nativeAdhocRole == NATIVE_ADHOC_ROLE_HOST) && s_nativeAdhocTargetSelected &&
	    ((flags & NATIVE_ADHOC_MATCH_FLAG_REQUEST) != 0))
	{
		address.s_addr = __atomic_load_n(&s_nativeAdhocRequestAddress, __ATOMIC_ACQUIRE);
		sceNetAdhocMatchingCancelTarget(s_nativeAdhocMatchingId, &address);
	}
	else if (!s_nativeAdhocTargetSelected && (s_nativeAdhocRole == NATIVE_ADHOC_ROLE_CLIENT) &&
	         ((flags & NATIVE_ADHOC_MATCH_FLAG_CANDIDATE) != 0))
	{
		address.s_addr = __atomic_load_n(&s_nativeAdhocCandidateAddress, __ATOMIC_ACQUIRE);
		NativeAdhoc_BuildHello(s_nativeAdhocHelloSend, s_nativeAdhocRole);
		result = sceNetAdhocMatchingSelectTarget(s_nativeAdhocMatchingId, &address, NATIVE_ADHOC_HELLO_SIZE, s_nativeAdhocHelloSend);
		if (result < 0)
		{
			NativeAdhoc_Fail("sceNetAdhocMatchingSelectTarget(client)", result);
			return;
		}
		s_nativeAdhocTargetSelected = 1;
	}
	else if (!s_nativeAdhocTargetSelected && (s_nativeAdhocRole == NATIVE_ADHOC_ROLE_HOST) &&
	         ((flags & NATIVE_ADHOC_MATCH_FLAG_REQUEST) != 0))
	{
		address.s_addr = __atomic_load_n(&s_nativeAdhocRequestAddress, __ATOMIC_ACQUIRE);
		result = sceNetAdhocMatchingSelectTarget(s_nativeAdhocMatchingId, &address, 0, NULL);
		if (result < 0)
		{
			NativeAdhoc_Fail("sceNetAdhocMatchingSelectTarget(host)", result);
			return;
		}
		s_nativeAdhocTargetSelected = 1;
	}

	if ((flags & NATIVE_ADHOC_MATCH_FLAG_ESTABLISHED) != 0)
	{
		s_nativeAdhocPeerAddress.s_addr = __atomic_load_n(&s_nativeAdhocEstablishedAddress, __ATOMIC_ACQUIRE);
		NativeAdhoc_CreateSocket();
		return;
	}

	if (NativeAdhoc_DeadlineExpired())
	{
		NativeAdhoc_Fail("ad hoc matching timeout", -1);
	}
}

static void NativeAdhoc_UpdateHostAccept(void)
{
	SceNetSockaddrIn address;
	unsigned int addressSize = sizeof(address);
	int socket;
	int result;

	memset(&address, 0, sizeof(address));
	socket = sceNetAccept(s_nativeAdhocListener, (SceNetSockaddr *)&address, &addressSize);
	if (socket >= 0)
	{
		if (address.sin_addr.s_addr != s_nativeAdhocPeerAddress.s_addr)
		{
			sceNetSocketClose(socket);
			return;
		}
		s_nativeAdhocSocket = socket;
		result = NativeAdhoc_SetNonblocking(s_nativeAdhocSocket, 1);
		if (result < 0)
		{
			NativeAdhoc_Fail("sceNetSetsockopt(accepted NBIO)", result);
			return;
		}
		NativeAdhoc_StreamConnected();
		return;
	}

	if (!NativeAdhoc_IsNetError(socket, SCE_NET_ERROR_EAGAIN) &&
	    !NativeAdhoc_IsNetError(socket, SCE_NET_ERROR_EWOULDBLOCK) &&
	    !NativeAdhoc_IsNetError(socket, SCE_NET_ERROR_EINTR))
	{
		NativeAdhoc_Fail("sceNetAccept", socket);
		return;
	}
	if (NativeAdhoc_DeadlineExpired())
	{
		NativeAdhoc_Fail("sceNetAccept timeout", -1);
	}
}

static void NativeAdhoc_UpdateClientConnect(void)
{
	SceNetSockaddrIn address;
	SceNetSockaddrIn peer;
	unsigned int peerSize = sizeof(peer);
	int result;

	memset(&peer, 0, sizeof(peer));
	if (sceNetGetpeername(s_nativeAdhocSocket, (SceNetSockaddr *)&peer, &peerSize) == 0)
	{
		NativeAdhoc_StreamConnected();
		return;
	}

	memset(&address, 0, sizeof(address));
	address.sin_len = sizeof(address);
	address.sin_family = SCE_NET_AF_INET;
	address.sin_port = sceNetHtons(NATIVE_ADHOC_STREAM_PORT);
	address.sin_addr = s_nativeAdhocPeerAddress;
	address.sin_vport = sceNetHtons(SCE_NET_ADHOC_PORT);

	result = sceNetConnect(s_nativeAdhocSocket, (const SceNetSockaddr *)&address, sizeof(address));
	if ((result == 0) || NativeAdhoc_IsNetError(result, SCE_NET_ERROR_EISCONN))
	{
		NativeAdhoc_StreamConnected();
		return;
	}
	if (!NativeAdhoc_IsNetError(result, SCE_NET_ERROR_EINPROGRESS) &&
	    !NativeAdhoc_IsNetError(result, SCE_NET_ERROR_EALREADY) &&
	    !NativeAdhoc_IsNetError(result, SCE_NET_ERROR_EWOULDBLOCK) &&
	    !NativeAdhoc_IsNetError(result, SCE_NET_ERROR_EINTR))
	{
		NativeAdhoc_Fail("sceNetConnect", result);
		return;
	}
	if (NativeAdhoc_DeadlineExpired())
	{
		NativeAdhoc_Fail("sceNetConnect timeout", -1);
	}
}

static void NativeAdhoc_UpdateHello(void)
{
	const int sendResult = NativeAdhoc_StepSend(s_nativeAdhocHelloSend, sizeof(s_nativeAdhocHelloSend), &s_nativeAdhocSendOffset);
	const int recvResult = NativeAdhoc_StepRecv(s_nativeAdhocHelloRecv, sizeof(s_nativeAdhocHelloRecv), &s_nativeAdhocRecvOffset);

	if ((sendResult < 0) || (recvResult < 0))
	{
		return;
	}
	if ((sendResult > 0) && (recvResult > 0))
	{
		if (!NativeAdhoc_ValidateHello(s_nativeAdhocHelloRecv, sizeof(s_nativeAdhocHelloRecv), NativeAdhoc_RemoteRole()))
		{
			NativeAdhoc_Fail("protocol compatibility check", -1);
			return;
		}

		s_nativeAdhocSendOffset = 0;
		s_nativeAdhocRecvOffset = 0;
		if (s_nativeAdhocRole == NATIVE_ADHOC_ROLE_HOST)
		{
			s_nativeAdhocState = NATIVE_ADHOC_STATE_HOST_WAIT_STATE;
			NativeAdhoc_SetDeadline(NATIVE_ADHOC_CONTROL_TIMEOUT_US);
		}
		else
		{
			s_nativeAdhocSnapshotReason = NATIVE_ADHOC_SNAPSHOT_INITIAL;
			memset(s_nativeAdhocSnapshotHeader, 0, sizeof(s_nativeAdhocSnapshotHeader));
			s_nativeAdhocState = NATIVE_ADHOC_STATE_CLIENT_RECV_HEADER;
			NativeAdhoc_SetDeadline(NATIVE_ADHOC_BOOTSTRAP_TIMEOUT_US);
		}
		return;
	}

	if (NativeAdhoc_DeadlineExpired())
	{
		NativeAdhoc_Fail("protocol hello timeout", -1);
	}
}

static void NativeAdhoc_UpdateBootstrap(void)
{
	int result;

	switch (s_nativeAdhocState)
	{
	case NATIVE_ADHOC_STATE_HOST_SEND_HEADER:
		result = NativeAdhoc_StepSend(s_nativeAdhocSnapshotHeader, sizeof(s_nativeAdhocSnapshotHeader), &s_nativeAdhocSendOffset);
		if (result > 0)
		{
			s_nativeAdhocSendOffset = 0;
			s_nativeAdhocState = NATIVE_ADHOC_STATE_HOST_SEND_SNAPSHOT;
		}
		break;

	case NATIVE_ADHOC_STATE_HOST_SEND_SNAPSHOT:
		result = NativeAdhoc_StepSend(s_nativeAdhocSnapshot, s_nativeAdhocSnapshotSize, &s_nativeAdhocSendOffset);
		if (result > 0)
		{
			NativeAdhoc_FreeSnapshot();
			memset(s_nativeAdhocControlRecv, 0, sizeof(s_nativeAdhocControlRecv));
			s_nativeAdhocRecvOffset = 0;
			s_nativeAdhocState = NATIVE_ADHOC_STATE_HOST_WAIT_READY;
			NativeAdhoc_SetDeadline(NATIVE_ADHOC_CONTROL_TIMEOUT_US);
		}
		break;

	case NATIVE_ADHOC_STATE_HOST_WAIT_READY:
		result = NativeAdhoc_StepRecv(s_nativeAdhocControlRecv, sizeof(s_nativeAdhocControlRecv), &s_nativeAdhocRecvOffset);
		if (result > 0)
		{
			if (!NativeAdhoc_ValidateControl(
			        s_nativeAdhocControlRecv,
			        NATIVE_ADHOC_WIRE_READY,
			        s_nativeAdhocPendingEpoch,
			        s_nativeAdhocSnapshotReason))
			{
				NativeAdhoc_Fail("READY packet", -1);
				break;
			}
			NativeAdhoc_BuildControl(
			    s_nativeAdhocControlSend,
			    NATIVE_ADHOC_WIRE_START,
			    s_nativeAdhocPendingEpoch,
			    s_nativeAdhocSnapshotReason);
			s_nativeAdhocSendOffset = 0;
			s_nativeAdhocState = NATIVE_ADHOC_STATE_HOST_SEND_START;
			NativeAdhoc_SetDeadline(NATIVE_ADHOC_CONTROL_TIMEOUT_US);
		}
		break;

	case NATIVE_ADHOC_STATE_HOST_SEND_START:
		result = NativeAdhoc_StepSend(s_nativeAdhocControlSend, sizeof(s_nativeAdhocControlSend), &s_nativeAdhocSendOffset);
		if (result > 0)
		{
			NativeAdhoc_Activate();
		}
		break;

	case NATIVE_ADHOC_STATE_CLIENT_RECV_HEADER:
		result = NativeAdhoc_StepRecv(s_nativeAdhocSnapshotHeader, sizeof(s_nativeAdhocSnapshotHeader), &s_nativeAdhocRecvOffset);
		if (result > 0)
		{
			if (!NativeAdhoc_ValidateSnapshotHeader())
			{
				NativeAdhoc_Fail("snapshot header", -1);
				break;
			}
			s_nativeAdhocSnapshot = (u8 *)malloc(s_nativeAdhocSnapshotSize);
			if (s_nativeAdhocSnapshot == NULL)
			{
				NativeAdhoc_Fail("snapshot allocation", -1);
				break;
			}
			s_nativeAdhocRecvOffset = 0;
			s_nativeAdhocState = NATIVE_ADHOC_STATE_CLIENT_RECV_SNAPSHOT;
			NativeAdhoc_SetDeadline(NATIVE_ADHOC_BOOTSTRAP_TIMEOUT_US);
		}
		break;

	case NATIVE_ADHOC_STATE_CLIENT_RECV_SNAPSHOT:
		result = NativeAdhoc_StepRecv(s_nativeAdhocSnapshot, s_nativeAdhocSnapshotSize, &s_nativeAdhocRecvOffset);
		if (result > 0)
		{
			if (NativeAdhoc_Crc32(s_nativeAdhocSnapshot, s_nativeAdhocSnapshotSize) != s_nativeAdhocSnapshotCrc)
			{
				NativeAdhoc_Fail("snapshot CRC", -1);
				break;
			}
			if (!NativeCheckpoint_Restore(s_nativeAdhocSnapshot, s_nativeAdhocSnapshotSize))
			{
				NativeAdhoc_Fail("NativeCheckpoint_Restore", -1);
				break;
			}
			if (s_nativeAdhocSnapshotReason == NATIVE_ADHOC_SNAPSHOT_INITIAL)
			{
				s_nativeAdhocSessionSynchronized = 1;
			}
			NativeAdhocTiming_Restore(&s_nativeAdhocTiming);
			NativeAdhoc_FreeSnapshot();
			NativeAdhoc_ClearInputOverride();
			NativeAdhoc_ForceDeterministicModes();
			if ((sdata == NULL) || (sdata->gGT == NULL) || ((u16)sdata->gGT->levelID != s_nativeAdhocSnapshotLevelId))
			{
				NativeAdhoc_Fail("snapshot level validation", -1);
				break;
			}
			NativeAdhoc_BuildControl(
			    s_nativeAdhocControlSend,
			    NATIVE_ADHOC_WIRE_READY,
			    s_nativeAdhocPendingEpoch,
			    s_nativeAdhocSnapshotReason);
			s_nativeAdhocSendOffset = 0;
			s_nativeAdhocState = NATIVE_ADHOC_STATE_CLIENT_SEND_READY;
			NativeAdhoc_SetDeadline(NATIVE_ADHOC_CONTROL_TIMEOUT_US);
		}
		break;

	case NATIVE_ADHOC_STATE_CLIENT_SEND_READY:
		result = NativeAdhoc_StepSend(s_nativeAdhocControlSend, sizeof(s_nativeAdhocControlSend), &s_nativeAdhocSendOffset);
		if (result > 0)
		{
			memset(s_nativeAdhocControlRecv, 0, sizeof(s_nativeAdhocControlRecv));
			s_nativeAdhocRecvOffset = 0;
			s_nativeAdhocState = NATIVE_ADHOC_STATE_CLIENT_RECV_START;
			NativeAdhoc_SetDeadline(NATIVE_ADHOC_CONTROL_TIMEOUT_US);
		}
		break;

	case NATIVE_ADHOC_STATE_CLIENT_RECV_START:
		result = NativeAdhoc_StepRecv(s_nativeAdhocControlRecv, sizeof(s_nativeAdhocControlRecv), &s_nativeAdhocRecvOffset);
		if (result > 0)
		{
			if (!NativeAdhoc_ValidateControl(
			        s_nativeAdhocControlRecv,
			        NATIVE_ADHOC_WIRE_START,
			        s_nativeAdhocPendingEpoch,
			        s_nativeAdhocSnapshotReason))
			{
				NativeAdhoc_Fail("START packet", -1);
				break;
			}
			NativeAdhoc_Activate();
			}
			break;

	default:
		break;
	}

	if ((s_nativeAdhocState >= NATIVE_ADHOC_STATE_HOST_SEND_HEADER) &&
	    (s_nativeAdhocState <= NATIVE_ADHOC_STATE_CLIENT_RECV_START) &&
	    NativeAdhoc_DeadlineExpired())
	{
		NativeAdhoc_Fail("snapshot synchronization timeout", -1);
	}
}

static void NativeAdhoc_BeginLoadSynchronization(void)
{
	s_nativeAdhocFramePending = 0;
	s_nativeAdhocLoadStableTicks = 0;
	s_nativeAdhocState = NATIVE_ADHOC_STATE_LOAD_LOCAL;
	s_nativeAdhocStatus = NATIVE_ADHOC_STATUS_SYNCHRONIZING;
	NativeAdhoc_SetDeadline(NATIVE_ADHOC_LOAD_TIMEOUT_US);
	NativeAdhoc_ClearInputOverride();
}

static void NativeAdhoc_UpdateLoadLocal(void)
{
	if (!NativeAdhoc_CheckLink())
	{
		return;
	}

	if (!NativeAdhoc_IsGameStable())
	{
		s_nativeAdhocLoadStableTicks = 0;
		if (NativeAdhoc_DeadlineExpired())
		{
			NativeAdhoc_Fail("local level load timeout", -1);
		}
		return;
	}

	if ((sdata->gGT->levelID == MAIN_MENU_LEVEL) && (sdata->mainMenuState == MAIN_MENU_TITLE))
	{
		NativeAdhoc_EndSessionNormally();
		return;
	}

	s_nativeAdhocLoadStableTicks++;
	if (s_nativeAdhocLoadStableTicks < 2)
	{
		return;
	}

	NativeAdhoc_BuildControl(
	    s_nativeAdhocControlSend,
	    NATIVE_ADHOC_WIRE_LOAD_READY,
	    s_nativeAdhocEpoch,
	    (u32)(u16)sdata->gGT->levelID);
	memset(s_nativeAdhocControlRecv, 0, sizeof(s_nativeAdhocControlRecv));
	s_nativeAdhocSendOffset = 0;
	s_nativeAdhocRecvOffset = 0;
	s_nativeAdhocState = NATIVE_ADHOC_STATE_LOAD_READY;
	NativeAdhoc_SetDeadline(NATIVE_ADHOC_CONTROL_TIMEOUT_US);
}

static void NativeAdhoc_UpdateLoadReady(void)
{
	const u32 levelId = (u32)(u16)sdata->gGT->levelID;
	const int sendResult = NativeAdhoc_StepSend(s_nativeAdhocControlSend, sizeof(s_nativeAdhocControlSend), &s_nativeAdhocSendOffset);
	const int recvResult = NativeAdhoc_StepRecv(s_nativeAdhocControlRecv, sizeof(s_nativeAdhocControlRecv), &s_nativeAdhocRecvOffset);

	if ((sendResult < 0) || (recvResult < 0))
	{
		return;
	}
	if ((sendResult > 0) && (recvResult > 0))
	{
		if (!NativeAdhoc_ValidateControl(
		        s_nativeAdhocControlRecv,
		        NATIVE_ADHOC_WIRE_LOAD_READY,
		        s_nativeAdhocEpoch,
		        levelId))
		{
			NativeAdhoc_Fail("LOAD_READY packet", -1);
			return;
		}

		if (s_nativeAdhocRole == NATIVE_ADHOC_ROLE_HOST)
		{
			NativeAdhoc_CaptureSnapshot(NATIVE_ADHOC_SNAPSHOT_RESYNC, s_nativeAdhocEpoch + 1);
		}
		else
		{
			s_nativeAdhocSnapshotReason = NATIVE_ADHOC_SNAPSHOT_RESYNC;
			memset(s_nativeAdhocSnapshotHeader, 0, sizeof(s_nativeAdhocSnapshotHeader));
			s_nativeAdhocRecvOffset = 0;
			s_nativeAdhocState = NATIVE_ADHOC_STATE_CLIENT_RECV_HEADER;
			NativeAdhoc_SetDeadline(NATIVE_ADHOC_BOOTSTRAP_TIMEOUT_US);
		}
		return;
	}

	if (NativeAdhoc_DeadlineExpired())
	{
		NativeAdhoc_Fail("load synchronization timeout", -1);
	}
}

static u32 NativeAdhoc_FrameFingerprint(void)
{
	struct GameTracker *gGT = sdata->gGT;
	u32 words[18];

	words[0] = (u32)sdata->frameCounter;
	words[1] = (u32)sdata->randomNumber;
	words[2] = (u32)sdata->audioRNG;
	words[3] = (u32)sdata->Loading.stage;
	words[4] = (u32)sdata->mainGameState;
	words[5] = (u32)gGT->timer;
	words[6] = (u32)gGT->framesInThisLEV;
	words[7] = (u32)gGT->elapsedTimeMS;
	words[8] = (u32)gGT->msInThisLEV;
	words[9] = (u32)gGT->elapsedEventTime;
	words[10] = (u32)gGT->frameTimer_VsyncCallback;
	words[11] = (u32)gGT->trafficLightsTimer;
	words[12] = (u32)gGT->levelID;
	words[13] = (u32)gGT->gameMode1;
	words[14] = (u32)gGT->gameMode2;
	words[15] = (u32)gGT->numPlyrCurrGame;
	words[16] = (u32)gGT->numPlyrNextGame;
	words[17] = s_nativeAdhocEpoch;
	return NativeAdhoc_Crc32(words, sizeof(words));
}

static void NativeAdhoc_BuildInputPacket(void)
{
	memset(s_nativeAdhocFrameSend, 0, sizeof(s_nativeAdhocFrameSend));
	NativeAdhoc_WriteU32(s_nativeAdhocFrameSend, NATIVE_ADHOC_WIRE_MAGIC);
	NativeAdhoc_WriteU16(s_nativeAdhocFrameSend + 4, NATIVE_ADHOC_PROTOCOL_VERSION);
	NativeAdhoc_WriteU16(s_nativeAdhocFrameSend + 6, NATIVE_ADHOC_WIRE_INPUT);
	NativeAdhoc_WriteU16(s_nativeAdhocFrameSend + 8, NATIVE_ADHOC_INPUT_SIZE);
	s_nativeAdhocFrameSend[10] = (u8)s_nativeAdhocRole;
	s_nativeAdhocFrameSend[11] = sizeof(struct PlatformInputPadSnapshot);
	NativeAdhoc_WriteU32(s_nativeAdhocFrameSend + 12, s_nativeAdhocEpoch);
	NativeAdhoc_WriteU32(s_nativeAdhocFrameSend + 16, s_nativeAdhocFrame);
	NativeAdhoc_WriteU32(s_nativeAdhocFrameSend + 20, s_nativeAdhocFrameFingerprint);
	memcpy(s_nativeAdhocFrameSend + 24, &s_nativeAdhocLocalInput, sizeof(s_nativeAdhocLocalInput));
	NativeAdhoc_WriteU32(s_nativeAdhocFrameSend + 36, NativeAdhoc_Crc32(s_nativeAdhocFrameSend, 36));
}

static int NativeAdhoc_ValidateInputPacket(void)
{
	return (NativeAdhoc_ReadU32(s_nativeAdhocFrameRecv) == NATIVE_ADHOC_WIRE_MAGIC) &&
	       (NativeAdhoc_ReadU16(s_nativeAdhocFrameRecv + 4) == NATIVE_ADHOC_PROTOCOL_VERSION) &&
	       (NativeAdhoc_ReadU16(s_nativeAdhocFrameRecv + 6) == NATIVE_ADHOC_WIRE_INPUT) &&
	       (NativeAdhoc_ReadU16(s_nativeAdhocFrameRecv + 8) == NATIVE_ADHOC_INPUT_SIZE) &&
	       (s_nativeAdhocFrameRecv[10] == NativeAdhoc_RemoteRole()) &&
	       (s_nativeAdhocFrameRecv[11] == sizeof(struct PlatformInputPadSnapshot)) &&
	       (NativeAdhoc_ReadU32(s_nativeAdhocFrameRecv + 12) == s_nativeAdhocEpoch) &&
	       (NativeAdhoc_ReadU32(s_nativeAdhocFrameRecv + 16) == s_nativeAdhocFrame) &&
	       (NativeAdhoc_ReadU32(s_nativeAdhocFrameRecv + 20) == s_nativeAdhocFrameFingerprint) &&
	       (NativeAdhoc_ReadU32(s_nativeAdhocFrameRecv + 36) == NativeAdhoc_Crc32(s_nativeAdhocFrameRecv, 36));
}

static void NativeAdhoc_MakeDisconnectedInput(struct PlatformInputPadSnapshot *snapshot)
{
	memset(snapshot, 0, sizeof(*snapshot));
	snapshot->status = 0xff;
	snapshot->id = 0xff;
	snapshot->buttons[0] = 0xff;
	snapshot->buttons[1] = 0xff;
	snapshot->analog[0] = 0x80;
	snapshot->analog[1] = 0x80;
	snapshot->analog[2] = 0x80;
	snapshot->analog[3] = 0x80;
}

static int NativeAdhoc_ApplyFrameInputs(void)
{
	struct PlatformInputPadSnapshot combined[PLATFORM_INPUT_PAD_COUNT];
	struct PlatformInputPadSnapshot remote;

	memcpy(&remote, s_nativeAdhocFrameRecv + 24, sizeof(remote));
	memset(remote.reserved, 0, sizeof(remote.reserved));

	for (int i = 0; i < PLATFORM_INPUT_PAD_COUNT; i++)
	{
		NativeAdhoc_MakeDisconnectedInput(&combined[i]);
	}

	if (s_nativeAdhocRole == NATIVE_ADHOC_ROLE_HOST)
	{
		combined[0] = s_nativeAdhocLocalInput;
		combined[1] = remote;
	}
	else
	{
		combined[0] = remote;
		combined[1] = s_nativeAdhocLocalInput;
	}

	if (Platform_InputApplyPadSnapshots(combined, PLATFORM_INPUT_PAD_COUNT) != PLATFORM_INPUT_PAD_COUNT)
	{
		NativeAdhoc_Fail("Platform_InputApplyPadSnapshots", -1);
		return 0;
	}
	return 1;
}

static void NativeAdhoc_Start(enum NativeAdhocRole role)
{
	if (s_nativeAdhocDialogActive)
	{
		return;
	}
	if (s_nativeAdhocState != NATIVE_ADHOC_STATE_IDLE)
	{
		NativeAdhoc_CleanupNetwork();
		NativeAdhoc_ClearInputOverride();
		NativeAdhoc_RestoreNativeModes();
		NativeAdhoc_ResetRuntime();
	}

	s_nativeAdhocGameplayFailure = 0;
	s_nativeAdhocRole = role;
	s_nativeAdhocStatus = NATIVE_ADHOC_STATUS_CONNECTING;
	NativeAdhoc_SaveNativeModes();
	if (NativeAdhoc_InitNetwork())
	{
		NativeAdhoc_StartDialog();
	}
}

void NativeAdhoc_StartHost(void)
{
	NativeAdhoc_Start(NATIVE_ADHOC_ROLE_HOST);
}

void NativeAdhoc_StartClient(void)
{
	NativeAdhoc_Start(NATIVE_ADHOC_ROLE_CLIENT);
}

void NativeAdhoc_Cancel(void)
{
	if (s_nativeAdhocDialogActive)
	{
		if (!s_nativeAdhocDialogCancelRequested)
		{
			int result = sceNetCheckDialogAbort();
			if (result < 0)
			{
				NativeAdhoc_Fail("sceNetCheckDialogAbort", result);
				return;
			}
			s_nativeAdhocDialogCancelRequested = 1;
		}
		s_nativeAdhocState = NATIVE_ADHOC_STATE_DIALOG_ABORTING;
		return;
	}

	NativeAdhoc_CleanupNetwork();
	NativeAdhoc_ClearInputOverride();
	NativeAdhoc_RestoreNativeModes();
	s_nativeAdhocGameplayFailure = 0;
	NativeAdhoc_ResetRuntime();
}

void NativeAdhoc_Update(void)
{
	if (s_nativeAdhocNetCtlInitialized)
	{
		sceNetCtlCheckCallback();
	}

	switch (s_nativeAdhocState)
	{
	case NATIVE_ADHOC_STATE_IDLE:
	case NATIVE_ADHOC_STATE_ERROR:
		break;
	case NATIVE_ADHOC_STATE_DIALOG:
	case NATIVE_ADHOC_STATE_DIALOG_ABORTING:
		NativeAdhoc_UpdateDialog();
		break;
	case NATIVE_ADHOC_STATE_MATCHING:
		NativeAdhoc_UpdateMatching();
		break;
	case NATIVE_ADHOC_STATE_HOST_ACCEPT:
		NativeAdhoc_UpdateHostAccept();
		break;
	case NATIVE_ADHOC_STATE_CLIENT_CONNECT:
		NativeAdhoc_UpdateClientConnect();
		break;
	case NATIVE_ADHOC_STATE_HELLO:
		NativeAdhoc_UpdateHello();
		break;
	case NATIVE_ADHOC_STATE_HOST_WAIT_STATE:
		if (NativeAdhoc_DeadlineExpired())
		{
			NativeAdhoc_Fail("host state preparation timeout", -1);
		}
		break;
	case NATIVE_ADHOC_STATE_HOST_SEND_HEADER:
	case NATIVE_ADHOC_STATE_HOST_SEND_SNAPSHOT:
	case NATIVE_ADHOC_STATE_HOST_WAIT_READY:
	case NATIVE_ADHOC_STATE_HOST_SEND_START:
	case NATIVE_ADHOC_STATE_CLIENT_RECV_HEADER:
	case NATIVE_ADHOC_STATE_CLIENT_RECV_SNAPSHOT:
	case NATIVE_ADHOC_STATE_CLIENT_SEND_READY:
	case NATIVE_ADHOC_STATE_CLIENT_RECV_START:
		if (NativeAdhoc_CheckLink())
		{
			NativeAdhoc_UpdateBootstrap();
		}
		break;
	case NATIVE_ADHOC_STATE_ACTIVE:
		if (NativeAdhoc_CheckLink() && !NativeAdhoc_IsGameStable())
		{
			NativeAdhoc_BeginLoadSynchronization();
		}
		break;
	case NATIVE_ADHOC_STATE_LOAD_LOCAL:
		NativeAdhoc_UpdateLoadLocal();
		break;
	case NATIVE_ADHOC_STATE_LOAD_READY:
		if (NativeAdhoc_CheckLink())
		{
			NativeAdhoc_UpdateLoadReady();
		}
		break;
	}
}

int NativeAdhoc_HostNeedsState(void)
{
	return s_nativeAdhocState == NATIVE_ADHOC_STATE_HOST_WAIT_STATE;
}

int NativeAdhoc_HostStateReady(void)
{
	if (s_nativeAdhocState != NATIVE_ADHOC_STATE_HOST_WAIT_STATE)
	{
		return 0;
	}
	s_nativeAdhocSessionSynchronized = 1;
	return NativeAdhoc_CaptureSnapshot(NATIVE_ADHOC_SNAPSHOT_INITIAL, 1);
}

int NativeAdhoc_BeginGameFrame(void)
{
	struct PlatformInputPadSnapshot physical[PLATFORM_INPUT_PAD_COUNT];
	int sendResult;
	int recvResult;

	if (s_nativeAdhocState == NATIVE_ADHOC_STATE_LOAD_LOCAL)
	{
		return s_nativeAdhocLoadStableTicks == 0;
	}
	if (s_nativeAdhocState != NATIVE_ADHOC_STATE_ACTIVE)
	{
		if ((s_nativeAdhocState >= NATIVE_ADHOC_STATE_HOST_WAIT_STATE) &&
		    (s_nativeAdhocState <= NATIVE_ADHOC_STATE_CLIENT_RECV_START))
		{
			sceKernelDelayThread(NATIVE_ADHOC_IDLE_DELAY_US);
			return 0;
		}
		if (s_nativeAdhocState == NATIVE_ADHOC_STATE_LOAD_READY)
		{
			sceKernelDelayThread(NATIVE_ADHOC_IDLE_DELAY_US);
			return 0;
		}
		return 1;
	}

	if (!s_nativeAdhocFramePending)
	{
		if (Platform_InputCapturePadSnapshots(physical, PLATFORM_INPUT_PAD_COUNT) != PLATFORM_INPUT_PAD_COUNT)
		{
			NativeAdhoc_Fail("Platform_InputCapturePadSnapshots", -1);
			return 0;
		}

		s_nativeAdhocLocalInput = physical[0];
		memset(s_nativeAdhocLocalInput.reserved, 0, sizeof(s_nativeAdhocLocalInput.reserved));
		s_nativeAdhocFrameFingerprint = NativeAdhoc_FrameFingerprint();
		NativeAdhoc_BuildInputPacket();
		memset(s_nativeAdhocFrameRecv, 0, sizeof(s_nativeAdhocFrameRecv));
		s_nativeAdhocFrameSendOffset = 0;
		s_nativeAdhocFrameRecvOffset = 0;
		s_nativeAdhocFramePending = 1;
		NativeAdhoc_SetDeadline(NATIVE_ADHOC_FRAME_TIMEOUT_US);
	}

	sendResult = NativeAdhoc_StepSend(s_nativeAdhocFrameSend, sizeof(s_nativeAdhocFrameSend), &s_nativeAdhocFrameSendOffset);
	recvResult = NativeAdhoc_StepRecv(s_nativeAdhocFrameRecv, sizeof(s_nativeAdhocFrameRecv), &s_nativeAdhocFrameRecvOffset);
	if ((sendResult < 0) || (recvResult < 0))
	{
		return 0;
	}
	if ((sendResult > 0) && (recvResult > 0))
	{
		if (!NativeAdhoc_ValidateInputPacket())
		{
			NativeAdhoc_Fail("lockstep frame validation", -1);
			return 0;
		}
		if (!NativeAdhoc_ApplyFrameInputs())
		{
			return 0;
		}

		s_nativeAdhocFrame++;
		s_nativeAdhocFramePending = 0;
		NativeAdhoc_SetDeadline(0);
		return 1;
	}

	if (NativeAdhoc_DeadlineExpired())
	{
		NativeAdhoc_Fail("lockstep frame timeout", -1);
		return 0;
	}

	sceKernelDelayThread(NATIVE_ADHOC_IDLE_DELAY_US);
	return 0;
}

int NativeAdhoc_IsSessionActive(void)
{
	return s_nativeAdhocState == NATIVE_ADHOC_STATE_ACTIVE;
}

int NativeAdhoc_IsTimingControlled(void)
{
	return s_nativeAdhocSessionSynchronized &&
	       (s_nativeAdhocState != NATIVE_ADHOC_STATE_IDLE) &&
	       (s_nativeAdhocState != NATIVE_ADHOC_STATE_ERROR);
}

int NativeAdhoc_IsMenuInputBlocked(void)
{
	return s_nativeAdhocDialogActive;
}

int NativeAdhoc_IsCommonDialogActive(void)
{
	return s_nativeAdhocDialogActive;
}

int NativeAdhoc_ConsumeGameplayFailure(void)
{
	const int failed = s_nativeAdhocGameplayFailure;
	s_nativeAdhocGameplayFailure = 0;
	return failed;
}

enum NativeAdhocStatus NativeAdhoc_GetStatus(void)
{
	return s_nativeAdhocStatus;
}

void NativeAdhoc_Shutdown(void)
{
	if (s_nativeAdhocDialogActive)
	{
		if (!s_nativeAdhocDialogCancelRequested)
		{
			sceNetCheckDialogAbort();
			s_nativeAdhocDialogCancelRequested = 1;
		}

		for (int step = 0; step < NATIVE_ADHOC_SHUTDOWN_DIALOG_STEPS; step++)
		{
			SceCommonDialogStatus status = sceNetCheckDialogGetStatus();
			if (status == SCE_COMMON_DIALOG_STATUS_FINISHED)
			{
				SceNetCheckDialogResult result;
				memset(&result, 0, sizeof(result));
				sceNetCheckDialogGetResult(&result);
				sceNetCheckDialogTerm();
				s_nativeAdhocDialogActive = 0;
				break;
			}
			NativeRenderer_SwapWindow();
			sceKernelDelayThread(NATIVE_ADHOC_SHUTDOWN_DIALOG_DELAY_US);
		}
	}

	if (!s_nativeAdhocDialogActive)
	{
		NativeAdhoc_CleanupNetwork();
	}
	NativeAdhoc_ClearInputOverride();
	NativeAdhoc_RestoreNativeModes();
	NativeAdhoc_ResetRuntime();
}

#else

void NativeAdhoc_StartHost(void) {}
void NativeAdhoc_StartClient(void) {}
void NativeAdhoc_Cancel(void) {}
void NativeAdhoc_Update(void) {}
void NativeAdhoc_Shutdown(void) {}
int NativeAdhoc_BeginGameFrame(void) { return 1; }
int NativeAdhoc_IsSessionActive(void) { return 0; }
int NativeAdhoc_IsTimingControlled(void) { return 0; }
int NativeAdhoc_IsMenuInputBlocked(void) { return 0; }
int NativeAdhoc_IsCommonDialogActive(void) { return 0; }
int NativeAdhoc_HostNeedsState(void) { return 0; }
int NativeAdhoc_HostStateReady(void) { return 0; }
int NativeAdhoc_ConsumeGameplayFailure(void) { return 0; }
enum NativeAdhocStatus NativeAdhoc_GetStatus(void) { return NATIVE_ADHOC_STATUS_IDLE; }

#endif
