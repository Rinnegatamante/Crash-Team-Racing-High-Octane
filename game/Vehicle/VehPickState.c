#include <common.h>

enum
{
	VEH_PICK_DAMAGE_NONE = 0,
	VEH_PICK_DAMAGE_SPIN = 1,
	VEH_PICK_DAMAGE_BLAST = 2,
	VEH_PICK_DAMAGE_SQUISH = 3,
	VEH_PICK_DAMAGE_BURN = 4,
	VEH_PICK_DAMAGE_MASK_GRAB = 5,

	VEH_PICK_REASON_BOMB = 1,
	VEH_PICK_REASON_MOTIONLESS_POTION = 2,
	VEH_PICK_REASON_MISSILE = 3,
	VEH_PICK_REASON_MOVING_POTION = 4,
	VEH_PICK_REASON_TURBO_SQUISH = 5,
	VEH_PICK_REASON_MASK_WEAPON = 6,

	VEH_PICK_VOICELINE_VICTIM_LAUGH = 2,
	VEH_PICK_VOICELINE_COMMON_DAMAGE = 1,
	VEH_PICK_VOICELINE_SQUISH = 4,
	VEH_PICK_VOICELINE_PRIORITY = 0x10,

	VEH_PICK_SHIELD_DAMAGE_INVINCIBLE_TIMER = 0x2a0,
	VEH_PICK_SPIN_NO_INPUT_TIMER = 0x3c0,
	VEH_PICK_BLAST_NO_INPUT_TIMER = 0x960,
	VEH_PICK_SQUISH_NO_INPUT_TIMER = 0xf0,
	VEH_PICK_SQUISH_TIMER = 0xf00,
	VEH_PICK_BURN_NO_INPUT_TIMER = 0x780,
	VEH_PICK_BURN_TIMER = 0xf00,
	VEH_PICK_MASK_GRAB_NO_INPUT_TIMER = 0xd20,

	VEH_PICK_SOUND_SQUISH = 0x5a,
	VEH_PICK_SOUND_BURN = 0x69,

	VEH_PICK_ATTACK_QUIP_BOMB = 0x1,
	VEH_PICK_ATTACK_QUIP_MISSILE = 0x2,
	VEH_PICK_ATTACK_QUIP_MOVING_POTION = 0x4,
	VEH_PICK_ATTACK_QUIP_MASK = 0x8,

	VEH_PICK_RUMBLE_FRAMES = 8,
	VEH_PICK_RUMBLE_FORCE = 0x7f,
	VEH_PICK_BATTLE_HUD_OFFSET_Y = 0x14,
	VEH_PICK_BATTLE_HUD_COOLDOWN = 5,
};

