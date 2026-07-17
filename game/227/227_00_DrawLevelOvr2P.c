#include <common.h>
#include "../RenderLevel/DrawLevelOvr_shared.h"

enum Ovr227DrawLevelConstants
{
	OVR227_PRIM_RESERVE_BIAS = 0xd00,
	OVR227_DEPTH_SCALE = 0x19c0,
	OVR227_TEXTURE_LOD_DEPTH_THRESHOLD0 = 0x1000,
	OVR227_TEXTURE_LOD_DEPTH_THRESHOLD1 = 0x800,
	OVR227_TOP_LEVEL_NEAR_DEPTH_THRESHOLD = 0x600,
	OVR227_RECURSIVE_NEAR_DEPTH_THRESHOLD = 0x300,
};

static void DrawLevelOvr2P_CopyClipRecordJumpTable(void);

CTR_STATIC_ASSERT(sizeof(struct OverlayRDATA_227) == 0x564);
CTR_STATIC_ASSERT(sizeof(struct DrawLevelOvrBucketSetupRecord) == 0x64);
CTR_STATIC_ASSERT(sizeof(((struct OverlayRDATA_227 *)0)->scratchInitTable) == 0x60);
CTR_STATIC_ASSERT(sizeof(((struct OverlayRDATA_227 *)0)->clipRecordJumpTable) == 0x60);

static u32 DrawLevelOvr2P_TranslateCopiedLabel(u32 address)
{
	for (s32 bucketIndex = 0; bucketIndex < OVR227_BUCKET_COUNT; bucketIndex++)
	{
		for (s32 copyWordIndex = 0; copyWordIndex < OVR227_SETUP_COPY0_WORD_COUNT; copyWordIndex++)
		{
			if (R227.bucketSetups[bucketIndex].copy0[copyWordIndex] == address)
			{
				return R226.bucketSetups[bucketIndex].copy0[copyWordIndex];
			}
		}

		for (s32 copyWordIndex = 0; copyWordIndex < OVR227_SETUP_COPY1_WORD_COUNT; copyWordIndex++)
		{
			if (R227.bucketSetups[bucketIndex].copy1[copyWordIndex] == address)
			{
				return R226.bucketSetups[bucketIndex].copy1[copyWordIndex];
			}
		}
	}

	for (s32 jumpWordIndex = 0; jumpWordIndex < OVR227_CLIP_RECORD_JUMP_WORD_COUNT; jumpWordIndex++)
	{
		if (R227.clipRecordJumpTable[jumpWordIndex] == address)
		{
			return R226.clipRecordJumpTable[jumpWordIndex];
		}
	}

	return address;
}

static void DrawLevelOvr2P_CopyScratchWordsTranslated(const u32 *source, const struct DrawLevelOvrBucketSetupCopy *copy)
{
	u32 *scratch = CTR_SCRATCHPAD_PTR(u32, copy->scratchOffset);

	for (u32 scratchWordIndex = 0; scratchWordIndex <= copy->lastWordIndex; scratchWordIndex++)
	{
		scratch[scratchWordIndex] = DrawLevelOvr2P_TranslateCopiedLabel(source[scratchWordIndex]);
	}
}

static const struct DrawLevelOvrBucketSetupRecord *DrawLevelOvr2P_FindBucketSetupRecord(u32 setupAddress)
{
	for (s32 bucketIndex = 0; bucketIndex < OVR227_BUCKET_COUNT; bucketIndex++)
	{
		u32 recordAddress = OVR227_RDATA_BUCKET_SETUP_BASE + (u32)(bucketIndex * sizeof(R227.bucketSetups[0]));

		if (recordAddress == setupAddress)
		{
			return &R227.bucketSetups[bucketIndex];
		}
	}

	return NULL;
}

