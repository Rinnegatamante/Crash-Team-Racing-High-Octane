#include <common.h>
#include "../RenderLevel/DrawLevelOvr_shared.h"

enum Ovr228DrawLevelConstants
{
	OVR228_WATER_BSP_LIST_PRIM_RESERVE_BIAS = 0x1040,
	OVR228_SPLIT_GROUND_LIST_B_PRIM_RESERVE_BIAS = 0x16c0,
	OVR228_TERMINAL_PRIM_RESERVE_BIAS = 0x1a00,
	OVR228_SPLIT_GROUND_RENDERED_A_SETUP_INDEX = 3,
	OVR228_SPLIT_GROUND_LIST_B_SETUP_INDEX = 4,
	OVR228_SPLIT_GROUND_RENDERED_B_SETUP_INDEX = 5,
	OVR228_WIDE_DYNAMIC_SETUP_INDEX = 6,
	OVR228_QUAD_4X4_RENDERED_SETUP_INDEX = 7,
	OVR228_CANONICAL_GROUND_4X2_LIST_SETUP_INDEX = 5,
	OVR228_CANONICAL_GROUND_4X2_RENDERED_SETUP_INDEX = 6,
	OVR228_CANONICAL_DYNAMIC_RENDERED_SETUP_INDEX = 8,
	OVR228_CANONICAL_WIDE_DYNAMIC_SETUP_INDEX = 9,
	OVR228_CANONICAL_QUAD_4X4_RENDERED_SETUP_INDEX = 10,
};

static int DrawLevelOvr3P_DispatchBucketHandler(u32 handlerAddress, void *bucketValue, struct PushBuffer *pb, struct mesh_info *mesh, struct PrimMem *primMem,
                                                const int *visFaceList);

static const struct DrawLevelOvrBucketSetupRecord *DrawLevelOvr3P_FindBucketSetupRecord(u32 setupAddress, int *setupIndex)
{
	for (s32 bucketIndex = 0; bucketIndex < OVR228_BUCKET_COUNT; bucketIndex++)
	{
		u32 recordAddress = OVR228_RDATA_BUCKET_SETUP_BASE + (u32)(bucketIndex * sizeof(R228.bucketSetups[0]));

		if (recordAddress == setupAddress)
		{
			if (setupIndex != NULL)
			{
				*setupIndex = bucketIndex;
			}

			return &R228.bucketSetups[bucketIndex];
		}
	}

	return NULL;
}

static const u32 *DrawLevelOvr3P_GetBucketSetupCopySource(const struct DrawLevelOvrBucketSetupRecord *setup, int setupIndex,
                                                          const struct DrawLevelOvrBucketSetupCopy *copy)
{
	u32 recordAddress = OVR228_RDATA_BUCKET_SETUP_BASE + (u32)(setupIndex * sizeof(R228.bucketSetups[0]));
	u32 copy0Address = recordAddress + OFFSETOF(struct DrawLevelOvrBucketSetupRecord, copy0);
	u32 copy1Address = recordAddress + OFFSETOF(struct DrawLevelOvrBucketSetupRecord, copy1);

	if (copy->sourceAddress == copy0Address)
	{
		return setup->copy0;
	}

	if (copy->sourceAddress == copy1Address)
	{
		return setup->copy1;
	}

	return NULL;
}

static u32 DrawLevelOvr3P_TranslateCopiedWord(int setupIndex, const struct DrawLevelOvrBucketSetupCopy *copy, u32 wordIndex, u32 value)
{
	int canonicalSetupIndex = -1;

	if (setupIndex == OVR228_SPLIT_GROUND_RENDERED_A_SETUP_INDEX)
	{
		canonicalSetupIndex = OVR228_CANONICAL_DYNAMIC_RENDERED_SETUP_INDEX;
	}
	else if (setupIndex == OVR228_SPLIT_GROUND_LIST_B_SETUP_INDEX)
	{
		canonicalSetupIndex = OVR228_CANONICAL_GROUND_4X2_LIST_SETUP_INDEX;
	}
	else if (setupIndex == OVR228_SPLIT_GROUND_RENDERED_B_SETUP_INDEX)
	{
		canonicalSetupIndex = OVR228_CANONICAL_GROUND_4X2_RENDERED_SETUP_INDEX;
	}
	else if (setupIndex == OVR228_WIDE_DYNAMIC_SETUP_INDEX)
	{
		canonicalSetupIndex = OVR228_CANONICAL_WIDE_DYNAMIC_SETUP_INDEX;
	}
	else if (setupIndex == OVR228_QUAD_4X4_RENDERED_SETUP_INDEX)
	{
		canonicalSetupIndex = OVR228_CANONICAL_QUAD_4X4_RENDERED_SETUP_INDEX;
	}
	else
	{
		return value;
	}

	// NOTE(aalhendi): Some 228 helper/direct labels overlap 229 virtual labels.
	// Keep R228 exact, then alias copied scratch setup words to canonical 226
	// helper tables only after target objdump proof.
	if ((copy->scratchOffset == DRAW_LEVEL_OVR_COPIED_SETUP0_SCRATCH_OFFSET) && (wordIndex < DRAW_LEVEL_OVR_COPIED_SETUP0_WORD_COUNT))
	{
		return R226.bucketSetups[canonicalSetupIndex].copy0[wordIndex];
	}

	if ((copy->scratchOffset == DRAW_LEVEL_OVR_COPIED_SETUP1_SCRATCH_OFFSET) && (wordIndex < DRAW_LEVEL_OVR_COPIED_SETUP1_WORD_COUNT))
	{
		return R226.bucketSetups[canonicalSetupIndex].copy1[wordIndex];
	}

	return value;
}

