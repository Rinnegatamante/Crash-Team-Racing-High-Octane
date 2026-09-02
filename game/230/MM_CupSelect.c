#include <common.h>

enum
{
	MM_CUSTOM_CUPS_TRACK_COUNT = 0x12,
};

enum
{
	MM_CUSTOM_CUPS_LANGUAGE_ENGLISH = 0,
	MM_CUSTOM_CUPS_LANGUAGE_FRENCH,
	MM_CUSTOM_CUPS_LANGUAGE_GERMAN,
	MM_CUSTOM_CUPS_LANGUAGE_ITALIAN,
	MM_CUSTOM_CUPS_LANGUAGE_SPANISH,
	MM_CUSTOM_CUPS_LANGUAGE_DUTCH,
	MM_CUSTOM_CUPS_LANGUAGE_COUNT,
};

struct MMCupSelectCustomCupsText
{
	const char *openEditor;
	const char *editCup;
	const char *randomize;
};

static const struct MMCupSelectCustomCupsText s_customCupsText[MM_CUSTOM_CUPS_LANGUAGE_COUNT] =
{
	{"SELECT: CUSTOM CUPS", "D-PAD: CHANGE TRACKS", "R1: RANDOMIZE TRACKS"},
	{"SELECT: COUPES PERSONNALISEES", "D-PAD: CHANGER PISTES", "R1: PISTES ALEATOIRES"},
	{"SELECT: EIGENE CUPS", "STEUERKREUZ: STRECKEN WECHSELN", "R1: STRECKEN ZUFAELLIG"},
	{"SELECT: COPPE PERSONALIZZATE", "D-PAD: CAMBIA PISTE", "R1: RANDOMIZZA PISTE"},
	{"SELECT: COPAS PERSONALIZADAS", "CRUCETA: CAMBIAR PISTAS", "R1: PISTAS ALEATORIAS"},
	{"SELECT: AANGEPASTE BEKERS", "D-PAD: BANEN WIJZIGEN", "R1: WILLEKEURIGE BANEN"},
};

#if defined(__vita__)
extern int cfg_language;
#endif

static const struct MMCupSelectCustomCupsText *MM_CupSelect_CustomCups_GetText(void)
{
	int language = MM_CUSTOM_CUPS_LANGUAGE_ENGLISH;

#if defined(__vita__)
	if ((cfg_language >= 2) && (cfg_language <= 7))
	{
		language = cfg_language - 2;
	}
#endif

	return &s_customCupsText[language];
}

struct MMCupSelectCustomCupsState
{
	s8 choice[GAME_PROGRESS_CUP_COUNT][MM_CUP_TRACK_COUNT];
	s8 selectedTrack;
	s8 modifiedCup;
	u8 open;
};

static struct MMCupSelectCustomCupsState customCups;

static s8 MM_CupSelect_CustomCups_FindTrackRow(s16 trackID)
{
	for (s8 row = 0; row < MM_CUSTOM_CUPS_TRACK_COUNT; row++)
	{
		if (D230.arcadeTracks[row].levID == trackID)
		{
			return row;
		}
	}

	return 0;
}

static void MM_CupSelect_CustomCups_SyncChoices(void)
{
	for (u8 cupIndex = 0; cupIndex < GAME_PROGRESS_CUP_COUNT; cupIndex++)
	{
		for (u8 trackIndex = 0; trackIndex < MM_CUP_TRACK_COUNT; trackIndex++)
		{
			customCups.choice[cupIndex][trackIndex] =
			    MM_CupSelect_CustomCups_FindTrackRow(data.ArcadeCups[cupIndex].CupTrack[trackIndex].trackID);
		}
	}

	customCups.selectedTrack = 0;
	customCups.modifiedCup = 0;
	customCups.open = 0;
}

static b32 MM_CupSelect_CustomCups_TrackAlreadyUsed(u8 cupIndex, u8 numTracksSet, s8 trackRow)
{
	for (u8 trackIndex = 0; trackIndex < numTracksSet; trackIndex++)
	{
		if (customCups.choice[cupIndex][trackIndex] == trackRow)
		{
			return 1;
		}
	}

	return 0;
}

static void MM_CupSelect_CustomCups_ApplyCup(u8 cupIndex)
{
	for (u8 trackIndex = 0; trackIndex < MM_CUP_TRACK_COUNT; trackIndex++)
	{
		struct MainMenu_LevelRow *track = &D230.arcadeTracks[customCups.choice[cupIndex][trackIndex]];

		data.ArcadeCups[cupIndex].CupTrack[trackIndex].trackID = track->levID;
		data.ArcadeCups[cupIndex].CupTrack[trackIndex].iconID = track->videoThumbnail;
	}
}

static void MM_CupSelect_CustomCups_Randomize(u8 cupIndex)
{
	u8 trackIndex = 0;

	while (trackIndex < MM_CUP_TRACK_COUNT)
	{
		MixRNG_Scramble();

		s8 trackRow = ((Timer_GetTime_Total() & 0xf) + (sdata->randomNumber >> 8)) % MM_CUSTOM_CUPS_TRACK_COUNT;

		if (MM_CupSelect_CustomCups_TrackAlreadyUsed(cupIndex, trackIndex, trackRow))
		{
			continue;
		}

		customCups.choice[cupIndex][trackIndex] = trackRow;
		trackIndex++;
	}
}

