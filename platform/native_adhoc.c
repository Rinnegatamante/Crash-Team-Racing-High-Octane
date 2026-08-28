#include <common.h>
#include <platform.h>

#include "platform/native_adhoc.h"
#include "platform/native_checkpoint.h"
#include "platform/native_input.h"
#include "platform/native_log.h"
#include "platform/native_gpu_links.h"

#include <stdlib.h>
#include <string.h>

#ifdef __vita__
#include <vitasdk.h>

#define NATIVE_ADHOC_MAGIC 0x41485443u
#define NATIVE_ADHOC_VERSION 7u
#define NATIVE_ADHOC_PORT 31847
#define NATIVE_ADHOC_PDP_BUFSIZE 0x8000
#define NATIVE_ADHOC_NET_MEMORY_SIZE (1024 * 1024)
#define NATIVE_ADHOC_HELLO_INTERVAL_US 250000u
#define NATIVE_ADHOC_CONTROL_INTERVAL_US 100000u
#define NATIVE_ADHOC_SESSION_INTERVAL_US 250000u
#define NATIVE_ADHOC_HEARTBEAT_INTERVAL_US 500000u
#define NATIVE_ADHOC_PEER_TIMEOUT_US 5000000u
#define NATIVE_ADHOC_INPUT_RING_SIZE 64u
#define NATIVE_ADHOC_INPUT_RING_MASK (NATIVE_ADHOC_INPUT_RING_SIZE - 1u)
#define NATIVE_ADHOC_INPUT_DELAY 2u
#define NATIVE_ADHOC_INPUT_REDUNDANCY 6u
#define NATIVE_ADHOC_HASH_INTERVAL 30u
#define NATIVE_ADHOC_HASH_HISTORY 8u
#define NATIVE_ADHOC_SNAPSHOT_CHUNK_SIZE 1024u
#define NATIVE_ADHOC_SNAPSHOT_MAX_SIZE (64u * 1024u * 1024u)
#define NATIVE_ADHOC_SNAPSHOT_BURST 12u
#define NATIVE_ADHOC_INVALID_FRAME 0xffffffffu
#define NATIVE_ADHOC_RENDER_OT_ENTRIES 0x400u

CTR_STATIC_ASSERT((NATIVE_ADHOC_INPUT_RING_SIZE & NATIVE_ADHOC_INPUT_RING_MASK) == 0);

extern int cfg_language;

enum NativeAdhocPacketType
{
	NATIVE_ADHOC_PACKET_HELLO = 1,
	NATIVE_ADHOC_PACKET_WELCOME,
	NATIVE_ADHOC_PACKET_MENU_INPUT,
	NATIVE_ADHOC_PACKET_FRAME_INPUT,
	NATIVE_ADHOC_PACKET_SESSION,
	NATIVE_ADHOC_PACKET_READY,
	NATIVE_ADHOC_PACKET_START,
	NATIVE_ADHOC_PACKET_START_ACK,
	NATIVE_ADHOC_PACKET_HASH,
	NATIVE_ADHOC_PACKET_DESYNC,
	NATIVE_ADHOC_PACKET_SNAPSHOT_BEGIN,
	NATIVE_ADHOC_PACKET_SNAPSHOT_CHUNK,
	NATIVE_ADHOC_PACKET_SNAPSHOT_ACK,
	NATIVE_ADHOC_PACKET_SNAPSHOT_DONE,
	NATIVE_ADHOC_PACKET_RESUME,
	NATIVE_ADHOC_PACKET_RESUME_ACK,
	NATIVE_ADHOC_PACKET_HEARTBEAT,
	NATIVE_ADHOC_PACKET_RACE_PROPOSAL,
	NATIVE_ADHOC_PACKET_RACE_CONFIG,
	NATIVE_ADHOC_PACKET_RACE_CONFIG_ACK,
};

struct NativeAdhocPacketHeader
{
	u32 magic;
	u16 version;
	u8 type;
	u8 role;
	u32 sequence;
};

struct NativeAdhocWelcomePacket
{
	struct NativeAdhocPacketHeader header;
	u32 sessionId;
};

struct NativeAdhocMenuInputPacket
{
	struct NativeAdhocPacketHeader header;
	struct PlatformInputPadSnapshot pad;
};

struct NativeAdhocFrameInput
{
	u32 frame;
	struct PlatformInputPadSnapshot pad;
};

struct NativeAdhocFrameInputPacket
{
	struct NativeAdhocPacketHeader header;
	u32 sessionId;
	u32 latestFrame;
	u32 ackFrame;
	u8 count;
	u8 reserved[3];
	struct NativeAdhocFrameInput inputs[NATIVE_ADHOC_INPUT_REDUNDANCY];
};

struct NativeAdhocSessionPacket
{
	struct NativeAdhocPacketHeader header;
	u32 sessionId;
	u32 build;
	s32 levelID;
	s32 currLEV;
	u32 gameMode1;
	u32 gameMode2;
	s32 arcadeDifficulty;
	s32 numLaps;
	s32 language;
	s16 characterIDs[8];
	u32 randomNumber;
	u32 deadcoed0;
	u32 deadcoed1;
	u32 advRng0;
	u32 advRng1;
	u32 psxRandSeed;
};

struct NativeAdhocReadyPacket
{
	struct NativeAdhocPacketHeader header;
	u32 sessionId;
	u32 build;
	s32 levelID;
	s32 currLEV;
	s32 arcadeDifficulty;
	s32 numLaps;
	s32 language;
	s16 characterIDs[8];
};

CTR_STATIC_ASSERT(sizeof(((struct NativeAdhocSessionPacket *)0)->characterIDs) == sizeof(data.characterIDs));
CTR_STATIC_ASSERT(sizeof(((struct NativeAdhocReadyPacket *)0)->characterIDs) == sizeof(data.characterIDs));

struct NativeAdhocControlPacket
{
	struct NativeAdhocPacketHeader header;
	u32 sessionId;
	u32 snapshotId;
	u32 simulationFrame;
};

struct NativeAdhocHashPacket
{
	struct NativeAdhocPacketHeader header;
	u32 sessionId;
	u32 frame;
	u32 hash;
};

struct NativeAdhocDesyncPacket
{
	struct NativeAdhocPacketHeader header;
	u32 sessionId;
	u32 frame;
	u32 hostHash;
	u32 clientHash;
};

struct NativeAdhocSnapshotBeginPacket
{
	struct NativeAdhocPacketHeader header;
	u32 sessionId;
	u32 snapshotId;
	u32 simulationFrame;
	u32 totalSize;
	u32 checksum;
	u32 chunkSize;
	u32 chunkCount;
	u32 initialSync;
};

struct NativeAdhocSnapshotChunkPacket
{
	struct NativeAdhocPacketHeader header;
	u32 sessionId;
	u32 snapshotId;
	u32 chunkIndex;
	u32 dataSize;
	u8 data[NATIVE_ADHOC_SNAPSHOT_CHUNK_SIZE];
};

struct NativeAdhocSnapshotAckPacket
{
	struct NativeAdhocPacketHeader header;
	u32 sessionId;
	u32 snapshotId;
	u32 chunkIndex;
};

struct NativeAdhocSnapshotDonePacket
{
	struct NativeAdhocPacketHeader header;
	u32 sessionId;
	u32 snapshotId;
	u32 simulationFrame;
};

struct NativeAdhocHeartbeatPacket
{
	struct NativeAdhocPacketHeader header;
	u32 sessionId;
	u32 simulationFrame;
};

struct NativeAdhocRaceProposalPacket
{
	struct NativeAdhocPacketHeader header;
	u32 sessionId;
	u32 build;
	u8 p2Character;
	u8 reserved[3];
};

struct NativeAdhocRaceConfigPacket
{
	struct NativeAdhocPacketHeader header;
	u32 sessionId;
	u32 build;
	s32 currLEV;
	s32 arcadeDifficulty;
	s32 numLaps;
	s32 language;
	u8 p1Character;
	u8 p2Character;
	u8 reserved[2];
};

struct NativeAdhocRaceConfigAckPacket
{
	struct NativeAdhocPacketHeader header;
	u32 sessionId;
	s32 currLEV;
	u8 p1Character;
	u8 p2Character;
	u8 reserved[2];
};

struct NativeAdhocInputSlot
{
	u32 frame;
	struct PlatformInputPadSnapshot pad;
	u8 valid;
};

struct NativeAdhocHashSlot
{
	u32 frame;
	u32 hash;
	u8 valid;
};

struct NativeAdhocContext
{
	int role;
	int status;
	int socket;
	int netInitialized;
	int netCtlInitialized;
	int netModuleLoadedByUs;
	int appUtilInitializedByUs;
	void *netMemory;
	int adhocInitialized;
	int adhocctlInitialized;
	int dialogRunning;
	int adhocModuleLoadedByUs;
	int peerKnown;
	SceNetEtherAddr localMac;
	SceNetEtherAddr peerMac;
	u32 sessionId;
	u32 txSequence;
	u32 rxMenuInputSequence;
	u32 lastHelloTime;
	u32 lastHeartbeatTime;
	u32 lastSessionTime;
	u32 lastControlTime;
	u32 lastPeerTime;
	struct PlatformInputPadSnapshot remoteMenuPad;
	int remoteMenuPadValid;

	int raceSyncPending;
	int raceLoadPrepared;
	int raceProposalValid;
	u8 raceProposalP2;
	int raceConfigSent;
	int raceConfigReceived;
	int raceConfigAcked;
	u32 lastRaceConfigTime;
	struct NativeAdhocRaceConfigPacket raceConfig;

	int localLevelReady;
	int peerLevelReady;
	int sessionReceived;
	int initialSyncComplete;
	struct NativeAdhocSessionPacket session;

	int simulationActive;
	u32 simulationFrame;
	u32 installedFrame;
	struct NativeAdhocInputSlot inputs[2][NATIVE_ADHOC_INPUT_RING_SIZE];
	u32 latestLocalInputFrame;
	int latestLocalInputValid;

	struct NativeAdhocHashSlot hashes[NATIVE_ADHOC_HASH_HISTORY];
	u32 nextHashSlot;
	u32 pendingHostHashFrame;
	u32 pendingHostHash;
	int pendingHostHashValid;
	int resyncRequested;

	u8 *txSnapshot;
	u8 *txSnapshotAcked;
	u32 txSnapshotSize;
	u32 txSnapshotChecksum;
	u32 txSnapshotId;
	u32 txSnapshotChunkCount;
	u32 txSnapshotCursor;
	u32 txSnapshotFrame;
	int txSnapshotInitial;
	int txSnapshotActive;

	u8 *rxSnapshot;
	u8 *rxSnapshotReceived;
	u32 rxSnapshotSize;
	u32 rxSnapshotChecksum;
	u32 rxSnapshotId;
	u32 rxSnapshotChunkCount;
	u32 rxSnapshotReceivedCount;
	u32 rxSnapshotFrame;
	u32 lastRestoredSnapshotId;
	int rxSnapshotInitial;
	int rxSnapshotActive;
	int rxRestorePending;
	int snapshotDonePending;

	int controlPending;
	int controlType;
	u32 controlSnapshotId;
	u32 controlFrame;
	u32 lastAppliedControlSnapshotId;
};

static struct NativeAdhocContext s_nativeAdhoc =
{
	.role = NATIVE_ADHOC_ROLE_NONE,
	.status = NATIVE_ADHOC_STATUS_OFF,
	.socket = -1,
	.installedFrame = NATIVE_ADHOC_INVALID_FRAME,
};

static struct PushBuffer s_nativeAdhocRenderPushBuffer;
static u32 s_nativeAdhocRenderOT[NATIVE_ADHOC_RENDER_OT_ENTRIES];
static int s_nativeAdhocSingleViewRenderActive;
static int s_nativeAdhocReturnToMainMenuPending;
static int s_nativeAdhocReturnVisualPending;
static u32 s_nativeAdhocConnectionLostNoticeUntil;
static int s_nativeAdhocConnectionLostNoticePending;

static void NativeAdhoc_InitHeader(struct NativeAdhocPacketHeader *header, int type);
static int NativeAdhoc_SendRaw(const SceNetEtherAddr *dst, const void *packet, int len);

static const SceNetEtherAddr s_nativeAdhocBroadcastMac =
{
	.data = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
};

static u32 NativeAdhoc_Now(void)
{
	return sceKernelGetProcessTimeLow();
}

static void NativeAdhoc_FreeSnapshotBuffers(void)
{
	free(s_nativeAdhoc.txSnapshot);
	free(s_nativeAdhoc.txSnapshotAcked);
	free(s_nativeAdhoc.rxSnapshot);
	free(s_nativeAdhoc.rxSnapshotReceived);
	s_nativeAdhoc.txSnapshot = NULL;
	s_nativeAdhoc.txSnapshotAcked = NULL;
	s_nativeAdhoc.rxSnapshot = NULL;
	s_nativeAdhoc.rxSnapshotReceived = NULL;
	s_nativeAdhoc.txSnapshotActive = 0;
	s_nativeAdhoc.rxSnapshotActive = 0;
	s_nativeAdhoc.rxRestorePending = 0;
	s_nativeAdhoc.snapshotDonePending = 0;
}

static void NativeAdhoc_MakeNeutralPad(struct PlatformInputPadSnapshot *pad)
{
	memset(pad, 0, sizeof(*pad));
	pad->status = 0;
	pad->id = 0x73;
	pad->buttons[0] = 0xff;
	pad->buttons[1] = 0xff;
	pad->analog[0] = 0x80;
	pad->analog[1] = 0x80;
	pad->analog[2] = 0x80;
	pad->analog[3] = 0x80;
	pad->connected = 1;
}

static void NativeAdhoc_ClearInputRing(void)
{
	memset(s_nativeAdhoc.inputs, 0, sizeof(s_nativeAdhoc.inputs));
	s_nativeAdhoc.latestLocalInputValid = 0;
	s_nativeAdhoc.installedFrame = NATIVE_ADHOC_INVALID_FRAME;
}

static void NativeAdhoc_StoreInput(int playerIndex, u32 frame, const struct PlatformInputPadSnapshot *pad)
{
	struct NativeAdhocInputSlot *slot;

	if ((playerIndex < 0) || (playerIndex > 1) || (pad == NULL))
	{
		return;
	}

	slot = &s_nativeAdhoc.inputs[playerIndex][frame & NATIVE_ADHOC_INPUT_RING_MASK];
	slot->frame = frame;
	slot->pad = *pad;
	slot->pad.connected = 1;
	slot->valid = 1;
}

static int NativeAdhoc_GetInput(int playerIndex, u32 frame, struct PlatformInputPadSnapshot *pad)
{
	struct NativeAdhocInputSlot *slot;

	if ((playerIndex < 0) || (playerIndex > 1))
	{
		return 0;
	}

	slot = &s_nativeAdhoc.inputs[playerIndex][frame & NATIVE_ADHOC_INPUT_RING_MASK];
	if (!slot->valid || (slot->frame != frame))
	{
		return 0;
	}

	if (pad != NULL)
	{
		*pad = slot->pad;
	}
	return 1;
}

static void NativeAdhoc_ResetLockstep(u32 frame)
{
	struct PlatformInputPadSnapshot neutral;

	NativeAdhoc_ClearInputRing();
	s_nativeAdhoc.simulationFrame = frame;
	NativeAdhoc_MakeNeutralPad(&neutral);
	for (u32 i = 0; i < NATIVE_ADHOC_INPUT_DELAY; i++)
	{
		NativeAdhoc_StoreInput(0, frame + i, &neutral);
		NativeAdhoc_StoreInput(1, frame + i, &neutral);
	}
}

