#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ae81c-0x800ae834
void CS_LoadBossCallback(struct LoadQueueSlot *lqs)
{
	void *ptr = lqs->ptrDestination;
	sdata->load_inProgress = 0;
	D233.ptrModelBossHead = ptr;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ae834-0x800ae9a8
void CS_LoadBoss(const struct BossCutsceneData *bcd)
{
	struct GameTracker *gGT = sdata->gGT;
	int otherHubMempack = LOAD_HUB_MEMPACK_PAIR_INDEX_SUM - gGT->activeMempackIndex;

	CDSYS_XAPauseRequest();

	// erase HEAD + BODY
	D233.ptrModelBossBody = 0;
	D233.ptrModelBossHead = 0;

	// invalidate alternative-hub, because
	// the boss will load in that level's RAM
	gGT->levID_in_each_mempack[otherHubMempack] = LOAD_NO_LEVEL_IN_MEMPACK;

	// Swap to pack of hub you're NOT on,
	// wipe the pack to reload the new BOSS
	MEMPACK_SwapPacks(otherHubMempack);
	MEMPACK_ClearLowMem();

	sdata->load_inProgress = 1;

	if (bcd->vrmFile_UNUSED != 0)
	{
		LOAD_AppendQueue(0, LT_VRAM, bcd->vrmFile_UNUSED - 1 + otherHubMempack, NULL, NULL);
	}

	// CTR Model File (body)
	if (bcd->bodyFile != 0)
	{
		LOAD_AppendQueue(0, LT_DRAM, bcd->bodyFile - 1 + otherHubMempack, &D233.ptrModelBossBody, LOAD_QUEUE_CALLBACK_SET_POINTER);
	}

	// CTR Model File (head)
	LOAD_AppendQueue(0, LT_DRAM, bcd->headFile - 1 + otherHubMempack, NULL, CS_LoadBossCallback);
}
