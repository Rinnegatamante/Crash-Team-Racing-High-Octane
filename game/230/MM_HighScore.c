#include <common.h>

enum
{
	MM_HIGHSCORE_MAIN_TRANSITION_MAX_FRAME = 0xc,
	MM_HIGHSCORE_SLIDE_TRANSITION_FRAMES = 8,
	MM_HIGHSCORE_LAST_ARCADE_TRACK = 0x11,
	MM_HIGHSCORE_TRACK_SLIDE_STEP_X = 0x40,
	MM_HIGHSCORE_ROW_SLIDE_STEP_Y = 0x1b,
	MM_HIGHSCORE_OFFSCREEN_X = 0x200,
	MM_HIGHSCORE_OFFSCREEN_Y = 0xd8,
	MM_HIGHSCORE_WIPE_RECT_W = 0x228,
	MM_HIGHSCORE_WIPE_RECT_H = 0x19,
	MM_HIGHSCORE_WIPE_RECT_X_OFFSET = -0x14,
	MM_HIGHSCORE_WIPE_RECT_Y_OFFSET = 9,
	MM_HIGHSCORE_TEXT_SHADOW_X = 3,
	MM_HIGHSCORE_TEXT_SHADOW_Y = 1,
	MM_HIGHSCORE_ARROW_ICON_GROUP = 4,
	MM_HIGHSCORE_ARROW_ICON_ID = 0x38,
	MM_HIGHSCORE_ARROW_LEFT_X_OFFSET = 0xec,
	MM_HIGHSCORE_ARROW_RIGHT_X_OFFSET = 0x112,
	MM_HIGHSCORE_ARROW_Y_OFFSET = 0x15,
	MM_HIGHSCORE_ARROW_SCALE = 0x1000,
	MM_HIGHSCORE_ARROW_LEFT_ROTATION = 0x800,
	MM_HIGHSCORE_TITLE_X_OFFSET = 0x100,
	MM_HIGHSCORE_TITLE_Y_OFFSET = 0xe,
	MM_HIGHSCORE_FLASH_TIMER_BIT = 4,
	MM_HIGHSCORE_BEST_TRACK_LABEL_X_OFFSET = 0x20,
	MM_HIGHSCORE_BEST_TRACK_LABEL_Y_OFFSET = 0x2b,
	MM_HIGHSCORE_SCORE_MODE_TIME_TRIAL = 0,
	MM_HIGHSCORE_GHOST_STAR_COUNT = 2,
	MM_HIGHSCORE_GHOST_STAR_ICON_GROUP = 5,
	MM_HIGHSCORE_GHOST_STAR_ICON_ID = 0x37,
	MM_HIGHSCORE_GHOST_STAR_X_OFFSET = 0xf0,
	MM_HIGHSCORE_GHOST_STAR_X_STEP = 0x10,
	MM_HIGHSCORE_GHOST_STAR_Y_OFFSET = 4,
	MM_HIGHSCORE_GHOST_STAR_SCALE = 0x1000,
	MM_HIGHSCORE_TITLE_META_INDEX = 0,
	MM_HIGHSCORE_BEST_TRACK_META_INDEX = 1,
	MM_HIGHSCORE_BEST_LAP_LABEL_META_INDEX = 7,
	MM_HIGHSCORE_BEST_LAP_ENTRY_META_INDEX = 8,
	MM_HIGHSCORE_BEST_LAP_LABEL_X_OFFSET = 0x124,
	MM_HIGHSCORE_BEST_LAP_LABEL_Y_OFFSET = 0x2b,
	MM_HIGHSCORE_BEST_LAP_TEXT_X_OFFSET = 0x160,
	MM_HIGHSCORE_BEST_LAP_NAME_Y_OFFSET = 0x39,
	MM_HIGHSCORE_BEST_LAP_TIME_Y_OFFSET = 0x4a,
	MM_HIGHSCORE_BEST_LAP_ICON_X_OFFSET = 0x124,
	MM_HIGHSCORE_BEST_LAP_ICON_Y_OFFSET = 0x38,
	MM_HIGHSCORE_DRIVER_COLOR_OFFSET = 5,
	MM_HIGHSCORE_ICON_TRANSPARENCY = 1,
	MM_HIGHSCORE_ICON_SCALE = 0x1000,
	MM_HIGHSCORE_VISIBLE_SCORE_ROWS = 5,
	MM_HIGHSCORE_FIRST_VISIBLE_ENTRY = 1,
	MM_HIGHSCORE_FIRST_VISIBLE_META_INDEX = 2,
	MM_HIGHSCORE_SCORE_ROW_Y_STEP = 0x1f,
	MM_HIGHSCORE_SCORE_ICON_X_OFFSET = 0x20,
	MM_HIGHSCORE_SCORE_NAME_X_OFFSET = 0x5c,
	MM_HIGHSCORE_SCORE_NAME_Y_OFFSET = 0x39,
	MM_HIGHSCORE_SCORE_TIME_Y_OFFSET = 0x4a,
	MM_HIGHSCORE_VIDEO_META_INDEX = 9,
	MM_HIGHSCORE_MENU_META_INDEX = 10,
	MM_HIGHSCORE_VIDEO_BOX_W = 0xb0,
	MM_HIGHSCORE_VIDEO_BOX_H = 0x4b,
	MM_HIGHSCORE_VIDEO_BOX_X_OFFSET = 0x124,
	MM_HIGHSCORE_VIDEO_BOX_Y_OFFSET = 0x5a,
	MM_HIGHSCORE_MENU_WIDTH = 0xa4,
};

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 230 0x800b2f0c-0x800b2fbc.
void MM_HighScore_Text3D(char *string, int posX, int posY, s16 font, u32 flags)
{
	// draw a string
	DecalFont_DrawLine(string, posX, posY, font, flags);

	// draw the same string in a different place
	DecalFont_DrawLine(string, posX + MM_HIGHSCORE_TEXT_SHADOW_X, posY + MM_HIGHSCORE_TEXT_SHADOW_Y, font, (flags & (JUSTIFY_CENTER | JUSTIFY_RIGHT)) | BLACK);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 overlay 230 0x800b2fbc-0x800b3914.
void MM_HighScore_Draw(u16 trackIndex, u32 rowIndex, u32 posX, u32 posY)
{
	struct GameTracker *gGT = sdata->gGT;

	s16 offsetX = (s16)posX;
	s16 offsetY = (s16)posY;

	s16 levelID = D230.arcadeTracks[trackIndex].levID;

	s16 lineWidth = DecalFont_GetLineWidth(sdata->lngStrings[data.metaDataLEV[levelID].name_LNG], FONT_BIG);
	lineWidth = lineWidth >> 1;

	// get color data
	s16 numColor = ((sdata->frameCounter & MM_HIGHSCORE_FLASH_TIMER_BIT) == 0) ? RED : ORANGE;
	u32 *colorPtr = data.ptrColor[numColor];

	struct Icon **iconPtrArray = ICONGROUP_GETICONS(gGT->iconGroup[MM_HIGHSCORE_ARROW_ICON_GROUP]);
	const struct TransitionMeta *titleMeta = &D230.transitionMeta_HighScores[MM_HIGHSCORE_TITLE_META_INDEX];
	const struct TransitionMeta *bestTrackMeta = &D230.transitionMeta_HighScores[MM_HIGHSCORE_BEST_TRACK_META_INDEX];
	const struct TransitionMeta *bestLapLabelMeta = &D230.transitionMeta_HighScores[MM_HIGHSCORE_BEST_LAP_LABEL_META_INDEX];
	const struct TransitionMeta *bestLapEntryMeta = &D230.transitionMeta_HighScores[MM_HIGHSCORE_BEST_LAP_ENTRY_META_INDEX];

	// Draw arrow pointing Left
	DecalHUD_Arrow2D(iconPtrArray[MM_HIGHSCORE_ARROW_ICON_ID], titleMeta->currX + (offsetX - lineWidth) + MM_HIGHSCORE_ARROW_LEFT_X_OFFSET,
	                 titleMeta->currY + offsetY + MM_HIGHSCORE_ARROW_Y_OFFSET, &gGT->backBuffer->primMem, gGT->pushBuffer_UI.ptrOT, colorPtr[0], colorPtr[1],
	                 colorPtr[2], colorPtr[3], 0, MM_HIGHSCORE_ARROW_SCALE, MM_HIGHSCORE_ARROW_LEFT_ROTATION);

	// Draw arrow pointing Right
	DecalHUD_Arrow2D(iconPtrArray[MM_HIGHSCORE_ARROW_ICON_ID], titleMeta->currX + (lineWidth + offsetX) + MM_HIGHSCORE_ARROW_RIGHT_X_OFFSET,
	                 titleMeta->currY + offsetY + MM_HIGHSCORE_ARROW_Y_OFFSET, &gGT->backBuffer->primMem, gGT->pushBuffer_UI.ptrOT, colorPtr[0], colorPtr[1],
	                 colorPtr[2], colorPtr[3], 0, MM_HIGHSCORE_ARROW_SCALE, 0);

	// draw track name
	DecalFont_DrawLine(sdata->lngStrings[data.metaDataLEV[levelID].name_LNG], titleMeta->currX + (s16)(posX + MM_HIGHSCORE_TITLE_X_OFFSET),
	                   titleMeta->currY + (s16)(posY + MM_HIGHSCORE_TITLE_Y_OFFSET), FONT_BIG, JUSTIFY_CENTER);

	Color iconColor = D230.highscore_iconColor;

	MM_HighScore_Text3D(sdata->lngStrings[LNG_BEST_TRACK_TIMES], bestTrackMeta->currX + offsetX + MM_HIGHSCORE_BEST_TRACK_LABEL_X_OFFSET,
	                    bestTrackMeta->currY + offsetY + MM_HIGHSCORE_BEST_TRACK_LABEL_Y_OFFSET, FONT_SMALL, 0);

	// first entry: Time Trial or Relic
	struct HighScoreEntry *entry = &sdata->gameProgress.highScoreTracks[levelID].scoreEntry[rowIndex * MEMCARD_HIGH_SCORE_ENTRIES_PER_MODE];

	// if Time Trial
	// with ghost stars, and Best Lap
	if ((rowIndex & 0xffff) == MM_HIGHSCORE_SCORE_MODE_TIME_TRIAL)
	{
		s32 prevLevelID = gGT->levelID;

		gGT->levelID = levelID;
		GAMEPROG_GetPtrHighScoreTrack();

		// draw ghost stars
		for (s32 ghostStarIndex = 0; ghostStarIndex < MM_HIGHSCORE_GHOST_STAR_COUNT; ghostStarIndex++)
		{
			if (((sdata->gameProgress.highScoreTracks[gGT->levelID].timeTrialFlags >> D230.highScoreGhostStars.beatenFlagBit[ghostStarIndex]) & 1) != 0)
			{
				colorPtr = data.ptrColor[D230.highScoreGhostStars.colorIndex[ghostStarIndex]];

				struct Icon **ptrIconArray = ICONGROUP_GETICONS(gGT->iconGroup[MM_HIGHSCORE_GHOST_STAR_ICON_GROUP]);

				DecalHUD_DrawPolyGT4(ptrIconArray[MM_HIGHSCORE_GHOST_STAR_ICON_ID],
				                     titleMeta->currX + offsetX + (ghostStarIndex * MM_HIGHSCORE_GHOST_STAR_X_STEP) + MM_HIGHSCORE_GHOST_STAR_X_OFFSET,
				                     titleMeta->currY + offsetY + MM_HIGHSCORE_GHOST_STAR_Y_OFFSET,
				                     // pointer to PrimMem struct
				                     &gGT->backBuffer->primMem,
				                     // pointer to OT mem
				                     gGT->pushBuffer_UI.ptrOT, colorPtr[0], colorPtr[1], colorPtr[2], colorPtr[3], 0, MM_HIGHSCORE_GHOST_STAR_SCALE);
			}
		}

		gGT->levelID = prevLevelID;
		GAMEPROG_GetPtrHighScoreTrack();

		MM_HighScore_Text3D(sdata->lngStrings[LNG_BEST_LAP_TIME], bestLapLabelMeta->currX + offsetX + MM_HIGHSCORE_BEST_LAP_LABEL_X_OFFSET,
		                    bestLapLabelMeta->currY + offsetY + MM_HIGHSCORE_BEST_LAP_LABEL_Y_OFFSET, FONT_SMALL, 0);

		// Character Name
		MM_HighScore_Text3D(entry[0].name, bestLapEntryMeta->currX + offsetX + MM_HIGHSCORE_BEST_LAP_TEXT_X_OFFSET,
		                    bestLapEntryMeta->currY + offsetY + MM_HIGHSCORE_BEST_LAP_NAME_Y_OFFSET, FONT_BIG,
		                    entry[0].characterID + MM_HIGHSCORE_DRIVER_COLOR_OFFSET);

		// Draw time string
		// NOTE(aalhendi): Retail also uses currX as the Y transition base here.
		MM_HighScore_Text3D(RECTMENU_DrawTime(entry[0].time), bestLapEntryMeta->currX + offsetX + MM_HIGHSCORE_BEST_LAP_TEXT_X_OFFSET,
		                    bestLapEntryMeta->currX + offsetY + MM_HIGHSCORE_BEST_LAP_TIME_Y_OFFSET, FONT_SMALL, 0);

		// Character Icon
		RECTMENU_DrawPolyGT4(gGT->ptrIcons[data.MetaDataCharacters[entry[0].characterID].iconID],
		                     bestLapEntryMeta->currX + offsetX + MM_HIGHSCORE_BEST_LAP_ICON_X_OFFSET,
		                     bestLapEntryMeta->currY + offsetY + MM_HIGHSCORE_BEST_LAP_ICON_Y_OFFSET, &gGT->backBuffer->primMem, (gGT->pushBuffer_UI).ptrOT,
		                     iconColor.self, iconColor.self, iconColor.self, iconColor.self, MM_HIGHSCORE_ICON_TRANSPARENCY, MM_HIGHSCORE_ICON_SCALE);
	}

	// Draw five "best track times"
	// Icon, Name, and Time
	for (s32 scoreRowIndex = 0; scoreRowIndex < MM_HIGHSCORE_VISIBLE_SCORE_ROWS; scoreRowIndex++)
	{
		s32 entryIndex = scoreRowIndex + MM_HIGHSCORE_FIRST_VISIBLE_ENTRY;
		s32 metaIndex = scoreRowIndex + MM_HIGHSCORE_FIRST_VISIBLE_META_INDEX;

		// Character Icon
		RECTMENU_DrawPolyGT4(gGT->ptrIcons[data.MetaDataCharacters[entry[entryIndex].characterID].iconID],
		                     D230.transitionMeta_HighScores[metaIndex].currX + offsetX + MM_HIGHSCORE_SCORE_ICON_X_OFFSET,
		                     D230.transitionMeta_HighScores[metaIndex].currY + offsetY + (scoreRowIndex * MM_HIGHSCORE_SCORE_ROW_Y_STEP) +
		                         MM_HIGHSCORE_SCORE_NAME_Y_OFFSET,
		                     &gGT->backBuffer->primMem, gGT->pushBuffer_UI.ptrOT, iconColor.self, iconColor.self, iconColor.self, iconColor.self,
		                     MM_HIGHSCORE_ICON_TRANSPARENCY, MM_HIGHSCORE_ICON_SCALE);

		// draw the name string
		MM_HighScore_Text3D(entry[entryIndex].name, D230.transitionMeta_HighScores[metaIndex].currX + offsetX + MM_HIGHSCORE_SCORE_NAME_X_OFFSET,
		                    D230.transitionMeta_HighScores[metaIndex].currY + offsetY + (scoreRowIndex * MM_HIGHSCORE_SCORE_ROW_Y_STEP) +
		                        MM_HIGHSCORE_SCORE_NAME_Y_OFFSET,
		                    FONT_BIG, entry[entryIndex].characterID + MM_HIGHSCORE_DRIVER_COLOR_OFFSET);

		// draw the Time string
		MM_HighScore_Text3D(
		    RECTMENU_DrawTime(entry[entryIndex].time), D230.transitionMeta_HighScores[metaIndex].currX + offsetX + MM_HIGHSCORE_SCORE_NAME_X_OFFSET,
		    D230.transitionMeta_HighScores[metaIndex].currY + offsetY + (scoreRowIndex * MM_HIGHSCORE_SCORE_ROW_Y_STEP) + MM_HIGHSCORE_SCORE_TIME_Y_OFFSET,
		    FONT_SMALL, 0);
	}

	RECT videoBox;
	videoBox.w = MM_HIGHSCORE_VIDEO_BOX_W;
	videoBox.h = MM_HIGHSCORE_VIDEO_BOX_H;
	videoBox.x = D230.transitionMeta_HighScores[MM_HIGHSCORE_VIDEO_META_INDEX].currX + offsetX + MM_HIGHSCORE_VIDEO_BOX_X_OFFSET;
	videoBox.y = D230.transitionMeta_HighScores[MM_HIGHSCORE_VIDEO_META_INDEX].currY + offsetY + MM_HIGHSCORE_VIDEO_BOX_Y_OFFSET;

	MM_TrackSelect_Video_Draw(&videoBox, &D230.arcadeTracks[0], trackIndex, (D230.highScoreTransition.state == EXITING_MENU), 0);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b3914-0x800b3954.
void MM_HighScore_Init(void)
{
	D230.highScoreTransition.state = ENTERING_MENU;
	D230.highScoreTransition.mainFrame = MM_HIGHSCORE_MAIN_TRANSITION_MAX_FRAME;
	D230.highScoreSelection.targetRow = 0;
	D230.highScoreSelection.currentRow = 0;

	// reset all video variables
	MM_TrackSelect_Video_SetDefaults();
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 overlay 230 0x800b3954-0x800b3fe4.
void MM_HighScore_MenuProc(struct RectMenu *menu_unused)
{
	(void)menu_unused;
	s16 nextFrameCount;
	RECT wipeRect;

	b32 videoResetRequested = false;
	if (D230.highScoreTransition.state != IN_MENU)
	{
		nextFrameCount = D230.highScoreTransition.mainFrame;
		if (D230.highScoreTransition.state < EXITING_MENU) // entering_menu, in_menu
		{
			if (D230.highScoreTransition.state == ENTERING_MENU)
			{
				MM_TransitionInOut(D230.transitionMeta_HighScores, (int)D230.highScoreTransition.mainFrame, MM_HIGHSCORE_SLIDE_TRANSITION_FRAMES);
				nextFrameCount = D230.highScoreTransition.mainFrame + -1;
				if (D230.highScoreTransition.mainFrame == 0)
				{
					D230.highScoreTransition.state = IN_MENU;
					nextFrameCount = D230.highScoreTransition.mainFrame;
				}
			}
		}
		else if (((D230.highScoreTransition.state == EXITING_MENU) && (D230.highScoreTransition.trackFrame == 0)) && (D230.highScoreTransition.rowFrame == 0))
		{
			MM_TransitionInOut(D230.transitionMeta_HighScores, (int)D230.highScoreTransition.mainFrame, MM_HIGHSCORE_SLIDE_TRANSITION_FRAMES);
			D230.highScoreTransition.mainFrame = D230.highScoreTransition.mainFrame + 1;
			nextFrameCount = D230.highScoreTransition.mainFrame;
			if (MM_HIGHSCORE_MAIN_TRANSITION_MAX_FRAME < D230.highScoreTransition.mainFrame)
			{
				MM_JumpTo_Title_Returning();
				return;
			}
		}
		D230.highScoreTransition.mainFrame = nextFrameCount;
		if (D230.highScoreTransition.state != IN_MENU)
		{
			goto LAB_OVR_230__800b3c78;
		}
	}
	if ((sdata->buttonTapPerPlayer[0] & BTN_UP) == 0)
	{
		if (((sdata->buttonTapPerPlayer[0] & BTN_DOWN) != 0) && (D230.menuHighScore.rowSelected < 1))
		{
			videoResetRequested = true;
		}
	}
	else if (D230.menuHighScore.rowSelected == 1)
	{
		videoResetRequested = true;
	}
	// if player didn't press any of the "back" buttons
	if ((sdata->buttonTapPerPlayer[0] & (BTN_SQUARE_one | BTN_TRIANGLE)) == 0)
	{
		if ((sdata->buttonTapPerPlayer[0] & BTN_LEFT) == 0)
		{
			if ((sdata->buttonTapPerPlayer[0] & BTN_RIGHT) == 0)
			{
				s32 menuResult = RECTMENU_ProcessInput(&D230.menuHighScore);
				if ((s16)menuResult == -1)
				{
					D230.highScoreTransition.state = EXITING_MENU;
				}
				else if (((s16)menuResult == 1) && (D230.menuHighScore.rowSelected == 2))
				{
					D230.highScoreTransition.state = D230.menuHighScore.rowSelected;
				}
				if (((u16)D230.menuHighScore.rowSelected < 2) && (D230.highScoreSelection.targetRow != D230.menuHighScore.rowSelected))
				{
					D230.highScoreTransition.pendingVerticalMove = -1;
					if (D230.menuHighScore.rowSelected != 0)
					{
						D230.highScoreTransition.pendingVerticalMove = 1;
					}
					D230.highScoreSelection.targetRow = D230.menuHighScore.rowSelected;
				}
			}
			else
			{
				videoResetRequested = true;
				D230.highScoreTransition.pendingHorizontalMove = 1;
				b32 trackOpen;
				do
				{
					D230.highScoreSelection.targetTrack = D230.highScoreSelection.targetTrack + 1;
					if (MM_HIGHSCORE_LAST_ARCADE_TRACK < D230.highScoreSelection.targetTrack)
					{
						D230.highScoreSelection.targetTrack = 0;
					}
					trackOpen = MM_TrackSelect_boolTrackOpen(D230.arcadeTracks + D230.highScoreSelection.targetTrack);
				} while (!trackOpen);
			}
		}
		else
		{
			videoResetRequested = true;
			D230.highScoreTransition.pendingHorizontalMove = -1;
			b32 trackOpen;
			do
			{
				D230.highScoreSelection.targetTrack = D230.highScoreSelection.targetTrack - 1;
				if (D230.highScoreSelection.targetTrack < 0)
				{
					D230.highScoreSelection.targetTrack = MM_HIGHSCORE_LAST_ARCADE_TRACK;
				}
				trackOpen = MM_TrackSelect_boolTrackOpen(D230.arcadeTracks + D230.highScoreSelection.targetTrack);
			} while (!trackOpen);
		}
	}
	else
	{
		videoResetRequested = true;
		// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b3ad8-0x800b3ae4 for high-score back SFX.
		OtherFX_Play(2, 1);
		D230.highScoreTransition.state = EXITING_MENU;
	}

LAB_OVR_230__800b3c78:

{
	b32 videoState = (((videoResetRequested) || (D230.highScoreTransition.trackFrame != 0)) || (D230.highScoreTransition.rowFrame != 0)) ||
	                 (D230.highScoreTransition.state == EXITING_MENU);
	MM_TrackSelect_Video_State(videoState);
}
	nextFrameCount = D230.highScoreTransition.trackFrame + -1;
	if (D230.highScoreTransition.trackFrame == 0)
	{
		nextFrameCount = D230.highScoreTransition.rowFrame + -1;
		if (D230.highScoreTransition.rowFrame == 0)
		{
			if (D230.highScoreSelection.currentTrack == D230.highScoreSelection.targetTrack)
			{
				if (D230.highScoreSelection.currentRow != D230.highScoreSelection.targetRow)
				{
					D230.highScoreTransition.rowFrame = MM_HIGHSCORE_SLIDE_TRANSITION_FRAMES;
					D230.highScoreTransition.activeVerticalMove = D230.highScoreTransition.pendingVerticalMove;
				}
			}
			else
			{
				D230.highScoreTransition.trackFrame = MM_HIGHSCORE_SLIDE_TRANSITION_FRAMES;
				D230.highScoreTransition.activeHorizontalMove = D230.highScoreTransition.pendingHorizontalMove;
			}
		}
		else
		{
			u8 slideReachedTarget = D230.highScoreTransition.rowFrame == 1;
			D230.highScoreTransition.rowFrame = nextFrameCount;
			if (slideReachedTarget)
			{
				D230.highScoreSelection.currentRow = D230.highScoreSelection.targetRow;
			}
		}
	}
	else
	{
		u8 slideReachedTarget = D230.highScoreTransition.trackFrame == 1;
		D230.highScoreTransition.trackFrame = nextFrameCount;
		if (slideReachedTarget)
		{
			D230.highScoreSelection.currentTrack = D230.highScoreSelection.targetTrack;
		}
	}

	RECTMENU_DrawSelf(&D230.menuHighScore, D230.transitionMeta_HighScores[MM_HIGHSCORE_MENU_META_INDEX].currX,
	                  D230.transitionMeta_HighScores[MM_HIGHSCORE_MENU_META_INDEX].currY, MM_HIGHSCORE_MENU_WIDTH);

	s32 currOffsetY = 0;
	s32 currOffsetX = 0;

	if (D230.highScoreTransition.trackFrame == 0)
	{
		currOffsetY = (MM_HIGHSCORE_SLIDE_TRANSITION_FRAMES - D230.highScoreTransition.rowFrame) * D230.highScoreTransition.activeVerticalMove *
		              MM_HIGHSCORE_ROW_SLIDE_STEP_Y;
	}

	else
	{
		currOffsetX = (MM_HIGHSCORE_SLIDE_TRANSITION_FRAMES - D230.highScoreTransition.trackFrame) * D230.highScoreTransition.activeHorizontalMove *
		              MM_HIGHSCORE_TRACK_SLIDE_STEP_X;
	}

	u32 *ot = sdata->gGT->backBuffer->otMem.uiOT;
	const struct TransitionMeta *titleMeta = &D230.transitionMeta_HighScores[MM_HIGHSCORE_TITLE_META_INDEX];

	if (((currOffsetX != -MM_HIGHSCORE_OFFSCREEN_X) && (currOffsetX != MM_HIGHSCORE_OFFSCREEN_X)) &&
	    ((currOffsetY != -MM_HIGHSCORE_OFFSCREEN_Y && (currOffsetY != MM_HIGHSCORE_OFFSCREEN_Y))))
	{
		MM_HighScore_Draw(D230.highScoreSelection.currentTrack, (int)D230.highScoreSelection.currentRow, (int)(s16)currOffsetX, (int)(s16)currOffsetY);
		if (D230.highScoreTransition.rowFrame != 0)
		{
			// draw rectangle
			wipeRect.w = MM_HIGHSCORE_WIPE_RECT_W;
			wipeRect.h = MM_HIGHSCORE_WIPE_RECT_H;
			wipeRect.x = titleMeta->currX + MM_HIGHSCORE_WIPE_RECT_X_OFFSET;
			wipeRect.y = titleMeta->currY + (s16)currOffsetY + MM_HIGHSCORE_WIPE_RECT_Y_OFFSET;
			RECTMENU_DrawInnerRect(&wipeRect, 0, ot);
		}
	}
	s32 nextOffsetX = 0;
	s32 nextOffsetY = 0;
	if (D230.highScoreTransition.trackFrame == 0)
	{
		nextOffsetY = D230.highScoreTransition.rowFrame * -MM_HIGHSCORE_ROW_SLIDE_STEP_Y * (int)D230.highScoreTransition.activeVerticalMove;
	}
	else
	{
		nextOffsetX = D230.highScoreTransition.trackFrame * -MM_HIGHSCORE_TRACK_SLIDE_STEP_X * (int)D230.highScoreTransition.activeHorizontalMove;
	}
	if (((currOffsetX != nextOffsetX) || (currOffsetY != nextOffsetY)) &&
	    ((nextOffsetX != -MM_HIGHSCORE_OFFSCREEN_X &&
	      (((nextOffsetX != MM_HIGHSCORE_OFFSCREEN_X && (nextOffsetY != -MM_HIGHSCORE_OFFSCREEN_Y)) && (nextOffsetY != MM_HIGHSCORE_OFFSCREEN_Y))))))
	{
		MM_HighScore_Draw(D230.highScoreSelection.targetTrack, (int)D230.highScoreSelection.targetRow, (int)(s16)nextOffsetX, (int)(s16)nextOffsetY);
	}

	// draw rectangle
	wipeRect.w = MM_HIGHSCORE_WIPE_RECT_W;
	wipeRect.h = MM_HIGHSCORE_WIPE_RECT_H;
	wipeRect.x = titleMeta->currX + MM_HIGHSCORE_WIPE_RECT_X_OFFSET;
	wipeRect.y = titleMeta->currY + (s16)nextOffsetY + MM_HIGHSCORE_WIPE_RECT_Y_OFFSET;
	RECTMENU_DrawInnerRect(&wipeRect, 0, ot);

	return;
}