static void NativeAdhoc_ResetRaceConfigState(void)
{
	s_nativeAdhoc.raceSyncPending = 0;
	s_nativeAdhoc.raceLoadPrepared = 0;
	s_nativeAdhoc.raceProposalValid = 0;
	s_nativeAdhoc.raceProposalP2 = 0;
	s_nativeAdhoc.raceConfigSent = 0;
	s_nativeAdhoc.raceConfigReceived = 0;
	s_nativeAdhoc.raceConfigAcked = 0;
	s_nativeAdhoc.lastRaceConfigTime = 0;
	memset(&s_nativeAdhoc.raceConfig, 0, sizeof(s_nativeAdhoc.raceConfig));
}

static void NativeAdhoc_SendRaceProposal(void)
{
	struct NativeAdhocRaceProposalPacket packet;

	if ((s_nativeAdhoc.role != NATIVE_ADHOC_ROLE_CLIENT) || !s_nativeAdhoc.peerKnown || !s_nativeAdhoc.raceSyncPending)
	{
		return;
	}

	memset(&packet, 0, sizeof(packet));
	NativeAdhoc_InitHeader(&packet.header, NATIVE_ADHOC_PACKET_RACE_PROPOSAL);
	packet.sessionId = s_nativeAdhoc.sessionId;
	packet.build = BUILD;
	packet.p2Character = s_nativeAdhoc.raceProposalP2;
	NativeAdhoc_SendRaw(&s_nativeAdhoc.peerMac, &packet, sizeof(packet));
}

static void NativeAdhoc_FillRaceConfig(struct NativeAdhocRaceConfigPacket *packet, const struct GameTracker *gGT)
{
	memset(packet, 0, sizeof(*packet));
	NativeAdhoc_InitHeader(&packet->header, NATIVE_ADHOC_PACKET_RACE_CONFIG);
	packet->sessionId = s_nativeAdhoc.sessionId;
	packet->build = BUILD;
	packet->currLEV = gGT->currLEV;
	packet->arcadeDifficulty = gGT->arcadeDifficulty;
	packet->numLaps = gGT->numLaps;
	packet->language = cfg_language;
	packet->p1Character = data.characterIDs[0];
	packet->p2Character = s_nativeAdhoc.raceProposalP2;
}

static void NativeAdhoc_SendRaceConfig(void)
{
	if ((s_nativeAdhoc.role != NATIVE_ADHOC_ROLE_HOST) || !s_nativeAdhoc.peerKnown || !s_nativeAdhoc.raceConfigSent)
	{
		return;
	}
	NativeAdhoc_SendRaw(&s_nativeAdhoc.peerMac, &s_nativeAdhoc.raceConfig, sizeof(s_nativeAdhoc.raceConfig));
}

static void NativeAdhoc_SendRaceConfigAck(const struct NativeAdhocRaceConfigPacket *config)
{
	struct NativeAdhocRaceConfigAckPacket packet;

	if ((config == NULL) || (s_nativeAdhoc.role != NATIVE_ADHOC_ROLE_CLIENT) || !s_nativeAdhoc.peerKnown)
	{
		return;
	}

	memset(&packet, 0, sizeof(packet));
	NativeAdhoc_InitHeader(&packet.header, NATIVE_ADHOC_PACKET_RACE_CONFIG_ACK);
	packet.sessionId = s_nativeAdhoc.sessionId;
	packet.currLEV = config->currLEV;
	packet.p1Character = config->p1Character;
	packet.p2Character = config->p2Character;
	NativeAdhoc_SendRaw(&s_nativeAdhoc.peerMac, &packet, sizeof(packet));
}

static void NativeAdhoc_ApplyRaceConfig(const struct NativeAdhocRaceConfigPacket *packet, struct GameTracker *gGT)
{
	if ((packet == NULL) || (gGT == NULL))
	{
		return;
	}

	gGT->currLEV = packet->currLEV;
	gGT->arcadeDifficulty = packet->arcadeDifficulty;
	gGT->numLaps = packet->numLaps;
	gGT->numPlyrNextGame = 2;
	cfg_language = packet->language;
	data.characterIDs[0] = packet->p1Character;
	data.characterIDs[1] = packet->p2Character;
}

static void NativeAdhoc_PumpRaceConfig(u32 now)
{
	struct GameTracker *gGT = sdata->gGT;

	if (!s_nativeAdhoc.raceSyncPending || s_nativeAdhoc.raceLoadPrepared || !s_nativeAdhoc.peerKnown)
	{
		return;
	}

	if (s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_CLIENT)
	{
		if (!s_nativeAdhoc.raceConfigReceived && ((u32)(now - s_nativeAdhoc.lastRaceConfigTime) >= NATIVE_ADHOC_CONTROL_INTERVAL_US))
		{
			NativeAdhoc_SendRaceProposal();
			s_nativeAdhoc.lastRaceConfigTime = now;
		}
		return;
	}

	if (s_nativeAdhoc.role != NATIVE_ADHOC_ROLE_HOST)
	{
		return;
	}

	if (s_nativeAdhoc.raceProposalValid && !s_nativeAdhoc.raceConfigSent && (gGT != NULL))
	{
		NativeAdhoc_FillRaceConfig(&s_nativeAdhoc.raceConfig, gGT);
		s_nativeAdhoc.raceConfigSent = 1;
		s_nativeAdhoc.lastRaceConfigTime = 0;
		Platform_Log("[CTR Adhoc] canonical pre-load config track=%d diff=%d laps=%d lang=%d p1=%u p2=%u\n",
			s_nativeAdhoc.raceConfig.currLEV,
			s_nativeAdhoc.raceConfig.arcadeDifficulty,
			s_nativeAdhoc.raceConfig.numLaps,
			s_nativeAdhoc.raceConfig.language,
			s_nativeAdhoc.raceConfig.p1Character,
			s_nativeAdhoc.raceConfig.p2Character);
	}

	if (s_nativeAdhoc.raceConfigSent && !s_nativeAdhoc.raceConfigAcked &&
		((u32)(now - s_nativeAdhoc.lastRaceConfigTime) >= NATIVE_ADHOC_CONTROL_INTERVAL_US))
	{
		NativeAdhoc_SendRaceConfig();
		s_nativeAdhoc.lastRaceConfigTime = now;
	}
}

static u32 NativeAdhoc_RaceSeed(const struct GameTracker *gGT)
{
	u32 seed = s_nativeAdhoc.sessionId ^ 0x4354524eu;

	if (gGT != NULL)
	{
		seed ^= (u32)gGT->currLEV * 0x9e3779b9u;
		seed ^= (u32)gGT->arcadeDifficulty * 0x85ebca6bu;
		seed ^= (u32)gGT->numLaps * 0xc2b2ae35u;
	}

	// Only the two human selections are authoritative before LOAD_Robots2P.
	// Slots 2..5 are populated by the 2P AI pack during loading, while 6..7
	// are unused leftovers in a six-racer 2P race and must not perturb the seed.
	seed = (seed * 33u) ^ data.characterIDs[0];
	seed = (seed * 33u) ^ data.characterIDs[1];
	if (seed == 0)
	{
		seed = 1;
	}
	return seed;
}

int NativeAdhoc_PrepareRaceLoad(struct GameTracker *gGT)
{
	u32 seed;

	if (!s_nativeAdhoc.peerKnown || (gGT == NULL) || (gGT->numPlyrNextGame != 2))
	{
		return 1;
	}

	if (s_nativeAdhoc.raceLoadPrepared)
	{
		return 1;
	}

	if (!s_nativeAdhoc.raceSyncPending)
	{
		s_nativeAdhoc.raceSyncPending = 1;
		s_nativeAdhoc.raceConfigSent = 0;
		s_nativeAdhoc.raceConfigReceived = 0;
		s_nativeAdhoc.raceConfigAcked = 0;
		s_nativeAdhoc.lastRaceConfigTime = 0;
		memset(&s_nativeAdhoc.raceConfig, 0, sizeof(s_nativeAdhoc.raceConfig));

		if (s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_CLIENT)
		{
			s_nativeAdhoc.raceProposalP2 = data.characterIDs[1];
			NativeAdhoc_SendRaceProposal();
			s_nativeAdhoc.lastRaceConfigTime = NativeAdhoc_Now();
			Platform_Log("[CTR Adhoc] proposed P2 character %u before race load\n", s_nativeAdhoc.raceProposalP2);
		}
	}

	NativeAdhoc_PumpRaceConfig(NativeAdhoc_Now());

	if (s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_HOST)
	{
		if (!s_nativeAdhoc.raceConfigSent || !s_nativeAdhoc.raceConfigAcked)
		{
			return 0;
		}
		NativeAdhoc_ApplyRaceConfig(&s_nativeAdhoc.raceConfig, gGT);
	}
	else
	{
		if (!s_nativeAdhoc.raceConfigReceived)
		{
			return 0;
		}
		NativeAdhoc_ApplyRaceConfig(&s_nativeAdhoc.raceConfig, gGT);
	}

	seed = NativeAdhoc_RaceSeed(gGT);
	sdata->randomNumber = seed ^ 0x13579bdfu;
	gGT->deadcoed_struct.state0 = seed ^ 0xa5a5a5a5u;
	gGT->deadcoed_struct.state1 = (seed * 1664525u) + 1013904223u;
	sdata->advRng.state0 = seed ^ 0x5a5a5a5au;
	sdata->advRng.state1 = (seed * 1103515245u) + 12345u;
	PSX_BIOS_SetRandSeed(seed);

	s_nativeAdhoc.localLevelReady = 0;
	s_nativeAdhoc.peerLevelReady = 0;
	s_nativeAdhoc.sessionReceived = s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_HOST;
	s_nativeAdhoc.initialSyncComplete = 0;
	s_nativeAdhoc.simulationActive = 0;
	s_nativeAdhoc.controlPending = 0;
	NativeAdhoc_ClearInputRing();
	s_nativeAdhoc.raceLoadPrepared = 1;
	s_nativeAdhoc.raceSyncPending = 0;
	Platform_Log("[CTR Adhoc] deterministic race seed 0x%08x for LEV %d config p1=%u p2=%u lang=%d\n",
		seed, gGT->currLEV, data.characterIDs[0], data.characterIDs[1], cfg_language);
	return 1;
}

int NativeAdhoc_EnforcePreparedRaceConfig(struct GameTracker *gGT)
{
	if ((gGT == NULL) || !s_nativeAdhoc.peerKnown || !s_nativeAdhoc.raceLoadPrepared)
	{
		return 0;
	}

	NativeAdhoc_ApplyRaceConfig(&s_nativeAdhoc.raceConfig, gGT);
	gGT->numPlyrNextGame = 2;
	gGT->numPlyrCurrGame = 2;
	Platform_Log("[CTR Adhoc] load config enforced curr=%d next=%d p1=%u p2=%u\n",
		gGT->numPlyrCurrGame, gGT->numPlyrNextGame, data.characterIDs[0], data.characterIDs[1]);
	return 1;
}

void NativeAdhoc_BeginRenderFrame(struct GameTracker *gGT)
{
	int localIndex;

	s_nativeAdhocSingleViewRenderActive = 0;
	if ((s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_NONE) || (gGT == NULL) || (gGT->numPlyrCurrGame != 2) || ((gGT->gameMode1 & MAIN_MENU) != 0))
	{
		return;
	}

	localIndex = NativeAdhoc_GetLocalPlayerIndex();
	struct PushBuffer *local = &gGT->pushBuffer[localIndex];

	// Build a genuine 1P PushBuffer instead of cloning a 2P PushBuffer and
	// patching a few fields. This prevents stale split-screen projection and
	// RenderBucket metadata from leaking into the fullscreen render path.
	memset(&s_nativeAdhocRenderPushBuffer, 0, sizeof(s_nativeAdhocRenderPushBuffer));
	PushBuffer_Init(&s_nativeAdhocRenderPushBuffer, localIndex, 1);
	s_nativeAdhocRenderPushBuffer.pos = local->pos;
	s_nativeAdhocRenderPushBuffer.rot = local->rot;
	memcpy(s_nativeAdhocRenderPushBuffer.data6, local->data6, sizeof(s_nativeAdhocRenderPushBuffer.data6));
	s_nativeAdhocRenderPushBuffer.fadeFromBlack_currentValue = local->fadeFromBlack_currentValue;
	s_nativeAdhocRenderPushBuffer.fadeFromBlack_desiredResult = local->fadeFromBlack_desiredResult;
	s_nativeAdhocRenderPushBuffer.fade_step = local->fade_step;
	PushBuffer_UpdateFrustum(&s_nativeAdhocRenderPushBuffer);

	// Keep the fullscreen presentation completely separate from both retail 2P
	// camera OTs. Some retail render paths still populate those segments even
	// though adhoc only presents one camera; sharing either segment lets stale
	// split-screen primitives leak into the fullscreen chain.
	NativeGpuLinks_RegisterRangeChecked("adhoc single-view OT", s_nativeAdhocRenderOT, sizeof(s_nativeAdhocRenderOT));
	ClearOTagR(s_nativeAdhocRenderOT, NATIVE_ADHOC_RENDER_OT_ENTRIES);

	// Retail 2P reserves six words before the lower camera OT for the shared UI
	// chain. pushBuffer_UI.ptrOT points at the second word, so +4 is the last
	// shared UI slot that the lower camera OT normally chains into.
	setaddr(&s_nativeAdhocRenderOT[0], &gGT->pushBuffer_UI.ptrOT[4]);
	s_nativeAdhocRenderPushBuffer.ptrOT = s_nativeAdhocRenderOT;
	s_nativeAdhocSingleViewRenderActive = 1;
}

void NativeAdhoc_EndRenderFrame(void)
{
	s_nativeAdhocSingleViewRenderActive = 0;
}

int NativeAdhoc_IsSingleViewRenderActive(void)
{
	return s_nativeAdhocSingleViewRenderActive;
}

struct PushBuffer *NativeAdhoc_GetRenderPushBuffer(void)
{
	return s_nativeAdhocSingleViewRenderActive ? &s_nativeAdhocRenderPushBuffer : NULL;
}

static int NativeAdhoc_IsPacketHeaderValid(const struct NativeAdhocPacketHeader *header, int len)
{
	return
		(len >= (int)sizeof(*header)) &&
		(header->magic == NATIVE_ADHOC_MAGIC) &&
		(header->version == NATIVE_ADHOC_VERSION) &&
		((header->role == NATIVE_ADHOC_ROLE_HOST) || (header->role == NATIVE_ADHOC_ROLE_CLIENT)) &&
		(header->role != s_nativeAdhoc.role);
}

static void NativeAdhoc_InitHeader(struct NativeAdhocPacketHeader *header, int type)
{
	header->magic = NATIVE_ADHOC_MAGIC;
	header->version = NATIVE_ADHOC_VERSION;
	header->type = (u8)type;
	header->role = (u8)s_nativeAdhoc.role;
	header->sequence = ++s_nativeAdhoc.txSequence;
}