static void DrawLevelOvr2P_ApplyBucketSetup(u32 setupAddress, u32 handlerAddress)
{
	const struct DrawLevelOvrBucketSetupRecord *setup = DrawLevelOvr2P_FindBucketSetupRecord(setupAddress);

	if (setup == NULL)
	{
		DrawLevelOvr1P_Scratch()->currentHandlerAddress = handlerAddress;
		return;
	}

	DrawLevelOvr2P_CopyScratchWordsTranslated(setup->copy0, &setup->copies[0]);
	DrawLevelOvr2P_CopyScratchWordsTranslated(setup->copy1, &setup->copies[1]);
	DrawLevelOvr1P_Scratch()->currentHandlerAddress = handlerAddress;
}

static void DrawLevelOvr2P_CopyScratchInitTable(void)
{
	u32 *scratch = CTR_SCRATCHPAD_PTR(u32, DRAW_LEVEL_OVR1P_SCRATCH_INIT_TABLE_OFFSET);

	for (s32 scratchWordIndex = 0; scratchWordIndex < OVR227_SCRATCH_INIT_WORD_COUNT; scratchWordIndex++)
	{
		scratch[scratchWordIndex] = R227.scratchInitTable[scratchWordIndex];
	}
}

static void DrawLevelOvr2P_SeedSharedHelperThresholdScratch(void)
{
	// NOTE(aalhendi): Retail 227 bakes these thresholds as immediates in
	// copied BSP handler bodies. Native reuses 226 C helpers that still read
	// the equivalent thresholds from scratch, so seed the shared-helper view.
	DrawLevelOvr1P_RenderScratch()->depthScale = OVR227_DEPTH_SCALE;
	DrawLevelOvr1P_RenderScratch()->textureLodDepthThreshold0 = OVR227_TEXTURE_LOD_DEPTH_THRESHOLD0;
	DrawLevelOvr1P_RenderScratch()->textureLodDepthThreshold1 = OVR227_TEXTURE_LOD_DEPTH_THRESHOLD1;
	DrawLevelOvr1P_RenderScratch()->topLevelNearDepthThreshold = OVR227_TOP_LEVEL_NEAR_DEPTH_THRESHOLD;
	DrawLevelOvr1P_RenderScratch()->recursiveNearDepthThreshold = OVR227_RECURSIVE_NEAR_DEPTH_THRESHOLD;
}

static const struct DrawLevelOvr1PBucket *DrawLevelOvr2P_FindBucketByHandler(u32 handlerAddress)
{
	for (s32 bucketIndex = 0; bucketIndex < OVR227_BUCKET_COUNT; bucketIndex++)
	{
		if (R227.bucketHandlerAddresses[bucketIndex] == handlerAddress)
		{
			return &sDrawLevelOvr1PBuckets[bucketIndex];
		}
	}

	return NULL;
}

static int DrawLevelOvr2P_DispatchBucketHandler(u32 handlerAddress, void *bucketValue, struct PushBuffer *pb, struct mesh_info *mesh, struct PrimMem *primMem,
                                                const int *visFaceList)
{
	const struct DrawLevelOvr1PBucket *bucket = DrawLevelOvr2P_FindBucketByHandler(handlerAddress);

	if (bucket == NULL)
	{
		return 0;
	}

	if (bucket->kind == DRAW_LEVEL_OVR1P_BUCKET_QUADBLOCKS_RENDERED)
	{
		return DrawLevelOvr1P_DrawRenderedQuadBlocks((struct QuadBlock **)bucketValue, pb, mesh, primMem, bucket->role);
	}

	if (bucket->role == DRAW_LEVEL_OVR1P_BUCKET_FULL_DYNAMIC_LIST)
	{
		return Ovr226_800a0ef4_DrawFullDynamicBspList((struct VisMemBspListNode *)bucketValue, pb, mesh, primMem, visFaceList);
	}

	if (bucket->role == DRAW_LEVEL_OVR1P_BUCKET_WATER_LIST)
	{
		return Ovr226_800a1e30_DrawWaterBspList((struct VisMemBspListNode *)bucketValue, pb, mesh, primMem, visFaceList);
	}

	switch (bucket->role)
	{
	case DRAW_LEVEL_OVR1P_BUCKET_4X1_LIST:
	case DRAW_LEVEL_OVR1P_BUCKET_4X2_LIST:
	case DRAW_LEVEL_OVR1P_BUCKET_DYNAMIC_LIST:
	case DRAW_LEVEL_OVR1P_BUCKET_4X4_LIST:
		return DrawLevelOvr1P_DrawBspListQuadBlocks((struct VisMemBspListNode *)bucketValue, pb, mesh, primMem, visFaceList, bucket->role);

	default:
		return 0;
	}
}

