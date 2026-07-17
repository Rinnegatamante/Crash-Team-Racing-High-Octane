#include <common.h>

enum AHPauseIconGroupID
{
	AH_PAUSE_ICON_GROUP_HUD = 4,
	AH_PAUSE_ICON_GROUP_MISC = 5,
};

enum AHPauseHudIconID
{
	AH_PAUSE_HUD_ICON_BOSS_STAR = 0x37,
	AH_PAUSE_HUD_ICON_PAGE_ARROW = 0x38,
};

enum AHPauseLayout
{
	AH_PAUSE_UI_COORD_SCALE = 0x100,

	AH_PAUSE_TITLE_CENTER_X = 0x100,
	AH_PAUSE_TITLE_Y = 0xf,
	AH_PAUSE_TITLE_FRAME_PAD_W = 0x14,
	AH_PAUSE_TITLE_FRAME_MIN_TEXT_W = 0x20b,
	AH_PAUSE_TITLE_FRAME_W = 0x21e,

	AH_PAUSE_ARROW_Y = 0x16,
	AH_PAUSE_ARROW_LEFT_X_OFFSET = 0xec,
	AH_PAUSE_ARROW_RIGHT_X_OFFSET = 0x112,
	AH_PAUSE_ARROW_SCALE = 0x1000,
	AH_PAUSE_ARROW_LEFT_ROT_Y = 0x800,

	AH_PAUSE_HUB_TEXT_X = 0x50,
	AH_PAUSE_HUB_ICON_X = 0x15e,
	AH_PAUSE_HUB_FIRST_TEXT_X = 0x6e,
	AH_PAUSE_HUB_FIRST_ICON_X = 0x16d,
	AH_PAUSE_HUB_FIRST_ROW_BASE = 4,
	AH_PAUSE_LEVEL_SCAN_COUNT = 0x41,
	AH_PAUSE_FIRST_CRYSTAL_LEVEL = AH_PAUSE_RELIC_TRACK_COUNT,
	AH_PAUSE_HUB_REWARD_ICON_COUNT = 3,
	AH_PAUSE_ROW_STEP_Y = 0x10,
	AH_PAUSE_ROW_TEXT_Y = 0x26,
	AH_PAUSE_ROW_ICON_Y = 0x2f,
	AH_PAUSE_ROW_ICON_STEP_X = 0x1e,

	AH_PAUSE_BOSS_STAR_X_OFFSET = 0x18,
	AH_PAUSE_BOSS_STAR_Y_OFFSET = 0x2a,

	AH_PAUSE_GEM_PAGE_CENTER_INDEX = 2,
	AH_PAUSE_GEM_PAGE_ICON_SPACING_X = 60,
	AH_PAUSE_GEM_PAGE_Y_BASE = 0x6a,
	AH_PAUSE_GEM_PAGE_Y_ALT_SHIFT = 4,

	AH_PAUSE_TOKEN_TOTAL_CENTER_INDEX = 2,
	AH_PAUSE_TOKEN_TOTAL_X_BASE = 0xf0,
	AH_PAUSE_TOKEN_TOTAL_SPACING_X = 60,
	AH_PAUSE_TOKEN_TOTAL_ROW_Y_STEP = 0x28,
	AH_PAUSE_TOKEN_TOTAL_ICON_Y = 0x41,
	AH_PAUSE_TOKEN_TOTAL_NUMBER_X = 0x36,
	AH_PAUSE_TOKEN_TOTAL_NUMBER_Y = 0x3a,
	AH_PAUSE_TOKEN_TOTAL_X_GLYPH_X = 0x24,
	AH_PAUSE_TOKEN_TOTAL_X_GLYPH_Y = 0x3e,

	AH_PAUSE_RELIC_TOTAL_CENTER_INDEX = 1,
	AH_PAUSE_RELIC_TOTAL_X_BASE = 0xf6,
	AH_PAUSE_RELIC_TOTAL_SPACING_X = 90,
	AH_PAUSE_RELIC_TOTAL_Y = 0x49,
	AH_PAUSE_RELIC_TOTAL_NUMBER_X = 0x19,
	AH_PAUSE_RELIC_TOTAL_X_GLYPH_X = 10,
	AH_PAUSE_RELIC_TOTAL_X_GLYPH_Y = 0x4e,
	AH_PAUSE_X_GLYPH = 'X',

