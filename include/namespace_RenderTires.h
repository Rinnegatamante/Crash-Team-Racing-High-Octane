#ifndef CTR_NATIVE_NAMESPACE_RENDERTIRES_H
#define CTR_NATIVE_NAMESPACE_RENDERTIRES_H

#include <ctr_math.h>

struct Icon;

union DrawTiresScratchWord
{
	struct
	{
		s16 lo;
		s16 hi;
	};
	s32 word;
};

struct DrawTiresPackedVec3
{
	s16 x;
	s16 y;
	union DrawTiresScratchWord z;
};

struct DrawTiresWheelLocal
{
	struct DrawTiresPackedVec3 center;
	struct DrawTiresPackedVec3 rim;
};

struct DrawTiresScratch
{
	u32 savedRegs[12];
	s32 numPlyr;
	s32 playerCounter;
	struct Icon **wheelSprites;
	u32 tireColor;
	s32 otRangeNormal;
	s32 otRangeSecondary;
	union DrawTiresScratchWord wheelSize;
	s16 vertSplit;
	s16 pad4e;
	s16 splitCameraY;
	s16 pad52;
	s32 lodThreshold;
	struct DrawTiresWheelLocal wheelLocal[4];
	struct DrawTiresPackedVec3 viewNormalVectors[4];
	struct DrawTiresPackedVec3 transformedRimVectors[4];
	SVec3Slot tireAxisA[4];
	SVec3Slot tireAxisB[4];
	s32 projectedSxy[4];
	s32 cornerDepthBias[2];
	u32 jumpTable[8];
	u32 pad150[4];
	s32 instFlags;
	s32 pad164[2];
	s16 depthOffsetStartBytes;
	s16 depthOffsetEndBytes;
	s32 otRangeStart;
	s32 otRangeEnd;
};

CTR_STATIC_ASSERT(sizeof(union DrawTiresScratchWord) == 0x4);
CTR_STATIC_ASSERT(offsetof(union DrawTiresScratchWord, lo) == 0x0);
CTR_STATIC_ASSERT(offsetof(union DrawTiresScratchWord, hi) == 0x2);
CTR_STATIC_ASSERT(sizeof(struct DrawTiresPackedVec3) == 0x8);
CTR_STATIC_ASSERT(offsetof(struct DrawTiresPackedVec3, x) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct DrawTiresPackedVec3, y) == 0x2);
CTR_STATIC_ASSERT(offsetof(struct DrawTiresPackedVec3, z) == 0x4);
CTR_STATIC_ASSERT(sizeof(struct DrawTiresWheelLocal) == 0x10);
CTR_STATIC_ASSERT(offsetof(struct DrawTiresWheelLocal, center) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct DrawTiresWheelLocal, rim) == 0x8);
CTR_STATIC_ASSERT(offsetof(struct DrawTiresScratch, numPlyr) == 0x30);
CTR_STATIC_ASSERT(offsetof(struct DrawTiresScratch, playerCounter) == 0x34);
CTR_STATIC_ASSERT(offsetof(struct DrawTiresScratch, wheelSprites) == 0x38);
CTR_STATIC_ASSERT(offsetof(struct DrawTiresScratch, tireColor) == 0x3c);
CTR_STATIC_ASSERT(offsetof(struct DrawTiresScratch, otRangeNormal) == 0x40);
CTR_STATIC_ASSERT(offsetof(struct DrawTiresScratch, otRangeSecondary) == 0x44);
CTR_STATIC_ASSERT(offsetof(struct DrawTiresScratch, wheelSize) == 0x48);
CTR_STATIC_ASSERT(offsetof(struct DrawTiresScratch, vertSplit) == 0x4c);
CTR_STATIC_ASSERT(offsetof(struct DrawTiresScratch, splitCameraY) == 0x50);
CTR_STATIC_ASSERT(offsetof(struct DrawTiresScratch, lodThreshold) == 0x54);
CTR_STATIC_ASSERT(CTR_OFFSET_OF_ARRAY(struct DrawTiresScratch, wheelLocal, 0) == 0x58);
CTR_STATIC_ASSERT(CTR_OFFSET_OF_ARRAY(struct DrawTiresScratch, wheelLocal, 1) == 0x68);
CTR_STATIC_ASSERT(CTR_OFFSET_OF_ARRAY(struct DrawTiresScratch, wheelLocal, 2) == 0x78);
CTR_STATIC_ASSERT(CTR_OFFSET_OF_ARRAY(struct DrawTiresScratch, wheelLocal, 3) == 0x88);
CTR_STATIC_ASSERT(offsetof(struct DrawTiresScratch, viewNormalVectors) == 0x98);
CTR_STATIC_ASSERT(offsetof(struct DrawTiresScratch, transformedRimVectors) == 0xb8);
CTR_STATIC_ASSERT(offsetof(struct DrawTiresScratch, tireAxisA) == 0xd8);
CTR_STATIC_ASSERT(offsetof(struct DrawTiresScratch, tireAxisB) == 0xf8);
CTR_STATIC_ASSERT(offsetof(struct DrawTiresScratch, projectedSxy) == 0x118);
CTR_STATIC_ASSERT(offsetof(struct DrawTiresScratch, cornerDepthBias) == 0x128);
CTR_STATIC_ASSERT(offsetof(struct DrawTiresScratch, jumpTable) == 0x130);
CTR_STATIC_ASSERT(offsetof(struct DrawTiresScratch, instFlags) == 0x160);
CTR_STATIC_ASSERT(offsetof(struct DrawTiresScratch, depthOffsetStartBytes) == 0x16c);
CTR_STATIC_ASSERT(offsetof(struct DrawTiresScratch, depthOffsetEndBytes) == 0x16e);
CTR_STATIC_ASSERT(offsetof(struct DrawTiresScratch, otRangeStart) == 0x170);
CTR_STATIC_ASSERT(offsetof(struct DrawTiresScratch, otRangeEnd) == 0x174);
CTR_STATIC_ASSERT(sizeof(struct DrawTiresScratch) == 0x178);

#endif
