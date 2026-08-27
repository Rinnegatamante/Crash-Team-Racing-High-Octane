#include <common.h>

#if defined(__vita__)
#include "OxideMenuModel.h"
#endif

enum
{
	MM_CHARACTER_SELECT_SCREEN_W = 0x200,
	MM_CHARACTER_SELECT_SCREEN_H = 0xd8,
	MM_CHARACTER_SELECT_DISTANCE_TO_SCREEN = 0x100,
	MM_CHARACTER_SELECT_MODEL_MOVE_FP = 0x1000,
	MM_CHARACTER_SELECT_MODEL_MOVE_FP_SHIFT = 0xc,
	MM_CHARACTER_SELECT_MODEL_MOVE_NEXT = 1,
	MM_CHARACTER_SELECT_MODEL_MOVE_PREV = -1,
#if defined(__vita__)
	MM_CHARACTER_SELECT_ICON_COUNT = 0x10,
#else
	MM_CHARACTER_SELECT_ICON_COUNT = 0xf,
#endif
	MM_CHARACTER_SELECT_EXPANSION_ICON_FIRST = 0xc,
	MM_CHARACTER_SELECT_DEFAULT_DRIVER_COUNT = 8,
	MM_CHARACTER_SELECT_MAX_PLAYERS = 4,
	MM_CHARACTER_SELECT_LIMITED_LAYOUT_OFFSET = 4,
	MM_CHARACTER_SELECT_FULL_LAYOUT_COUNT = 2,
	MM_CHARACTER_SELECT_TRANSITION_FRAMES = 0xc,
	MM_CHARACTER_SELECT_TRANSITION_STEP = 8,
	MM_CHARACTER_SELECT_ANGLE_STEP = 0x400,
	MM_CHARACTER_SELECT_ANGLE_OFFSET = 400,
	MM_CHARACTER_SELECT_SPIN_STEP = 0x40,
	MM_CHARACTER_SELECT_WHEEL_SIZE = 0xccc,
	MM_CHARACTER_SELECT_LAYOUT_3P = 2,
	MM_CHARACTER_SELECT_LAYOUT_4P = 3,
	MM_CHARACTER_SELECT_LAYOUT_1P_LIMITED = 4,
	MM_CHARACTER_SELECT_LAYOUT_2P_LIMITED = 5,
	MM_CHARACTER_SELECT_TITLE_TRANSITION_INDEX = 15,
	MM_CHARACTER_SELECT_DRIVER_WINDOW_TRANSITION_FIRST = 0x10,
	MM_CHARACTER_SELECT_3P_TITLE_X = 0x9c,
	MM_CHARACTER_SELECT_3P_SELECT_Y = 0x14,
	MM_CHARACTER_SELECT_3P_CHARACTER_Y = 0x26,
	MM_CHARACTER_SELECT_4P_TITLE_X = 0xfc,
	MM_CHARACTER_SELECT_4P_SELECT_Y = 8,
	MM_CHARACTER_SELECT_4P_CHARACTER_Y = 0x18,
	MM_CHARACTER_SELECT_LIMITED_TITLE_X = 0xfc,
	MM_CHARACTER_SELECT_LIMITED_TITLE_Y = 10,
	MM_CHARACTER_SELECT_INPUT_DPAD = BTN_RIGHT | BTN_LEFT | BTN_DOWN | BTN_UP,
	MM_CHARACTER_SELECT_INPUT_MENU = BTN_TRIANGLE | BTN_CIRCLE | BTN_SQUARE_one | BTN_CROSS_one,
	MM_CHARACTER_SELECT_INPUT_CONFIRM = BTN_CIRCLE | BTN_CROSS_one,
	MM_CHARACTER_SELECT_INPUT_BACK = BTN_TRIANGLE | BTN_SQUARE_one,
	MM_CHARACTER_SELECT_ICON_DECAL_OFFSET_X = 6,
	MM_CHARACTER_SELECT_ICON_DECAL_OFFSET_Y = 4,
	MM_CHARACTER_SELECT_ICON_RECT_W = 0x34,
	MM_CHARACTER_SELECT_ICON_RECT_H = 0x21,
	MM_CHARACTER_SELECT_CURSOR_LABEL_OFFSET_X = -6,
	MM_CHARACTER_SELECT_CURSOR_LABEL_OFFSET_Y = -3,
	MM_CHARACTER_SELECT_HIGHLIGHT_OFFSET_X = 3,
	MM_CHARACTER_SELECT_HIGHLIGHT_OFFSET_Y = 2,
	MM_CHARACTER_SELECT_HIGHLIGHT_W = 0x2e,
	MM_CHARACTER_SELECT_HIGHLIGHT_H = 0x1d,
	MM_CHARACTER_SELECT_SELECTED_BORDER_COUNT = 2,
	MM_CHARACTER_SELECT_SELECTED_BORDER_INSET_X = 3,
	MM_CHARACTER_SELECT_SELECTED_BORDER_INSET_Y = 2,
	MM_CHARACTER_SELECT_SELECTED_BORDER_SHRINK_W = 6,
	MM_CHARACTER_SELECT_SELECTED_BORDER_SHRINK_H = 4,
	MM_CHARACTER_SELECT_4P_NAME_BOTTOM_OFFSET = -6,
	MM_CHARACTER_SELECT_COLOR_PHASE_FRAME_STEP = 0x100,
	MM_CHARACTER_SELECT_COLOR_PHASE_PLAYER_STEP = 0x400,
	MM_CHARACTER_SELECT_COLOR_TRIG_MASK = 0x3ff,
	MM_CHARACTER_SELECT_COLOR_TRIG_HIGH_HALF_BIT = 0x400,
	MM_CHARACTER_SELECT_COLOR_TRIG_NEGATE_BIT = 0x800,
	MM_CHARACTER_SELECT_COLOR_PULSE_THRESHOLD = 0xc00,
	MM_CHARACTER_SELECT_COLOR_PULSE_SCALE_SHIFT = 7,
	MM_CHARACTER_SELECT_COLOR_PULSE_FP_SHIFT = 0xc,
	MM_CHARACTER_SELECT_1P_WINDOW_X = 0x18,
	MM_CHARACTER_SELECT_1P_WINDOW_W = 0xe0,
	MM_CHARACTER_SELECT_STATS_BOX_X = 0x128,
	MM_CHARACTER_SELECT_STATS_BOX_Y = 0xb,
	MM_CHARACTER_SELECT_STATS_BOX_W = 0xcc,
	MM_CHARACTER_SELECT_STATS_BOX_H = 0x44,
	MM_CHARACTER_SELECT_STATS_CLASS_X = 0x190,
	MM_CHARACTER_SELECT_STATS_CLASS_Y = 0xf,
	MM_CHARACTER_SELECT_STATS_LABEL_X = 0x17e,
	MM_CHARACTER_SELECT_STATS_SPEED_Y = 0x1e,
	MM_CHARACTER_SELECT_STATS_ACCEL_Y = 0x2d,
	MM_CHARACTER_SELECT_STATS_TURN_Y = 0x3c,
	MM_CHARACTER_SELECT_STATS_BAR_X = 0x188,
	MM_CHARACTER_SELECT_STATS_BAR_START_Y = 0x21,
	MM_CHARACTER_SELECT_STATS_BAR_END_Y = 0x28,
	MM_CHARACTER_SELECT_STATS_BAR_SHADOW_Y = 0x22,
	MM_CHARACTER_SELECT_STATS_BAR_HEIGHT = 7,
	MM_CHARACTER_SELECT_STATS_BAR_SHADOW_HEIGHT = 5,
	MM_CHARACTER_SELECT_STATS_BAR_ROW_STEP = 0xf,
	MM_CHARACTER_SELECT_STATS_BAR_SEGMENT_COUNT = 6,
	MM_CHARACTER_SELECT_STATS_BAR_SEGMENT_WIDTH = 0xd,
	MM_CHARACTER_SELECT_STATS_BAR_RATE = 3,
	MM_CHARACTER_SELECT_STATS_BAR_COLOR_CODE = 0x38000000,
};

static const s16 s_nativeCharacterSelectStatTargets[NUM_CLASSES][3] =
{
	{0x37, 0x37, 0x37},
	{0x30, 0x50, 0x20},
	{0x50, 0x20, 0x0a},
	{0x1c, 0x30, 0x50},
};

static const u32 s_nativeCharacterSelectStatColors[7] =
{
	0xc80000,
	0x0a8700,
	0x00b428,
	0x00b4b4,
	0x0064dc,
	0x0028dc,
	0x0000eb,
};

static const s16 s_nativeCharacterSelectClassStrings[3] =
{
	LNG_BEGINNER,
	LNG_INTERMEDIATE,
	LNG_ADVANCED,
};

static SVec2 s_nativeCharacterSelect1PWindowPos;
static s16 s_nativeCharacterSelectStatLengths[3];
static s16 s_nativeCharacterSelectStatCharacterID = -1;