	AH_PAUSE_TOTAL_TEXT_Y = 0x6e,

	AH_PAUSE_OUTER_RECT_CENTER_X = 0x10a,
	AH_PAUSE_OUTER_RECT_Y = 0x20,
	AH_PAUSE_OUTER_RECT_H = 2,
	AH_PAUSE_OUTER_EDGE_ALPHA = 0x20,
	AH_PAUSE_INNER_RECT_CENTER_X = 0x100,
	AH_PAUSE_INNER_RECT_Y = 10,
	AH_PAUSE_INNER_RECT_H = 0x82,
	AH_PAUSE_INNER_RECT_COLOR = WHITE,
	AH_PAUSE_INNER_RECT_OT_OFFSET = 3,

	AH_PAUSE_LOCKED_ALPHA = 0x1000,
	AH_PAUSE_TOKEN_TOTAL_SCALE = 0x1000,
	AH_PAUSE_NON_HUB_SCALE_SHIFT = 2,

	AH_PAUSE_MODEL_ROT_X_WEIGHT = 0x10,
	AH_PAUSE_MODEL_ROT_Y_WEIGHT = 0x20,
	AH_PAUSE_MODEL_ROT_FRAME_WEIGHT = 0x40,
	AH_PAUSE_MODEL_ROT_MASK = 0xfff,

	AH_PAUSE_INSTANCE_DEPTH = 0x100,
	AH_PAUSE_PAGE_FLIP_FRAMES = 8,
	AH_PAUSE_PAGE_FLIP_SECOND_HALF = 5,
	AH_PAUSE_PAGE_FLIP_X_STEP = 0x80,
};

enum AHPauseColorSlot
{
	AH_PAUSE_ARROW_COLOR_PRIMARY = ORANGE,
	AH_PAUSE_ARROW_COLOR_SECONDARY = RED,
	AH_PAUSE_BOSS_STAR_LOCKED_COLOR = BLACK,
	AH_PAUSE_BOSS_STAR_UNLOCKED_COLOR = PERIWINKLE,
};

enum AHPauseInstanceFlags
{
	AH_PAUSE_INSTANCE_DYNAMIC_FLAGS = HIDE_MODEL | DRAW_TRANSPARENT | USE_SPECULAR_LIGHT | DRAW_BILLBOARD,
};

enum AHPauseMemberFlags
{
	AH_PAUSE_MEMBER_UNLOCKED = 1,
};

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b1ef8-0x800b1f78.
void AH_Pause_Destroy(void)
{
	s32 i;

	// global -> register
	struct PauseObject *ptrPauseObject = D232.ptrPauseObject;

	// check register
	if (ptrPauseObject == 0)
	{
		return;
	}

	// loop through 14 instances, destroy them
	for (i = 0; i < AH_PAUSE_MEMBER_COUNT; i++)
	{
		struct AHPauseMember *member = &ptrPauseObject->members[i];
		INSTANCE_Death(member->inst);
	}

	// kill thread
	D232.ptrPauseObject = 0;
	ptrPauseObject->t->flags |= THREAD_FLAG_DEAD;
}

