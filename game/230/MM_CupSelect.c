#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b0eb8-0x800b0eec.
void MM_CupSelect_Init(void)
{
	// reset transition data
	D230.cupSelectTransition.frame = MM_CUP_SELECT_INITIAL_TRANSITION_FRAMES;
	D230.cupSelectTransition.state = ENTERING_MENU;

	// disable menu callback execution while the cup menu transitions in
	D230.menuCupSelect.state &= ~(EXECUTE_FUNCPTR);
	// allow the callback to keep drawing while input stays blocked
	D230.menuCupSelect.state |= DISABLE_INPUT_ALLOW_FUNCPTRS;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 overlay 230 0x800b0eec-0x800b164c.
void MM_CupSelect_MenuProc(struct RectMenu *menu)
{
	struct GameTracker *gGT = sdata->gGT;

	if (menu->funcState == RECTMENU_FUNC_STATE_INPUT)
	{
		D230.cupSelectTransition.startAfterExit = (menu->rowSelected != -1);
		D230.cupSelectTransition.state = EXITING_MENU;
		D230.menuCupSelect.state &= ~(EXECUTE_FUNCPTR);
		D230.menuCupSelect.state |= DISABLE_INPUT_ALLOW_FUNCPTRS;
		return;
	}

	s16 elapsedFrames = D230.cupSelectTransition.frame;

	// if not stationary
	if (D230.cupSelectTransition.state != IN_MENU)
	{
		// if transitioning in
		if (D230.cupSelectTransition.state == ENTERING_MENU)
		{
			MM_TransitionInOut(D230.transitionMeta_cupSel, elapsedFrames, MM_CUP_SELECT_LERP_FRAMES);

			// if no more frames
			if (elapsedFrames == 0)
			{
				// menu is now in focus
				D230.cupSelectTransition.state = IN_MENU;
				D230.menuCupSelect.state &= ~(DISABLE_INPUT_ALLOW_FUNCPTRS);
				D230.menuCupSelect.state |= EXECUTE_FUNCPTR;
			}

			else
			{
				elapsedFrames--;
			}
		}
		// if transitioning out
		else if (D230.cupSelectTransition.state == EXITING_MENU)
		{
			MM_TransitionInOut(D230.transitionMeta_cupSel, elapsedFrames, MM_CUP_SELECT_LERP_FRAMES);

			// increase frame count
			elapsedFrames++;

			// if more than 12 frames pass
			if (MM_CUP_SELECT_TRANSITION_OUT_DONE_FRAME < elapsedFrames)
			{
				// if cup selected
				if (D230.cupSelectTransition.startAfterExit != 0)
				{
					// set cupID to the cup selected
					gGT->cup.cupID = menu->rowSelected;

					// set track index to zero, to go to first track
					gGT->cup.trackIndex = 0;

					// loop through 8 drivers
					for (s32 driverIndex = 0; driverIndex < MM_CUP_SELECT_DRIVER_SLOT_COUNT; driverIndex++)
					{
						// set all points for all 8 drivers to zero
						gGT->cup.points[driverIndex] = 0;
					}

					// passthrough Menu for the function
					sdata->ptrDesiredMenu = &data.menuQueueLoadTrack;

					// set current level
					gGT->currLEV = data.ArcadeCups[gGT->cup.cupID].CupTrack[gGT->cup.trackIndex].trackID;
					return;
				}

				// return to character selection
				sdata->ptrDesiredMenu = &D230.menuCharacterSelect;

				MM_Characters_RestoreIDs();
				return;
			}
		}
	}

	D230.cupSelectTransition.frame = elapsedFrames;

	DecalFont_DrawLine(sdata->lngStrings[LNG_SELECT_CUP_RACE], D230.transitionMeta_cupSel[MM_CUP_SELECT_TITLE_META_INDEX].currX + MM_CUP_SELECT_TITLE_X_OFFSET,
	                   D230.transitionMeta_cupSel[MM_CUP_SELECT_TITLE_META_INDEX].currY + MM_CUP_SELECT_TITLE_Y_OFFSET, FONT_BIG, MM_CUP_SELECT_TEXT_COLOR);

	// Loop through all four cups
	for (u8 cupIndex = 0; cupIndex < GAME_PROGRESS_CUP_COUNT; cupIndex++)
	{
		// Use solid color
		u32 txtColor = MM_CUP_SELECT_TEXT_COLOR;

		// If this cup is the one you selected
		if (cupIndex == menu->rowSelected)
		{
			// Make text flash
			if ((sdata->frameCounter & MM_CUP_SELECT_FLASH_FRAME_BIT) == 0)
			{
				txtColor |= MM_CUP_SELECT_FLASH_COLOR_BIT;
			}
		}

		int startX = (s16)D230.transitionMeta_cupSel[cupIndex].currX + (cupIndex & 1) * MM_CUP_SELECT_COLUMN_WIDTH;
		int startY = (s16)D230.transitionMeta_cupSel[cupIndex].currY + (cupIndex >> 1) * MM_CUP_SELECT_ROW_HEIGHT;

		// draw the name of the cup
		DecalFont_DrawLine(sdata->lngStrings[data.ArcadeCups[cupIndex].lngIndex_CupName], startX + MM_CUP_SELECT_NAME_X_OFFSET,
		                   startY + MM_CUP_SELECT_NAME_Y_OFFSET, FONT_CREDITS, txtColor);

		startX = startX + MM_CUP_SELECT_CONTENT_X_OFFSET;
		startY = startY + MM_CUP_SELECT_CONTENT_Y_OFFSET;

		// loop through 3 stars to draw
		for (u8 starIndex = 0; starIndex < GAME_PROGRESS_CUP_DIFFICULTY_COUNT; starIndex++)
		{
			int cupWinBitIndex = D230.cupSelectStars.winBitBase[starIndex] + cupIndex;
			if (CHECK_ADV_BIT(sdata->gameProgress.unlocks, cupWinBitIndex))
			{
				u32 *starColor = data.ptrColor[D230.cupSelectStars.colorIndex[starIndex]];

				struct Icon **iconPtrArray = ICONGROUP_GETICONS(gGT->iconGroup[MM_CUP_SELECT_STAR_ICON_GROUP]);

				DecalHUD_DrawPolyGT4(iconPtrArray[MM_CUP_SELECT_STAR_ICON_ID],
				                     startX + (cupIndex & 1) * MM_CUP_SELECT_STAR_COLUMN_BIAS + MM_CUP_SELECT_STAR_X_OFFSET,
				                     startY + starIndex * MM_CUP_SELECT_STAR_Y_STEP + MM_CUP_SELECT_STAR_Y_OFFSET, &gGT->backBuffer->primMem,
				                     gGT->pushBuffer_UI.ptrOT, starColor[0], starColor[1], starColor[2], starColor[3], 0, FP(1.0));
			}
		}

		// loop through all four track icons in one cup
		for (u8 trackIndex = 0; trackIndex < MM_CUP_TRACK_COUNT; trackIndex++)
		{
			int posX = startX + (trackIndex & 1) * MM_CUP_SELECT_TRACK_X_STEP;
			int posY = startY + (trackIndex >> 1) * MM_CUP_SELECT_TRACK_Y_STEP;

			// Draw Icon of each track
			RECTMENU_DrawPolyGT4(gGT->ptrIcons[data.ArcadeCups[cupIndex].CupTrack[trackIndex].iconID], posX, posY, &gGT->backBuffer->primMem,
			                     gGT->pushBuffer_UI.ptrOT, D230.cupSel_Color.self, D230.cupSel_Color.self, D230.cupSel_Color.self, D230.cupSel_Color.self, 0,
			                     FP(0.5));
		}

		RECT cupBox;

		if (cupIndex == menu->rowSelected)
		{
			// highlight box
			cupBox.x = startX + MM_CUP_SELECT_HIGHLIGHT_X_OFFSET;
			cupBox.y = startY + MM_CUP_SELECT_HIGHLIGHT_Y_OFFSET;
			cupBox.w = MM_CUP_SELECT_HIGHLIGHT_WIDTH;
			cupBox.h = MM_CUP_SELECT_HIGHLIGHT_HEIGHT;

			CTR_Box_DrawClearBox(&cupBox, &sdata->menuRowHighlight_Normal, TRANS_50_DECAL, gGT->backBuffer->otMem.uiOT);
		}

		// background box
		cupBox.x = startX + MM_CUP_SELECT_BACKGROUND_X_OFFSET;
		cupBox.y = startY + MM_CUP_SELECT_BACKGROUND_Y_OFFSET;
		cupBox.w = MM_CUP_SELECT_BACKGROUND_WIDTH;
		cupBox.h = MM_CUP_SELECT_BACKGROUND_HEIGHT;

		RECTMENU_DrawInnerRect(&cupBox, 0, gGT->backBuffer->otMem.uiOT);
	}
}
