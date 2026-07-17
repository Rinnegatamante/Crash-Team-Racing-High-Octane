#include <common.h>


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8004b230-0x8004b258.
void TakeCupProgress_Activate(s16 stringIndex)
{
	// string index to print
	sdata->stringIndexSaveCupProgress = stringIndex;

	// Draw the menu
	// for "Save Game", "yes / no"
	RECTMENU_Show(&data.menuSaveGame);
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8004b258-0x8004b31c.
void TakeCupProgress_MenuProc(struct RectMenu *menu)
{
	if (menu->funcState == RECTMENU_FUNC_STATE_UPDATE)
	{
		s16 stringIndex = sdata->stringIndexSaveCupProgress;
		if (stringIndex != 0)
		{
			DecalFont_DrawMultiLine(sdata->lngStrings[stringIndex], 0x100, 0x3c, 0x1cc, FONT_BIG, JUSTIFY_CENTER);
		}
		return;
	}

	if ((menu->rowSelected == -1) || (menu->rowSelected == 1))
	{
		RECTMENU_Hide(menu);
		return;
	}

	if (menu->rowSelected == 0)
	{
		sdata->boolSaveCupProgress = 1;
		SelectProfile_ToggleMode(SELECT_PROFILE_MODE_SLOT_SAVE);
		sdata->ptrDesiredMenu = &data.menuWarning2;
	}
}
