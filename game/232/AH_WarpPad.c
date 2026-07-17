#include <common.h>

// NOTE(aalhendi): WarpPad level IDs come from "warppad#NN" instance names
// and use retail adventure numbering, not the native LevelID enum.
enum AdventureWarpPadID
{
	AH_WP_ID_SLIDE_COLISEUM = 16,
	AH_WP_ID_TURBO_TRACK = 17,
	AH_WP_ID_FIRST_BATTLE_TRACK = 18,
	AH_WP_ID_SKULL_ROCK = 21,
	AH_WP_ID_ROCKY_ROAD = 23,
	AH_WP_ID_FIRST_GEM_CUP = ADVENTURE_CUP_SYNTHETIC_LEVEL_ID_BASE,
};

enum
{
	AH_WP_GEM_CUP_COUNT = 5,
	AH_WP_BATTLE_TRACK_ID_SPAN = 7,
	AH_WP_RACER_SLOT_COUNT = 8,
	AH_WP_REWARD_INSTANCE_COUNT = 3,
	AH_WP_WISP_COUNT = 2,
	AH_WP_WARP_LOAD_FRAMES = 61,
	AH_WP_TROPHY_PORTAL_HOLD_FRAMES = 0x400,
	AH_WP_LONG_RANGE_NEAR_DIST_SQ = 0x144000,
	AH_WP_SHORT_RANGE_NEAR_DIST_SQ = 0x90000,
	AH_WP_WARP_TRIGGER_DIST_SQ = 0x8fff,
	AH_WP_REWARD_FADE_DIST_SQ = 0x900000,
	AH_WP_REWARD_SPACING_ANGLE = 0x555,
	AH_WP_SPAWN_FORWARD_OFFSET = 0x400,
	AH_WP_FLOATING_OBJECT_HEIGHT = 0x100,
	AH_WP_PRIZE_RING_RADIUS = 0xc0,
	AH_WP_PRIZE_TILT_Z = 0x155,
	AH_WP_RELIC_PRIZE_SCALE = 0x1800,
	AH_WP_STANDARD_ITEM_SCALE = 0x2000,
	AH_WP_TROPHY_PRIZE_SCALE = 0x2800,
	AH_WP_CLOSED_SINGLE_DIGIT_OFFSET = -0x80,
	AH_WP_CLOSED_SINGLE_ITEM_OFFSET = 0x80,
	AH_WP_CLOSED_ITEM_WITH_TENS_OFFSET = 0xc0,
	AH_WP_CLOSED_X_WITH_TENS_OFFSET = 0x40,
	AH_WP_CLOSED_TENS_OFFSET = -0x40,
	AH_WP_CLOSED_ONES_WITH_TENS_OFFSET = -0xa0,
	AH_WP_SPIN_PRIZE_STEP = 0x40,
	AH_WP_SPIN_PRIZE_REWARD_STEP = 0x80,
	AH_WP_SPIN_BEAM_STEP = 0x200,
	AH_WP_SPIN_WISP_STEP = 0x100,
	AH_WP_SPIN_REWARD_RING_STEP = 0x4,
	AH_WP_REWARD_PHASE_STEP = 0x20,
	AH_WP_WISP_RISE_RATE = 0x20,
	AH_WP_WISP_RISE_RATE_STEP = 0x10,
	AH_WP_WISP_FAR_MAX_HEIGHT = 0x600,
	AH_WP_WISP_NEAR_MAX_HEIGHT = 0x400,
	AH_WP_WISP_FIRST_FRAMES = 4,
	AH_WP_WISP_FADE_IN_STEP = 0x380,
	AH_WP_WISP_FADE_OUT_RANGE = 0xc00,
	AH_WP_OPEN_BEAM_ALPHA = 0xc00,
	AH_WP_OPEN_RING_ALPHA = 0x400,
	AH_WP_OPEN_RING_HEIGHT_STEP = 0x400,
	AH_WP_FULL_ALPHA = 0x1000,
	AH_WP_PURPLE_TOKEN_GROUP = 4,
	AH_WP_NAME_LEVEL_ID_OFFSET = 8,
	AH_WP_DIGIT_MODEL_FIRST_1_TO_8 = 0x38,
	AH_WP_DIGIT_MODEL_0 = 0x6d,
	AH_WP_DIGIT_MODEL_9 = 0x6e,
};

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800abafc-0x800abbdc.
s16 *AH_WarpPad_GetSpawnPosRot(s16 *posData)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Thread *t = gGT->threadBuckets[WARPPAD].thread;

	// check all warppads
	while (1)
	{
		// if can't find a warppad
		if (t == 0)
		{
			// quit
			return 0;
		}

		// if warppad found that matches level exited
		if (((struct WarpPad *)t->object)->levelID == gGT->prevLEV)
		{
			// end loop
			break;
		}

		t = t->siblingThread;
	}

	struct Instance *inst = t->inst;
	struct InstDef *instDef = inst->instDef;

	posData[0] = inst->matrix.t[0] + ((MATH_Cos(instDef->rot.y) * AH_WP_SPAWN_FORWARD_OFFSET) >> 0xC);

	posData[1] = inst->matrix.t[1];

	posData[2] = inst->matrix.t[2] + ((MATH_Sin(instDef->rot.y) * -AH_WP_SPAWN_FORWARD_OFFSET) >> 0xC);

	return &instDef->rot.x;
}

CTR_STATIC_ASSERT(sizeof(struct WarpPad) == 0x78);
CTR_STATIC_ASSERT(offsetof(struct WarpPad, lightDirGem) == 0x50);
CTR_STATIC_ASSERT(offsetof(struct WarpPad, digit10s) == 0x68);
CTR_STATIC_ASSERT(offsetof(struct WarpPad, levelID) == 0x6c);

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800abbdc-0x800abd80.
void AH_WarpPad_AllWarppadNum()
{
	struct Thread *t = sdata->gGT->threadBuckets[WARPPAD].thread;

	for (; t != 0; t = t->siblingThread)
	{
		struct WarpPad *wp = t->object;

		// DCxDemo says:
		// 1 to 8 is taken from mpk i guess, 0, 9 and X are seprate models

		if ((wp->inst[2] != 0) && (wp->digit1s != 0) && (wp->digit1s != 9))
		{
			struct Instance *inst = wp->inst[2];
			struct ModelHeader *mh = &inst->model->headers[0];
			AH_WarpPad_SetNumModelData(inst, &mh[wp->digit1s - 1]);
		}

		if ((wp->inst[3] != 0) && (wp->digit10s != 0))
		{
			struct Instance *inst = wp->inst[3];
			struct ModelHeader *mh = &inst->model->headers[0];
			AH_WarpPad_SetNumModelData(inst, mh);
		}
	}
}

