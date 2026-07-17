#include <common.h>

enum AHMapIconID
{
	AH_MAP_ICON_WARPPAD = 0x31,
	AH_MAP_ICON_BOSS_STAR = 0x37,
};

enum AHMapArrowType
{
	AH_MAP_ARROW_WARPPAD_TROPHY = 0,
	AH_MAP_ARROW_HUB_ROUTE,
	AH_MAP_ARROW_BOSS,
};

enum AHMapColor
{
	AH_MAP_COLOR_COMPLETE = RED,
	AH_MAP_COLOR_FLASH_PRIMARY = CRASH_BLUE,
	AH_MAP_COLOR_FLASH_SECONDARY = WHITE,
	AH_MAP_COLOR_RELIC_TOKEN = PAPU_YELLOW,
	AH_MAP_COLOR_INVALID = BLACK,
	AH_MAP_COLOR_LOCKED = GRAY,
};

CTR_STATIC_ASSERT(AH_MAP_ICON_WARPPAD == 0x31);
CTR_STATIC_ASSERT(AH_MAP_ICON_BOSS_STAR == 0x37);
CTR_STATIC_ASSERT(AH_MAP_ARROW_WARPPAD_TROPHY == 0);
CTR_STATIC_ASSERT(AH_MAP_ARROW_HUB_ROUTE == 1);
CTR_STATIC_ASSERT(AH_MAP_ARROW_BOSS == 2);
CTR_STATIC_ASSERT(AH_MAP_COLOR_COMPLETE == 3);
CTR_STATIC_ASSERT(AH_MAP_COLOR_FLASH_PRIMARY == 5);
CTR_STATIC_ASSERT(AH_MAP_COLOR_FLASH_SECONDARY == 4);
CTR_STATIC_ASSERT(AH_MAP_COLOR_RELIC_TOKEN == 0xe);
CTR_STATIC_ASSERT(AH_MAP_COLOR_INVALID == 0x15);
CTR_STATIC_ASSERT(AH_MAP_COLOR_LOCKED == 0x17);

enum AHMapBossItemState
{
	AH_MAP_BOSS_ITEM_NONE = -1,
	AH_MAP_BOSS_ITEM_LOCKED = 0,
	AH_MAP_BOSS_ITEM_OPEN = 1,
	AH_MAP_BOSS_ITEM_COMPLETE = 2,
};

CTR_STATIC_ASSERT(AH_MAP_BOSS_ITEM_NONE == -1);
CTR_STATIC_ASSERT(AH_MAP_BOSS_ITEM_LOCKED == 0);
CTR_STATIC_ASSERT(AH_MAP_BOSS_ITEM_OPEN == 1);
CTR_STATIC_ASSERT(AH_MAP_BOSS_ITEM_COMPLETE == 2);

enum AHMapMainConstants
{
	AH_MAP_HUD_AND_DEBUG_SPEEDOMETER = 0x8,
	AH_MAP_ICON_TOP_HALF = 3,
	AH_MAP_ICON_BOTTOM_HALF = 4,
	AH_MAP_SCREEN_POS_X = 500,
	AH_MAP_SCREEN_POS_Y = 195,
	AH_MAP_HUD_SLOT_SLIDE_METER = 8,
	AH_MAP_HUD_SLOT_RELIC_COUNT = 0xe,
	AH_MAP_HUD_SLOT_KEY_COUNT = 0xf,
	AH_MAP_HUD_SLOT_TROPHY_COUNT = 0x10,
	AH_MAP_HUD_COUNTER_OFFSET_X = 0x10,
	AH_MAP_HUD_COUNTER_OFFSET_Y = -10,
};

enum AHMapArrowOutlineConstants
{
	AH_MAP_ARROW_OUTLINE_COUNT = 3,
	AH_MAP_ARROW_OUTLINE_PHASE_STEP = 0xc,
	AH_MAP_ARROW_OUTLINE_PHASE_MASK = 0x3f,
	AH_MAP_ARROW_OUTLINE_PHASE_DELAY = 6,
	AH_MAP_ARROW_OUTLINE_VISIBLE_PHASES = 0xc,
	AH_MAP_ARROW_OUTLINE_RADIUS_MUL = 0x2aa,
	AH_MAP_ARROW_OUTLINE_RADIUS_BIAS = FP_ONE,
	AH_MAP_ARROW_OUTLINE_RADIUS_SHIFT = 0x1a,
	AH_MAP_ARROW_OUTLINE_TROPHY_STEP = 0x200,
	AH_MAP_ARROW_OUTLINE_ROUTE_STEP = 0x555,
	AH_MAP_ARROW_OUTLINE_BOSS_STEP = 0x199,
};