void AH_Pause_Draw(s32 pageID, s32 posX)
{
	RECT r;
	s32 levelID = D232.advPausePages[pageID].hubID;
	s32 lngIndex = D232.advPausePages[pageID].titleLng;

	s32 relicTotal;
	struct AdvProgress *adv = &sdata->advProgress;

	if (lngIndex < 0)
	{
		lngIndex = data.metaDataLEV[levelID].name_LNG;
	}

	char *titleString = sdata->lngStrings[lngIndex];

	DecalFont_DrawLine(titleString, posX + AH_PAUSE_TITLE_CENTER_X, AH_PAUSE_TITLE_Y, FONT_BIG, 0xffff8000);

	s32 titleWidth = DecalFont_GetLineWidth(titleString, FONT_BIG);

	s32 titleHalfWidth = titleWidth >> 1;

	// orange/red
	s32 colorIndex = AH_PAUSE_ARROW_COLOR_PRIMARY;
	if ((sdata->frameCounter & 4) == 0)
	{
		colorIndex = AH_PAUSE_ARROW_COLOR_SECONDARY;
	}

	u32 *arrowColor = data.ptrColor[colorIndex];

	struct GameTracker *gGT = sdata->gGT;
	struct PrimMem *primMem = &gGT->backBuffer->primMem;

	struct Icon **iconPtrArray = ICONGROUP_GETICONS(gGT->iconGroup[AH_PAUSE_ICON_GROUP_HUD]);

	// Draw arrow pointing Left
	DecalHUD_Arrow2D(iconPtrArray[AH_PAUSE_HUD_ICON_PAGE_ARROW], (posX - titleHalfWidth) + AH_PAUSE_ARROW_LEFT_X_OFFSET, AH_PAUSE_ARROW_Y,

	                 primMem, gGT->pushBuffer_UI.ptrOT,

	                 arrowColor[0], arrowColor[1], arrowColor[2], arrowColor[3],

	                 0, AH_PAUSE_ARROW_SCALE, AH_PAUSE_ARROW_LEFT_ROT_Y);

	// Draw arrow pointing Right
	DecalHUD_Arrow2D(iconPtrArray[AH_PAUSE_HUD_ICON_PAGE_ARROW], (posX + titleHalfWidth) + AH_PAUSE_ARROW_RIGHT_X_OFFSET, AH_PAUSE_ARROW_Y,

	                 primMem, gGT->pushBuffer_UI.ptrOT,

	                 arrowColor[0], arrowColor[1], arrowColor[2], arrowColor[3],

	                 0, AH_PAUSE_ARROW_SCALE, 0);

	struct PauseObject *ptrPauseObject = D232.ptrPauseObject;

	// loop through 14 instances
	for (s32 i = 0; i < AH_PAUSE_MEMBER_COUNT; i++)
	{
		struct AHPauseMember *member = &ptrPauseObject->members[i];

		// assume no awards won
		member->unlockFlags &= ~AH_PAUSE_MEMBER_UNLOCKED;

		// dont draw instance
		member->iconIndex = AH_PAUSE_ICON_NONE;
	}

	s32 type = D232.advPausePages[pageID].type;

	if (type == AH_PAUSE_PAGE_HUB)
	{
		s32 hubID = levelID - GEM_STONE_VALLEY;
		s32 rowIndex = 0;
		s32 pauseIndex = 0;
		s32 crystalID = -1;
		s32 textX = AH_PAUSE_HUB_TEXT_X;
		s32 iconX = AH_PAUSE_HUB_ICON_X;
		s32 rowBase = 0;

		if (hubID == 0)
		{
			textX = AH_PAUSE_HUB_FIRST_TEXT_X;
			iconX = AH_PAUSE_HUB_FIRST_ICON_X;
			rowBase = AH_PAUSE_HUB_FIRST_ROW_BASE;
		}

		for (s32 trackID = 0; trackID < AH_PAUSE_LEVEL_SCAN_COUNT; trackID++)
		{
			struct MetaDataLEV *mdLev = &data.metaDataLEV[trackID];

			if (mdLev->hubID != hubID)
			{
				continue;
			}

			if (trackID >= AH_PAUSE_FIRST_CRYSTAL_LEVEL)
			{
				crystalID = trackID;
				continue;
			}

			s32 rowY = rowBase + rowIndex * AH_PAUSE_ROW_STEP_Y;
			rowIndex++;

			DecalFont_DrawLine(sdata->lngStrings[mdLev->name_LNG], posX + textX, rowY + AH_PAUSE_ROW_TEXT_Y, FONT_BIG, 0);

			if (hubID != 0)
			{
				for (s32 j = 0; j < AH_PAUSE_HUB_REWARD_ICON_COUNT; j++)
				{
					struct AHPauseMember *member = &ptrPauseObject->members[pauseIndex + j];
					struct Instance *inst = member->inst;

					// Remove SelectProfile with regular UI variant
					inst->matrix.t[0] = UI_ConvertX_2(posX + iconX + j * AH_PAUSE_ROW_ICON_STEP_X, AH_PAUSE_UI_COORD_SCALE);

					inst->matrix.t[1] = UI_ConvertY_2(rowY + AH_PAUSE_ROW_ICON_Y, AH_PAUSE_UI_COORD_SCALE);
				}

				struct AHPauseMember *member = &ptrPauseObject->members[pauseIndex];
				member->iconIndex = AH_PAUSE_ICON_TROPHY;
				member->unlockFlags |= CHECK_ADV_BIT(adv->rewards, trackID + ADV_REWARD_FIRST_TROPHY);
				pauseIndex++;
			}
			else
			{
				struct AHPauseMember *member = &ptrPauseObject->members[pauseIndex];
				struct Instance *inst = member->inst;

				// Remove SelectProfile with regular UI variant
				inst->matrix.t[0] = UI_ConvertX_2(posX + iconX + AH_PAUSE_ROW_ICON_STEP_X, AH_PAUSE_UI_COORD_SCALE);

				inst->matrix.t[1] = UI_ConvertY_2(rowY + AH_PAUSE_ROW_ICON_Y, AH_PAUSE_UI_COORD_SCALE);
			}

			struct AHPauseMember *member = &ptrPauseObject->members[pauseIndex];

			if (CHECK_ADV_BIT(adv->rewards, trackID + ADV_REWARD_FIRST_PLATINUM_RELIC))
			{
				member->unlockFlags |= AH_PAUSE_MEMBER_UNLOCKED;
				member->iconIndex = AH_PAUSE_ICON_PLATINUM_RELIC;
			}
			else if (CHECK_ADV_BIT(adv->rewards, trackID + ADV_REWARD_FIRST_GOLD_RELIC))
			{
				member->unlockFlags |= AH_PAUSE_MEMBER_UNLOCKED;
				member->iconIndex = AH_PAUSE_ICON_GOLD_RELIC;
			}
			else
			{
				member->iconIndex = AH_PAUSE_ICON_SAPPHIRE_RELIC;
				member->unlockFlags |= CHECK_ADV_BIT(adv->rewards, trackID + ADV_REWARD_FIRST_SAPPHIRE_RELIC);
			}

			pauseIndex++;

			if (hubID != 0)
			{
				member = &ptrPauseObject->members[pauseIndex];
				member->iconIndex = AH_PAUSE_ICON_FIRST_TOKEN + mdLev->ctrTokenGroupID;
				member->unlockFlags |= CHECK_ADV_BIT(adv->rewards, trackID + ADV_REWARD_FIRST_CTR_TOKEN);
				pauseIndex++;
			}
		}

		s32 bossRowY = rowBase + rowIndex * AH_PAUSE_ROW_STEP_Y;
		s32 bossID = D232.advPausePages[pageID].characterID_Boss;

		DecalFont_DrawLine(sdata->lngStrings[data.MetaDataCharacters[bossID].name_LNG_long], posX + textX, bossRowY + AH_PAUSE_ROW_TEXT_Y, FONT_BIG, WHITE);

		if (hubID == 0)
		{
			// === Draw Star ===

			// black
			s32 bossStarColor = AH_PAUSE_BOSS_STAR_LOCKED_COLOR;

			// set to alternate color slot (if beaten oxide at least once)
			if (CHECK_ADV_BIT(adv->rewards, data.BeatBossPrize[0]))
			{
				bossStarColor = AH_PAUSE_BOSS_STAR_UNLOCKED_COLOR;
			}

			u32 *starColor = data.ptrColor[bossStarColor];

			struct Icon **iconPtrArray = ICONGROUP_GETICONS(gGT->iconGroup[AH_PAUSE_ICON_GROUP_MISC]);

			DecalHUD_DrawPolyGT4(iconPtrArray[AH_PAUSE_HUD_ICON_BOSS_STAR],

			                     posX + iconX + AH_PAUSE_BOSS_STAR_X_OFFSET, bossRowY + AH_PAUSE_BOSS_STAR_Y_OFFSET,

			                     &gGT->backBuffer->primMem, gGT->pushBuffer_UI.ptrOT,

			                     starColor[0], starColor[1], starColor[2], starColor[3],

			                     0, AH_PAUSE_ARROW_SCALE);

			pauseIndex = rowIndex;

			for (s32 i = 0; i < AH_PAUSE_GEM_ICON_COUNT; i++)
			{
				struct AHPauseMember *member = &ptrPauseObject->members[pauseIndex + i];
				struct Instance *inst = member->inst;

				// Remove SelectProfile with regular UI variant
				inst->matrix.t[0] = UI_ConvertX_2(posX + AH_PAUSE_TITLE_CENTER_X + (i - AH_PAUSE_GEM_PAGE_CENTER_INDEX) * AH_PAUSE_GEM_PAGE_ICON_SPACING_X,
				                                  AH_PAUSE_UI_COORD_SCALE);

				inst->matrix.t[1] = UI_ConvertY_2(((i & 1) << AH_PAUSE_GEM_PAGE_Y_ALT_SHIFT) | AH_PAUSE_GEM_PAGE_Y_BASE, AH_PAUSE_UI_COORD_SCALE);

				// gem color
				member->iconIndex = AH_PAUSE_ICON_FIRST_GEM + i;

				// unlock gem
				member->unlockFlags |= CHECK_ADV_BIT(adv->rewards, i + ADV_REWARD_FIRST_GEM);
			}
		}
		else
		{
			struct AHPauseMember *member = &ptrPauseObject->members[pauseIndex];
			struct Instance *inst = member->inst;

			// Remove SelectProfile with regular UI variant
			inst->matrix.t[0] = UI_ConvertX_2(posX + iconX + AH_PAUSE_ROW_ICON_STEP_X, AH_PAUSE_UI_COORD_SCALE);

			inst->matrix.t[1] = UI_ConvertY_2(bossRowY + AH_PAUSE_ROW_ICON_Y, AH_PAUSE_UI_COORD_SCALE);

			member->iconIndex = AH_PAUSE_ICON_BOSS_KEY;
			member->unlockFlags |= CHECK_ADV_BIT(adv->rewards, data.BeatBossPrize[hubID]);
			pauseIndex++;

			if (crystalID >= 0)
			{
				struct MetaDataLEV *mdLev = &data.metaDataLEV[crystalID];
				s32 crystalRowY = bossRowY + AH_PAUSE_ROW_STEP_Y;

				DecalFont_DrawLine(sdata->lngStrings[mdLev->name_LNG], posX + textX, crystalRowY + AH_PAUSE_ROW_TEXT_Y, FONT_BIG, PERIWINKLE);

				member = &ptrPauseObject->members[pauseIndex];
				inst = member->inst;

				// Remove SelectProfile with regular UI variant
				inst->matrix.t[0] = UI_ConvertX_2(posX + iconX + AH_PAUSE_ROW_ICON_STEP_X, AH_PAUSE_UI_COORD_SCALE);

				inst->matrix.t[1] = UI_ConvertY_2(crystalRowY + AH_PAUSE_ROW_ICON_Y, AH_PAUSE_UI_COORD_SCALE);

				member->iconIndex = AH_PAUSE_ICON_FIRST_TOKEN + mdLev->ctrTokenGroupID;
				member->unlockFlags |= CHECK_ADV_BIT(adv->rewards, hubID + ADV_REWARD_PURPLE_TOKEN_HUB_ID_BASE);
			}
		}
	}

	else if (type == AH_PAUSE_PAGE_TOKEN_TOTALS)
	{
		s16 tokenTotals[AH_PAUSE_TOKEN_ICON_COUNT] = {0, 0, 0, 0, 0};

		for (s32 i = 0; i < AH_PAUSE_CTR_TOKEN_TRACK_COUNT; i++)
		{
			if (CHECK_ADV_BIT(adv->rewards, i + ADV_REWARD_FIRST_CTR_TOKEN))
			{
				tokenTotals[data.metaDataLEV[i].ctrTokenGroupID]++;
			}
		}

		// NOTE(aalhendi): Purple tokens are stored in a separate reward bit range not in ctrTokenGroupID.
		for (s32 i = 0; i < AH_PAUSE_PURPLE_TOKEN_COUNT; i++)
		{
			if (CHECK_ADV_BIT(adv->rewards, i + ADV_REWARD_FIRST_PURPLE_TOKEN))
			{
				tokenTotals[AH_PAUSE_PURPLE_TOKEN_INDEX]++;
			}
		}

		for (s32 i = 0; i < AH_PAUSE_TOKEN_ICON_COUNT; i++)
		{
			s16 instPosX = posX + AH_PAUSE_TOKEN_TOTAL_X_BASE + ((i - AH_PAUSE_TOKEN_TOTAL_CENTER_INDEX) * AH_PAUSE_TOKEN_TOTAL_SPACING_X);
			s16 instPosY = (i & 1) * AH_PAUSE_TOKEN_TOTAL_ROW_Y_STEP;
			struct AHPauseMember *member = &ptrPauseObject->members[i];

			member->iconIndex = AH_PAUSE_ICON_FIRST_TOKEN + i;
			member->unlockFlags |= AH_PAUSE_MEMBER_UNLOCKED;

			struct Instance *inst = member->inst;

			// Remove SelectProfile with regular UI variant
			inst->matrix.t[0] = UI_ConvertX_2(instPosX, AH_PAUSE_UI_COORD_SCALE);

			inst->matrix.t[1] = UI_ConvertY_2(instPosY + AH_PAUSE_TOKEN_TOTAL_ICON_Y, AH_PAUSE_UI_COORD_SCALE);

			SelectProfile_PrintInteger(tokenTotals[i], instPosX + AH_PAUSE_TOKEN_TOTAL_NUMBER_X, instPosY + AH_PAUSE_TOKEN_TOTAL_NUMBER_Y, 0, 0);

			s32 strX = AH_PAUSE_X_GLYPH; //"X\0\0" + nullterm
			DecalFont_DrawLine((char *)&strX, instPosX + AH_PAUSE_TOKEN_TOTAL_X_GLYPH_X, instPosY + AH_PAUSE_TOKEN_TOTAL_X_GLYPH_Y, FONT_SMALL, 0);
		}
	}

	else if (type == AH_PAUSE_PAGE_RELIC_TOTALS)
	{
		char totalString[32];
		s16 relicTotals[AH_PAUSE_RELIC_ICON_COUNT] = {0, 0, 0};

		for (s32 i = 0; i < AH_PAUSE_RELIC_TRACK_COUNT; i++)
		{
			// platinum
			if (CHECK_ADV_BIT(adv->rewards, i + ADV_REWARD_FIRST_PLATINUM_RELIC))
			{
				relicTotals[2]++;
			}
			// gold
			else if (CHECK_ADV_BIT(adv->rewards, i + ADV_REWARD_FIRST_GOLD_RELIC))
			{
				relicTotals[1]++;
			}
			// sapphire
			else if (CHECK_ADV_BIT(adv->rewards, i + ADV_REWARD_FIRST_SAPPHIRE_RELIC))
			{
				relicTotals[0]++;
			}
		}

		for (s32 i = 0; i < AH_PAUSE_RELIC_ICON_COUNT; i++)
		{
			s16 instPosX = posX + AH_PAUSE_RELIC_TOTAL_X_BASE + ((i - AH_PAUSE_RELIC_TOTAL_CENTER_INDEX) * AH_PAUSE_RELIC_TOTAL_SPACING_X);
			struct AHPauseMember *member = &ptrPauseObject->members[i];

			member->iconIndex = AH_PAUSE_ICON_SAPPHIRE_RELIC + i;
			member->unlockFlags |= AH_PAUSE_MEMBER_UNLOCKED;

			struct Instance *inst = member->inst;

			// Remove SelectProfile with regular UI variant
			inst->matrix.t[0] = UI_ConvertX_2(instPosX, AH_PAUSE_UI_COORD_SCALE);

			inst->matrix.t[1] = UI_ConvertY_2(AH_PAUSE_RELIC_TOTAL_Y, AH_PAUSE_UI_COORD_SCALE);

			SelectProfile_PrintInteger(relicTotals[i], instPosX + AH_PAUSE_RELIC_TOTAL_NUMBER_X, AH_PAUSE_RELIC_TOTAL_Y, 0, 0);

			s32 strX = AH_PAUSE_X_GLYPH; //"X\0\0" + nullterm
			DecalFont_DrawLine((char *)&strX, instPosX + AH_PAUSE_RELIC_TOTAL_X_GLYPH_X, AH_PAUSE_RELIC_TOTAL_X_GLYPH_Y, FONT_SMALL, 0);
		}

		relicTotal = relicTotals[0] + relicTotals[1] + relicTotals[2];

		sprintf(totalString, "%s %d", sdata->lngStrings[LNG_TOTAL], relicTotal);

		DecalFont_DrawLine(totalString, posX + AH_PAUSE_TITLE_CENTER_X, AH_PAUSE_TOTAL_TEXT_Y, FONT_BIG, 0xffff8000);
	}

	s32 titleFrameTextWidth = DecalFont_GetLineWidth(titleString, FONT_BIG);

	s32 titleFrameWidth = titleFrameTextWidth + AH_PAUSE_TITLE_FRAME_PAD_W;
	if ((s16)titleFrameTextWidth < AH_PAUSE_TITLE_FRAME_MIN_TEXT_W)
	{
		titleFrameWidth = AH_PAUSE_TITLE_FRAME_W;
	}

	titleHalfWidth = titleFrameWidth >> 1;

	r.x = AH_PAUSE_OUTER_RECT_CENTER_X - titleHalfWidth;
	r.y = AH_PAUSE_OUTER_RECT_Y;
	r.w = (s16)titleFrameWidth - AH_PAUSE_TITLE_FRAME_PAD_W;
	r.h = AH_PAUSE_OUTER_RECT_H;

	Color color;
	color.self = sdata->battleSetup_Color_UI_1;
	u32 *ot = gGT->backBuffer->otMem.uiOT;
	RECTMENU_DrawOuterRect_Edge(&r, color, AH_PAUSE_OUTER_EDGE_ALPHA, ot);

	r.x = AH_PAUSE_INNER_RECT_CENTER_X - titleHalfWidth;
	r.y = AH_PAUSE_INNER_RECT_Y;
	r.w = (s16)titleFrameWidth;
	r.h = AH_PAUSE_INNER_RECT_H;

	// Draw 2D Menu rectangle background
	RECTMENU_DrawInnerRect(&r, AH_PAUSE_INNER_RECT_COLOR, &ot[AH_PAUSE_INNER_RECT_OT_OFFSET]);

	for (s32 i = 0; i < AH_PAUSE_MEMBER_COUNT; i++)
	{
		struct AHPauseMember *member = &ptrPauseObject->members[i];
		s32 index = member->iconIndex;

		struct Instance *inst = member->inst;
		SVec3 *rot = &member->rot;

		if (index < 0)
		{
			// make invisible
			inst->flags |= HIDE_MODEL;
		}
		else
		{
			inst->flags &= ~AH_PAUSE_INSTANCE_DYNAMIC_FLAGS;
			inst->flags |= D232.advPauseInst[index].instFlags;

			if ((member->unlockFlags & AH_PAUSE_MEMBER_UNLOCKED) == 0)
			{
				inst->flags &= ~AH_PAUSE_INSTANCE_DYNAMIC_FLAGS;
				inst->colorRGBA = 0;
				inst->alphaScale = AH_PAUSE_LOCKED_ALPHA;
			}

			else
			{
				u8 *ptrColor = (u8 *)&D232.advPauseInst[index].color;

				inst->alphaScale = 0;
				inst->colorRGBA = INST_PackColorRGB(ptrColor[0], ptrColor[1], ptrColor[2]);
			}

			s32 scale = D232.advPauseInst[index].scale;

			if (type == AH_PAUSE_PAGE_TOKEN_TOTALS)
			{
				scale = AH_PAUSE_TOKEN_TOTAL_SCALE;
			}
			else if (type != AH_PAUSE_PAGE_HUB)
			{
				scale = scale << AH_PAUSE_NON_HUB_SCALE_SHIFT;
			}

			inst->scale.x = scale;
			inst->scale.y = scale;
			inst->scale.z = scale;

			s32 modelID = D232.advPauseInst[index].modelID;

			inst->model = gGT->modelPtr[modelID];

			ConvertRotToMatrix(&inst->matrix, rot);

			if ((inst->flags & (DRAW_BILLBOARD | DRAW_TRANSPARENT | USE_SPECULAR_LIGHT)) == USE_SPECULAR_LIGHT)
			{
				SVec3 *specArr = &D232.advPauseInst[index].lightDir;

				Vector_SpecLightSpin2D(inst, rot, specArr);
			}

			else
			{
				inst->colorRGBA = 0;
			}
		}

		rot->y = inst->matrix.t[0] * AH_PAUSE_MODEL_ROT_X_WEIGHT + inst->matrix.t[1] * AH_PAUSE_MODEL_ROT_Y_WEIGHT +
		         sdata->frameCounter * AH_PAUSE_MODEL_ROT_FRAME_WEIGHT;

		rot->y &= AH_PAUSE_MODEL_ROT_MASK;
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b3144-0x800b344c.
void AH_Pause_Update(void)
{
	struct GameTracker *gGT = sdata->gGT;

	if (D232.ptrPauseObject == 0)
	{
		struct PauseObject *ptrPauseObject = &D232.pauseObject;

		D232.pausePageTimer = 0;
		D232.pausePageCurr = gGT->levelID - GEM_STONE_VALLEY;
		gGT->advPausePage = D232.pausePageCurr;

		// 0 = size
		// 0 = no relation to param4
		// 0x300 = SmallStackPool
		// 0xd = "other" thread bucket
		struct Thread *t = PROC_BirthWithObject(SIZE_RELATIVE_POOL_BUCKET(0, NONE, SMALL, OTHER), 0, R232.s_PAUSE, 0);

		D232.ptrPauseObject = ptrPauseObject;
		ptrPauseObject->t = t;

		for (s32 i = 0; i < AH_PAUSE_MEMBER_COUNT; i++)
		{
			struct AHPauseMember *member = &ptrPauseObject->members[i];
			struct Instance *inst = INSTANCE_Birth3D(gGT->modelPtr[STATIC_GEM], R232.s_pause, t);

			member->iconIndex = AH_PAUSE_ICON_NONE;
			member->inst = inst;
			member->rot.x = 0;
			member->rot.y = 0;
			member->rot.z = 0;

			inst->flags |= (USE_SPECULAR_LIGHT | SCREENSPACE_INSTANCE | HIDE_MODEL);

			struct InstDrawPerPlayer *idpp = INST_GETIDPP(inst);

			idpp[0].pushBuffer = &gGT->pushBuffer_UI;
			for (s32 j = 1; j < gGT->numPlyrCurrGame; j++)
			{
				idpp[j].pushBuffer = 0;
			}

			CTR_MatrixSetRotIdentity(&inst->matrix);
			inst->matrix.t[2] = AH_PAUSE_INSTANCE_DEPTH;
		}
	}

	s32 tap = sdata->buttonTapPerPlayer[0];

	if ((tap & (BTN_RIGHT | BTN_LEFT)) != 0)
	{
		if ((tap & BTN_LEFT) != 0)
		{
			D232.pausePageDir = -1;
			gGT->advPausePage += -1;

			if (gGT->advPausePage < 0)
			{
				gGT->advPausePage = AH_PAUSE_MENU_PAGE_COUNT - 1;
			}
		}

		// assume BTN_RIGHT
		else
		{
			D232.pausePageDir = 1;
			gGT->advPausePage += 1;

			if (gGT->advPausePage >= AH_PAUSE_MENU_PAGE_COUNT)
			{
				gGT->advPausePage = 0;
			}
		}

		// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b3340-0x800b3350 for adventure pause page-turn SFX.
		OtherFX_Play(0, 1);
	}

	// page is flipping
	if (D232.pausePageTimer > 0)
	{
		D232.pausePageTimer--;
	}
	// page is not flipping, flip desired
	else if (gGT->advPausePage != D232.pausePageCurr)
	{
		D232.pausePagePrev = D232.pausePageCurr;
		D232.pausePageDir_dup = D232.pausePageDir;

		D232.pausePageTimer = AH_PAUSE_PAGE_FLIP_FRAMES;

		D232.pausePageCurr = gGT->advPausePage;
	}

	s32 pageID;
	s32 posX;

	// second half
	if (D232.pausePageTimer < AH_PAUSE_PAGE_FLIP_SECOND_HALF)
	{
		pageID = D232.pausePageCurr;
		posX = D232.pausePageTimer * D232.pausePageDir * -AH_PAUSE_PAGE_FLIP_X_STEP;
	}

	// first half
	else
	{
		pageID = D232.pausePagePrev;
		posX = (AH_PAUSE_PAGE_FLIP_FRAMES - D232.pausePageTimer) * D232.pausePageDir * AH_PAUSE_PAGE_FLIP_X_STEP;
	}

	AH_Pause_Draw(pageID, posX);
}
