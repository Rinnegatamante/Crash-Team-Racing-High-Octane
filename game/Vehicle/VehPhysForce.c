#include <common.h>


// NOTE(aalhendi): ASM-verified helper for NTSC-U 926 0x8005e104-0x8005e214.
void VehPhysForce_ConvertSpeedToVecOut(struct Driver *driver, Vec3 *vel)
{
	int yAngle = driver->axisRotationY;
	int ySine = MATH_Sin(yAngle);
	int yCos = MATH_Cos(yAngle);
	int yComponent = CTR_MipsSra(CTR_MipsMulLo(driver->speed, yCos), FRACTIONAL_BITS);

	int xAngle = driver->axisRotationX;
	int xCos = MATH_Cos(xAngle);
	int xSine = MATH_Sin(xAngle);

	vel->x = CTR_MipsSra(CTR_MipsMulLo(yComponent, xSine), FRACTIONAL_BITS);
	vel->y = CTR_MipsSra(CTR_MipsMulLo(driver->speed, ySine), FRACTIONAL_BITS);
	vel->z = CTR_MipsSra(CTR_MipsMulLo(yComponent, xCos), FRACTIONAL_BITS);
}

void VehPhysForce_ConvertSpeedToVec(struct Driver *driver)
{
	VehPhysForce_ConvertSpeedToVecOut(driver, &driver->velocity);
}

enum
{
	VEH_PHYS_FORCE_QUAD_LOW_GRAVITY = 0x2,
	VEH_PHYS_FORCE_LOW_GRAVITY_DIVISOR = 100,
	VEH_PHYS_FORCE_MUD_TERMINAL_SPEED = 0x100,
	VEH_PHYS_FORCE_SKID_SPEED_THRESHOLD = 0x300,
	VEH_PHYS_FORCE_TERRAIN_SCALE_NEUTRAL = 0x100,
	VEH_PHYS_FORCE_TERRAIN_SIDE_LOCK_TIMER = -0x140,
	VEH_PHYS_FORCE_TERRAIN_RUMBLE_FRAMES = 4,
	VEH_PHYS_FORCE_TERRAIN_RUMBLE_FORCE = 0x7f,
	VEH_PHYS_FORCE_ROLLBACK_WINDOW_TIMER = 0x280,
	VEH_PHYS_FORCE_MATRIX_BLEND_FULL = 0x100,
	VEH_PHYS_FORCE_KART_SCALE_BASE = 0xccc,
	VEH_PHYS_FORCE_MASK_GRAB_SCALE_XZ_FACTOR = 0x28,
	VEH_PHYS_FORCE_MIN_SQUASH_XZ_SCALE = 0x400,
	VEH_PHYS_FORCE_TARGET_SQUISH_DEFAULT = -800,
	VEH_PHYS_FORCE_SQUISH_DEADBAND = 0x960,
	VEH_PHYS_FORCE_AIR_SQUISH_MIN = -800,
	VEH_PHYS_FORCE_LANDING_SQUISH_MIN = -0x640,
	VEH_PHYS_FORCE_SQUISH_MAX = 800,
	VEH_PHYS_FORCE_HAZARD_BLINK_MASK = 0x80,
	VEH_PHYS_FORCE_FALL_STRETCH_HEIGHT_MAX = 0xa00,
	VEH_PHYS_FORCE_FALL_STRETCH_MIN = 0x280,
	VEH_PHYS_FORCE_FALL_STRETCH_MAX = 0x320,
	VEH_PHYS_FORCE_TNT_SCALE_Y_THRESHOLD = 2500,
	VEH_PHYS_FORCE_TNT_SCALE_Y_BASE = 0x800,
	VEH_PHYS_FORCE_SQUISH_INTERP_SPEED = 300,
	VEH_PHYS_FORCE_SQUISH_SCALE_INTERP_SPEED = 0xa0,
	VEH_PHYS_FORCE_SQUISH_SCALE_XZ_FACTOR = 0xa0,
	VEH_PHYS_FORCE_SQUISH_RESTORE_SFX = 0x5b,
	VEH_PHYS_FORCE_SQUISH_OFFSET_NORMAL_SCALE = 0x13,
	VEH_PHYS_FORCE_WAKE_PARTICLE_ICON_GROUP = 9,
	VEH_PHYS_FORCE_WAKE_WATERLINE_Y = 0,
	VEH_PHYS_FORCE_WAKE_VISIBLE_MIN_Y = -0x4f,
	VEH_PHYS_FORCE_WAKE_INITIAL_SCALE = 0x1000,
	VEH_PHYS_FORCE_WAKE_PARTICLE_SPEED_MIN = 0xc00,
	VEH_PHYS_FORCE_WAKE_PARTICLE_PREV_Y_MIN = -0x200,
	VEH_PHYS_FORCE_WAKE_BURST_PARTICLE_COUNT = 10,
	VEH_PHYS_FORCE_TURBO_PAD_RESERVES = 0x3c0,
	VEH_PHYS_FORCE_SUPER_TURBO_PAD_RESERVES = 0x78,
	VEH_PHYS_FORCE_TURBO_PAD_FIRE_LEVEL = 0x100,
	VEH_PHYS_FORCE_SUPER_TURBO_PAD_FIRE_LEVEL = 0x800,
	VEH_PHYS_FORCE_COLLISION_BEST_DIST_INIT = 0x7fffffff,
	VEH_PHYS_FORCE_SURFACE_PUSHBACK_Y_BIAS = 4,
	VEH_PHYS_FORCE_SURFACE_PUSHBACK_SHIFT = 6,
};

CTR_STATIC_ASSERT(VEH_PHYS_FORCE_QUAD_LOW_GRAVITY == 0x2);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_LOW_GRAVITY_DIVISOR == 100);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_MUD_TERMINAL_SPEED == 0x100);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_SKID_SPEED_THRESHOLD == 0x300);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_TERRAIN_SCALE_NEUTRAL == 0x100);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_TERRAIN_SIDE_LOCK_TIMER == -0x140);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_TERRAIN_RUMBLE_FRAMES == 4);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_TERRAIN_RUMBLE_FORCE == 0x7f);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_ROLLBACK_WINDOW_TIMER == 0x280);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_MATRIX_BLEND_FULL == 0x100);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_KART_SCALE_BASE == 0xccc);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_MASK_GRAB_SCALE_XZ_FACTOR == 0x28);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_MIN_SQUASH_XZ_SCALE == 0x400);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_TARGET_SQUISH_DEFAULT == -800);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_SQUISH_DEADBAND == 0x960);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_AIR_SQUISH_MIN == -800);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_LANDING_SQUISH_MIN == -0x640);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_SQUISH_MAX == 800);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_HAZARD_BLINK_MASK == 0x80);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_FALL_STRETCH_HEIGHT_MAX == 0xa00);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_FALL_STRETCH_MIN == 0x280);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_FALL_STRETCH_MAX == 0x320);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_TNT_SCALE_Y_THRESHOLD == 2500);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_TNT_SCALE_Y_BASE == 0x800);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_SQUISH_INTERP_SPEED == 300);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_SQUISH_SCALE_INTERP_SPEED == 0xa0);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_SQUISH_SCALE_XZ_FACTOR == 0xa0);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_SQUISH_RESTORE_SFX == 0x5b);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_SQUISH_OFFSET_NORMAL_SCALE == 0x13);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_WAKE_PARTICLE_ICON_GROUP == 9);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_WAKE_WATERLINE_Y == 0);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_WAKE_VISIBLE_MIN_Y == -0x4f);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_WAKE_INITIAL_SCALE == 0x1000);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_WAKE_PARTICLE_SPEED_MIN == 0xc00);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_WAKE_PARTICLE_PREV_Y_MIN == -0x200);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_WAKE_BURST_PARTICLE_COUNT == 10);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_TURBO_PAD_RESERVES == 0x3c0);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_SUPER_TURBO_PAD_RESERVES == 0x78);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_TURBO_PAD_FIRE_LEVEL == 0x100);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_SUPER_TURBO_PAD_FIRE_LEVEL == 0x800);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_COLLISION_BEST_DIST_INIT == 0x7fffffff);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_SURFACE_PUSHBACK_Y_BIAS == 4);
CTR_STATIC_ASSERT(VEH_PHYS_FORCE_SURFACE_PUSHBACK_SHIFT == 6);