static u32 DrawLevelOvr3P_TranslateClipRecordLabel(u32 address)
{
	for (s32 jumpWordIndex = 0; jumpWordIndex < OVR228_CLIP_RECORD_JUMP_WORD_COUNT; jumpWordIndex++)
	{
		if (R228.clipRecordJumpTable[jumpWordIndex] == address)
		{
			return R226.clipRecordJumpTable[jumpWordIndex];
		}
	}

	return address;
}

static void DrawLevelOvr3P_CopyScratchWords(const struct DrawLevelOvrBucketSetupRecord *setup, int setupIndex, const struct DrawLevelOvrBucketSetupCopy *copy)
{
	const u32 *source = DrawLevelOvr3P_GetBucketSetupCopySource(setup, setupIndex, copy);
	u32 *scratch = CTR_SCRATCHPAD_PTR(u32, copy->scratchOffset);

	if (source == NULL)
	{
		return;
	}

	for (u32 scratchWordIndex = 0; scratchWordIndex <= copy->lastWordIndex; scratchWordIndex++)
	{
		scratch[scratchWordIndex] = DrawLevelOvr3P_TranslateCopiedWord(setupIndex, copy, scratchWordIndex, source[scratchWordIndex]);
	}
}

static void DrawLevelOvr3P_ApplyBucketSetup(u32 setupAddress, u32 handlerAddress)
{
	int setupIndex = -1;
	const struct DrawLevelOvrBucketSetupRecord *setup = DrawLevelOvr3P_FindBucketSetupRecord(setupAddress, &setupIndex);

	if (setup != NULL)
	{
		for (s32 copyIndex = 0; copyIndex < 2; copyIndex++)
		{
			const struct DrawLevelOvrBucketSetupCopy *copy = &setup->copies[copyIndex];

			if (copy->lastWordIndex == 0)
			{
				break;
			}

			DrawLevelOvr3P_CopyScratchWords(setup, setupIndex, copy);
		}
	}

	DrawLevelOvr1P_Scratch()->currentHandlerAddress = handlerAddress;
}

static void DrawLevelOvr3P_CopyScratchInitTable(void)
{
	u32 *scratch = CTR_SCRATCHPAD_PTR(u32, DRAW_LEVEL_OVR1P_SCRATCH_INIT_TABLE_OFFSET);

	for (s32 scratchWordIndex = 0; scratchWordIndex < OVR228_SCRATCH_INIT_WORD_COUNT; scratchWordIndex++)
	{
		scratch[scratchWordIndex] = R228.scratchInitTable[scratchWordIndex];
	}
}

static void DrawLevelOvr3P_CopyClipRecordJumpTable(void)
{
	u32 *clipRecordJumpTable = CTR_SCRATCHPAD_PTR(u32, DRAW_LEVEL_OVR1P_GT3_CLIP_RECORD_JUMP_TABLE_OFFSET);

	for (s32 jumpWordIndex = 0; jumpWordIndex < OVR228_CLIP_RECORD_JUMP_WORD_COUNT; jumpWordIndex++)
	{
		clipRecordJumpTable[jumpWordIndex] = DrawLevelOvr3P_TranslateClipRecordLabel(R228.clipRecordJumpTable[jumpWordIndex]);
	}
}

