#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800348e8-0x80034908.
b32 LOAD_IsOpen_RacingOrBattle(void)
{
	return sdata->gGT->overlayIndex_Threads == OVERLAY_INDEX_RACING_OR_BATTLE;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80034908-0x80034920.
b32 LOAD_IsOpen_MainMenu(void)
{
	return sdata->gGT->overlayIndex_Threads == OVERLAY_INDEX_MAIN_MENU;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80034920-0x80034940.
b32 LOAD_IsOpen_AdvHub(void)
{
	return sdata->gGT->overlayIndex_Threads == OVERLAY_INDEX_ADV_HUB;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80034940-0x80034960.
b32 LOAD_IsOpen_Podiums(void)
{
	return sdata->gGT->overlayIndex_Threads == OVERLAY_INDEX_PODIUMS;
}
