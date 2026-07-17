#include <common.h>

enum
{
	VEH_PHYS_CRASH_FAST_SQRT_ITERATIONS = 0x10,
	VEH_PHYS_CRASH_VECTOR_SPEED_SHIFT = 8,
	VEH_PHYS_CRASH_MATRIX_FRAC_SHIFT = 0xc,
	VEH_PHYS_CRASH_UNIT_VECTOR_SCALE = 0x1000,
	VEH_PHYS_CRASH_BOUNCE_Y_CLAMP = 0x3200,
	VEH_PHYS_CRASH_BOT_NAV_ROT_SHIFT = 4,
	VEH_PHYS_CRASH_BOT_SPEED_SHIFT = 8,

	VEH_PHYS_CRASH_DAMAGE_TYPE_MASK = 2,
	VEH_PHYS_CRASH_DAMAGE_TYPE_TURBO = 3,
	VEH_PHYS_CRASH_DAMAGE_REASON_SHIELD = 0,
	VEH_PHYS_CRASH_DAMAGE_REASON_TURBO = 5,
	VEH_PHYS_CRASH_DAMAGE_REASON_MASK = 6,
	VEH_PHYS_CRASH_TURBO_DAMAGE_IMPACT = 0xa00,

	VEH_PHYS_CRASH_BUBBLE_POP_FX = 0x4f,
	VEH_PHYS_CRASH_VOICELINE_CRASH = 1,
	VEH_PHYS_CRASH_VOICELINE_HARD_CRASH = 5,
	VEH_PHYS_CRASH_VOICELINE_PRIORITY = 0x10,

	VEH_PHYS_CRASH_FEEDBACK_MIN_IMPACT = 0x200,
	VEH_PHYS_CRASH_VOLUME_IMPACT_MAX = 0x1900,
	VEH_PHYS_CRASH_VOLUME_MIN = 0x3f,
	VEH_PHYS_CRASH_VOLUME_MAX = 0xff,
	VEH_PHYS_CRASH_HARD_CRASH_VOLUME = 0xdc,
	VEH_PHYS_CRASH_FEEDBACK_COOLDOWN_FRAMES = 3,

	VEH_PHYS_CRASH_RUMBLE_CHANNEL = 8,
	VEH_PHYS_CRASH_RUMBLE_FORCE = 0x7f,
	VEH_PHYS_CRASH_JOG_TURNING = 0x29,
	VEH_PHYS_CRASH_JOG_STRAIGHT = 0x19,
	VEH_PHYS_CRASH_JOG_DURATION = 0x60,
};