CTR_STATIC_ASSERT(VEH_PICK_DAMAGE_NONE == 0);
CTR_STATIC_ASSERT(VEH_PICK_DAMAGE_SPIN == 1);
CTR_STATIC_ASSERT(VEH_PICK_DAMAGE_BLAST == 2);
CTR_STATIC_ASSERT(VEH_PICK_DAMAGE_SQUISH == 3);
CTR_STATIC_ASSERT(VEH_PICK_DAMAGE_BURN == 4);
CTR_STATIC_ASSERT(VEH_PICK_DAMAGE_MASK_GRAB == 5);
CTR_STATIC_ASSERT(VEH_PICK_REASON_BOMB == 1);
CTR_STATIC_ASSERT(VEH_PICK_REASON_MOTIONLESS_POTION == 2);
CTR_STATIC_ASSERT(VEH_PICK_REASON_MISSILE == 3);
CTR_STATIC_ASSERT(VEH_PICK_REASON_MOVING_POTION == 4);
CTR_STATIC_ASSERT(VEH_PICK_REASON_TURBO_SQUISH == 5);
CTR_STATIC_ASSERT(VEH_PICK_REASON_MASK_WEAPON == 6);
CTR_STATIC_ASSERT(VEH_PICK_VOICELINE_VICTIM_LAUGH == 2);
CTR_STATIC_ASSERT(VEH_PICK_VOICELINE_COMMON_DAMAGE == 1);
CTR_STATIC_ASSERT(VEH_PICK_VOICELINE_SQUISH == 4);
CTR_STATIC_ASSERT(VEH_PICK_VOICELINE_PRIORITY == 0x10);
CTR_STATIC_ASSERT(VEH_PICK_SHIELD_DAMAGE_INVINCIBLE_TIMER == 0x2a0);
CTR_STATIC_ASSERT(VEH_PICK_SPIN_NO_INPUT_TIMER == 0x3c0);
CTR_STATIC_ASSERT(VEH_PICK_BLAST_NO_INPUT_TIMER == 0x960);
CTR_STATIC_ASSERT(VEH_PICK_SQUISH_NO_INPUT_TIMER == 0xf0);
CTR_STATIC_ASSERT(VEH_PICK_SQUISH_TIMER == 0xf00);
CTR_STATIC_ASSERT(VEH_PICK_BURN_NO_INPUT_TIMER == 0x780);
CTR_STATIC_ASSERT(VEH_PICK_BURN_TIMER == 0xf00);
CTR_STATIC_ASSERT(VEH_PICK_MASK_GRAB_NO_INPUT_TIMER == 0xd20);
CTR_STATIC_ASSERT(VEH_PICK_SOUND_SQUISH == 0x5a);
CTR_STATIC_ASSERT(VEH_PICK_SOUND_BURN == 0x69);
CTR_STATIC_ASSERT(VEH_PICK_ATTACK_QUIP_BOMB == 0x1);
CTR_STATIC_ASSERT(VEH_PICK_ATTACK_QUIP_MISSILE == 0x2);
CTR_STATIC_ASSERT(VEH_PICK_ATTACK_QUIP_MOVING_POTION == 0x4);
CTR_STATIC_ASSERT(VEH_PICK_ATTACK_QUIP_MASK == 0x8);
CTR_STATIC_ASSERT(VEH_PICK_RUMBLE_FRAMES == 8);
CTR_STATIC_ASSERT(VEH_PICK_RUMBLE_FORCE == 0x7f);
CTR_STATIC_ASSERT(VEH_PICK_BATTLE_HUD_OFFSET_Y == 0x14);
CTR_STATIC_ASSERT(VEH_PICK_BATTLE_HUD_COOLDOWN == 5);

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80064568-0x80064be4.
int VehPickState_NewState(struct Driver *victimDriver, int damageType, struct Driver *attackDriver, int reason)
{
	int voice;

	int victimState = victimDriver->kartState;
	victimDriver->pendingDamageType = 0;

	int victimCharacter = data.characterIDs[victimDriver->driverID];

	if (victimState == KS_MASK_GRABBED)
	{
		return 0;
	}

	if (
	    // If player is using mask weapon
	    ((victimDriver->actionsFlagSet & ACTION_MASK_WEAPON) != 0) ||

	    (victimDriver->invincibleTimer != 0))
	{
	VictimLaugh:
		Voiceline_RequestPlay(VEH_PICK_VOICELINE_VICTIM_LAUGH, victimCharacter, VEH_PICK_VOICELINE_PRIORITY);
		return 0;
	}

	if (victimDriver->instBubbleHold != NULL)
	{
		struct Shield *shieldObj = victimDriver->instBubbleHold->thread->object;

		shieldObj->flags |= SHIELD_FLAG_POP_ON_DAMAGE;

		victimDriver->invincibleTimer = VEH_PICK_SHIELD_DAMAGE_INVINCIBLE_TIMER;

		victimDriver->instBubbleHold = NULL;

		goto VictimLaugh;
	}

	voice = 0;

	if (damageType == VEH_PICK_DAMAGE_NONE)
	{
		return 1;
	}

	// spinning
	else if (damageType == VEH_PICK_DAMAGE_SPIN)
	{
		// 1.0s
		victimDriver->NoInputTimer = VEH_PICK_SPIN_NO_INPUT_TIMER;

		if (victimState != KS_SPINNING)
		{
		SPINOUT:
			victimDriver->funcPtrs[DRIVER_FUNC_INIT] = VehPhysProc_SpinFirst_Init;
		}
	}

	// blasted
	else if (damageType == VEH_PICK_DAMAGE_BLAST)
	{
		// quit if already blasted
		if (victimState == KS_BLASTED)
		{
			return 0;
		}

		// quit if already blasted
		if (victimDriver->funcPtrs[DRIVER_FUNC_INIT] == VehStuckProc_Tumble_Init)
		{
			return 0;
		}

		victimDriver->funcPtrs[DRIVER_FUNC_INIT] = VehStuckProc_Tumble_Init;

		// 2.4s
		victimDriver->NoInputTimer = VEH_PICK_BLAST_NO_INPUT_TIMER;

		victimDriver->squishTimer = 0;

		voice = VEH_PICK_VOICELINE_COMMON_DAMAGE;
	}

	// squished
	else if (damageType == VEH_PICK_DAMAGE_SQUISH)
	{
		if (victimState != KS_SPINNING)
		{
			// squish sound
			OtherFX_Play_Echo(VEH_PICK_SOUND_SQUISH, 1, victimDriver->actionsFlagSet & ACTION_ENGINE_ECHO);

			voice = VEH_PICK_VOICELINE_SQUISH;
		}

		// 0.25s
		victimDriver->NoInputTimer = VEH_PICK_SQUISH_NO_INPUT_TIMER;

		victimDriver->squishTimer = VEH_PICK_SQUISH_TIMER;

		goto SPINOUT;
	}

	// burned
	else if (damageType == VEH_PICK_DAMAGE_BURN)
	{
		if (victimDriver->burnTimer == 0)
		{
			OtherFX_Play(VEH_PICK_SOUND_BURN, 1);

			voice = VEH_PICK_VOICELINE_COMMON_DAMAGE;
		}

		// 2.0s
		victimDriver->NoInputTimer = VEH_PICK_BURN_NO_INPUT_TIMER;

		victimDriver->burnTimer = VEH_PICK_BURN_TIMER;

		goto SPINOUT;
	}

	// mask grab
	else if (damageType == VEH_PICK_DAMAGE_MASK_GRAB)
	{
		// 3.36s
		victimDriver->NoInputTimer = VEH_PICK_MASK_GRAB_NO_INPUT_TIMER;

		victimDriver->funcPtrs[DRIVER_FUNC_INIT] = VehStuckProc_PlantEaten_Init;

		voice = VEH_PICK_VOICELINE_COMMON_DAMAGE;
	}
	else
	{
		// Retail sends any other nonzero damage type through spinout.
		victimDriver->NoInputTimer = VEH_PICK_SPIN_NO_INPUT_TIMER;

		if (victimState != KS_SPINNING)
		{
			goto SPINOUT;
		}
	}

	if (voice != 0)
	{
		Voiceline_RequestPlay(voice, victimCharacter, VEH_PICK_VOICELINE_PRIORITY);
	}

	switch (reason)
	{
	// hit by bomb
	case VEH_PICK_REASON_BOMB:
		victimDriver->numTimesBombHitYou++;
		break;

	// hit by motionless potion
	case VEH_PICK_REASON_MOTIONLESS_POTION:
		victimDriver->numTimesMotionlessPotionHitYou++;
		break;

	// hit by missile
	case VEH_PICK_REASON_MISSILE:
		victimDriver->numTimesMissileHitYou++;
		break;

	default:
		break;
	}

	if (attackDriver != NULL)
	{
		if (attackDriver != victimDriver)
		{
			switch (reason)
			{
			// hit by bomb
			case VEH_PICK_REASON_BOMB:
				attackDriver->numTimesBombsHitSomeone++;
				attackDriver->quip4 |= VEH_PICK_ATTACK_QUIP_BOMB;
				break;

			// hit by missile
			case VEH_PICK_REASON_MISSILE:
				attackDriver->numTimesMissileHitSomeone++;
				attackDriver->quip4 |= VEH_PICK_ATTACK_QUIP_MISSILE;
				break;

			// hit by moving potion
			case VEH_PICK_REASON_MOVING_POTION:
				attackDriver->numTimesMovingPotionHitSomeone++;
				attackDriver->quip4 |= VEH_PICK_ATTACK_QUIP_MOVING_POTION;
				break;

			// squished by turbo
			case VEH_PICK_REASON_TURBO_SQUISH:
				attackDriver->numTimesSquishedSomeone++;
				break;

			// hit by mask weapon
			case VEH_PICK_REASON_MASK_WEAPON:
				attackDriver->quip4 |= VEH_PICK_ATTACK_QUIP_MASK;
				break;

			default:
				break;
			}
		}
	}

	victimDriver->kartState = KS_NORMAL;
	victimDriver->reserves = 0;
	victimDriver->turbo_outsideTimer = 0;
	victimDriver->matrixArray = BAKED_GTE_MATRIX_NONE;
	victimDriver->matrixIndex = 0;

	GAMEPAD_ShockFreq(victimDriver, VEH_PICK_RUMBLE_FRAMES, 0);
	GAMEPAD_ShockForce1(victimDriver, VEH_PICK_RUMBLE_FRAMES, VEH_PICK_RUMBLE_FORCE);

	int gameMode1 = sdata->gGT->gameMode1;

	if ((attackDriver != NULL) && ((gameMode1 & END_OF_RACE) == 0))
	{
		struct PushBuffer *pb = &sdata->gGT->pushBuffer[attackDriver->driverID];

		SVec2 posScreen;
		RB_Fruit_GetScreenCoords(pb, attackDriver->instSelf, posScreen.v);

		// screenPosXY
		attackDriver->BattleHUD.startX = pb->rect.x + posScreen.x;
		attackDriver->BattleHUD.startY = pb->rect.y + posScreen.y - VEH_PICK_BATTLE_HUD_OFFSET_Y;

		// if-checked for Battle inside the function
		RB_Player_KillPlayer(attackDriver, victimDriver);

		// NOTE(aalhendi): Retail rechecks END_OF_RACE after RB_Player_KillPlayer,
		// which can transition battle finish state inside this block.
		if ((sdata->gGT->gameMode1 & END_OF_RACE) != 0)
		{
			attackDriver->quip1 = (s16)reason;
			victimDriver->quip3 = (s16)reason;
		}

		if ((attackDriver == victimDriver) && ((sdata->gGT->gameMode1 & POINT_LIMIT) != 0))
		{
			if (victimDriver->BattleHUD.cooldown == VEH_PICK_BATTLE_HUD_COOLDOWN)
			{
				victimDriver->BattleHUD.scoreDelta--;
			}
			else
			{
				victimDriver->BattleHUD.scoreDelta = -1;
			}
		}
		else
		{
			if (attackDriver->BattleHUD.cooldown == VEH_PICK_BATTLE_HUD_COOLDOWN)
			{
				attackDriver->BattleHUD.scoreDelta++;
			}
			else
			{
				attackDriver->BattleHUD.scoreDelta = 1;
			}
		}

		attackDriver->BattleHUD.cooldown = VEH_PICK_BATTLE_HUD_COOLDOWN;
		victimDriver->numTimesAttackedByPlayer[attackDriver->driverID]++;
		attackDriver->numTimesAttackingPlayer[victimDriver->driverID]++;

		if (attackDriver != victimDriver)
		{
			attackDriver->numTimesAttacking++;
		}
	}

	victimDriver->instSelf->thread->flags &= ~THREAD_FLAG_DISABLE_COLLISION;
	victimDriver->instSelf->flags &= ~HIDE_MODEL;

	return 1;
}