static void MM_CupSelect_CustomCups_Update(struct RectMenu *menu)
{
	struct GameTracker *gGT = sdata->gGT;
	const struct MMCupSelectCustomCupsText *customText = MM_CupSelect_CustomCups_GetText();

	DecalFont_DrawLine(customText->openEditor, 0x100, 0x4, FONT_SMALL, JUSTIFY_CENTER | ORANGE);

	if (D230.cupSelectTransition.state != IN_MENU)
	{
		return;
	}

	u32 buttonTap = sdata->gGamepads->gamepad[0].buttonsTapped;

	if (buttonTap & BTN_SELECT)
	{
		if (customCups.open == 0)
		{
			if ((menu->rowSelected < 0) || (menu->rowSelected >= GAME_PROGRESS_CUP_COUNT))
			{
				return;
			}

			customCups.modifiedCup = menu->rowSelected;
			customCups.selectedTrack = 0;
			customCups.open = 1;

			menu->state &= ~(EXECUTE_FUNCPTR);
			menu->state |= DISABLE_INPUT_ALLOW_FUNCPTRS;
		}
		else
		{
			customCups.open = 0;

			menu->state &= ~(DISABLE_INPUT_ALLOW_FUNCPTRS);
			menu->state |= EXECUTE_FUNCPTR;
		}
	}

	if (customCups.open == 0)
	{
		return;
	}

	menu->rowSelected = customCups.modifiedCup;

	if (buttonTap & BTN_UP)
	{
		customCups.selectedTrack = (customCups.selectedTrack + MM_CUP_TRACK_COUNT - 1) % MM_CUP_TRACK_COUNT;
	}

	if (buttonTap & BTN_DOWN)
	{
		customCups.selectedTrack = (customCups.selectedTrack + 1) % MM_CUP_TRACK_COUNT;
	}

	if (buttonTap & BTN_LEFT)
	{
		s8 *trackRow = &customCups.choice[customCups.modifiedCup][customCups.selectedTrack];

		if (*trackRow == 0)
		{
			*trackRow = MM_CUSTOM_CUPS_TRACK_COUNT - 1;
		}
		else
		{
			(*trackRow)--;
		}
	}

	if (buttonTap & BTN_RIGHT)
	{
		s8 *trackRow = &customCups.choice[customCups.modifiedCup][customCups.selectedTrack];

		(*trackRow)++;

		if (*trackRow >= MM_CUSTOM_CUPS_TRACK_COUNT)
		{
			*trackRow = 0;
		}
	}

	if (buttonTap & BTN_R1)
	{
		MM_CupSelect_CustomCups_Randomize(customCups.modifiedCup);
	}

	MM_CupSelect_CustomCups_ApplyCup(customCups.modifiedCup);

	int isRight = customCups.modifiedCup & 1;
	int isBottom = customCups.modifiedCup >= 2;
	int textX = isRight ? 0x118 : 0x38;
	int textY = isBottom ? 0x84 : 0x30;

	DecalFont_DrawLine("-", textX - 0x10, textY + 0x10 * customCups.selectedTrack, FONT_SMALL, ORANGE);

	for (u8 trackIndex = 0; trackIndex < MM_CUP_TRACK_COUNT; trackIndex++)
	{
		s16 trackID = data.ArcadeCups[customCups.modifiedCup].CupTrack[trackIndex].trackID;

		DecalFont_DrawLine(sdata->lngStrings[data.metaDataLEV[trackID].name_LNG], textX, textY + 0x10 * trackIndex, FONT_SMALL, ORANGE);
	}

	RECT selectedCupWindow = {
	    isRight ? 0x100 : 0x20,
	    isBottom ? 0x79 : 0x25,
	    0xE0,
	    0x4E,
	};

	RECT helpWindow = {
	    0x40,
	    isBottom ? 0x25 : 0x79,
	    0x180,
	    0x50,
	};

	DecalFont_DrawLine(customText->editCup, 0x100, helpWindow.y + 0x18, FONT_SMALL, JUSTIFY_CENTER | PERIWINKLE);
	DecalFont_DrawLine(customText->randomize, 0x100, helpWindow.y + 0x30, FONT_SMALL, JUSTIFY_CENTER | PERIWINKLE);

	RECTMENU_DrawInnerRect(&selectedCupWindow, 1, gGT->backBuffer->otMem.uiOT);
	RECTMENU_DrawInnerRect(&helpWindow, 1, gGT->backBuffer->otMem.uiOT);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b0eb8-0x800b0eec.
void MM_CupSelect_Init(void)
{
	MM_CupSelect_CustomCups_SyncChoices();

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

	MM_CupSelect_CustomCups_Update(menu);

	if (menu->funcState == RECTMENU_FUNC_STATE_INPUT)
	{
		customCups.open = 0;

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
			if ((sdata->frameCounter & FPS_DOUBLE(MM_CUP_SELECT_FLASH_FRAME_BIT)) == 0)
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