static void MM_Characters_NativeResetStats(void)
{
	s_nativeCharacterSelectStatCharacterID = -1;
	for (s32 i = 0; i < 3; i++)
	{
		s_nativeCharacterSelectStatLengths[i] = 0;
	}
}

static void MM_Characters_NativeDrawStats(void)
{
	struct GameTracker *gGT = sdata->gGT;
	if (gGT->numPlyrNextGame != 1)
	{
		return;
	}

	s16 characterID = data.characterIDs[0];
	struct MetaDataCHAR *mdc = &data.MetaDataCharacters[characterID];
	s32 engineID = mdc->engineID;
	if ((u32)engineID >= NUM_CLASSES)
	{
		engineID = BALANCED;
	}

	if (s_nativeCharacterSelectStatCharacterID != characterID)
	{
		s_nativeCharacterSelectStatCharacterID = characterID;
		for (s32 i = 0; i < 3; i++)
		{
			s_nativeCharacterSelectStatLengths[i] = 0;
		}
	}

	for (s32 i = 0; i < 3; i++)
	{
		s16 target = (characterID == PENTA_PENGUIN) ? 0x50 : s_nativeCharacterSelectStatTargets[engineID][i];
		s16 *length = &s_nativeCharacterSelectStatLengths[i];
		if (*length < target)
		{
			*length += MM_CHARACTER_SELECT_STATS_BAR_RATE;
			if (*length > target)
			{
				*length = target;
			}
		}
		else if (*length > target)
		{
			*length = target;
		}
	}

	struct TransitionMeta *driverTransition =
		&D230.characterSelectTransitionMeta[MM_CHARACTER_SELECT_DRIVER_WINDOW_TRANSITION_FIRST];
	s16 transitionX = driverTransition->currX;
	s16 labelX = MM_CHARACTER_SELECT_STATS_LABEL_X + transitionX;
	s16 barX = MM_CHARACTER_SELECT_STATS_BAR_X + transitionX;

	s32 classIndex = 0;
	if ((characterID == PENTA_PENGUIN) || (engineID == SPEED))
	{
		classIndex = 2;
	}
	else if (engineID < SPEED)
	{
		classIndex = 1;
	}

	char *classText = sdata->lngStrings[s_nativeCharacterSelectClassStrings[classIndex]];
	char *statTexts[3] =
	{
		sdata->lngStrings[LNG_SPEED],
		sdata->lngStrings[LNG_ACCEL],
		sdata->lngStrings[LNG_TURN],
	};

	s32 contentLeft = barX;
	s32 contentRight = barX + CTR_WIDESCREEN_SCALE_X(0x50);
	for (s32 i = 0; i < 3; i++)
	{
		s32 statWidth = DecalFont_GetLineWidth(statTexts[i], FONT_BIG);
		s32 statLeft = labelX - statWidth;
		if (statLeft < contentLeft)
		{
			contentLeft = statLeft;
		}
		if (labelX > contentRight)
		{
			contentRight = labelX;
		}
	}

	s16 classX = (s16)((contentLeft + contentRight) >> 1);
	s32 classWidth = DecalFont_GetLineWidth(classText, FONT_BIG);
	s32 classLeft = classX - (classWidth >> 1);
	s32 classRight = classLeft + classWidth;
	if (classLeft < contentLeft)
	{
		contentLeft = classLeft;
	}
	if (classRight > contentRight)
	{
		contentRight = classRight;
	}

	DecalFont_DrawLine(classText,
	                   classX, MM_CHARACTER_SELECT_STATS_CLASS_Y, FONT_BIG, JUSTIFY_CENTER | ORANGE);
	DecalFont_DrawLine(statTexts[0], labelX, MM_CHARACTER_SELECT_STATS_SPEED_Y,
	                   FONT_BIG, JUSTIFY_RIGHT | ORANGE_RED);
	DecalFont_DrawLine(statTexts[1], labelX, MM_CHARACTER_SELECT_STATS_ACCEL_Y,
	                   FONT_BIG, JUSTIFY_RIGHT | LIME_GREEN);
	DecalFont_DrawLine(statTexts[2], labelX, MM_CHARACTER_SELECT_STATS_TURN_Y,
	                   FONT_BIG, JUSTIFY_RIGHT | BLUE);

	struct PrimMem *primMem = &gGT->backBuffer->primMem;
	Color white = MakeColor(0xff, 0xff, 0xff);
	Color black = MakeColor(0, 0, 0);
	s16 barStartY = MM_CHARACTER_SELECT_STATS_BAR_START_Y;
	s16 barEndY = MM_CHARACTER_SELECT_STATS_BAR_END_Y;
	s16 shadowY = MM_CHARACTER_SELECT_STATS_BAR_SHADOW_Y;

	for (s32 statIndex = 0; statIndex < 3; statIndex++)
	{
		s16 statLength = s_nativeCharacterSelectStatLengths[statIndex];
		s16 drawLength = (s16)CTR_WIDESCREEN_SCALE_X(statLength);
		RECT r;
		r.x = barX;
		r.y = barStartY;
		r.w = drawLength;
		r.h = MM_CHARACTER_SELECT_STATS_BAR_HEIGHT;
		CTR_Box_DrawWireBox(&r, &white, gGT->pushBuffer_UI.ptrOT, primMem);

		r.x = barX + 1;
		r.y = shadowY;
		r.w = drawLength - 2;
		r.h = MM_CHARACTER_SELECT_STATS_BAR_SHADOW_HEIGHT;
		if (r.w > 0)
		{
			CTR_Box_DrawWireBox(&r, &black, gGT->pushBuffer_UI.ptrOT, primMem);
		}

		s32 segmentStart = 0;
		s32 segmentEnd = MM_CHARACTER_SELECT_STATS_BAR_SEGMENT_WIDTH;
		for (s32 segmentIndex = 0;
		     segmentIndex < MM_CHARACTER_SELECT_STATS_BAR_SEGMENT_COUNT;
		     segmentIndex++)
		{
			s16 currSegmentLen = MM_CHARACTER_SELECT_STATS_BAR_SEGMENT_WIDTH;
			if (statLength <= segmentEnd)
			{
				currSegmentLen = statLength - segmentStart;
			}
			if (currSegmentLen < 0)
			{
				currSegmentLen = 0;
			}

			if ((segmentStart + currSegmentLen <= statLength) && (currSegmentLen > 0))
			{
				POLY_G4 *poly = primMem->cursor;
				if (primMem->end < (void *)poly)
				{
					return;
				}
				primMem->cursor = poly + 1;

				u32 color0 = s_nativeCharacterSelectStatColors[segmentIndex];
				u32 color1 = s_nativeCharacterSelectStatColors[segmentIndex + 1];
				CtrGpu_WriteColorCode(&poly->r0, color0 | MM_CHARACTER_SELECT_STATS_BAR_COLOR_CODE);
				CtrGpu_WriteColorCode(&poly->r1, color1 | MM_CHARACTER_SELECT_STATS_BAR_COLOR_CODE);
				CtrGpu_WriteColorCode(&poly->r2, color0 | MM_CHARACTER_SELECT_STATS_BAR_COLOR_CODE);
				CtrGpu_WriteColorCode(&poly->r3, color1 | MM_CHARACTER_SELECT_STATS_BAR_COLOR_CODE);

				s16 left = barX + CTR_WIDESCREEN_SCALE_X(segmentStart);
				s16 right = barX + CTR_WIDESCREEN_SCALE_X(segmentStart + currSegmentLen);
				poly->x0 = left;
				poly->y0 = barStartY;
				poly->x1 = right;
				poly->y1 = barStartY;
				poly->x2 = left;
				poly->y2 = barEndY;
				poly->x3 = right;
				poly->y3 = barEndY;
				addPolyG4(gGT->pushBuffer_UI.ptrOT, poly);
			}

			segmentStart += MM_CHARACTER_SELECT_STATS_BAR_SEGMENT_WIDTH;
			segmentEnd += MM_CHARACTER_SELECT_STATS_BAR_SEGMENT_WIDTH;
		}

		barStartY += MM_CHARACTER_SELECT_STATS_BAR_ROW_STEP;
		barEndY += MM_CHARACTER_SELECT_STATS_BAR_ROW_STEP;
		shadowY += MM_CHARACTER_SELECT_STATS_BAR_ROW_STEP;
	}

	RECT box;
	box.x = contentLeft - 6;
	box.y = MM_CHARACTER_SELECT_STATS_BOX_Y;
	box.w = (contentRight - contentLeft) + 12;
	box.h = MM_CHARACTER_SELECT_STATS_BOX_H;
	RECTMENU_DrawInnerRect(&box, 4, gGT->backBuffer->otMem.uiOT);
}