static int NativeAdhoc_SendRaw(const SceNetEtherAddr *dst, const void *packet, int len)
{
	int result;

	if ((s_nativeAdhoc.socket < 0) || (dst == NULL) || (packet == NULL) || (len <= 0))
	{
		return 0;
	}

	result = sceNetAdhocPdpSend(
		s_nativeAdhoc.socket,
		dst,
		NATIVE_ADHOC_PORT,
		packet,
		len,
		0,
		SCE_NET_ADHOC_F_NONBLOCK);

	return (result >= 0) || (result == SCE_ERROR_NET_ADHOC_WOULD_BLOCK);
}

static void NativeAdhoc_SendHello(void)
{
	struct NativeAdhocPacketHeader packet;
	NativeAdhoc_InitHeader(&packet, NATIVE_ADHOC_PACKET_HELLO);
	NativeAdhoc_SendRaw(&s_nativeAdhocBroadcastMac, &packet, sizeof(packet));
}

static void NativeAdhoc_SendWelcome(const SceNetEtherAddr *dst)
{
	struct NativeAdhocWelcomePacket packet;
	memset(&packet, 0, sizeof(packet));
	NativeAdhoc_InitHeader(&packet.header, NATIVE_ADHOC_PACKET_WELCOME);
	packet.sessionId = s_nativeAdhoc.sessionId;
	NativeAdhoc_SendRaw(dst, &packet, sizeof(packet));
}

static void NativeAdhoc_SetPeer(const SceNetEtherAddr *peer)
{
	if (peer == NULL)
	{
		return;
	}

	s_nativeAdhoc.peerMac = *peer;
	s_nativeAdhoc.peerKnown = 1;
	s_nativeAdhoc.status = NATIVE_ADHOC_STATUS_CONNECTED;
	s_nativeAdhoc.lastPeerTime = NativeAdhoc_Now();
}

static int NativeAdhoc_OpenSocket(void)
{
	int socket;

	if (sceNetAdhocctlGetEtherAddr(&s_nativeAdhoc.localMac) < 0)
	{
		Platform_Log("[CTR Adhoc] failed to get local MAC\n");
		return 0;
	}

	socket = sceNetAdhocPdpCreate(&s_nativeAdhoc.localMac, NATIVE_ADHOC_PORT, NATIVE_ADHOC_PDP_BUFSIZE, 0);
	if (socket < 0)
	{
		Platform_Log("[CTR Adhoc] sceNetAdhocPdpCreate failed: 0x%08x\n", socket);
		return 0;
	}

	s_nativeAdhoc.socket = socket;
	s_nativeAdhoc.status = NATIVE_ADHOC_STATUS_WAITING;
	s_nativeAdhoc.lastHelloTime = 0;
	s_nativeAdhoc.lastHeartbeatTime = NativeAdhoc_Now();
	s_nativeAdhoc.lastPeerTime = NativeAdhoc_Now();
	if (s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_HOST)
	{
		s_nativeAdhoc.sessionId = NativeAdhoc_Now() ^ 0x4354524eu;
		for (int i = 0; i < 6; i++)
		{
			s_nativeAdhoc.sessionId = (s_nativeAdhoc.sessionId * 33u) ^ s_nativeAdhoc.localMac.data[i];
		}
		if (s_nativeAdhoc.sessionId == 0)
		{
			s_nativeAdhoc.sessionId = 1;
		}
	}
	Platform_Log("[CTR Adhoc] PDP ready, role=%s port=%d\n",
		s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_HOST ? "host" : "client",
		NATIVE_ADHOC_PORT);
	return 1;
}

static void NativeAdhoc_FinishDialog(void)
{
	SceNetCheckDialogResult result;

	memset(&result, 0, sizeof(result));
	sceNetCheckDialogGetResult(&result);
	sceNetCheckDialogTerm();
	s_nativeAdhoc.dialogRunning = 0;

	if (result.result != SCE_COMMON_DIALOG_RESULT_OK)
	{
		Platform_Log("[CTR Adhoc] NetCheck dialog result: 0x%08x\n", result.result);
		s_nativeAdhoc.status = NATIVE_ADHOC_STATUS_ERROR;
		return;
	}

	if (!NativeAdhoc_OpenSocket())
	{
		s_nativeAdhoc.status = NATIVE_ADHOC_STATUS_ERROR;
	}
}

static u32 NativeAdhoc_Crc32Update(u32 crc, const void *dataPtr, int size)
{
	const u8 *dataBytes = (const u8 *)dataPtr;

	for (int i = 0; i < size; i++)
	{
		crc ^= dataBytes[i];
		for (int bit = 0; bit < 8; bit++)
		{
			u32 mask = (u32)-(s32)(crc & 1u);
			crc = (crc >> 1) ^ (0xedb88320u & mask);
		}
	}
	return crc;
}

static u32 NativeAdhoc_Crc32(const void *data, int size)
{
	return NativeAdhoc_Crc32Update(0xffffffffu, data, size) ^ 0xffffffffu;
}

static u32 NativeAdhoc_HashDriver(u32 crc, const struct Driver *driver)
{
	if (driver == NULL)
	{
		u32 empty = 0;
		return NativeAdhoc_Crc32Update(crc, &empty, sizeof(empty));
	}

	crc = NativeAdhoc_Crc32Update(crc, &driver->driverID, sizeof(driver->driverID));
	crc = NativeAdhoc_Crc32Update(crc, &driver->posCurr, sizeof(driver->posCurr));
	crc = NativeAdhoc_Crc32Update(crc, &driver->posPrev, sizeof(driver->posPrev));
	crc = NativeAdhoc_Crc32Update(crc, &driver->velocity, sizeof(driver->velocity));
	crc = NativeAdhoc_Crc32Update(crc, &driver->rotCurr, sizeof(driver->rotCurr));
	crc = NativeAdhoc_Crc32Update(crc, &driver->rotPrev, sizeof(driver->rotPrev));
	crc = NativeAdhoc_Crc32Update(crc, &driver->xSpeed, sizeof(driver->xSpeed));
	crc = NativeAdhoc_Crc32Update(crc, &driver->ySpeed, sizeof(driver->ySpeed));
	crc = NativeAdhoc_Crc32Update(crc, &driver->zSpeed, sizeof(driver->zSpeed));
	crc = NativeAdhoc_Crc32Update(crc, &driver->speed, sizeof(driver->speed));
	crc = NativeAdhoc_Crc32Update(crc, &driver->speedApprox, sizeof(driver->speedApprox));
	crc = NativeAdhoc_Crc32Update(crc, &driver->reserves, sizeof(driver->reserves));
	crc = NativeAdhoc_Crc32Update(crc, &driver->fireSpeed, sizeof(driver->fireSpeed));
	crc = NativeAdhoc_Crc32Update(crc, &driver->fireSpeedCap, sizeof(driver->fireSpeedCap));
	crc = NativeAdhoc_Crc32Update(crc, &driver->actionsFlagSet, sizeof(driver->actionsFlagSet));
	crc = NativeAdhoc_Crc32Update(crc, &driver->actionsFlagSetPrevFrame, sizeof(driver->actionsFlagSetPrevFrame));
	crc = NativeAdhoc_Crc32Update(crc, &driver->lapTime, sizeof(driver->lapTime));
	crc = NativeAdhoc_Crc32Update(crc, &driver->lapIndex, sizeof(driver->lapIndex));
	crc = NativeAdhoc_Crc32Update(crc, &driver->driverRank, sizeof(driver->driverRank));
	crc = NativeAdhoc_Crc32Update(crc, &driver->distanceToFinish_curr, sizeof(driver->distanceToFinish_curr));
	crc = NativeAdhoc_Crc32Update(crc, &driver->distanceToFinish_checkpoint, sizeof(driver->distanceToFinish_checkpoint));
	crc = NativeAdhoc_Crc32Update(crc, &driver->checkpoint, sizeof(driver->checkpoint));
	crc = NativeAdhoc_Crc32Update(crc, &driver->kartState, sizeof(driver->kartState));
	crc = NativeAdhoc_Crc32Update(crc, &driver->numWumpas, sizeof(driver->numWumpas));
	crc = NativeAdhoc_Crc32Update(crc, &driver->heldItemID, sizeof(driver->heldItemID));
	crc = NativeAdhoc_Crc32Update(crc, &driver->numHeldItems, sizeof(driver->numHeldItems));
	crc = NativeAdhoc_Crc32Update(crc, &driver->itemRollTimer, sizeof(driver->itemRollTimer));
	crc = NativeAdhoc_Crc32Update(crc, &driver->noItemTimer, sizeof(driver->noItemTimer));
	crc = NativeAdhoc_Crc32Update(crc, &driver->hazardTimer, sizeof(driver->hazardTimer));
	crc = NativeAdhoc_Crc32Update(crc, &driver->invincibleTimer, sizeof(driver->invincibleTimer));
	crc = NativeAdhoc_Crc32Update(crc, &driver->invisibleTimer, sizeof(driver->invisibleTimer));
	return crc;
}

static u32 NativeAdhoc_HashThreadBucket(u32 crc, const struct GameTracker *gGT, int bucketIndex)
{
	const struct Thread *thread = gGT->threadBuckets[bucketIndex].thread;
	u32 count = 0;

	while ((thread != NULL) && (count < 1024u))
	{
		const struct Instance *inst = thread->inst;
		crc = NativeAdhoc_Crc32Update(crc, &bucketIndex, sizeof(bucketIndex));
		crc = NativeAdhoc_Crc32Update(crc, &thread->cooldownFrameCount, sizeof(thread->cooldownFrameCount));
		crc = NativeAdhoc_Crc32Update(crc, &thread->flags, sizeof(thread->flags));
		crc = NativeAdhoc_Crc32Update(crc, &thread->timesDestroyed, sizeof(thread->timesDestroyed));
		crc = NativeAdhoc_Crc32Update(crc, &thread->modelIndex, sizeof(thread->modelIndex));
		if (inst != NULL)
		{
			s32 modelID = inst->model != NULL ? inst->model->id : -1;
			crc = NativeAdhoc_Crc32Update(crc, &modelID, sizeof(modelID));
			crc = NativeAdhoc_Crc32Update(crc, &inst->scale, sizeof(inst->scale));
			crc = NativeAdhoc_Crc32Update(crc, &inst->flags, sizeof(inst->flags));
			crc = NativeAdhoc_Crc32Update(crc, &inst->matrix, sizeof(inst->matrix));
			crc = NativeAdhoc_Crc32Update(crc, &inst->animIndex, sizeof(inst->animIndex));
			crc = NativeAdhoc_Crc32Update(crc, &inst->animFrame, sizeof(inst->animFrame));
		}
		count++;
		thread = thread->siblingThread;
	}

	return NativeAdhoc_Crc32Update(crc, &count, sizeof(count));
}

static u32 NativeAdhoc_GameplayHash(const struct GameTracker *gGT)
{
	u32 crc = 0xffffffffu;
	local_persist const int gameplayBuckets[] = {PLAYER, ROBOT, MINE, TRACKING, OTHER};

	crc = NativeAdhoc_Crc32Update(crc, &gGT->levelID, sizeof(gGT->levelID));
	crc = NativeAdhoc_Crc32Update(crc, &gGT->gameMode1, sizeof(gGT->gameMode1));
	crc = NativeAdhoc_Crc32Update(crc, &gGT->gameMode2, sizeof(gGT->gameMode2));
	crc = NativeAdhoc_Crc32Update(crc, &gGT->timer, sizeof(gGT->timer));
	crc = NativeAdhoc_Crc32Update(crc, &gGT->trafficLightsTimer, sizeof(gGT->trafficLightsTimer));
	crc = NativeAdhoc_Crc32Update(crc, &gGT->elapsedEventTime, sizeof(gGT->elapsedEventTime));
	crc = NativeAdhoc_Crc32Update(crc, &gGT->frozenTimeRemaining, sizeof(gGT->frozenTimeRemaining));
	crc = NativeAdhoc_Crc32Update(crc, &gGT->deadcoed_struct, sizeof(gGT->deadcoed_struct));
	crc = NativeAdhoc_Crc32Update(crc, &sdata->randomNumber, sizeof(sdata->randomNumber));
	crc = NativeAdhoc_Crc32Update(crc, &sdata->advRng, sizeof(sdata->advRng));
	crc = NativeAdhoc_Crc32Update(crc, &sdata->numPlayersFinishedRace, sizeof(sdata->numPlayersFinishedRace));
	for (int i = 0; i < 8; i++)
	{
		crc = NativeAdhoc_HashDriver(crc, gGT->drivers[i]);
	}
	for (u32 i = 0; i < len(gameplayBuckets); i++)
	{
		crc = NativeAdhoc_HashThreadBucket(crc, gGT, gameplayBuckets[i]);
	}
	return crc ^ 0xffffffffu;
}

static void NativeAdhoc_SaveHash(u32 frame, u32 hash)
{
	struct NativeAdhocHashSlot *slot = &s_nativeAdhoc.hashes[s_nativeAdhoc.nextHashSlot % NATIVE_ADHOC_HASH_HISTORY];
	slot->frame = frame;
	slot->hash = hash;
	slot->valid = 1;
	s_nativeAdhoc.nextHashSlot++;
}

static int NativeAdhoc_FindHash(u32 frame, u32 *hash)
{
	for (u32 i = 0; i < NATIVE_ADHOC_HASH_HISTORY; i++)
	{
		if (s_nativeAdhoc.hashes[i].valid && (s_nativeAdhoc.hashes[i].frame == frame))
		{
			if (hash != NULL)
			{
				*hash = s_nativeAdhoc.hashes[i].hash;
			}
			return 1;
		}
	}
	return 0;
}

static void NativeAdhoc_SendMenuInput(const struct PlatformInputPadSnapshot *pad)
{
	struct NativeAdhocMenuInputPacket packet;

	if (!s_nativeAdhoc.peerKnown || (pad == NULL))
	{
		return;
	}
	memset(&packet, 0, sizeof(packet));
	NativeAdhoc_InitHeader(&packet.header, NATIVE_ADHOC_PACKET_MENU_INPUT);
	packet.pad = *pad;
	packet.pad.connected = 1;
	NativeAdhoc_SendRaw(&s_nativeAdhoc.peerMac, &packet, sizeof(packet));
}

static void NativeAdhoc_SendFrameInputs(void)
{
	struct NativeAdhocFrameInputPacket packet;
	int localIndex = NativeAdhoc_GetLocalPlayerIndex();

	if (!s_nativeAdhoc.peerKnown || !s_nativeAdhoc.latestLocalInputValid)
	{
		return;
	}

	memset(&packet, 0, sizeof(packet));
	NativeAdhoc_InitHeader(&packet.header, NATIVE_ADHOC_PACKET_FRAME_INPUT);
	packet.sessionId = s_nativeAdhoc.sessionId;
	packet.latestFrame = s_nativeAdhoc.latestLocalInputFrame;
	packet.ackFrame = s_nativeAdhoc.simulationFrame == 0 ? 0 : s_nativeAdhoc.simulationFrame - 1u;
	for (u32 i = 0; (i < NATIVE_ADHOC_INPUT_REDUNDANCY) && (i <= packet.latestFrame); i++)
	{
		u32 frame = packet.latestFrame - i;
		struct PlatformInputPadSnapshot pad;
		if (NativeAdhoc_GetInput(localIndex, frame, &pad))
		{
			packet.inputs[packet.count].frame = frame;
			packet.inputs[packet.count].pad = pad;
			packet.count++;
		}
	}
	NativeAdhoc_SendRaw(&s_nativeAdhoc.peerMac, &packet, sizeof(packet));
}