static int DrawLevelOvr3P_DrawViewportBucket(struct DrawLevelOvr1PRenderList *renderList, s32 renderListOffset, struct PushBuffer *pb, struct mesh_info *mesh,
                                             struct PrimMem *primMem, const int *visFaceList, u8 **clipCursor, int playerIndex, int applySetup,
                                             int *didDispatch)
{
	u32 bucketIndex = (u32)renderListOffset / sizeof(u32);
	const struct DrawLevelOvr1PBucket *bucket = &sDrawLevelOvr1PBuckets[bucketIndex];
	void *bucketValue = DrawLevelOvr1P_GetRenderListBucketValue(renderList, bucket);
	u32 setupAddress = R228.bucketSetupAddresses[bucketIndex];
	u32 handlerAddress = R228.bucketHandlerAddresses[bucketIndex];
	struct QuadBlock **renderedOverflowBase = (struct QuadBlock **)data.ptrRenderedQuadblockDestination_forEachPlayer[playerIndex];

	*didDispatch = 0;

	if (bucketValue == NULL)
	{
		DrawLevelOvr_ClearRenderedOverflowBase(playerIndex);
		return 1;
	}

	if (applySetup)
	{
		DrawLevelOvr3P_ApplyBucketSetup(setupAddress, handlerAddress);
	}

	DrawLevelOvr1P_SetViewportScratchContext(pb, visFaceList, data.PtrClipBuffer[playerIndex], *clipCursor, renderedOverflowBase);
	if (!DrawLevelOvr3P_DispatchBucketHandler(handlerAddress, bucketValue, pb, mesh, primMem, visFaceList))
	{
		return 0;
	}

	*clipCursor = DrawLevelOvr1P_GetClipRecordCursor();
	*didDispatch = 1;
	return 1;
}

static int DrawLevelOvr3P_DispatchBucketTable(struct DrawLevelOvr1PRenderList *renderLists, struct PushBuffer *pushBuffers, struct mesh_info *mesh,
                                              struct PrimMem *primMem, const int *visFaceList0, const int *visFaceList1, const int *visFaceList2,
                                              u8 **clipCursors)
{
	for (s32 renderListOffset = DRAW_LEVEL_OVR1P_RENDER_LIST_OFFSET_4X1_LIST; renderListOffset >= 0; renderListOffset -= (s32)sizeof(u32))
	{
		int setupApplied = 0;
		int didDispatch = 0;

		DrawLevelOvr1P_Scratch()->currentBucketOffset = (u32)renderListOffset;

		if (!DrawLevelOvr3P_DrawViewportBucket(&renderLists[0], renderListOffset, &pushBuffers[0], mesh, primMem, visFaceList0, &clipCursors[0], 0, 1,
		                                       &didDispatch))
		{
			return 0;
		}
		setupApplied |= didDispatch;

		if (!DrawLevelOvr3P_DrawViewportBucket(&renderLists[1], renderListOffset, &pushBuffers[1], mesh, primMem, visFaceList1, &clipCursors[1], 1,
		                                       !setupApplied, &didDispatch))
		{
			return 0;
		}
		setupApplied |= didDispatch;

		if (!DrawLevelOvr3P_DrawViewportBucket(&renderLists[2], renderListOffset, &pushBuffers[2], mesh, primMem, visFaceList2, &clipCursors[2], 2,
		                                       !setupApplied, &didDispatch))
		{
			return 0;
		}
	}

	return 1;
}

