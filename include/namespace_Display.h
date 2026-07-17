#ifndef CTR_NATIVE_NAMESPACE_DISPLAY_H
#define CTR_NATIVE_NAMESPACE_DISPLAY_H

struct PrimMem
{
	// 0x0
	u32 capacityBytes;

	// 0x4
	void *start;

	// 0x8
	void *end;

	// 0xC
	void *cursor;

	// 0x10
	// NOTE(aalhendi): Retail keeps 0x100 bytes reserved after this guard.
	void *guardEnd;

	// 0x14
	s32 primitiveCount;

	// 0x18
	// NOTE(aalhendi): Retail mirrors the allocation base here.
	void *allocationStart;
};

struct OTMem
{
	// 0x0
	u32 capacityBytes;

	// 0x4
	uint32_t *start;

	// 0x8
	uint32_t *end;

	// 0xC
	uint32_t *cursor;

	// 0x10
	// NOTE(aalhendi): UI ordering-table pointer, also stored in pushBuffer_UI.
	uint32_t *uiOT;
};

struct DisplayBlurFlatPacket
{
	u32 tag;
	u32 drawModeStart;
	u32 maskBitEnable;
	u32 colorAndCode;
	u32 xy0;
	u32 xy1;
	u32 xy2;
	u32 xy3;
	u32 drawModeEnd;
	u32 maskBitDisable;
};

struct DisplayBlurTile
{
	s16 srcX;
	s16 srcY;
	s16 srcW;
	s16 srcH;
	s16 dstX;
	s16 dstY;
	s16 dstW;
	s16 dstH;
};

// 0xA4
struct DB
{
	// 0x00
	DRAWENV drawEnv;

	// 0x5C
	DISPENV dispEnv;

	// 0x70
	u8 blurCameraMask;
	u8 blurCameraMaskPadding[3];

	// 0x74
	struct PrimMem primMem;

	// 0x90
	struct OTMem otMem;
};

CTR_STATIC_ASSERT(sizeof(struct PrimMem) == 0x1C);
CTR_STATIC_ASSERT(offsetof(struct PrimMem, capacityBytes) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct PrimMem, start) == 0x4);
CTR_STATIC_ASSERT(offsetof(struct PrimMem, end) == 0x8);
CTR_STATIC_ASSERT(offsetof(struct PrimMem, cursor) == 0xC);
CTR_STATIC_ASSERT(offsetof(struct PrimMem, guardEnd) == 0x10);
CTR_STATIC_ASSERT(offsetof(struct PrimMem, primitiveCount) == 0x14);
CTR_STATIC_ASSERT(offsetof(struct PrimMem, allocationStart) == 0x18);
CTR_STATIC_ASSERT(sizeof(struct OTMem) == 0x14);
CTR_STATIC_ASSERT(offsetof(struct OTMem, capacityBytes) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct OTMem, start) == 0x4);
CTR_STATIC_ASSERT(offsetof(struct OTMem, end) == 0x8);
CTR_STATIC_ASSERT(offsetof(struct OTMem, cursor) == 0xC);
CTR_STATIC_ASSERT(offsetof(struct OTMem, uiOT) == 0x10);
CTR_STATIC_ASSERT(sizeof(struct DisplayBlurFlatPacket) == 0x28);
CTR_STATIC_ASSERT(offsetof(struct DisplayBlurFlatPacket, tag) == 0x00);
CTR_STATIC_ASSERT(offsetof(struct DisplayBlurFlatPacket, drawModeStart) == 0x04);
CTR_STATIC_ASSERT(offsetof(struct DisplayBlurFlatPacket, maskBitEnable) == 0x08);
CTR_STATIC_ASSERT(offsetof(struct DisplayBlurFlatPacket, colorAndCode) == 0x0C);
CTR_STATIC_ASSERT(offsetof(struct DisplayBlurFlatPacket, xy0) == 0x10);
CTR_STATIC_ASSERT(offsetof(struct DisplayBlurFlatPacket, xy1) == 0x14);
CTR_STATIC_ASSERT(offsetof(struct DisplayBlurFlatPacket, xy2) == 0x18);
CTR_STATIC_ASSERT(offsetof(struct DisplayBlurFlatPacket, xy3) == 0x1C);
CTR_STATIC_ASSERT(offsetof(struct DisplayBlurFlatPacket, drawModeEnd) == 0x20);
CTR_STATIC_ASSERT(offsetof(struct DisplayBlurFlatPacket, maskBitDisable) == 0x24);
CTR_STATIC_ASSERT(sizeof(struct DisplayBlurTile) == 0x10);
CTR_STATIC_ASSERT(offsetof(struct DisplayBlurTile, srcX) == 0x00);
CTR_STATIC_ASSERT(offsetof(struct DisplayBlurTile, srcY) == 0x02);
CTR_STATIC_ASSERT(offsetof(struct DisplayBlurTile, srcW) == 0x04);
CTR_STATIC_ASSERT(offsetof(struct DisplayBlurTile, srcH) == 0x06);
CTR_STATIC_ASSERT(offsetof(struct DisplayBlurTile, dstX) == 0x08);
CTR_STATIC_ASSERT(offsetof(struct DisplayBlurTile, dstY) == 0x0A);
CTR_STATIC_ASSERT(offsetof(struct DisplayBlurTile, dstW) == 0x0C);
CTR_STATIC_ASSERT(offsetof(struct DisplayBlurTile, dstH) == 0x0E);
#ifndef CTR_NATIVE
CTR_STATIC_ASSERT(offsetof(struct DB, drawEnv) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct DB, dispEnv) == 0x5C);
CTR_STATIC_ASSERT(offsetof(struct DB, blurCameraMask) == 0x70);
CTR_STATIC_ASSERT(offsetof(struct DB, primMem) == 0x74);
CTR_STATIC_ASSERT(offsetof(struct DB, otMem) == 0x90);
CTR_STATIC_ASSERT(sizeof(struct DB) == 0xA4);
#endif

#endif