CTR_STATIC_ASSERT(VEH_PHYS_CRASH_FAST_SQRT_ITERATIONS == 0x10);
CTR_STATIC_ASSERT(VEH_PHYS_CRASH_VECTOR_SPEED_SHIFT == 8);
CTR_STATIC_ASSERT(VEH_PHYS_CRASH_MATRIX_FRAC_SHIFT == 0xc);
CTR_STATIC_ASSERT(VEH_PHYS_CRASH_UNIT_VECTOR_SCALE == 0x1000);
CTR_STATIC_ASSERT(VEH_PHYS_CRASH_BOUNCE_Y_CLAMP == 0x3200);
CTR_STATIC_ASSERT(VEH_PHYS_CRASH_BOT_NAV_ROT_SHIFT == 4);
CTR_STATIC_ASSERT(VEH_PHYS_CRASH_BOT_SPEED_SHIFT == 8);
CTR_STATIC_ASSERT(VEH_PHYS_CRASH_DAMAGE_TYPE_MASK == 2);
CTR_STATIC_ASSERT(VEH_PHYS_CRASH_DAMAGE_TYPE_TURBO == 3);
CTR_STATIC_ASSERT(VEH_PHYS_CRASH_DAMAGE_REASON_SHIELD == 0);
CTR_STATIC_ASSERT(VEH_PHYS_CRASH_DAMAGE_REASON_TURBO == 5);
CTR_STATIC_ASSERT(VEH_PHYS_CRASH_DAMAGE_REASON_MASK == 6);
CTR_STATIC_ASSERT(VEH_PHYS_CRASH_TURBO_DAMAGE_IMPACT == 0xa00);
CTR_STATIC_ASSERT(VEH_PHYS_CRASH_BUBBLE_POP_FX == 0x4f);
CTR_STATIC_ASSERT(VEH_PHYS_CRASH_VOICELINE_CRASH == 1);
CTR_STATIC_ASSERT(VEH_PHYS_CRASH_VOICELINE_HARD_CRASH == 5);
CTR_STATIC_ASSERT(VEH_PHYS_CRASH_VOICELINE_PRIORITY == 0x10);
CTR_STATIC_ASSERT(VEH_PHYS_CRASH_FEEDBACK_MIN_IMPACT == 0x200);
CTR_STATIC_ASSERT(VEH_PHYS_CRASH_VOLUME_IMPACT_MAX == 0x1900);
CTR_STATIC_ASSERT(VEH_PHYS_CRASH_VOLUME_MIN == 0x3f);
CTR_STATIC_ASSERT(VEH_PHYS_CRASH_VOLUME_MAX == 0xff);
CTR_STATIC_ASSERT(VEH_PHYS_CRASH_HARD_CRASH_VOLUME == 0xdc);
CTR_STATIC_ASSERT(VEH_PHYS_CRASH_FEEDBACK_COOLDOWN_FRAMES == 3);
CTR_STATIC_ASSERT(VEH_PHYS_CRASH_RUMBLE_CHANNEL == 8);
CTR_STATIC_ASSERT(VEH_PHYS_CRASH_RUMBLE_FORCE == 0x7f);
CTR_STATIC_ASSERT(VEH_PHYS_CRASH_JOG_TURNING == 0x29);
CTR_STATIC_ASSERT(VEH_PHYS_CRASH_JOG_STRAIGHT == 0x19);
CTR_STATIC_ASSERT(VEH_PHYS_CRASH_JOG_DURATION == 0x60);

static u32 VehPhysCrash_LengthSq2(s32 x, s32 z)
{
	return (u32)CTR_MipsAddLo(CTR_MipsMulLo(x, x), CTR_MipsMulLo(z, z));
}

static u32 VehPhysCrash_LengthSq3(s32 x, s32 y, s32 z)
{
	return (u32)CTR_MipsAddLo(CTR_MipsAddLo(CTR_MipsMulLo(x, x), CTR_MipsMulLo(y, y)), CTR_MipsMulLo(z, z));
}

