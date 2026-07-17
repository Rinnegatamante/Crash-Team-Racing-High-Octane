#ifndef CTR_NATIVE_NAMESPACE_GHOST_H
#define CTR_NATIVE_NAMESPACE_GHOST_H

// GhostTape is 0x268 large
// GhostRecBuf is 0x3e00

enum
{
	GHOST_OP_POSITION = 0x80,
	GHOST_OP_ANIMATION,
	GHOST_OP_BOOST,
	GHOST_OP_INSTANCE,
	GHOST_OP_IDLE,
};
typedef u8 GhostOpcode;

enum GhostPacketSize
{
	GHOST_SIZE_POSITION = 11,
	GHOST_SIZE_ANIMATION = 3,
	GHOST_SIZE_BOOST = 6,
	GHOST_SIZE_INSTANCE = 2,
	GHOST_SIZE_IDLE = 1,
	GHOST_SIZE_VELOCITY = 5,
};

enum GhostTapeRecordConstant
{
	GHOST_RECORD_BUFFER_SIZE = 0x3e00,
	GHOST_TAPE_VERSION_RETAIL = -4,
	GHOST_RECORD_POSITION_SHIFT = 3,
	GHOST_RECORD_ROTATION_SHIFT = 4,
	GHOST_RECORD_INTERVAL_MASK_8 = 7,
	GHOST_RECORD_INTERVAL_MASK_32 = 0x1f,
	GHOST_RECORD_VELOCITY_MAX = 0x80,
	GHOST_RECORD_VELOCITY_MIN_EXCLUSIVE = -0x7c,
	GHOST_RECORD_TIME_DELTA_MAX_EXCLUSIVE = 0xff01,
	GHOST_RECORD_INSTANCE_SPLIT_FLAG = 0x2000,
	GHOST_RECORD_INSTANCE_SPLIT_SHIFT = 0xd,
	GHOST_RECORD_BUFFER_END_GUARD = 0x40,
	GHOST_RECORD_OVERFLOW_TEXT_FRAMES = CTR_SECONDS_TO_FRAMES(6),
	GHOST_RECORD_BOOST_COOLDOWN_FRAMES = 0x1e,
};

CTR_STATIC_ASSERT(sizeof(GhostOpcode) == 0x1);
CTR_STATIC_ASSERT(GHOST_OP_POSITION == 0x80);
CTR_STATIC_ASSERT(GHOST_OP_IDLE == 0x84);
CTR_STATIC_ASSERT(GHOST_SIZE_POSITION == 11);
CTR_STATIC_ASSERT(GHOST_SIZE_ANIMATION == 3);
CTR_STATIC_ASSERT(GHOST_SIZE_BOOST == 6);
CTR_STATIC_ASSERT(GHOST_SIZE_INSTANCE == 2);
CTR_STATIC_ASSERT(GHOST_SIZE_IDLE == 1);
CTR_STATIC_ASSERT(GHOST_SIZE_VELOCITY == 5);
CTR_STATIC_ASSERT(GHOST_RECORD_BUFFER_SIZE == 0x3e00);
CTR_STATIC_ASSERT(GHOST_TAPE_VERSION_RETAIL == -4);
CTR_STATIC_ASSERT(GHOST_RECORD_POSITION_SHIFT == 3);
CTR_STATIC_ASSERT(GHOST_RECORD_ROTATION_SHIFT == 4);
CTR_STATIC_ASSERT(GHOST_RECORD_INTERVAL_MASK_8 == 7);
CTR_STATIC_ASSERT(GHOST_RECORD_INTERVAL_MASK_32 == 0x1f);
CTR_STATIC_ASSERT(GHOST_RECORD_VELOCITY_MAX == 0x80);
CTR_STATIC_ASSERT(GHOST_RECORD_VELOCITY_MIN_EXCLUSIVE == -0x7c);
CTR_STATIC_ASSERT(GHOST_RECORD_TIME_DELTA_MAX_EXCLUSIVE == 0xff01);
CTR_STATIC_ASSERT(GHOST_RECORD_INSTANCE_SPLIT_FLAG == 0x2000);
CTR_STATIC_ASSERT(GHOST_RECORD_INSTANCE_SPLIT_SHIFT == 0xd);
CTR_STATIC_ASSERT(GHOST_RECORD_BUFFER_END_GUARD == 0x40);
CTR_STATIC_ASSERT(GHOST_RECORD_OVERFLOW_TEXT_FRAMES == 0xb4);
CTR_STATIC_ASSERT(GHOST_RECORD_BOOST_COOLDOWN_FRAMES == 0x1e);

#define GHOST_IS_OPCODE(b) ((u8)((b) + GHOST_OP_POSITION) < (GHOST_OP_IDLE - GHOST_OP_POSITION + 1))
#define Ghost_ReadBE16(p)  ((u16)((p)[0] << 8 | (p)[1]))

struct GhostPacket
{
	SVec3 pos;
	SVec3 rot;

	u8 *bufferPacket;

	// 0x10 -- size of packet
};

CTR_STATIC_ASSERT(sizeof(struct GhostPacket) == 0x10);

struct GhostTape
{
	// 0x0
	struct GhostHeader *gh;
	void *ptrStart; // gh->0x28
	void *ptrEnd;   // gh->0x28 + gh->size
	void *ptrCurr;

	// 0x10
	int unk10;

	// 0x14
	int timeElapsedInRace;

	// 0x18
	int timeInPacket32_backup;

	// 0x1c
	int unk1C;
	int unk20;

	// 0x24
	// all in GhostReplay_Init2, unused?
	SVec3 unk1;
	SVec3 unk2;
	SVec3 unk3;
	SVec3 unk4;

	// 0x3C
	int timeInPacket01;

	// 0x40
	int timeInPacket32;

	// 0x44
	int timeBetweenPackets;

	// 0x48
	int numPacketsInArray;

	// 0x4C
	int packetID;

	// 0x50
	struct GhostPacket packets[0x21];

	// 0x260
	int constDEADC0ED;

	// 0x264
	struct GhostHeader *gh_again; // duplicate?

	// 0x268 bytes large
};


struct GhostHeader
{
	// 0x0
	s16 version;
	s16 size;

	// 0x4
	s16 levelID;

	// 0x6
	s16 characterID;

	// 0x8
	int speedApprox; // useless decoy
	int ySpeed;      // useless decoy

	// 0x10
	int timeElapsedInRace;

	// 0x14
	// try fresh-boot time trial, dereference 8008fbf4,
	// you'll see it's all zeros, beat the race, double-deref 8008d754,
	// and it's still all zeros, could be accident, or a throw-off.
	// Only time this is non-zero is if pre-existing memory isn't wiped
	char emptyPadding[0x14];

	// 0x28
	// char recordBuffer[0]; // yes, zero bytes
};

#define GHOSTHEADER_GETRECORDBUFFER(x) (char *)((u32)x + sizeof(struct GhostHeader))

#endif