static void NativeAdhoc_FillSession(struct NativeAdhocSessionPacket *packet, const struct GameTracker *gGT)
{
	memset(packet, 0, sizeof(*packet));
	NativeAdhoc_InitHeader(&packet->header, NATIVE_ADHOC_PACKET_SESSION);
	packet->sessionId = s_nativeAdhoc.sessionId;
	packet->build = BUILD;
	packet->levelID = gGT->levelID;
	packet->currLEV = gGT->currLEV;
	packet->gameMode1 = gGT->gameMode1;
	packet->gameMode2 = gGT->gameMode2;
	packet->arcadeDifficulty = gGT->arcadeDifficulty;
	packet->numLaps = gGT->numLaps;
	packet->language = cfg_language;
	memcpy(packet->characterIDs, data.characterIDs, sizeof(packet->characterIDs));
	if (s_nativeAdhoc.raceLoadPrepared)
	{
		packet->characterIDs[0] = s_nativeAdhoc.raceConfig.p1Character;
		packet->characterIDs[1] = s_nativeAdhoc.raceConfig.p2Character;
	}
	packet->randomNumber = (u32)sdata->randomNumber;
	packet->deadcoed0 = (u32)gGT->deadcoed_struct.state0;
	packet->deadcoed1 = (u32)gGT->deadcoed_struct.state1;
	packet->advRng0 = (u32)sdata->advRng.state0;
	packet->advRng1 = (u32)sdata->advRng.state1;
	packet->psxRandSeed = PSX_BIOS_GetRandSeed();
}

static void NativeAdhoc_ApplySessionSeeds(const struct NativeAdhocSessionPacket *packet, struct GameTracker *gGT)
{
	if ((packet == NULL) || (gGT == NULL))
	{
		return;
	}

	sdata->randomNumber = packet->randomNumber;
	gGT->deadcoed_struct.state0 = packet->deadcoed0;
	gGT->deadcoed_struct.state1 = packet->deadcoed1;
	sdata->advRng.state0 = packet->advRng0;
	sdata->advRng.state1 = packet->advRng1;
	PSX_BIOS_SetRandSeed(packet->psxRandSeed);
}

static int NativeAdhoc_SessionMatchesLocal(const struct NativeAdhocSessionPacket *packet, const struct GameTracker *gGT)
{
	if ((packet == NULL) || (gGT == NULL))
	{
		return 0;
	}

	if ((packet->build != BUILD) || (packet->levelID != gGT->levelID) || (packet->currLEV != gGT->currLEV) ||
		(packet->arcadeDifficulty != gGT->arcadeDifficulty) || (packet->numLaps != gGT->numLaps) || (packet->language != cfg_language))
	{
		return 0;
	}

	// Before the authoritative snapshot only the two human character slots
	// must already match. LOAD_Robots2P deterministically fills 2..5 from
	// those two choices; 6..7 are unused in a six-racer 2P race.
	return (packet->characterIDs[0] == data.characterIDs[0]) &&
	       (packet->characterIDs[1] == data.characterIDs[1]);
}

static void NativeAdhoc_ApplySessionConfig(const struct NativeAdhocSessionPacket *packet, struct GameTracker *gGT)
{
	if ((packet == NULL) || (gGT == NULL))
	{
		return;
	}

	memcpy(data.characterIDs, packet->characterIDs, sizeof(packet->characterIDs));
	NativeAdhoc_ApplySessionSeeds(packet, gGT);
}

static void NativeAdhoc_LogSessionMismatch(const struct NativeAdhocSessionPacket *packet, const struct GameTracker *gGT)
{
	if ((packet == NULL) || (gGT == NULL))
	{
		return;
	}

	Platform_Log(
		"[CTR Adhoc] session mismatch build=%u/%u level=%d/%d currLEV=%d/%d diff=%d/%d laps=%d/%d lang=%d/%d p1=%d/%d p2=%d/%d\n",
		packet->build, (u32)BUILD,
		packet->levelID, gGT->levelID,
		packet->currLEV, gGT->currLEV,
		packet->arcadeDifficulty, gGT->arcadeDifficulty,
		packet->numLaps, gGT->numLaps,
		packet->language, cfg_language,
		packet->characterIDs[0], data.characterIDs[0],
		packet->characterIDs[1], data.characterIDs[1]);
}

static void NativeAdhoc_SendSession(void)
{
	if ((s_nativeAdhoc.role != NATIVE_ADHOC_ROLE_HOST) || !s_nativeAdhoc.localLevelReady || !s_nativeAdhoc.peerKnown)
	{
		return;
	}
	NativeAdhoc_SendRaw(&s_nativeAdhoc.peerMac, &s_nativeAdhoc.session, sizeof(s_nativeAdhoc.session));
}

static void NativeAdhoc_SendReady(void)
{
	struct NativeAdhocReadyPacket packet;
	struct GameTracker *gGT = sdata->gGT;

	if ((s_nativeAdhoc.role != NATIVE_ADHOC_ROLE_CLIENT) || !s_nativeAdhoc.localLevelReady || !s_nativeAdhoc.sessionReceived || (gGT == NULL))
	{
		return;
	}
	memset(&packet, 0, sizeof(packet));
	NativeAdhoc_InitHeader(&packet.header, NATIVE_ADHOC_PACKET_READY);
	packet.sessionId = s_nativeAdhoc.sessionId;
	packet.build = BUILD;
	packet.levelID = gGT->levelID;
	packet.currLEV = gGT->currLEV;
	packet.arcadeDifficulty = gGT->arcadeDifficulty;
	packet.numLaps = gGT->numLaps;
	packet.language = cfg_language;
	memcpy(packet.characterIDs, data.characterIDs, sizeof(packet.characterIDs));
	NativeAdhoc_SendRaw(&s_nativeAdhoc.peerMac, &packet, sizeof(packet));
}

static int NativeAdhoc_ReadyMatchesSession(const struct NativeAdhocReadyPacket *packet)
{
	return
		(packet->build == s_nativeAdhoc.session.build) &&
		(packet->levelID == s_nativeAdhoc.session.levelID) &&
		(packet->currLEV == s_nativeAdhoc.session.currLEV) &&
		(packet->arcadeDifficulty == s_nativeAdhoc.session.arcadeDifficulty) &&
		(packet->numLaps == s_nativeAdhoc.session.numLaps) &&
		(packet->language == s_nativeAdhoc.session.language) &&
		(memcmp(packet->characterIDs, s_nativeAdhoc.session.characterIDs, sizeof(packet->characterIDs)) == 0);
}

static void NativeAdhoc_SendHeartbeat(void)
{
	struct NativeAdhocHeartbeatPacket packet;
	if (!s_nativeAdhoc.peerKnown)
	{
		return;
	}
	memset(&packet, 0, sizeof(packet));
	NativeAdhoc_InitHeader(&packet.header, NATIVE_ADHOC_PACKET_HEARTBEAT);
	packet.sessionId = s_nativeAdhoc.sessionId;
	packet.simulationFrame = s_nativeAdhoc.simulationFrame;
	NativeAdhoc_SendRaw(&s_nativeAdhoc.peerMac, &packet, sizeof(packet));
}

static void NativeAdhoc_SendHash(u32 frame, u32 hash)
{
	struct NativeAdhocHashPacket packet;
	memset(&packet, 0, sizeof(packet));
	NativeAdhoc_InitHeader(&packet.header, NATIVE_ADHOC_PACKET_HASH);
	packet.sessionId = s_nativeAdhoc.sessionId;
	packet.frame = frame;
	packet.hash = hash;
	NativeAdhoc_SendRaw(&s_nativeAdhoc.peerMac, &packet, sizeof(packet));
}

static void NativeAdhoc_SendDesync(u32 frame, u32 hostHash, u32 clientHash)
{
	struct NativeAdhocDesyncPacket packet;
	memset(&packet, 0, sizeof(packet));
	NativeAdhoc_InitHeader(&packet.header, NATIVE_ADHOC_PACKET_DESYNC);
	packet.sessionId = s_nativeAdhoc.sessionId;
	packet.frame = frame;
	packet.hostHash = hostHash;
	packet.clientHash = clientHash;
	NativeAdhoc_SendRaw(&s_nativeAdhoc.peerMac, &packet, sizeof(packet));
	Platform_Log("[CTR Adhoc] desync frame=%u host=0x%08x client=0x%08x\n", frame, hostHash, clientHash);
}

static void NativeAdhoc_CheckHostHash(u32 frame, u32 hostHash)
{
	u32 clientHash;
	if (s_nativeAdhoc.role != NATIVE_ADHOC_ROLE_CLIENT)
	{
		return;
	}
	if (NativeAdhoc_FindHash(frame, &clientHash))
	{
		if (clientHash != hostHash)
		{
			NativeAdhoc_SendDesync(frame, hostHash, clientHash);
		}
		return;
	}
	s_nativeAdhoc.pendingHostHashFrame = frame;
	s_nativeAdhoc.pendingHostHash = hostHash;
	s_nativeAdhoc.pendingHostHashValid = 1;
}

static void NativeAdhoc_SendSnapshotBegin(void)
{
	struct NativeAdhocSnapshotBeginPacket packet;
	if (!s_nativeAdhoc.txSnapshotActive)
	{
		return;
	}
	memset(&packet, 0, sizeof(packet));
	NativeAdhoc_InitHeader(&packet.header, NATIVE_ADHOC_PACKET_SNAPSHOT_BEGIN);
	packet.sessionId = s_nativeAdhoc.sessionId;
	packet.snapshotId = s_nativeAdhoc.txSnapshotId;
	packet.simulationFrame = s_nativeAdhoc.txSnapshotFrame;
	packet.totalSize = s_nativeAdhoc.txSnapshotSize;
	packet.checksum = s_nativeAdhoc.txSnapshotChecksum;
	packet.chunkSize = NATIVE_ADHOC_SNAPSHOT_CHUNK_SIZE;
	packet.chunkCount = s_nativeAdhoc.txSnapshotChunkCount;
	packet.initialSync = s_nativeAdhoc.txSnapshotInitial;
	NativeAdhoc_SendRaw(&s_nativeAdhoc.peerMac, &packet, sizeof(packet));
}

static void NativeAdhoc_SendSnapshotChunk(u32 chunkIndex)
{
	struct NativeAdhocSnapshotChunkPacket packet;
	u32 offset;
	u32 remaining;
	u32 dataSize;
	int packetSize;

	if (!s_nativeAdhoc.txSnapshotActive || (chunkIndex >= s_nativeAdhoc.txSnapshotChunkCount))
	{
		return;
	}
	offset = chunkIndex * NATIVE_ADHOC_SNAPSHOT_CHUNK_SIZE;
	remaining = s_nativeAdhoc.txSnapshotSize - offset;
	dataSize = remaining < NATIVE_ADHOC_SNAPSHOT_CHUNK_SIZE ? remaining : NATIVE_ADHOC_SNAPSHOT_CHUNK_SIZE;
	memset(&packet, 0, sizeof(packet));
	NativeAdhoc_InitHeader(&packet.header, NATIVE_ADHOC_PACKET_SNAPSHOT_CHUNK);
	packet.sessionId = s_nativeAdhoc.sessionId;
	packet.snapshotId = s_nativeAdhoc.txSnapshotId;
	packet.chunkIndex = chunkIndex;
	packet.dataSize = dataSize;
	memcpy(packet.data, &s_nativeAdhoc.txSnapshot[offset], dataSize);
	packetSize = (int)(sizeof(packet) - sizeof(packet.data) + dataSize);
	NativeAdhoc_SendRaw(&s_nativeAdhoc.peerMac, &packet, packetSize);
}

static void NativeAdhoc_SendSnapshotAck(u32 snapshotId, u32 chunkIndex)
{
	struct NativeAdhocSnapshotAckPacket packet;
	memset(&packet, 0, sizeof(packet));
	NativeAdhoc_InitHeader(&packet.header, NATIVE_ADHOC_PACKET_SNAPSHOT_ACK);
	packet.sessionId = s_nativeAdhoc.sessionId;
	packet.snapshotId = snapshotId;
	packet.chunkIndex = chunkIndex;
	NativeAdhoc_SendRaw(&s_nativeAdhoc.peerMac, &packet, sizeof(packet));
}

static void NativeAdhoc_SendSnapshotDone(void)
{
	struct NativeAdhocSnapshotDonePacket packet;
	if (!s_nativeAdhoc.snapshotDonePending)
	{
		return;
	}
	memset(&packet, 0, sizeof(packet));
	NativeAdhoc_InitHeader(&packet.header, NATIVE_ADHOC_PACKET_SNAPSHOT_DONE);
	packet.sessionId = s_nativeAdhoc.sessionId;
	packet.snapshotId = s_nativeAdhoc.rxSnapshotId;
	packet.simulationFrame = s_nativeAdhoc.rxSnapshotFrame;
	NativeAdhoc_SendRaw(&s_nativeAdhoc.peerMac, &packet, sizeof(packet));
}

static void NativeAdhoc_SendControl(void)
{
	struct NativeAdhocControlPacket packet;
	if (!s_nativeAdhoc.controlPending)
	{
		return;
	}
	memset(&packet, 0, sizeof(packet));
	NativeAdhoc_InitHeader(&packet.header, s_nativeAdhoc.controlType);
	packet.sessionId = s_nativeAdhoc.sessionId;
	packet.snapshotId = s_nativeAdhoc.controlSnapshotId;
	packet.simulationFrame = s_nativeAdhoc.controlFrame;
	NativeAdhoc_SendRaw(&s_nativeAdhoc.peerMac, &packet, sizeof(packet));
}

static void NativeAdhoc_SendControlAck(int type, u32 snapshotId, u32 frame)
{
	struct NativeAdhocControlPacket packet;
	memset(&packet, 0, sizeof(packet));
	NativeAdhoc_InitHeader(&packet.header, type);
	packet.sessionId = s_nativeAdhoc.sessionId;
	packet.snapshotId = snapshotId;
	packet.simulationFrame = frame;
	NativeAdhoc_SendRaw(&s_nativeAdhoc.peerMac, &packet, sizeof(packet));
}