void AH_WarpPad_SetNumModelData(struct Instance *inst, struct ModelHeader *mh)
{
	struct InstDrawPerPlayer *idpp = INST_GETIDPP(inst);

	idpp[0].ptrCommandList = mh->ptrCommandList;
	idpp[0].ptrColorLayout = (u32)mh->ptrColors;
	idpp[0].ptrTexLayout = mh->ptrTexLayout;
	idpp[0].ptrCurrFrame = mh->ptrFrameData;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800abd80-0x800abdfc.
void AH_WarpPad_MenuProc(struct RectMenu *menu)
{
	struct GameTracker *gGT = sdata->gGT;

	RECTMENU_Hide(menu);

	if (menu->rowSelected == 0)
	{
		gGT->gameMode2 |= TOKEN_RACE;
	}

	else if (menu->rowSelected == 1)
	{
		gGT->gameMode1 |= RELIC_RACE;
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800abdfc-0x800abf48.
void AH_WarpPad_SpinRewards(struct Instance *prizeInst, struct WarpPad *warppadObj, int index, int x, int y, int z)
{
	SVec3 *lightDir;

	ConvertRotToMatrix(&prizeInst->matrix, &warppadObj->spinRot_Prize);

	u32 modelID = prizeInst->model->id;

	if (modelID != STATIC_TROPHY) // if not trophy (no lightDir on trophy)
	{
		if (modelID == STATIC_GEM) // gem
		{
			lightDir = &warppadObj->lightDirGem;
		}
		else
		{
			if (modelID == STATIC_RELIC) // relic
			{
				lightDir = &warppadObj->lightDirRelic;
			}
			else
			{
				if (modelID == STATIC_TOKEN) // token
				{
					lightDir = &warppadObj->lightDirToken;
				}
				else
				{
					goto SpinReward;
				}
			}
		}
		Vector_SpecLightSpin3D(prizeInst, &warppadObj->spinRot_Prize, lightDir);
	}

SpinReward:;

	// initialized as AH_WP_REWARD_SPACING_ANGLE * index, but not const
	u32 thirds = warppadObj->thirds[index];

	u32 trig = MATH_Sin(thirds);
	prizeInst->matrix.t[1] = y + ((trig << 6) >> 0xc) + AH_WP_FLOATING_OBJECT_HEIGHT;

	// do not use original "thirds",
	// set new value without "+="
	thirds = AH_WP_REWARD_SPACING_ANGLE * index + warppadObj->spinRot_Rewards.y;

	trig = MATH_Sin(thirds);
	prizeInst->matrix.t[0] = x + (trig * 0xA0 >> 0xc);

	trig = MATH_Cos(thirds);
	prizeInst->matrix.t[2] = z + (trig * 0xA0 >> 0xc);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800abf48-0x800ad2c8.
void AH_WarpPad_ThTick(struct Thread *t)
{
	int i;
	b32 boolOpen = false;
	struct GameTracker *gGT = sdata->gGT;
	struct WarpPad *warppadObj = t->object;
	struct Instance *warppadInst = t->inst;
	struct Instance **visInstSrc = gGT->cameraDC[0].visInstSrc;
	struct Instance **instArr = &warppadObj->inst[0];
	MATRIX *warppadMatrix = &warppadInst->matrix;

	int modelID;
	int levelID = warppadObj->levelID;
	int x, y, z, dist;
	char *warppadLNG;

	int angleCamToWarppad;
	int angleSin, angleCos;

	int wispMaxHeight;
	int wispRiseRate;
	int rng1;
	int rng2;

	int rewardScale;
	int rewardScale2;

	int champID;
	int champSlot;

	char randKartSpawn[8];

#if defined(CTR_NATIVE)
	// NOTE(aalhendi): Retail can read PS1 low RAM when the hub-swap frame
	// leaves this list null; native treats that as an empty visible-instance list.
	if (visInstSrc != NULL)
#endif
	{
		while (visInstSrc[0] != 0)
		{
			if (visInstSrc[0] == warppadInst)
			{
				boolOpen = true;
				break;
			}

			visInstSrc++;
		}
	}

	// make instances visible
	if (boolOpen)
	{
		for (i = 0; i < WPIS_NUM_INSTANCES; i++)
		{
			if (instArr[i] != 0)
			{
				instArr[i]->flags &= ~HIDE_MODEL;
			}
		}
	}

	// make instances invisible
	else
	{
		for (i = 0; i < WPIS_NUM_INSTANCES; i++)
		{
			if (instArr[i] != 0)
			{
				instArr[i]->flags |= HIDE_MODEL;
			}
		}
	}

	warppadInst->flags |= HIDE_MODEL;

	struct Driver *driver = gGT->drivers[0];
	struct Instance *driverInst = driver->instSelf;

	x = warppadMatrix->t[0] - driverInst->matrix.t[0];
	y = warppadMatrix->t[1] - driverInst->matrix.t[1];
	z = warppadMatrix->t[2] - driverInst->matrix.t[2];
	dist = x * x + y * y + z * z;

	// if near a portal
	if (
	    // Trophy tracks (-16)
	    ((levelID < AH_WP_ID_SLIDE_COLISEUM) && (dist < AH_WP_LONG_RANGE_NEAR_DIST_SQ)) ||

	    // Slide Col + Turbo Track (-16)
	    ((((u16)(levelID - AH_WP_ID_SLIDE_COLISEUM)) < 2) && (dist < AH_WP_SHORT_RANGE_NEAR_DIST_SQ)) ||

	    // Battle tracks (-18)
	    ((((u16)(levelID - AH_WP_ID_FIRST_BATTLE_TRACK)) < AH_WP_BATTLE_TRACK_ID_SPAN) && (dist < AH_WP_LONG_RANGE_NEAR_DIST_SQ)) ||

	    // Gem cups
	    ((levelID >= AH_WP_ID_FIRST_GEM_CUP) && (dist < AH_WP_SHORT_RANGE_NEAR_DIST_SQ)))
	{
		// if you are near a new warppad, or if you already were
		// determined as near the same warppad in the last frame,
		// then use this warppad as the "closest". Otherwise the
		// game could run this for two warppads right next to each other
		if ((D232.levelID == -1) || (D232.levelID == levelID))
		{
			// saved as nearest warppad
			D232.levelID = levelID;


			// if not giving Aku Hint
			if (sdata->AkuAkuHintState == 0)
			{
				// default
				if (levelID < AH_WP_ID_FIRST_GEM_CUP)
				{
					warppadLNG = sdata->lngStrings[data.metaDataLEV[levelID].name_LNG];
				}
				// gem cups
				else
				{
					warppadLNG = sdata->lngStrings[data.AdvCups[levelID - AH_WP_ID_FIRST_GEM_CUP].lngIndex_CupName];
				}

				// midpoing X,
				// 30 pixels above botttom Y
				DecalFont_DrawLine(warppadLNG, gGT->pushBuffer[0].rect.x + gGT->pushBuffer[0].rect.w / 2,
				                   gGT->pushBuffer[0].rect.y + gGT->pushBuffer[0].rect.h - 30, FONT_BIG, (JUSTIFY_CENTER | ORANGE));
			}

			// if track is unlocked, ignore all other ELSE-IFs
			if (instArr[WPIS_CLOSED_1S] == 0)
			{
			}

			else if (

			    // gem cup
			    (levelID >= AH_WP_ID_FIRST_GEM_CUP) &&

			    // Dont have hint "you must have 4 tokens for a gem"
			    !CHECK_ADV_BIT(sdata->advProgress.rewards, ADV_REWARD_HINT_GEM_CUPS_CHALLENGE)

			)
			{
				// give hint "you must have 4 tokens for a gem"
				MainFrame_RequestMaskHint(ADV_MASK_HINT_ID_GEM_CUPS_CHALLENGE, 0);
			}

			else if (

			    // Trophy track
			    (levelID < AH_WP_ID_SLIDE_COLISEUM) &&

			    // Dont have hint "you must have more trophies"
			    !CHECK_ADV_BIT(sdata->advProgress.rewards, ADV_REWARD_HINT_NEED_MORE_TROPHIES) &&

			    // required item is not KEY
			    (instArr[WPIS_CLOSED_ITEM]->model->id != STATIC_KEY))
			{
				// give hint for "need more trophies"
				MainFrame_RequestMaskHint(ADV_MASK_HINT_ID_NEED_MORE_TROPHIES, 0);
			}

			else if (

			    // Slide Col
			    (levelID == AH_WP_ID_SLIDE_COLISEUM) &&

			    // Dont have hint "you must have 10 relics"
			    !CHECK_ADV_BIT(sdata->advProgress.rewards, ADV_REWARD_HINT_MUST_GET_10_RELICS))
			{
				// give hint for "need more trophies"
				MainFrame_RequestMaskHint(ADV_MASK_HINT_ID_MUST_GET_10_RELICS, 0);
			}
		}
	}

	// not near portal
	else
	{
		D232.levelID = -1;
	}

	// if warppad is locked
	if (instArr[WPIS_CLOSED_1S] != 0)
	{
		angleCamToWarppad = ratan2(warppadMatrix->t[0] - gGT->pushBuffer[0].pos.x, warppadMatrix->t[2] - gGT->pushBuffer[0].pos.z);

		angleCamToWarppad = -angleCamToWarppad;

		angleSin = MATH_Sin(angleCamToWarppad);
		angleCos = MATH_Cos(angleCamToWarppad);

		// no 10s digit
		if (instArr[WPIS_CLOSED_10S] == 0)
		{
			instArr[WPIS_CLOSED_1S]->matrix.t[0] = warppadMatrix->t[0] + (angleCos * AH_WP_CLOSED_SINGLE_DIGIT_OFFSET >> 0xC);
			instArr[WPIS_CLOSED_1S]->matrix.t[2] = warppadMatrix->t[2] + (angleSin * AH_WP_CLOSED_SINGLE_DIGIT_OFFSET >> 0xC);

			instArr[WPIS_CLOSED_ITEM]->matrix.t[0] = warppadMatrix->t[0] + (angleCos * AH_WP_CLOSED_SINGLE_ITEM_OFFSET >> 0xC);
			instArr[WPIS_CLOSED_ITEM]->matrix.t[2] = warppadMatrix->t[2] + (angleSin * AH_WP_CLOSED_SINGLE_ITEM_OFFSET >> 0xC);
		}

		// 10s digit
		else
		{
			instArr[WPIS_CLOSED_ITEM]->matrix.t[0] = warppadMatrix->t[0] + (angleCos * AH_WP_CLOSED_ITEM_WITH_TENS_OFFSET >> 0xC);
			instArr[WPIS_CLOSED_ITEM]->matrix.t[2] = warppadMatrix->t[2] + (angleSin * AH_WP_CLOSED_ITEM_WITH_TENS_OFFSET >> 0xC);

			instArr[WPIS_CLOSED_X]->matrix.t[0] = warppadMatrix->t[0] + (angleCos * AH_WP_CLOSED_X_WITH_TENS_OFFSET >> 0xC);
			instArr[WPIS_CLOSED_X]->matrix.t[2] = warppadMatrix->t[2] + (angleSin * AH_WP_CLOSED_X_WITH_TENS_OFFSET >> 0xC);

			instArr[WPIS_CLOSED_10S]->matrix.t[0] = warppadMatrix->t[0] + (angleCos * AH_WP_CLOSED_TENS_OFFSET >> 0xC);
			instArr[WPIS_CLOSED_10S]->matrix.t[2] = warppadMatrix->t[2] + (angleSin * AH_WP_CLOSED_TENS_OFFSET >> 0xC);

			instArr[WPIS_CLOSED_1S]->matrix.t[0] = warppadMatrix->t[0] + (angleCos * AH_WP_CLOSED_ONES_WITH_TENS_OFFSET >> 0xC);
			instArr[WPIS_CLOSED_1S]->matrix.t[2] = warppadMatrix->t[2] + (angleSin * AH_WP_CLOSED_ONES_WITH_TENS_OFFSET >> 0xC);
		}

		warppadObj->spinRot_Prize.x = 0;
		warppadObj->spinRot_Prize.z = 0;

		warppadObj->spinRot_Prize.y += AH_WP_SPIN_PRIZE_STEP;

		struct Instance *closedItemInst = instArr[WPIS_CLOSED_ITEM];

		// converted to TEST in rebuildPS1
		ConvertRotToMatrix(&closedItemInst->matrix, &warppadObj->spinRot_Prize);

		modelID = closedItemInst->model->id;

		// Trophy has no specular light
		if (modelID == STATIC_TROPHY)
		{
			return;
		}

		// NOTE(aalhendi): Retail passes the per-WarpPad spec-light arrays at
		// offsets 0x50/0x58/0x60.

		// Relic
		if (modelID == STATIC_RELIC)
		{
			Vector_SpecLightSpin3D(closedItemInst, &warppadObj->spinRot_Prize, &warppadObj->lightDirRelic);
			return;
		}

		// Token
		if (modelID == STATIC_TOKEN)
		{
			Vector_SpecLightSpin3D(closedItemInst, &warppadObj->spinRot_Prize, &warppadObj->lightDirToken);
			return;
		}

		// If Gem, change colors every 2 seconds
		if (modelID == STATIC_GEM)
		{
			i = (gGT->timer / 0x3C) % 5;

			closedItemInst->colorRGBA = INST_PackColorRGB(data.AdvCups[i].color[0], data.AdvCups[i].color[1], data.AdvCups[i].color[2]);
		}

		// for Key or Gem
		Vector_SpecLightSpin3D(closedItemInst, &warppadObj->spinRot_Prize, &warppadObj->lightDirGem);
		return;
	}

	// === Assume Unlocked ===

	if ((dist > AH_WP_WARP_TRIGGER_DIST_SQ) && (warppadObj->boolEnteredWarppad == 0))
	{
		goto WarpPad_AnimateOpen;
	}

	// Retail repeats this setup every close/warping frame before the
	// transition/load gate.
	LOAD_Robots1P(data.characterIDs[0]);

	// variable reuse, get track speed champion
	champID = data.metaDataLEV[levelID].characterID_Champion;

	// default
	champSlot = 0;

	// If Speed Champion is on the track (Crash-Pura)
	// and is not the same characterID as this driver
	if ((champID < AH_WP_RACER_SLOT_COUNT) && (champID != data.characterIDs[driver->driverID]))
	{
		// set everyone to spawn in order
		for (i = 1; i < AH_WP_RACER_SLOT_COUNT; i++)
		{
			if (champID == data.characterIDs[i])
			{
				sdata->kartSpawnOrderArray[i] = 0;
				champSlot = i;
			}

			else if (i == 7)
			{
				sdata->kartSpawnOrderArray[AH_WP_RACER_SLOT_COUNT - 1] = champSlot;
			}

			else
			{
				sdata->kartSpawnOrderArray[i] = i;
			}
		}
	}

	// Speed Champion is invalid
	else
	{
		for (i = 1; i < AH_WP_RACER_SLOT_COUNT; i++)
		{
			randKartSpawn[i] = i;
		}

		for (i = 0; i < AH_WP_RACER_SLOT_COUNT - 1; i++)
		{
			rng1 = RngDeadCoed(&sdata->advRng);

			rng2 = AH_WP_RACER_SLOT_COUNT - 1 - i;

			rng2 = (rng1 & 0xfff) % rng2 + 1;
			rng2 = (s16)rng2;

			sdata->kartSpawnOrderArray[(s32)randKartSpawn[rng2]] = (char)i;

			while (rng2 < AH_WP_RACER_SLOT_COUNT - 1)
			{
				randKartSpawn[rng2] = randKartSpawn[rng2 + 1];
				rng2++;
			}
		}
	}

	// spawn P1 in the back
	sdata->kartSpawnOrderArray[0] = 7;

	// if flag is on-screen, loading has already been finalized
	if (RaceFlag_IsTransitioning())
	{
		goto WarpPad_AnimateOpen;
	}

	levelID = warppadObj->levelID;

	// gem cups
	if (levelID >= AH_WP_ID_FIRST_GEM_CUP)
	{
		warppadObj->boolEnteredWarppad = 1;
		warppadObj->framesWarping++;
		gGT->drivers[0]->funcPtrs[DRIVER_FUNC_INIT] = VehStuckProc_Warp_Init;
		if (warppadObj->framesWarping < AH_WP_WARP_LOAD_FRAMES)
		{
			goto WarpPad_AnimateOpen;
		}

		sdata->Loading.OnBegin.AddBitsConfig0 |= ADVENTURE_CUP;

		gGT->cup.cupID = levelID - AH_WP_ID_FIRST_GEM_CUP;
		gGT->cup.trackIndex = 0;
		for (i = 0; i < AH_WP_RACER_SLOT_COUNT; i++)
		{
			gGT->cup.points[i] = 0;
		}

		levelID = data.advCupTrackIDs[4 * gGT->cup.cupID];
		goto WarpPad_RequestLoad;
	}

	// Slide Col or Turbo Track
	if (((u16)(levelID - AH_WP_ID_SLIDE_COLISEUM)) < 2)
	{
		warppadObj->boolEnteredWarppad = 1;
		warppadObj->framesWarping++;
		gGT->drivers[0]->funcPtrs[DRIVER_FUNC_INIT] = VehStuckProc_Warp_Init;
		if (warppadObj->framesWarping < AH_WP_WARP_LOAD_FRAMES)
		{
			goto WarpPad_AnimateOpen;
		}

		sdata->Loading.OnBegin.AddBitsConfig0 |= RELIC_RACE;
		goto WarpPad_RequestLoad;
	}

	// Battle Tracks
	if ((((u16)(levelID - AH_WP_ID_FIRST_BATTLE_TRACK)) < 2) || (levelID == AH_WP_ID_SKULL_ROCK) || (levelID == AH_WP_ID_ROCKY_ROAD))
	{
		warppadObj->boolEnteredWarppad = 1;
		warppadObj->framesWarping++;
		gGT->drivers[0]->funcPtrs[DRIVER_FUNC_INIT] = VehStuckProc_Warp_Init;
		if (warppadObj->framesWarping < AH_WP_WARP_LOAD_FRAMES)
		{
			goto WarpPad_AnimateOpen;
		}

		sdata->Loading.OnBegin.AddBitsConfig0 |= CRYSTAL_CHALLENGE;

		// Dont have hint "collect every crystal"
		if (!CHECK_ADV_BIT(sdata->advProgress.rewards, ADV_REWARD_HINT_COLLECT_EVERY_CRYSTAL))
		{
			MainFrame_RequestMaskHint(ADV_MASK_HINT_ID_COLLECT_EVERY_CRYSTAL, 1);
		}

		// if can't spawn aku cause he's already here,
		// quit function, wait till he's done to start race
		if (!AH_MaskHint_boolCanSpawn())
		{
			goto WarpPad_AnimateOpen;
		}

		gGT->originalEventTime = D232.battleCrystalEventTime[levelID - AH_WP_ID_FIRST_BATTLE_TRACK];
		goto WarpPad_RequestLoad;
	}

	if (levelID < AH_WP_ID_SLIDE_COLISEUM)
	{
		if (CHECK_ADV_BIT(sdata->advProgress.rewards, levelID + ADV_REWARD_FIRST_TROPHY))
		{
			if (gGT->currAdvProfile.numTrophies >= data.metaDataLEV[levelID].numTrophiesToOpen)
			{
				if (warppadObj->framesWarping < AH_WP_WARP_LOAD_FRAMES)
				{
					goto WarpPad_TrophyAnimateOnly;
				}

				// if never opened
				if (sdata->boolOpenTokenRelicMenu == 0)
				{
					if ((gGT->gameMode1 & ADVENTURE_ARENA) != 0)
					{
						D232.menuTokenRelic.rowSelected = CHECK_ADV_BIT(sdata->advProgress.rewards, levelID + ADV_REWARD_FIRST_CTR_TOKEN);

						RECTMENU_Show(&D232.menuTokenRelic);

						// now opened
						sdata->boolOpenTokenRelicMenu = 1;
					}
				}

				// if opened, but not closed yet
				if (!RECTMENU_BoolHidden(&D232.menuTokenRelic))
				{
					goto WarpPad_TrophyAnimateOnly;
				}

				// Relic Hint
				i = ADV_MASK_HINT_ID_RELIC_CHALLENGE;

				// CTR Token Hint
				if ((gGT->gameMode2 & 8) != 0)
				{
					i = ADV_MASK_HINT_ID_CTR_TOKEN_CHALLENGE;
				}

				// if hint is locked
				if (!CHECK_ADV_BIT(sdata->advProgress.rewards, ADV_REWARD_FIRST_HINT + i))
				{
					MainFrame_RequestMaskHint(i, 1);
				}

				// if can't spawn aku cause he's already here,
				// quit function, wait till he's done to start race
				if (!AH_MaskHint_boolCanSpawn())
				{
					goto WarpPad_TrophyAnimateOnly;
				}

				// reset for future gameplay
				sdata->boolOpenTokenRelicMenu = 0;
				warppadObj->boolEnteredWarppad = 0;

				// Rem Adventure Arena
				sdata->Loading.OnBegin.RemBitsConfig0 |= ADVENTURE_ARENA;

				MainRaceTrack_RequestLoad(levelID);
				goto WarpPad_TrophyAnimateOnly;
			}
		}
	}

	if (CHECK_ADV_BIT(sdata->advProgress.rewards, levelID + ADV_REWARD_FIRST_TROPHY))
	{
		i = data.metaDataLEV[levelID].hubID + ADV_REWARD_BOSS_KEY_HUB_ID_BASE;

		if (!CHECK_ADV_BIT(sdata->advProgress.rewards, i))
		{
			goto WarpPad_AnimateOpen;
		}
	}

	warppadObj->boolEnteredWarppad = 1;
	warppadObj->framesWarping++;
	gGT->drivers[0]->funcPtrs[DRIVER_FUNC_INIT] = VehStuckProc_Warp_Init;
	if (warppadObj->framesWarping < AH_WP_WARP_LOAD_FRAMES)
	{
		goto WarpPad_AnimateOpen;
	}

WarpPad_RequestLoad:

	// Rem Adventure Arena
	sdata->Loading.OnBegin.RemBitsConfig0 |= ADVENTURE_ARENA;

	MainRaceTrack_RequestLoad(levelID);
	goto WarpPad_AnimateOpen;

WarpPad_TrophyAnimateOnly:

	if (warppadObj->framesWarping < AH_WP_TROPHY_PORTAL_HOLD_FRAMES)
	{
		warppadObj->framesWarping++;
	}

	warppadObj->boolEnteredWarppad = 1;

	gGT->drivers[0]->funcPtrs[DRIVER_FUNC_INIT] = VehStuckProc_Warp_Init;

WarpPad_AnimateOpen:

	if ((instArr[WPIS_OPEN_BEAM] != 0) && ((gGT->timer & 1) != 0))
	{
		warppadObj->spinRot_Beam.x = 0;
		warppadObj->spinRot_Beam.z = 0;

		// what on earth was this RNG?
		// how'd they come up with something so random, that looks so good?
		i = MixRNG_Scramble();
		warppadObj->spinRot_Beam.y += ((s16)(i >> 3) + (s16)((i >> 3) / 6) * -6 + 1) * AH_WP_SPIN_BEAM_STEP;

		// converted to TEST in rebuildPS1
		ConvertRotToMatrix(&instArr[WPIS_OPEN_BEAM]->matrix, &warppadObj->spinRot_Beam);
	}

	wispRiseRate = AH_WP_WISP_RISE_RATE;

	wispMaxHeight = AH_WP_WISP_FAR_MAX_HEIGHT;

	// if close to this warppad
	if (D232.levelID != -1)
	{
		wispMaxHeight = AH_WP_WISP_NEAR_MAX_HEIGHT;
	}

	for (i = 0; i < AH_WP_WISP_COUNT; i++)
	{
		if (instArr[WPIS_OPEN_RING1 + i] != 0)
		{
			warppadObj->spinRot_Wisp[i].x = 0;
			warppadObj->spinRot_Wisp[i].z = 0;

			warppadObj->spinRot_Wisp[i].y += AH_WP_SPIN_WISP_STEP;

			// converted to TEST in rebuildPS1
			ConvertRotToMatrix(&instArr[WPIS_OPEN_RING1 + i]->matrix, &warppadObj->spinRot_Wisp[i]);

			// if height hasn't reached max height
			if (instArr[WPIS_OPEN_RING1 + i]->matrix.t[1] < (warppadInst->matrix.t[1] + wispMaxHeight))
			{
				instArr[WPIS_OPEN_RING1 + i]->matrix.t[1] += wispRiseRate;

				// if height hasn't reached 4x RiseRate,
				// first 4 frames of rising
				if (instArr[WPIS_OPEN_RING1 + i]->matrix.t[1] < (warppadInst->matrix.t[1] + wispRiseRate * AH_WP_WISP_FIRST_FRAMES))
				{
					// reduce transparency
					instArr[WPIS_OPEN_RING1 + i]->alphaScale -= AH_WP_WISP_FADE_IN_STEP;
				}

				// after first 4 frames
				else
				{
					// add transparency as the wisp spirals upward (~0x60  per frame)
					instArr[WPIS_OPEN_RING1 + i]->alphaScale += AH_WP_WISP_FADE_OUT_RANGE / (wispMaxHeight / wispRiseRate);
				}
			}

			// eached max height
			else
			{
				// reset height
				instArr[WPIS_OPEN_RING1 + i]->matrix.t[1] = warppadInst->matrix.t[1];

				// full transparency
				instArr[WPIS_OPEN_RING1 + i]->alphaScale = AH_WP_FULL_ALPHA;

				rng1 = MixRNG_Scramble() >> 3;

				rng2 = rng1;
				if (rng1 < 0)
				{
					rng2 = rng1 + 0xfff;
				}

				warppadObj->spinRot_Wisp[i].y = (s16)rng1 + (s16)(rng2 >> 0xc) * -AH_WP_FULL_ALPHA;
			}
		}

		wispRiseRate += AH_WP_WISP_RISE_RATE_STEP;
	}

	warppadObj->spinRot_Prize.y += AH_WP_SPIN_PRIZE_REWARD_STEP;

	rewardScale = 0x100;

	if (dist > AH_WP_REWARD_FADE_DIST_SQ * 2)
	{
		rewardScale = 0;
	}

	else if (dist > AH_WP_REWARD_FADE_DIST_SQ)
	{
		// range [90, 90*2] to [0%, 100%]
		rewardScale = ((((AH_WP_REWARD_FADE_DIST_SQ * 2) - dist) * 0x100) / AH_WP_REWARD_FADE_DIST_SQ);
	}

	for (i = 0; i < AH_WP_REWARD_INSTANCE_COUNT; i++)
	{
		warppadObj->spinRot_Prize.z = AH_WP_PRIZE_TILT_Z;

		if (instArr[WPIS_OPEN_PRIZE1 + i] != 0)
		{
			AH_WarpPad_SpinRewards(instArr[WPIS_OPEN_PRIZE1 + i], warppadObj, i, warppadInst->matrix.t[0], warppadInst->matrix.t[1], warppadInst->matrix.t[2]);

			modelID = instArr[WPIS_OPEN_PRIZE1 + i]->model->id;

			if (rewardScale == 0)
			{
				// invisible
				instArr[WPIS_OPEN_PRIZE1 + i]->flags |= HIDE_MODEL;
			}

			else
			{
				// visible
				instArr[WPIS_OPEN_PRIZE1 + i]->flags &= ~HIDE_MODEL;

				// token
				rewardScale2 = AH_WP_STANDARD_ITEM_SCALE;

				// not token
				if (modelID != STATIC_TOKEN)
				{
					// trophy
					rewardScale2 = AH_WP_TROPHY_PRIZE_SCALE;

					// relic
					if (modelID == STATIC_RELIC)
					{
						rewardScale2 = AH_WP_RELIC_PRIZE_SCALE;
					}
				}

				rewardScale2 = (u32)(rewardScale2 * rewardScale) >> 8;
				CTR_SET_VEC3(instArr[WPIS_OPEN_PRIZE1 + i]->scale.v, (s16)rewardScale2, (s16)rewardScale2, (s16)rewardScale2);
			}
		}

		warppadObj->thirds[i] += AH_WP_REWARD_PHASE_STEP;
		warppadObj->spinRot_Rewards.y += AH_WP_SPIN_REWARD_RING_STEP;
	}

	if (instArr[WPIS_CLOSED_1S] != 0)
	{
		INSTANCE_Death(instArr[WPIS_CLOSED_1S]);
		INSTANCE_Death(instArr[WPIS_CLOSED_10S]);
		INSTANCE_Death(instArr[WPIS_CLOSED_X]);
		INSTANCE_Death(instArr[WPIS_CLOSED_ITEM]);
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ad2c8-0x800ad3ec.
void AH_WarpPad_ThDestroy(struct Thread *t)
{
	struct WarpPad *warppadObj = t->object;

	// array of instances in warppad object
	struct Instance **instArr = &warppadObj->inst[0];

	if (instArr[WPIS_CLOSED_1S] != 0)
	{
		INSTANCE_Death(instArr[WPIS_CLOSED_1S]);
		instArr[WPIS_CLOSED_1S] = 0;
	}

	if (instArr[WPIS_CLOSED_10S] != 0)
	{
		INSTANCE_Death(instArr[WPIS_CLOSED_10S]);
		instArr[WPIS_CLOSED_10S] = 0;
	}

	if (instArr[WPIS_CLOSED_X] != 0)
	{
		INSTANCE_Death(instArr[WPIS_CLOSED_X]);
		instArr[WPIS_CLOSED_X] = 0;
	}

	if (instArr[WPIS_CLOSED_ITEM] != 0)
	{
		INSTANCE_Death(instArr[WPIS_CLOSED_ITEM]);
		instArr[WPIS_CLOSED_ITEM] = 0;
	}

	if (instArr[WPIS_OPEN_BEAM] != 0)
	{
		INSTANCE_Death(instArr[WPIS_OPEN_BEAM]);
		instArr[WPIS_OPEN_BEAM] = 0;
	}

	for (int i = WPIS_OPEN_RING1; i < WPIS_OPEN_PRIZE1; i++)
	{
		if (instArr[i] != 0)
		{
			INSTANCE_Death(instArr[i]);
			instArr[i] = 0;
		}
	}

	for (int i = WPIS_OPEN_PRIZE1; i < WPIS_NUM_INSTANCES; i++)
	{
		if (instArr[i] != 0)
		{
			INSTANCE_Death(instArr[i]);
			instArr[i] = 0;
		}
	}
}

static const s16 s_warpPadRewardModelIDs[3] = {STATIC_TROPHY, STATIC_RELIC, STATIC_TOKEN};

// NOTE(aalhendi): Source-backed for NTSC-U 926 0x800ad3ec-0x800ae870.
void AH_WarpPad_LInB(struct Instance *inst)
{
	int i;
	int levelID = 0;
	struct Thread *t;
	struct WarpPad *warppadObj;
	struct GameTracker *gGT = sdata->gGT;

	int unlockItem_numOwned;
	int unlockItem_numNeeded;
	int unlockItem_modelID;
	int rewardModelID;
	int rewardAngle;
	int tokenGroupID;

	struct Instance *newInst;

	if (inst->thread != NULL)
	{
		return;
	}

	t = PROC_BirthWithObject(SIZE_RELATIVE_POOL_BUCKET(sizeof(struct WarpPad), NONE, MEDIUM, WARPPAD),

	                         AH_WarpPad_ThTick, // behavior
	                         "warppad",         // debug name
	                         0                  // thread relative
	);

	if (t == 0)
	{
		return;
	}
	inst->thread = t;
	t->inst = inst;

	t->funcThDestroy = AH_WarpPad_ThDestroy;

	// locked
	t->modelIndex = AH_WP_VISUAL_LOCKED;

	// make invisible
	// this is the red triangle
	// instance from DCxDemo's LEV Viewer
	inst->flags |= HIDE_MODEL;

	warppadObj = t->object;
	warppadObj->levelID = 0; // this is dingo canyon
	warppadObj->boolEnteredWarppad = 0;
	warppadObj->framesWarping = 0;

	for (i = 0; i < WPIS_NUM_INSTANCES; i++)
	{
		warppadObj->inst[i] = 0;
	}

	// each warppad has a name "warppad#xxx"
	// "warppad#0" is dingo canyon, level ID 0
	// "warppad#16" is slide col, level ID 16
	// "warppad#102" is gem cup 2
	// "warppad#104" is gem cup 4
	// etc

	for (i = AH_WP_NAME_LEVEL_ID_OFFSET; inst->name[i] != 0; i++)
	{
		levelID = levelID * 10 + inst->name[i] - '0';
	}

	warppadObj->levelID = levelID;

	unlockItem_modelID = 0;
	unlockItem_numOwned = 0;
	unlockItem_numNeeded = -1;

	// Trophy Track
	if (levelID < AH_WP_ID_SLIDE_COLISEUM)
	{
		// optimization idea:
		// instead of data.metaDataLEV[levelID].hubID
		// can we just do gGT->levelID-0x19?

		// if trophy owned
		if (CHECK_ADV_BIT(sdata->advProgress.rewards, levelID + ADV_REWARD_FIRST_TROPHY))
		{
		GetKeysRequirement:

			// keys needed to unlock track again
			unlockItem_modelID = STATIC_KEY;
			unlockItem_numOwned = gGT->currAdvProfile.numKeys;
			unlockItem_numNeeded = D232.keysNeededByHub[data.metaDataLEV[levelID].hubID];
		}

		// if trophy not owned
		else
		{
			// number trophies needed to open
			unlockItem_modelID = STATIC_TROPHY;
			unlockItem_numOwned = gGT->currAdvProfile.numTrophies;
			unlockItem_numNeeded = data.metaDataLEV[levelID].numTrophiesToOpen;
		}
	}

	// Slide Col
	else if (levelID == AH_WP_ID_SLIDE_COLISEUM)
	{
		// number relics needed to open
		unlockItem_modelID = STATIC_RELIC;
		unlockItem_numOwned = gGT->currAdvProfile.numRelics;
		unlockItem_numNeeded = 10;
	}

	// Turbo Track
	else if (levelID == AH_WP_ID_TURBO_TRACK)
	{
		// number gems needed to open
		unlockItem_modelID = STATIC_GEM;
		unlockItem_numNeeded = 5;

		// count number of gems owned
		unlockItem_numOwned = 0;
		for (i = 0; i < AH_WP_GEM_CUP_COUNT; i++)
		{
			if (CHECK_ADV_BIT(sdata->advProgress.rewards, ADV_REWARD_FIRST_GEM + i))
			{
				unlockItem_numOwned++;
			}
		}
	}

	// battle maps
	else if ((((u16)(levelID - AH_WP_ID_FIRST_BATTLE_TRACK)) < 2) || (levelID == AH_WP_ID_SKULL_ROCK) || (levelID == AH_WP_ID_ROCKY_ROAD))
	{
		goto GetKeysRequirement;
	}

	// gem cups
	else if (((u16)(levelID - AH_WP_ID_FIRST_GEM_CUP)) < AH_WP_GEM_CUP_COUNT)
	{
		// number tokens needed to open
		unlockItem_modelID = STATIC_TOKEN;
		unlockItem_numNeeded = 4;

		int *arrTokenCount = &gGT->currAdvProfile.numCtrTokens.red;
		unlockItem_numOwned = arrTokenCount[levelID - AH_WP_ID_FIRST_GEM_CUP];
	}

	// if unlocked
	if (unlockItem_numOwned >= unlockItem_numNeeded)
	{
		warppadObj->digit1s = 0;
		t->modelIndex = AH_WP_VISUAL_TROPHY_OPEN;

		// if beam model exists
		if (gGT->modelPtr[STATIC_BEAM] != 0)
		{
			newInst = INSTANCE_Birth3D(gGT->modelPtr[STATIC_BEAM], "beam", t);

			CTR_MatrixCopyRot(&newInst->matrix, &inst->matrix);
			newInst->matrix.t[0] = inst->matrix.t[0];
			newInst->matrix.t[1] = inst->matrix.t[1];
			newInst->matrix.t[2] = inst->matrix.t[2];

			newInst->alphaScale = AH_WP_OPEN_BEAM_ALPHA;

			warppadObj->inst[WPIS_OPEN_BEAM] = newInst;
		}

		// if spiral ring exists
		if (gGT->modelPtr[STATIC_BOTTOMRING] != 0)
		{
			for (i = 0; i < AH_WP_WISP_COUNT; i++)
			{
				newInst = INSTANCE_Birth3D(gGT->modelPtr[STATIC_BOTTOMRING], "bottomRing", t);

				CTR_MatrixCopyRot(&newInst->matrix, &inst->matrix);
				newInst->matrix.t[0] = inst->matrix.t[0];
				newInst->matrix.t[1] = inst->matrix.t[1] + i * AH_WP_OPEN_RING_HEIGHT_STEP;
				newInst->matrix.t[2] = inst->matrix.t[2];

				newInst->alphaScale = AH_WP_OPEN_RING_ALPHA;

				warppadObj->inst[WPIS_OPEN_RING1 + i] = newInst;
			}
		}

		for (i = 0; i < AH_WP_REWARD_INSTANCE_COUNT; i++)
		{
			warppadObj->thirds[i] = AH_WP_REWARD_SPACING_ANGLE * i;
		}

		CTR_SET_VEC3(warppadObj->spinRot_Prize.v, 0, 0, 0);

		CTR_SET_VEC3(warppadObj->spinRot_Beam.v, 0, 0, 0);

		for (i = 0; i < AH_WP_WISP_COUNT; i++)
		{
			CTR_SET_VEC3(warppadObj->spinRot_Wisp[i].v, 0, 0, 0);
		}

		if (levelID < AH_WP_ID_SLIDE_COLISEUM)
		{
			// unlocked all
			t->modelIndex = AH_WP_VISUAL_COMPLETE;

			// if trophy not owned
			if (!CHECK_ADV_BIT(sdata->advProgress.rewards, levelID + ADV_REWARD_FIRST_TROPHY))
			{
				// open for trophy
				t->modelIndex = AH_WP_VISUAL_TROPHY_OPEN;

				rewardAngle = 0;
				for (i = 0; i < AH_WP_REWARD_INSTANCE_COUNT; i++)
				{
					rewardModelID = s_warpPadRewardModelIDs[i];
					newInst = INSTANCE_Birth3D(gGT->modelPtr[rewardModelID], "prize1", t);
					warppadObj->inst[WPIS_OPEN_PRIZE1 + i] = newInst;

					CTR_MatrixCopyRot(&newInst->matrix, &inst->matrix);
					newInst->matrix.t[0] = inst->matrix.t[0] + ((MATH_Sin(rewardAngle) * AH_WP_PRIZE_RING_RADIUS) >> 0xc);
					newInst->matrix.t[1] = inst->matrix.t[1] + AH_WP_FLOATING_OBJECT_HEIGHT;
					newInst->matrix.t[2] = inst->matrix.t[2] + ((MATH_Cos(rewardAngle) * AH_WP_PRIZE_RING_RADIUS) >> 0xc);

					if (rewardModelID == STATIC_RELIC)
					{
						newInst->colorRGBA = INST_COLOR_SAPPHIRE_RELIC;
						newInst->flags |= USE_SPECULAR_LIGHT;
						CTR_SET_VEC3(newInst->scale.v, AH_WP_RELIC_PRIZE_SCALE, AH_WP_RELIC_PRIZE_SCALE, AH_WP_RELIC_PRIZE_SCALE);
					}

					else if (rewardModelID == STATIC_TOKEN)
					{
						tokenGroupID = data.metaDataLEV[levelID].ctrTokenGroupID;

						// token color
						newInst->colorRGBA =
						    INST_PackColorRGB(data.AdvCups[tokenGroupID].color[0], data.AdvCups[tokenGroupID].color[1], data.AdvCups[tokenGroupID].color[2]);

						newInst->flags |= (DRAW_TRANSPARENT | USE_SPECULAR_LIGHT);

						warppadObj->lightDirToken = D232.lightDirToken[tokenGroupID];

						CTR_SET_VEC3(newInst->scale.v, AH_WP_STANDARD_ITEM_SCALE, AH_WP_STANDARD_ITEM_SCALE, AH_WP_STANDARD_ITEM_SCALE);
					}

					else
					{
						CTR_SET_VEC3(newInst->scale.v, AH_WP_TROPHY_PRIZE_SCALE, AH_WP_TROPHY_PRIZE_SCALE, AH_WP_TROPHY_PRIZE_SCALE);
					}

					rewardAngle += AH_WP_REWARD_SPACING_ANGLE;
				}

				return;
			}

			// if relic not owned
			if (!CHECK_ADV_BIT(sdata->advProgress.rewards, levelID + ADV_REWARD_FIRST_SAPPHIRE_RELIC))
			{
				// open for relic/token
				t->modelIndex = AH_WP_VISUAL_RELIC_TOKEN_OPEN;

				newInst = INSTANCE_Birth3D(gGT->modelPtr[STATIC_RELIC], "prize2", t);

				// relic blue
				newInst->colorRGBA = INST_COLOR_SAPPHIRE_RELIC;

				newInst->flags |= USE_SPECULAR_LIGHT;

				warppadObj->lightDirRelic = D232.lightDirRelic[0];

				CTR_MatrixCopyRot(&newInst->matrix, &inst->matrix);
				newInst->matrix.t[0] = inst->matrix.t[0];
				newInst->matrix.t[1] = inst->matrix.t[1] + AH_WP_FLOATING_OBJECT_HEIGHT;
				newInst->matrix.t[2] = inst->matrix.t[2];

				CTR_SET_VEC3(newInst->scale.v, AH_WP_RELIC_PRIZE_SCALE, AH_WP_RELIC_PRIZE_SCALE, AH_WP_RELIC_PRIZE_SCALE);

				warppadObj->inst[WPIS_OPEN_PRIZE1] = newInst;
			}

			// if token owned
			if (CHECK_ADV_BIT(sdata->advProgress.rewards, levelID + ADV_REWARD_FIRST_CTR_TOKEN))
			{
				return;
			}

			tokenGroupID = data.metaDataLEV[levelID].ctrTokenGroupID;

			// open for relic/token
			t->modelIndex = AH_WP_VISUAL_RELIC_TOKEN_OPEN;

			newInst = INSTANCE_Birth3D(gGT->modelPtr[STATIC_TOKEN], "prize2", t);

			// token color
			newInst->colorRGBA =
			    INST_PackColorRGB(data.AdvCups[tokenGroupID].color[0], data.AdvCups[tokenGroupID].color[1], data.AdvCups[tokenGroupID].color[2]);

			newInst->flags |= (DRAW_TRANSPARENT | USE_SPECULAR_LIGHT);

			warppadObj->lightDirToken = D232.lightDirToken[tokenGroupID];

			CTR_MatrixCopyRot(&newInst->matrix, &inst->matrix);
			newInst->matrix.t[0] = inst->matrix.t[0];
			newInst->matrix.t[1] = inst->matrix.t[1] + AH_WP_FLOATING_OBJECT_HEIGHT;
			newInst->matrix.t[2] = inst->matrix.t[2];

			CTR_SET_VEC3(newInst->scale.v, AH_WP_STANDARD_ITEM_SCALE, AH_WP_STANDARD_ITEM_SCALE, AH_WP_STANDARD_ITEM_SCALE);

			warppadObj->inst[WPIS_OPEN_PRIZE2] = newInst;

			return;

		SlideColTurboTrack:

			// if relic not owned
			if (levelID < AH_WP_ID_FIRST_BATTLE_TRACK)
			{
				if (!CHECK_ADV_BIT(sdata->advProgress.rewards, levelID + ADV_REWARD_FIRST_SAPPHIRE_RELIC))
				{
					// SlideCol/TurboTrack
					if (levelID >= AH_WP_ID_SLIDE_COLISEUM)
					{
						t->modelIndex = AH_WP_VISUAL_COLOR_CYCLE_OPEN;
					}
					// open for token/relic
					else if (t->modelIndex != AH_WP_VISUAL_TROPHY_OPEN)
					{
						t->modelIndex = AH_WP_VISUAL_RELIC_TOKEN_OPEN;
					}

					newInst = INSTANCE_Birth3D(gGT->modelPtr[STATIC_RELIC], "prize2", t);

					// relic blue
					newInst->colorRGBA = INST_COLOR_SAPPHIRE_RELIC;

					newInst->flags |= USE_SPECULAR_LIGHT;

					warppadObj->lightDirRelic = D232.lightDirRelic[0];

					CTR_SET_VEC3(newInst->scale.v, AH_WP_RELIC_PRIZE_SCALE, AH_WP_RELIC_PRIZE_SCALE, AH_WP_RELIC_PRIZE_SCALE);

					warppadObj->inst[WPIS_OPEN_PRIZE1] = newInst;
				}
			}

			for (i = 0; i < AH_WP_REWARD_INSTANCE_COUNT; i++)
			{
				newInst = warppadObj->inst[WPIS_OPEN_PRIZE1 + i];

				if (newInst == 0)
				{
					continue;
				}

				CTR_MatrixCopyRot(&newInst->matrix, &inst->matrix);
				newInst->matrix.t[0] = inst->matrix.t[0];
				newInst->matrix.t[1] = inst->matrix.t[1] + AH_WP_FLOATING_OBJECT_HEIGHT;
				newInst->matrix.t[2] = inst->matrix.t[2];
			}
		}

		// slide col, turbo track
		else if (levelID < AH_WP_ID_FIRST_BATTLE_TRACK)
		{
			// already unlocked
			t->modelIndex = AH_WP_VISUAL_COMPLETE;

			goto SlideColTurboTrack;
		}

		// battle tracks
		else if ((((u16)(levelID - AH_WP_ID_FIRST_BATTLE_TRACK)) < 2) || (levelID == AH_WP_ID_SKULL_ROCK) || (levelID == AH_WP_ID_ROCKY_ROAD))
		{
			i = R232.battleTrackPurpleTokenOffset[levelID - AH_WP_ID_FIRST_BATTLE_TRACK] + ADV_REWARD_FIRST_PURPLE_TOKEN;

			// already unlocked
			t->modelIndex = AH_WP_VISUAL_COMPLETE;

			if (!CHECK_ADV_BIT(sdata->advProgress.rewards, i))
			{
				// rainbow
				t->modelIndex = AH_WP_VISUAL_COLOR_CYCLE_OPEN;

				newInst = INSTANCE_Birth3D(gGT->modelPtr[STATIC_TOKEN], "prize2", t);

				newInst->flags |= USE_SPECULAR_LIGHT;

				tokenGroupID = AH_WP_PURPLE_TOKEN_GROUP;

				// token color
				newInst->colorRGBA =
				    INST_PackColorRGB(data.AdvCups[tokenGroupID].color[0], data.AdvCups[tokenGroupID].color[1], data.AdvCups[tokenGroupID].color[2]);

				warppadObj->lightDirToken = D232.lightDirToken[tokenGroupID];

				CTR_SET_VEC3(newInst->scale.v, AH_WP_STANDARD_ITEM_SCALE, AH_WP_STANDARD_ITEM_SCALE, AH_WP_STANDARD_ITEM_SCALE);

				warppadObj->inst[WPIS_OPEN_PRIZE1] = newInst;

				// for matrix copy
				goto SlideColTurboTrack;
			}
		}

		// gem cups
		else if (((u16)(levelID - AH_WP_ID_FIRST_GEM_CUP)) < AH_WP_GEM_CUP_COUNT)
		{
			// bit index of gem
			i = (levelID - AH_WP_ID_FIRST_GEM_CUP) + ADV_REWARD_FIRST_GEM;

			// if gem is already unlocked, quit
			if (CHECK_ADV_BIT(sdata->advProgress.rewards, i))
			{
				// beaten
				t->modelIndex = AH_WP_VISUAL_COMPLETE;

				return;
			}

			// rainbow color
			t->modelIndex = AH_WP_VISUAL_COLOR_CYCLE_OPEN;

			newInst = INSTANCE_Birth3D(gGT->modelPtr[STATIC_GEM], "prize2", t);

			newInst->flags |= USE_SPECULAR_LIGHT;

			i = levelID - AH_WP_ID_FIRST_GEM_CUP;

			// token color
			newInst->colorRGBA = INST_PackColorRGB(data.AdvCups[i].color[0], data.AdvCups[i].color[1], data.AdvCups[i].color[2]);

			warppadObj->inst[WPIS_OPEN_PRIZE1] = newInst;

			// store in Gem array
			warppadObj->lightDirGem = D232.lightDirGem[i];

			CTR_SET_VEC3(newInst->scale.v, AH_WP_STANDARD_ITEM_SCALE, AH_WP_STANDARD_ITEM_SCALE, AH_WP_STANDARD_ITEM_SCALE);

			// for matrix copy
			goto SlideColTurboTrack;
		}

		return;
	}

	// === if locked ===

	if (unlockItem_numNeeded < 10)
	{
		warppadObj->digit10s = 0;
		warppadObj->digit1s = unlockItem_numNeeded;
	}

	else
	{
		warppadObj->digit10s = 1;
		warppadObj->digit1s = unlockItem_numNeeded - 10;
	}

	// ====== Item ========

	// WPIS_CLOSED_ITEM
	newInst = INSTANCE_Birth3D(gGT->modelPtr[unlockItem_modelID], "reqObj", t);

	CTR_MatrixCopyRot(&newInst->matrix, &inst->matrix);
	newInst->matrix.t[0] = inst->matrix.t[0];
	newInst->matrix.t[1] = inst->matrix.t[1] + AH_WP_FLOATING_OBJECT_HEIGHT;
	newInst->matrix.t[2] = inst->matrix.t[2];

	CTR_SET_VEC3(newInst->scale.v, AH_WP_STANDARD_ITEM_SCALE, AH_WP_STANDARD_ITEM_SCALE, AH_WP_STANDARD_ITEM_SCALE);

	// no specular for trophy
	if (unlockItem_modelID != STATIC_TROPHY)
	{
		newInst->flags |= USE_SPECULAR_LIGHT;

		// relic
		if (unlockItem_modelID == STATIC_RELIC)
		{
			// Relic blue color
			newInst->colorRGBA = INST_COLOR_SAPPHIRE_RELIC;

			warppadObj->lightDirRelic = D232.lightDirRelic[0];
		}

		// Key
		else if (unlockItem_modelID == STATIC_KEY)
		{
			// Key color
			newInst->colorRGBA = INST_COLOR_KEY;

			// store in Gem array (intended by ND, not a bug)
			warppadObj->lightDirGem = D232.lightDirGem[0];
		}

		// Gem
		else if (unlockItem_modelID == STATIC_GEM)
		{
			newInst->colorRGBA = INST_PackColorRGB(data.AdvCups[0].color[0], data.AdvCups[0].color[1], data.AdvCups[0].color[2]);

			// store in Gem array
			warppadObj->lightDirGem = D232.lightDirGem[0];
		}

		// assume token
		else
		{
			i = levelID - AH_WP_ID_FIRST_GEM_CUP;

			// token color
			newInst->colorRGBA = INST_PackColorRGB(data.AdvCups[i].color[0], data.AdvCups[i].color[1], data.AdvCups[i].color[2]);

			warppadObj->lightDirToken = D232.lightDirToken[i];
		}
	}

	warppadObj->inst[WPIS_CLOSED_ITEM] = newInst;

	// ====== "X" ========

	// WPIS_CLOSED_X
	newInst = INSTANCE_Birth3D(gGT->modelPtr[STATIC_BIGX], "x", t);

	CTR_MatrixSetRotIdentity(&newInst->matrix);
	newInst->matrix.t[0] = inst->matrix.t[0];
	newInst->matrix.t[1] = inst->matrix.t[1] + AH_WP_FLOATING_OBJECT_HEIGHT;
	newInst->matrix.t[2] = inst->matrix.t[2];

	CTR_SET_VEC3(newInst->scale.v, AH_WP_STANDARD_ITEM_SCALE, AH_WP_STANDARD_ITEM_SCALE, AH_WP_STANDARD_ITEM_SCALE);

	// always face camera
	newInst->model->headers[0].flags |= 1;

	warppadObj->inst[WPIS_CLOSED_X] = newInst;

	// ====== "10s" ========

	if (warppadObj->digit10s != 0)
	{
		// WPIS_CLOSED_10S
		newInst = INSTANCE_Birth3D(gGT->modelPtr[STATIC_BIG1], "warpnum", t);

		CTR_MatrixSetRotIdentity(&newInst->matrix);
		newInst->matrix.t[0] = inst->matrix.t[0];
		newInst->matrix.t[1] = inst->matrix.t[1] + AH_WP_FLOATING_OBJECT_HEIGHT;
		newInst->matrix.t[2] = inst->matrix.t[2];

		CTR_SET_VEC3(newInst->scale.v, AH_WP_STANDARD_ITEM_SCALE, AH_WP_STANDARD_ITEM_SCALE, AH_WP_STANDARD_ITEM_SCALE);

		// always face camera
		for (i = 0; i < newInst->model->numHeaders; i++)
		{
			newInst->model->headers[i].flags |= 1;
		}

		warppadObj->inst[WPIS_CLOSED_10S] = newInst;
	}

	// ====== "1s" ========

	// STATIC_BIG (1-8)
	i = AH_WP_DIGIT_MODEL_FIRST_1_TO_8;
	if (warppadObj->digit1s == 0)
	{
		i = AH_WP_DIGIT_MODEL_0;
	}
	if (warppadObj->digit1s == 9)
	{
		i = AH_WP_DIGIT_MODEL_9;
	}

	// WPIS_CLOSED_1S
	newInst = INSTANCE_Birth3D(gGT->modelPtr[i], "warpnum", t);

	CTR_MatrixSetRotIdentity(&newInst->matrix);
	newInst->matrix.t[0] = inst->matrix.t[0];
	newInst->matrix.t[1] = inst->matrix.t[1] + AH_WP_FLOATING_OBJECT_HEIGHT;
	newInst->matrix.t[2] = inst->matrix.t[2];

	CTR_SET_VEC3(newInst->scale.v, AH_WP_STANDARD_ITEM_SCALE, AH_WP_STANDARD_ITEM_SCALE, AH_WP_STANDARD_ITEM_SCALE);

	// always face camera
	for (i = 0; i < newInst->model->numHeaders; i++)
	{
		newInst->model->headers[i].flags |= 1;
	}

	warppadObj->inst[WPIS_CLOSED_1S] = newInst;
}