static int VehPhysForce_OnGravity_Abs(int value)
{
	return value < 0 ? CTR_MipsNegLo(value) : value;
}

static void VehPhysForce_OnGravity_SetLightMatrixTranspose(const MATRIX *m)
{
	u32 r0 = CTR_PackS16Pair(m->m[0][0], m->m[0][1]);
	u32 r1 = CTR_PackS16Pair(m->m[0][2], m->m[1][0]);
	u32 r2 = CTR_PackS16Pair(m->m[1][1], m->m[1][2]);
	u32 r3 = CTR_PackS16Pair(m->m[2][0], m->m[2][1]);
	s16 r4 = m->m[2][2];
	const u32 highMask = 0xffff0000u;

	// NOTE(aalhendi): Retail constructs a transposed copy of matrixMovingDir in
	// the light matrix regs before projecting velocity and gravity into local space.
	CTC2((r0 & 0xffffu) | (r1 & highMask), 8);
	CTC2((r3 & 0xffffu) | (r0 & highMask), 9);
	CTC2((r2 & 0xffffu) | (r3 & highMask), 10);
	CTC2((r1 & 0xffffu) | (r2 & highMask), 11);
	CTC2((u32)(s32)r4, 12);
}

static Vec3 VehPhysForce_OnGravity_RotateVectorLocal(const MATRIX *m, s16 vx, s16 vy, s16 vz)
{
	Vec3 out;

	(void)m;
	MTC2(CTR_PackS16Pair(vx, vy), 0);
	MTC2((u32)(u16)vz, 1);
	gte_mvmva(1, 1, 0, 3, 0);
	out.x = MFC2_S(25);
	out.y = MFC2_S(26);
	out.z = MFC2_S(27);

	return out;
}