static int DrawLevelOvr3P_DispatchBucketHandler(u32 handlerAddress, void *bucketValue, struct PushBuffer *pb, struct mesh_info *mesh, struct PrimMem *primMem,
                                                const int *visFaceList)
{
	if (handlerAddress == OVR228_RETAIL_LABEL_WATER_BSP_LIST_HANDLER)
	{
		DrawLevelOvr1P_SetPrimReserveBias(OVR228_WATER_BSP_LIST_PRIM_RESERVE_BIAS);
		return Ovr226_800a1e30_DrawWaterBspList((struct VisMemBspListNode *)bucketValue, pb, mesh, primMem, visFaceList);
	}

	if (handlerAddress == OVR228_RETAIL_LABEL_WATER_RENDERED_HANDLER)
	{
		DrawLevelOvr1P_SetPrimReserveBias(OVR228_WATER_BSP_LIST_PRIM_RESERVE_BIAS);
		return Ovr226_800a2904_DrawWaterRenderedListWithDefaultHandler((struct QuadBlock **)bucketValue, pb, mesh, primMem,
		                                                               OVR228_RETAIL_LABEL_WATER_RENDERED_DEFAULT_WRAPPER);
	}

	if (handlerAddress == OVR228_RETAIL_LABEL_SPLIT_GROUND_LIST_A_HANDLER)
	{
		DrawLevelOvr1P_SetPrimReserveBias(OVR228_WATER_BSP_LIST_PRIM_RESERVE_BIAS);
		return DrawLevelOvr1P_DrawSplitGroundListABspList((struct VisMemBspListNode *)bucketValue, pb, mesh, primMem, visFaceList);
	}

	if (handlerAddress == OVR228_RETAIL_LABEL_SPLIT_GROUND_RENDERED_A_HANDLER)
	{
		DrawLevelOvr1P_SetPrimReserveBias(OVR228_WATER_BSP_LIST_PRIM_RESERVE_BIAS);
		DrawLevelOvr1P_SetSplitGroundThresholdScratch();
		return DrawLevelOvr1P_DrawRenderedQuadBlocks((struct QuadBlock **)bucketValue, pb, mesh, primMem, DRAW_LEVEL_OVR1P_BUCKET_DYNAMIC_RENDERED);
	}

	if (handlerAddress == OVR228_RETAIL_LABEL_SPLIT_GROUND_LIST_B_HANDLER)
	{
		int result;

		DrawLevelOvr1P_SetPrimReserveBias(OVR228_SPLIT_GROUND_LIST_B_PRIM_RESERVE_BIAS);
		DrawLevelOvr1P_SetSplitGroundThresholdScratch();
		DrawLevelOvr1P_SetMosaicReloadSpanOverride(DRAW_LEVEL_OVR1P_SPLIT_GROUND_MOSAIC_RELOAD_SPAN);
		result =
		    DrawLevelOvr1P_DrawBspListQuadBlocks((struct VisMemBspListNode *)bucketValue, pb, mesh, primMem, visFaceList, DRAW_LEVEL_OVR1P_BUCKET_4X2_LIST);
		DrawLevelOvr1P_SetMosaicReloadSpanOverride(0);
		return result;
	}

	if (handlerAddress == OVR228_RETAIL_LABEL_SPLIT_GROUND_RENDERED_B_HANDLER)
	{
		int result;

		DrawLevelOvr1P_SetPrimReserveBias(OVR228_SPLIT_GROUND_LIST_B_PRIM_RESERVE_BIAS);
		DrawLevelOvr1P_SetSplitGroundThresholdScratch();
		DrawLevelOvr1P_SetMosaicReloadSpanOverride(DRAW_LEVEL_OVR1P_SPLIT_GROUND_MOSAIC_RELOAD_SPAN);
		result = DrawLevelOvr1P_DrawRenderedQuadBlocks((struct QuadBlock **)bucketValue, pb, mesh, primMem, DRAW_LEVEL_OVR1P_BUCKET_4X2_RENDERED);
		DrawLevelOvr1P_SetMosaicReloadSpanOverride(0);
		return result;
	}

	if (handlerAddress == OVR228_RETAIL_LABEL_WIDE_DYNAMIC_HANDLER)
	{
		DrawLevelOvr1P_SetPrimReserveBias(OVR228_WATER_BSP_LIST_PRIM_RESERVE_BIAS);
		DrawLevelOvr1P_SetSplitGroundThresholdScratch();
		return DrawLevelOvr1P_DrawBspListQuadBlocks((struct VisMemBspListNode *)bucketValue, pb, mesh, primMem, visFaceList, DRAW_LEVEL_OVR1P_BUCKET_4X4_LIST);
	}

	if (handlerAddress == OVR228_RETAIL_LABEL_QUAD_4X4_RENDERED_HANDLER)
	{
		DrawLevelOvr1P_SetPrimReserveBias(OVR228_WATER_BSP_LIST_PRIM_RESERVE_BIAS);
		DrawLevelOvr1P_SetSplitGroundThresholdScratch();
		return DrawLevelOvr1P_DrawRenderedQuadBlocks((struct QuadBlock **)bucketValue, pb, mesh, primMem, DRAW_LEVEL_OVR1P_BUCKET_4X4_RENDERED);
	}

	// NOTE(aalhendi): Reject handler addresses that are not present in the
	// recovered R228 bucket table.
	return 0;
}

static int DrawLevelOvr3P_ConsumeClipRecords(struct PushBuffer *pb, struct PrimMem *primMem, u8 *clipCursor, int playerIndex)
{
	(void)clipCursor;
	(void)playerIndex;

	DrawLevelOvr1P_SetPrimReserveBias(OVR228_TERMINAL_PRIM_RESERVE_BIAS);
	return DrawLevelOvr1P_ConsumeClipRecords(pb, primMem);
}

