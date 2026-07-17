#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b344c-0x800b351c.
void AH_HintMenu_FiveArrows(int posY, s16 rotation)
{
	u32 *ptrColor = &D232.fiveArrow_col1[0];
	if ((sdata->frameCounter & 2) != 0)
	{
		ptrColor = &D232.fiveArrow_col2[0];
	}

	for (int i = 0; i < AH_HINTMENU_ARROW_COUNT; i++)
	{
		AH_Map_HubArrow(
		    // posX
		    (i * AH_HINTMENU_ARROW_SPACING + AH_HINTMENU_ARROW_START_X),

		    // posY
		    (posY + AH_HINTMENU_ARROW_Y_OFFSET),

		    &D232.fiveArrowPos[0],

		    (char *)ptrColor, 0x800, (int)rotation);
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b351c-0x800b3594.
void AH_HintMenu_MaskPosRot(void)
{
	struct Instance *mask = sdata->instMaskHints3D;

	ConvertRotToMatrix(&mask->matrix, &D232.maskRot);

	// Set position
	mask->matrix.t[0] = D232.maskPos.x;
	mask->matrix.t[1] = D232.maskPos.y;
	mask->matrix.t[2] = D232.maskPos.z;

	// Apply the default mask model scale from D232.
	((struct MaskHint *)mask->thread->object)->scale = D232.maskScale;

	return;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 overlay 232 0x800b3594-0x800b3dd8.
void AH_HintMenu_MenuProc(struct RectMenu *menu)
{
	struct GameTracker *gGT = sdata->gGT;
	b32 shouldClose = false;
	RECT box;
	char hintsFound[AH_HINTMENU_HINT_STRING_COUNT];
	Color color;

	MainFreeze_SafeAdvDestroy();

	UNLOCK_ADV_BIT(sdata->advProgress.rewards, ADV_REWARD_HINT_WELCOME_TO_ARENA);

	int numHintsFound = 0;

	int i = 0;
	const s16 *ptrLngID = &D232.hintMenuLngIndex[0];

	for (/**/; *ptrLngID > -1; ptrLngID++, i++)
	{
		int hintID = (ptrLngID[0] - AH_HINTMENU_HINT_LNG_FIRST) / 2;
		int bitIndex = hintID + ADV_REWARD_FIRST_HINT;

		if (CHECK_ADV_BIT(sdata->advProgress.rewards, bitIndex))
		{
			hintsFound[numHintsFound] = i;
			numHintsFound++;
		}
	}

	s16 rowCount = numHintsFound + 1;

	if (menu->rowSelected > numHintsFound)
	{
		menu->rowSelected = numHintsFound;
	}

	if ((D232.hintMenu_scrollIndex > (numHintsFound - AH_HINTMENU_SCROLL_MARGIN)) &&
	    (D232.hintMenu_scrollIndex = (s16)(numHintsFound - AH_HINTMENU_SCROLL_MARGIN), D232.hintMenu_scrollIndex < 0))
	{
		D232.hintMenu_scrollIndex = 0;
	}

	int lngIndex = D232.hintMenuLngIndex[(s32)hintsFound[menu->rowSelected]];

	// if viewing a hint
	if (D232.hintMenu_boolViewHint != 0)
	{
		AH_HintMenu_MaskPosRot();

		if (D232.maskCooldown > 0)
		{
			D232.maskCooldown--;
		}

		// If you press Cross, Square, Triangle, or Circle
		if (((sdata->buttonTapPerPlayer[0] & AH_HINTMENU_INPUT_VIEW_EXIT) != 0) && (VehTalkMask_boolNoXA() || (D232.maskCooldown == 0)))
		{
			D232.hintMenu_boolViewHint = 0;

			RECTMENU_ClearInput();

			VehTalkMask_End();
		}

		DecalFont_DrawLine(sdata->lngStrings[lngIndex + 0], 0x100, 0x2c, 1, 0xffff8000);

		// height of multiLine
		int textHeight = DecalFont_DrawMultiLine(sdata->lngStrings[lngIndex + 1], 0x96, 0x3f, 0x14e, 2, 0);

		char *strExit = sdata->lngStrings[LNG_HINT_EXIT];

		DecalFont_DrawLine(strExit, 0x100, textHeight + 0x4f, 1, 0xffff8000);

		int exitWidth = DecalFont_GetLineWidth(strExit, 1);

		int exitBoxWidth = exitWidth + 6;
		box.x = 0xff - (exitBoxWidth >> 1);
		box.w = exitBoxWidth;
		box.y = (s16)textHeight + 0x4e;
		box.h = 0x11;

		uint32_t *ot = gGT->backBuffer->otMem.uiOT;
		CTR_Box_DrawClearBox(&box, &sdata->menuRowHighlight_Normal, TRANS_50_DECAL, ot);

		box.y = 0x3c;
		box.x = -0xe;
		box.w = 0x21c;
		box.h = 2;

		color.self = sdata->battleSetup_Color_UI_1;
		RECTMENU_DrawOuterRect_Edge(&box, color, 0x20, ot);

		box.y = 0x28;
		box.h = (s16)textHeight + 0x3b;
		box.x = -0xe;
		box.w = 0x21c;

		RECTMENU_DrawInnerRect(&box, 4, &ot[3]);

		return;
	}

	int tapP1 = sdata->buttonTapPerPlayer[0];

	// If you press Up, Down, Cross, Square, Triangle, Circle
	if ((tapP1 & AH_HINTMENU_INPUT_NAV) == 0)
	{
		goto LAB_800b38cc;
	}

	// If you dont press Up
	if ((tapP1 & BTN_UP) == 0)
	{
		// If you dont press Down
		if ((tapP1 & BTN_DOWN) == 0)
		{
			// If you dont press Cross or Circle
			if ((tapP1 & AH_HINTMENU_INPUT_CONFIRM) == 0)
			{
				// If you press Triangle or Square
				if ((tapP1 & AH_HINTMENU_INPUT_BACK) != 0)
				{
					// Play sound
					// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b38b4-0x800b38bc for hint-menu back SFX.
					OtherFX_Play(2, 1);
					goto LAB_800b38c0;
				}
			}

			// If you press Cross or Circle
			else
			{
				// Play Sound
				// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b3788-0x800b3790 for hint-menu confirm SFX.
				OtherFX_Play(1, 1);

				if (menu->rowSelected == numHintsFound)
				{
				LAB_800b38c0:
					shouldClose = true;
				}
				else
				{
					// If there is no loading in progress
					if ((sdata->load_inProgress == 0) && (sdata->XA_State == 0))
					{
						sdata->instMaskHints3D = VehTalkMask_Init();

						D232.maskCooldown = AH_HINTMENU_VIEW_COOLDOWN_FRAMES;

						VehTalkMask_PlayXA(sdata->instMaskHints3D, (lngIndex - AH_HINTMENU_HINT_LNG_FIRST) / 2);

						AH_HintMenu_MaskPosRot();

						// talking mask instance
						struct Instance *inst = sdata->instMaskHints3D;
						inst->flags |= SCREENSPACE_INSTANCE;

						struct InstDrawPerPlayer *idpp = INST_GETIDPP(inst);

						idpp[0].pushBuffer = &gGT->pushBuffer_UI;
						for (int j = 1; j < gGT->numPlyrCurrGame; j++)
						{
							idpp[j].pushBuffer = 0;
						}

						// now viewing a hint
						D232.hintMenu_boolViewHint = 1;
					}
				}
			}
		}

		// If you press Down
		else
		{
			// if you are not on the bottom row
			if (menu->rowSelected < numHintsFound)
			{
				// Play Sound
				// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b375c-0x800b3764 for hint-menu down SFX.
				OtherFX_Play(0, 1);

				// change which row is selected
				menu->rowSelected = menu->rowSelected + 1;
			}
		}
	}

	// If you press Up
	else
	{
		// if you are not on the top row
		if (0 < menu->rowSelected)
		{
			// Play Sound
			// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b3724-0x800b372c for hint-menu up SFX.
			OtherFX_Play(0, 1);

			// change the row selected
			menu->rowSelected--;
		}
	}

	// clear gamepad input (for menus)
	RECTMENU_ClearInput();

	u32 isGoodMask;

LAB_800b38cc:

	isGoodMask = VehPickupItem_MaskBoolGoodGuy(gGT->drivers[0]);

	// Draw the "Hints" string
	DecalFont_DrawLine(sdata->lngStrings[LNG_AKU_AKU_HINTS_MENU + (isGoodMask == 0)], 0x100, 0x2c, 1, 0xffff8000);

	if (D232.hintMenu_scrollIndex + AH_HINTMENU_VISIBLE_ROWS <= menu->rowSelected)
	{
		D232.hintMenu_scrollIndex = menu->rowSelected - AH_HINTMENU_SCROLL_MARGIN;
	}
	if (menu->rowSelected < D232.hintMenu_scrollIndex)
	{
		D232.hintMenu_scrollIndex = menu->rowSelected;
	}
	s16 visibleRows = AH_HINTMENU_VISIBLE_ROWS;
	if (rowCount < D232.hintMenu_scrollIndex + AH_HINTMENU_VISIBLE_ROWS)
	{
		visibleRows = rowCount;
	}

	// if scroll bar is not on top
	if (0 < D232.hintMenu_scrollIndex)
	{
		AH_HintMenu_FiveArrows(0x3f, 0);
	}

	int menuHeight = 0x10;
	int visibleRowIndex = 0;
	if (0 < visibleRows)
	{
		do
		{
			int hintListIndex = D232.hintMenu_scrollIndex + (int)(s16)visibleRowIndex;
			u32 rowLngIndex;
			if (hintListIndex < numHintsFound)
			{
				rowLngIndex = D232.hintMenuLngIndex[(s32)hintsFound[hintListIndex]];
			}
			else
			{
				// EXIT
				rowLngIndex = LNG_HINT_EXIT;
			}
			int rowPosY = menuHeight + 0x40;
			menuHeight = menuHeight + 0x10;

			// "EXIT"
			DecalFont_DrawLine(sdata->lngStrings[rowLngIndex], 0x100, rowPosY, 1, 0xffff8000);

			visibleRowIndex = visibleRowIndex + 1;
		} while (visibleRowIndex < visibleRows);
	}

	// if scroll bar is not on bottom
	if (D232.hintMenu_scrollIndex + (int)visibleRows < (int)rowCount)
	{
		AH_HintMenu_FiveArrows((menuHeight + 0x40), 0x800);
	}
	box.x = -0xe;
	box.w = 0x21c;
	box.y = (menu->rowSelected - D232.hintMenu_scrollIndex) * 0x10 + 0x4f;
	box.h = 0x11;

	uint32_t *ot = gGT->backBuffer->otMem.uiOT;
	CTR_Box_DrawClearBox(&box, &sdata->menuRowHighlight_Normal, TRANS_50_DECAL, ot);

	box.y = 0x3c;
	box.h = 2;
	box.x = -0x14;
	box.w = 0x228;

	color.self = sdata->battleSetup_Color_UI_1;
	RECTMENU_DrawOuterRect_Edge(&box, color, 0x20, ot);

	box.y = 0x28;
	box.h = (s16)menuHeight + 0x2b;
	box.x = -0x14;
	box.w = 0x228;

	RECTMENU_DrawInnerRect(&box, 4, ot);

	if ((shouldClose) ||

	    // If you dont press Start, Square, or Triangle
	    ((sdata->buttonTapPerPlayer[0] & AH_HINTMENU_INPUT_CLOSE) != 0))
	{
		RECTMENU_ClearInput();
		sdata->ptrDesiredMenu = MainFreeze_GetMenuPtr();
	}
	return;
}