static int DrawLevelOvr2P_DrawViewportBucket(struct DrawLevelOvr1PRenderList *renderList, s32 renderListOffset, struct PushBuffer *pb, struct mesh_info *mesh,
                                             struct PrimMem *primMem, const int *visFaceList, u8 **clipCursor, int playerIndex, int applySetup)
{
	u32 bucketIndex = (u32)renderListOffset / sizeof(u32);
	const struct DrawLevelOvr1PBucket *bucket = &sDrawLevelOvr1PBuckets[bucketIndex];
	void *bucketValue = DrawLevelOvr1P_GetRenderListBucketValue(renderList, bucket);
	u32 setupAddress = R227.bucketSetupAddresses[bucketIndex];
	u32 handlerAddress = R227.bucketHandlerAddresses[bucketIndex];
	struct QuadBlock **renderedOverflowBase = (struct QuadBlock **)data.ptrRenderedQuadblockDestination_forEachPlayer[playerIndex];

	if (bucketValue == NULL)
	{
		DrawLevelOvr_ClearRenderedOverflowBase(playerIndex);
		return 1;
	}

	if (applySetup)
	{
		DrawLevelOvr2P_ApplyBucketSetup(setupAddress, handlerAddress);
	}

	DrawLevelOvr1P_SetViewportScratchContext(pb, visFaceList, data.PtrClipBuffer[playerIndex], *clipCursor, renderedOverflowBase);
	if (!DrawLevelOvr2P_DispatchBucketHandler(handlerAddress, bucketValue, pb, mesh, primMem, visFaceList))
	{
		return 0;
	}

	*clipCursor = DrawLevelOvr1P_GetClipRecordCursor();
	return 1;
}

static int DrawLevelOvr2P_DispatchBucketTable(struct DrawLevelOvr1PRenderList *renderLists, struct PushBuffer *pushBuffers, struct mesh_info *mesh,
                                              struct PrimMem *primMem, const int *visFaceList0, const int *visFaceList1, u8 **clipCursors)
{
	for (s32 renderListOffset = DRAW_LEVEL_OVR1P_RENDER_LIST_OFFSET_FULL_DYNAMIC_LIST; renderListOffset >= 0; renderListOffset -= (s32)sizeof(u32))
	{
		u32 bucketIndex = (u32)renderListOffset / sizeof(u32);
		const struct DrawLevelOvr1PBucket *bucket = &sDrawLevelOvr1PBuckets[bucketIndex];
		void *viewport0BucketValue = DrawLevelOvr1P_GetRenderListBucketValue(&renderLists[0], bucket);

		DrawLevelOvr1P_Scratch()->currentBucketOffset = (u32)renderListOffset;

		if (!DrawLevelOvr2P_DrawViewportBucket(&renderLists[0], renderListOffset, &pushBuffers[0], mesh, primMem, visFaceList0, &clipCursors[0], 0, 1))
		{
			return 0;
		}

		if (!DrawLevelOvr2P_DrawViewportBucket(&renderLists[1], renderListOffset, &pushBuffers[1], mesh, primMem, visFaceList1, &clipCursors[1], 1,
		                                       viewport0BucketValue == NULL))
		{
			return 0;
		}
	}

	return 1;
}

static int DrawLevelOvr2P_ConsumeClipRecordsForViewport(struct PushBuffer *pb, struct PrimMem *primMem, u8 *clipCursor, int playerIndex)
{
	u8 *start = data.PtrClipBuffer[playerIndex];

	DrawLevelOvr1P_SetClipRecordStart(start);
	DrawLevelOvr1P_SetClipRecordCursor(clipCursor);
	DrawLevelOvr2P_CopyClipRecordJumpTable();
	return DrawLevelOvr1P_ConsumeClipRecords(pb, primMem);
}

