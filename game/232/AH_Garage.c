#include <common.h>

enum AHGarageConstants
{
	AH_GARAGE_ACCESS_FLAG = 1,
	AH_GARAGE_NORMAL_OPEN_SFX = 0x95,
	AH_GARAGE_OXIDE_OPEN_SFX = 0x96,
	AH_GARAGE_DOOR_MOVE_STEP = 0x20,
	AH_GARAGE_DOOR_HEIGHT = 0x300,
	AH_GARAGE_DOOR_COOLDOWN_MS = 2 * SECOND,
	AH_GARAGE_TOP_ROT_STEP = 0x40,
	AH_GARAGE_CHALLENGE_NEAR_DIST_MAX_SQ = 0x143fff,
	AH_GARAGE_BOSS_COLLIDE_RADIUS = 0x300,
	AH_GARAGE_BOSS_COLLIDE_RADIUS_SQ = 0x90000,
	AH_GARAGE_INTERIOR_FORWARD_OFFSET = -0x280,
	AH_GARAGE_INTERIOR_DIST_SQ = 0x40000,
	AH_GARAGE_FADE_STEP = -0x2aa,
	AH_GARAGE_OXIDE_FINAL_BOSS_ID = 5,
	AH_GARAGE_TOP_FORWARD_OFFSET = 0x4c,
	AH_GARAGE_TOP_DEPTH_BIAS = 0xfe,
};

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ae8a0-0x800ae8e0.
void AH_Garage_ThDestroy(struct Thread *t)
{
	struct BossGarageDoor *garage = t->object;

	if (garage->garageTopInst != NULL)
	{
		INSTANCE_Death(garage->garageTopInst);
		garage->garageTopInst = NULL;
	}

	return;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ae8e0-0x800ae988.
void AH_Garage_Open(struct ScratchpadStruct *sps, void *hitObject)
{
	struct Thread *otherTh = hitObject;

	if (otherTh->modelIndex != DYNAMIC_PLAYER)
	{
		return;
	}

	struct Thread *garageThread = sps->Union.ThBuckColl.thread;
	struct BossGarageDoor *garage = garageThread->object;
	struct Instance *garageInst = garageThread->inst;

	if (
	    // if door is not opening
	    (garage->direction != BOSS_GARAGE_DOOR_OPENING) &&

	    // if door is closed,
	    // if posY is the same as instDef posY
	    (garageInst->matrix.t[1] == garageInst->instDef->pos.y))
	{
		u32 soundID = AH_GARAGE_NORMAL_OPEN_SFX;

		// if you are not in gemstone valley
		// play sound of normal boss door opening

		// Level ID
		// if you are in Gemstone Valley
		if (sdata->gGT->levelID == GEM_STONE_VALLEY)
		{
			// play sound of oxide door opening
			soundID = AH_GARAGE_OXIDE_OPEN_SFX;
		}

		// Play sound
		OtherFX_Play(soundID, 1);
	}

	// door is now opening
	garage->direction = BOSS_GARAGE_DOOR_OPENING;

	// enable access through a door (disable collision)
	sdata->doorAccessFlags |= AH_GARAGE_ACCESS_FLAG;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ae988-0x800af070.
void AH_Garage_ThTick(struct Thread *t)
{
	b32 bossIsOpen = true;
	s32 i;
	s32 top;
	s32 move;
	s32 ratio;
	s32 bottom;
	const s16 *check;
	u32 bitIndex;
	u32 soundID;
	u32 hintMask;
	Vec3 dist;
	Vec3 pos;
	struct GameTracker *gGT = sdata->gGT;
	struct AdvProgress *adv = &sdata->advProgress;
	struct BossGarageDoor *garage = t->object;
	struct Instance *inst = t->inst;
	struct Instance *drv_inst = gGT->drivers[0]->instSelf;
	s32 levelID = gGT->levelID;
	s32 hubID = levelID - GEM_STONE_VALLEY;
	struct ScratchpadStruct *sps = CTR_SCRATCHPAD_PTR(struct ScratchpadStruct, 0x108);

	// if door is not opening or closing
	if (garage->direction == BOSS_GARAGE_DOOR_STOPPED)
	{
		// if door is fully closed
		if (garage->cooldown == 0)
		{
			inst->flags |= SPLIT_SPECIAL;
			inst->flags &= ~(SPLIT_LINE | REFLECTION_FUNC23 | WATER_SPLIT_WHITE);
		}

		// if door is not fully closed
		else
		{
			// subtract frame timer
			garage->cooldown -= gGT->elapsedTimeMS;

			// if countdown is not done, dont close door
			if (garage->cooldown > 0)
			{
				goto LAB_800aeb6c;
			}

			// play sound of normal boss door opening, except for Oxide
			soundID = (levelID == GEM_STONE_VALLEY) ? AH_GARAGE_OXIDE_OPEN_SFX : AH_GARAGE_NORMAL_OPEN_SFX;

			// Play sound
			OtherFX_Play(soundID, 1);

			// erase cooldown
			garage->cooldown = 0;

			// door is closing
			garage->direction = BOSS_GARAGE_DOOR_CLOSING;

			inst->flags &= ~HIDE_MODEL;
		}
	}
	// if door is opening or closing
	else
	{
		// Increment animation by 0x20 in either direction
		move = inst->matrix.t[1] + garage->direction * AH_GARAGE_DOOR_MOVE_STEP;
		inst->matrix.t[1] = move;

		top = inst->instDef->pos.y + AH_GARAGE_DOOR_HEIGHT;
		bottom = inst->instDef->pos.y;

		// If the door has gone past the top (height=0x300)
		if (move > top)
		{
			// Set position to the top
			inst->matrix.t[1] = top;

			// Door is now open (not moving)
			garage->direction = BOSS_GARAGE_DOOR_STOPPED;

			// Cooldown for 2 seconds
			garage->cooldown = AH_GARAGE_DOOR_COOLDOWN_MS;

			// Make invisible
			inst->flags |= HIDE_MODEL;
		}
		// If the door has gone past the bottom
		else if (move < bottom)
		{
			// Set position to the bottom
			inst->matrix.t[1] = bottom;

			garage->direction = BOSS_GARAGE_DOOR_STOPPED;

			garage->cooldown = 0;

			// Enable door collision
			sdata->doorAccessFlags &= ~AH_GARAGE_ACCESS_FLAG;
		}
		// If the door is between the top and bottom positions
		else if (garage->garageTopInst != 0)
		{
			// Update rotation of garagetop
			garage->rot.x += (s16)garage->direction * AH_GARAGE_TOP_ROT_STEP;

			// converted to TEST in rebuildPS1
			ConvertRotToMatrix(&garage->garageTopInst->matrix, &garage->rot);
		}

		inst->flags &= ~SPLIT_SPECIAL;
		inst->flags |= (SPLIT_LINE | REFLECTION_FUNC23 | WATER_SPLIT_WHITE);
	}

LAB_800aeb6c:

	// If you're in Gemstone Valley
	if (levelID == GEM_STONE_VALLEY)
	{
		// ripper roo boss key
		bitIndex = ADV_REWARD_FIRST_BOSS_KEY;

		// check all boss keys
		for (i = 0; i < AH_BOSS_KEY_COUNT; i++)
		{
			if (!CHECK_ADV_BIT(adv->rewards, bitIndex))
			{
				goto LAB_800aebd0;
			}
			bitIndex++;
		}
	}
	// If you're not in Gemstone Valley
	else
	{
		check = &data.advHubTrackIDs[(levelID - N_SANITY_BEACH) * AH_HUB_TRACK_COUNT];
		// check all tracks on hub
		for (i = 0; i < AH_HUB_TRACK_COUNT; i++)
		{
			// if any trophy on this hub is not unlocked
			if (!CHECK_ADV_BIT(adv->rewards, check[i] + ADV_REWARD_FIRST_TROPHY))
			{
				// boss is not open
				goto LAB_800aebd0;
			}
		}
	}
	goto LAB_800aec34;

LAB_800aebd0:
	bossIsOpen = false;

LAB_800aec34:
	dist.x = drv_inst->matrix.t[0] - inst->instDef->pos.x;
	dist.y = drv_inst->matrix.t[1] - inst->instDef->pos.y;
	dist.z = drv_inst->matrix.t[2] - inst->instDef->pos.z;

	// if in a state where you're seeing the boss key open an adv door,
	// or some other kind of cutscene where you can't move
	if ((gGT->gameMode2 & 4) != 0)
	{
		return;
	}

	// check distance
	if (AH_GARAGE_CHALLENGE_NEAR_DIST_MAX_SQ < dist.x * dist.x + dist.y * dist.y + dist.z * dist.z)
	{
		goto LAB_800aede0;
	}

	RECT view = gGT->pushBuffer[0].rect;

	// if aku is not giving a hint
	if (sdata->AkuAkuHintState == 0)
	{
		// draw string, lng_challenge
		DecalFont_DrawLine(

		    sdata->lngStrings[data.lng_challenge[R232.bossCharacterIDs[hubID]]],

		    view.x + (view.w >> 1), ((view.y + view.h) - 0x1e), 1, 0xffff8000);
	}

	if (bossIsOpen)
	{
		goto LAB_800aede8;
	}

	hintMask = 0;

	// if this is gemstone valley
	if (levelID == GEM_STONE_VALLEY)
	{
		// if hint is not unlocked "need 4 keys for oxide"
		if (!CHECK_ADV_BIT(sdata->advProgress.rewards, ADV_REWARD_HINT_NEED_FOUR_KEYS_FOR_OXIDE))
		{
			// HintID: need four keys to race oxide
			hintMask = ADV_MASK_HINT_ID_NEED_FOUR_KEYS_FOR_OXIDE;
		}
	}
	// not gemstone valley
	else
	{
		//  if hint is not unlocked "to access this boss garage..."
		if (!CHECK_ADV_BIT(sdata->advProgress.rewards, ADV_REWARD_HINT_NEED_FOUR_TROPHIES_FOR_BOSS))
		{
			// HintID: need four trophies to enter boss
			hintMask = ADV_MASK_HINT_ID_NEED_FOUR_TROPHIES_FOR_BOSS;
		}
	}

	if (hintMask != 0)
	{
		MainFrame_RequestMaskHint(hintMask, 0);
	}

LAB_800aede0:

	if (!bossIsOpen)
	{
		return;
	}

LAB_800aede8:

	sps->Input1.pos = inst->instDef->pos;
	sps->Input1.hitRadius = AH_GARAGE_BOSS_COLLIDE_RADIUS;
	sps->Input1.hitRadiusSquared = AH_GARAGE_BOSS_COLLIDE_RADIUS_SQ;
	sps->Input1.modelID = STATIC_PINGARAGE;

	sps->Union.ThBuckColl.thread = t;
	sps->Union.ThBuckColl.funcCallback = AH_Garage_Open;

	// Open garage door when player gets within radius of door
	PROC_CollideHitboxWithBucket(gGT->threadBuckets[PLAYER].thread, sps, 0);

	ratio = MATH_Sin((int)inst->instDef->rot.y);

	pos.x = (int)inst->instDef->pos.x + (ratio * AH_GARAGE_INTERIOR_FORWARD_OFFSET >> 0xc);
	pos.y = (int)inst->instDef->pos.y;

	ratio = MATH_Cos((int)inst->instDef->rot.y);

	pos.z = (int)inst->instDef->pos.z + (ratio * AH_GARAGE_INTERIOR_FORWARD_OFFSET >> 0xc);

	// DriverPos - DoorPos
	dist.x = drv_inst->matrix.t[0] - pos.x;
	dist.y = drv_inst->matrix.t[1] - pos.y;
	dist.z = drv_inst->matrix.t[2] - pos.z;

	// If small distance (inside garage)
	if (dist.x * dist.x + dist.y * dist.y + dist.z * dist.z < AH_GARAGE_INTERIOR_DIST_SQ)
	{
		// Fade To Black
		gGT->pushBuffer_UI.fadeFromBlack_desiredResult = 0;
		gGT->pushBuffer_UI.fade_step = AH_GARAGE_FADE_STEP;
	}

	// If fade complete, start loading level
	if (gGT->pushBuffer_UI.fadeFromBlack_currentValue == 0)
	{
		sdata->Loading.OnBegin.RemBitsConfig0 |= ADVENTURE_ARENA;
		sdata->Loading.OnBegin.AddBitsConfig0 |= ADVENTURE_BOSS;

		if ((levelID == GEM_STONE_VALLEY) && (gGT->currAdvProfile.numRelics == ADV_OXIDE_FINAL_RELIC_COUNT))
		{
			// set string index (0-5) to "N Oxide's Final Challenge"
			gGT->bossID = AH_GARAGE_OXIDE_FINAL_BOSS_ID;
		}

		else
		{
			gGT->bossID = R232.bossCharacterIDs[hubID];
		}

		// new levelID
		levelID = R232.bossRaceLevelIDs[hubID];

		// Set the boss character (P2)
		data.characterIDs[1] = data.metaDataLEV[levelID].characterID_Boss;

		RaceFlag_SetDrawOrder(1);
		MainRaceTrack_RequestLoad(levelID);
	}
	return;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800af070-0x800af3a4.
void AH_Garage_LInB(struct Instance *inst)
{
	b32 bossIsOpen = true;
	s32 i;
	const s16 *check;
	u32 bitIndex;
	s32 ratio;
	struct Thread *t;
	struct Instance *garageTop;
	struct BossGarageDoor *garage;
	struct GameTracker *gGT = sdata->gGT;
	struct AdvProgress *adv = &sdata->advProgress;
	s32 levelID = gGT->levelID;

	if (inst->thread != NULL)
	{
		return;
	}

	t = PROC_BirthWithObject(SIZE_RELATIVE_POOL_BUCKET(sizeof(struct BossGarageDoor), NONE, SMALL, STATIC),

	                         AH_Garage_ThTick, // behavior
	                         R232.s_garage,    // debug name
	                         0                 // thread relative
	);

	if (t == NULL)
	{
		return;
	}

	inst->thread = t;
	t->inst = inst;
	t->funcThDestroy = AH_Garage_ThDestroy;

	garage = t->object;
	garage->direction = BOSS_GARAGE_DOOR_STOPPED;
	garage->cooldown = 0;

	// if it is Oxide's Door
	if (inst->model->id == STATIC_OXIDEGARAGE)
	{
		garage->garageTopInst = NULL;
	}

	// if this is not oxide's door
	else
	{
		// make a "garagetop" to make door appear to roll up

		garageTop = INSTANCE_Birth3D(gGT->modelPtr[STATIC_GARAGETOP], R232.s_garagetop, t);

		// copy matrix from one instance to the other
		CTR_MatrixCopyRot(&garageTop->matrix, &inst->matrix);
		garageTop->matrix.t[0] = inst->matrix.t[0];
		garageTop->matrix.t[1] = inst->matrix.t[1];
		garageTop->matrix.t[2] = inst->matrix.t[2];

		ratio = MATH_Sin((int)inst->instDef->rot.y);

		// continue setting GarageTop position
		garageTop->matrix.t[0] = inst->matrix.t[0] + (ratio * AH_GARAGE_TOP_FORWARD_OFFSET >> 0xc);
		garageTop->matrix.t[1] = inst->matrix.t[1] + AH_GARAGE_DOOR_HEIGHT;

		ratio = MATH_Cos((int)inst->instDef->rot.y);

		// continue setting GarageTop position
		garageTop->matrix.t[2] = inst->matrix.t[2] + (ratio * AH_GARAGE_TOP_FORWARD_OFFSET >> 0xc);

		garageTop->depthBiasNormal = AH_GARAGE_TOP_DEPTH_BIAS;

		garage->garageTopInst = garageTop;
	}

	if (levelID == GEM_STONE_VALLEY)
	{
		// ripper roo boss key
		bitIndex = ADV_REWARD_FIRST_BOSS_KEY;
		// check all boss keys
		for (i = 0; i < AH_BOSS_KEY_COUNT; i++)
		{
			if (!CHECK_ADV_BIT(adv->rewards, bitIndex))
			{
				goto GarageLocked;
			}
			bitIndex++;
		}
		bossIsOpen = true;
	}

	// if not gemstone valley
	else
	{
		check = &data.advHubTrackIDs[(levelID - N_SANITY_BEACH) * AH_HUB_TRACK_COUNT];
		// check all tracks on hub
		for (i = 0; i < AH_HUB_TRACK_COUNT; i++)
		{
			// if any trophy on this hub is not unlocked
			if (!CHECK_ADV_BIT(adv->rewards, check[i] + ADV_REWARD_FIRST_TROPHY))
			{
				// boss is not open
				goto GarageLocked;
			}
		}
	}

	// if boss is open
	if (bossIsOpen)
	{
		// check if boss was NOT beaten on this hub (levelID - 0x19)
		bitIndex = data.BeatBossPrize[levelID - GEM_STONE_VALLEY];
		t->modelIndex = CHECK_ADV_BIT(adv->rewards, bitIndex) ? 2 : 1;
	}

	// if boss is not open
	else
	{
	GarageLocked:
		// locked
		t->modelIndex = 0;
	}

	garage->rot.x = inst->instDef->rot.x;
	garage->rot.y = inst->instDef->rot.y;
	garage->rot.z = inst->instDef->rot.z;

	inst->depthBiasNormal = 1;
	inst->depthBiasSecondary = inst->depthBiasNormal;
	inst->specLightX = 0;
	inst->vertSplit = inst->instDef->pos.y + AH_GARAGE_DOOR_HEIGHT;
}