void DrawLevelOvr3P(void *LevRenderList, struct PushBuffer *pb, struct BSP *bspList, struct PrimMem *primMem, const int *visFaceList0, const int *visFaceList1,
                    const int *visFaceList2, const struct TextureLayout *waterEnvMap)
{
	struct DrawLevelOvr1PRenderList *renderLists = LevRenderList;
	struct mesh_info *mesh = (struct mesh_info *)bspList;
	u8 *clipCursors[3];
	u32 hostStackAnchor;

	// NOTE(aalhendi): ASM-audited against NTSC-U 926 228 entry/setup
	// 0x800a0cbc-0x800a10c4. Runtime proof is tracked separately from
	// source ownership and public route promotion.
	DrawLevelOvr1P_Scratch()->savedStackPtr32 = (u32)(uintptr_t)&hostStackAnchor;
	DrawLevelOvr1P_Scratch()->visFaceListArgPtr32[0] = (u32)(uintptr_t)visFaceList0;
	if (visFaceList0 == NULL)
	{
		return;
	}

	DrawLevelOvr1P_Scratch()->visFaceListArgPtr32[1] = (u32)(uintptr_t)visFaceList1;
	if (visFaceList1 == NULL)
	{
		return;
	}

	DrawLevelOvr1P_Scratch()->visFaceListArgPtr32[2] = (u32)(uintptr_t)visFaceList2;
	if (visFaceList2 == NULL)
	{
		return;
	}

	DrawLevelOvr1P_Scratch()->waterEnvMapPtr32 = (u32)(uintptr_t)waterEnvMap;
	DrawLevelOvr1P_Scratch()->primMemEndPtr32 = (u32)(uintptr_t)primMem->end;

	if (mesh->ptrQuadBlockArray == NULL)
	{
		return;
	}

	clipCursors[0] = data.PtrClipBuffer[0];
	clipCursors[1] = data.PtrClipBuffer[1];
	clipCursors[2] = data.PtrClipBuffer[2];

	DrawLevelOvr1P_Scratch()->pushBufferPtr32[0] = (u32)(uintptr_t)&pb[0];
	DrawLevelOvr1P_Scratch()->pushBufferPtr32[1] = (u32)(uintptr_t)&pb[1];
	DrawLevelOvr1P_Scratch()->pushBufferPtr32[2] = (u32)(uintptr_t)&pb[2];
	DrawLevelOvr1P_Scratch()->playerClipCursorPtr32[0] = (u32)(uintptr_t)clipCursors[0];
	DrawLevelOvr1P_Scratch()->playerClipCursorPtr32[1] = (u32)(uintptr_t)clipCursors[1];
	DrawLevelOvr1P_Scratch()->playerClipCursorPtr32[2] = (u32)(uintptr_t)clipCursors[2];

	DrawLevelOvr1P_SetPrimReserveBias(0);
	DrawLevelOvr1P_SetListHandlersSeedRenderedCursor(0);
	Ovr226_800a0dc4_ClearProjectedScratch();
	DrawLevelOvr3P_CopyScratchInitTable();
	DrawLevelOvr1P_Scratch()->renderListPtr32 = (u32)(uintptr_t)LevRenderList;

	if (!DrawLevelOvr3P_DispatchBucketTable(renderLists, pb, mesh, primMem, visFaceList0, visFaceList1, visFaceList2, clipCursors))
	{
		return;
	}

	DrawLevelOvr1P_Scratch()->playerClipCursorPtr32[0] = (u32)(uintptr_t)clipCursors[0];
	DrawLevelOvr1P_Scratch()->playerClipCursorPtr32[1] = (u32)(uintptr_t)clipCursors[1];
	DrawLevelOvr1P_Scratch()->playerClipCursorPtr32[2] = (u32)(uintptr_t)clipCursors[2];

	DrawLevelOvr3P_CopyClipRecordJumpTable();
	if (!DrawLevelOvr_ConsumeClipRecordsForViewport(&pb[0], primMem, clipCursors[0], 0, DrawLevelOvr3P_ConsumeClipRecords))
	{
		return;
	}

	if (!DrawLevelOvr_ConsumeClipRecordsForViewport(&pb[1], primMem, clipCursors[1], 1, DrawLevelOvr3P_ConsumeClipRecords))
	{
		return;
	}

	if (!DrawLevelOvr_ConsumeClipRecordsForViewport(&pb[2], primMem, clipCursors[2], 2, DrawLevelOvr3P_ConsumeClipRecords))
	{
		return;
	}
}