void DrawLevelOvr2P(void *LevRenderList, struct PushBuffer *pb, struct BSP *bspList, struct PrimMem *primMem, const int *visFaceList0, const int *visFaceList1,
                    const struct TextureLayout *waterEnvMap)
{
	struct DrawLevelOvr1PRenderList *renderLists = LevRenderList;
	struct mesh_info *mesh = (struct mesh_info *)bspList;
	u8 *clipCursors[2] = {data.PtrClipBuffer[0], data.PtrClipBuffer[1]};
	u32 hostStackAnchor;

	// NOTE(aalhendi): ASM-audited against NTSC-U 926 227 entry/setup
	// 0x800a0cbc-0x800a1010. Native keeps explicit host pointers while
	// preserving the retail scratch ownership and two-viewport ordering.
	DrawLevelOvr1P_Scratch()->savedStackPtr32 = (u32)(uintptr_t)&hostStackAnchor;
	DrawLevelOvr1P_Scratch()->primMemEndPtr32 = (u32)(uintptr_t)primMem->end;
	DrawLevelOvr1P_Scratch()->visFaceListArgPtr32[0] = (u32)(uintptr_t)visFaceList0;
	DrawLevelOvr1P_Scratch()->visFaceListArgPtr32[1] = (u32)(uintptr_t)visFaceList1;

	if ((visFaceList0 == NULL) || (visFaceList1 == NULL))
	{
		return;
	}

	DrawLevelOvr1P_Scratch()->waterEnvMapPtr32 = (u32)(uintptr_t)waterEnvMap;
	DrawLevelOvr1P_Scratch()->pushBufferPtr32[0] = (u32)(uintptr_t)&pb[0];
	DrawLevelOvr1P_Scratch()->pushBufferPtr32[1] = (u32)(uintptr_t)&pb[1];
	DrawLevelOvr1P_Scratch()->playerClipCursorPtr32[0] = (u32)(uintptr_t)clipCursors[0];
	DrawLevelOvr1P_Scratch()->playerClipCursorPtr32[1] = (u32)(uintptr_t)clipCursors[1];

	if (mesh->ptrQuadBlockArray == NULL)
	{
		return;
	}

	DrawLevelOvr1P_SetPrimReserveBias(OVR227_PRIM_RESERVE_BIAS);
	DrawLevelOvr1P_SetListHandlersSeedRenderedCursor(0);
	Ovr226_800a0dc4_ClearProjectedScratch();
	DrawLevelOvr2P_CopyScratchInitTable();
	DrawLevelOvr2P_SeedSharedHelperThresholdScratch();

	if (!DrawLevelOvr2P_DispatchBucketTable(renderLists, pb, mesh, primMem, visFaceList0, visFaceList1, clipCursors))
	{
		return;
	}

	DrawLevelOvr1P_Scratch()->playerClipCursorPtr32[0] = (u32)(uintptr_t)clipCursors[0];
	DrawLevelOvr1P_Scratch()->playerClipCursorPtr32[1] = (u32)(uintptr_t)clipCursors[1];

	if (!DrawLevelOvr2P_ConsumeClipRecordsForViewport(&pb[0], primMem, clipCursors[0], 0))
	{
		return;
	}

	if (!DrawLevelOvr2P_ConsumeClipRecordsForViewport(&pb[1], primMem, clipCursors[1], 1))
	{
		return;
	}
}

static void DrawLevelOvr2P_CopyClipRecordJumpTable(void)
{
	u32 *clipRecordJumpTable = CTR_SCRATCHPAD_PTR(u32, DRAW_LEVEL_OVR1P_GT3_CLIP_RECORD_JUMP_TABLE_OFFSET);

	for (s32 jumpWordIndex = 0; jumpWordIndex < OVR227_CLIP_RECORD_JUMP_WORD_COUNT; jumpWordIndex++)
	{
		clipRecordJumpTable[jumpWordIndex] = DrawLevelOvr2P_TranslateCopiedLabel(R227.clipRecordJumpTable[jumpWordIndex]);
	}
}