static int NativeAdhoc_BeginSnapshot(int initialSync, u32 simulationFrame)
{
	int size;

	if ((s_nativeAdhoc.role != NATIVE_ADHOC_ROLE_HOST) || !s_nativeAdhoc.peerKnown)
	{
		return 0;
	}

	free(s_nativeAdhoc.txSnapshot);
	free(s_nativeAdhoc.txSnapshotAcked);
	s_nativeAdhoc.txSnapshot = NULL;
	s_nativeAdhoc.txSnapshotAcked = NULL;

	size = NativeCheckpoint_GetGameplaySize();
	if ((size <= 0) || ((u32)size > NATIVE_ADHOC_SNAPSHOT_MAX_SIZE))
	{
		Platform_Log("[CTR Adhoc] invalid gameplay checkpoint size: %d\n", size);
		return 0;
	}
	s_nativeAdhoc.txSnapshot = (u8 *)malloc((size_t)size);
	if (s_nativeAdhoc.txSnapshot == NULL)
	{
		Platform_Log("[CTR Adhoc] failed to allocate %d-byte gameplay checkpoint\n", size);
		return 0;
	}
	if (!NativeCheckpoint_CaptureGameplay(s_nativeAdhoc.txSnapshot, size))
	{
		Platform_Log("[CTR Adhoc] gameplay checkpoint capture failed\n");
		free(s_nativeAdhoc.txSnapshot);
		s_nativeAdhoc.txSnapshot = NULL;
		return 0;
	}

	s_nativeAdhoc.txSnapshotSize = (u32)size;
	s_nativeAdhoc.txSnapshotChecksum = NativeAdhoc_Crc32(s_nativeAdhoc.txSnapshot, size);
	s_nativeAdhoc.txSnapshotId++;
	if (s_nativeAdhoc.txSnapshotId == 0)
	{
		s_nativeAdhoc.txSnapshotId = 1;
	}
	s_nativeAdhoc.txSnapshotChunkCount = (s_nativeAdhoc.txSnapshotSize + NATIVE_ADHOC_SNAPSHOT_CHUNK_SIZE - 1u) / NATIVE_ADHOC_SNAPSHOT_CHUNK_SIZE;
	s_nativeAdhoc.txSnapshotAcked = (u8 *)calloc(s_nativeAdhoc.txSnapshotChunkCount, 1);
	if (s_nativeAdhoc.txSnapshotAcked == NULL)
	{
		free(s_nativeAdhoc.txSnapshot);
		s_nativeAdhoc.txSnapshot = NULL;
		return 0;
	}
	s_nativeAdhoc.txSnapshotCursor = 0;
	s_nativeAdhoc.txSnapshotFrame = simulationFrame;
	s_nativeAdhoc.txSnapshotInitial = initialSync;
	s_nativeAdhoc.txSnapshotActive = 1;
	s_nativeAdhoc.simulationActive = 0;
	s_nativeAdhoc.installedFrame = NATIVE_ADHOC_INVALID_FRAME;
	s_nativeAdhoc.lastControlTime = 0;
	Platform_Log("[CTR Adhoc] snapshot #%u size=%u chunks=%u frame=%u mode=%s\n",
		s_nativeAdhoc.txSnapshotId,
		s_nativeAdhoc.txSnapshotSize,
		s_nativeAdhoc.txSnapshotChunkCount,
		simulationFrame,
		initialSync ? "initial" : "recovery");
	NativeAdhoc_SendSnapshotBegin();
	return 1;
}

static void NativeAdhoc_PumpSnapshotTx(void)
{
	u32 sent = 0;
	u32 scanned = 0;

	if (!s_nativeAdhoc.txSnapshotActive)
	{
		return;
	}
	while ((sent < NATIVE_ADHOC_SNAPSHOT_BURST) && (scanned < s_nativeAdhoc.txSnapshotChunkCount))
	{
		u32 chunk = s_nativeAdhoc.txSnapshotCursor;
		s_nativeAdhoc.txSnapshotCursor++;
		if (s_nativeAdhoc.txSnapshotCursor >= s_nativeAdhoc.txSnapshotChunkCount)
		{
			s_nativeAdhoc.txSnapshotCursor = 0;
		}
		scanned++;
		if (!s_nativeAdhoc.txSnapshotAcked[chunk])
		{
			NativeAdhoc_SendSnapshotChunk(chunk);
			sent++;
		}
	}
}

static int NativeAdhoc_PrepareSnapshotRx(const struct NativeAdhocSnapshotBeginPacket *packet)
{
	int expectedSize = NativeCheckpoint_GetGameplaySize();
	u32 expectedChunks;

	if ((expectedSize <= 0) || ((u32)expectedSize > NATIVE_ADHOC_SNAPSHOT_MAX_SIZE) ||
		(packet->totalSize != (u32)expectedSize) ||
		(packet->chunkSize != NATIVE_ADHOC_SNAPSHOT_CHUNK_SIZE))
	{
		return 0;
	}
	expectedChunks = packet->totalSize / NATIVE_ADHOC_SNAPSHOT_CHUNK_SIZE;
	if ((packet->totalSize % NATIVE_ADHOC_SNAPSHOT_CHUNK_SIZE) != 0)
	{
		expectedChunks++;
	}
	if ((expectedChunks == 0) || (packet->chunkCount != expectedChunks))
	{
		return 0;
	}

	// A completed snapshot is immutable protocol history. Delayed BEGIN/CHUNK
	// packets for it must never reopen the RX state or trigger a second restore.
	// If START/RESUME has not arrived yet, SNAPSHOT_DONE is already retransmitted
	// by snapshotDonePending; after control is applied the duplicate is simply stale.
	if ((s_nativeAdhoc.lastRestoredSnapshotId != 0) &&
	    (s_nativeAdhoc.lastRestoredSnapshotId == packet->snapshotId))
	{
		return 1;
	}

	if (s_nativeAdhoc.rxSnapshotActive && (s_nativeAdhoc.rxSnapshotId == packet->snapshotId))
	{
		return 1;
	}

	free(s_nativeAdhoc.rxSnapshot);
	free(s_nativeAdhoc.rxSnapshotReceived);
	s_nativeAdhoc.rxSnapshot = (u8 *)malloc(packet->totalSize);
	s_nativeAdhoc.rxSnapshotReceived = (u8 *)calloc(packet->chunkCount, 1);
	if ((s_nativeAdhoc.rxSnapshot == NULL) || (s_nativeAdhoc.rxSnapshotReceived == NULL))
	{
		free(s_nativeAdhoc.rxSnapshot);
		free(s_nativeAdhoc.rxSnapshotReceived);
		s_nativeAdhoc.rxSnapshot = NULL;
		s_nativeAdhoc.rxSnapshotReceived = NULL;
		return 0;
	}

	s_nativeAdhoc.rxSnapshotSize = packet->totalSize;
	s_nativeAdhoc.rxSnapshotChecksum = packet->checksum;
	s_nativeAdhoc.rxSnapshotId = packet->snapshotId;
	s_nativeAdhoc.rxSnapshotChunkCount = packet->chunkCount;
	s_nativeAdhoc.rxSnapshotReceivedCount = 0;
	s_nativeAdhoc.rxSnapshotFrame = packet->simulationFrame;
	s_nativeAdhoc.rxSnapshotInitial = packet->initialSync != 0;
	s_nativeAdhoc.rxSnapshotActive = 1;
	s_nativeAdhoc.rxRestorePending = 0;
	s_nativeAdhoc.snapshotDonePending = 0;
	s_nativeAdhoc.simulationActive = 0;
	s_nativeAdhoc.installedFrame = NATIVE_ADHOC_INVALID_FRAME;
	Platform_Log("[CTR Adhoc] receiving snapshot #%u size=%u chunks=%u frame=%u\n",
		packet->snapshotId, packet->totalSize, packet->chunkCount, packet->simulationFrame);
	return 1;
}

static void NativeAdhoc_HandleSnapshotChunk(const struct NativeAdhocSnapshotChunkPacket *packet, int len)
{
	u32 offset;
	u32 expectedSize;
	int baseSize = (int)(sizeof(*packet) - sizeof(packet->data));

	if (!s_nativeAdhoc.rxSnapshotActive || (packet->snapshotId != s_nativeAdhoc.rxSnapshotId) ||
		(packet->chunkIndex >= s_nativeAdhoc.rxSnapshotChunkCount) || (packet->dataSize > NATIVE_ADHOC_SNAPSHOT_CHUNK_SIZE) ||
		(len != baseSize + (int)packet->dataSize))
	{
		return;
	}
	if (packet->chunkIndex > (0xffffffffu / NATIVE_ADHOC_SNAPSHOT_CHUNK_SIZE))
	{
		return;
	}
	offset = packet->chunkIndex * NATIVE_ADHOC_SNAPSHOT_CHUNK_SIZE;
	if (offset >= s_nativeAdhoc.rxSnapshotSize)
	{
		return;
	}
	expectedSize = s_nativeAdhoc.rxSnapshotSize - offset;
	if (expectedSize > NATIVE_ADHOC_SNAPSHOT_CHUNK_SIZE)
	{
		expectedSize = NATIVE_ADHOC_SNAPSHOT_CHUNK_SIZE;
	}
	if (packet->dataSize != expectedSize)
	{
		return;
	}

	if (!s_nativeAdhoc.rxSnapshotReceived[packet->chunkIndex])
	{
		memcpy(&s_nativeAdhoc.rxSnapshot[offset], packet->data, packet->dataSize);
		s_nativeAdhoc.rxSnapshotReceived[packet->chunkIndex] = 1;
		s_nativeAdhoc.rxSnapshotReceivedCount++;
	}
	NativeAdhoc_SendSnapshotAck(packet->snapshotId, packet->chunkIndex);

	if ((s_nativeAdhoc.rxSnapshotReceivedCount == s_nativeAdhoc.rxSnapshotChunkCount) && !s_nativeAdhoc.rxRestorePending)
	{
		u32 checksum = NativeAdhoc_Crc32(s_nativeAdhoc.rxSnapshot, (int)s_nativeAdhoc.rxSnapshotSize);
		if (checksum != s_nativeAdhoc.rxSnapshotChecksum)
		{
			Platform_Log("[CTR Adhoc] snapshot checksum mismatch expected=0x%08x got=0x%08x\n", s_nativeAdhoc.rxSnapshotChecksum, checksum);
			s_nativeAdhoc.status = NATIVE_ADHOC_STATUS_ERROR;
			return;
		}
		s_nativeAdhoc.rxRestorePending = 1;
	}
}

static void NativeAdhoc_BeginControlFromSnapshot(void)
{
	s_nativeAdhoc.controlPending = 1;
	s_nativeAdhoc.controlType = s_nativeAdhoc.txSnapshotInitial ? NATIVE_ADHOC_PACKET_START : NATIVE_ADHOC_PACKET_RESUME;
	s_nativeAdhoc.controlSnapshotId = s_nativeAdhoc.txSnapshotId;
	s_nativeAdhoc.controlFrame = s_nativeAdhoc.txSnapshotFrame;
	s_nativeAdhoc.lastControlTime = 0;
	NativeAdhoc_ResetLockstep(s_nativeAdhoc.controlFrame);
	NativeAdhoc_SendControl();
}

static void NativeAdhoc_BeginDirectStart(u32 simulationFrame)
{
	s_nativeAdhoc.txSnapshotId++;
	if (s_nativeAdhoc.txSnapshotId == 0)
	{
		s_nativeAdhoc.txSnapshotId = 1;
	}

	s_nativeAdhoc.controlPending = 1;
	s_nativeAdhoc.controlType = NATIVE_ADHOC_PACKET_START;
	s_nativeAdhoc.controlSnapshotId = s_nativeAdhoc.txSnapshotId;
	s_nativeAdhoc.controlFrame = simulationFrame;
	s_nativeAdhoc.lastControlTime = 0;
	NativeAdhoc_ResetLockstep(simulationFrame);
	NativeAdhoc_SendControl();
	Platform_Log("[CTR Adhoc] direct lockstep start requested at frame %u\n", simulationFrame);
}

static void NativeAdhoc_ApplyControl(const struct NativeAdhocControlPacket *packet, int ackType)
{
	if (packet->snapshotId != s_nativeAdhoc.lastAppliedControlSnapshotId)
	{
		NativeAdhoc_ResetLockstep(packet->simulationFrame);
		s_nativeAdhoc.lastAppliedControlSnapshotId = packet->snapshotId;
		s_nativeAdhoc.initialSyncComplete = 1;
		s_nativeAdhoc.simulationActive = 1;
		sdata->frameCounter = 0;
		if (sdata->gGT != NULL)
		{
			sdata->gGT->elapsedTimeMS = 0x20;
		}
		if (sdata->gGamepads != NULL)
		{
			GAMEPAD_GetNumConnected(sdata->gGamepads);
			for (int i = 0; i < 2; i++)
			{
				sdata->gGamepads->gamepad[i].buttonsHeldCurrFrame = 0;
				sdata->gGamepads->gamepad[i].buttonsHeldPrevFrame = 0;
				sdata->gGamepads->gamepad[i].buttonsTapped = 0;
				sdata->gGamepads->gamepad[i].buttonsReleased = 0;
			}
			Platform_Log("[CTR Adhoc] client pad bindings connected=%d p0=%d p1=%d local=%d\n",
				sdata->gGamepads->numGamepadsConnected,
				sdata->gGamepads->gamepad[0].ptrControllerPacket != NULL,
				sdata->gGamepads->gamepad[1].ptrControllerPacket != NULL,
				NativeAdhoc_GetLocalPlayerIndex());
		}
		s_nativeAdhoc.snapshotDonePending = 0;
		s_nativeAdhoc.rxSnapshotActive = 0;
		s_nativeAdhoc.rxRestorePending = 0;
		free(s_nativeAdhoc.rxSnapshot);
		free(s_nativeAdhoc.rxSnapshotReceived);
		s_nativeAdhoc.rxSnapshot = NULL;
		s_nativeAdhoc.rxSnapshotReceived = NULL;
		Platform_Log("[CTR Adhoc] simulation %s at frame %u\n",
			ackType == NATIVE_ADHOC_PACKET_START_ACK ? "started" : "resumed",
			packet->simulationFrame);
		if (ackType == NATIVE_ADHOC_PACKET_START_ACK)
		{
			if ((sdata->gGT != NULL) && (sdata->gGT->levelID < NITRO_COURT))
			{
				Audio_SetState_Safe(AUDIO_RACE_INTRO);
			}
			NativeAdhoc_ResetRaceConfigState();
		}
	}
	NativeAdhoc_SendControlAck(ackType, packet->snapshotId, packet->simulationFrame);
}

static int NativeAdhoc_PacketSessionMatches(u32 sessionId)
{
	return (sessionId != 0) && (sessionId == s_nativeAdhoc.sessionId);
}

