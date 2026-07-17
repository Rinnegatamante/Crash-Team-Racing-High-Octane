#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800347d0-0x80034874.
// Dont get confused, packID is LOAD_GetAdvPackIndex(),
// which gives the pack of the hub you're NOT on, because the
// game does 3-GetAdvPackIndex to load the hub you ARE on
void LOAD_TalkingMask(int packID, int maskID)
{
	sdata->modelMaskHints3D = 0;

	// invalidate alternative-hub, because
	// the mask will load in that level's RAM
	sdata->gGT->levID_in_each_mempack[packID] = -1;

	// Swap to pack of hub you're NOT on,
	// wipe the pack to reload the new MASK
	MEMPACK_SwapPacks(packID);
	MEMPACK_ClearLowMem();

	sdata->load_inProgress = 1;

	int offset = maskID * 4 + (packID - 1) * 2;

	// NOTE(aalhendi): Retail queues legacy VRAM type 3 with no final callback.
	LOAD_AppendQueue(sdata->ptrBigfileCdPos_2, LT_VRAM, BI_UKAHEAD + offset, NULL, NULL);

	LOAD_AppendQueue(sdata->ptrBigfileCdPos_2, LT_GETADDR, BI_UKAHEAD + offset + 1, NULL, LOAD_Callback_MaskHints3D);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80034874-0x800348e8.
void LOAD_LevelFile(int levelID)
{
	struct GameTracker *gGT = sdata->gGT;

	// why here?
	sdata->modelMaskHints3D = 0;

	gGT->hudFlags &= HUD_FLAG_CLEAR_RACE_HUD_MASK;

	gGT->prevLEV = gGT->levelID;
	gGT->levelID = levelID;

	// disable all rendering except checkeredFlag
	gGT->renderFlags &= RENDER_FLAG_CHECKERED_FLAG;

	if (RaceFlag_IsFullyOffScreen())
	{
		RaceFlag_BeginTransition(1);
	}

	// start loading
	sdata->Loading.stage = LOAD_TEN_STAGES_0;
}