static Vec3 VehPhysForce_OnGravity_RotateVector(const MATRIX *m, s16 vx, s16 vy, s16 vz)
{
	Vec3 out;

	(void)m;
	MTC2(CTR_PackS16Pair(vx, vy), 0);
	MTC2((u32)(u16)vz, 1);
	gte_mvmva(1, 0, 0, 3, 0);
	out.x = MFC2_S(25);
	out.y = MFC2_S(26);
	out.z = MFC2_S(27);

	return out;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005e214-0x8005ea60
void VehPhysForce_OnGravity(struct Driver *driver, Vec3 *velocity)
{
	int elapsedTimeMS = sdata->gGT->elapsedTimeMS;

	gte_SetRotMatrix(&driver->matrixMovingDir);
	VehPhysForce_OnGravity_SetLightMatrixTranspose(&driver->matrixMovingDir);

	Vec3 localVelocity = VehPhysForce_OnGravity_RotateVectorLocal(&driver->matrixMovingDir, (s16)velocity->x, (s16)velocity->y, (s16)velocity->z);
	int originalLocalZ = localVelocity.z;
	int gravityY = CTR_MipsNegLo(driver->const_Gravity);
	struct QuadBlock *underDriver = driver->underDriver;

	// NOTE(aalhendi): Retail does not branch before reading this flag. Native
	// can reach this frame after TeleportSelf clears underDriver and before
	// fixed collision repopulates it; PS1 low-RAM behavior is no low-gravity
	// flag, while host C would crash on a null dereference.
	if ((underDriver != NULL) && ((underDriver->quadFlags & VEH_PHYS_FORCE_QUAD_LOW_GRAVITY) != 0))
	{
		int scaledGravity = CTR_MipsAddLo(CTR_MipsSll(gravityY, 2), gravityY);
		gravityY = CTR_MipsAddLo(CTR_MipsSll(scaledGravity, 3), gravityY) / VEH_PHYS_FORCE_LOW_GRAVITY_DIVISOR;
	}

	gravityY = CTR_MipsSra(CTR_MipsMulLo(gravityY, elapsedTimeMS), 5);
	Vec3 localGravity = VehPhysForce_OnGravity_RotateVectorLocal(&driver->matrixMovingDir, 0, (s16)gravityY, 0);

	if (((localGravity.z < 0) && (driver->forwardAccelImpulse > 0)) || ((localGravity.z > 0) && (driver->forwardAccelImpulse < 0)))
	{
		localGravity.z = 0;
	}

	u32 actionsFlagSet = driver->actionsFlagSet;
	int speedApprox = driver->speedApprox;
	int baseSpeed = driver->baseSpeed;
	if ((actionsFlagSet & ACTION_ACCEL_PREVENTION) || ((baseSpeed > 0) && (speedApprox < 0)) || ((driver->baseSpeed < 0) && (driver->speedApprox > 0)))
	{
		localGravity.x = 0;
		localGravity.z = 0;
	}

	int originalLocalX = localVelocity.x;
	int originalLocalY = localVelocity.y;
	int localX = CTR_MipsAddLo(originalLocalX, localGravity.x);
	int localY = CTR_MipsAddLo(originalLocalY, localGravity.y);
	int localZ = CTR_MipsAddLo(originalLocalZ, localGravity.z);

	int maxForwardSpeed = CTR_MipsAddLo(driver->fireSpeed, driver->const_SlopeForwardSpeedBonus);
	if (localZ > maxForwardSpeed)
	{
		localZ = originalLocalZ;
		if (originalLocalZ < maxForwardSpeed)
		{
			localZ = maxForwardSpeed;
		}
	}

	int minForwardSpeed = CTR_MipsSubLo(driver->fireSpeed, CTR_MipsSra(driver->const_SlopeForwardSpeedBonus, 1));
	if (localZ < minForwardSpeed)
	{
		localZ = originalLocalZ;
		if (originalLocalZ > minForwardSpeed)
		{
			localZ = minForwardSpeed;
		}
	}

	int maxPerpendicularSpeed = driver->const_SideSpeedClamp;
	if (localX > maxPerpendicularSpeed)
	{
		localX = originalLocalX;
		if (originalLocalX < maxPerpendicularSpeed)
		{
			localX = maxPerpendicularSpeed;
		}
	}

	int minPerpendicularSpeed = CTR_MipsNegLo(maxPerpendicularSpeed);
	if (localX < minPerpendicularSpeed)
	{
		localX = originalLocalX;
		if (originalLocalX > minPerpendicularSpeed)
		{
			localX = minPerpendicularSpeed;
		}
	}

	TerrainFlags terrainFlags = driver->terrainMeta1->flags;
	int terminalVelocity = driver->const_TerminalVelocity;
	if ((localY < 0) && ((terrainFlags & TERRAIN_FLAG_MUD_PHYSICS) != 0))
	{
		terminalVelocity = VEH_PHYS_FORCE_MUD_TERMINAL_SPEED;
		if (originalLocalY < -VEH_PHYS_FORCE_MUD_TERMINAL_SPEED)
		{
			originalLocalY = -VEH_PHYS_FORCE_MUD_TERMINAL_SPEED;
		}
	}

	int clampedLocalY = localY;
	if (localY > terminalVelocity)
	{
		clampedLocalY = originalLocalY;
		if (originalLocalY < terminalVelocity)
		{
			clampedLocalY = terminalVelocity;
		}
	}

	int minTerminalVelocity = CTR_MipsNegLo(terminalVelocity);
	if (clampedLocalY < minTerminalVelocity)
	{
		clampedLocalY = originalLocalY;
		if (originalLocalY > minTerminalVelocity)
		{
			clampedLocalY = minTerminalVelocity;
		}
	}

	localY = clampedLocalY;

	if (driver->kartState == KS_MASK_GRABBED)
	{
		localX = 0;
		localZ = 0;
	}
	else if (((driver->actionsFlagSetPrevFrame & ACTION_TOUCH_GROUND) != 0) || (driver->kartState == KS_BLASTED) ||
	         ((driver->terrainScaledBaseSpeed < driver->speedApprox) && (driver->terrainMeta2->speedMultiplier < VEH_PHYS_FORCE_TERRAIN_SCALE_NEUTRAL)))
	{
		int perpendicularFriction;
		int forwardFriction;

		if ((actionsFlagSet & ACTION_ACCEL_PREVENTION) == 0)
		{
			if (baseSpeed == 0)
			{
				perpendicularFriction = driver->const_NoPedalFriction_Perpendicular;
				forwardFriction = driver->const_NoPedalFriction_Forward;

				if (driver->rainCloudEffect == RAIN_CLOUD_EFFECT_HEAVY_FRICTION)
				{
					perpendicularFriction = CTR_MipsSll(driver->const_BrakeFriction, 4);
					forwardFriction = perpendicularFriction;
				}
			}
			else
			{
				int absSpeedApprox = VehPhysForce_OnGravity_Abs(speedApprox);
				if ((absSpeedApprox < (VEH_PHYS_FORCE_SKID_SPEED_THRESHOLD + 1)) ||
				    (((baseSpeed < 1) || (speedApprox >= 0)) && ((baseSpeed >= 0) || (speedApprox < 1))))
				{
					if (driver->kartState == KS_DRIFTING)
					{
						perpendicularFriction = driver->const_DriftCurve;
						forwardFriction = driver->const_DriftFriction;
					}
					else
					{
						perpendicularFriction = driver->const_PedalFriction_Perpendicular;
						forwardFriction = driver->const_PedalFriction_Forward;

						if (absSpeedApprox > VEH_PHYS_FORCE_SKID_SPEED_THRESHOLD)
						{
							int absBaseSpeed = VehPhysForce_OnGravity_Abs(baseSpeed);
							if (absSpeedApprox < CTR_MipsSra(absBaseSpeed, 1))
							{
								actionsFlagSet |= ACTION_BACK_SKID;
							}
						}
					}
				}
				else
				{
					perpendicularFriction = driver->const_PedalFriction_Perpendicular;
					forwardFriction = driver->const_BrakeFriction;

					if (absSpeedApprox > VEH_PHYS_FORCE_SKID_SPEED_THRESHOLD)
					{
						actionsFlagSet |= ACTION_BACK_SKID;
					}
				}
			}
		}
		else
		{
			int absSpeedApprox = VehPhysForce_OnGravity_Abs(speedApprox);
			if (absSpeedApprox > VEH_PHYS_FORCE_SKID_SPEED_THRESHOLD)
			{
				actionsFlagSet |= ACTION_BACK_SKID;
			}

			perpendicularFriction = driver->const_BrakeFriction;
			if (driver->rainCloudEffect == RAIN_CLOUD_EFFECT_HEAVY_FRICTION)
			{
				perpendicularFriction = CTR_MipsSll(perpendicularFriction, 4);
				forwardFriction = perpendicularFriction;
			}
			else if (driver->kartState == KS_BLASTED)
			{
				perpendicularFriction = CTR_MipsSra(CTR_MipsMulLo(perpendicularFriction, 3), 2);
				forwardFriction = perpendicularFriction;
			}
			else
			{
				forwardFriction = perpendicularFriction;
				if (driver->kartState == KS_SPINNING)
				{
					perpendicularFriction = CTR_MipsSra(perpendicularFriction, 1);
					forwardFriction = perpendicularFriction;
				}
			}
		}

		perpendicularFriction = CTR_MipsSra(CTR_MipsMulLo(perpendicularFriction, elapsedTimeMS), 5);
		forwardFriction = CTR_MipsSra(CTR_MipsMulLo(forwardFriction, elapsedTimeMS), 5);

		int terrainFrictionScale = driver->terrainMeta1->groundFrictionScale;
		if (terrainFrictionScale != VEH_PHYS_FORCE_TERRAIN_SCALE_NEUTRAL)
		{
			perpendicularFriction = CTR_MipsSra(CTR_MipsMulLo(terrainFrictionScale, perpendicularFriction), 8);
			forwardFriction = CTR_MipsSra(CTR_MipsMulLo(terrainFrictionScale, forwardFriction), 8);
		}

		int terrainTimer = driver->terrainFrictionTimer;
		if (terrainTimer < 0)
		{
			int absLocalX = localX;
			if (terrainTimer == VEH_PHYS_FORCE_TERRAIN_SIDE_LOCK_TIMER)
			{
				absLocalX = VehPhysForce_OnGravity_Abs(localX);
				perpendicularFriction = CTR_MipsSra(absLocalX, 1);
			}
			else
			{
				perpendicularFriction =
				    CTR_MipsAddLo(perpendicularFriction, CTR_MipsSra(CTR_MipsMulLo(perpendicularFriction, driver->const_TerrainFrictionBoost), 8));
				if (perpendicularFriction < 0)
				{
					perpendicularFriction = 0;
				}

				absLocalX = VehPhysForce_OnGravity_Abs(localX);
			}

			if (absLocalX > 0)
			{
				actionsFlagSet |= ACTION_BACK_SKID | ACTION_FRONT_SKID;
				GAMEPAD_ShockForce1(driver, VEH_PHYS_FORCE_TERRAIN_RUMBLE_FRAMES, VEH_PHYS_FORCE_TERRAIN_RUMBLE_FORCE);
				GAMEPAD_ShockFreq(driver, VEH_PHYS_FORCE_TERRAIN_RUMBLE_FRAMES, 0);
			}

			terrainTimer = CTR_MipsAddLo(terrainTimer, elapsedTimeMS);
			if (terrainTimer > 0)
			{
				terrainTimer = 0;
			}
			driver->terrainFrictionTimer = (s16)terrainTimer;
		}
		else if (terrainTimer > 0)
		{
			terrainTimer = CTR_MipsSubLo(terrainTimer, elapsedTimeMS);
			if (terrainTimer < 0)
			{
				terrainTimer = 0;
			}

			perpendicularFriction =
			    CTR_MipsAddLo(perpendicularFriction, CTR_MipsSra(CTR_MipsMulLo(perpendicularFriction, driver->const_TerrainFrictionBoost), 8));
			driver->terrainFrictionTimer = (s16)terrainTimer;
			if (perpendicularFriction < 0)
			{
				perpendicularFriction = 0;
			}
		}

		if (((actionsFlagSet & ACTION_MASK_WEAPON) == 0) && ((terrainFlags & TERRAIN_FLAG_MUD_PHYSICS) != 0))
		{
			int absSideSpeed = VehPhysForce_OnGravity_Abs(CTR_MipsSra(localX, 3));
			if (perpendicularFriction < absSideSpeed)
			{
				perpendicularFriction = absSideSpeed;
			}

			int minForwardFriction = 0;
			if ((localZ == 0) || (baseSpeed == 0) || ((localZ ^ baseSpeed) >= 0))
			{
				if (((baseSpeed <= localZ) || (localZ > 0)) && ((localZ <= baseSpeed) || (localZ < 0)))
				{
					goto APPLY_TERRAIN_FRICTION;
				}

				minForwardFriction = CTR_MipsSra(VehPhysForce_OnGravity_Abs(CTR_MipsSubLo(localZ, baseSpeed)), 1);
			}
			else
			{
				minForwardFriction = VehPhysForce_OnGravity_Abs(CTR_MipsSra(localZ, 1));
			}

			if (forwardFriction < minForwardFriction)
			{
				forwardFriction = minForwardFriction;
			}
		}

	APPLY_TERRAIN_FRICTION:
	{
		int xFriction = perpendicularFriction;
		if ((terrainFlags & TERRAIN_FLAG_SIDESLIP_FRICTION) != 0)
		{
			xFriction = CTR_MipsSra(CTR_MipsMulLo(perpendicularFriction, 3), 2);
			if (xFriction < forwardFriction)
			{
				xFriction = forwardFriction;
			}
		}

		localX = VehCalc_InterpBySpeed(localX, xFriction, 0);
		localZ = VehCalc_InterpBySpeed(localZ, forwardFriction, 0);
	}
	}

	*velocity = VehPhysForce_OnGravity_RotateVector(&driver->matrixMovingDir, (s16)localX, (s16)localY, (s16)localZ);
	driver->actionsFlagSet = actionsFlagSet;

	if ((actionsFlagSet & ACTION_AIRBORNE) == 0)
	{
		if ((driver->boolFirstFrameSinceRevEngine != 0) && (localZ != 0))
		{
			driver->forwardDir = (localZ < 0) ? -1 : 1;
			driver->boolFirstFrameSinceRevEngine = 0;
			goto CHECK_ROLLBACK_FROM_PREVIOUS_DIRECTION;
		}

		if (originalLocalZ < 0)
		{
			goto SET_FORWARD_DIRECTION_IF_NONNEGATIVE;
		}

		if (localZ < 0)
		{
			driver->forwardDir = -1;
		}

		if (originalLocalZ > 0)
		{
			goto CHECK_ROLLBACK_FROM_FORWARD;
		}

	SET_FORWARD_DIRECTION_IF_NONNEGATIVE:
		if (localZ >= 0)
		{
			driver->forwardDir = 1;
		}
	}

CHECK_ROLLBACK_FROM_PREVIOUS_DIRECTION:
	if (originalLocalZ >= 0)
	{
		goto CHECK_ROLLBACK_FROM_FORWARD;
	}

	if (localZ <= 0)
	{
		return;
	}
	goto START_ROLLBACK;

CHECK_ROLLBACK_FROM_FORWARD:
	if (localZ < 0)
	{
		goto START_ROLLBACK;
	}
	if (originalLocalZ > 0)
	{
		return;
	}
	if (localZ <= 0)
	{
		return;
	}

START_ROLLBACK:
	if (driver->vShiftWindowTimer != 0)
	{
		driver->vShiftCount = (s16)CTR_MipsAddLo((u16)driver->vShiftCount, 1);
	}
	driver->vShiftWindowTimer = VEH_PHYS_FORCE_ROLLBACK_WINDOW_TIMER;
}

static Vec3 VehPhysForce_OnApplyForces_RotateVector(const MATRIX *m, s16 vx, s16 vy, s16 vz)
{
	Vec3 out;

	// NOTE(aalhendi): Retail loads matrixFacingDir into CP2 color matrix regs
	// and runs opcode 0x4c6012 for this center-offset calculation.
	gte_SetColorMatrix(m);
	MTC2(CTR_PackS16Pair(vx, vy), 0);
	MTC2((u32)(u16)vz, 1);
	gte_mvmva(1, 2, 0, 3, 0);
	out.x = MFC2_S(25);
	out.y = MFC2_S(26);
	out.z = MFC2_S(27);

	return out;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005ea60-0x8005ebac.
void VehPhysForce_OnApplyForces(struct Thread *thread, struct Driver *driver)
{
	(void)thread;

	const int maxMudSinkYLevel = FP(-1);
	const int maxSpeed = FP8(100);
	driver->speed = min(driver->speed, maxSpeed);

	/* origin of driver model is center-bottom of kart,
	use orientation matrix, and half-radius {0, 25, 0},
	to find the "true" center of the 3D model */
	driver->originToCenter = VehPhysForce_OnApplyForces_RotateVector(&driver->matrixFacingDir, 0, 25, 0);

	VehPhysForce_ConvertSpeedToVecOut(driver, &driver->velocity);

	if ((driver->underDriver) && (driver->underDriver->terrain_type == TERRAIN_MUD))
	{
		if (driver->posCurr.y > maxMudSinkYLevel)
		{
			// sink slower as you approach the mud's bottom
			int sinkSpeed = CTR_MipsSubLo(maxMudSinkYLevel, driver->posCurr.y);
			driver->velocity.y = max(driver->velocity.y, sinkSpeed);
		}
	}

	VehPhysForce_OnGravity(driver, &driver->velocity);

	const SVec3 up = {.x = FP(0), .y = FP(1), .z = FP(0)};
	driver->normalVecUP = up;
	driver->AxisAngle1_normalVec = up;
	driver->collisionFlags = 0;
	driver->currBlockTouching = nullptr;

	driver->velocity.x = CTR_MipsAddLo(driver->velocity.x, driver->accel.x);
	driver->velocity.z = CTR_MipsAddLo(driver->velocity.z, driver->accel.z);
	driver->velocity.y = CTR_MipsAddLo(driver->velocity.y, driver->accel.y);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005ebac-0x8005ee34.
void VehPhysForce_CollideDrivers(struct Thread *thread, struct Driver *driver)
{
	CollStepFlags stepFlagSet = driver->stepFlagSet;

	driver->velocity.x = CTR_MipsSubLo(driver->velocity.x, driver->accel.x);
	driver->velocity.y = CTR_MipsSubLo(driver->velocity.y, driver->accel.y);
	driver->velocity.z = CTR_MipsSubLo(driver->velocity.z, driver->accel.z);

	if ((stepFlagSet & COLL_STEP_FLAG_KILL_PLANE) != 0)
	{
		driver->collisionFlags |= DRIVER_COLL_FLAG_MASK_GRAB_REQUEST;
	}

	if ((stepFlagSet & COLL_STEP_TRIGGER_SUPER_TURBO_PAD) != 0)
	{
		VehFire_Increment(driver, VEH_PHYS_FORCE_SUPER_TURBO_PAD_RESERVES, TURBO_PAD | FREEZE_RESERVES_ON_TURBO_PAD, VEH_PHYS_FORCE_SUPER_TURBO_PAD_FIRE_LEVEL);
	}
	else if ((stepFlagSet & COLL_STEP_TRIGGER_TURBO_PAD) != 0)
	{
		if ((sdata->gGT->gameMode2 & CHEAT_TURBOPAD) == 0)
		{
			VehFire_Increment(driver, VEH_PHYS_FORCE_TURBO_PAD_RESERVES, TURBO_PAD | FREEZE_RESERVES_ON_TURBO_PAD, VEH_PHYS_FORCE_TURBO_PAD_FIRE_LEVEL);
		}
		else
		{
			VehFire_Increment(driver, VEH_PHYS_FORCE_SUPER_TURBO_PAD_RESERVES, TURBO_PAD | FREEZE_RESERVES_ON_TURBO_PAD,
			                  VEH_PHYS_FORCE_SUPER_TURBO_PAD_FIRE_LEVEL);
		}
	}

	if ((stepFlagSet & COLL_STEP_FLAG_WATER_BSP) != 0)
	{
		thread->inst->vertSplit = 0;
		thread->inst->flags |= SPLIT_LINE;
	}
	else
	{
		thread->inst->flags &= ~SPLIT_LINE;
	}

	if ((thread->flags & THREAD_FLAG_DISABLE_COLLISION) == 0)
	{
		struct DriverCollisionSearch search;

		CTR_SET_VEC3(search.bucket.pos.v, (s16)CTR_MipsSra(driver->posCurr.x, FRACTIONAL_BITS_8), (s16)CTR_MipsSra(driver->posCurr.y, FRACTIONAL_BITS_8),
		             (s16)CTR_MipsSra(driver->posCurr.z, FRACTIONAL_BITS_8));
		search.bucket.th = NULL;
		search.bucket.bestDistSq = VEH_PHYS_FORCE_COLLISION_BEST_DIST_INIT;

		PROC_CollidePointWithBucket(thread->siblingThread, &search.bucket);
		PROC_CollidePointWithBucket(sdata->gGT->threadBuckets[ROBOT].thread, &search.bucket);

		if (search.bucket.th != NULL)
		{
			int radiusSum = CTR_MipsAddLo(thread->driverHitRadius, search.bucket.th->driverHitRadius);

			if (search.bucket.bestDistSq < CTR_MipsMulLo(radiusSum, radiusSum))
			{
				VehPhysCrash_AnyTwoCars(thread, &search, &driver->velocity);
			}
		}
	}

	if ((driver->collisionFlags & DRIVER_COLL_FLAG_SURFACE_PUSHBACK) != 0)
	{
		int diffX = CTR_MipsSubLo(CTR_MipsSra(driver->posCurr.x, FRACTIONAL_BITS_8), driver->spsHitPos.x);
		int diffZ = CTR_MipsSubLo(CTR_MipsSra(driver->posCurr.z, FRACTIONAL_BITS_8), driver->spsHitPos.z);
		int floorDiffY =
		    CTR_MipsAddLo(CTR_MipsSubLo(CTR_MipsSra(driver->quadBlockHeight, FRACTIONAL_BITS_8), driver->spsHitPos.y), VEH_PHYS_FORCE_SURFACE_PUSHBACK_Y_BIAS);

		if (CTR_MipsAddLo(CTR_MipsAddLo(CTR_MipsMulLo(driver->spsNormalVec.x, diffX), CTR_MipsMulLo(driver->spsNormalVec.y, floorDiffY)),
		                  CTR_MipsMulLo(driver->spsNormalVec.z, diffZ)) < 0)
		{
			int diffY = CTR_MipsSubLo(CTR_MipsSra(driver->posCurr.y, FRACTIONAL_BITS_8), driver->spsHitPos.y);

			driver->velocity.x = CTR_MipsAddLo(driver->velocity.x, CTR_MipsSll(diffX, VEH_PHYS_FORCE_SURFACE_PUSHBACK_SHIFT));
			driver->velocity.y = CTR_MipsAddLo(driver->velocity.y, CTR_MipsSll(diffY, VEH_PHYS_FORCE_SURFACE_PUSHBACK_SHIFT));
			driver->velocity.z = CTR_MipsAddLo(driver->velocity.z, CTR_MipsSll(diffZ, VEH_PHYS_FORCE_SURFACE_PUSHBACK_SHIFT));
		}
	}
}

static int VehPhysForce_TranslateMatrix_Abs(int value)
{
	return value < 0 ? CTR_MipsNegLo(value) : value;
}

static int VehPhysForce_TranslateMatrix_Div256TowardZero(int value)
{
	if (value < 0)
	{
		value = CTR_MipsAddLo(value, 0xff);
	}

	return CTR_MipsSra(value, 8);
}

static Vec3 VehPhysForce_TranslateMatrix_RotateVector(const MATRIX *m, s16 vx, s16 vy, s16 vz)
{
	Vec3 out;

	// NOTE(aalhendi): Retail loads matrixFacingDir into CP2 control regs and
	// rotates the baked animation offset with opcode 0x486012. The GTE side
	// effects are observable by later vehicle wobble paths.
	gte_SetRotMatrix(m);
	MTC2(CTR_PackS16Pair(vx, vy), 0);
	MTC2((u32)(u16)vz, 1);
	gte_mvmva(1, 0, 0, 3, 0);
	out.x = MFC2_S(25);
	out.y = MFC2_S(26);
	out.z = MFC2_S(27);

	return out;
}

static struct MatrixND *VehPhysForce_TranslateMatrix_GetBakedEntry(u8 matrixArray, u8 matrixIndex)
{
	return &((struct MatrixND *)data.bakedGteMath[matrixArray].physEntry)[matrixIndex];
}

static u8 VehPhysForce_TranslateMatrix_RemapIndex(u8 matrixIndex, int fromArray, int toArray)
{
	int fromLast = CTR_MipsSubLo(data.bakedGteMath[fromArray].numEntries, 1);
	int blend = CTR_MipsSubLo(VEH_PHYS_FORCE_MATRIX_BLEND_FULL, CTR_MipsDiv(CTR_MipsSll(matrixIndex, 8), fromLast));

	if (blend < 0)
	{
		blend = 0;
	}
	if (blend > VEH_PHYS_FORCE_MATRIX_BLEND_FULL)
	{
		blend = VEH_PHYS_FORCE_MATRIX_BLEND_FULL;
	}

	return (u8)CTR_MipsSra(CTR_MipsMulLo(blend, CTR_MipsSubLo(data.bakedGteMath[toArray].numEntries, 1)), 8);
}

static void VehPhysForce_TranslateMatrix_ResetMatrixAnim(struct Driver *d)
{
	d->matrixArray = BAKED_GTE_MATRIX_NONE;
	d->matrixIndex = 0;
}

static void VehPhysForce_TranslateMatrix_UpdateSquashStretch(struct Instance *inst, struct Driver *d)
{
	if (d->kartState == KS_WARP_PAD)
	{
		return;
	}

	if ((d->kartState == KS_MASK_GRABBED) && ((d->actionsFlagSet & ACTION_TOUCH_GROUND) == 0))
	{
		int scaleXZ;

		inst->scale.y = (s16)CTR_MipsAddLo((u16)d->jumpSquishStretch, VEH_PHYS_FORCE_KART_SCALE_BASE);

		scaleXZ = CTR_MipsSubLo(VEH_PHYS_FORCE_KART_SCALE_BASE,
		                        VehPhysForce_TranslateMatrix_Div256TowardZero(CTR_MipsMulLo(d->jumpSquishStretch, VEH_PHYS_FORCE_MASK_GRAB_SCALE_XZ_FACTOR)));
		if (scaleXZ < VEH_PHYS_FORCE_MIN_SQUASH_XZ_SCALE)
		{
			scaleXZ = VEH_PHYS_FORCE_MIN_SQUASH_XZ_SCALE;
		}

		inst->scale.x = scaleXZ;
		inst->scale.z = scaleXZ;
		return;
	}

	int jumpHeightCurr = d->jumpHeightCurr;
	int targetSquish = VEH_PHYS_FORCE_TARGET_SQUISH_DEFAULT;

	if ((d->actionsFlagSet & ACTION_JUMP_STARTED) == 0)
	{
		int smoothed = CTR_MipsSra(CTR_MipsAddLo(CTR_MipsMulLo(d->jumpSquishStretch2, 9), CTR_MipsMulLo(jumpHeightCurr, 7)), 4);
		int delta = CTR_MipsSll(CTR_MipsSubLo(d->jumpSquishStretch2, smoothed), 2);

		if (VehPhysForce_TranslateMatrix_Abs(delta) < VEH_PHYS_FORCE_SQUISH_DEADBAND)
		{
			delta = 0;
		}

		if (((d->actionsFlagSet | d->actionsFlagSetPrevFrame) & ACTION_STARTED_TOUCH_GROUND) == 0)
		{
			if (delta < VEH_PHYS_FORCE_AIR_SQUISH_MIN)
			{
				delta = VEH_PHYS_FORCE_AIR_SQUISH_MIN;
			}
		}
		else if (delta < VEH_PHYS_FORCE_LANDING_SQUISH_MIN)
		{
			delta = VEH_PHYS_FORCE_LANDING_SQUISH_MIN;
		}

		if (delta > VEH_PHYS_FORCE_SQUISH_MAX)
		{
			delta = VEH_PHYS_FORCE_SQUISH_MAX;
		}

		targetSquish = delta;
	}

	if ((d->hazardTimer > 0) && ((d->hazardTimer & VEH_PHYS_FORCE_HAZARD_BLINK_MASK) == 0) && (targetSquish > VEH_PHYS_FORCE_TARGET_SQUISH_DEFAULT))
	{
		targetSquish = VEH_PHYS_FORCE_TARGET_SQUISH_DEFAULT;
	}

	if (((d->actionsFlagSet & ACTION_TOUCH_GROUND) == 0) && (jumpHeightCurr < 0))
	{
		int mapped = VehCalc_MapToRange(CTR_MipsNegLo(jumpHeightCurr), 0, VEH_PHYS_FORCE_FALL_STRETCH_HEIGHT_MAX, VEH_PHYS_FORCE_FALL_STRETCH_MIN,
		                                VEH_PHYS_FORCE_FALL_STRETCH_MAX);

		if (targetSquish < mapped)
		{
			targetSquish = mapped;
		}

		d->jumpSquishStretch2 = jumpHeightCurr;
	}

	if ((d->instTntRecv != NULL) && (d->instTntRecv->scale.y < VEH_PHYS_FORCE_TNT_SCALE_Y_THRESHOLD))
	{
		targetSquish = CTR_MipsAddLo(targetSquish, CTR_MipsSll(CTR_MipsSubLo(d->instTntRecv->scale.y, VEH_PHYS_FORCE_TNT_SCALE_Y_BASE), 1));
	}

	if (VehPhysForce_TranslateMatrix_Abs(d->jumpSquishStretch) < VehPhysForce_TranslateMatrix_Abs(targetSquish))
	{
		d->jumpSquishStretch = targetSquish;
	}

	d->jumpSquishStretch = VehCalc_InterpBySpeed(d->jumpSquishStretch, VEH_PHYS_FORCE_SQUISH_INTERP_SPEED, 0);
	d->jumpSquishStretch2 = (s16)CTR_MipsSra(CTR_MipsAddLo(CTR_MipsMulLo(d->jumpSquishStretch2, 9), CTR_MipsMulLo(jumpHeightCurr, 7)), 4);

	if (d->squishTimer != 0)
	{
		inst->scale.y = 0;
	}
	else if (inst->scale.y == 0)
	{
		if (d->instSelf->thread->modelIndex == DYNAMIC_PLAYER)
		{
			OtherFX_Play_Echo(VEH_PHYS_FORCE_SQUISH_RESTORE_SFX, 1, (d->actionsFlagSet & ACTION_ENGINE_ECHO) != 0);
		}

		inst->scale.y = (s16)CTR_MipsAddLo((u16)d->jumpSquishStretch, VEH_PHYS_FORCE_KART_SCALE_BASE);
		d->matrixArray = BAKED_GTE_MATRIX_SQUISH_RECOVER;
		d->matrixIndex = 0;
	}
	else
	{
		inst->scale.y =
		    VehCalc_InterpBySpeed(inst->scale.y, VEH_PHYS_FORCE_SQUISH_SCALE_INTERP_SPEED, CTR_MipsAddLo(d->jumpSquishStretch, VEH_PHYS_FORCE_KART_SCALE_BASE));
	}

	int scaleXZ = CTR_MipsSubLo(VEH_PHYS_FORCE_KART_SCALE_BASE,
	                            VehPhysForce_TranslateMatrix_Div256TowardZero(CTR_MipsMulLo(d->jumpSquishStretch, VEH_PHYS_FORCE_SQUISH_SCALE_XZ_FACTOR)));
	inst->scale.x = VehCalc_InterpBySpeed(inst->scale.x, VEH_PHYS_FORCE_SQUISH_SCALE_INTERP_SPEED, scaleXZ);
	inst->scale.z = VehCalc_InterpBySpeed(inst->scale.z, VEH_PHYS_FORCE_SQUISH_SCALE_INTERP_SPEED, scaleXZ);
}

static void VehPhysForce_TranslateMatrix_UpdateMatrixAnimation(struct Driver *d)
{
	if ((d->reserves == 0) || (d->fireSpeed < d->const_Speed_ClassStat) || ((d->actionsFlagSet & ACTION_TURBO_INPUT_LATCH) != 0))
	{
		if (d->matrixArray == BAKED_GTE_MATRIX_WHEELIE_HOLD)
		{
			d->matrixArray = BAKED_GTE_MATRIX_WHEELIE_RECOVER;
			d->matrixIndex = 0;
		}
		else if (d->matrixArray == BAKED_GTE_MATRIX_WHEELIE_START)
		{
			d->matrixIndex = VehPhysForce_TranslateMatrix_RemapIndex(d->matrixIndex, BAKED_GTE_MATRIX_WHEELIE_START, BAKED_GTE_MATRIX_WHEELIE_RECOVER);
			d->matrixArray = BAKED_GTE_MATRIX_WHEELIE_RECOVER;
		}
		else if (d->matrixArray == BAKED_GTE_MATRIX_WHEELIE_RECOVER)
		{
			d->matrixIndex++;
			if (d->matrixIndex >= data.bakedGteMath[BAKED_GTE_MATRIX_WHEELIE_RECOVER].numEntries)
			{
				VehPhysForce_TranslateMatrix_ResetMatrixAnim(d);
			}
		}
	}
	else
	{
		if (d->matrixArray == BAKED_GTE_MATRIX_WHEELIE_START)
		{
			d->matrixIndex++;
			if (d->matrixIndex >= data.bakedGteMath[BAKED_GTE_MATRIX_WHEELIE_START].numEntries)
			{
				d->matrixArray = BAKED_GTE_MATRIX_WHEELIE_HOLD;
				d->matrixIndex = 0;
			}
		}
		else if (d->matrixArray == BAKED_GTE_MATRIX_WHEELIE_RECOVER)
		{
			d->matrixIndex = VehPhysForce_TranslateMatrix_RemapIndex(d->matrixIndex, BAKED_GTE_MATRIX_WHEELIE_RECOVER, BAKED_GTE_MATRIX_WHEELIE_START);
			d->matrixArray = BAKED_GTE_MATRIX_WHEELIE_START;
		}
		else if (d->matrixArray == BAKED_GTE_MATRIX_NONE)
		{
			d->matrixArray = BAKED_GTE_MATRIX_WHEELIE_START;
			d->matrixIndex = 0;
		}
	}

	if (d->matrixArray == BAKED_GTE_MATRIX_SQUISH_RECOVER)
	{
		d->matrixIndex++;
		if (d->matrixIndex >= data.bakedGteMath[BAKED_GTE_MATRIX_SQUISH_RECOVER].numEntries)
		{
			VehPhysForce_TranslateMatrix_ResetMatrixAnim(d);
		}
	}
}

static void VehPhysForce_TranslateMatrix_CopyFacingMatrix(struct Instance *inst, struct Driver *d)
{
	inst->matrix.m[0][0] = d->matrixFacingDir.m[0][0];
	inst->matrix.m[0][1] = d->matrixFacingDir.m[0][1];
	inst->matrix.m[0][2] = d->matrixFacingDir.m[0][2];
	inst->matrix.m[1][0] = d->matrixFacingDir.m[1][0];
	inst->matrix.m[1][1] = d->matrixFacingDir.m[1][1];
	inst->matrix.m[1][2] = d->matrixFacingDir.m[1][2];
	inst->matrix.m[2][0] = d->matrixFacingDir.m[2][0];
	inst->matrix.m[2][1] = d->matrixFacingDir.m[2][1];
	inst->matrix.m[2][2] = d->matrixFacingDir.m[2][2];
}

static void VehPhysForce_TranslateMatrix_UpdateInstanceMatrix(struct Instance *inst, struct Driver *d)
{
	int screenOffsetY = CTR_MipsSra(CTR_MipsMulLo((s8)d->Screen_OffsetY, 3), 3);

	if (d->matrixArray == BAKED_GTE_MATRIX_NONE)
	{
		VehPhysForce_TranslateMatrix_CopyFacingMatrix(inst, d);

		inst->matrix.t[0] = CTR_MipsSra(d->posCurr.x, 8);
		inst->matrix.t[1] = CTR_MipsAddLo(CTR_MipsSra(d->posCurr.y, 8), screenOffsetY);
		inst->matrix.t[2] = CTR_MipsSra(d->posCurr.z, 8);
	}
	else
	{
		struct MatrixND *entry = VehPhysForce_TranslateMatrix_GetBakedEntry(d->matrixArray, d->matrixIndex);
		s16 *entryVec = (s16 *)entry;
		Vec3 rotated;

		MatrixNDOverlapMatrix *matrix = MatrixND_GetOverlapMatrix(entry);
		MatrixRotate(&inst->matrix, &d->matrixFacingDir, (MATRIX *)matrix);

		rotated = VehPhysForce_TranslateMatrix_RotateVector(&d->matrixFacingDir, entryVec[0], entryVec[1], entryVec[2]);

		inst->matrix.t[0] = CTR_MipsSra(CTR_MipsAddLo(d->posCurr.x, rotated.x), 8);
		inst->matrix.t[1] = CTR_MipsAddLo(CTR_MipsSra(CTR_MipsAddLo(d->posCurr.y, rotated.y), 8), screenOffsetY);
		inst->matrix.t[2] = CTR_MipsSra(CTR_MipsAddLo(d->posCurr.z, rotated.z), 8);
	}

	if (d->squishTimer != 0)
	{
		inst->matrix.t[0] =
		    CTR_MipsAddLo(inst->matrix.t[0], CTR_MipsSra(CTR_MipsMulLo(d->AxisAngle2_normalVec.x, VEH_PHYS_FORCE_SQUISH_OFFSET_NORMAL_SCALE), 12));
		inst->matrix.t[1] =
		    CTR_MipsAddLo(inst->matrix.t[1], CTR_MipsSra(CTR_MipsMulLo(d->AxisAngle2_normalVec.y, VEH_PHYS_FORCE_SQUISH_OFFSET_NORMAL_SCALE), 12));
		inst->matrix.t[2] =
		    CTR_MipsAddLo(inst->matrix.t[2], CTR_MipsSra(CTR_MipsMulLo(d->AxisAngle2_normalVec.z, VEH_PHYS_FORCE_SQUISH_OFFSET_NORMAL_SCALE), 12));
	}
}

static void VehPhysForce_TranslateMatrix_HideWake(struct Instance *inst, struct Driver *d)
{
	struct Instance *wake = d->wakeInst;

	if (inst->matrix.t[1] > 0)
	{
		inst->flags &= ~SPLIT_LINE;
	}

	if (wake != NULL)
	{
		wake->flags |= HIDE_MODEL;
		d->wakeScale = 0;
		wake->scale.x = 0;
		wake->scale.z = d->wakeScale;
	}
}

static void VehPhysForce_TranslateMatrix_SpawnWakeParticle(struct Driver *d)
{
	struct Particle *p = Particle_Init(0, sdata->gGT->iconGroup[VEH_PHYS_FORCE_WAKE_PARTICLE_ICON_GROUP], &data.emSet_Falling[0]);

	if (p != NULL)
	{
		p->otIndexOffset = d->instSelf->depthBiasNormal;
		p->driverInst = d->instSelf;
		p->driverID = d->driverID;
	}
}

static void VehPhysForce_TranslateMatrix_SetWakeRotation(struct Instance *wake, struct Driver *d)
{
	s32 sin = MATH_Sin(d->angle);
	s32 cos = MATH_Cos(d->angle);
	s32 negSin = CTR_MipsNegLo(sin);

	// Retail writes paired matrix shorts with word stores.
	wake->matrix.m[0][0] = (s16)cos;
	wake->matrix.m[0][1] = (s16)((u32)cos >> 16);
	wake->matrix.m[0][2] = (s16)sin;
	wake->matrix.m[1][0] = (s16)((u32)sin >> 16);
	wake->matrix.m[1][1] = VEH_PHYS_FORCE_WAKE_INITIAL_SCALE;
	wake->matrix.m[1][2] = 0;
	wake->matrix.m[2][0] = (s16)negSin;
	wake->matrix.m[2][1] = (s16)((u32)negSin >> 16);
	wake->matrix.m[2][2] = (s16)cos;
}

static void VehPhysForce_TranslateMatrix_UpdateWake(struct Instance *inst, struct Driver *d)
{
	struct Instance *wake;

	if (inst->matrix.t[1] >= VEH_PHYS_FORCE_WAKE_WATERLINE_Y)
	{
		VehPhysForce_TranslateMatrix_HideWake(inst, d);
		return;
	}

	if ((inst->matrix.t[1] < VEH_PHYS_FORCE_WAKE_VISIBLE_MIN_Y) || ((inst->flags & SPLIT_LINE) == 0))
	{
		VehPhysForce_TranslateMatrix_HideWake(inst, d);
		return;
	}

	wake = d->wakeInst;
	if (wake == NULL)
	{
		return;
	}

	wake->flags &= ~HIDE_MODEL;
	wake->depthBiasNormal = (u8)CTR_MipsAddLo(inst->depthBiasNormal, 1);
	wake->depthBiasSecondary = (u8)CTR_MipsSubLo(inst->depthBiasSecondary, 1);

	wake->matrix.t[0] = inst->matrix.t[0];
	wake->matrix.t[1] = VEH_PHYS_FORCE_WAKE_WATERLINE_Y;
	wake->matrix.t[2] = inst->matrix.t[2];

	VehPhysForce_TranslateMatrix_SetWakeRotation(wake, d);

	if (d->wakeScale == 0)
	{
		d->wakeScale = VEH_PHYS_FORCE_WAKE_INITIAL_SCALE;

		if (sdata->gGT->numPlyrCurrGame < 2)
		{
			if ((VehPhysForce_TranslateMatrix_Abs(d->speed) > VEH_PHYS_FORCE_WAKE_PARTICLE_SPEED_MIN) &&
			    (d->posPrev.y > VEH_PHYS_FORCE_WAKE_PARTICLE_PREV_Y_MIN))
			{
				int i;

				for (i = VEH_PHYS_FORCE_WAKE_BURST_PARTICLE_COUNT; i > 0; i--)
				{
					VehPhysForce_TranslateMatrix_SpawnWakeParticle(d);
				}
			}
		}
	}
	else if (sdata->gGT->numPlyrCurrGame < 2)
	{
		if (VehPhysForce_TranslateMatrix_Abs(d->speed) > VEH_PHYS_FORCE_WAKE_PARTICLE_SPEED_MIN)
		{
			VehPhysForce_TranslateMatrix_SpawnWakeParticle(d);
		}
	}

	wake->scale.x = d->wakeScale;
	wake->scale.z = d->wakeScale;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005ee34-0x8005f89c
void VehPhysForce_TranslateMatrix(struct Thread *thread, struct Driver *driver)
{
	struct Instance *inst = thread->inst;

	VehPhysForce_TranslateMatrix_UpdateSquashStretch(inst, driver);
	VehPhysForce_RotAxisAngle(&driver->matrixFacingDir, driver->AxisAngle2_normalVec.v, driver->rotCurr.y);
	VehPhysForce_TranslateMatrix_UpdateMatrixAnimation(driver);
	VehPhysForce_TranslateMatrix_UpdateInstanceMatrix(inst, driver);
	VehPhysForce_TranslateMatrix_UpdateWake(inst, driver);
}

static int VehPhysForce_CountLeadingSignBits(s32 value)
{
	u32 bits = (u32)value;
	u32 sign = bits >> 31;
	int count = 0;

	while (count < 32 && (((bits >> (31 - count)) & 1) == sign))
	{
		count++;
	}

	return count;
}

static struct TrigPair VehPhysForce_TrigAngleSinCos(int angle)
{
	struct TrigTable trig = data.trigApprox[ANG_MODULO_HALF_PI(angle)];
	u32 packed = ((u32)(u16)trig.sin) | ((u32)(u16)trig.cos << 16);
	struct TrigPair pair;

	if (IS_ANG_FIRST_OR_THIRD_QUADRANT(angle))
	{
		pair.sin = (s16)packed;
		pair.cos = (s16)(packed >> 16);

		if (IS_ANG_THIRD_OR_FOURTH_QUADRANT(angle))
		{
			pair.sin = CTR_MipsNegLo(pair.sin);
			pair.cos = CTR_MipsNegLo(pair.cos);
		}
	}
	else
	{
		pair.sin = (s16)(packed >> 16);
		pair.cos = (s16)packed;

		if (IS_ANG_THIRD_OR_FOURTH_QUADRANT(angle))
		{
			pair.sin = CTR_MipsNegLo(pair.sin);
		}
		else
		{
			pair.cos = CTR_MipsNegLo(pair.cos);
		}
	}

	return pair;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005f89c-0x8005fb4c.
void VehPhysForce_RotAxisAngle(MATRIX *m, s16 *normVec, s16 angle)
{
	s32 normalX = normVec[0];
	s32 normalY = normVec[1];
	s32 normalZ = normVec[2];
	struct TrigPair trig = VehPhysForce_TrigAngleSinCos(angle);
	s32 normalXSq = CTR_MipsMulLo(normalX, normalX);
	s32 normalZSq = CTR_MipsMulLo(normalZ, normalZ);
	s32 crossXZ = CTR_MipsMulLo(normalX, CTR_MipsNegLo(normalZ));
	s32 denom = CTR_MipsAddLo(normalXSq, normalZSq);
	s32 scaledSinY = CTR_MipsSra(CTR_MipsMulLo(trig.sin, normalY), 12);
	s32 scaledCosY = CTR_MipsSra(CTR_MipsMulLo(trig.cos, normalY), 12);
	s32 outX = scaledSinY;
	s32 outY;
	s32 outZ = scaledCosY;

	m->m[0][1] = (s16)normalX;
	m->m[1][1] = (s16)normalY;
	m->m[2][1] = (s16)normalZ;

	if (denom == 0)
	{
		s32 dot = CTR_MipsAddLo(CTR_MipsMulLo(trig.sin, normalX), CTR_MipsMulLo(trig.cos, normalZ));

		if (normalY < 0)
		{
			outX = CTR_MipsNegLo(outX);
		}

		outY = CTR_MipsSra(CTR_MipsNegLo(dot), 12);
	}
	else
	{
		int shift = CTR_MipsSubLo(0x14, VehPhysForce_CountLeadingSignBits(denom));
		s32 sinRemainder;
		s32 cosRemainder;
		s32 divX;
		s32 divZ;
		s32 dot;

		if (shift > 0)
		{
			normalXSq = CTR_MipsSra(normalXSq, shift);
			normalZSq = CTR_MipsSra(normalZSq, shift);
			crossXZ = CTR_MipsSra(crossXZ, shift);
			denom = CTR_MipsSra(denom, shift);
		}

		sinRemainder = CTR_MipsSubLo(trig.sin, scaledSinY);
		cosRemainder = CTR_MipsSubLo(trig.cos, scaledCosY);
		divX = CTR_MipsDiv(CTR_MipsAddLo(CTR_MipsMulLo(sinRemainder, normalZSq), CTR_MipsMulLo(cosRemainder, crossXZ)), denom);
		divZ = CTR_MipsDiv(CTR_MipsAddLo(CTR_MipsMulLo(sinRemainder, crossXZ), CTR_MipsMulLo(cosRemainder, normalXSq)), denom);
		dot = CTR_MipsAddLo(CTR_MipsMulLo(trig.sin, normalX), CTR_MipsMulLo(trig.cos, normalZ));

		outX = CTR_MipsAddLo(outX, divX);
		outY = CTR_MipsSra(CTR_MipsNegLo(dot), 12);
		outZ = CTR_MipsAddLo(outZ, divZ);
	}

	m->m[0][2] = (s16)outX;
	m->m[1][2] = (s16)outY;
	m->m[2][2] = (s16)outZ;

	// NOTE(aalhendi): Retail uses GTE `OP(sf=12)` with the normal vector and the generated
	// third column to derive the first matrix column. The partial register
	// writes are intentional; callers observe this exact GTE state.
	CTC2((u32)normalX, 0);
	CTC2((u32)normalY, 2);
	CTC2((u32)normalZ, 4);
	MTC2((u32)(s32)m->m[0][2], 9);
	MTC2((u32)(s32)m->m[1][2], 10);
	MTC2((u32)(s32)m->m[2][2], 11);
	gte_op12();
	m->m[0][0] = (s16)MFC2_S(25);
	m->m[1][0] = (s16)MFC2_S(26);
	m->m[2][0] = (s16)MFC2_S(27);
}

static SVec3 VehPhysForce_CounterSteer_RotateVector(const MATRIX *m, s16 vx, s16 vy, s16 vz)
{
	SVec3 out;
	(void)m;

	// NOTE(aalhendi): PhysTerrainSlope already loaded matrixMovingDir into GTE
	// rotation regs, matching retail before VehPhysForce_CounterSteer runs.
	MTC2(CTR_PackS16Pair(vx, vy), 0);
	MTC2((u32)(u16)vz, 1);
	gte_mvmva(1, 0, 0, 3, 0);
	out.x = (s16)MFC2(25);
	out.y = (s16)MFC2(26);
	out.z = (s16)MFC2(27);

	return out;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005fb4c-0x8005fc8c.
void VehPhysForce_CounterSteer(struct Driver *driver)
{
	driver->accel.x = 0;
	driver->accel.y = 0;
	driver->accel.z = 0;

	int speedApprox = driver->speedApprox;
	if (speedApprox < 0)
	{
		speedApprox = CTR_MipsNegLo(speedApprox);
	}

	if (speedApprox <= FP8(3) || driver->actionsFlagSet & ACTION_WARP || driver->kartState == KS_CRASHING || driver->wallRubTimer ||
	    !(driver->actionsFlagSet & ACTION_TOUCH_GROUND) || driver->terrainMeta1->counterSteerRatio == 0)
	{
		return;
	}

	int angleLimit = (u8)driver->const_ModelTurnCounterSteerStrength;
	int angle = CTR_MipsSubLo(driver->turnAngleCurr, driver->turnAnglePrev);
	if (angle > angleLimit)
	{
		angle = angleLimit;
	}
	else if (angle < CTR_MipsNegLo(angleLimit))
	{
		angle = CTR_MipsNegLo(angleLimit);
	}

	int sine = MATH_Sin(angle);
	int counterSteerStrength = CTR_MipsSra(CTR_MipsMulLo(driver->terrainMeta1->counterSteerRatio, -8000), 8);
	SVec3 accel = VehPhysForce_CounterSteer_RotateVector(&driver->matrixMovingDir, (s16)CTR_MipsSra(CTR_MipsMulLo(counterSteerStrength, sine), 12), 0, 0);

	driver->accel = accel;
}