CTR_STATIC_ASSERT(AH_MAP_HUD_AND_DEBUG_SPEEDOMETER == 0x8);
CTR_STATIC_ASSERT(AH_MAP_ICON_TOP_HALF == 3);
CTR_STATIC_ASSERT(AH_MAP_ICON_BOTTOM_HALF == 4);
CTR_STATIC_ASSERT(AH_MAP_SCREEN_POS_X == 500);
CTR_STATIC_ASSERT(AH_MAP_SCREEN_POS_Y == 195);
CTR_STATIC_ASSERT(AH_MAP_HUD_SLOT_SLIDE_METER == 8);
CTR_STATIC_ASSERT(AH_MAP_HUD_SLOT_RELIC_COUNT == 0xe);
CTR_STATIC_ASSERT(AH_MAP_HUD_SLOT_KEY_COUNT == 0xf);
CTR_STATIC_ASSERT(AH_MAP_HUD_SLOT_TROPHY_COUNT == 0x10);
CTR_STATIC_ASSERT(AH_MAP_HUD_COUNTER_OFFSET_X == 0x10);
CTR_STATIC_ASSERT(AH_MAP_HUD_COUNTER_OFFSET_Y == -10);
CTR_STATIC_ASSERT(AH_MAP_ARROW_OUTLINE_COUNT == 3);
CTR_STATIC_ASSERT(AH_MAP_ARROW_OUTLINE_PHASE_STEP == 0xc);
CTR_STATIC_ASSERT(AH_MAP_ARROW_OUTLINE_PHASE_MASK == 0x3f);
CTR_STATIC_ASSERT(AH_MAP_ARROW_OUTLINE_PHASE_DELAY == 6);
CTR_STATIC_ASSERT(AH_MAP_ARROW_OUTLINE_VISIBLE_PHASES == 0xc);
CTR_STATIC_ASSERT(AH_MAP_ARROW_OUTLINE_RADIUS_MUL == 0x2aa);
CTR_STATIC_ASSERT(AH_MAP_ARROW_OUTLINE_RADIUS_BIAS == 0x1000);
CTR_STATIC_ASSERT(AH_MAP_ARROW_OUTLINE_RADIUS_SHIFT == 0x1a);
CTR_STATIC_ASSERT(AH_MAP_ARROW_OUTLINE_TROPHY_STEP == 0x200);
CTR_STATIC_ASSERT(AH_MAP_ARROW_OUTLINE_ROUTE_STEP == 0x555);
CTR_STATIC_ASSERT(AH_MAP_ARROW_OUTLINE_BOSS_STEP == 0x199);

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b0b98-0x800b0ce0.
void AH_Map_LoadSave_Prim(const SVec2 *vertPos, char *vertCol, void *ot, struct PrimMem *primMem)
{
	POLY_G4 *p = primMem->cursor;

	if (primMem->end < (void *)p)
	{
		return;
	}

	primMem->cursor = p + 1;

	setPolyG4(p);

	p->r0 = vertCol[0];
	p->g0 = vertCol[1];
	p->b0 = vertCol[2];

	p->r1 = vertCol[4];
	p->g1 = vertCol[5];
	p->b1 = vertCol[6];

	p->r2 = vertCol[8];
	p->g2 = vertCol[9];
	p->b2 = vertCol[10];

	p->r3 = vertCol[12];
	p->g3 = vertCol[13];
	p->b3 = vertCol[14];

	p->x0 = vertPos[0].x;
	p->y0 = vertPos[0].y;

	p->x1 = vertPos[1].x;
	p->y1 = vertPos[1].y;

	p->x2 = vertPos[2].x;
	p->y2 = vertPos[2].y;

	p->x3 = vertPos[3].x;
	p->y3 = vertPos[3].y;

	AddPrim(ot, p);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b0ce0-0x800b0f18.
void AH_Map_LoadSave_Full(int posX, int posY, const SVec2 *vertPos, char *vertCol, int scale, int angle)
{
	SVec2 basePos[4];
	SVec2 drawPos[4];

	struct GameTracker *gGT = sdata->gGT;

	int sin = MATH_Sin(angle);
	int cos = MATH_Cos(angle);

	for (int i = 0; i < 4; i++)
	{
		basePos[i].x = posX + 6 +
		               (s16)(((((vertPos[i].x * cos) >> 0xc) + ((vertPos[i].y * sin) >> 0xc)) * ((scale * 8) / 5)

		                          ) >>
		                     0xc);

		basePos[i].y = posY + 4 +
		               (s16)(((((vertPos[i].y * cos) >> 0xc) - ((vertPos[i].x * sin) >> 0xc)) * scale

		                      ) >>
		                     0xc);
	}

	const SVec2 *offset = &D232.loadSavePrimOffset[0];

	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			drawPos[j].x = basePos[j].x + offset[i].x;

			drawPos[j].y = basePos[j].y + offset[i].y;
		}

		AH_Map_LoadSave_Prim(&drawPos[0], vertCol, gGT->pushBuffer_UI.ptrOT, &gGT->backBuffer->primMem);

		vertCol = (char *)&D232.colorQuad[0];
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b0f18-0x800b1150.
void AH_Map_HubArrow(int posX, int posY, const SVec2 *vertPos, char *vertCol, int scale, int angle)
{
	SVec2 basePos[3];
	SVec2 drawPos[3];

	struct GameTracker *gGT = sdata->gGT;

	int sin = MATH_Sin(angle);
	int cos = MATH_Cos(angle);

	for (int i = 0; i < 3; i++)
	{
		basePos[i].x = posX + 6 +
		               (s16)(((((vertPos[i].x * cos) >> 0xc) + ((vertPos[i].y * sin) >> 0xc)) * ((scale * 8) / 5)

		                          ) >>
		                     0xc);

		basePos[i].y = posY + 4 +
		               (s16)(((((vertPos[i].y * cos) >> 0xc) - ((vertPos[i].x * sin) >> 0xc)) * scale

		                      ) >>
		                     0xc);
	}

	const SVec2 *offset = &D232.hubArrowPrimOffset[0];

	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			drawPos[j].x = basePos[j].x + offset[i].x;

			drawPos[j].y = basePos[j].y + offset[i].y;
		}

		RECTMENU_DrawRwdTriangle(drawPos[0].v, vertCol, gGT->pushBuffer_UI.ptrOT, &gGT->backBuffer->primMem);

		vertCol = (char *)&D232.colorTri[0];
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b1150-0x800b14f4.
void AH_Map_HubArrowOuter(struct UIMap *map, int arrowIndex, int posX, int posY, int inputAngle, int type)
{
	struct GameTracker *gGT;
	gGT = sdata->gGT;

	(void)map;

	arrowIndex = (s16)arrowIndex;
	type = (s16)type;

	posX += D232.hubArrowInnerOffset[type].x;
	posY += D232.hubArrowInnerOffset[type].y;

	int timer = gGT->timer >> 0;

	int outlineColorR;
	int outlineColorG;
	int outlineAngleStep;

	outlineColorG = 0x40;
	if ((timer & 1) != 0)
	{
		outlineColorG = 0xe0;
	}

	if (type == 0)
	{
		outlineColorR = outlineColorG;
		outlineAngleStep = AH_MAP_ARROW_OUTLINE_TROPHY_STEP;
	}

	else if (type == 1)
	{
		outlineColorR = 0xff;
		outlineAngleStep = AH_MAP_ARROW_OUTLINE_ROUTE_STEP;

		int directionIndex = ((inputAngle >> 0x8) & 0xc) >> 2;
		posX += D232.hubArrowOuterOffset[directionIndex].x;
		posY += D232.hubArrowOuterOffset[directionIndex].y;
	}

	else
	{
		outlineColorR = outlineColorG;
		outlineAngleStep = AH_MAP_ARROW_OUTLINE_BOSS_STEP;
		inputAngle ^= 0x800;
	}

	inputAngle = (s16)inputAngle;

	for (int outlineIndex = 0; outlineIndex < AH_MAP_ARROW_OUTLINE_COUNT; outlineIndex++)
	{
		u32 outlinePhase = (~(timer + (int)arrowIndex * AH_MAP_ARROW_OUTLINE_PHASE_STEP) & AH_MAP_ARROW_OUTLINE_PHASE_MASK) +
		                   ((AH_MAP_ARROW_OUTLINE_COUNT - 1) - (int)(s16)outlineIndex) * -AH_MAP_ARROW_OUTLINE_PHASE_DELAY;

		if (outlinePhase >= AH_MAP_ARROW_OUTLINE_VISIBLE_PHASES)
		{
			continue;
		}

		int outlineRadius =
		    ((outlinePhase * AH_MAP_ARROW_OUTLINE_RADIUS_MUL + AH_MAP_ARROW_OUTLINE_RADIUS_BIAS) * 0x10000) >> AH_MAP_ARROW_OUTLINE_RADIUS_SHIFT;

		b32 isFirstPoint = true;

		int shiftToggle = 1;

		int prevX = 0;
		int prevY = 0;

		for (int outlineAngle = 0; outlineAngle < outlineAngleStep + 0xfff; outlineAngle += outlineAngleStep)
		{
			if (type != 2)
			{
				shiftToggle = 0;
			}

			int angle = outlineAngle + inputAngle;

			int sin = MATH_Sin(angle);
			int cos = MATH_Cos(angle);

			int radiusShift = (shiftToggle & 1) + 0xc;

			sin = posX + ((((outlineRadius << 3) / 5) * sin) >> radiusShift);
			cos = posY - ((outlineRadius * cos) >> radiusShift);

			if (!isFirstPoint)
			{
				CTR_Box_DrawWirePrims((Point){{prevX, prevY}}, (Point){{sin, cos}}, MakeColor(outlineColorR, outlineColorG, 0xff),
				                      (void *)gGT->pushBuffer_UI.ptrOT);
			}

			isFirstPoint = false;
			prevX = sin;
			prevY = cos;
			shiftToggle++;
		}
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b14f4-0x800b1a18.
void AH_Map_HubItems(struct UIMap *map, s16 *arrowCounter)
{
	struct GameTracker *gGT = sdata->gGT;
	struct AdvProgress *adv = &sdata->advProgress;
	s16 levelID = gGT->levelID;
	struct HubItem *item = D232.hubItemsXY_ptrArray[levelID - GEM_STONE_VALLEY];
	Vec3 pos3D;

	if (item->posX != AH_HUB_ITEM_LIST_END_POS_X)
	{
		do
		{
			AdventureHubItemType iconType = item->iconType;
			s16 routeLockState = -1;
			s16 bossState = AH_MAP_BOSS_ITEM_NONE;

			b32 open = true;

			// One-key route arrow, only locked in N. Sanity Beach.
			if (iconType == AH_HUB_ITEM_ROUTE_KEY1_IF_BEACH)
			{
				routeLockState = 0;

				if (levelID == N_SANITY_BEACH)
				{
					// locked if key < 1
					routeLockState = (gGT->currAdvProfile.numKeys < 1);
				}
			}
			else if (AH_HUB_ITEM_ROUTE_KEY1_IF_BEACH < iconType)
			{
				// gemstone valley
				if (iconType == AH_HUB_ITEM_OXIDE_WARPPAD)
				{
					// check all boss keys
					for (int i = 0; i < AH_BOSS_KEY_COUNT; i++)
					{
						u32 bit = i + ADV_REWARD_FIRST_BOSS_KEY;

						if (!CHECK_ADV_BIT(adv->rewards, bit))
						{
							open = false;
							break;
						}
					}

					if (open)
					{
						bossState = ((adv->storyFlags & ADV_REWARD_BEAT_OXIDE_FIRST_BOSS_MASK) != 0) ? AH_MAP_BOSS_ITEM_COMPLETE : AH_MAP_BOSS_ITEM_OPEN;
					}
					else
					{
						bossState = AH_MAP_BOSS_ITEM_LOCKED;
					}
				}
				else if (AH_HUB_ITEM_PINSTRIPE_GARAGE < iconType)
				{
					// save/load screen synthetic hub marker
					if (iconType == AH_HUB_ITEM_SAVE_LOAD_MARKER)
					{
						int saveLoadPosX = (int)item->posX - 0x200;
						int saveLoadPosY = (int)item->posY - 0x100;

						UI_Map_GetIconPos(map, &saveLoadPosX, &saveLoadPosY);

						AH_Map_LoadSave_Full(saveLoadPosX, saveLoadPosY, &D232.loadSavePos[0], (char *)&D232.loadSave_col[0], 0x800, (int)item->angle);
					}
				}
				else
				{
					int base = levelID - N_SANITY_BEACH;
					s16 *trophies = &data.advHubTrackIDs[base * AH_HUB_TRACK_COUNT];

					for (int i = 0; i < AH_HUB_TRACK_COUNT; i++)
					{
						if (!CHECK_ADV_BIT(adv->rewards, trophies[i] + ADV_REWARD_FIRST_TROPHY))
						{
							open = false;
							break;
						}
					}

					if (open)
					{
						bossState = CHECK_ADV_BIT(adv->rewards, base + ADV_REWARD_FIRST_BOSS_KEY) ? AH_MAP_BOSS_ITEM_COMPLETE : AH_MAP_BOSS_ITEM_OPEN;
					}
					else
					{
						bossState = AH_MAP_BOSS_ITEM_LOCKED;
					}
				}
			}
			// Two-key route arrow.
			else if (iconType == AH_HUB_ITEM_ROUTE_KEY2)
			{
				// locked if keys < 2
				routeLockState = (gGT->currAdvProfile.numKeys < 2);
			}
			else if (iconType < AH_HUB_ITEM_ROUTE_OPEN_B)
			{
				// Three-key route arrow.
				if (iconType == AH_HUB_ITEM_ROUTE_KEY3)
				{
					// locked if keys < 3
					routeLockState = (gGT->currAdvProfile.numKeys < 3);
				}
			}
			// Open route arrows.
			else if ((iconType == AH_HUB_ITEM_ROUTE_OPEN_B) || (iconType == AH_HUB_ITEM_ROUTE_OPEN_A))
			{
				// never locked
				routeLockState = 0;
			}

			if (routeLockState >= 0)
			{
				int routePosX = (int)item->posX - 0x200;
				int routePosY = (int)item->posY - 0x100;

				UI_Map_GetIconPos(map, &routePosX, &routePosY);
				if ((routeLockState == 0) && (D232.mapPriorityArrowDrawn == 0))
				{
					AH_Map_HubArrowOuter(map, (int)*arrowCounter, routePosX, routePosY, (0x1000 - (u16)item->angle), AH_MAP_ARROW_HUB_ROUTE);
					*arrowCounter = *arrowCounter + 1;
				}

				int colorOffset;

				// if even frame
				if ((gGT->timer & 2) == 0)
				{
					colorOffset = (int)routeLockState * 6;
				}
				else
				{
					colorOffset = ((int)routeLockState * 2 + 1) * 3;
				}

				AH_Map_HubArrow(routePosX, routePosY, &D232.hubArrowPos[0], (char *)&D232.hubArrow_col1[colorOffset], 0x800, (int)item->angle);
			}

			if (bossState >= AH_MAP_BOSS_ITEM_LOCKED)
			{
				pos3D.x = (int)item->posX;
				pos3D.y = 0;
				pos3D.z = (int)item->posY;

				// if beat boss race
				int bossIconColor;

				if (bossState == AH_MAP_BOSS_ITEM_COMPLETE)
				{
					// red
					bossIconColor = AH_MAP_COLOR_COMPLETE;
				}
				else
				{
					// locked boss race
					// bossState == AH_MAP_BOSS_ITEM_LOCKED

					// grey
					bossIconColor = AH_MAP_COLOR_LOCKED;

					// open, not beaten
					if (bossState == AH_MAP_BOSS_ITEM_OPEN)
					{
						// blue and white
						// depending on frames
						bossIconColor = AH_MAP_COLOR_FLASH_PRIMARY;
						if ((gGT->timer & 2) != 0)
						{
							bossIconColor = AH_MAP_COLOR_FLASH_SECONDARY;
						}
					}
				}

				// open, not beaten
				if (bossState == AH_MAP_BOSS_ITEM_OPEN)
				{
					D232.mapPriorityArrowDrawn = bossState;

					int bossArrowPosX = pos3D.x;
					int bossArrowPosY = pos3D.z;

					UI_Map_GetIconPos(map, &bossArrowPosX, &bossArrowPosY);

					AH_Map_HubArrowOuter(map, (int)*arrowCounter, bossArrowPosX, bossArrowPosY, 0, AH_MAP_ARROW_BOSS);

					*arrowCounter = *arrowCounter + 1;
				}

				// draw star icon for boss
				UI_Map_DrawRawIcon(map, pos3D.v, AH_MAP_ICON_BOSS_STAR, bossIconColor, 0, 0x1000);
			}
			item++;
		} while (item->posX != AH_HUB_ITEM_LIST_END_POS_X);
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b1a18-0x800b1c90.
void AH_Map_Warppads(struct UIMap *map, struct Thread *warppadThread, s16 *arrowCounter)
{
	struct GameTracker *gGT = sdata->gGT;

	// find minDistance, set to max
	int minDistance = 0x7fffffff;
	struct Instance *closestWarppadInst = NULL;

	MATRIX *driverMatrix = &gGT->drivers[0]->instSelf->matrix;

	for (
	    /**/; warppadThread != NULL; warppadThread = warppadThread->siblingThread)
	{
		int visualState = warppadThread->modelIndex;
		b32 drawsTrophyArrow = false;
		b32 includeInSoundDistance = true;

		struct Instance *warppadInst = warppadThread->inst;
		int color;

		switch ((u32)visualState)
		{
		case AH_WP_VISUAL_LOCKED:
			color = AH_MAP_COLOR_LOCKED;
			includeInSoundDistance = false;
			break;
		case AH_WP_VISUAL_TROPHY_OPEN:
			color = AH_MAP_COLOR_FLASH_PRIMARY;
			if ((gGT->timer & 2) != 0)
			{
				color = AH_MAP_COLOR_FLASH_SECONDARY;
			}
			drawsTrophyArrow = true;
			break;
		case AH_WP_VISUAL_COMPLETE:
			color = AH_MAP_COLOR_COMPLETE;
			break;
		case AH_WP_VISUAL_RELIC_TOKEN_OPEN:
			color = AH_MAP_COLOR_RELIC_TOKEN;
			break;
		case AH_WP_VISUAL_COLOR_CYCLE_OPEN:
			// Each Slide Coliseum/Turbo Track color lasts two frames.
			color = ((gGT->timer >> 1) & 7) + AH_MAP_COLOR_FLASH_PRIMARY;
			break;
		default:
			color = AH_MAP_COLOR_INVALID;
			includeInSoundDistance = false;
			break;
		}

		if (drawsTrophyArrow)
		{
			// get posZ in 3D, turns into posY in 2D
			int arrowPosX = warppadInst->matrix.t[0];
			int arrowPosY = warppadInst->matrix.t[2];

			D232.mapPriorityArrowDrawn = 1;

			// Get Icon Dimensions
			UI_Map_GetIconPos(map, &arrowPosX, &arrowPosY);

			AH_Map_HubArrowOuter(map, (int)*arrowCounter, arrowPosX, arrowPosY, 0, AH_MAP_ARROW_WARPPAD_TROPHY);

			*arrowCounter = *arrowCounter + 1;
		}

		UI_Map_DrawRawIcon(map, &warppadInst->matrix.t[0], AH_MAP_ICON_WARPPAD, color, 0, 0x1000);

		if (!includeInSoundDistance)
		{
			// skip distance check
			continue;
		}

		int distX = warppadInst->matrix.t[0] - driverMatrix->t[0];
		int distY = warppadInst->matrix.t[1] - driverMatrix->t[1];
		int distZ = warppadInst->matrix.t[2] - driverMatrix->t[2];

		int currDistance = SquareRoot0_stub(distX * distX + distY * distY + distZ * distZ);

		if (minDistance > currDistance)
		{
			minDistance = currDistance;
			closestWarppadInst = warppadInst;
		}
	}

	// play sound from closest unlocked warppad
	if (closestWarppadInst != NULL)
	{
		PlayWarppadSound(minDistance << 1);
	}

	return;
}

#if defined(CTR_NATIVE)
force_inline void AH_MaskHint_DrawRepeatPrompt(void);
#endif

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b1c90-0x800b1ef8.
void AH_Map_Main(void)
{
	struct GameTracker *gGT = sdata->gGT;

	sdata->HudAndDebugFlags &= ~AH_MAP_HUD_AND_DEBUG_SPEEDOMETER;

	s16 driverIconCounter = 0;
	s16 arrowCounter = 0;
	struct Driver *advDriver = gGT->drivers[0];
	struct UiElement2D *hud = data.hudStructPtr[gGT->numPlyrCurrGame - 1];
	struct UIMap *map = NULL;

	int raceFlagState = RaceFlag_GetCanDraw();
	if (raceFlagState == 0)
	{
		RaceFlag_SetCanDraw(1);
	}

	if (
	    // if Aku Hint is not unlocked
	    !CHECK_ADV_BIT(sdata->advProgress.rewards, ADV_REWARD_HINT_WELCOME_TO_ARENA) &&

	    RaceFlag_IsFullyOffScreen())
	{
		// Trigger Aku Hint:
		// Welcome to Adventure Arena
		MainFrame_RequestMaskHint(ADV_MASK_HINT_ID_WELCOME_TO_ARENA, 0);
	}


	// NOTE(aalhendi): Retail keeps this AI-only Adventure Hub speedometer fallback.
	if ((gGT->numPlyrCurrGame == 0) && ((advDriver->actionsFlagSet & ACTION_BOT) != 0))
	{
		sdata->HudAndDebugFlags = AH_MAP_HUD_AND_DEBUG_SPEEDOMETER;
	}

	if (gGT->level1->ptrSpawnType1->count != 0)
	{
		void **pointers = ST1_GETPOINTERS(gGT->level1->ptrSpawnType1);
		map = pointers[ST1_MAP];
	}

	// if game is not paused
	if ((gGT->gameMode1 & PAUSE_ALL) == 0)
	{
		// Jump meter and landing boost
		UI_JumpMeter_Update(advDriver);
	}

	if ((gGT->hudFlags & HUD_FLAG_HIDE_ADVENTURE_MAP) == 0)
	{
		arrowCounter = 0;

		D232.mapPriorityArrowDrawn = 0;

		UI_Map_DrawDrivers(map, gGT->threadBuckets[PLAYER].thread, &driverIconCounter);

		AH_Map_Warppads(map, gGT->threadBuckets[WARPPAD].thread, &arrowCounter);

		AH_Map_HubItems(map, &arrowCounter);

		UI_Map_DrawMap(gGT->ptrIcons[AH_MAP_ICON_TOP_HALF], gGT->ptrIcons[AH_MAP_ICON_BOTTOM_HALF],

		               AH_MAP_SCREEN_POS_X, AH_MAP_SCREEN_POS_Y,

		               &gGT->backBuffer->primMem, gGT->pushBuffer_UI.ptrOT, 1);

		UI_DrawSlideMeter(hud[AH_MAP_HUD_SLOT_SLIDE_METER].x, hud[AH_MAP_HUD_SLOT_SLIDE_METER].y, advDriver);
	}

	UI_DrawNumRelic(hud[AH_MAP_HUD_SLOT_RELIC_COUNT].x + AH_MAP_HUD_COUNTER_OFFSET_X, hud[AH_MAP_HUD_SLOT_RELIC_COUNT].y + AH_MAP_HUD_COUNTER_OFFSET_Y);
	UI_DrawNumKey(hud[AH_MAP_HUD_SLOT_KEY_COUNT].x + AH_MAP_HUD_COUNTER_OFFSET_X, hud[AH_MAP_HUD_SLOT_KEY_COUNT].y + AH_MAP_HUD_COUNTER_OFFSET_Y);
	UI_DrawNumTrophy(hud[AH_MAP_HUD_SLOT_TROPHY_COUNT].x + AH_MAP_HUD_COUNTER_OFFSET_X, hud[AH_MAP_HUD_SLOT_TROPHY_COUNT].y + AH_MAP_HUD_COUNTER_OFFSET_Y);

#if defined(CTR_NATIVE)
	// NOTE(aalhendi): Retail appends this prompt after DrawOTag starts; the PS1
	// GPU can still consume that late OT write. Native DrawOTag parses
	// synchronously, so emit only this static prompt during the hub UI pass and
	// leave AH_MaskHint_Update to run the real state/audio timing later.
	if (sdata->AkuAkuHintState == 5)
	{
		AH_MaskHint_DrawRepeatPrompt();
	}
#endif
}