static void NativeAdhoc_ReceivePackets(void)
{
	u8 buffer[sizeof(struct NativeAdhocSnapshotChunkPacket)];

	if (s_nativeAdhoc.socket < 0)
	{
		return;
	}

	for (;;)
	{
		SceNetEtherAddr src;
		SceUShort16 srcPort = 0;
		int len = sizeof(buffer);
		int result = sceNetAdhocPdpRecv(
			s_nativeAdhoc.socket,
			&src,
			&srcPort,
			buffer,
			&len,
			0,
			SCE_NET_ADHOC_F_NONBLOCK);

		if (result == SCE_ERROR_NET_ADHOC_WOULD_BLOCK)
		{
			break;
		}
		if (result < 0)
		{
			Platform_Log("[CTR Adhoc] PDP recv failed: 0x%08x\n", result);
			break;
		}

		struct NativeAdhocPacketHeader *header = (struct NativeAdhocPacketHeader *)buffer;
		if (!NativeAdhoc_IsPacketHeaderValid(header, len))
		{
			continue;
		}

		if (header->type == NATIVE_ADHOC_PACKET_HELLO)
		{
			if (s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_HOST)
			{
				NativeAdhoc_SetPeer(&src);
				NativeAdhoc_SendWelcome(&src);
			}
			continue;
		}

		if ((header->type == NATIVE_ADHOC_PACKET_WELCOME) && (len == (int)sizeof(struct NativeAdhocWelcomePacket)))
		{
			if (s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_CLIENT)
			{
				struct NativeAdhocWelcomePacket *packet = (struct NativeAdhocWelcomePacket *)buffer;
				s_nativeAdhoc.sessionId = packet->sessionId;
				NativeAdhoc_SetPeer(&src);
			}
			continue;
		}

		if (!s_nativeAdhoc.peerKnown || (memcmp(&src, &s_nativeAdhoc.peerMac, sizeof(src)) != 0))
		{
			continue;
		}
		s_nativeAdhoc.lastPeerTime = NativeAdhoc_Now();

		if ((header->type == NATIVE_ADHOC_PACKET_MENU_INPUT) && (len == (int)sizeof(struct NativeAdhocMenuInputPacket)))
		{
			struct NativeAdhocMenuInputPacket *packet = (struct NativeAdhocMenuInputPacket *)buffer;
			if (!s_nativeAdhoc.remoteMenuPadValid || ((s32)(packet->header.sequence - s_nativeAdhoc.rxMenuInputSequence) > 0))
			{
				s_nativeAdhoc.remoteMenuPad = packet->pad;
				s_nativeAdhoc.remoteMenuPad.connected = 1;
				s_nativeAdhoc.remoteMenuPadValid = 1;
				s_nativeAdhoc.rxMenuInputSequence = packet->header.sequence;
			}
			continue;
		}

		if ((header->type == NATIVE_ADHOC_PACKET_FRAME_INPUT) && (len == (int)sizeof(struct NativeAdhocFrameInputPacket)))
		{
			struct NativeAdhocFrameInputPacket *packet = (struct NativeAdhocFrameInputPacket *)buffer;
			if (!NativeAdhoc_PacketSessionMatches(packet->sessionId) || (packet->count > NATIVE_ADHOC_INPUT_REDUNDANCY))
			{
				continue;
			}
			int remoteIndex = NativeAdhoc_GetRemotePlayerIndex();
			for (u32 i = 0; i < packet->count; i++)
			{
				NativeAdhoc_StoreInput(remoteIndex, packet->inputs[i].frame, &packet->inputs[i].pad);
			}
			continue;
		}

		if ((header->type == NATIVE_ADHOC_PACKET_RACE_PROPOSAL) && (len == (int)sizeof(struct NativeAdhocRaceProposalPacket)))
		{
			struct NativeAdhocRaceProposalPacket *packet = (struct NativeAdhocRaceProposalPacket *)buffer;
			if ((s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_HOST) && NativeAdhoc_PacketSessionMatches(packet->sessionId) && (packet->build == BUILD))
			{
				s_nativeAdhoc.raceProposalP2 = packet->p2Character;
				s_nativeAdhoc.raceProposalValid = 1;
				Platform_Log("[CTR Adhoc] received P2 character proposal %u\n", packet->p2Character);
			}
			continue;
		}

		if ((header->type == NATIVE_ADHOC_PACKET_RACE_CONFIG) && (len == (int)sizeof(struct NativeAdhocRaceConfigPacket)))
		{
			struct NativeAdhocRaceConfigPacket *packet = (struct NativeAdhocRaceConfigPacket *)buffer;
			if ((s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_CLIENT) && NativeAdhoc_PacketSessionMatches(packet->sessionId) && (packet->build == BUILD))
			{
				if ((s_nativeAdhoc.raceSyncPending || s_nativeAdhoc.raceLoadPrepared) &&
					(packet->p2Character == s_nativeAdhoc.raceProposalP2))
				{
					s_nativeAdhoc.raceConfig = *packet;
					s_nativeAdhoc.raceConfigReceived = 1;
					NativeAdhoc_ApplyRaceConfig(packet, sdata->gGT);
					NativeAdhoc_SendRaceConfigAck(packet);
					Platform_Log("[CTR Adhoc] accepted pre-load config track=%d diff=%d laps=%d lang=%d p1=%u p2=%u\n",
						packet->currLEV, packet->arcadeDifficulty, packet->numLaps, packet->language,
						packet->p1Character, packet->p2Character);
				}
				else if (s_nativeAdhoc.raceSyncPending || s_nativeAdhoc.raceLoadPrepared)
				{
					Platform_Log("[CTR Adhoc] rejected pre-load config P2=%u expected=%u\n", packet->p2Character, s_nativeAdhoc.raceProposalP2);
				}
			}
			continue;
		}

		if ((header->type == NATIVE_ADHOC_PACKET_RACE_CONFIG_ACK) && (len == (int)sizeof(struct NativeAdhocRaceConfigAckPacket)))
		{
			struct NativeAdhocRaceConfigAckPacket *packet = (struct NativeAdhocRaceConfigAckPacket *)buffer;
			if ((s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_HOST) && NativeAdhoc_PacketSessionMatches(packet->sessionId) &&
				s_nativeAdhoc.raceConfigSent &&
				(packet->currLEV == s_nativeAdhoc.raceConfig.currLEV) &&
				(packet->p1Character == s_nativeAdhoc.raceConfig.p1Character) &&
				(packet->p2Character == s_nativeAdhoc.raceConfig.p2Character))
			{
				s_nativeAdhoc.raceConfigAcked = 1;
				Platform_Log("[CTR Adhoc] pre-load config acknowledged by client\n");
			}
			continue;
		}

		if ((header->type == NATIVE_ADHOC_PACKET_SESSION) && (len == (int)sizeof(struct NativeAdhocSessionPacket)))
		{
			struct NativeAdhocSessionPacket *packet = (struct NativeAdhocSessionPacket *)buffer;
			if ((s_nativeAdhoc.role != NATIVE_ADHOC_ROLE_CLIENT) || !NativeAdhoc_PacketSessionMatches(packet->sessionId))
			{
				continue;
			}
			s_nativeAdhoc.session = *packet;
			s_nativeAdhoc.sessionReceived = 1;
			if (s_nativeAdhoc.localLevelReady && !NativeAdhoc_SessionMatchesLocal(packet, sdata->gGT))
			{
				NativeAdhoc_LogSessionMismatch(packet, sdata->gGT);
				s_nativeAdhoc.status = NATIVE_ADHOC_STATUS_ERROR;
			}
			else if (s_nativeAdhoc.localLevelReady)
			{
				NativeAdhoc_ApplySessionConfig(packet, sdata->gGT);
				NativeAdhoc_SendReady();
			}
			continue;
		}

		if ((header->type == NATIVE_ADHOC_PACKET_READY) && (len == (int)sizeof(struct NativeAdhocReadyPacket)))
		{
			struct NativeAdhocReadyPacket *packet = (struct NativeAdhocReadyPacket *)buffer;
			if ((s_nativeAdhoc.role != NATIVE_ADHOC_ROLE_HOST) || !NativeAdhoc_PacketSessionMatches(packet->sessionId) || !s_nativeAdhoc.localLevelReady)
			{
				continue;
			}
			if (!NativeAdhoc_ReadyMatchesSession(packet))
			{
				Platform_Log("[CTR Adhoc] peer READY does not match host session\n");
				s_nativeAdhoc.status = NATIVE_ADHOC_STATUS_ERROR;
				continue;
			}
			s_nativeAdhoc.peerLevelReady = 1;
			continue;
		}

		if ((header->type == NATIVE_ADHOC_PACKET_HASH) && (len == (int)sizeof(struct NativeAdhocHashPacket)))
		{
			struct NativeAdhocHashPacket *packet = (struct NativeAdhocHashPacket *)buffer;
			if (NativeAdhoc_PacketSessionMatches(packet->sessionId))
			{
				NativeAdhoc_CheckHostHash(packet->frame, packet->hash);
			}
			continue;
		}

		if ((header->type == NATIVE_ADHOC_PACKET_DESYNC) && (len == (int)sizeof(struct NativeAdhocDesyncPacket)))
		{
			struct NativeAdhocDesyncPacket *packet = (struct NativeAdhocDesyncPacket *)buffer;
			if ((s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_HOST) && NativeAdhoc_PacketSessionMatches(packet->sessionId))
			{
				Platform_Log("[CTR Adhoc] recovery requested after desync at frame %u\n", packet->frame);
				s_nativeAdhoc.resyncRequested = 1;
			}
			continue;
		}

		if ((header->type == NATIVE_ADHOC_PACKET_SNAPSHOT_BEGIN) && (len == (int)sizeof(struct NativeAdhocSnapshotBeginPacket)))
		{
			struct NativeAdhocSnapshotBeginPacket *packet = (struct NativeAdhocSnapshotBeginPacket *)buffer;
			if ((s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_CLIENT) && NativeAdhoc_PacketSessionMatches(packet->sessionId))
			{
				if (!NativeAdhoc_PrepareSnapshotRx(packet))
				{
					s_nativeAdhoc.status = NATIVE_ADHOC_STATUS_ERROR;
				}
			}
			continue;
		}

		if (header->type == NATIVE_ADHOC_PACKET_SNAPSHOT_CHUNK)
		{
			struct NativeAdhocSnapshotChunkPacket *packet = (struct NativeAdhocSnapshotChunkPacket *)buffer;
			if ((s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_CLIENT) && NativeAdhoc_PacketSessionMatches(packet->sessionId))
			{
				NativeAdhoc_HandleSnapshotChunk(packet, len);
			}
			continue;
		}

		if ((header->type == NATIVE_ADHOC_PACKET_SNAPSHOT_ACK) && (len == (int)sizeof(struct NativeAdhocSnapshotAckPacket)))
		{
			struct NativeAdhocSnapshotAckPacket *packet = (struct NativeAdhocSnapshotAckPacket *)buffer;
			if ((s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_HOST) && NativeAdhoc_PacketSessionMatches(packet->sessionId) &&
				s_nativeAdhoc.txSnapshotActive && (packet->snapshotId == s_nativeAdhoc.txSnapshotId) &&
				(packet->chunkIndex < s_nativeAdhoc.txSnapshotChunkCount))
			{
				s_nativeAdhoc.txSnapshotAcked[packet->chunkIndex] = 1;
			}
			continue;
		}

		if ((header->type == NATIVE_ADHOC_PACKET_SNAPSHOT_DONE) && (len == (int)sizeof(struct NativeAdhocSnapshotDonePacket)))
		{
			struct NativeAdhocSnapshotDonePacket *packet = (struct NativeAdhocSnapshotDonePacket *)buffer;
			if ((s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_HOST) && NativeAdhoc_PacketSessionMatches(packet->sessionId) &&
				s_nativeAdhoc.txSnapshotActive && (packet->snapshotId == s_nativeAdhoc.txSnapshotId) &&
				(packet->simulationFrame == s_nativeAdhoc.txSnapshotFrame))
			{
				s_nativeAdhoc.txSnapshotActive = 0;
				NativeAdhoc_BeginControlFromSnapshot();
			}
			continue;
		}

		if (((header->type == NATIVE_ADHOC_PACKET_START) || (header->type == NATIVE_ADHOC_PACKET_RESUME)) && (len == (int)sizeof(struct NativeAdhocControlPacket)))
		{
			struct NativeAdhocControlPacket *packet = (struct NativeAdhocControlPacket *)buffer;
			if ((s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_CLIENT) && NativeAdhoc_PacketSessionMatches(packet->sessionId))
			{
				NativeAdhoc_ApplyControl(packet, header->type == NATIVE_ADHOC_PACKET_START ? NATIVE_ADHOC_PACKET_START_ACK : NATIVE_ADHOC_PACKET_RESUME_ACK);
			}
			continue;
		}

		if (((header->type == NATIVE_ADHOC_PACKET_START_ACK) || (header->type == NATIVE_ADHOC_PACKET_RESUME_ACK)) && (len == (int)sizeof(struct NativeAdhocControlPacket)))
		{
			struct NativeAdhocControlPacket *packet = (struct NativeAdhocControlPacket *)buffer;
			if ((s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_HOST) && NativeAdhoc_PacketSessionMatches(packet->sessionId) && s_nativeAdhoc.controlPending &&
				(packet->snapshotId == s_nativeAdhoc.controlSnapshotId) && (packet->simulationFrame == s_nativeAdhoc.controlFrame))
			{
				s_nativeAdhoc.controlPending = 0;
				s_nativeAdhoc.initialSyncComplete = 1;
				s_nativeAdhoc.simulationActive = 1;
				sdata->frameCounter = 0;
				if (sdata->gGT != NULL)
				{
					sdata->gGT->elapsedTimeMS = 0x20;
				}
				if (sdata->gGamepads != NULL)
				{
					GAMEPAD_GetNumConnected(sdata->gGamepads);
					for (int i = 0; i < 2; i++)
					{
						sdata->gGamepads->gamepad[i].buttonsHeldCurrFrame = 0;
						sdata->gGamepads->gamepad[i].buttonsHeldPrevFrame = 0;
						sdata->gGamepads->gamepad[i].buttonsTapped = 0;
						sdata->gGamepads->gamepad[i].buttonsReleased = 0;
					}
					Platform_Log("[CTR Adhoc] host pad bindings connected=%d p0=%d p1=%d local=%d\n",
						sdata->gGamepads->numGamepadsConnected,
						sdata->gGamepads->gamepad[0].ptrControllerPacket != NULL,
						sdata->gGamepads->gamepad[1].ptrControllerPacket != NULL,
						NativeAdhoc_GetLocalPlayerIndex());
				}
				if (header->type == NATIVE_ADHOC_PACKET_START_ACK)
				{
					if ((sdata->gGT != NULL) && (sdata->gGT->levelID < NITRO_COURT))
					{
						Audio_SetState_Safe(AUDIO_RACE_INTRO);
					}
					NativeAdhoc_ResetRaceConfigState();
				}
				free(s_nativeAdhoc.txSnapshot);
				free(s_nativeAdhoc.txSnapshotAcked);
				s_nativeAdhoc.txSnapshot = NULL;
				s_nativeAdhoc.txSnapshotAcked = NULL;
				Platform_Log("[CTR Adhoc] host simulation active at frame %u\n", s_nativeAdhoc.simulationFrame);
			}
			continue;
		}

		if ((header->type == NATIVE_ADHOC_PACKET_HEARTBEAT) && (len == (int)sizeof(struct NativeAdhocHeartbeatPacket)))
		{
			struct NativeAdhocHeartbeatPacket *packet = (struct NativeAdhocHeartbeatPacket *)buffer;
			if (!NativeAdhoc_PacketSessionMatches(packet->sessionId))
			{
				continue;
			}
		}
	}
}

static void NativeAdhoc_ObserveLoading(void)
{
	if (!s_nativeAdhoc.simulationActive || (sdata == NULL))
	{
		return;
	}
	if (sdata->Loading.stage == LOAD_IDLE)
	{
		return;
	}

	Platform_Log("[CTR Adhoc] loading transition, pausing lockstep at frame %u\n", s_nativeAdhoc.simulationFrame);
	s_nativeAdhoc.simulationActive = 0;
	s_nativeAdhoc.localLevelReady = 0;
	s_nativeAdhoc.peerLevelReady = 0;
	s_nativeAdhoc.sessionReceived = s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_HOST;
	s_nativeAdhoc.initialSyncComplete = 0;
	s_nativeAdhoc.controlPending = 0;
	s_nativeAdhoc.resyncRequested = 0;
	memset(s_nativeAdhoc.hashes, 0, sizeof(s_nativeAdhoc.hashes));
	s_nativeAdhoc.nextHashSlot = 0;
	NativeAdhoc_ClearInputRing();
}