#if defined(__vita__)
static struct CharacterSelectMeta s_oxideCharacterSelectMeta1P2P[0x10] =
{
	{128, 96, {0, 4, 8, 1}, 0, 0xFFFF},
	{192, 96, {1, 5, 0, 2}, 1, 0xFFFF},
	{256, 96, {2, 6, 1, 3}, 2, 0xFFFF},
	{320, 96, {3, 7, 2, 9}, 3, 0xFFFF},
	{128, 135, {0, 12, 10, 5}, 4, 0xFFFF},
	{192, 135, {1, 13, 4, 6}, 5, 0xFFFF},
	{256, 135, {2, 14, 5, 7}, 6, 0xFFFF},
	{320, 135, {3, 15, 6, 11}, 7, 0xFFFF},
	{64, 96, {8, 10, 8, 0}, 12, 0x5},
	{384, 96, {9, 11, 3, 9}, 8, 0xA},
	{64, 135, {8, 10, 10, 4}, 10, 0x7},
	{384, 135, {9, 15, 7, 11}, 9, 0x8},
	{128, 174, {4, 12, 12, 13}, 11, 0x9},
	{192, 174, {5, 13, 12, 14}, 13, 0x6},
	{256, 174, {6, 14, 13, 15}, 14, 0xB},
	{320, 174, {7, 15, 14, 11}, 15, GAME_UNLOCK_BIT_OXIDE},
};

static struct CharacterSelectMeta s_oxideCharacterSelectMeta3P[0x10] =
{
	{32, 71, {12, 4, 0, 1}, 0, 0xFFFF},
	{96, 71, {13, 5, 0, 2}, 1, 0xFFFF},
	{160, 71, {14, 6, 1, 3}, 2, 0xFFFF},
	{224, 71, {15, 7, 2, 3}, 3, 0xFFFF},
	{32, 110, {0, 8, 4, 5}, 4, 0xFFFF},
	{96, 110, {1, 9, 4, 6}, 5, 0xFFFF},
	{160, 110, {2, 10, 5, 7}, 6, 0xFFFF},
	{224, 110, {3, 11, 6, 7}, 7, 0xFFFF},
	{32, 149, {4, 8, 8, 9}, 12, 0x5},
	{96, 149, {5, 9, 8, 10}, 8, 0xA},
	{160, 149, {6, 10, 9, 11}, 10, 0x7},
	{224, 149, {7, 11, 10, 11}, 9, 0x8},
	{32, 32, {12, 0, 12, 13}, 11, 0x9},
	{96, 32, {13, 1, 12, 14}, 13, 0x6},
	{160, 32, {14, 2, 13, 15}, 14, 0xB},
	{224, 32, {15, 3, 14, 15}, 15, GAME_UNLOCK_BIT_OXIDE},
};

static struct CharacterSelectMeta s_oxideCharacterSelectMeta4P[0x10] =
{
	{128, 71, {0, 4, 10, 1}, 0, 0xFFFF},
	{192, 71, {14, 5, 0, 2}, 1, 0xFFFF},
	{256, 71, {15, 6, 1, 3}, 2, 0xFFFF},
	{320, 71, {3, 7, 2, 11}, 3, 0xFFFF},
	{128, 110, {0, 4, 12, 5}, 4, 0xFFFF},
	{192, 110, {1, 8, 4, 6}, 5, 0xFFFF},
	{256, 110, {2, 9, 5, 7}, 6, 0xFFFF},
	{320, 110, {3, 7, 6, 13}, 7, 0xFFFF},
	{192, 149, {5, 8, 8, 9}, 12, 0x5},
	{256, 149, {6, 9, 8, 9}, 14, 0xB},
	{64, 71, {10, 12, 10, 0}, 10, 0x7},
	{384, 71, {11, 13, 3, 11}, 9, 0x8},
	{64, 110, {10, 12, 12, 4}, 11, 0x9},
	{384, 110, {11, 13, 7, 13}, 8, 0xA},
	{192, 32, {14, 1, 14, 15}, 13, 0x6},
	{256, 32, {15, 2, 14, 15}, 15, GAME_UNLOCK_BIT_OXIDE},
};

static struct Model *s_oxideCharacterSelectModel;

internal struct Model *MM_Characters_GetOxideMenuModel(void)
{
	if (s_oxideCharacterSelectModel == NULL)
	{
		u8 *fileBuf = s_oxideMenuModelFile;
		s32 ptrMapOffset = (s32)CTR_ReadU32LE(fileBuf);
		u8 *modelBuf = fileBuf + sizeof(u32);
		struct DramPointerMap *ptrMap = (struct DramPointerMap *)(modelBuf + ptrMapOffset);

		LOAD_RunPtrMap((char *)modelBuf, DRAM_GETOFFSETS(ptrMap), ptrMap->numBytes >> DRAM_POINTER_MAP_WORD_SHIFT);

		s_oxideCharacterSelectModel = (struct Model *)modelBuf;

		if ((s_oxideCharacterSelectModel->headers != NULL) && (s_oxideCharacterSelectModel->numHeaders > 0))
		{
			struct ModelHeader *header = &s_oxideCharacterSelectModel->headers[0];
			header->scale.x = (header->scale.x * 5) >> 3;
			header->scale.y = (header->scale.y * 5) >> 3;
			header->scale.z = (header->scale.z * 5) >> 3;
		}
	}

	return s_oxideCharacterSelectModel;
}

internal struct CharacterSelectMeta *MM_Characters_GetOxideMetaForLayout(s32 layoutIndex)
{
	switch (layoutIndex)
	{
	case 0:
	case 1:
		return s_oxideCharacterSelectMeta1P2P;
	case MM_CHARACTER_SELECT_LAYOUT_3P:
		return s_oxideCharacterSelectMeta3P;
	case MM_CHARACTER_SELECT_LAYOUT_4P:
		return s_oxideCharacterSelectMeta4P;
	default:
		return D230.characterSelectMetaByLayout[layoutIndex];
	}
}
#endif

// NOTE(aalhendi): ASM-verified NTSC-U 926 overlay 230 0x800ad98c-0x800ada4c.
void MM_Characters_AnimateColors(u8 *colorData, s16 playerID, s16 flag)
{
	u8 colorAdjustmentValue;
	u32 trigApproximationIndex;
	u32 trigApprox;

	// access int RGBA as a char array,
	// for editing components of color
	u8 *ptrColor = (u8 *)data.ptrColor[playerID + PLAYER_BLUE];

	trigApprox = 0;

	// if player has not selected character yet
	// see MM_Characters_MenuProc
	if (flag == 0)
	{
		trigApproximationIndex = sdata->frameCounter * MM_CHARACTER_SELECT_COLOR_PHASE_FRAME_STEP + playerID * MM_CHARACTER_SELECT_COLOR_PHASE_PLAYER_STEP;

		// approximate trigonometry
		trigApprox = CTR_ReadU32LE(&data.trigApprox[trigApproximationIndex & MM_CHARACTER_SELECT_COLOR_TRIG_MASK]);

		if ((trigApproximationIndex & MM_CHARACTER_SELECT_COLOR_TRIG_HIGH_HALF_BIT) == 0)
		{
			trigApprox = trigApprox << 0x10;
		}
		trigApprox = trigApprox >> 0x10;

		if ((trigApproximationIndex & MM_CHARACTER_SELECT_COLOR_TRIG_NEGATE_BIT) != 0)
		{
			trigApprox = -(int)trigApprox;
		}
	}

	colorAdjustmentValue = 0;
	if (MM_CHARACTER_SELECT_COLOR_PULSE_THRESHOLD < (int)trigApprox)
	{
		colorAdjustmentValue = ((trigApprox << MM_CHARACTER_SELECT_COLOR_PULSE_SCALE_SHIFT) >> MM_CHARACTER_SELECT_COLOR_PULSE_FP_SHIFT);
	}

	colorData[0] = ptrColor[0] | colorAdjustmentValue;
	colorData[1] = ptrColor[1] | colorAdjustmentValue;
	colorData[2] = ptrColor[2] | colorAdjustmentValue;
	colorData[3] = 0;

	return;
}

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 230 0x800ada4c-0x800adae4.
int MM_Characters_GetNextDriver(s16 direction, s16 characterID)
{
	u8 nextIcon = D230.activeCharacterSelectMeta[(s32)characterID].nextIconByDirection[direction];
	s16 unlocked = D230.activeCharacterSelectMeta[(s32)nextIcon].unlockFlags;

	// set new driver to the driver
	// you'd get when pressing Up button
	s16 newDriver = nextIcon;

	if (
	    // if desired driver is not unlocked by default
	    (unlocked != MM_CHARACTER_UNLOCK_ALWAYS) &&

	    !CHECK_ADV_BIT(sdata->gameProgress.unlocks, unlocked))
	{
		// set new driver to the driver you already have
		newDriver = characterID;
	}

	// return new driver
	return newDriver;
}

