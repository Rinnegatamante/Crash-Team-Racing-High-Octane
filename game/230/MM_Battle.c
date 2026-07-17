#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800abaa8-0x800abaf0.
void MM_Battle_DrawIcon_Character(struct Icon *icon, int posX, int posY, struct PrimMem *primMem, uint32_t *ot, char transparency, s16 scale)
{
	if (icon == 0)
	{
		return;
	}
	DecalHUD_DrawPolyFT4(icon, posX, posY, primMem, ot, transparency, scale);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b164c-0x800b1660.
void MM_Battle_CloseSubMenu(struct RectMenu *menu)
{
	menu->state |= ONLY_DRAW_TITLE;
}

enum
{
	BATTLE_TRANSITION_FRAME_COUNT = 0xc,
	BATTLE_ANIMATED_TRANSITION_META_COUNT = 8,
	BATTLE_SETTINGS_COUNT = 5,
	BATTLE_TEAM_COUNT = 4,
	BATTLE_VALID_TEAM_MIN = 0,
	BATTLE_VALID_TEAM_MAX = BATTLE_TEAM_COUNT - 1,
	BATTLE_ROW_TYPE = 0,
	BATTLE_ROW_LENGTH = 1,
	BATTLE_ROW_TEAMS = 2,
	BATTLE_ROW_WEAPON_TOP = 3,
	BATTLE_ROW_WEAPON_BOTTOM = 4,
	BATTLE_ROW_START = 5,
	BATTLE_ROW_LIFE_COUNT = 10,
	BATTLE_WEAPON_ROW_COUNT = 2,
	BATTLE_WEAPONS_PER_ROW = 6,
	BATTLE_WEAPON_ITEM_COUNT = 11,
	BATTLE_WEAPON_FLAG_COUNT = 0xe,
	BATTLE_REQUIRED_WEAPON_FLAGS = 0xcde,
	BATTLE_CONFIRM_INPUT = BTN_CROSS_one | BTN_CIRCLE,
	BATTLE_BACK_INPUT = BTN_SQUARE_one | BTN_TRIANGLE,
	BATTLE_MENU_INPUT = BTN_UP | BTN_DOWN | BTN_LEFT | BTN_RIGHT | BATTLE_CONFIRM_INPUT | BATTLE_BACK_INPUT,
	BATTLE_TIME_LIMIT_TYPE_ROW = 1,
	BATTLE_LIFE_LIMIT_TYPE_ROW = 2,
	BATTLE_MINUTES_TO_EVENT_TIME = 0xe100,
	BATTLE_INACTIVE_TEAM_POINTS = -500,
	BATTLE_ICON_SCALE = 0x1000,
	BATTLE_WEAPON_ICON_ROTATE_RIGHT = 1,
	BATTLE_WEAPON_DISABLED_TEXT_COLOR = 0x15,
	BATTLE_WEAPON_ENABLED_TEXT_COLOR = 4,
	BATTLE_TITLE_META_INDEX = 9,
	BATTLE_TITLE_X_OFFSET = 0x100,
	BATTLE_TITLE_Y_OFFSET = 10,
	BATTLE_LABEL_X_OFFSET = 0x8c,
	BATTLE_MENU_X_OFFSET = 0x9c,
	BATTLE_ROW_TYPE_MENU_META_INDEX = 0,
	BATTLE_ROW_TYPE_LABEL_META_INDEX = 1,
	BATTLE_ROW_LENGTH_MENU_META_INDEX = 2,
	BATTLE_ROW_LENGTH_LABEL_META_INDEX = 3,
	BATTLE_ROW_TEAM_META_INDEX = 4,
	BATTLE_ROW_TEAM_LABEL_META_INDEX = 5,
	BATTLE_ROW_WEAPON_PANEL_META_INDEX = 6,
	BATTLE_ROW_WEAPON_LABEL_META_INDEX = 7,
	BATTLE_ROW_START_META_INDEX = 8,
	BATTLE_TYPE_ROW_Y_OFFSET = 0x24,
	BATTLE_MENU_DEFAULT_HEIGHT = 0xd,
	BATTLE_LENGTH_ROW_Y_OFFSET = 0x20,
	BATTLE_LENGTH_LABEL_Y_OFFSET = 4,
	BATTLE_LENGTH_DUAL_MENU_X_OFFSET = 0x142,
	BATTLE_LENGTH_DUAL_MENU_WIDTH = 0x8e,
	BATTLE_WIDE_MENU_WIDTH = 0x134,
	BATTLE_TEAM_PANEL_START_X = 0x9f,
	BATTLE_TEAM_LABEL_Y_OFFSET = 10,
	BATTLE_TEAM_SEGMENT_BASE_WIDTH = 4,
	BATTLE_TEAM_PLAYER_WIDTH = 0x2a,
	BATTLE_TEAM_PANEL_TOTAL_WIDTH = 0x12e,
	BATTLE_TEAM_PLAYER_CENTER_OFFSET = -0x15,
	BATTLE_TEAM_PLAYER_ICON_Y_OFFSET = 6,
	BATTLE_TEAM_COLOR_H = 0x1a,
	BATTLE_TEAM_COLOR_Y_OFFSET = 5,
	BATTLE_TEAM_HIGHLIGHT_W = 0x134,
	BATTLE_TEAM_HIGHLIGHT_H = 0x1e,
	BATTLE_TEAM_HIGHLIGHT_Y_OFFSET = 3,
	BATTLE_TEAM_PANEL_W = 0x140,
	BATTLE_TEAM_PANEL_H = 0x24,
	BATTLE_TEAM_PANEL_X_OFFSET = 0x96,
	BATTLE_WEAPON_LABEL_Y_OFFSET = 0x44,
	BATTLE_ERROR_COLOR_FRAME_BIT = 1,
	BATTLE_ERROR_COLOR_A = -0x7fff,
	BATTLE_ERROR_COLOR_B = -0x7ffd,
	BATTLE_ERROR_TEXT_X = 0x100,
	BATTLE_ERROR_TEXT_LINE_1_Y_OFFSET = 0x6a,
	BATTLE_ERROR_TEXT_LINE_2_Y_OFFSET = 0x7a,
	BATTLE_START_MENU_Y_OFFSET = 0x78,
	BATTLE_WEAPON_PANEL_W = 0x140,
	BATTLE_WEAPON_PANEL_H = 0x44,
	BATTLE_WEAPON_PANEL_X_OFFSET = 0x96,
	BATTLE_WEAPON_PANEL_Y_OFFSET = 0x2a,
	BATTLE_WEAPON_GRID_X_OFFSET = 6,
	BATTLE_WEAPON_GRID_Y_OFFSET = 2,
	BATTLE_WEAPON_GRID_X_STEP = 0x34,
	BATTLE_WEAPON_GRID_ROW_STAGGER_X = 0x1a,
	BATTLE_WEAPON_GRID_Y_STEP = 0x20,
	BATTLE_WEAPON_AMMO_TEXT_FIRST = 7,
	BATTLE_WEAPON_AMMO_TEXT_COUNT = 2,
	BATTLE_WEAPON_HIGHLIGHT_LEFT_X_OFFSET = 4,
	BATTLE_WEAPON_HIGHLIGHT_RIGHT_X_OFFSET = 0x1e,
	BATTLE_WEAPON_HIGHLIGHT_W = 0x34,
	BATTLE_WEAPON_HIGHLIGHT_H = 0x20,
	BATTLE_WEAPON_HIGHLIGHT_Y_OFFSET = 2,
	BATTLE_WEAPON_PANEL_INSET_X = 3,
	BATTLE_WEAPON_PANEL_INSET_Y = 2,
	BATTLE_WEAPON_PANEL_INSET_W_SHRINK = 6,
	BATTLE_WEAPON_PANEL_INSET_H_SHRINK = 4,
	BATTLE_LABEL_TEXT_FLAGS = 0x4000,
};

enum
{
	BATTLE_COLOR_RGB_MASK = 0xffffffu,
	BATTLE_GPU_CODE_POLY_FT4 = 0x2c000000u,
	BATTLE_GPU_CODE_POLY_FT4_SEMI_TRANS = 0x2e000000u,
	BATTLE_GPU_TAG_LENGTH_POLY_FT4 = 0x09000000u,
	BATTLE_TPAGE_TRANSPARENCY_MASK = 0xff9fffffu,
	BATTLE_TPAGE_TRANSPARENCY_SHIFT = 0x15,
};

#define BATTLE_TITLE_TEXT_FLAGS 0xffff8000u

// NOTE(aalhendi): ASM-verified NTSC-U 926 overlay 230 0x800b1660-0x800b1830.
void MM_Battle_DrawIcon_Weapon(struct Icon *icon, u32 posX, int posY, struct PrimMem *primMem, u32 *ot, char transparency, s16 scale, u16 rotation,
                               const Color *color)
{
	if (!icon)
	{
		return;
	}

	POLY_FT4 *p = (POLY_FT4 *)primMem->cursor;

	u32 uv0 = CTR_ReadU32LE(&icon->texLayout.u0);
	u32 uv1 = CTR_ReadU32LE(&icon->texLayout.u1);
	u32 uv2 = CTR_ReadU32LE(&icon->texLayout.u2);
	s32 scaledWidth = (((s32)((u8)icon->texLayout.u1 - (u8)icon->texLayout.u0)) * scale) >> 0xc;
	s32 scaledHeight = (((s32)((u8)icon->texLayout.v2 - (u8)icon->texLayout.v0)) * scale) >> 0xc;
	u32 code = BATTLE_GPU_CODE_POLY_FT4;
	u32 packedY = (u32)(u16)posY << 0x10;

	if ((u8)transparency != 0)
	{
		code = BATTLE_GPU_CODE_POLY_FT4_SEMI_TRANS;
		uv1 = (uv1 & BATTLE_TPAGE_TRANSPARENCY_MASK) | ((((u32)(u8)transparency - 1) << BATTLE_TPAGE_TRANSPARENCY_SHIFT));
	}

	CtrGpu_WriteColorCode(&p->r0, (color->self & BATTLE_COLOR_RGB_MASK) | code);
	CtrGpu_WritePackedUVWord(&p->u0, uv0);
	CtrGpu_WritePackedUVWord(&p->u1, uv1);
	CtrGpu_WritePackedUV(&p->u2, (u16)uv2);
	CtrGpu_WritePackedUV(&p->u3, (u16)(uv2 >> 0x10));

	if ((rotation & 1) != 0)
	{
		u32 sidewaysX = posX + scaledHeight;
		u32 packedSideY = packedY + ((u32)scaledWidth << 0x10);

		if ((s16)rotation == BATTLE_WEAPON_ICON_ROTATE_RIGHT)
		{
			CtrGpu_WritePackedXY(&p->x1, posX | packedY);
			CtrGpu_WritePackedXY(&p->x3, sidewaysX | packedY);
			CtrGpu_WritePackedXY(&p->x0, posX | packedSideY);
			CtrGpu_WritePackedXY(&p->x2, sidewaysX | packedSideY);
		}
		else
		{
			CtrGpu_WritePackedXY(&p->x2, posX | packedY);
			CtrGpu_WritePackedXY(&p->x0, sidewaysX | packedY);
			CtrGpu_WritePackedXY(&p->x3, posX | packedSideY);
			CtrGpu_WritePackedXY(&p->x1, sidewaysX | packedSideY);
		}
	}
	else
	{
		u32 rightX = posX + scaledWidth;
		u32 packedBottomY = packedY + ((u32)scaledHeight << 0x10);

		if (((u32)rotation << 0x10) == 0)
		{
			CtrGpu_WritePackedXY(&p->x0, posX | packedY);
			CtrGpu_WritePackedXY(&p->x1, rightX | packedY);
			CtrGpu_WritePackedXY(&p->x2, posX | packedBottomY);
			CtrGpu_WritePackedXY(&p->x3, rightX | packedBottomY);
		}
		else
		{
			CtrGpu_WritePackedXY(&p->x3, posX | packedY);
			CtrGpu_WritePackedXY(&p->x2, rightX | packedY);
			CtrGpu_WritePackedXY(&p->x1, posX | packedBottomY);
			CtrGpu_WritePackedXY(&p->x0, rightX | packedBottomY);
		}
	}

	CtrGpu_LinkPacket24(ot, &p->tag, p, BATTLE_GPU_TAG_LENGTH_POLY_FT4);

	primMem->cursor = p + 1;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b1830-0x800b1848.
void MM_Battle_Init(void)
{
	D230.battleTransition.frame = BATTLE_TRANSITION_FRAME_COUNT;
	D230.battleTransition.state = ENTERING_MENU;
}

void MM_Battle_MenuProc(struct RectMenu *unused)
{
	(void)unused;
	u16 teamSegmentWidths[BATTLE_TEAM_COUNT];
	s16 teamPlayerCounts[BATTLE_TEAM_COUNT];

	struct GameTracker *gGT = sdata->gGT;

	// save all five battle settings,
	// these are selected rows from all battle options
	for (s32 settingIndex = 0; settingIndex < BATTLE_SETTINGS_COUNT; settingIndex++)
	{
		D230.battleMenuArray[settingIndex]->rowSelected = sdata->battleSettings[settingIndex];
	}

	s16 nextTransitionFrames = D230.battleTransition.frame;
	if (D230.battleTransition.state != IN_MENU)
	{
		if ((s16)D230.battleTransition.state < 2)
		{
			// if transitioning in
			if (D230.battleTransition.state == ENTERING_MENU)
			{
				MM_TransitionInOut(D230.transitionMeta_battle, (int)D230.battleTransition.frame, BATTLE_ANIMATED_TRANSITION_META_COUNT);

				// reduce frames
				nextTransitionFrames = D230.battleTransition.frame - 1;

				// if finished
				if (D230.battleTransition.frame == 0)
				{
					// menu is now in focus
					D230.battleTransition.state = IN_MENU;
					nextTransitionFrames = D230.battleTransition.frame;
				}
			}
		}
		else
		{
			// if transitioning out
			if (D230.battleTransition.state == EXITING_MENU)
			{
				MM_TransitionInOut(D230.transitionMeta_battle, (int)D230.battleTransition.frame, BATTLE_ANIMATED_TRANSITION_META_COUNT);

				// count frames
				D230.battleTransition.frame++;

				nextTransitionFrames = D230.battleTransition.frame;

				// if 12 frames past
				if (BATTLE_TRANSITION_FRAME_COUNT < D230.battleTransition.frame)
				{
					// if starting race
					if (D230.battleTransition.startAfterExit != 0)
					{
						// passthrough Menu for funcPtr "QueueLoadTrack"
						sdata->ptrDesiredMenu = &data.menuQueueLoadTrack;
						return;
					}

					// == else goBack ==

					MM_TrackSelect_Init();
					sdata->ptrDesiredMenu = &D230.menuTrackSelect;

					return;
				}
			}
		}
	}
	D230.battleTransition.frame = nextTransitionFrames;

	// There are no battle teams (clear flags)
	gGT->battleSetup.teamFlags = 0;

	// there are no battle teams (clear amount of teams)
	gGT->battleSetup.numTeams = 0;

	u8 numPlyr = gGT->numPlyrNextGame;

	// loop through all players
	for (s32 playerIndex = 0; playerIndex < numPlyr; playerIndex++)
	{
		// get the team of each player
		u32 teamFlag = (s16)(1 << gGT->battleSetup.teamOfEachPlayer[playerIndex]);

		// If we have not accounted for this team existing
		if ((gGT->battleSetup.teamFlags & teamFlag) == 0)
		{
			// This team now exists
			gGT->battleSetup.teamFlags |= teamFlag;

			// increase number of teams
			gGT->battleSetup.numTeams++;
		}
	}

	// Reset team points
	for (s32 teamIndex = 0; teamIndex < BATTLE_TEAM_COUNT; teamIndex++)
	{
		if ((gGT->battleSetup.teamFlags & (1 << teamIndex)) == 0)
		{
			gGT->battleSetup.pointsPerTeam[teamIndex] = BATTLE_INACTIVE_TEAM_POINTS;
		}
		else
		{
			gGT->battleSetup.pointsPerTeam[teamIndex] = 0;
		}
	}

	// Related to Battle mode
	if ((
	        // If number of teams is less than 2
	        ((gGT->battleSetup.numTeams) < 2) ||

	        // If no weapons are slected
	        ((gGT->battleSetup.enabledWeapons & BATTLE_REQUIRED_WEAPON_FLAGS) == 0)) &&

	    // If you are hovering over row 5 (Start Battle)
	    (sdata->battleSetupRowHighlighted == BATTLE_ROW_START))
	{
		// Move cursor back to row 4 (in weapons selection)
		sdata->battleSetupRowHighlighted = BATTLE_ROW_WEAPON_BOTTOM;
	}

	for (s32 playerIndex = 0; playerIndex < numPlyr; playerIndex++)
	{
		// If you are selecting "Teams" row
		if (sdata->battleSetupRowHighlighted == BATTLE_ROW_TEAMS)
		{
			// If you press Left on D-Pad or move stick to the Left
			if ((sdata->buttonTapPerPlayer[playerIndex] & BTN_LEFT) != 0)
			{
				// If you have room to move left
				// if your team number is more than 0
				if (BATTLE_VALID_TEAM_MIN < gGT->battleSetup.teamOfEachPlayer[playerIndex])
				{
					// play sound
					// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b1b54-0x800b1b6c for battle team-left SFX.
					OtherFX_Play(0, 1);

					// Move your icon to the left
					gGT->battleSetup.teamOfEachPlayer[playerIndex]--;
				}

				// clear the gamepad input so that it
				// does not use this frame's input on the next frame
				sdata->buttonTapPerPlayer[playerIndex] = 0;
			}

			// If you press Right on D-Pad or move stick to the Right
			if ((sdata->buttonTapPerPlayer[playerIndex] & BTN_RIGHT) != 0)
			{
				// If there is room to move right,
				// If your team number is less than 3
				if (gGT->battleSetup.teamOfEachPlayer[playerIndex] < BATTLE_VALID_TEAM_MAX)
				{
					// play sound
					// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b1bc0-0x800b1bd8 for battle team-right SFX.
					OtherFX_Play(0, 1);

					// Move your icon to the right
					gGT->battleSetup.teamOfEachPlayer[playerIndex]++;
				}

				// clear the gamepad input so that it
				// does not use this frame's input on the next frame
				sdata->buttonTapPerPlayer[playerIndex] = 0;
			}
		}
	}

	// make a copy of the row you have highlighted
	s16 previousHighlightedRow = sdata->battleSetupRowHighlighted;

	if ((D230.battleTransition.state == IN_MENU) &&

	    // If you press D-pad or Cross, Square, Triangle, Circle
	    ((sdata->buttonTapPerPlayer[0] & BATTLE_MENU_INPUT) != 0))
	{
		// if you are not in any drop-down menu
		if ((s16)sdata->battleSetupExpandMenu < 0)
		{
			int buttonTapP1 = sdata->buttonTapPerPlayer[0];

			// If you dont press Up
			if ((buttonTapP1 & BTN_UP) == 0)
			{
				// If you dont press Down
				if ((buttonTapP1 & BTN_DOWN) == 0)
				{
					// If you dont press Left
					if ((buttonTapP1 & BTN_LEFT) == 0)
					{
						// If you dont press Right
						if ((buttonTapP1 & BTN_RIGHT) == 0)
						{
							// If you dont press Cross or Circle
							if ((buttonTapP1 & BATTLE_CONFIRM_INPUT) == 0)
							{
								// If you press Square or Trianlge
								if ((buttonTapP1 & BATTLE_BACK_INPUT) != 0)
								{
									// Play "Go Back" sound
									// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b20f8-0x800b2110 for battle setup back SFX.
									OtherFX_Play(2, 1);

									// go back when transition is done, dont start race
									D230.battleTransition.startAfterExit = 0;

									// start transition out
									D230.battleTransition.state = EXITING_MENU;
								}
							}

							// If you press Cross or Circle
							else
							{
								// Play sound
								// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b1de4-0x800b1df4 for battle setup confirm SFX.
								OtherFX_Play(1, 1);

								switch (sdata->battleSetupRowHighlighted)
								{
									// If row selected can be
									// expanded (type, length, lives)
								case BATTLE_ROW_TYPE:
								case BATTLE_ROW_LENGTH:
								case BATTLE_ROW_LIFE_COUNT:
									// Row expanded = row selected
									sdata->battleSetupExpandMenu = sdata->battleSetupRowHighlighted;
									break;

									// If row selected is 3 or 4
								case BATTLE_ROW_WEAPON_TOP:
								case BATTLE_ROW_WEAPON_BOTTOM:
								{
									// Enable or disable a weapon when you click it
									int weaponIndex =
									    (int)sdata->battleSetupWeaponHighlighted +
									    ((int)(((u32)sdata->battleSetupRowHighlighted - BATTLE_ROW_WEAPON_TOP) * 0x10000) >> 0x10) * BATTLE_WEAPONS_PER_ROW;
									gGT->battleSetup.enabledWeapons ^= D230.battleWeaponItems[weaponIndex].enabledWeaponFlag;
									break;
								}

									// If row selected is "start battle",
									// all code below is for starting the battle
								case BATTLE_ROW_START:

									// get game mode, minus some flags
									gGT->gameMode1 &= ~(POINT_LIMIT | LIFE_LIMIT | TIME_LIMIT);

									u32 battleModeFlags = D230.battleSetupTables.typeModeFlags[D230.menuBattleType.rowSelected];
									gGT->gameMode1 |= battleModeFlags;

									if ((battleModeFlags & TIME_LIMIT) != 0)
									{
										// point limit
										gGT->gameMode1 |= POINT_LIMIT;
									}

									if (((gGT->gameMode1 & LIFE_LIMIT) != 0) &&
									    (0 < D230.battleSetupTables.lifeModeTimeLimitMinutes[D230.menuBattleLengthLifeTime.rowSelected]))
									{
										// time limit
										gGT->gameMode1 |= TIME_LIMIT;
									}

									// set kill limit
									gGT->battleSetup.killLimit = D230.battleSetupTables.pointLimitValues[D230.menuBattleLengthPoints.rowSelected];

									// if time limit
									s32 eventTimeMinutes;
									if (D230.menuBattleType.rowSelected == BATTLE_TIME_LIMIT_TYPE_ROW)
									{
										eventTimeMinutes = D230.battleSetupTables.timeLimitMinutes[D230.menuBattleLengthTimeTime.rowSelected];
									}

									else
									{
										eventTimeMinutes = D230.battleSetupTables.lifeModeTimeLimitMinutes[D230.menuBattleLengthLifeTime.rowSelected];
									}

									// set time limit based on number of minutes
									gGT->originalEventTime = eventTimeMinutes;
									s32 originalEventTime = gGT->originalEventTime;
									if (0 < originalEventTime)
									{
										gGT->originalEventTime = originalEventTime * BATTLE_MINUTES_TO_EVENT_TIME;
									}

									gGT->battleSetup.numWeapons = 0;

									// life limit
									gGT->battleSetup.lifeLimit = D230.battleSetupTables.lifeLimitValues[D230.menuBattleLengthLifeLife.rowSelected];

									// write RNG array of weaponIDs, based on weapon flags
									for (u32 weaponFlagIndex = 0; weaponFlagIndex < BATTLE_WEAPON_FLAG_COUNT; weaponFlagIndex++)
									{
										// bit flag of weapons enabled
										if ((gGT->battleSetup.enabledWeapons & 1 << weaponFlagIndex) != 0)
										{
											// write weaponID in RNG array
											gGT->battleSetup.RNG_itemSetCustom[gGT->battleSetup.numWeapons] = weaponFlagIndex;

											// increment number of weapons RNG can choose from
											gGT->battleSetup.numWeapons++;
										}
									}

									// start battle when transition is done
									D230.battleTransition.startAfterExit = 1;

									// start transition out
									D230.battleTransition.state = EXITING_MENU;

									// check if player changed team,
									// then clear stats if a change happened

									for (s32 playerIndex = 0; playerIndex < numPlyr; playerIndex++)
									{
										if (sdata->teamOfEachPlayer[playerIndex] != gGT->battleSetup.teamOfEachPlayer[playerIndex])
										{
											MainStats_ClearBattleVS();
										}
										sdata->teamOfEachPlayer[playerIndex] = gGT->battleSetup.teamOfEachPlayer[playerIndex];
									}

									sdata->buttonTapPerPlayer[1] = 0;
									sdata->buttonTapPerPlayer[2] = 0;
									sdata->buttonTapPerPlayer[3] = 0;
								}
							}
						}

						// If you press Right
						else
						{
							// if row 3 or 4 (weapons)
							if ((u32)sdata->battleSetupRowHighlighted - BATTLE_ROW_WEAPON_TOP < BATTLE_WEAPON_ROW_COUNT)
							{
								// change which weapon is highlighted
								sdata->battleSetupWeaponHighlighted++;
							}

							else
							{
								if ((D230.menuBattleType.rowSelected == BATTLE_LIFE_LIMIT_TYPE_ROW) && (sdata->battleSetupRowHighlighted == BATTLE_ROW_LENGTH))
								{
									sdata->battleSetupRowHighlighted = BATTLE_ROW_LIFE_COUNT;
								}
							}
						}
					}

					// If you press Left
					else
					{
						// if row 3 or 4 (weapons)
						if ((u32)sdata->battleSetupRowHighlighted - BATTLE_ROW_WEAPON_TOP < BATTLE_WEAPON_ROW_COUNT)
						{
							// change which weapon is highlighted
							sdata->battleSetupWeaponHighlighted--;
						}

						else
						{
							if ((D230.menuBattleType.rowSelected == BATTLE_LIFE_LIMIT_TYPE_ROW) && (sdata->battleSetupRowHighlighted == BATTLE_ROW_LIFE_COUNT))
							{
								goto LAB_800b1d7c;
							}
						}
					}
				}

				// If you press Down
				else
				{
					if ((D230.menuBattleType.rowSelected == BATTLE_LIFE_LIMIT_TYPE_ROW) && (sdata->battleSetupRowHighlighted == BATTLE_ROW_LIFE_COUNT))
					{
						sdata->battleSetupRowHighlighted = D230.menuBattleType.rowSelected;
					}
					else
					{
						// Move one row down
						sdata->battleSetupRowHighlighted++;

						// If you go below row 5 (Start Battle)
						if (BATTLE_ROW_START < sdata->battleSetupRowHighlighted)
						{
							// Go back to row 5
							sdata->battleSetupRowHighlighted = BATTLE_ROW_START;
						}
					}
				}
			}

			// If you press Up
			else
			{
				if ((D230.menuBattleType.rowSelected == BATTLE_LIFE_LIMIT_TYPE_ROW) && (sdata->battleSetupRowHighlighted == BATTLE_ROW_LIFE_COUNT))
				{
				LAB_800b1d7c:
					sdata->battleSetupRowHighlighted = D230.battleTransition.state;
				}
				else
				{
					// Go up one row
					sdata->battleSetupRowHighlighted--;

					// If you go above the top row (0)
					if (sdata->battleSetupRowHighlighted < 0)
					{
						// Go back to the top row
						sdata->battleSetupRowHighlighted = BATTLE_ROW_TYPE;
					}
				}
			}

			// If you are a row less than 5,
			// any row except the bottom
			if ((u32)sdata->battleSetupRowHighlighted - BATTLE_ROW_WEAPON_TOP < BATTLE_WEAPON_ROW_COUNT)
			{
				s32 highlightedWeaponRowOffset = (u32)sdata->battleSetupRowHighlighted - (BATTLE_ROW_WEAPON_TOP - 1);
				if (sdata->battleSetupWeaponHighlighted < 0)
				{
					sdata->battleSetupWeaponHighlighted = 0;
				}
				if (BATTLE_WEAPONS_PER_ROW - highlightedWeaponRowOffset < (int)sdata->battleSetupWeaponHighlighted)
				{
					sdata->battleSetupWeaponHighlighted = BATTLE_WEAPONS_PER_ROW - (s16)highlightedWeaponRowOffset;
				}
			}

			if (sdata->battleSetupRowHighlighted != previousHighlightedRow)
			{
				// Play sound
				// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b2178-0x800b2194 for battle row-change SFX.
				OtherFX_Play(0, 1);
			}
		}

		// if you are in any drop-down menu
		else
		{
			struct RectMenu *dropdownMenu = NULL;

			// If you are chosing Length
			// Dropdown menu for Points (5, 10, 15)
			// Dropdown menu for Time (3 minutes, 6 minutes, etc)
			if (sdata->battleSetupExpandMenu == BATTLE_ROW_LENGTH)
			{
				if (D230.menuBattleType.rowSelected == BATTLE_TIME_LIMIT_TYPE_ROW)
				{
					dropdownMenu = &D230.menuBattleLengthTimeTime;
				}
				else
				{
					if ((s16)D230.menuBattleType.rowSelected < 2)
					{
						if (D230.menuBattleType.rowSelected == BATTLE_ROW_TYPE)
						{
							dropdownMenu = &D230.menuBattleLengthPoints;
						}
					}
					else
					{
						if (D230.menuBattleType.rowSelected == BATTLE_LIFE_LIMIT_TYPE_ROW)
						{
							dropdownMenu = &D230.menuBattleLengthLifeTime;
						}
					}
				}
			}

			// If not choosing Length
			else
			{
				// < 2 means 0 or 1, seems pointless,
				// considering the previous != 1 check and next == 0 check
				if (sdata->battleSetupExpandMenu < 2)
				{
					// if you are chosing type of battle
					// Dropdown menu for (Point Limit, Life Limit, TIme Limit)
					if (sdata->battleSetupExpandMenu == BATTLE_ROW_TYPE)
					{
						dropdownMenu = &D230.menuBattleType;
					}
				}

				// sdata->battleSetupExpandMenu == BATTLE_ROW_TEAMS
				else
				{
					// If you're not chosing life limit
					// Dropdown for 3 lives, 6 lives, 9 lives
					if (sdata->battleSetupExpandMenu == BATTLE_ROW_LIFE_COUNT)
					{
						dropdownMenu = &D230.menuBattleLengthLifeLife;
					}
				}
			}
			if (dropdownMenu != NULL)
			{
				RECTMENU_ProcessInput(dropdownMenu);
				if ((dropdownMenu->state & ONLY_DRAW_TITLE) != 0)
				{
					dropdownMenu->state &= ~(ONLY_DRAW_TITLE);

					sdata->battleSetupExpandMenu = -1;
				}
			}
		}

		// clear gamepad input (for menus)
		RECTMENU_ClearInput();
	}

	struct TransitionMeta *tmbattle = D230.transitionMeta_battle;

	DecalFont_DrawLine(sdata->lngStrings[LNG_SETUP_BATTLE], tmbattle[BATTLE_TITLE_META_INDEX].currX + BATTLE_TITLE_X_OFFSET,
	                   tmbattle[BATTLE_TITLE_META_INDEX].currY + BATTLE_TITLE_Y_OFFSET, FONT_BIG, BATTLE_TITLE_TEXT_FLAGS);

	DecalFont_DrawLine(sdata->lngStrings[LNG_TYPE], tmbattle[BATTLE_ROW_TYPE_LABEL_META_INDEX].currX + BATTLE_LABEL_X_OFFSET,
	                   tmbattle[BATTLE_ROW_TYPE_LABEL_META_INDEX].currY + BATTLE_TYPE_ROW_Y_OFFSET, FONT_BIG, BATTLE_LABEL_TEXT_FLAGS);

	D230.menuBattleType.state &= ~(HIDE_ROW_HIGHLIGHT | SHOW_ONLY_HIGHLIT_ROW);

	// if you are not choosing type of battle
	if (sdata->battleSetupExpandMenu != BATTLE_ROW_TYPE)
	{
		D230.menuBattleType.state |= SHOW_ONLY_HIGHLIT_ROW;
	}

	if (sdata->battleSetupRowHighlighted != BATTLE_ROW_TYPE)
	{
		D230.menuBattleType.state |= HIDE_ROW_HIGHLIGHT;
	}

	RECTMENU_DrawSelf(&D230.menuBattleType, tmbattle[BATTLE_ROW_TYPE_MENU_META_INDEX].currX + BATTLE_MENU_X_OFFSET,
	                  tmbattle[BATTLE_ROW_TYPE_MENU_META_INDEX].currY + BATTLE_TYPE_ROW_Y_OFFSET, BATTLE_WIDE_MENU_WIDTH);

	s16 menuHeight = BATTLE_MENU_DEFAULT_HEIGHT;
	RECTMENU_GetHeight(&D230.menuBattleType, &menuHeight, 0);
	s16 lengthRowY = menuHeight + BATTLE_LENGTH_ROW_Y_OFFSET;
	s16 afterLengthY = lengthRowY;

	DecalFont_DrawLine(sdata->lngStrings[LNG_LENGTH], tmbattle[BATTLE_ROW_LENGTH_LABEL_META_INDEX].currX + BATTLE_LABEL_X_OFFSET,
	                   tmbattle[BATTLE_ROW_LENGTH_LABEL_META_INDEX].currY + lengthRowY + BATTLE_LENGTH_LABEL_Y_OFFSET, FONT_BIG, BATTLE_LABEL_TEXT_FLAGS);

	struct RectMenu *lengthMenu;
	if (D230.menuBattleType.rowSelected == BATTLE_TIME_LIMIT_TYPE_ROW)
	{
		lengthMenu = &D230.menuBattleLengthTimeTime;
	}
	else
	{
		afterLengthY = lengthRowY;
		if (BATTLE_TIME_LIMIT_TYPE_ROW < D230.menuBattleType.rowSelected)
		{
			if (D230.menuBattleType.rowSelected == BATTLE_LIFE_LIMIT_TYPE_ROW)
			{
				D230.menuBattleLengthLifeTime.state &= ~(HIDE_ROW_HIGHLIGHT | SHOW_ONLY_HIGHLIT_ROW);

				if (sdata->battleSetupExpandMenu != BATTLE_ROW_LENGTH)
				{
					D230.menuBattleLengthLifeTime.state |= SHOW_ONLY_HIGHLIT_ROW;
				}
				if (sdata->battleSetupRowHighlighted != BATTLE_ROW_LENGTH)
				{
					D230.menuBattleLengthLifeTime.state |= HIDE_ROW_HIGHLIGHT;
				}

				RECTMENU_DrawSelf(&D230.menuBattleLengthLifeTime, tmbattle[BATTLE_ROW_LENGTH_MENU_META_INDEX].currX + BATTLE_MENU_X_OFFSET,
				                  tmbattle[BATTLE_ROW_LENGTH_MENU_META_INDEX].currY + lengthRowY + BATTLE_LENGTH_LABEL_Y_OFFSET, BATTLE_LENGTH_DUAL_MENU_WIDTH);

				D230.menuBattleLengthLifeLife.state &= ~(HIDE_ROW_HIGHLIGHT | SHOW_ONLY_HIGHLIT_ROW);

				if (sdata->battleSetupExpandMenu != BATTLE_ROW_LIFE_COUNT)
				{
					D230.menuBattleLengthLifeLife.state |= SHOW_ONLY_HIGHLIT_ROW;
				}
				if (sdata->battleSetupRowHighlighted != BATTLE_ROW_LIFE_COUNT)
				{
					D230.menuBattleLengthLifeLife.state |= HIDE_ROW_HIGHLIGHT;
				}

				RECTMENU_DrawSelf(&D230.menuBattleLengthLifeLife, tmbattle[BATTLE_ROW_LENGTH_MENU_META_INDEX].currX + BATTLE_LENGTH_DUAL_MENU_X_OFFSET,
				                  tmbattle[BATTLE_ROW_LENGTH_MENU_META_INDEX].currY + lengthRowY + BATTLE_LENGTH_LABEL_Y_OFFSET, BATTLE_LENGTH_DUAL_MENU_WIDTH);

				s16 lifeTimeMenuHeight = BATTLE_MENU_DEFAULT_HEIGHT;
				RECTMENU_GetHeight(&D230.menuBattleLengthLifeTime, &lifeTimeMenuHeight, 0);
				s16 lifeCountMenuHeight = BATTLE_MENU_DEFAULT_HEIGHT;
				RECTMENU_GetHeight(&D230.menuBattleLengthLifeLife, &lifeCountMenuHeight, 0);
				afterLengthY = lifeCountMenuHeight + lengthRowY;
				if (lifeCountMenuHeight < lifeTimeMenuHeight)
				{
					afterLengthY = lifeTimeMenuHeight + lengthRowY;
				}
			}
			goto LAB_800b25f0;
		}
		if (D230.menuBattleType.rowSelected != BATTLE_ROW_TYPE)
		{
			goto LAB_800b25f0;
		}
		lengthMenu = &D230.menuBattleLengthPoints;
	}

	lengthMenu->state &= ~(HIDE_ROW_HIGHLIGHT | SHOW_ONLY_HIGHLIT_ROW);

	if (sdata->battleSetupExpandMenu != BATTLE_ROW_LENGTH)
	{
		lengthMenu->state |= SHOW_ONLY_HIGHLIT_ROW;
	}
	if (sdata->battleSetupRowHighlighted != BATTLE_ROW_LENGTH)
	{
		lengthMenu->state |= HIDE_ROW_HIGHLIGHT;
	}

	RECTMENU_DrawSelf(lengthMenu, tmbattle[BATTLE_ROW_LENGTH_MENU_META_INDEX].currX + BATTLE_MENU_X_OFFSET,
	                  tmbattle[BATTLE_ROW_LENGTH_MENU_META_INDEX].currY + lengthRowY + BATTLE_LENGTH_LABEL_Y_OFFSET, BATTLE_WIDE_MENU_WIDTH);

	menuHeight = BATTLE_MENU_DEFAULT_HEIGHT;
	RECTMENU_GetHeight(lengthMenu, &menuHeight, 0);
	afterLengthY = menuHeight + lengthRowY;

	s32 teamPanelX;
LAB_800b25f0:

	teamPanelX = BATTLE_TEAM_PANEL_START_X;

	DecalFont_DrawLine(sdata->lngStrings[LNG_TEAMS], tmbattle[BATTLE_ROW_TEAM_LABEL_META_INDEX].currX + BATTLE_LABEL_X_OFFSET,
	                   tmbattle[BATTLE_ROW_TEAM_LABEL_META_INDEX].currY + afterLengthY + BATTLE_TEAM_LABEL_Y_OFFSET, FONT_BIG, BATTLE_LABEL_TEXT_FLAGS);

	s32 accumulatedTeamWidth = BATTLE_TEAM_SEGMENT_BASE_WIDTH;
	int finalTeamWidth = 0;

	for (s32 teamIndex = 0; teamIndex < BATTLE_TEAM_COUNT; teamIndex++)
	{
		int totalTeamWidth = accumulatedTeamWidth;
		teamPlayerCounts[teamIndex] = 0;
		teamSegmentWidths[teamIndex] = BATTLE_TEAM_SEGMENT_BASE_WIDTH;

		for (s32 playerIndex = 0; playerIndex < numPlyr; playerIndex++)
		{
			int playerTeam = (int)gGT->battleSetup.teamOfEachPlayer[playerIndex];
			if (playerTeam == teamIndex)
			{
				teamPlayerCounts[playerTeam]++;
				teamSegmentWidths[playerTeam] += BATTLE_TEAM_PLAYER_WIDTH;
				totalTeamWidth = totalTeamWidth + BATTLE_TEAM_PLAYER_WIDTH;
			}
		}

		finalTeamWidth = totalTeamWidth;
		accumulatedTeamWidth = totalTeamWidth + BATTLE_TEAM_SEGMENT_BASE_WIDTH;
	}

	u32 remainingTeamWidth = BATTLE_TEAM_PANEL_TOTAL_WIDTH - finalTeamWidth;
	s32 signedRemainingTeamWidth = (s32)remainingTeamWidth;
	u32 sharedTeamPadding = signedRemainingTeamWidth + 3;
	if ((int)sharedTeamPadding < 0)
	{
		sharedTeamPadding = signedRemainingTeamWidth + 6;
	}
	sharedTeamPadding = sharedTeamPadding >> 2;

	for (s32 teamIndex = 0; teamIndex < BATTLE_TEAM_COUNT; teamIndex++)
	{
		teamSegmentWidths[teamIndex] += sharedTeamPadding;
		remainingTeamWidth = remainingTeamWidth - sharedTeamPadding;
		if ((int)(remainingTeamWidth) < (int)(sharedTeamPadding))
		{
			sharedTeamPadding = remainingTeamWidth;
		}
	}

	uint32_t *ot = gGT->backBuffer->otMem.uiOT;

	for (s32 teamIndex = 0; teamIndex < BATTLE_TEAM_COUNT; teamIndex++)
	{
		u16 teamSegmentWidth = teamSegmentWidths[teamIndex];
		int playerIconX = teamPanelX + (teamSegmentWidth >> 1) + (int)teamPlayerCounts[teamIndex] * BATTLE_TEAM_PLAYER_CENTER_OFFSET;

		for (s32 playerIndex = 0; playerIndex < numPlyr; playerIndex++)
		{
			if (gGT->battleSetup.teamOfEachPlayer[playerIndex] == teamIndex)
			{
				s16 iconX = (s16)playerIconX;
				playerIconX = playerIconX + BATTLE_TEAM_PLAYER_WIDTH;

				MM_Battle_DrawIcon_Character(gGT->ptrIcons[data.MetaDataCharacters[data.characterIDs[playerIndex]].iconID],
				                             (int)tmbattle[BATTLE_ROW_TEAM_META_INDEX].currX + (int)iconX,
				                             (int)tmbattle[BATTLE_ROW_TEAM_META_INDEX].currY + (int)afterLengthY + BATTLE_TEAM_PLAYER_ICON_Y_OFFSET,

				                             &gGT->backBuffer->primMem, gGT->pushBuffer_UI.ptrOT, 1, BATTLE_ICON_SCALE);
			}
		}

		RECT teamColorRect;
		teamColorRect.h = BATTLE_TEAM_COLOR_H;
		teamColorRect.x = tmbattle[BATTLE_ROW_TEAM_META_INDEX].currX + (s16)teamPanelX;
		teamColorRect.y = tmbattle[BATTLE_ROW_TEAM_META_INDEX].currY + afterLengthY + BATTLE_TEAM_COLOR_Y_OFFSET;
		teamPanelX = teamPanelX + (u32)teamSegmentWidth;

		teamColorRect.w = teamSegmentWidth;

		Color color;
		color.self = *data.ptrColor[PLAYER_BLUE + teamIndex];
		CTR_Box_DrawSolidBox(&teamColorRect, color, ot);
	}

	if (sdata->battleSetupRowHighlighted == BATTLE_ROW_TEAMS)
	{
		RECT teamHighlightRect;
		teamHighlightRect.w = BATTLE_TEAM_HIGHLIGHT_W;
		teamHighlightRect.h = BATTLE_TEAM_HIGHLIGHT_H;
		teamHighlightRect.x = tmbattle[BATTLE_ROW_TEAM_META_INDEX].currX + BATTLE_MENU_X_OFFSET;
		teamHighlightRect.y = tmbattle[BATTLE_ROW_TEAM_META_INDEX].currY + afterLengthY + BATTLE_TEAM_HIGHLIGHT_Y_OFFSET;

		CTR_Box_DrawClearBox(&teamHighlightRect, &sdata->menuRowHighlight_Normal, TRANS_50_DECAL, (uint32_t *)ot);
	}

	RECT teamPanelRect;
	teamPanelRect.w = BATTLE_TEAM_PANEL_W;
	teamPanelRect.h = BATTLE_TEAM_PANEL_H;
	teamPanelRect.x = tmbattle[BATTLE_ROW_TEAM_META_INDEX].currX + BATTLE_TEAM_PANEL_X_OFFSET;
	teamPanelRect.y = tmbattle[BATTLE_ROW_TEAM_META_INDEX].currY + afterLengthY;

	// Draw 2D Menu rectangle background
	RECTMENU_DrawInnerRect(&teamPanelRect, 0, ot);

	DecalFont_DrawLine(sdata->lngStrings[LNG_WEAPONS], tmbattle[BATTLE_ROW_WEAPON_LABEL_META_INDEX].currX + BATTLE_LABEL_X_OFFSET,
	                   tmbattle[BATTLE_ROW_WEAPON_LABEL_META_INDEX].currY + afterLengthY + BATTLE_WEAPON_LABEL_Y_OFFSET, FONT_BIG, BATTLE_LABEL_TEXT_FLAGS);

	// make flashing color for error message

	// set default color
	s16 flashingErrorColor = BATTLE_ERROR_COLOR_A;

	// if time on timer is odd
	if ((sdata->frameCounter & BATTLE_ERROR_COLOR_FRAME_BIT) != 0)
	{
		// change color
		flashingErrorColor = BATTLE_ERROR_COLOR_B;
	}

	s32 errorLine1 = 0;
	s32 errorLine2 = 0;

	// If you have no weapons selected, which are in flags "0xcde"
	if ((gGT->battleSetup.enabledWeapons & BATTLE_REQUIRED_WEAPON_FLAGS) == 0)
	{
		errorLine1 = LNG_WEAPONS_ERROR_LINE1;
		errorLine2 = LNG_AT_LEAST_ONE_WEAPON;
	}

	// if you have at least one weapon selected
	else
	{
		// If number of teams is less than 2
		if (gGT->battleSetup.numTeams < 2)
		{
			errorLine1 = LNG_TEAMS_ERROR_LINE1;

			errorLine2 = LNG_TWO_OR_MORE_TEAMS;
		}
	}

	// If you have no errors that prevent
	// the player from starting the Battle
	if (errorLine1 == 0)
	{
		D230.menuBattleStartGame.state &= ~(HIDE_ROW_HIGHLIGHT | SHOW_ONLY_HIGHLIT_ROW);

		if (sdata->battleSetupExpandMenu != BATTLE_ROW_START)
		{
			D230.menuBattleStartGame.state |= SHOW_ONLY_HIGHLIT_ROW;
		}
		if (sdata->battleSetupRowHighlighted != BATTLE_ROW_START)
		{
			D230.menuBattleStartGame.state |= HIDE_ROW_HIGHLIGHT;
		}
		RECTMENU_DrawSelf(&D230.menuBattleStartGame, tmbattle[BATTLE_ROW_START_META_INDEX].currX + BATTLE_MENU_X_OFFSET,
		                  tmbattle[BATTLE_ROW_START_META_INDEX].currY + afterLengthY + BATTLE_START_MENU_Y_OFFSET, BATTLE_WIDE_MENU_WIDTH);

		menuHeight = BATTLE_MENU_DEFAULT_HEIGHT;
		RECTMENU_GetHeight(&D230.menuBattleStartGame, &menuHeight, 0);
	}

	// If you have no errors that prevent
	// the player from starting the Battle
	else
	{
		// Print two lines of error text,
		// one on top of the other, centered text,
		// BATTLE_ERROR_TEXT_X for halfway on the X-axis,
		// flashing color

		DecalFont_DrawLine(sdata->lngStrings[errorLine1], BATTLE_ERROR_TEXT_X, afterLengthY + BATTLE_ERROR_TEXT_LINE_1_Y_OFFSET, FONT_BIG,
		                   (int)flashingErrorColor);
		DecalFont_DrawLine(sdata->lngStrings[errorLine2], BATTLE_ERROR_TEXT_X, afterLengthY + BATTLE_ERROR_TEXT_LINE_2_Y_OFFSET, FONT_BIG,
		                   (int)flashingErrorColor);
	}
	RECT weaponPanelRect;
	weaponPanelRect.w = BATTLE_WEAPON_PANEL_W;
	weaponPanelRect.h = BATTLE_WEAPON_PANEL_H;
	weaponPanelRect.x = tmbattle[BATTLE_ROW_WEAPON_PANEL_META_INDEX].currX + BATTLE_WEAPON_PANEL_X_OFFSET;
	weaponPanelRect.y = tmbattle[BATTLE_ROW_WEAPON_PANEL_META_INDEX].currY + afterLengthY + BATTLE_WEAPON_PANEL_Y_OFFSET;

	// Loop through all 11 weapon icons
	for (s32 weaponIndex = 0; weaponIndex < BATTLE_WEAPON_ITEM_COUNT; weaponIndex++)
	{
		s32 weaponRow = weaponIndex / BATTLE_WEAPONS_PER_ROW;

		const struct BattleWeaponMenuItem *weaponItem = &D230.battleWeaponItems[weaponIndex];
		const Color *weaponColor = &D230.battleWeaponEnabledColor;
		u32 weaponTextColor = BATTLE_WEAPON_ENABLED_TEXT_COLOR;

		// Check if this weapon is not enabled
		if ((gGT->battleSetup.enabledWeapons & weaponItem->enabledWeaponFlag) == 0)
		{
			weaponColor = &D230.battleWeaponDisabledColor;
			weaponTextColor = BATTLE_WEAPON_DISABLED_TEXT_COLOR;
		}

		// weaponIndex % 6
		// Go to 2nd row after 6th icon
		int weaponPosX = (u32)weaponPanelRect.x + BATTLE_WEAPON_GRID_X_OFFSET + (weaponIndex % BATTLE_WEAPONS_PER_ROW) * BATTLE_WEAPON_GRID_X_STEP +
		                 weaponRow * BATTLE_WEAPON_GRID_ROW_STAGGER_X;

		s32 weaponPosY = (u32)weaponPanelRect.y + BATTLE_WEAPON_GRID_Y_OFFSET + weaponRow * BATTLE_WEAPON_GRID_Y_STEP;

		// If the icon is bowling bomb or missile on the 2nd row
		if (((weaponIndex - BATTLE_WEAPON_AMMO_TEXT_FIRST) & 0xffff) < BATTLE_WEAPON_AMMO_TEXT_COUNT)
		{
			// draw the "3" over the icons
			DecalFont_DrawLine(&R230.s_3[0], weaponPosX, weaponPosY, 2, weaponTextColor);
		}

		MM_Battle_DrawIcon_Weapon(gGT->ptrIcons[weaponItem->iconID], weaponPosX, weaponPosY, &gGT->backBuffer->primMem, (u32 *)gGT->pushBuffer_UI.ptrOT, 1,
		                          BATTLE_ICON_SCALE, BATTLE_WEAPON_ICON_ROTATE_RIGHT, weaponColor);
	}

	if ((u32)sdata->battleSetupRowHighlighted - BATTLE_ROW_WEAPON_TOP < BATTLE_WEAPON_ROW_COUNT)
	{
		RECT weaponHighlightRect;
		s16 highlightedWeaponX = weaponPanelRect.x + sdata->battleSetupWeaponHighlighted * BATTLE_WEAPON_GRID_X_STEP;
		weaponHighlightRect.x = highlightedWeaponX + BATTLE_WEAPON_HIGHLIGHT_LEFT_X_OFFSET;
		if (sdata->battleSetupRowHighlighted == BATTLE_ROW_WEAPON_BOTTOM)
		{
			weaponHighlightRect.x = highlightedWeaponX + BATTLE_WEAPON_HIGHLIGHT_RIGHT_X_OFFSET;
		}
		weaponHighlightRect.w = BATTLE_WEAPON_HIGHLIGHT_W;
		weaponHighlightRect.h = BATTLE_WEAPON_HIGHLIGHT_H;
		weaponHighlightRect.y =
		    weaponPanelRect.y + (sdata->battleSetupRowHighlighted - BATTLE_ROW_WEAPON_TOP) * BATTLE_WEAPON_GRID_Y_STEP + BATTLE_WEAPON_HIGHLIGHT_Y_OFFSET;

		CTR_Box_DrawClearBox(&weaponHighlightRect, &sdata->menuRowHighlight_Normal, TRANS_50_DECAL, ot);
	}

	RECT weaponPanelInsetRect;
	weaponPanelInsetRect.x = weaponPanelRect.x + BATTLE_WEAPON_PANEL_INSET_X;
	weaponPanelInsetRect.y = weaponPanelRect.y + BATTLE_WEAPON_PANEL_INSET_Y;
	weaponPanelInsetRect.w = weaponPanelRect.w - BATTLE_WEAPON_PANEL_INSET_W_SHRINK;
	weaponPanelInsetRect.h = weaponPanelRect.h - BATTLE_WEAPON_PANEL_INSET_H_SHRINK;

	CTR_Box_DrawClearBox(&weaponPanelInsetRect, &D230.battleWeaponPanelColor, TRANS_50_DECAL, ot);

	RECTMENU_DrawInnerRect(&weaponPanelRect, 0, ot);

	// save all five battle settings
	// these are selected rows from all battle options
	for (s32 settingIndex = 0; settingIndex < BATTLE_SETTINGS_COUNT; settingIndex++)
	{
		sdata->battleSettings[settingIndex] = D230.battleMenuArray[settingIndex]->rowSelected;
	}
	return;
}