static void NativeAdhoc_PumpConnected(void)
{
	u32 now = NativeAdhoc_Now();

	NativeAdhoc_ObserveLoading();

	if ((u32)(now - s_nativeAdhoc.lastHeartbeatTime) >= NATIVE_ADHOC_HEARTBEAT_INTERVAL_US)
	{
		NativeAdhoc_SendHeartbeat();
		s_nativeAdhoc.lastHeartbeatTime = now;
	}

	NativeAdhoc_PumpRaceConfig(now);

	if (s_nativeAdhoc.localLevelReady && !s_nativeAdhoc.initialSyncComplete && !s_nativeAdhoc.txSnapshotActive && !s_nativeAdhoc.rxSnapshotActive && !s_nativeAdhoc.controlPending)
	{
		if ((u32)(now - s_nativeAdhoc.lastSessionTime) >= NATIVE_ADHOC_SESSION_INTERVAL_US)
		{
			if (s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_HOST)
			{
				NativeAdhoc_SendSession();
			}
			else
			{
				NativeAdhoc_SendReady();
			}
			s_nativeAdhoc.lastSessionTime = now;
		}
	}

	if ((s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_HOST) && s_nativeAdhoc.localLevelReady && s_nativeAdhoc.peerLevelReady &&
		!s_nativeAdhoc.initialSyncComplete && !s_nativeAdhoc.controlPending &&
		!s_nativeAdhoc.txSnapshotActive && !s_nativeAdhoc.rxSnapshotActive)
	{
		if (!NativeAdhoc_BeginSnapshot(1, 0))
		{
			Platform_Log("[CTR Adhoc] failed to create initial gameplay snapshot\n");
			s_nativeAdhoc.status = NATIVE_ADHOC_STATUS_ERROR;
		}
	}

	if (s_nativeAdhoc.txSnapshotActive)
	{
		if ((u32)(now - s_nativeAdhoc.lastControlTime) >= NATIVE_ADHOC_SESSION_INTERVAL_US)
		{
			NativeAdhoc_SendSnapshotBegin();
			s_nativeAdhoc.lastControlTime = now;
		}
		NativeAdhoc_PumpSnapshotTx();
	}

	if (s_nativeAdhoc.snapshotDonePending && ((u32)(now - s_nativeAdhoc.lastControlTime) >= NATIVE_ADHOC_CONTROL_INTERVAL_US))
	{
		NativeAdhoc_SendSnapshotDone();
		s_nativeAdhoc.lastControlTime = now;
	}

	if (s_nativeAdhoc.controlPending && ((u32)(now - s_nativeAdhoc.lastControlTime) >= NATIVE_ADHOC_CONTROL_INTERVAL_US))
	{
		NativeAdhoc_SendControl();
		s_nativeAdhoc.lastControlTime = now;
	}
}

int NativeAdhoc_IsSupported(void)
{
	return 1;
}

int NativeAdhoc_Begin(int role)
{
	SceNetAdhocctlAdhocId adhocId;
	SceNetCheckDialogParam param;
	SceNetAdhocctlGroupName groupName;
	SceAppUtilInitParam appUtilInitParam;
	SceAppUtilBootParam appUtilBootParam;
	int result;

	if ((role != NATIVE_ADHOC_ROLE_HOST) && (role != NATIVE_ADHOC_ROLE_CLIENT))
	{
		return 0;
	}

	NativeAdhoc_Shutdown();
	memset(&s_nativeAdhoc, 0, sizeof(s_nativeAdhoc));
	s_nativeAdhoc.role = role;
	s_nativeAdhoc.status = NATIVE_ADHOC_STATUS_ERROR;
	s_nativeAdhoc.socket = -1;
	s_nativeAdhoc.installedFrame = NATIVE_ADHOC_INVALID_FRAME;

	memset(&appUtilInitParam, 0, sizeof(appUtilInitParam));
	memset(&appUtilBootParam, 0, sizeof(appUtilBootParam));
	result = sceAppUtilInit(&appUtilInitParam, &appUtilBootParam);
	if ((result < 0) && (result != SCE_APPUTIL_ERROR_BUSY))
	{
		Platform_Log("[CTR Adhoc] sceAppUtilInit failed: 0x%08x\n", result);
		NativeAdhoc_Shutdown();
		return 0;
	}
	if (result >= 0)
	{
		s_nativeAdhoc.appUtilInitializedByUs = 1;
	}

	if (sceSysmoduleIsLoaded(SCE_SYSMODULE_NET) != SCE_SYSMODULE_LOADED)
	{
		result = sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
		if (result < 0)
		{
			Platform_Log("[CTR Adhoc] failed to load SceNet: 0x%08x\n", result);
			NativeAdhoc_Shutdown();
			return 0;
		}
		s_nativeAdhoc.netModuleLoadedByUs = 1;
	}

	SceNetInitParam netInitParam;
	memset(&netInitParam, 0, sizeof(netInitParam));
	s_nativeAdhoc.netMemory = malloc(NATIVE_ADHOC_NET_MEMORY_SIZE);
	if (s_nativeAdhoc.netMemory == NULL)
	{
		Platform_Log("[CTR Adhoc] failed to allocate SceNet memory\n");
		NativeAdhoc_Shutdown();
		return 0;
	}
	netInitParam.memory = s_nativeAdhoc.netMemory;
	netInitParam.size = NATIVE_ADHOC_NET_MEMORY_SIZE;
	netInitParam.flags = 0;
	result = sceNetInit(&netInitParam);
	if (result < 0)
	{
		Platform_Log("[CTR Adhoc] sceNetInit failed: 0x%08x\n", result);
		NativeAdhoc_Shutdown();
		return 0;
	}
	s_nativeAdhoc.netInitialized = 1;

	result = sceNetCtlInit();
	if (result < 0)
	{
		Platform_Log("[CTR Adhoc] sceNetCtlInit failed: 0x%08x\n", result);
		NativeAdhoc_Shutdown();
		return 0;
	}
	s_nativeAdhoc.netCtlInitialized = 1;

	if (sceSysmoduleIsLoaded(SCE_SYSMODULE_PSPNET_ADHOC) != SCE_SYSMODULE_LOADED)
	{
		result = sceSysmoduleLoadModule(SCE_SYSMODULE_PSPNET_ADHOC);
		if (result < 0)
		{
			Platform_Log("[CTR Adhoc] failed to load PSPNet Adhoc: 0x%08x\n", result);
			NativeAdhoc_Shutdown();
			return 0;
		}
		s_nativeAdhoc.adhocModuleLoadedByUs = 1;
	}

	result = sceNetAdhocInit();
	if ((result < 0) && (result != SCE_ERROR_NET_ADHOC_ALREADY_INITIALIZED))
	{
		Platform_Log("[CTR Adhoc] sceNetAdhocInit failed: 0x%08x\n", result);
		NativeAdhoc_Shutdown();
		return 0;
	}
	s_nativeAdhoc.adhocInitialized = 1;

	memset(&adhocId, 0, sizeof(adhocId));
	adhocId.type = SCE_NET_ADHOCCTL_ADHOCTYPE_RESERVED;
	memcpy(adhocId.data, "CTRN00001", SCE_NET_ADHOCCTL_ADHOCID_LEN);
	result = sceNetAdhocctlInit(&adhocId);
	if ((result < 0) && (result != SCE_ERROR_NET_ADHOCCTL_ALREADY_INITIALIZED))
	{
		Platform_Log("[CTR Adhoc] sceNetAdhocctlInit failed: 0x%08x\n", result);
		NativeAdhoc_Shutdown();
		return 0;
	}
	s_nativeAdhoc.adhocctlInitialized = 1;

	sceNetCheckDialogParamInit(&param);
	memset(&groupName, 0, sizeof(groupName));
	param.groupName = &groupName;
	memcpy(param.npCommunicationId.data, "CTRN00001", 9);
	param.npCommunicationId.term = '\0';
	param.npCommunicationId.num = 0;
	param.mode = SCE_NETCHECK_DIALOG_MODE_PSP_ADHOC_CONN;
	param.timeoutUs = 0;

	result = sceNetCheckDialogInit(&param);
	if (result < 0)
	{
		Platform_Log("[CTR Adhoc] sceNetCheckDialogInit failed: 0x%08x\n", result);
		NativeAdhoc_Shutdown();
		return 0;
	}

	s_nativeAdhoc.dialogRunning = 1;
	s_nativeAdhoc.status = NATIVE_ADHOC_STATUS_DIALOG;
	Platform_Log("[CTR Adhoc] bootstrap started, role=%s\n", role == NATIVE_ADHOC_ROLE_HOST ? "host" : "client");
	return 1;
}

void NativeAdhoc_Shutdown(void)
{
	NativeAdhoc_FreeSnapshotBuffers();
	if (s_nativeAdhoc.dialogRunning)
	{
		sceNetCheckDialogTerm();
	}
	if (s_nativeAdhoc.socket >= 0)
	{
		sceNetAdhocPdpDelete(s_nativeAdhoc.socket, 0);
	}
	if (s_nativeAdhoc.netCtlInitialized)
	{
		sceNetCtlAdhocDisconnect();
	}
	if (s_nativeAdhoc.adhocctlInitialized)
	{
		sceNetAdhocctlTerm();
	}
	if (s_nativeAdhoc.adhocInitialized)
	{
		sceNetAdhocTerm();
	}
	if (s_nativeAdhoc.adhocModuleLoadedByUs)
	{
		sceSysmoduleUnloadModule(SCE_SYSMODULE_PSPNET_ADHOC);
	}
	if (s_nativeAdhoc.netCtlInitialized)
	{
		sceNetCtlTerm();
	}
	if (s_nativeAdhoc.netInitialized)
	{
		sceNetTerm();
	}
	free(s_nativeAdhoc.netMemory);
	if (s_nativeAdhoc.netModuleLoadedByUs)
	{
		sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
	}
	if (s_nativeAdhoc.appUtilInitializedByUs)
	{
		sceAppUtilShutdown();
	}

	memset(&s_nativeAdhoc, 0, sizeof(s_nativeAdhoc));
	s_nativeAdhoc.role = NATIVE_ADHOC_ROLE_NONE;
	s_nativeAdhoc.status = NATIVE_ADHOC_STATUS_OFF;
	s_nativeAdhoc.socket = -1;
	s_nativeAdhoc.installedFrame = NATIVE_ADHOC_INVALID_FRAME;
}

void NativeAdhoc_Update(void)
{
	u32 now;

	if (s_nativeAdhoc.status == NATIVE_ADHOC_STATUS_OFF)
	{
		return;
	}

	if (s_nativeAdhoc.dialogRunning)
	{
		if (sceNetCheckDialogGetStatus() == SCE_COMMON_DIALOG_STATUS_FINISHED)
		{
			NativeAdhoc_FinishDialog();
		}
		return;
	}

	if ((s_nativeAdhoc.status != NATIVE_ADHOC_STATUS_WAITING) && (s_nativeAdhoc.status != NATIVE_ADHOC_STATUS_CONNECTED))
	{
		return;
	}

	NativeAdhoc_ReceivePackets();
	now = NativeAdhoc_Now();

	if ((s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_CLIENT) && !s_nativeAdhoc.peerKnown &&
		((u32)(now - s_nativeAdhoc.lastHelloTime) >= NATIVE_ADHOC_HELLO_INTERVAL_US))
	{
		NativeAdhoc_SendHello();
		s_nativeAdhoc.lastHelloTime = now;
	}

	if (s_nativeAdhoc.peerKnown)
	{
		NativeAdhoc_PumpConnected();
		if ((u32)(now - s_nativeAdhoc.lastPeerTime) > NATIVE_ADHOC_PEER_TIMEOUT_US)
		{
			Platform_Log("[CTR Adhoc] peer timed out after %u ms, returning to main menu\n",
			             NATIVE_ADHOC_PEER_TIMEOUT_US / 1000u);
			NativeAdhoc_FreeSnapshotBuffers();
			s_nativeAdhoc.peerKnown = 0;
			s_nativeAdhoc.remoteMenuPadValid = 0;
			NativeAdhoc_ResetRaceConfigState();
			s_nativeAdhoc.localLevelReady = 0;
			s_nativeAdhoc.peerLevelReady = 0;
			s_nativeAdhoc.sessionReceived = 0;
			s_nativeAdhoc.initialSyncComplete = 0;
			s_nativeAdhoc.simulationActive = 0;
			s_nativeAdhoc.controlPending = 0;
			NativeAdhoc_ClearInputRing();
			s_nativeAdhoc.status = NATIVE_ADHOC_STATUS_ERROR;
			s_nativeAdhocReturnToMainMenuPending = 1;
			s_nativeAdhocReturnVisualPending = 1;
			s_nativeAdhocConnectionLostNoticePending = 1;
			s_nativeAdhocConnectionLostNoticeUntil = 0;
		}
	}
}

void NativeAdhoc_WaitForFrame(void)
{
	Platform_PollInput();
	sceDisplayWaitVblankStart();
}

void NativeAdhoc_ProcessPadSnapshots(struct PlatformInputPadSnapshot *pads, int count)
{
	struct PlatformInputPadSnapshot localPad;
	struct PlatformInputPadSnapshot p0;
	struct PlatformInputPadSnapshot p1;
	int localIndex;
	int remoteIndex;

	if ((pads == NULL) || (count < 2) || !NativeAdhoc_IsConnected())
	{
		return;
	}

	localPad = pads[0];
	localPad.connected = 1;
	localIndex = NativeAdhoc_GetLocalPlayerIndex();
	remoteIndex = 1 - localIndex;

	if (s_nativeAdhoc.simulationActive)
	{
		u32 targetFrame = s_nativeAdhoc.simulationFrame + NATIVE_ADHOC_INPUT_DELAY;
		if (!NativeAdhoc_GetInput(localIndex, targetFrame, NULL))
		{
			NativeAdhoc_StoreInput(localIndex, targetFrame, &localPad);
			s_nativeAdhoc.latestLocalInputFrame = targetFrame;
			s_nativeAdhoc.latestLocalInputValid = 1;
		}
		NativeAdhoc_SendFrameInputs();

		int haveP0 = NativeAdhoc_GetInput(0, s_nativeAdhoc.simulationFrame, &p0);
		int haveP1 = NativeAdhoc_GetInput(1, s_nativeAdhoc.simulationFrame, &p1);
		if (!haveP0)
		{
			NativeAdhoc_MakeNeutralPad(&p0);
		}
		if (!haveP1)
		{
			NativeAdhoc_MakeNeutralPad(&p1);
		}

		// Once lockstep is active, never expose the physical Vita controller
		// directly on canonical slot 0. On a joining client the physical pad
		// belongs to P2; leaking it through pads[0] makes it control P1 while
		// waiting for the peer input and contaminates the next simulation tick.
		pads[0] = p0;
		pads[1] = p1;
		s_nativeAdhoc.installedFrame = (haveP0 && haveP1) ? s_nativeAdhoc.simulationFrame : NATIVE_ADHOC_INVALID_FRAME;
		return;
	}

	NativeAdhoc_SendMenuInput(&localPad);
	pads[localIndex] = localPad;
	if (s_nativeAdhoc.remoteMenuPadValid)
	{
		pads[remoteIndex] = s_nativeAdhoc.remoteMenuPad;
		pads[remoteIndex].connected = 1;
	}
	else
	{
		NativeAdhoc_MakeNeutralPad(&pads[remoteIndex]);
	}
}