static s32 VehPhysCrash_Dot3(s32 ax, s32 ay, s32 az, s32 bx, s32 by, s32 bz)
{
	return CTR_MipsAddLo(CTR_MipsAddLo(CTR_MipsMulLo(ax, bx), CTR_MipsMulLo(ay, by)), CTR_MipsMulLo(az, bz));
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005cd1c-0x8005cf64.
void VehPhysCrash_ConvertVecToSpeed(struct Driver *d, Vec3 *vel)
{
	int speed2D = VehCalc_FastSqrt(VehPhysCrash_LengthSq2(vel->x, vel->z), VEH_PHYS_CRASH_FAST_SQRT_ITERATIONS);
	s16 speed3D =
	    (s16)(VehCalc_FastSqrt(VehPhysCrash_LengthSq3(vel->x, vel->y, vel->z), VEH_PHYS_CRASH_FAST_SQRT_ITERATIONS) >> VEH_PHYS_CRASH_VECTOR_SPEED_SHIFT);

	d->speed = speed3D;
	d->axisRotationY = (s16)ratan2(CTR_MipsSll(vel->y, VEH_PHYS_CRASH_VECTOR_SPEED_SHIFT), speed2D);
	d->axisRotationX = (s16)ratan2(vel->x, vel->z);

	int projOnMovingDirAxis =
	    CTR_MipsSra(VehPhysCrash_Dot3(vel->x, vel->y, vel->z, d->matrixMovingDir.m[0][1], d->matrixMovingDir.m[1][1], d->matrixMovingDir.m[2][1]),
	                VEH_PHYS_CRASH_MATRIX_FRAC_SHIFT);

	int projX = CTR_MipsSra(CTR_MipsMulLo(d->matrixMovingDir.m[0][1], projOnMovingDirAxis), VEH_PHYS_CRASH_MATRIX_FRAC_SHIFT);
	int projY = CTR_MipsSra(CTR_MipsMulLo(d->matrixMovingDir.m[1][1], projOnMovingDirAxis), VEH_PHYS_CRASH_MATRIX_FRAC_SHIFT);
	int projZ = CTR_MipsSra(CTR_MipsMulLo(d->matrixMovingDir.m[2][1], projOnMovingDirAxis), VEH_PHYS_CRASH_MATRIX_FRAC_SHIFT);

	speed3D = (s16)(VehCalc_FastSqrt(VehPhysCrash_LengthSq3(projX, projY, projZ), VEH_PHYS_CRASH_FAST_SQRT_ITERATIONS) >> VEH_PHYS_CRASH_VECTOR_SPEED_SHIFT);

	d->jumpHeightCurr = speed3D;
	if (projOnMovingDirAxis < 0)
	{
		d->jumpHeightCurr = (s16)CTR_MipsNegLo(speed3D);
	}

	projX = CTR_MipsSubLo(vel->x, projX);
	projY = CTR_MipsSubLo(vel->y, projY);
	projZ = CTR_MipsSubLo(vel->z, projZ);

	speed3D = (s16)(VehCalc_FastSqrt(VehPhysCrash_LengthSq3(projX, projY, projZ), VEH_PHYS_CRASH_FAST_SQRT_ITERATIONS) >> VEH_PHYS_CRASH_VECTOR_SPEED_SHIFT);

	d->speedApprox = speed3D;

	if (VehPhysCrash_Dot3(projX, projY, projZ, d->matrixMovingDir.m[0][2], d->matrixMovingDir.m[1][2], d->matrixMovingDir.m[2][2]) < 0)
	{
		d->speedApprox = (s16)CTR_MipsNegLo(speed3D);
	}
}

static int VehPhysCrash_BounceSelf_Div6Shift9(int value)
{
	s64 product = (s64)value * 0x2aaaaaab;
	int high = (s32)((u64)product >> 32);

	return CTR_MipsSubLo(CTR_MipsSra(high, 9), CTR_MipsSra(value, 31));
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005cf64-0x8005d0d0.
int VehPhysCrash_BounceSelf(const SVec3 *normal, const Vec3 *origin, Vec3 *vel, b32 boolOtherDriver)
{
	int diffX = CTR_MipsSubLo(vel->x, origin->x);
	int diffY = CTR_MipsSubLo(vel->y, origin->y);
	int diffZ = CTR_MipsSubLo(vel->z, origin->z);
	int dot = CTR_MipsSra(CTR_MipsAddLo(CTR_MipsAddLo(CTR_MipsMulLo(diffX, normal->x), CTR_MipsMulLo(diffY, normal->y)), CTR_MipsMulLo(diffZ, normal->z)),
	                      VEH_PHYS_CRASH_MATRIX_FRAC_SHIFT);

	if (boolOtherDriver == 0)
	{
		if (dot >= 0)
		{
			return 0;
		}
	}
	else if (dot <= 0)
	{
		return 0;
	}

	int absDot = dot;
	if (absDot < 0)
	{
		absDot = CTR_MipsNegLo(absDot);
	}

	if (sdata->vehicleCollisionImpactStrength < absDot)
	{
		sdata->vehicleCollisionImpactStrength = absDot;
	}

	diffX = CTR_MipsSubLo(diffX, VehPhysCrash_BounceSelf_Div6Shift9(CTR_MipsMulLo(dot, normal->x)));
	diffY = CTR_MipsSubLo(diffY, VehPhysCrash_BounceSelf_Div6Shift9(CTR_MipsMulLo(dot, normal->y)));
	diffZ = CTR_MipsSubLo(diffZ, VehPhysCrash_BounceSelf_Div6Shift9(CTR_MipsMulLo(dot, normal->z)));

	vel->x = CTR_MipsAddLo(diffX, origin->x);

	int oldY = vel->y;
	int newY = CTR_MipsAddLo(diffY, origin->y);
	if ((oldY < newY) && (newY > VEH_PHYS_CRASH_BOUNCE_Y_CLAMP))
	{
		newY = VEH_PHYS_CRASH_BOUNCE_Y_CLAMP;
	}
	vel->y = newY;
	vel->z = CTR_MipsAddLo(diffZ, origin->z);

	return 0;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005d0d0-0x8005d218.
void VehPhysCrash_AI(struct Driver *bot, Vec3 *vel)
{
	sdata->botCrashNavRot.x = (s16)CTR_MipsSll(bot->botData.botNavFrame->rot[0], VEH_PHYS_CRASH_BOT_NAV_ROT_SHIFT);
	sdata->botCrashNavRot.y = (s16)CTR_MipsSll(bot->botData.botNavFrame->rot[1], VEH_PHYS_CRASH_BOT_NAV_ROT_SHIFT);
	sdata->botCrashNavRot.z = (s16)CTR_MipsSll(bot->botData.botNavFrame->rot[2], VEH_PHYS_CRASH_BOT_NAV_ROT_SHIFT);

	struct VehPhysCrashAiScratch *scratch = (struct VehPhysCrashAiScratch *)(void *)&sdata->dataLibFiller[0];
	MATRIX *matrix = &scratch->matrix;

	ConvertRotToMatrix(matrix, &sdata->botCrashNavRot);

	scratch->forward.x = CTR_MipsSra(matrix->m[0][2], VEH_PHYS_CRASH_BOT_NAV_ROT_SHIFT);
	scratch->forward.y = CTR_MipsSra(matrix->m[1][2], VEH_PHYS_CRASH_BOT_NAV_ROT_SHIFT);
	scratch->forward.z = CTR_MipsSra(matrix->m[2][2], VEH_PHYS_CRASH_BOT_NAV_ROT_SHIFT);

	int botSpeed = CTR_MipsSra(CTR_MipsAddLo(CTR_MipsAddLo(CTR_MipsMulLo(scratch->forward.x, vel->x), CTR_MipsMulLo(scratch->forward.y, vel->y)),
	                                         CTR_MipsMulLo(scratch->forward.z, vel->z)),
	                           VEH_PHYS_CRASH_BOT_SPEED_SHIFT);

	bot->botData.aiPhysics.speedLinear = botSpeed;
	bot->botData.aiPhysics.accel.x = CTR_MipsSubLo(vel->x, CTR_MipsSra(CTR_MipsMulLo(scratch->forward.x, botSpeed), VEH_PHYS_CRASH_BOT_SPEED_SHIFT));
	bot->botData.botFlags |= BOT_FLAG_FREE_PHYSICS;
	bot->botData.aiPhysics.accel.z = CTR_MipsSubLo(vel->z, CTR_MipsSra(CTR_MipsMulLo(scratch->forward.z, botSpeed), VEH_PHYS_CRASH_BOT_SPEED_SHIFT));
}

static void VehPhysCrash_Attack_SetReason(struct Driver *driver, u8 reason)
{
	driver->pendingDamageReasonByte = reason;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005d218-0x8005d404.
int VehPhysCrash_Attack(struct Driver *driver1, struct Driver *driver2, b32 canPlayFeedback, b32 boolPlayBubblePop)
{
	if ((driver1->actionsFlagSet & ACTION_MASK_WEAPON) == 0)
	{
		if ((driver2->actionsFlagSet & ACTION_MASK_WEAPON) != 0)
		{
			driver1->pendingDamageType = VEH_PHYS_CRASH_DAMAGE_TYPE_MASK;
			VehPhysCrash_Attack_SetReason(driver1, VEH_PHYS_CRASH_DAMAGE_REASON_MASK);
			driver1->pendingDamageAttacker = driver2;

			if ((canPlayFeedback != 0) && (driver1->kartState != KS_BLASTED) && (driver1->invincibleTimer == 0))
			{
				OtherFX_DriverCrashing((driver1->actionsFlagSet & ACTION_ENGINE_ECHO) != 0, VEH_PHYS_CRASH_VOLUME_MAX);
				Voiceline_RequestPlay(VEH_PHYS_CRASH_VOICELINE_CRASH, data.characterIDs[driver1->driverID], VEH_PHYS_CRASH_VOICELINE_PRIORITY);
			}
		}

		if ((driver2->instBubbleHold != NULL) && (driver1->instBubbleHold == NULL))
		{
			struct Shield *bubble = driver2->instBubbleHold->thread->object;

			bubble->flags |= SHIELD_FLAG_CRASH_ATTACK;
			driver2->instBubbleHold = NULL;

			driver1->pendingDamageType = VEH_PHYS_CRASH_DAMAGE_TYPE_MASK;
			VehPhysCrash_Attack_SetReason(driver1, VEH_PHYS_CRASH_DAMAGE_REASON_SHIELD);
			driver1->pendingDamageAttacker = driver2;

			if ((canPlayFeedback != 0) && (driver1->kartState != KS_BLASTED) && (driver1->invincibleTimer == 0))
			{
				OtherFX_DriverCrashing((driver1->actionsFlagSet & ACTION_ENGINE_ECHO) != 0, VEH_PHYS_CRASH_VOLUME_MAX);

				if (boolPlayBubblePop != 0)
				{
					OtherFX_Play(VEH_PHYS_CRASH_BUBBLE_POP_FX, 1);
				}

				Voiceline_RequestPlay(VEH_PHYS_CRASH_VOICELINE_CRASH, data.characterIDs[driver1->driverID], VEH_PHYS_CRASH_VOICELINE_PRIORITY);
			}
		}

		if ((sdata->vehicleCollisionImpactStrength > VEH_PHYS_CRASH_TURBO_DAMAGE_IMPACT) && (driver2->reserves != 0) &&
		    ((driver2->actionsFlagSet & ACTION_TURBO_ITEM) != 0) && (driver1->reserves == 0))
		{
			driver2->forcedJumpType = FORCED_JUMP_HIGH;

			driver1->pendingDamageType = VEH_PHYS_CRASH_DAMAGE_TYPE_TURBO;
			VehPhysCrash_Attack_SetReason(driver1, VEH_PHYS_CRASH_DAMAGE_REASON_TURBO);
			driver1->pendingDamageAttacker = driver2;
		}
	}

	return canPlayFeedback;
}

// NOTE(aalhendi): These static helpers factor repeated retail blocks; they
// are not separate retail symbols.
static s32 VehPhysCrash_WeightedAverage(s32 lhs, s16 lhsWeight, s32 rhs, s16 rhsWeight)
{
	return CTR_MipsDiv(CTR_MipsAddLo(CTR_MipsMulLo(lhs, lhsWeight), CTR_MipsMulLo(rhs, rhsWeight)), CTR_MipsAddLo(lhsWeight, rhsWeight));
}

static void VehPhysCrash_WeightedVelocity(Vec3 *out, Vec3 *lhs, struct Driver *lhsDriver, Vec3 *rhs, struct Driver *rhsDriver)
{
	out->x = VehPhysCrash_WeightedAverage(lhs->x, lhsDriver->const_CollisionWeight, rhs->x, rhsDriver->const_CollisionWeight);
	out->y = VehPhysCrash_WeightedAverage(lhs->y, lhsDriver->const_CollisionWeight, rhs->y, rhsDriver->const_CollisionWeight);
	out->z = VehPhysCrash_WeightedAverage(lhs->z, lhsDriver->const_CollisionWeight, rhs->z, rhsDriver->const_CollisionWeight);
}

static void VehPhysCrash_AddImpulse(Vec3 *vel, const SVec3 *hitDir, s32 strength)
{
	vel->x = CTR_MipsAddLo(vel->x, CTR_MipsSra(CTR_MipsMulLo(hitDir->x, strength), VEH_PHYS_CRASH_VECTOR_SPEED_SHIFT));
	vel->y = CTR_MipsAddLo(vel->y, CTR_MipsSra(CTR_MipsMulLo(hitDir->y, strength), VEH_PHYS_CRASH_VECTOR_SPEED_SHIFT));
	vel->z = CTR_MipsAddLo(vel->z, CTR_MipsSra(CTR_MipsMulLo(hitDir->z, strength), VEH_PHYS_CRASH_VECTOR_SPEED_SHIFT));
}

static void VehPhysCrash_SubImpulse(Vec3 *vel, const SVec3 *hitDir, s32 strength)
{
	vel->x = CTR_MipsSubLo(vel->x, CTR_MipsSra(CTR_MipsMulLo(hitDir->x, strength), VEH_PHYS_CRASH_VECTOR_SPEED_SHIFT));
	vel->y = CTR_MipsSubLo(vel->y, CTR_MipsSra(CTR_MipsMulLo(hitDir->y, strength), VEH_PHYS_CRASH_VECTOR_SPEED_SHIFT));
	vel->z = CTR_MipsSubLo(vel->z, CTR_MipsSra(CTR_MipsMulLo(hitDir->z, strength), VEH_PHYS_CRASH_VECTOR_SPEED_SHIFT));
}

static void VehPhysCrash_BouncePair(const SVec3 *hitDir, const Vec3 *weightedVel, Vec3 *otherVel, Vec3 *selfVel)
{
	if (VehPhysCrash_BounceSelf(hitDir, weightedVel, otherVel, 1) < 0)
	{
		sdata->vehicleCollisionImpactStrength = 0;
	}

	if (VehPhysCrash_BounceSelf(hitDir, weightedVel, selfVel, 0) > 0)
	{
		sdata->vehicleCollisionImpactStrength = 0;
	}
}

static void VehPhysCrash_PlayHumanFeedback(struct Thread *selfThread, struct Thread *otherThread, struct Driver *selfDriver, struct Driver *otherDriver,
                                           u32 canPlayFeedback)
{
	if (sdata->vehicleCollisionImpactStrength <= VEH_PHYS_CRASH_FEEDBACK_MIN_IMPACT)
	{
		return;
	}

	if ((selfThread->modelIndex == DYNAMIC_PLAYER) || (otherThread->modelIndex == DYNAMIC_PLAYER))
	{
		int volume = VehCalc_MapToRange(sdata->vehicleCollisionImpactStrength, 0, VEH_PHYS_CRASH_VOLUME_IMPACT_MAX, VEH_PHYS_CRASH_VOLUME_MIN,
		                                VEH_PHYS_CRASH_VOLUME_MAX);

		if ((canPlayFeedback != 0) && (selfDriver->kartState != KS_BLASTED) && (selfDriver->invincibleTimer == 0) && (otherDriver->kartState != KS_BLASTED) &&
		    (otherDriver->invincibleTimer == 0))
		{
			OtherFX_DriverCrashing((selfDriver->actionsFlagSet & ACTION_ENGINE_ECHO) != 0, volume);

			// NOTE(aalhendi): Retail uses DAT_8008d838. This field currently
			// names the same USA address as the last audioDefaults slot.
			sdata->audioDefaults[8] = sdata->gGT->frameTimer_MainFrame_ResetDB;

			if ((u32)volume > VEH_PHYS_CRASH_HARD_CRASH_VOLUME)
			{
				Voiceline_RequestPlay(VEH_PHYS_CRASH_VOICELINE_HARD_CRASH, data.characterIDs[selfDriver->driverID], VEH_PHYS_CRASH_VOICELINE_PRIORITY);
			}
		}
	}

	GAMEPAD_ShockFreq(otherDriver, VEH_PHYS_CRASH_RUMBLE_CHANNEL, 0);
	GAMEPAD_ShockForce1(otherDriver, VEH_PHYS_CRASH_RUMBLE_CHANNEL, VEH_PHYS_CRASH_RUMBLE_FORCE);
	GAMEPAD_JogCon1(otherDriver, (otherDriver->simpTurnState > 0) ? VEH_PHYS_CRASH_JOG_TURNING : VEH_PHYS_CRASH_JOG_STRAIGHT, VEH_PHYS_CRASH_JOG_DURATION);

	GAMEPAD_ShockFreq(selfDriver, VEH_PHYS_CRASH_RUMBLE_CHANNEL, 0);
	GAMEPAD_ShockForce1(selfDriver, VEH_PHYS_CRASH_RUMBLE_CHANNEL, VEH_PHYS_CRASH_RUMBLE_FORCE);
	GAMEPAD_JogCon1(selfDriver, (selfDriver->simpTurnState > 0) ? VEH_PHYS_CRASH_JOG_TURNING : VEH_PHYS_CRASH_JOG_STRAIGHT, VEH_PHYS_CRASH_JOG_DURATION);

	selfDriver->actionsFlagSet |= ACTION_HUMAN_HUMAN_COLLISION;
	otherDriver->actionsFlagSet |= ACTION_HUMAN_HUMAN_COLLISION;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005d404-0x8005e104
void VehPhysCrash_AnyTwoCars(struct Thread *thread, struct DriverCollisionSearch *search, Vec3 *selfVel)
{
	int distance = VehCalc_FastSqrt(search->bucket.bestDistSq, 0);
	const SVec3 *dist = &search->bucket.dist;
	SVec3 *hitDir = &search->hitDir;

	if (distance == 0)
	{
		CTR_SET_VEC3(hitDir->v, 0, 0, VEH_PHYS_CRASH_UNIT_VECTOR_SCALE);
	}
	else
	{
		CTR_SET_VEC3(hitDir->v, (s16)CTR_MipsDiv(CTR_MipsSll(dist->x, VEH_PHYS_CRASH_MATRIX_FRAC_SHIFT), distance),
		             (s16)CTR_MipsDiv(CTR_MipsSll(dist->y, VEH_PHYS_CRASH_MATRIX_FRAC_SHIFT), distance),
		             (s16)CTR_MipsDiv(CTR_MipsSll(dist->z, VEH_PHYS_CRASH_MATRIX_FRAC_SHIFT), distance));
	}

	struct Thread *otherThread = search->bucket.th;
	struct Driver *otherDriver = otherThread->object;
	struct Driver *selfDriver = thread->object;

	int hitStrength = CTR_MipsSubLo(CTR_MipsAddLo(thread->driverHitRadius, otherThread->driverHitRadius), distance);
	if (hitStrength <= 0)
	{
		return;
	}

	sdata->vehicleCollisionImpactStrength = 0;

	if ((selfDriver->actionsFlagSet & ACTION_BOT) != 0)
	{
		Vec3 otherVel;
		Vec3 weightedVel;

		if ((otherDriver->actionsFlagSet & ACTION_BOT) == 0)
		{
			VehPhysForce_ConvertSpeedToVecOut(otherDriver, &otherVel);
			VehPhysCrash_WeightedVelocity(&weightedVel, selfVel, selfDriver, &otherVel, otherDriver);
			VehPhysCrash_BouncePair(hitDir, &weightedVel, &otherVel, selfVel);
			VehPhysCrash_AddImpulse(selfVel, hitDir, hitStrength);
			VehPhysCrash_SubImpulse(&otherVel, hitDir, hitStrength);
			VehPhysCrash_AI(selfDriver, selfVel);
			VehPhysCrash_ConvertVecToSpeed(otherDriver, &otherVel);
		}
		else
		{
			otherVel.x = CTR_MipsAddLo(otherDriver->xSpeed, otherDriver->botData.aiPhysics.accel.x);
			otherVel.y = CTR_MipsAddLo(otherDriver->ySpeed, otherDriver->botData.aiPhysics.accel.y);
			otherVel.z = CTR_MipsAddLo(otherDriver->zSpeed, otherDriver->botData.aiPhysics.accel.z);

			VehPhysCrash_WeightedVelocity(&weightedVel, selfVel, selfDriver, &otherVel, otherDriver);
			VehPhysCrash_BouncePair(hitDir, &weightedVel, &otherVel, selfVel);
			VehPhysCrash_AddImpulse(selfVel, hitDir, hitStrength);
			VehPhysCrash_SubImpulse(&otherVel, hitDir, hitStrength);
			VehPhysCrash_AI(otherDriver, &otherVel);
			VehPhysCrash_AI(selfDriver, selfVel);
			BOTS_CollideWithOtherAI(selfDriver, otherDriver);
		}

		return;
	}

	if ((otherDriver->actionsFlagSet & ACTION_BOT) != 0)
	{
		Vec3 otherVel;
		Vec3 weightedVel;

		otherVel.x = CTR_MipsAddLo(otherDriver->xSpeed, otherDriver->botData.aiPhysics.accel.x);
		otherVel.y = CTR_MipsAddLo(otherDriver->ySpeed, otherDriver->botData.aiPhysics.accel.y);
		otherVel.z = CTR_MipsAddLo(otherDriver->zSpeed, otherDriver->botData.aiPhysics.accel.z);

		VehPhysCrash_WeightedVelocity(&weightedVel, selfVel, selfDriver, &otherVel, otherDriver);
		VehPhysCrash_BouncePair(hitDir, &weightedVel, &otherVel, selfVel);
		VehPhysCrash_AddImpulse(selfVel, hitDir, hitStrength);
		VehPhysCrash_SubImpulse(&otherVel, hitDir, hitStrength);
		VehPhysCrash_AI(otherDriver, &otherVel);
	}
	else
	{
		Vec3 weightedVel;
		Vec3 *otherVel = &otherDriver->velocity;

		VehPhysCrash_WeightedVelocity(&weightedVel, selfVel, selfDriver, otherVel, otherDriver);
		VehPhysCrash_BouncePair(hitDir, &weightedVel, otherVel, selfVel);
		VehPhysCrash_AddImpulse(selfVel, hitDir, hitStrength);
		VehPhysCrash_SubImpulse(otherVel, hitDir, hitStrength);
	}

	u32 canPlayFeedback = ((u32)CTR_MipsSubLo(sdata->gGT->frameTimer_MainFrame_ResetDB, sdata->audioDefaults[8]) >= VEH_PHYS_CRASH_FEEDBACK_COOLDOWN_FRAMES);

	VehPhysCrash_PlayHumanFeedback(thread, otherThread, selfDriver, otherDriver, canPlayFeedback);

	int attackResult = VehPhysCrash_Attack(selfDriver, otherDriver, canPlayFeedback, 0);
	VehPhysCrash_Attack(otherDriver, selfDriver, attackResult, 1);
}
