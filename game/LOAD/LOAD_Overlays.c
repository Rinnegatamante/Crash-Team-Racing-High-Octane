#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80033474-0x800334f4.
void LOAD_OvrLOD(u32 numPlyrCurrGame)
{
	// change {1-4} -> {0-3}
	u32 overlayIndex = numPlyrCurrGame - 1;

	struct GameTracker *gGT = sdata->gGT;

	// if new LOD overlay needs to load
	if ((u32)gGT->overlayIndex_LOD != overlayIndex)
	{
#ifndef CTR_NATIVE
		// LOD overlay 226-229
		sdata->load_inProgress = 1;
		LOAD_AppendQueue(sdata->ptrBigfileCdPos_2, LT_SETADDR, BI_OVERLAYSECT2 + overlayIndex, &OVR_Region2, LOAD_Callback_Overlay_Generic);
#endif

		// save ID, and reload next overlay (sector read invalidation)
		gGT->overlayIndex_LOD = overlayIndex;
		gGT->overlayIndex_Threads = OVERLAY_INDEX_NONE;
	}
	return;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800334f4-0x80033570.
void LOAD_OvrEndRace(u32 overlayIndex)
{
	struct GameTracker *gGT = sdata->gGT;

	// if new EndOfRace overlay needs to load
	if ((u32)gGT->overlayIndex_EndOfRace != overlayIndex)
	{
#ifndef CTR_NATIVE
		// EndOfRace overlay 221-225
		sdata->load_inProgress = 1;
		LOAD_AppendQueue(sdata->ptrBigfileCdPos_2, LT_SETADDR, BI_OVERLAYSECT1 + overlayIndex, &OVR_Region1, LOAD_Callback_Overlay_Generic);
#endif

		gGT->overlayIndex_EndOfRace = overlayIndex;
		gGT->overlayIndex_LOD = OVERLAY_INDEX_NONE;
	}
	return;
}

#ifdef CTR_NATIVE
static void LOAD_NativeResetThreadsOverlay(enum OverlayIndex overlayIndex)
{
	switch (overlayIndex)
	{
	case OVERLAY_INDEX_NONE:
		break;
	case OVERLAY_INDEX_MAIN_MENU:
		OVR230_InitData();
		break;
	case OVERLAY_INDEX_RACING_OR_BATTLE:
		OVR231_InitData();
		break;
	case OVERLAY_INDEX_ADV_HUB:
		OVR232_InitData();
		break;
	case OVERLAY_INDEX_PODIUMS:
		OVR233_InitData();
		break;
	}
}
#endif

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80033570-0x800335dc.
void LOAD_OvrThreads(u32 overlayIndex)
{
	struct GameTracker *gGT = sdata->gGT;

	// if new Threads overlay needs to load
	if ((u32)gGT->overlayIndex_Threads != overlayIndex)
	{
#ifndef CTR_NATIVE
		gGT->overlayIndex_Threads = OVERLAY_INDEX_NONE;
		// Threads overlay 230-233
		sdata->load_inProgress = 1;
		LOAD_AppendQueue(sdata->ptrBigfileCdPos_2, LT_SETADDR, BI_OVERLAYSECT3 + overlayIndex, &OVR_Region3, data.overlayCallbackFuncs[overlayIndex]);
#else
		// NOTE(aalhendi): Native overlays are already linked, so reset the
		// overlay-owned data that retail would refresh by streaming into OVR_Region3.
		gGT->overlayIndex_Threads = OVERLAY_INDEX_NONE;
		LOAD_NativeResetThreadsOverlay((enum OverlayIndex)overlayIndex);
		((void (*)())data.overlayCallbackFuncs[overlayIndex])();
#endif
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800335dc-0x80033610.
int LOAD_GetAdvPackIndex(void)
{
	int levelID = sdata->gGT->levelID;

	if ((levelID != GEM_STONE_VALLEY) && (levelID != GLACIER_PARK))
	{
		return 1;
	}

	return 2;
}