void NativeAdhoc_NotifyLevelReady(struct GameTracker *gGT)
{
	if (!NativeAdhoc_IsConnected() || (gGT == NULL) || (gGT->numPlyrCurrGame != 2) || ((gGT->gameMode1 & MAIN_MENU) != 0))
	{
		return;
	}

	NativeAdhoc_EnforcePreparedRaceConfig(gGT);
	s_nativeAdhoc.localLevelReady = 1;
	s_nativeAdhoc.peerLevelReady = 0;
	s_nativeAdhoc.initialSyncComplete = 0;
	s_nativeAdhoc.simulationActive = 0;
	s_nativeAdhoc.lastSessionTime = 0;
	s_nativeAdhoc.installedFrame = NATIVE_ADHOC_INVALID_FRAME;
	memset(s_nativeAdhoc.hashes, 0, sizeof(s_nativeAdhoc.hashes));
	s_nativeAdhoc.nextHashSlot = 0;
	NativeAdhoc_ClearInputRing();

	if (s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_HOST)
	{
		NativeAdhoc_FillSession(&s_nativeAdhoc.session, gGT);
		s_nativeAdhoc.sessionReceived = 1;
		NativeAdhoc_SendSession();
	}
	else
	{
		if (s_nativeAdhoc.sessionReceived && !NativeAdhoc_SessionMatchesLocal(&s_nativeAdhoc.session, gGT))
		{
			NativeAdhoc_LogSessionMismatch(&s_nativeAdhoc.session, gGT);
			s_nativeAdhoc.status = NATIVE_ADHOC_STATUS_ERROR;
			return;
		}
		if (s_nativeAdhoc.sessionReceived)
		{
			NativeAdhoc_ApplySessionConfig(&s_nativeAdhoc.session, gGT);
		}
		NativeAdhoc_SendReady();
	}
	Platform_Log("[CTR Adhoc] local level ready id=%d\n", gGT->levelID);
}

static int NativeAdhoc_ApplyPendingSnapshot(void)
{
	struct GameTracker *gGT;

	if (!s_nativeAdhoc.rxRestorePending)
	{
		return 1;
	}
	if (!NativeCheckpoint_RestoreGameplay(s_nativeAdhoc.rxSnapshot, (int)s_nativeAdhoc.rxSnapshotSize))
	{
		Platform_Log("[CTR Adhoc] gameplay checkpoint restore failed\n");
		s_nativeAdhoc.status = NATIVE_ADHOC_STATUS_ERROR;
		return 0;
	}

	gGT = sdata->gGT;
	if (gGT != NULL)
	{
		MainDB_RebindNativePrimMem(gGT);
		gGT->ptrRenderBucketInstance = RenderBucket_GetNativeStorage();
	}
	if ((gGT != NULL) && (gGT->level1 != NULL))
	{
		// Visibility/PVS state is a render-only runtime cache. The checkpoint
		// restores the host mempack contents, but fields such as VisMem source
		// caches and bspList[].bsp are populated at runtime and are not all LEV
		// pointer-map slots. Rebuild them from the client's relocated LEV before
		// the first post-restore frame instead of consuming host cache pointers.
		MainInit_VisMem(gGT);
		memset(gGT->LevRenderLists, 0, sizeof(gGT->LevRenderLists));
		Platform_Log("[CTR Adhoc] rebuilt render visibility cache level=%p vis=%p mesh=%p\n",
			(void *)gGT->level1, (void *)gGT->visMem1, (void *)gGT->level1->ptr_mesh_info);
	}
	NativeAdhoc_ResetLockstep(s_nativeAdhoc.rxSnapshotFrame);
	s_nativeAdhoc.lastRestoredSnapshotId = s_nativeAdhoc.rxSnapshotId;
	s_nativeAdhoc.rxRestorePending = 0;
	s_nativeAdhoc.rxSnapshotActive = 0;
	free(s_nativeAdhoc.rxSnapshot);
	free(s_nativeAdhoc.rxSnapshotReceived);
	s_nativeAdhoc.rxSnapshot = NULL;
	s_nativeAdhoc.rxSnapshotReceived = NULL;
	s_nativeAdhoc.snapshotDonePending = 1;
	s_nativeAdhoc.lastControlTime = 0;
	NativeAdhoc_SendSnapshotDone();
	Platform_Log("[CTR Adhoc] restored snapshot #%u at frame %u\n", s_nativeAdhoc.rxSnapshotId, s_nativeAdhoc.rxSnapshotFrame);
	return 1;
}

int NativeAdhoc_BeginSimulationFrame(struct GameTracker *gGT)
{
	NativeAdhoc_Update();
	if (s_nativeAdhocReturnToMainMenuPending)
	{
		return 0;
	}
	if (!NativeAdhoc_IsConnected() || (gGT == NULL) || (gGT->numPlyrCurrGame != 2) || ((gGT->gameMode1 & MAIN_MENU) != 0))
	{
		return 1;
	}

	// Do not freeze the race-loading/checkered-flag state machine before
	// NativeAdhoc_NotifyLevelReady() has been reached. The state-pure wait is
	// only valid once this console has finished local level initialization.
	if (!s_nativeAdhoc.localLevelReady)
	{
		return 1;
	}

	if (!NativeAdhoc_ApplyPendingSnapshot())
	{
		return 0;
	}

	if ((s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_HOST) && s_nativeAdhoc.resyncRequested && s_nativeAdhoc.simulationActive)
	{
		u32 recoveryFrame = s_nativeAdhoc.simulationFrame;
		s_nativeAdhoc.resyncRequested = 0;
		if (!NativeAdhoc_BeginSnapshot(0, recoveryFrame))
		{
			Platform_Log("[CTR Adhoc] failed to create recovery snapshot at frame %u\n", recoveryFrame);
			s_nativeAdhoc.status = NATIVE_ADHOC_STATUS_ERROR;
			s_nativeAdhoc.simulationActive = 0;
		}
		return 0;
	}

	if (!s_nativeAdhoc.simulationActive)
	{
		return 0;
	}
	if (s_nativeAdhoc.installedFrame != s_nativeAdhoc.simulationFrame)
	{
		return 0;
	}
	gGT->elapsedTimeMS = 0x20;
	return 1;
}

void NativeAdhoc_EndSimulationFrame(struct GameTracker *gGT)
{
	u32 frame;
	u32 hash;

	if (!s_nativeAdhoc.simulationActive || (gGT == NULL))
	{
		return;
	}

	frame = s_nativeAdhoc.simulationFrame;
	if ((frame % NATIVE_ADHOC_HASH_INTERVAL) == 0)
	{
		hash = NativeAdhoc_GameplayHash(gGT);
		NativeAdhoc_SaveHash(frame, hash);
		if (frame <= 60u)
		{
			struct Driver *d0 = gGT->drivers[0];
			struct Driver *d1 = gGT->drivers[1];
			Platform_Log(
				"[CTR Adhoc] state role=%s frame=%u hash=0x%08x rng=0x%08x d0=%d pos=(%d,%d,%d) d1=%d pos=(%d,%d,%d)\n",
				s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_HOST ? "host" : "client",
				frame,
				hash,
				(u32)sdata->randomNumber,
				d0 != NULL ? d0->driverID : -1,
				d0 != NULL ? d0->posCurr.x : 0,
				d0 != NULL ? d0->posCurr.y : 0,
				d0 != NULL ? d0->posCurr.z : 0,
				d1 != NULL ? d1->driverID : -1,
				d1 != NULL ? d1->posCurr.x : 0,
				d1 != NULL ? d1->posCurr.y : 0,
				d1 != NULL ? d1->posCurr.z : 0);
		}
		if (s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_HOST)
		{
			NativeAdhoc_SendHash(frame, hash);
		}
		else if (s_nativeAdhoc.pendingHostHashValid && (s_nativeAdhoc.pendingHostHashFrame == frame))
		{
			if (hash != s_nativeAdhoc.pendingHostHash)
			{
				NativeAdhoc_SendDesync(frame, s_nativeAdhoc.pendingHostHash, hash);
			}
			s_nativeAdhoc.pendingHostHashValid = 0;
		}
	}

	s_nativeAdhoc.simulationFrame++;
	s_nativeAdhoc.installedFrame = NATIVE_ADHOC_INVALID_FRAME;
}

int NativeAdhoc_IsActive(void)
{
	return s_nativeAdhoc.role != NATIVE_ADHOC_ROLE_NONE;
}

int NativeAdhoc_IsConnected(void)
{
	return (s_nativeAdhoc.status == NATIVE_ADHOC_STATUS_CONNECTED) && s_nativeAdhoc.peerKnown;
}

int NativeAdhoc_IsSimulationActive(void)
{
	return NativeAdhoc_IsConnected() && s_nativeAdhoc.simulationActive;
}

int NativeAdhoc_IsDialogRunning(void)
{
	return s_nativeAdhoc.dialogRunning;
}

int NativeAdhoc_ShouldPresentDriver(int driverID)
{
	struct GameTracker *gGT = sdata->gGT;
	if (!NativeAdhoc_IsConnected() || (gGT == NULL) || (gGT->numPlyrCurrGame != 2) || ((gGT->gameMode1 & MAIN_MENU) != 0))
	{
		return 1;
	}
	if ((driverID < 0) || (driverID >= 2))
	{
		return 1;
	}
	return driverID == NativeAdhoc_GetLocalPlayerIndex();
}

int NativeAdhoc_ShouldReturnToMainMenu(void)
{
	return s_nativeAdhocReturnToMainMenuPending;
}

void NativeAdhoc_RequestReturnToMainMenu(void)
{
	s_nativeAdhocReturnToMainMenuPending = 1;
	s_nativeAdhocReturnVisualPending = 1;
	s_nativeAdhocConnectionLostNoticePending = 0;
	s_nativeAdhocConnectionLostNoticeUntil = 0;
}

void NativeAdhoc_AcknowledgeReturnToMainMenu(void)
{
	s_nativeAdhocReturnToMainMenuPending = 0;
}

int NativeAdhoc_IsReturningToMainMenu(void)
{
	if (!s_nativeAdhocReturnVisualPending)
	{
		return 0;
	}

	struct GameTracker *gGT = sdata->gGT;
	return (gGT == NULL) || (gGT->levelID != MAIN_MENU_LEVEL) || (sdata->Loading.stage != LOAD_IDLE);
}

int NativeAdhoc_ShouldDrawConnectionLostNotice(void)
{
	if (s_nativeAdhocReturnVisualPending)
	{
		struct GameTracker *gGT = sdata->gGT;
		if ((gGT == NULL) || (gGT->levelID != MAIN_MENU_LEVEL) || (sdata->Loading.stage != LOAD_IDLE))
		{
			return 0;
		}

		RaceFlag_SetFullyOffScreen();
		MM_JumpTo_Title_Returning();
		s_nativeAdhocReturnVisualPending = 0;
		if (s_nativeAdhocConnectionLostNoticePending)
		{
			s_nativeAdhocConnectionLostNoticePending = 0;
			s_nativeAdhocConnectionLostNoticeUntil = NativeAdhoc_Now() + 6000000u;
		}
	}

	if (s_nativeAdhocConnectionLostNoticeUntil == 0)
	{
		return 0;
	}
	return (s32)(s_nativeAdhocConnectionLostNoticeUntil - NativeAdhoc_Now()) > 0;
}

int NativeAdhoc_GetRole(void)
{
	return s_nativeAdhoc.role;
}

int NativeAdhoc_GetStatus(void)
{
	return s_nativeAdhoc.status;
}

int NativeAdhoc_GetLocalPlayerIndex(void)
{
	return s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_CLIENT ? 1 : 0;
}

int NativeAdhoc_GetRemotePlayerIndex(void)
{
	return 1 - NativeAdhoc_GetLocalPlayerIndex();
}

u32 NativeAdhoc_GetSimulationFrame(void)
{
	return s_nativeAdhoc.simulationFrame;
}

const char *NativeAdhoc_GetStatusText(void)
{
	if ((s_nativeAdhoc.status == NATIVE_ADHOC_STATUS_CONNECTED) && s_nativeAdhoc.localLevelReady && !s_nativeAdhoc.simulationActive)
	{
		return "SYNCHRONIZING GAME";
	}
	switch (s_nativeAdhoc.status)
	{
	case NATIVE_ADHOC_STATUS_DIALOG:
		return "SELECT ADHOC NETWORK";
	case NATIVE_ADHOC_STATUS_WAITING:
		return s_nativeAdhoc.role == NATIVE_ADHOC_ROLE_HOST ? "WAITING FOR PLAYER" : "SEARCHING FOR HOST";
	case NATIVE_ADHOC_STATUS_CONNECTED:
		return "PLAYER CONNECTED";
	case NATIVE_ADHOC_STATUS_ERROR:
		return "ADHOC CONNECTION ERROR";
	default:
		return "ADHOC";
	}
}

#else

int NativeAdhoc_IsSupported(void)
{
	return 0;
}

int NativeAdhoc_Begin(int role)
{
	(void)role;
	return 0;
}

void NativeAdhoc_Shutdown(void)
{
}

void NativeAdhoc_Update(void)
{
}

void NativeAdhoc_WaitForFrame(void)
{
}

void NativeAdhoc_ProcessPadSnapshots(struct PlatformInputPadSnapshot *pads, int count)
{
	(void)pads;
	(void)count;
}

int NativeAdhoc_PrepareRaceLoad(struct GameTracker *gGT)
{
	(void)gGT;
	return 1;
}

int NativeAdhoc_EnforcePreparedRaceConfig(struct GameTracker *gGT)
{
	(void)gGT;
	return 0;
}

void NativeAdhoc_NotifyLevelReady(struct GameTracker *gGT)
{
	(void)gGT;
}

int NativeAdhoc_BeginSimulationFrame(struct GameTracker *gGT)
{
	(void)gGT;
	return 1;
}

void NativeAdhoc_EndSimulationFrame(struct GameTracker *gGT)
{
	(void)gGT;
}

void NativeAdhoc_BeginRenderFrame(struct GameTracker *gGT)
{
	(void)gGT;
}

void NativeAdhoc_EndRenderFrame(void)
{
}

int NativeAdhoc_IsSingleViewRenderActive(void)
{
	return 0;
}

struct PushBuffer *NativeAdhoc_GetRenderPushBuffer(void)
{
	return NULL;
}

int NativeAdhoc_IsActive(void)
{
	return 0;
}

int NativeAdhoc_IsConnected(void)
{
	return 0;
}

int NativeAdhoc_IsSimulationActive(void)
{
	return 0;
}

int NativeAdhoc_IsDialogRunning(void)
{
	return 0;
}

int NativeAdhoc_ShouldPresentDriver(int driverID)
{
	(void)driverID;
	return 1;
}

int NativeAdhoc_ShouldReturnToMainMenu(void)
{
	return 0;
}

void NativeAdhoc_RequestReturnToMainMenu(void)
{
}

void NativeAdhoc_AcknowledgeReturnToMainMenu(void)
{
}

int NativeAdhoc_ShouldDrawConnectionLostNotice(void)
{
	return 0;
}

int NativeAdhoc_IsReturningToMainMenu(void)
{
	return 0;
}

int NativeAdhoc_GetRole(void)
{
	return NATIVE_ADHOC_ROLE_NONE;
}

int NativeAdhoc_GetStatus(void)
{
	return NATIVE_ADHOC_STATUS_OFF;
}

int NativeAdhoc_GetLocalPlayerIndex(void)
{
	return 0;
}

int NativeAdhoc_GetRemotePlayerIndex(void)
{
	return 1;
}

u32 NativeAdhoc_GetSimulationFrame(void)
{
	return 0;
}

const char *NativeAdhoc_GetStatusText(void)
{
	return "ADHOC UNAVAILABLE";
}

#endif