// used for preventing players highlighting the same character
// also for when you go left of komodo joe's icon
// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 230 0x800adae4-0x800adb64.
b32 MM_Characters_boolIsInvalid(s16 *iconPerPlayer, s16 characterID, s16 player)
{
	// if there are players
	if (sdata->gGT->numPlyrNextGame)
	{
		// loop through players
		for (s16 playerIndex = 0; playerIndex < sdata->gGT->numPlyrNextGame; playerIndex++)
		{
			// if driver is taken
			if ((playerIndex != player) && (characterID == iconPerPlayer[playerIndex]))
			{
				return 1;
			}
		}
	}

	// if driver is not taken
	return 0;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 overlay 230 0x800adb64-0x800adc0c.
// Search for character model by string,
// specific to main menu lev, altered in oxide mod
struct Model *MM_Characters_GetModelByName(const char *name)
{
	struct Model **models;
	struct Model *model;
	struct Level *level1 = sdata->gGT->level1;

	// if LEV is invalid
	if (level1 == NULL)
	{
		return NULL;
	}

	models = level1->ptrModelsPtrArray;
	if (models == NULL)
	{
		return NULL;
	}

	// loop through all models in array
	// of model pointers, until nullptr
	for (model = models[0]; model != NULL; models++, model = models[0])
	{
		if ((ModelName_ReadWord(model->name, 0) == ModelName_ReadWord(name, 0)) && (ModelName_ReadWord(model->name, 1) == ModelName_ReadWord(name, 1)) &&
		    (ModelName_ReadWord(model->name, 2) == ModelName_ReadWord(name, 2)) && (ModelName_ReadWord(model->name, 3) == ModelName_ReadWord(name, 3)))
		{
			// found it
			return model;
		}
	}

#if defined(__vita__)
	if ((ModelName_ReadWord(name, 0) == ModelName_ReadWord(data.MetaDataCharacters[NITROS_OXIDE].name_Debug, 0)) &&
	    (ModelName_ReadWord(name, 1) == ModelName_ReadWord(data.MetaDataCharacters[NITROS_OXIDE].name_Debug, 1)) &&
	    (ModelName_ReadWord(name, 2) == ModelName_ReadWord(data.MetaDataCharacters[NITROS_OXIDE].name_Debug, 2)) &&
	    (ModelName_ReadWord(name, 3) == ModelName_ReadWord(data.MetaDataCharacters[NITROS_OXIDE].name_Debug, 3)))
	{
		return MM_Characters_GetOxideMenuModel();
	}
#endif

	return NULL;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 overlay 230 0x800adc0c-0x800ae0bc PSX path.
void MM_Characters_DrawWindows(b32 boolShowDrivers)
{
	struct GameTracker *gGT = sdata->gGT;
	SVec3 rot;

	if (boolShowDrivers != 0)
	{
		// enable drawing wheels
		gGT->renderFlags |= RENDER_FLAG_TIRES;
	}

	for (s32 playerIndex = 0; playerIndex < gGT->numPlyrNextGame; playerIndex++)
	{
		SVec2 *windowPos = &D230.activeCharacterSelectWindowPos[playerIndex];
		struct TransitionMeta *tMeta = &D230.characterSelectTransitionMeta[playerIndex];

		struct PushBuffer *pb = &gGT->pushBuffer[playerIndex];
		pb->rect.x = windowPos->x + tMeta[MM_CHARACTER_SELECT_DRIVER_WINDOW_TRANSITION_FIRST].currX;
		pb->rect.y = windowPos->y + tMeta[MM_CHARACTER_SELECT_DRIVER_WINDOW_TRANSITION_FIRST].currY;
		pb->rect.w = D230.characterSelectWindowWidth;
		pb->rect.h = D230.characterSelectWindowHeight;

		// negative StartX
		if ((s16)pb->rect.x < 0)
		{
			pb->rect.w -= pb->rect.x;
			pb->rect.x = 0;
			if ((s16)pb->rect.w < 0)
			{
				pb->rect.w = 0;
			}
		}

		// negative StartY
		if ((s16)pb->rect.y < 0)
		{
			pb->rect.h -= pb->rect.y;
			pb->rect.y = 0;
			if ((s16)pb->rect.h < 0)
			{
				pb->rect.h = 0;
			}
		}

		// startX + sizeX out of bounds
		if ((MM_CHARACTER_SELECT_SCREEN_W < pb->rect.x + pb->rect.w) && (pb->rect.w = MM_CHARACTER_SELECT_SCREEN_W - pb->rect.x, pb->rect.w < 0))
		{
			pb->rect.x = MM_CHARACTER_SELECT_SCREEN_W;
			pb->rect.w = 0;

#ifdef CTR_NATIVE
			// NOTE(aalhendi): Native renderer guard; retail leaves w at zero.
			pb->rect.w = 1;
#endif
		}

		// startY + sizeY out of bounds
		if ((MM_CHARACTER_SELECT_SCREEN_H < pb->rect.y + pb->rect.h) && (pb->rect.h = MM_CHARACTER_SELECT_SCREEN_H - pb->rect.y, pb->rect.h < 0))
		{
			pb->rect.y = MM_CHARACTER_SELECT_SCREEN_H;
			pb->rect.h = 0;

#ifdef CTR_NATIVE
			// NOTE(aalhendi): Native renderer guard; retail leaves h at zero.
			pb->rect.h = 1;
#endif
		}

		// distanceToScreen
		pb->distanceToScreen_CURR = MM_CHARACTER_SELECT_DISTANCE_TO_SCREEN;
		pb->distanceToScreen_PREV = MM_CHARACTER_SELECT_DISTANCE_TO_SCREEN;

		// pushBuffer pos and rot to all zero
		pb->pos.x = 0;
		pb->pos.y = 0;
		pb->pos.z = 0;
		pb->rot.x = 0;
		pb->rot.y = 0;
		pb->rot.z = 0;

		// player -> instance
		struct Instance *driverInst = gGT->drivers[playerIndex]->instSelf;

		// Make Visible
		driverInst->flags &= ~HIDE_MODEL;

		// if driver is off-screen
		if ((gGT->numPlyrNextGame <= playerIndex) || (boolShowDrivers == 0))
		{
			// invisible
			driverInst->flags |= HIDE_MODEL;
		}

		struct InstDrawPerPlayer *idpp = INST_GETIDPP(driverInst);

		// clear pushBuffer in every InstDrawPerPlayer
		idpp[0].pushBuffer = 0;
		idpp[1].pushBuffer = 0;
		idpp[2].pushBuffer = 0;
		idpp[3].pushBuffer = 0;

		// set pushBuffer in InstDrawPerPlayer,
		// so that each camera can only see one driver
		idpp[playerIndex].pushBuffer = pb;

		s16 *currCharacterID = &D230.characterSelectPlayerState.currentCharacterID[playerIndex];

#if defined(__vita__)
		gGT->drivers[playerIndex]->wheelSize = (*currCharacterID == NITROS_OXIDE) ? 0 : MM_CHARACTER_SELECT_WHEEL_SIZE;
#endif

		driverInst->animFrame = 0;
		driverInst->animIndex = 0;

		struct Model *model = MM_Characters_GetModelByName(data.MetaDataCharacters[(int)*currCharacterID].name_Debug);

		// set modelPtr in Instance
		driverInst->model = model;

		// CameraDC, freecam mode
		gGT->cameraDC[playerIndex].cameraMode = CAMERA_MODE_FREECAM;

		// Set position of player
		driverInst->matrix.t[0] = D230.characterSelectDriverModel.pos.x;
		driverInst->matrix.t[1] = D230.characterSelectDriverModel.pos.y;
		driverInst->matrix.t[2] = D230.characterSelectDriverModel.pos.z;

		s16 *moveTimer = &D230.characterSelectModelMoveTimer[playerIndex];
		s16 nextMoveTimer = *moveTimer + -1;

		// If no transition between players
		if (*moveTimer == 0)
		{
			// compare to character ID
			if (*currCharacterID != data.characterIDs[playerIndex])
			{
				*moveTimer = D230.characterSelectDriverModel.moveFrames << 1;
				D230.characterSelectPlayerState.desiredCharacterID[playerIndex] = data.characterIDs[playerIndex];
			}
		}

		// if transition between players
		else
		{
			// get timer
			*moveTimer = nextMoveTimer;

			s32 slideDirection;
			s32 slideOffset;

			// if timer is before midpoint
			if ((int)nextMoveTimer < (int)D230.characterSelectDriverModel.moveFrames)
			{
				// make driver fly off screen
				*currCharacterID = D230.characterSelectPlayerState.desiredCharacterID[playerIndex];
				s32 moveFrameScale = RaceFlag_MoveModels((int)nextMoveTimer, (int)D230.characterSelectDriverModel.moveFrames);

				// direction moving
				slideDirection = -D230.characterSelectPlayerState.modelMoveDir[playerIndex];
				slideOffset = moveFrameScale * D230.characterSelectDriverModel.slideDistance >> MM_CHARACTER_SELECT_MODEL_MOVE_FP_SHIFT;
			}

			// if timer is after midpoint
			else
			{
				// make new driver fly on screen
				s32 moveFrameScale =
				    RaceFlag_MoveModels((int)nextMoveTimer - (int)D230.characterSelectDriverModel.moveFrames, (int)D230.characterSelectDriverModel.moveFrames);

				// direction moving
				slideDirection = D230.characterSelectPlayerState.modelMoveDir[playerIndex];
				slideOffset = (MM_CHARACTER_SELECT_MODEL_MOVE_FP - moveFrameScale) * (int)D230.characterSelectDriverModel.slideDistance >>
				              MM_CHARACTER_SELECT_MODEL_MOVE_FP_SHIFT;
			}

			driverInst->matrix.t[0] += slideDirection * slideOffset;
		}

		// driver rotation
		rot.x = D230.characterSelectDriverModel.rot.x;
		rot.y = D230.characterSelectDriverModel.rot.y + D230.characterSelectPlayerState.angle[playerIndex];
		rot.z = D230.characterSelectDriverModel.rot.z;

		ConvertRotToMatrix(&driverInst->matrix, &rot);
	}
	return;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 overlay 230 0x800ae0bc-0x800ae274.
void MM_Characters_SetMenuLayout(void)
{
	b32 expandRoster = false;

	// By default, draw "Select character" in 3P menu
	D230.characterSelectRosterExpanded = 0;

	s32 numPlyrNextGame = sdata->gGT->numPlyrNextGame;
	s32 layoutIndex = numPlyrNextGame - 1;

	// Loop through bottom characters,
	// if any are unlocked, use expanded
#if defined(__vita__)
	expandRoster = true;
#else
	for (s32 iconIndex = MM_CHARACTER_SELECT_EXPANSION_ICON_FIRST; iconIndex < MM_CHARACTER_SELECT_ICON_COUNT; iconIndex++)
	{
		// OG game code
		u16 unlocked = D230.characterSelectMeta1P2P[iconIndex].unlockFlags;

		if (CHECK_ADV_BIT(sdata->gameProgress.unlocks, unlocked))
		{
			expandRoster = true;
			break;
		}
	}
#endif

	if (
	    // if 1P2P (0 or 1)
	    (layoutIndex < MM_CHARACTER_SELECT_FULL_LAYOUT_COUNT) &&

	    // if very few characters are unlocked
	    (!expandRoster))
	{
		// layout [4] and [5] for 1P2P without expansion
		layoutIndex += MM_CHARACTER_SELECT_LIMITED_LAYOUT_OFFSET;
	}

	D230.characterSelectRosterExpanded = expandRoster;

	D230.characterSelectLayoutIndex = layoutIndex;

	D230.characterSelectDriverModel.pos.y = D230.characterSelectLayout.driverPosY[layoutIndex];
	D230.characterSelectDriverModel.pos.z = D230.characterSelectLayout.driverPosZ[layoutIndex];

	D230.characterSelectWindowWidth = D230.characterSelectLayout.windowW[layoutIndex];
	D230.characterSelectWindowHeight = D230.characterSelectLayout.windowH[layoutIndex];

	D230.activeCharacterSelectWindowPos = D230.characterSelectWindowPosByLayout[layoutIndex];

	if (numPlyrNextGame == 1)
	{
		s_nativeCharacterSelect1PWindowPos = D230.activeCharacterSelectWindowPos[0];
		s_nativeCharacterSelect1PWindowPos.x = MM_CHARACTER_SELECT_1P_WINDOW_X;
		D230.activeCharacterSelectWindowPos = &s_nativeCharacterSelect1PWindowPos;
		D230.characterSelectWindowWidth = MM_CHARACTER_SELECT_1P_WINDOW_W;
	}

#if defined(__vita__)
	D230.activeCharacterSelectMeta = MM_Characters_GetOxideMetaForLayout(layoutIndex);
#else
	D230.activeCharacterSelectMeta = D230.characterSelectMetaByLayout[layoutIndex];
#endif

	D230.characterSelectNameTextY = D230.characterSelectLayout.textY[layoutIndex];

	D230.characterSelectTransitionMeta = D230.characterSelectTransitionByPlayerCount[numPlyrNextGame - 1];

	return;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ae274-0x800ae2c0.
void MM_Characters_BackupIDs(void)
{
	for (s32 driverIndex = 0; driverIndex < MM_CHARACTER_SELECT_DEFAULT_DRIVER_COUNT; driverIndex++)
	{
		// make a backup when you leave character selection,
		// backup is restored when you go back to selection
		sdata->characterIDs_backup[driverIndex] = data.characterIDs[driverIndex];
	}
	return;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 overlay 230 0x800ae2c0-0x800ae464.
void MM_Characters_PreventOverlap(void)
{
	struct GameTracker *gGT = sdata->gGT;
	s8 availableDefaultCharacters[MM_CHARACTER_SELECT_DEFAULT_DRIVER_COUNT];

	// default 0,1,2,3,4,5,6,7
	CTR_WriteU32LE((u8 *)&availableDefaultCharacters[0], R230.packedDefaultCharacterIDWords[0]);
	CTR_WriteU32LE((u8 *)&availableDefaultCharacters[4], R230.packedDefaultCharacterIDWords[1]);

	for (s32 playerIndex = 0; playerIndex < gGT->numPlyrNextGame; playerIndex++)
	{
		// get character ID
		s32 characterID = data.characterIDs[playerIndex];

		// if not a secret character
		if (characterID < MM_CHARACTER_SELECT_DEFAULT_DRIVER_COUNT)
		{
			// character is taken
			availableDefaultCharacters[characterID] = -1;
		}
	}

	for (s32 playerIndex = 1; playerIndex < gGT->numPlyrNextGame; playerIndex++)
	{
		for (s32 previousPlayer = 0; previousPlayer < playerIndex; previousPlayer++)
		{
			// if two characters are the same
			if (data.characterIDs[playerIndex] == data.characterIDs[previousPlayer])
			{
				// look for a new character
				for (s32 defaultIndex = 0; defaultIndex < MM_CHARACTER_SELECT_DEFAULT_DRIVER_COUNT; defaultIndex++)
				{
					// get default character
					s8 *defaultCharacter = &availableDefaultCharacters[defaultIndex];
					s8 freeCharacter = *defaultCharacter;

					// if character is not taken
					if (-1 < freeCharacter)
					{
						// assign free character
						data.characterIDs[playerIndex] = (s16)freeCharacter;

						// character is now taken
						*defaultCharacter = -1;

						break;
					}
				}
			}
		}
	}
	return;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 overlay 230 0x800ae464-0x800ae6b0.
void MM_Characters_RestoreIDs(void)
{
	struct GameTracker *gGT = sdata->gGT;

	MM_Characters_NativeResetStats();

	// erase select bits
	sdata->characterSelectFlags = 0;
	D230.characterSelectTransitionFrame = MM_CHARACTER_SELECT_TRANSITION_FRAMES;
	D230.characterSelectMenuState = ENTERING_MENU;

	// This uses 80086e84, which controls character IDs
	for (s32 driverIndex = 0; driverIndex < MM_CHARACTER_SELECT_DEFAULT_DRIVER_COUNT; driverIndex++)
	{
		// set character ID to the last ID you entered
		data.characterIDs[driverIndex] = sdata->characterIDs_backup[driverIndex];
	}

	MM_Characters_SetMenuLayout();

	for (s32 iconIndex = 0; iconIndex < MM_CHARACTER_SELECT_ICON_COUNT; iconIndex++)
	{
		// would not need this if CSM was sorted
		// by order of character ID

		// Basically sets them to 0, 1, 2, 3, 4... up to 0xE,
		// setting Oxide's manually to 0xF is needed to make his icon appear

		D230.characterMenuID[(s32)D230.activeCharacterSelectMeta[iconIndex].characterID] = iconIndex;
	}

	for (s32 playerIndex = 0; playerIndex < gGT->numPlyrNextGame; playerIndex++)
	{
		// Determine if this icon is unlocked (and drawing)

		// get character ID
		s16 *currID = &data.characterIDs[playerIndex];

		// get unlock requirement for this character
		s16 unlocked = D230.activeCharacterSelectMeta[(s32)*currID].unlockFlags;

		if (
		    // If Icon has an unlock requirement
		    (unlocked != MM_CHARACTER_UNLOCK_ALWAYS) &&

		    // If Character is Locked
		    !CHECK_ADV_BIT(sdata->gameProgress.unlocks, unlocked))
		{
			// change character to Crash
			*currID = CRASH_BANDICOOT;
		}
	}

	MM_Characters_PreventOverlap();

	for (s32 playerIndex = 0; playerIndex < gGT->numPlyrNextGame; playerIndex++)
	{
		// set name string ID to the character ID of each player.
		// The string will only draw if both these variables match
		D230.characterSelectPlayerState.currentCharacterID[playerIndex] = data.characterIDs[playerIndex];
		D230.characterSelectPlayerState.desiredCharacterID[playerIndex] = data.characterIDs[playerIndex];

		// something to do with transitioning between icons
		D230.characterSelectModelMoveTimer[playerIndex] = 0;

		// rotation of each driver, 90 degrees difference
		D230.characterSelectPlayerState.angle[playerIndex] = (playerIndex * MM_CHARACTER_SELECT_ANGLE_STEP) + MM_CHARACTER_SELECT_ANGLE_OFFSET;
	}

	MM_Characters_DrawWindows(0);
	return;
}

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 230 0x800ae6b0-0x800ae74c.
void MM_Characters_HideDrivers(void)
{
	struct GameTracker *gGT = sdata->gGT;

	for (s32 playerIndex = 0; playerIndex < MM_CHARACTER_SELECT_MAX_PLAYERS; playerIndex++)
	{
		PushBuffer_Init(&gGT->pushBuffer[playerIndex], 0, 1);

		gGT->drivers[playerIndex]->instSelf->flags |= HIDE_MODEL;
	}

	return;
}

void MM_Characters_MenuProc(struct RectMenu *unused)
{
	(void)unused;
	b32 candidateInUseByOtherPlayer;
	b32 deadEndCandidateAvailable;
	s16 nextIcon;
	int intermediateIcon;
	s16 previousCandidateIcon;
	int nextIconCopy;
	s16 alternateIcon;
	s16 iconPerPlayer[4];

	RECT drawRect;

	s16 hitNavigationDeadEnd;

	int direction;

	struct GameTracker *gGT = sdata->gGT;

	u32 *ot = gGT->backBuffer->otMem.uiOT;

	for (s32 playerIndex = 0; playerIndex < MM_CHARACTER_SELECT_MAX_PLAYERS; playerIndex++)
	{
		iconPerPlayer[playerIndex] = D230.characterMenuID[data.characterIDs[playerIndex]];
	}

	// if menu is not in focus
	if (D230.characterSelectMenuState != IN_MENU)
	{
		MM_TransitionInOut(D230.characterSelectTransitionMeta, (int)D230.characterSelectTransitionFrame, MM_CHARACTER_SELECT_TRANSITION_STEP);
	}

	MM_Characters_SetMenuLayout();
	MM_Characters_DrawWindows(1);

	// if transitioning in
	if (D230.characterSelectMenuState == ENTERING_MENU)
	{
		// if no more frames
		if (D230.characterSelectTransitionFrame == 0)
		{
			// menu is now in focus
			D230.characterSelectMenuState = IN_MENU;
		}
		else
		{
			D230.characterSelectTransitionFrame--;
		}
	}

	// if transitioning out
	if (D230.characterSelectMenuState == EXITING_MENU)
	{
		// increase frame
		D230.characterSelectTransitionFrame++;

		// if more than 12 frames
		if (D230.characterSelectTransitionFrame > MM_CHARACTER_SELECT_TRANSITION_FRAMES)
		{
			// Make a backup of the characters
			// you selected in character selection screen
			MM_Characters_BackupIDs();

			// if returning to main menu
			if (D230.characterSelectExitsForward == 0)
			{
				MM_JumpTo_Title_Returning();
				MM_Characters_HideDrivers();
				return;
			}

			MM_Characters_HideDrivers();

			if (gNativeBossFightMode != 0)
			{
				MM_NativeBossFight_OpenBossSelect();
				return;
			}

			// if you are in a cup
			if ((gGT->gameMode2 & CUP_ANY_KIND) != 0)
			{
				sdata->ptrDesiredMenu = &D230.menuCupSelect;
				MM_CupSelect_Init();
				return;
			}

			// if going to track selection
			sdata->ptrDesiredMenu = &D230.menuTrackSelect;
			MM_TrackSelect_Init();
			return;
		}
	}

	int posX = D230.characterSelectTransitionMeta[MM_CHARACTER_SELECT_TITLE_TRANSITION_INDEX].currX;
	int posY = D230.characterSelectTransitionMeta[MM_CHARACTER_SELECT_TITLE_TRANSITION_INDEX].currY;

	u32 characterSelectType;
	char *characterSelectString;
	switch (D230.characterSelectLayoutIndex)
	{
	// 3P character selection
	case MM_CHARACTER_SELECT_LAYOUT_3P:

		// If you have a lot of characters unlocked, do not draw SELECT CHARACTER
		if (D230.characterSelectRosterExpanded)
		{
			goto dontDrawSelectCharacter;
		}

		DecalFont_DrawLine(sdata->lngStrings[LNG_SELECT_CHARACTER_SELECT], posX + MM_CHARACTER_SELECT_3P_TITLE_X, posY + MM_CHARACTER_SELECT_3P_SELECT_Y,
		                   FONT_BIG, (JUSTIFY_CENTER | ORANGE));
		characterSelectType = FONT_BIG;

		characterSelectString = sdata->lngStrings[LNG_CHARACTER];

		posX = posX + MM_CHARACTER_SELECT_3P_TITLE_X;
		posY = posY + MM_CHARACTER_SELECT_3P_CHARACTER_Y;
		break;

	// 4P character selection
	case MM_CHARACTER_SELECT_LAYOUT_4P:

		// If Fake Crash is unlocked, do not draw "Select Character"
		if (sdata->gameProgress.unlockFlags & UNLOCK_FAKE_CRASH)
		{
			goto dontDrawSelectCharacter;
		}

		DecalFont_DrawLine(sdata->lngStrings[LNG_SELECT_CHARACTER_SELECT], posX + MM_CHARACTER_SELECT_4P_TITLE_X, posY + MM_CHARACTER_SELECT_4P_SELECT_Y,
		                   FONT_CREDITS, (JUSTIFY_CENTER | ORANGE));
		characterSelectType = FONT_CREDITS;

		characterSelectString = sdata->lngStrings[LNG_CHARACTER];

		posX = posX + MM_CHARACTER_SELECT_4P_TITLE_X;
		posY = posY + MM_CHARACTER_SELECT_4P_CHARACTER_Y;
		break;

	// If you are in 1P or 2P character selection,
	// when you do NOT have a lot of characters selected
	case MM_CHARACTER_SELECT_LAYOUT_1P_LIMITED:
	case MM_CHARACTER_SELECT_LAYOUT_2P_LIMITED:
		characterSelectType = FONT_BIG;

		characterSelectString = sdata->lngStrings[LNG_SELECT_CHARACTER];

		posX = posX + MM_CHARACTER_SELECT_LIMITED_TITLE_X;
		posY = posY + MM_CHARACTER_SELECT_LIMITED_TITLE_Y;
		break;

	default:
		goto dontDrawSelectCharacter;
	}

	// Draw String
	DecalFont_DrawLine(characterSelectString, posX, posY, characterSelectType, (JUSTIFY_CENTER | ORANGE));

dontDrawSelectCharacter:

	for (s32 playerIndex = 0; playerIndex < gGT->numPlyrNextGame; playerIndex++)
	{
		u16 playerSelectFlag = (u16)(1 << playerIndex);
		s16 currentIcon = iconPerPlayer[playerIndex];
		s16 candidateIcon = currentIcon;
		b32 playerSelectedBeforeInput = (((int)(s16)sdata->characterSelectFlags >> playerIndex) & 1U) != 0;

		Color playerColor;
		MM_Characters_AnimateColors((u8 *)&playerColor, playerIndex, (int)(s16)(sdata->characterSelectFlags & playerSelectFlag));

		struct CharacterSelectMeta *preInputCharacterMeta = &D230.activeCharacterSelectMeta[currentIcon];
		u32 button = sdata->buttonTapPerPlayer[playerIndex];

		if ((D230.characterSelectMenuState == IN_MENU) &&
		    // If you press the D-Pad, or Cross, Square, Triangle, Circle
		    ((button & (MM_CHARACTER_SELECT_INPUT_DPAD | MM_CHARACTER_SELECT_INPUT_MENU)) != 0))
		{
			// if character has not been selected by this player
			if (!playerSelectedBeforeInput)
			{
				// If you pressed any of the D-pad buttons
				if ((button & MM_CHARACTER_SELECT_INPUT_DPAD) != 0)
				{
					hitNavigationDeadEnd = 0;

					// If you do not press Up
					if ((button & BTN_UP) == 0)
					{
						// If you do not press Down
						if ((button & BTN_DOWN) == 0)
						{
							// This must be if you press Left,
							// because the variable will change
							// if it is anything that isn't Left

							// Left
							direction = CHARACTER_SELECT_DIR_LEFT;

							// If you press Left
							if ((button & BTN_LEFT) != 0)
							{
								goto LAB_800aec08;
							}

							// At this point, you must have pressed Right

							// Right
							direction = CHARACTER_SELECT_DIR_RIGHT;

							// Move down character selection list
							D230.characterSelectPlayerState.modelMoveDir[playerIndex] = MM_CHARACTER_SELECT_MODEL_MOVE_NEXT;
						}

						// If you pressed Down
						else
						{
							// Down
							direction = CHARACTER_SELECT_DIR_DOWN;

							// Move down character selection list
							D230.characterSelectPlayerState.modelMoveDir[playerIndex] = MM_CHARACTER_SELECT_MODEL_MOVE_NEXT;
						}
					}

					// If you pressed Up
					else
					{
						// Up
						direction = CHARACTER_SELECT_DIR_UP;
					LAB_800aec08:
						// If you press Up or Left

						// Move up character selection list
						D230.characterSelectPlayerState.modelMoveDir[playerIndex] = MM_CHARACTER_SELECT_MODEL_MOVE_PREV;
					}

					previousCandidateIcon = candidateIcon;
					do
					{
						candidateIcon = MM_Characters_GetNextDriver(direction, previousCandidateIcon);
						alternateIcon = candidateIcon;

						if (candidateIcon == previousCandidateIcon)
						{
							hitNavigationDeadEnd = 1;
							nextIcon = MM_Characters_GetNextDriver(direction, (int)(s16)currentIcon);
							nextIconCopy = (int)nextIcon;
							candidateIcon = MM_Characters_GetNextDriver(D230.characterSelectFallbackDirection1[direction], nextIconCopy);
							intermediateIcon = (int)(s16)candidateIcon;

							if ((((intermediateIcon == alternateIcon) || (nextIconCopy == alternateIcon)) || (nextIconCopy == intermediateIcon)) ||
							    MM_Characters_boolIsInvalid(iconPerPlayer, intermediateIcon, playerIndex))
							{
								nextIcon = MM_Characters_GetNextDriver(D230.characterSelectFallbackDirection1[direction], (int)(s16)currentIcon);
								intermediateIcon = (int)nextIcon;
								candidateIcon = MM_Characters_GetNextDriver(direction, intermediateIcon);
								alternateIcon = (int)(s16)candidateIcon;

								if (((alternateIcon == previousCandidateIcon) || (intermediateIcon == previousCandidateIcon)) ||
								    ((intermediateIcon == alternateIcon || MM_Characters_boolIsInvalid(iconPerPlayer, alternateIcon, playerIndex))))
								{
									nextIcon = MM_Characters_GetNextDriver(direction, (int)(s16)currentIcon);
									intermediateIcon = (int)nextIcon;
									candidateIcon = MM_Characters_GetNextDriver(D230.characterSelectFallbackDirection2[direction], intermediateIcon);
									alternateIcon = (int)(s16)candidateIcon;

									if (((alternateIcon == previousCandidateIcon) || (intermediateIcon == previousCandidateIcon)) ||
									    ((intermediateIcon == alternateIcon || MM_Characters_boolIsInvalid(iconPerPlayer, alternateIcon, playerIndex))))
									{
										nextIcon = MM_Characters_GetNextDriver(D230.characterSelectFallbackDirection2[direction], (int)(s16)currentIcon);
										intermediateIcon = (int)nextIcon;
										candidateIcon = MM_Characters_GetNextDriver(direction, intermediateIcon);
										alternateIcon = (int)(s16)candidateIcon;

										if ((((alternateIcon == previousCandidateIcon) || (intermediateIcon == previousCandidateIcon)) ||
										     (intermediateIcon == alternateIcon)) ||
										    MM_Characters_boolIsInvalid(iconPerPlayer, alternateIcon, playerIndex))
										{
											candidateIcon = (u32)currentIcon;
										}
									}
								}
							}
						}
						candidateInUseByOtherPlayer = false;

						for (s32 otherPlayerIndex = 0; otherPlayerIndex < gGT->numPlyrNextGame; otherPlayerIndex++)
						{
							if ((otherPlayerIndex != playerIndex) && ((s16)candidateIcon == iconPerPlayer[otherPlayerIndex]))
							{
								candidateInUseByOtherPlayer = true;
								break;
							}
						}

						if (previousCandidateIcon << 0x10 != candidateIcon << 0x10)
						{
							// Play sound
							// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800aeeb8-0x800aeecc for character cursor-change SFX.
							OtherFX_Play(0, 1);
						}
						if (hitNavigationDeadEnd != 0)
						{
							deadEndCandidateAvailable = !candidateInUseByOtherPlayer;
							candidateInUseByOtherPlayer = false;
							if (deadEndCandidateAvailable)
							{
								break;
							}
							candidateIcon = (u32)currentIcon;
						}
						previousCandidateIcon = candidateIcon;
					} while (candidateInUseByOtherPlayer);
				}
				currentIcon = (u16)candidateIcon;

				for (s32 otherPlayerIndex = 0; otherPlayerIndex < gGT->numPlyrNextGame; otherPlayerIndex++)
				{
					if ((otherPlayerIndex != playerIndex) && ((s16)candidateIcon == iconPerPlayer[otherPlayerIndex]))
					{
						candidateIcon = (u32)(u16)iconPerPlayer[playerIndex];
					}
					currentIcon = (u16)candidateIcon;
				}

				// If this player pressed Cross or Circle
				if (((sdata->buttonTapPerPlayer)[playerIndex] & MM_CHARACTER_SELECT_INPUT_CONFIRM) != 0)
				{
					// this player has now selected a character
					sdata->characterSelectFlags = sdata->characterSelectFlags | (u16)(1 << playerIndex);

					u8 numPlyrNextGame = gGT->numPlyrNextGame;

					// Play sound
					// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800aefa4-0x800aefe4 for character confirm SFX.
					OtherFX_Play(1, 1);

					// if all players have selected their characters
					if ((int)(s16)sdata->characterSelectFlags == (1 << numPlyrNextGame) - 1)
					{
						// exit toward cup or track selection
						D230.characterSelectExitsForward = 1;
						D230.characterSelectMenuState = EXITING_MENU;
					}
				}

				if (
				    // if this is the first iteration of the loop
				    ((playerIndex & 0xffff) == 0) &&

				    // if you press Square or Triangle
				    ((sdata->buttonTapPerPlayer[0] & MM_CHARACTER_SELECT_INPUT_BACK) != 0))
				{
					// return to main menu
					D230.characterSelectExitsForward = 0;
					D230.characterSelectMenuState = EXITING_MENU;

					// Play sound
					// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800af01c-0x800af054 for character-select back SFX.
					OtherFX_Play(2, 1);
				}
			}
			else
			{
				// if you press Square or Triangle
				if ((button & MM_CHARACTER_SELECT_INPUT_BACK) != 0)
				{
					// Play sound
					// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800af060-0x800af074 for character deselect SFX.
					OtherFX_Play(2, 1);

					// this player has de-selected their character
					sdata->characterSelectFlags = sdata->characterSelectFlags & ~playerSelectFlag;
				}
			}

			// clear input
			sdata->buttonTapPerPlayer[playerIndex] = 0;
		}

		iconPerPlayer[playerIndex] = currentIcon;

		// transition of each icon
		struct TransitionMeta *currentIconTransition = &D230.characterSelectTransitionMeta[currentIcon];

		// if player has not selected a character
		b32 playerSelectedAfterInput = ((sdata->characterSelectFlags >> playerIndex) & 1U) != 0;
		Color outlineColor;
		if (!playerSelectedAfterInput)
		{
			// draw string
			// "1", "2", "3", "4", above the character icon
			DecalFont_DrawLine(D230.playerNumberStrings[playerIndex], currentIconTransition->currX + (u32)preInputCharacterMeta->posX - 6,
			                   currentIconTransition->currY + (u32)preInputCharacterMeta->posY - 3, FONT_BIG, WHITE);
			outlineColor = playerColor;
		}
		else
		{
			outlineColor = D230.characterSelect_Outline;
		}

		drawRect.x = currentIconTransition->currX + preInputCharacterMeta->posX;
		drawRect.y = currentIconTransition->currY + preInputCharacterMeta->posY;
		drawRect.w = MM_CHARACTER_SELECT_ICON_RECT_W;
		drawRect.h = MM_CHARACTER_SELECT_ICON_RECT_H;

		RECTMENU_DrawOuterRect_HighLevel(&drawRect, outlineColor, 0, ot);
	}

	MM_Characters_PreventOverlap();

	struct CharacterSelectMeta *iconDrawMeta = D230.activeCharacterSelectMeta;

	// loop through character icons
	for (s32 iconIndex = 0; iconIndex < MM_CHARACTER_SELECT_ICON_COUNT; iconIndex++)
	{
		s16 unlockRequirement = iconDrawMeta->unlockFlags;
		if (
		    // If Icon is unlocked by default,
		    (unlockRequirement == MM_CHARACTER_UNLOCK_ALWAYS) ||

		    // if character is unlocked
		    // from the global unlock bitfield
		    // also the variable written by cheats
		    CHECK_ADV_BIT(sdata->gameProgress.unlocks, unlockRequirement))
		{
			Color iconColor = D230.characterSelect_NeutralColor;

			for (s32 playerIndex = 0; playerIndex < gGT->numPlyrNextGame; playerIndex++)
			{
				b32 playerSelected = (((int)(s16)sdata->characterSelectFlags >> (playerIndex & 0x1fU)) & 1U) != 0;
				if (((s16)iconIndex == iconPerPlayer[playerIndex]) &&

				    // if player selected a character
				    playerSelected)
				{
					iconColor = D230.characterSelect_ChosenColor;
				}
			}

			struct TransitionMeta *iconTransition = &D230.characterSelectTransitionMeta[iconIndex];

			RECTMENU_DrawPolyGT4(gGT->ptrIcons[data.MetaDataCharacters[iconDrawMeta->characterID].iconID],
			                     iconTransition->currX + iconDrawMeta->posX + MM_CHARACTER_SELECT_ICON_DECAL_OFFSET_X,
			                     iconTransition->currY + iconDrawMeta->posY + MM_CHARACTER_SELECT_ICON_DECAL_OFFSET_Y,

			                     &gGT->backBuffer->primMem, gGT->pushBuffer_UI.ptrOT,

			                     iconColor.self, iconColor.self, iconColor.self, iconColor.self, TRANS_50_DECAL, FP(1.0));
		}

		iconDrawMeta++;
	}

	// reset
	struct CharacterSelectMeta *activeCharacterSelectMeta = D230.activeCharacterSelectMeta;

	for (s32 playerIndex = 0; playerIndex < MM_CHARACTER_SELECT_MAX_PLAYERS; playerIndex++)
	{
		data.characterIDs[playerIndex] = activeCharacterSelectMeta[(int)iconPerPlayer[playerIndex]].characterID;
	}

	MM_Characters_NativeDrawStats();

	for (s32 playerIndex = 0; playerIndex < gGT->numPlyrNextGame; playerIndex++)
	{
		s16 playerIcon = iconPerPlayer[playerIndex];
		activeCharacterSelectMeta = &D230.activeCharacterSelectMeta[playerIcon];
		b32 playerSelected = (((int)(s16)sdata->characterSelectFlags >> playerIndex) & 1U) != 0;

		// if player has not selected a character
		if (!playerSelected)
		{
			Color animatedColor;
			u16 selectedPlayerFlag = (u16)(1 << playerIndex);
			MM_Characters_AnimateColors((u8 *)&animatedColor, playerIndex,

			                            // flags of which characters are selected
			                            (int)(s16)(sdata->characterSelectFlags & selectedPlayerFlag));

			animatedColor.r = (u8)((int)((u32)animatedColor.r << 2) / 5);
			animatedColor.g = (u8)((int)((u32)animatedColor.g << 2) / 5);
			animatedColor.b = (u8)((int)((u32)animatedColor.b << 2) / 5);

			struct TransitionMeta *selectedIconTransition = &D230.characterSelectTransitionMeta[playerIcon];

			drawRect.x = selectedIconTransition->currX + activeCharacterSelectMeta->posX + MM_CHARACTER_SELECT_HIGHLIGHT_OFFSET_X;
			drawRect.y = selectedIconTransition->currY + activeCharacterSelectMeta->posY + MM_CHARACTER_SELECT_HIGHLIGHT_OFFSET_Y;
			drawRect.w = MM_CHARACTER_SELECT_HIGHLIGHT_W;
			drawRect.h = MM_CHARACTER_SELECT_HIGHLIGHT_H;

			// this draws the flashing blue square that appears when you highlight a character in the character select screen
			CTR_Box_DrawSolidBox(&drawRect, animatedColor, ot);
		}
		if ((D230.characterSelectModelMoveTimer[playerIndex] == 0) &&
		    (D230.characterSelectPlayerState.currentCharacterID[playerIndex] == data.characterIDs[playerIndex]))
		{
			// get number of players
			u8 numPlyrNextGame = gGT->numPlyrNextGame;

			// if number of players is 1 or 2
			u32 fontType = FONT_CREDITS;

			// if number of players is 3 or 4
			if (numPlyrNextGame >= 3)
			{
				fontType = FONT_SMALL;
			}

			struct TransitionMeta *driverWindowTransition =
			    &D230.characterSelectTransitionMeta[playerIndex + MM_CHARACTER_SELECT_DRIVER_WINDOW_TRANSITION_FIRST];
			SVec2 *windowPos = &D230.activeCharacterSelectWindowPos[playerIndex];
			s16 nameBaseY = driverWindowTransition->currY + windowPos->y;
			s16 nameYOffset = (s16)((((u32)(numPlyrNextGame < 3) ^ 1) << 0x12) >> 0x10);
			s16 nameY;

			if ((numPlyrNextGame == 4) && (playerIndex > 1))
			{
				nameY = nameBaseY + nameYOffset + MM_CHARACTER_SELECT_4P_NAME_BOTTOM_OFFSET;
			}
			else
			{
				nameY = nameBaseY + D230.characterSelectNameTextY + nameYOffset;
			}

			// draw string
			DecalFont_DrawLine(sdata->lngStrings[data.MetaDataCharacters[activeCharacterSelectMeta->characterID].name_LNG_long],
			                   (int)driverWindowTransition->currX + windowPos->x + (int)((u32)D230.characterSelectWindowWidth >> 1), (int)nameY, fontType,
			                   (JUSTIFY_CENTER | ORANGE));
		}

		// spin the character
		D230.characterSelectPlayerState.angle[playerIndex] += MM_CHARACTER_SELECT_SPIN_STEP;
	}

	// reset
	activeCharacterSelectMeta = D230.activeCharacterSelectMeta;

	// loop through all icons
	for (s32 iconIndex = 0; iconIndex < MM_CHARACTER_SELECT_ICON_COUNT; iconIndex++)
	{
		s16 unlockRequirement = activeCharacterSelectMeta[iconIndex].unlockFlags;

		if (
		    // If Icon is unlocked (from array of icons)
		    (unlockRequirement == MM_CHARACTER_UNLOCK_ALWAYS) ||

		    // if character is unlocked
		    // from the global unlock bitfield
		    // also the variable written by cheats
		    CHECK_ADV_BIT(sdata->gameProgress.unlocks, unlockRequirement))
		{
			struct TransitionMeta *iconTransition = &D230.characterSelectTransitionMeta[iconIndex];

			drawRect.x = iconTransition->currX + activeCharacterSelectMeta[iconIndex].posX;
			drawRect.y = iconTransition->currY + activeCharacterSelectMeta[iconIndex].posY;
			drawRect.w = MM_CHARACTER_SELECT_ICON_RECT_W;
			drawRect.h = MM_CHARACTER_SELECT_ICON_RECT_H;

			// Draw 2D Menu rectangle background
			RECTMENU_DrawInnerRect(&drawRect, 0, ot);
		}
	}

	SVec2 *windowPos = D230.activeCharacterSelectWindowPos;

	for (s32 playerIndex = 0; playerIndex < gGT->numPlyrNextGame; playerIndex++)
	{
		struct TransitionMeta *driverWindowTransition = &D230.characterSelectTransitionMeta[playerIndex + MM_CHARACTER_SELECT_DRIVER_WINDOW_TRANSITION_FIRST];
		b32 playerSelected = (((int)(s16)sdata->characterSelectFlags >> playerIndex) & 1U) != 0;
		Color animatedColor;

		// store window width and height in one 4-byte variable
		drawRect.x = driverWindowTransition->currX + windowPos->x;
		drawRect.y = driverWindowTransition->currY + windowPos->y;
		drawRect.w = D230.characterSelectWindowWidth;
		drawRect.h = D230.characterSelectWindowHeight;

		MM_Characters_AnimateColors((u8 *)&animatedColor, playerIndex,

		                            // flags of which characters are selected
		                            playerSelected ^ 1);

		RECTMENU_DrawOuterRect_HighLevel(&drawRect, animatedColor, 0, ot);

		// if player selected a character
		if (playerSelected)
		{
			RECT r58;
			r58.x = drawRect.x;
			r58.y = drawRect.y;
			r58.w = drawRect.w;
			r58.h = drawRect.h;

			for (s32 borderIndex = 0; borderIndex < MM_CHARACTER_SELECT_SELECTED_BORDER_COUNT; borderIndex++)
			{
				r58.x += MM_CHARACTER_SELECT_SELECTED_BORDER_INSET_X;
				r58.y += MM_CHARACTER_SELECT_SELECTED_BORDER_INSET_Y;
				r58.w -= MM_CHARACTER_SELECT_SELECTED_BORDER_SHRINK_W;
				r58.h -= MM_CHARACTER_SELECT_SELECTED_BORDER_SHRINK_H;

				animatedColor.r = (u8)((int)((u32)animatedColor.r << 2) / 5);
				animatedColor.g = (u8)((int)((u32)animatedColor.g << 2) / 5);
				animatedColor.b = (u8)((int)((u32)animatedColor.b << 2) / 5);

				RECTMENU_DrawOuterRect_HighLevel(&r58, animatedColor, 0, ot);
			}
		}
		windowPos++;

		// Draw 2D Menu rectangle background
		RECTMENU_DrawInnerRect(&drawRect, 9, &ot[3]);

		// not screen-space anymore,
		// this is viewport-space
		drawRect.x = 0;
		drawRect.y = 0;

		RECTMENU_DrawRwdBlueRect(&drawRect, &D230.characterSelect_BlueRectColors[0], &gGT->pushBuffer[playerIndex].ptrOT[0x3ff], &gGT->backBuffer->primMem);
	}
	return;
}
