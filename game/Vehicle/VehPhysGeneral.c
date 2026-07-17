#include <common.h>

enum
{
	VEH_PHYS_ANGULAR_STICK_MIN_SPEED = 0x10,
	VEH_PHYS_ANGULAR_STEER_SPEED_THRESHOLD = 0x300,
	VEH_PHYS_ANGULAR_STEER_ACCEL_COMPARE_SPEED = 0x2ff,
	VEH_PHYS_ANGULAR_TURN_RESPONSE_COAST_SCALE = 0x32,
	VEH_PHYS_ANGULAR_TURN_RESPONSE_ACCEL_SCALE = 100,
	VEH_PHYS_ANGULAR_TURN_RESPONSE_DECEL_SCALE = 50,
	VEH_PHYS_ANGULAR_DRIFT_SPINOUT_TIME = 0x140,
	VEH_PHYS_ANGULAR_CLASS_SPEED_SHIFT = 0x10,
	VEH_PHYS_ANGULAR_CLASS_SPEED_HALF_SHIFT = 0x11,
	VEH_PHYS_ANGULAR_TERRAIN_SCALE_NEUTRAL = 0x100,
	VEH_PHYS_ANGULAR_TURN_ASSIST_MIN_DELTA = 3,
	VEH_PHYS_ANGULAR_TURN_WOBBLE_MIN_DELTA = 10,
	VEH_PHYS_ANGULAR_TURN_WOBBLE_TIMER = 8,
	VEH_PHYS_ANGULAR_TURN_WOBBLE_VELOCITY = 0x14,
	VEH_PHYS_ANGULAR_TURN_WOBBLE_DISABLE_ANGLE = 0x32,
	VEH_PHYS_ANGULAR_AIR_TURN_SPEED_MAX = 0x600,
	VEH_PHYS_ANGULAR_ANGLE_MASK = 0xfff,
	VEH_PHYS_ANGULAR_TURN_INTEGRATION_SHIFT = 5,
	VEH_PHYS_ANGULAR_AXIS_INTEGRATION_SHIFT = 0xd,
	VEH_PHYS_ANGULAR_BRAKE_LEAN_SCALE = 10,

	VEH_PHYS_JUMP_NORMAL_Y_MIN = 0x15,
	VEH_PHYS_JUMP_TURBO_PAD_ACCEL = 8000,
	VEH_PHYS_JUMP_REVERSE_SLIDE_SPEED_COMPARE = 0x2ff,
	VEH_PHYS_JUMP_TERRAIN_SCALE_NEUTRAL = 0x100,
	VEH_PHYS_JUMP_FAST_SQRT_ITERATIONS = 0x10,
	VEH_PHYS_JUMP_SPEED_FIXED_SHIFT = 8,
	VEH_PHYS_JUMP_HIGH_TIMER_MS = 0x180,
	VEH_PHYS_JUMP_FORCED_MS = 0xa0,
	VEH_PHYS_JUMP_COOLDOWN_MS = 0x180,
	VEH_PHYS_JUMP_RUMBLE_CHANNEL = 8,
	VEH_PHYS_JUMP_RUMBLE_FORCE = 0x7f,
	VEH_PHYS_JUMP_SPRING_SFX = 9,
	VEH_PHYS_JUMP_NORMAL_SFX = 8,
	VEH_PHYS_JUMP_FORCED_SFX = 0x7e,
	VEH_PHYS_JUMP_VERTICAL_SPEED_DEFAULT = 0x3700,
	VEH_PHYS_JUMP_VERTICAL_SPEED_MAX = 0x5000,
	VEH_PHYS_JUMP_SPEEDOMETER_REVERSE_THRESHOLD = 0x100,
	VEH_PHYS_JUMP_SPEEDOMETER_DECAY_SHIFT = 3,
	VEH_PHYS_JUMP_SPEEDOMETER_BLEND_OLD = 0xd,
	VEH_PHYS_JUMP_SPEEDOMETER_TIMER_MASK = 7,
	VEH_PHYS_JUMP_SPEEDOMETER_TIMER_SCALE = 0x300,
	VEH_PHYS_JUMP_SPEEDOMETER_BLEND_SHIFT = 4,
	VEH_PHYS_JUMP_SPEEDOMETER_BLEND_NEW = 3,
};

CTR_STATIC_ASSERT(VEH_PHYS_ANGULAR_STICK_MIN_SPEED == 0x10);
CTR_STATIC_ASSERT(VEH_PHYS_ANGULAR_STEER_SPEED_THRESHOLD == 0x300);
CTR_STATIC_ASSERT(VEH_PHYS_ANGULAR_STEER_ACCEL_COMPARE_SPEED == 0x2ff);
CTR_STATIC_ASSERT(VEH_PHYS_ANGULAR_TURN_RESPONSE_COAST_SCALE == 0x32);
CTR_STATIC_ASSERT(VEH_PHYS_ANGULAR_TURN_RESPONSE_ACCEL_SCALE == 100);
CTR_STATIC_ASSERT(VEH_PHYS_ANGULAR_TURN_RESPONSE_DECEL_SCALE == 50);
CTR_STATIC_ASSERT(VEH_PHYS_ANGULAR_DRIFT_SPINOUT_TIME == 0x140);
CTR_STATIC_ASSERT(VEH_PHYS_ANGULAR_CLASS_SPEED_SHIFT == 0x10);
CTR_STATIC_ASSERT(VEH_PHYS_ANGULAR_CLASS_SPEED_HALF_SHIFT == 0x11);
CTR_STATIC_ASSERT(VEH_PHYS_ANGULAR_TERRAIN_SCALE_NEUTRAL == 0x100);
CTR_STATIC_ASSERT(VEH_PHYS_ANGULAR_TURN_ASSIST_MIN_DELTA == 3);
CTR_STATIC_ASSERT(VEH_PHYS_ANGULAR_TURN_WOBBLE_MIN_DELTA == 10);
CTR_STATIC_ASSERT(VEH_PHYS_ANGULAR_TURN_WOBBLE_TIMER == 8);
CTR_STATIC_ASSERT(VEH_PHYS_ANGULAR_TURN_WOBBLE_VELOCITY == 0x14);
CTR_STATIC_ASSERT(VEH_PHYS_ANGULAR_TURN_WOBBLE_DISABLE_ANGLE == 0x32);
CTR_STATIC_ASSERT(VEH_PHYS_ANGULAR_AIR_TURN_SPEED_MAX == 0x600);
CTR_STATIC_ASSERT(VEH_PHYS_ANGULAR_ANGLE_MASK == 0xfff);
CTR_STATIC_ASSERT(VEH_PHYS_ANGULAR_TURN_INTEGRATION_SHIFT == 5);
CTR_STATIC_ASSERT(VEH_PHYS_ANGULAR_AXIS_INTEGRATION_SHIFT == 0xd);
CTR_STATIC_ASSERT(VEH_PHYS_ANGULAR_BRAKE_LEAN_SCALE == 10);
CTR_STATIC_ASSERT(VEH_PHYS_JUMP_NORMAL_Y_MIN == 0x15);
CTR_STATIC_ASSERT(VEH_PHYS_JUMP_TURBO_PAD_ACCEL == 8000);
CTR_STATIC_ASSERT(VEH_PHYS_JUMP_REVERSE_SLIDE_SPEED_COMPARE == 0x2ff);
CTR_STATIC_ASSERT(VEH_PHYS_JUMP_TERRAIN_SCALE_NEUTRAL == 0x100);
CTR_STATIC_ASSERT(VEH_PHYS_JUMP_FAST_SQRT_ITERATIONS == 0x10);
CTR_STATIC_ASSERT(VEH_PHYS_JUMP_SPEED_FIXED_SHIFT == 8);
CTR_STATIC_ASSERT(VEH_PHYS_JUMP_HIGH_TIMER_MS == 0x180);
CTR_STATIC_ASSERT(VEH_PHYS_JUMP_FORCED_MS == 0xa0);
CTR_STATIC_ASSERT(VEH_PHYS_JUMP_COOLDOWN_MS == 0x180);
CTR_STATIC_ASSERT(VEH_PHYS_JUMP_RUMBLE_CHANNEL == 8);
CTR_STATIC_ASSERT(VEH_PHYS_JUMP_RUMBLE_FORCE == 0x7f);
CTR_STATIC_ASSERT(VEH_PHYS_JUMP_SPRING_SFX == 9);
CTR_STATIC_ASSERT(VEH_PHYS_JUMP_NORMAL_SFX == 8);
CTR_STATIC_ASSERT(VEH_PHYS_JUMP_FORCED_SFX == 0x7e);
CTR_STATIC_ASSERT(VEH_PHYS_JUMP_VERTICAL_SPEED_DEFAULT == 0x3700);
CTR_STATIC_ASSERT(VEH_PHYS_JUMP_VERTICAL_SPEED_MAX == 0x5000);
CTR_STATIC_ASSERT(VEH_PHYS_JUMP_SPEEDOMETER_REVERSE_THRESHOLD == 0x100);
CTR_STATIC_ASSERT(VEH_PHYS_JUMP_SPEEDOMETER_DECAY_SHIFT == 3);
CTR_STATIC_ASSERT(VEH_PHYS_JUMP_SPEEDOMETER_BLEND_OLD == 0xd);
CTR_STATIC_ASSERT(VEH_PHYS_JUMP_SPEEDOMETER_TIMER_MASK == 7);
CTR_STATIC_ASSERT(VEH_PHYS_JUMP_SPEEDOMETER_TIMER_SCALE == 0x300);
CTR_STATIC_ASSERT(VEH_PHYS_JUMP_SPEEDOMETER_BLEND_SHIFT == 4);
CTR_STATIC_ASSERT(VEH_PHYS_JUMP_SPEEDOMETER_BLEND_NEW == 3);

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005fc8c-0x80060458.
void VehPhysGeneral_PhysAngular(struct Thread *thread, struct Driver *driver)
{
	(void)thread;
	int speedApprox;
	int elapsedTimeMS;
	int classSpeed_original;
	int driverSpeed;
	int classSpeed_halved;
	struct Terrain *terrain;
	int rotCurrW_original;
	int rotCurrWAbs;
	u16 angle;
	int turnResistMinBitshift;
	int turnResistMaxBitshift;
	int driftAngleCurr_Final;
	int turnResistMax;
	int turnResistMin;
	u32 actionsFlagSet;
	b32 interpLessThanOriginal;
	b32 wInterpLessThanZero;
	s16 forwardDir;
	int rotCurrW_interp;
	s8 simpTurnState;
	s16 driftAngleCurr_og;

	PhysLerpRot(driver, 0);

	elapsedTimeMS = sdata->gGT->elapsedTimeMS;
	actionsFlagSet = driver->actionsFlagSet;
	forwardDir = driver->forwardDir;
	simpTurnState = driver->simpTurnState;
	speedApprox = (int)driver->speedApprox;
	rotCurrW_interp = CTR_MipsSll(simpTurnState, 8);
	if (speedApprox < 1)
	{
		if (driver->baseSpeed < 0)
		{
			forwardDir = -1;
			driver->forwardDir = -1;
		}
		if (-1 < speedApprox)
		{
			goto LAB_8005fd74;
		}
	}
	else
	{
	LAB_8005fd74:
		if (-1 < driver->baseSpeed)
		{
			forwardDir = 1;
			driver->forwardDir = 1;
		}
	}
	if (forwardDir < 0)
	{
		rotCurrW_interp = CTR_MipsNegLo(CTR_MipsSll(simpTurnState, 8));
		actionsFlagSet ^= ACTION_STEER_LEFT;
	}
	if (speedApprox < 0)
	{
		speedApprox = CTR_MipsNegLo(speedApprox);
	}
	if (((actionsFlagSet & ACTION_TOUCH_GROUND) != 0) && ((driver->stepFlagSet & COLL_STEP_TRIGGER_TURBO_PAD_MASK) == 0))
	{
		rotCurrW_interp = VehCalc_MapToRange(speedApprox, VEH_PHYS_ANGULAR_STICK_MIN_SPEED, VEH_PHYS_ANGULAR_STEER_SPEED_THRESHOLD, 0, rotCurrW_interp);
	}
	terrain = driver->terrainMeta1;
	rotCurrW_original = (int)driver->rotationSpinRate;
	if (rotCurrW_interp == 0)
	{
		int rate = CTR_MipsSra(
		    CTR_MipsMulLo(CTR_MipsAddLo(driver->const_TurnInputDelay, CTR_MipsMulLo((s8)driver->turnConst, VEH_PHYS_ANGULAR_TURN_RESPONSE_COAST_SCALE)),
		                  terrain->turnResponseScale),
		    8);

		rotCurrW_interp = VehCalc_InterpBySpeed(rotCurrW_original, rate, 0);

		forwardDir = (s16)rotCurrW_interp;
	}
	else
	{
		wInterpLessThanZero = rotCurrW_interp < 0;
		if (wInterpLessThanZero)
		{
			rotCurrW_interp = CTR_MipsNegLo(rotCurrW_interp);
			rotCurrW_original = CTR_MipsNegLo(rotCurrW_original);
		}
		if (rotCurrW_original < rotCurrW_interp)
		{
			int rate = CTR_MipsSra(
			    CTR_MipsMulLo(CTR_MipsAddLo(driver->const_TurnInputDelay, CTR_MipsMulLo((s8)driver->turnConst, VEH_PHYS_ANGULAR_TURN_RESPONSE_ACCEL_SCALE)),
			                  terrain->turnResponseScale),
			    8);
			rotCurrW_original = CTR_MipsAddLo(rotCurrW_original, rate);

			interpLessThanOriginal = rotCurrW_interp < rotCurrW_original;
		LAB_8005fee4:
			if (interpLessThanOriginal)
			{
				rotCurrW_original = rotCurrW_interp;
			}
		}
		else if (rotCurrW_interp < rotCurrW_original)
		{
			int rate = CTR_MipsSra(
			    CTR_MipsMulLo(CTR_MipsAddLo(driver->const_TurnInputDelay, CTR_MipsMulLo((s8)driver->turnConst, VEH_PHYS_ANGULAR_TURN_RESPONSE_DECEL_SCALE)),
			                  terrain->turnResponseScale),
			    8);
			rotCurrW_original = CTR_MipsSubLo(rotCurrW_original, rate);

			interpLessThanOriginal = rotCurrW_original < rotCurrW_interp;
			goto LAB_8005fee4;
		}
		forwardDir = (s16)rotCurrW_original;
		if (wInterpLessThanZero)
		{
			forwardDir = (s16)CTR_MipsNegLo(forwardDir);
		}
	}

	rotCurrW_original = (int)forwardDir;
	driver->rotationSpinRate = forwardDir;

	rotCurrW_interp = (int)driver->timeUntilDriftSpinout;
	if (rotCurrW_interp != 0)
	{
		classSpeed_halved = CTR_MipsSubLo(rotCurrW_interp, elapsedTimeMS);
		rotCurrW_interp = VehCalc_MapToRange(rotCurrW_interp, 0, VEH_PHYS_ANGULAR_DRIFT_SPINOUT_TIME, 0, (int)driver->previousFrameMultDrift);
		rotCurrW_original = CTR_MipsAddLo(rotCurrW_original, rotCurrW_interp);
		if (classSpeed_halved < 0)
		{
			classSpeed_halved = 0;
		}
		driver->timeUntilDriftSpinout = (s16)classSpeed_halved;
	}

	classSpeed_halved = CTR_MipsSll((u16)driver->const_Speed_ClassStat, VEH_PHYS_ANGULAR_CLASS_SPEED_SHIFT);
	classSpeed_original = CTR_MipsSra(classSpeed_halved, VEH_PHYS_ANGULAR_CLASS_SPEED_SHIFT);
	turnResistMax = CTR_MipsMulLo((u8)driver->const_turnResistMax, classSpeed_original);
	turnResistMin = CTR_MipsMulLo((u8)driver->const_turnResistMin, classSpeed_original);
	forwardDir = driver->turnAngleLerpVel;
	rotCurrW_interp = (int)driver->const_modelRotVelMax;
	turnResistMaxBitshift = CTR_MipsSra(turnResistMax, 8);
	turnResistMinBitshift = CTR_MipsSra(turnResistMin, 8);

	// gas and brake together
	if ((actionsFlagSet & ACTION_BRAKE_WITH_ACCEL) != 0)
	{
		turnResistMaxBitshift = CTR_MipsSra(turnResistMax, 9);
		if (VEH_PHYS_ANGULAR_STEER_SPEED_THRESHOLD < speedApprox)
		{
			// driver is leaving skids
			driver->actionsFlagSet |= ACTION_BACK_SKID;
		}
		turnResistMinBitshift = CTR_MipsSra(turnResistMin, 9);
		if (driver->baseSpeed == 0)
		{
			rotCurrW_interp = (int)driver->const_modelRotVelMin;
		}
		else
		{
			turnResistMax = (int)driver->speed;
			if (turnResistMax < 0)
			{
				turnResistMax = CTR_MipsNegLo(turnResistMax);
			}
			// Rotate the model to exaggerate steering above the steering speed threshold.
			rotCurrW_interp =
			    VehCalc_MapToRange(turnResistMax, VEH_PHYS_ANGULAR_STEER_SPEED_THRESHOLD,
			                       CTR_MipsSra(classSpeed_halved, VEH_PHYS_ANGULAR_CLASS_SPEED_HALF_SHIFT), (int)driver->const_modelRotVelMin, rotCurrW_interp);
		}
	}
	driverSpeed = (int)driver->speed;
	if (driverSpeed < 0)
	{
		driverSpeed = CTR_MipsNegLo(driverSpeed);
	}

	// this prevents you from steering sharp at low speeds
	turnResistMin = CTR_MipsSll(CTR_MipsAddLo((u8)driver->const_TurnRate, CTR_MipsSll((s8)driver->turnConst, 1) / 5), 8);
	turnResistMax = VehCalc_MapToRange(driverSpeed, turnResistMinBitshift, turnResistMaxBitshift, turnResistMin, 0);

	classSpeed_halved = 0;
	if (turnResistMinBitshift <= speedApprox)
	{
		rotCurrWAbs = rotCurrW_original;
		if (rotCurrW_original < 0)
		{
			rotCurrWAbs = CTR_MipsNegLo(rotCurrW_original);
		}
		if (turnResistMax < rotCurrWAbs)
		{
			classSpeed_halved = (int)driver->fireSpeed;
			if (classSpeed_halved < 0)
			{
				classSpeed_halved = CTR_MipsNegLo(classSpeed_halved);
			}
			classSpeed_halved = VehCalc_MapToRange(classSpeed_halved, turnResistMinBitshift, turnResistMaxBitshift, 0, rotCurrW_interp);
			classSpeed_halved = VehCalc_MapToRange(rotCurrWAbs, turnResistMax, turnResistMin, 0, classSpeed_halved);
			if (rotCurrW_original < 0)
			{
				classSpeed_halved = CTR_MipsNegLo(classSpeed_halved);
			}
		}
	}

	driftAngleCurr_og = driver->turnAngleCurr;

	// spins camera from side of driver, to back of driver,
	// when the drifting ends. "LerpToForwards"
	driver->turnAngleLerpVel = VehPhysGeneral_LerpToForwards(driver, (int)driftAngleCurr_og, (int)forwardDir, classSpeed_halved);

	classSpeed_halved = (int)(s16)driver->turnAngleLerpVel;

	if (terrain->turnAngleScale != VEH_PHYS_ANGULAR_TERRAIN_SCALE_NEUTRAL)
	{
		classSpeed_halved = CTR_MipsSra(CTR_MipsMulLo(terrain->turnAngleScale, classSpeed_halved), 8);
	}
	driftAngleCurr_Final =
	    CTR_MipsAddLo(driftAngleCurr_og, CTR_MipsSra(CTR_MipsMulLo(classSpeed_halved, elapsedTimeMS), VEH_PHYS_ANGULAR_TURN_INTEGRATION_SHIFT));
	driver->turnAngleCurr = (s16)driftAngleCurr_Final;
	turnResistMinBitshift = rotCurrW_original;
	if ((VEH_PHYS_ANGULAR_STEER_ACCEL_COMPARE_SPEED < speedApprox) && ((actionsFlagSet & ACTION_TOUCH_GROUND) != 0))
	{
		turnResistMaxBitshift = VehCalc_SteerAccel(driver->numFramesSpentSteering, (int)driver->const_SteerAccel_Stage2_FirstFrame,
		                                           (int)driver->const_SteerAccel_Stage2_FrameLength, (int)driver->const_SteerAccel_Stage4_FirstFrame,
		                                           (int)driver->const_SteerAccel_Stage1_MinSteer, (int)driver->const_SteerAccel_Stage1_MaxSteer);
		if (rotCurrW_original < 0)
		{
			turnResistMinBitshift = CTR_MipsNegLo(rotCurrW_original);
		}

		turnResistMinBitshift = CTR_MipsSra(CTR_MipsMulLo(driver->const_SteerAccelTurnVelScale, turnResistMinBitshift), 8);

		driver->numFramesSpentSteering = (s16)CTR_MipsAddLo((u16)driver->numFramesSpentSteering, 1);

		// the higher the value of turnResistMaxBitshift the more steering is "locked up"
		// try setting mov r3, xxxx at 80060170 for proof
		if (turnResistMinBitshift < turnResistMaxBitshift)
		{
			turnResistMaxBitshift = turnResistMinBitshift;
		}

		// steering left or right
		if ((actionsFlagSet & ACTION_STEER_LEFT) != 0)
		{
			turnResistMaxBitshift = CTR_MipsNegLo(turnResistMaxBitshift);
		}

		turnResistMax = (int)driver->const_SteerAccelTurnVelLimit;

		if ((rotCurrW_original < 1) ||
		    (turnResistMinBitshift = CTR_MipsNegLo(turnResistMax), turnResistMinBitshift <= CTR_MipsAddLo(rotCurrW_original, turnResistMaxBitshift)))
		{
			if (rotCurrW_original < 0)
			{
				turnResistMinBitshift = CTR_MipsAddLo(rotCurrW_original, turnResistMaxBitshift);
				if (turnResistMax < CTR_MipsAddLo(rotCurrW_original, turnResistMaxBitshift))
				{
					turnResistMinBitshift = turnResistMax;
				}
			}
			else
			{
				turnResistMinBitshift = CTR_MipsAddLo(rotCurrW_original, turnResistMaxBitshift);
			}
		}
	}
	turnResistMax = (int)driver->turnWobbleAngle;
	turnResistMaxBitshift = (int)driver->turnWobbleTimer;
	rotCurrW_original = (int)driver->turnWobbleVelocity;
	if (((terrain->flags & TERRAIN_FLAG_SKIP_TURN_ASSIST) == 0) && ((actionsFlagSet & ACTION_TOUCH_GROUND) != 0))
	{
		turnResistMin = driftAngleCurr_Final;
		if (driftAngleCurr_Final < 0)
		{
			turnResistMin = CTR_MipsNegLo(driftAngleCurr_Final);
		}
		if (CTR_MipsSra(CTR_MipsAddLo(CTR_MipsSll(rotCurrW_interp, 1), rotCurrW_interp), 2) < turnResistMin)
		{
			rotCurrW_interp = classSpeed_halved;
			if (classSpeed_halved < 0)
			{
				rotCurrW_interp = CTR_MipsNegLo(classSpeed_halved);
			}
			if (rotCurrW_interp < VEH_PHYS_ANGULAR_TURN_ASSIST_MIN_DELTA)
			{
				rotCurrW_interp = turnResistMax;
				if (turnResistMax < 0)
				{
					rotCurrW_interp = CTR_MipsNegLo(turnResistMax);
				}
				if (rotCurrW_interp < VEH_PHYS_ANGULAR_TURN_WOBBLE_MIN_DELTA)
				{
					turnResistMaxBitshift = VEH_PHYS_ANGULAR_TURN_WOBBLE_TIMER;
					rotCurrW_original = VEH_PHYS_ANGULAR_TURN_WOBBLE_VELOCITY;
					if (driftAngleCurr_Final < 0)
					{
						rotCurrW_original = -VEH_PHYS_ANGULAR_TURN_WOBBLE_VELOCITY;
					}
				}
			}
			goto LAB_80060284;
		}
	}
	turnResistMaxBitshift = 0;
LAB_80060284:
	rotCurrW_interp = turnResistMax;
	if (turnResistMax < 0)
	{
		rotCurrW_interp = CTR_MipsNegLo(turnResistMax);
	}
	if (VEH_PHYS_ANGULAR_TURN_WOBBLE_DISABLE_ANGLE < rotCurrW_interp)
	{
		turnResistMaxBitshift = 0;
	}
	if (turnResistMaxBitshift == 0)
	{
		rotCurrW_original = VEH_PHYS_ANGULAR_TURN_WOBBLE_MIN_DELTA;
		if (0 < turnResistMax)
		{
			rotCurrW_original = CTR_MipsNegLo(VEH_PHYS_ANGULAR_TURN_WOBBLE_MIN_DELTA);
		}
		rotCurrW_interp = rotCurrW_original;
		if (rotCurrW_original < 0)
		{
			rotCurrW_interp = CTR_MipsNegLo(rotCurrW_original);
		}
		rotCurrW_interp = VehCalc_InterpBySpeed(turnResistMax, rotCurrW_interp, 0);
		forwardDir = (s16)rotCurrW_interp;
	}
	else
	{
		turnResistMaxBitshift = CTR_MipsSubLo(turnResistMaxBitshift, 1);
		forwardDir = (s16)CTR_MipsAddLo(driver->turnWobbleAngle, rotCurrW_original);
	}
	angle = driver->angle;
	driver->turnWobbleTimer = (s16)turnResistMaxBitshift;
	driver->turnWobbleAngle = forwardDir;
	driver->turnWobbleVelocity = (s16)rotCurrW_original;
	rotCurrW_interp = VehCalc_MapToRange(speedApprox, 0, VEH_PHYS_ANGULAR_AIR_TURN_SPEED_MAX, classSpeed_halved, 0);
	rotCurrW_original = CTR_MipsSra(CTR_MipsMulLo(rotCurrW_interp, elapsedTimeMS), VEH_PHYS_ANGULAR_TURN_INTEGRATION_SHIFT);
	rotCurrW_interp = rotCurrW_original;
	if (rotCurrW_original < 0)
	{
		rotCurrW_interp = CTR_MipsNegLo(rotCurrW_original);
	}
	if (1 < rotCurrW_interp)
	{
		angle = (u16)(CTR_MipsSubLo(angle, rotCurrW_original) & VEH_PHYS_ANGULAR_ANGLE_MASK);
	}
	driver->ampTurnState = (s16)turnResistMinBitshift;

	angle = (u16)(CTR_MipsAddLo(angle, CTR_MipsSra(CTR_MipsMulLo(turnResistMinBitshift, elapsedTimeMS), VEH_PHYS_ANGULAR_AXIS_INTEGRATION_SHIFT)) &
	              VEH_PHYS_ANGULAR_ANGLE_MASK);
	driver->angle = angle;

	(driver->rotCurr).y = (s16)CTR_MipsAddLo(CTR_MipsAddLo(angle, (s16)driftAngleCurr_Final), forwardDir);

	if (((actionsFlagSet & ACTION_ACCEL_PREVENTION) == 0) && (driver->accelTapCount < DRIVER_ACCEL_TAP_STEER_COUNT))
	{
		if (terrain->turnLeanScale != VEH_PHYS_ANGULAR_TERRAIN_SCALE_NEUTRAL)
		{
			turnResistMinBitshift = CTR_MipsSra(CTR_MipsMulLo(turnResistMinBitshift, terrain->turnLeanScale), 8);
		}
	}
	else
	{
		turnResistMinBitshift = CTR_MipsSra(CTR_MipsMulLo(turnResistMinBitshift, VEH_PHYS_ANGULAR_BRAKE_LEAN_SCALE), 8);
	}

	driver->axisRotationX = (s16)(CTR_MipsAddLo((u16)driver->axisRotationX,
	                                            CTR_MipsSra(CTR_MipsMulLo(turnResistMinBitshift, elapsedTimeMS), VEH_PHYS_ANGULAR_AXIS_INTEGRATION_SHIFT)) &
	                              VEH_PHYS_ANGULAR_ANGLE_MASK);

	PhysTerrainSlope(driver);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80060458-0x80060488.
int VehPhysGeneral_LerpQuarterStrength(int current, int desired)
{
	if (desired != 0)
	{
		desired = CTR_MipsSra(desired, 2);

		if (desired == 0)
		{
			desired = 1;
		}
	}

	if (desired <= current)
	{
		current = desired;
	}

	return current;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80060488-0x800605a0.
int VehPhysGeneral_LerpToForwards(struct Driver *d, int currentAngle, int currentVelocity, int targetAngle)
{
	b32 mirrored = false;
	int desiredVelocity = 0;

	d->turnAngleLerpTarget = 0;
	if ((targetAngle < 0) || ((targetAngle == 0 && (currentAngle < 0))))
	{
		mirrored = true;
		currentAngle = CTR_MipsNegLo(currentAngle);
		currentVelocity = CTR_MipsNegLo(currentVelocity);
		targetAngle = CTR_MipsNegLo(targetAngle);
	}

	if (d->wallRubTimer != DRIVER_WALL_RUB_TIMER_START)
	{
		if (targetAngle < currentAngle)
		{
			u32 lerpStrength;

			if (d->const_modelRotVelMax < currentAngle)
			{
				lerpStrength = CTR_MipsSubLo(CTR_MipsSll((u8)d->const_ModelTurnReturnStrength, 4), (u8)d->const_ModelTurnReturnStrength);
			}
			else
			{
				lerpStrength = (u8)d->const_ModelTurnReturnStrength;
			}
			desiredVelocity = VehPhysGeneral_LerpQuarterStrength(lerpStrength, CTR_MipsSubLo(currentAngle, targetAngle));
			desiredVelocity = CTR_MipsNegLo(desiredVelocity);
		}
		else
		{
			if (currentAngle < targetAngle)
			{
				if (currentAngle < 0)
				{
					desiredVelocity =
					    VehPhysGeneral_LerpQuarterStrength((u8)d->const_ModelTurnNegativeReturnStrength, CTR_MipsSubLo(targetAngle, currentAngle));
				}
				else
				{
					desiredVelocity = VehPhysGeneral_LerpQuarterStrength((u8)d->const_ModelTurnCounterSteerStrength, CTR_MipsSubLo(targetAngle, currentAngle));
					d->turnAngleLerpTarget = (s16)targetAngle;
				}
			}
		}
	}

	// Interpolate rotation by speed
	desiredVelocity = VehCalc_InterpBySpeed(currentVelocity, d->const_ModelTurnVelocityLerp, desiredVelocity);
	if (mirrored)
	{
		desiredVelocity = CTR_MipsNegLo(desiredVelocity);
	}
	return desiredVelocity;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800605a0-0x80060630.
int VehPhysGeneral_JumpGetVelY(s16 *normalVec, Vec3 *speedXYZ)
{
	int normalY = normalVec[1];
	int absNormalY = normalY;

	if (absNormalY < 0)
	{
		absNormalY = CTR_MipsNegLo(absNormalY);
	}

	if (absNormalY < VEH_PHYS_JUMP_NORMAL_Y_MIN)
	{
		return 0;
	}

	int dot = CTR_MipsAddLo(CTR_MipsMulLo(speedXYZ->x, normalVec[0]), CTR_MipsMulLo(speedXYZ->z, normalVec[2]));

	return CTR_MipsDiv(dot, normalY);
}

static int VehPhysGeneral_Jump_Abs(int value)
{
	return value < 0 ? CTR_MipsNegLo(value) : value;
}

static int VehPhysGeneral_Jump_Div2TowardZero(int value)
{
	return CTR_MipsSra(CTR_MipsAddLo(value, (u32)value >> 31), 1);
}

static int VehPhysGeneral_Jump_Div4TowardZero(int value)
{
	if (value < 0)
	{
		value = CTR_MipsAddLo(value, 3);
	}

	return CTR_MipsSra(value, 2);
}

static Vec3 VehPhysGeneral_Jump_RotateLoadedVector(s16 vx, s16 vy, s16 vz)
{
	Vec3 out;

	MTC2(CTR_PackS16Pair(vx, vy), 0);
	MTC2((u32)(u16)vz, 1);
	gte_mvmva(1, 0, 0, 3, 0);
	out.x = MFC2_S(25);
	out.y = MFC2_S(26);
	out.z = MFC2_S(27);

	return out;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80060630-0x80060f0c
void VehPhysGeneral_JumpAndFriction(struct Thread *t, struct Driver *d)
{
	(void)t;

	// Retail loads matrixMovingDir once, then reuses the same CP2 rotation regs
	// for every jump/friction impulse in this function.
	gte_SetRotMatrix(&d->matrixMovingDir);

	if ((d->kartState != KS_DRIFTING) && ((d->actionsFlagSet & ACTION_MASK_WEAPON) == 0) && (d->reserves == 0))
	{
		int ampTurn = VehPhysGeneral_Jump_Abs(CTR_MipsSra((s16)d->ampTurnState, 8));

		int turnDecrease = VehCalc_MapToRange(ampTurn, 0, (u8)d->const_BackwardTurnRate, 0, (int)d->const_TurnDecreaseRate);
		int baseSpeed = d->baseSpeed;
		int absBaseSpeed = VehPhysGeneral_Jump_Abs(baseSpeed);

		if (absBaseSpeed < turnDecrease)
		{
			turnDecrease = absBaseSpeed;
		}

		if (baseSpeed < 0)
		{
			d->baseSpeed = (s16)CTR_MipsAddLo((u16)d->baseSpeed, turnDecrease);
		}
		else
		{
			d->baseSpeed = (s16)CTR_MipsSubLo((u16)d->baseSpeed, turnDecrease);
		}
	}

	if (d->wallRubTimer != 0)
	{
		if (d->wallRubSpeedLimit < d->baseSpeed)
		{
			d->baseSpeed = d->wallRubSpeedLimit;
		}

		if (d->baseSpeed < CTR_MipsNegLo(d->wallRubSpeedLimit))
		{
			d->baseSpeed = (s16)CTR_MipsNegLo(d->wallRubSpeedLimit);
		}
	}

	Vec3 movement = d->velocity;
	int speedLoss = 0;

	if ((d->actionsFlagSet & ACTION_TOUCH_GROUND) == 0)
	{
		goto CHECK_FOR_ANY_JUMP;
	}

	int acceleration = 0;

	if (((d->stepFlagSet & COLL_STEP_TRIGGER_TURBO_PAD_MASK) != 0) && (d->baseSpeed > 0))
	{
		acceleration = VEH_PHYS_JUMP_TURBO_PAD_ACCEL;
	}
	else if (d->baseSpeed != 0)
	{
		if (((d->terrainMeta1->flags & TERRAIN_FLAG_ACCEL_WHILE_REVERSE_SLIDING) == 0) || (d->baseSpeed < 1) || (d->speedApprox >= 0))
		{
			int speedApprox = d->speedApprox;
			int absSpeedApprox = VehPhysGeneral_Jump_Abs(speedApprox);

			if ((absSpeedApprox > VEH_PHYS_JUMP_REVERSE_SLIDE_SPEED_COMPARE) && ((d->baseSpeed < 1) || (speedApprox < 1)) &&
			    ((d->baseSpeed >= 0) || (speedApprox >= 0)))
			{
				goto PROCESS_ACCEL;
			}
		}

		acceleration = CTR_MipsAddLo(d->const_Accel_ClassStat, CTR_MipsSll((s8)d->accelConst, 5) / 5);

		if ((d->stepFlagSet & COLL_STEP_TRIGGER_TURBO_PAD_MASK) == 0)
		{
			if ((d->reserves != 0) && (d->baseSpeed > 0))
			{
				acceleration = d->const_Accel_Reserves;
			}

			int slowUntilSpeed = d->terrainMeta1->slowUntilSpeed;
			if ((slowUntilSpeed != VEH_PHYS_JUMP_TERRAIN_SCALE_NEUTRAL) && ((d->actionsFlagSet & ACTION_MASK_WEAPON) == 0))
			{
				acceleration = CTR_MipsSra(CTR_MipsMulLo(slowUntilSpeed, acceleration), VEH_PHYS_JUMP_SPEED_FIXED_SHIFT);
			}
		}
		else if (d->baseSpeed > 0)
		{
			acceleration = VEH_PHYS_JUMP_TURBO_PAD_ACCEL;
		}
	}

PROCESS_ACCEL:
{
	int forwardImpulse = CTR_MipsSra(CTR_MipsMulLo(acceleration, sdata->gGT->elapsedTimeMS), 5);
	Vec3 rotated = VehPhysGeneral_Jump_RotateLoadedVector(0, 0, (s16)forwardImpulse);

	if (d->baseSpeed < 0)
	{
		d->forwardAccelImpulse = (s16)CTR_MipsNegLo(forwardImpulse);

		movement.x = CTR_MipsSubLo(movement.x, rotated.x);
		movement.y = CTR_MipsSubLo(movement.y, rotated.y);
		movement.z = CTR_MipsSubLo(movement.z, rotated.z);

		d->forwardAccelVector.x = (s16)CTR_MipsNegLo(rotated.x);
		d->forwardAccelVector.y = (s16)CTR_MipsNegLo(rotated.y);
		d->forwardAccelVector.z = (s16)CTR_MipsNegLo(rotated.z);
	}
	else
	{
		d->forwardAccelImpulse = (s16)forwardImpulse;

		movement.x = CTR_MipsAddLo(movement.x, rotated.x);
		movement.y = CTR_MipsAddLo(movement.y, rotated.y);
		movement.z = CTR_MipsAddLo(movement.z, rotated.z);

		d->forwardAccelVector.x = (s16)rotated.x;
		d->forwardAccelVector.y = (s16)rotated.y;
		d->forwardAccelVector.z = (s16)rotated.z;
	}

	u32 movementLengthSq =
	    (u32)CTR_MipsAddLo(CTR_MipsAddLo(CTR_MipsMulLo(movement.x, movement.x), CTR_MipsMulLo(movement.y, movement.y)), CTR_MipsMulLo(movement.z, movement.z));
	speedLoss = CTR_MipsSubLo((s32)(VehCalc_FastSqrt(movementLengthSq, VEH_PHYS_JUMP_FAST_SQRT_ITERATIONS) >> VEH_PHYS_JUMP_SPEED_FIXED_SHIFT),
	                          VehPhysGeneral_Jump_Abs(d->baseSpeed));

	b32 clampToForwardImpulse = forwardImpulse < speedLoss;
	if (speedLoss < 0)
	{
		speedLoss = 0;
		clampToForwardImpulse = forwardImpulse < 0;
	}
	if (clampToForwardImpulse)
	{
		speedLoss = forwardImpulse;
	}

	if (((d->actionsFlagSet & ACTION_TOUCH_GROUND) == 0) || (d->jump_ForcedMS == 0))
	{
		goto CHECK_FOR_ANY_JUMP;
	}

	if (d->jump_HighJumpTimerMS != 0)
	{
		d->jump_HighJumpTimerMS = VEH_PHYS_JUMP_HIGH_TIMER_MS;
	}

	if (d->kartState == KS_BLASTED)
	{
		GAMEPAD_ShockFreq(d, VEH_PHYS_JUMP_RUMBLE_CHANNEL, 0);
		GAMEPAD_ShockForce1(d, VEH_PHYS_JUMP_RUMBLE_CHANNEL, VEH_PHYS_JUMP_RUMBLE_FORCE);
	}
}

	goto PROCESS_JUMP;

CHECK_FOR_ANY_JUMP:
	if (((d->actionsFlagSet & ACTION_WEAPON_FIRE_REQUEST) != 0) && (d->heldItemID == HELD_ITEM_SPRING))
	{
		d->actionsFlagSet &= ~ACTION_WEAPON_FIRE_REQUEST;

		if ((d->jump_CoyoteTimerMS != 0) && (d->jump_CooldownMS == 0))
		{
			d->jump_ForcedMS = VEH_PHYS_JUMP_FORCED_MS;

			int jumpForce = CTR_MipsAddLo(CTR_MipsSll(d->const_JumpForce, 3), d->const_JumpForce);
			d->jump_InitialVelY = (s16)VehPhysGeneral_Jump_Div4TowardZero(jumpForce);

			OtherFX_Play_Echo(VEH_PHYS_JUMP_SPRING_SFX, 1, (d->actionsFlagSet & ACTION_ENGINE_ECHO) != 0);

			d->jump_HighJumpTimerMS = VEH_PHYS_JUMP_HIGH_TIMER_MS;
			goto PROCESS_JUMP;
		}

		d->noItemTimer = 0;
	}

	if (d->forcedJumpType == FORCED_JUMP_NONE)
	{
		if ((d->jump_CoyoteTimerMS == 0) || (d->jump_TenBuffer == 0) || (d->jump_CooldownMS != 0))
		{
			if ((d->actionsFlagSet & ACTION_TOUCH_GROUND) != 0)
			{
				if ((d->underDriver != NULL) && (d->underDriver->mulNormVecY != 0))
				{
					int speedApprox = d->speedApprox;
					if (speedApprox < 0)
					{
						speedApprox = VehPhysGeneral_Jump_Abs(speedApprox);
					}

					s16 antiGravVelY = (s16)CTR_MipsSra(CTR_MipsMulLo(d->underDriver->mulNormVecY, speedApprox), 8);
					Vec3 rotated = VehPhysGeneral_Jump_RotateLoadedVector(0, antiGravVelY, 0);

					movement.x = CTR_MipsAddLo(movement.x, rotated.x);
					movement.y = CTR_MipsAddLo(movement.y, rotated.y);
					movement.z = CTR_MipsAddLo(movement.z, rotated.z);
				}
			}

			goto NOT_JUMPING;
		}

		d->jump_ForcedMS = VEH_PHYS_JUMP_FORCED_MS;
		d->numberOfJumps = (s16)CTR_MipsAddLo((u16)d->numberOfJumps, 1);
		d->jump_InitialVelY = d->const_JumpForce;

		OtherFX_Play_Echo(VEH_PHYS_JUMP_NORMAL_SFX, 1, (d->actionsFlagSet & ACTION_ENGINE_ECHO) != 0);
	}
	else
	{
		if ((d->jump_ForcedMS == 0) || (d->jump_InitialVelY == d->const_JumpForce))
		{
			OtherFX_Play(VEH_PHYS_JUMP_FORCED_SFX, 1);
		}

		d->jump_ForcedMS = VEH_PHYS_JUMP_FORCED_MS;

		int jumpForce = CTR_MipsAddLo(CTR_MipsSll(d->const_JumpForce, 1), d->const_JumpForce);
		if (d->forcedJumpType == FORCED_JUMP_HIGH)
		{
			d->jump_HighJumpTimerMS = VEH_PHYS_JUMP_HIGH_TIMER_MS;
			d->jump_InitialVelY = (s16)jumpForce;
		}
		else
		{
			d->jump_InitialVelY = (s16)VehPhysGeneral_Jump_Div2TowardZero(jumpForce);
		}

		d->forcedJumpType = FORCED_JUMP_NONE;
	}

PROCESS_JUMP:
	d->jump_CooldownMS = VEH_PHYS_JUMP_COOLDOWN_MS;
	d->jump_TenBuffer = 0;
	d->actionsFlagSet |= ACTION_JUMP_STARTED | ACTION_TURBO_INPUT_LATCH;

	int bestJumpVelY = 0;
	int jumpVelY = VehPhysGeneral_JumpGetVelY(d->AxisAngle4_normalVec.v, &movement);
	if (VehPhysGeneral_Jump_Abs(bestJumpVelY) < VehPhysGeneral_Jump_Abs(jumpVelY))
	{
		bestJumpVelY = jumpVelY;
	}

	s16 *normalVec = d->AxisAngle1_normalVec.v;
	if ((d->actionsFlagSet & ACTION_TOUCH_GROUND) == 0)
	{
		normalVec = d->AxisAngle2_normalVec.v;
	}

	jumpVelY = VehPhysGeneral_JumpGetVelY(normalVec, &movement);

	int jumpVelYSquared = CTR_MipsMulLo(bestJumpVelY, bestJumpVelY);
	if (VehPhysGeneral_Jump_Abs(bestJumpVelY) < VehPhysGeneral_Jump_Abs(jumpVelY))
	{
		jumpVelYSquared = CTR_MipsMulLo(jumpVelY, jumpVelY);
		bestJumpVelY = jumpVelY;
	}

	int verticalSpeed = VehCalc_FastSqrt(
	    (u32)CTR_MipsSra(CTR_MipsAddLo(jumpVelYSquared, CTR_MipsMulLo(d->jump_InitialVelY, d->jump_InitialVelY)), VEH_PHYS_JUMP_SPEED_FIXED_SHIFT),
	    VEH_PHYS_JUMP_SPEED_FIXED_SHIFT);

	int maxVerticalSpeed = sdata->gGT->level1->jumpVerticalSpeedCap << VEH_PHYS_JUMP_SPEED_FIXED_SHIFT;
	if (maxVerticalSpeed == 0)
	{
		maxVerticalSpeed = VEH_PHYS_JUMP_VERTICAL_SPEED_DEFAULT;
	}
	else if (maxVerticalSpeed > VEH_PHYS_JUMP_VERTICAL_SPEED_MAX)
	{
		maxVerticalSpeed = VEH_PHYS_JUMP_VERTICAL_SPEED_MAX;
	}

	verticalSpeed = CTR_MipsSubLo(verticalSpeed, bestJumpVelY);
	if (maxVerticalSpeed < verticalSpeed)
	{
		verticalSpeed = maxVerticalSpeed;
	}

	if (movement.y < verticalSpeed)
	{
		movement.y = verticalSpeed;
	}

NOT_JUMPING:
	VehPhysCrash_ConvertVecToSpeed(d, &movement);

	int speed = CTR_MipsSubLo((u16)d->speed, speedLoss);
	d->speed = (s16)speed;
	if (d->speed < 0)
	{
		d->speed = 0;
	}

	int speedApprox = d->speedApprox;
	if (speedApprox < 0)
	{
		speedApprox = VehPhysGeneral_Jump_Abs(speedApprox);

		if (speedApprox < VEH_PHYS_JUMP_SPEEDOMETER_REVERSE_THRESHOLD)
		{
			d->speedometerNeedleValue =
			    (s16)CTR_MipsSubLo((u16)d->speedometerNeedleValue, CTR_MipsSra(d->speedometerNeedleValue, VEH_PHYS_JUMP_SPEEDOMETER_DECAY_SHIFT));
		}
		else
		{
			d->speedometerNeedleValue =
			    (s16)((u32)CTR_MipsAddLo(CTR_MipsMulLo(d->speedometerNeedleValue, VEH_PHYS_JUMP_SPEEDOMETER_BLEND_OLD),
			                             CTR_MipsMulLo(sdata->gGT->timer & VEH_PHYS_JUMP_SPEEDOMETER_TIMER_MASK, VEH_PHYS_JUMP_SPEEDOMETER_TIMER_SCALE)) >>
			          VEH_PHYS_JUMP_SPEEDOMETER_BLEND_SHIFT);
		}
	}
	else
	{
		d->speedometerNeedleValue = (s16)CTR_MipsSra(CTR_MipsAddLo(CTR_MipsMulLo(d->speedometerNeedleValue, VEH_PHYS_JUMP_SPEEDOMETER_BLEND_OLD),
		                                                           CTR_MipsMulLo(speedApprox, VEH_PHYS_JUMP_SPEEDOMETER_BLEND_NEW)),
		                                             VEH_PHYS_JUMP_SPEEDOMETER_BLEND_SHIFT);
	}
}

enum ItemSet
{
	ITEMSET_Invalid = -1,
	ITEMSET_Race1 = 0,
	ITEMSET_Race2,
	ITEMSET_Race3,
	ITEMSET_Race4,
	ITEMSET_BattleDefault,
	ITEMSET_BattleCustom,
	ITEMSET_CrystalChallenge,
	ITEMSET_BossRace,
	ITEMSET_Count,
	ITEMSET_RNG_BUCKET_COUNT = 0xc8,
	ITEMSET_FALLBACK_ITEM_COUNT = HELD_ITEM_MISSILE_3X + 1,
	ITEMSET_WEAPON_COUNT_RACE1 = 0x14,
	ITEMSET_WEAPON_COUNT_RACE2 = 0x34,
	ITEMSET_WEAPON_COUNT_RACE3 = 0x14,
	ITEMSET_WEAPON_COUNT_RACE4 = 0x13,
	ITEMSET_WEAPON_COUNT_BATTLE_DEFAULT = 0x14,
	ITEMSET_WEAPON_COUNT_BOSS = 0x14,
	ITEMSET_RNG_RANDOM_SHIFT = 3,
	ITEMSET_BOSS_LOSSES_REPLACE_MASK_CLOCK_WARPBALL = 3,
	ITEMSET_BOSS_LOSSES_REPLACE_MASK_CLOCK = 4,
	ITEMSET_BOSS_LOSSES_REPLACE_CLOCK = 5,
	ITEMSET_THREE_MISSILES_MIN_PLAYERS = 3,
	ITEMSET_THREE_MISSILES_HELD_LIMIT = 2,
};

// all except CrystalChallenge
extern u8 *itemSetWeaponTables[ITEMSET_Count];
extern u8 itemSetWeaponCounts[ITEMSET_Count];

CTR_STATIC_ASSERT(ITEMSET_Invalid == -1);
CTR_STATIC_ASSERT(ITEMSET_Race1 == 0);
CTR_STATIC_ASSERT(ITEMSET_BattleDefault == 4);
CTR_STATIC_ASSERT(ITEMSET_BattleCustom == 5);
CTR_STATIC_ASSERT(ITEMSET_CrystalChallenge == 6);
CTR_STATIC_ASSERT(ITEMSET_BossRace == 7);
CTR_STATIC_ASSERT(ITEMSET_Count == 8);
CTR_STATIC_ASSERT(ITEMSET_RNG_BUCKET_COUNT == 0xc8);
CTR_STATIC_ASSERT(ITEMSET_FALLBACK_ITEM_COUNT == 0xc);
CTR_STATIC_ASSERT(ITEMSET_WEAPON_COUNT_RACE1 == 0x14);
CTR_STATIC_ASSERT(ITEMSET_WEAPON_COUNT_RACE2 == 0x34);
CTR_STATIC_ASSERT(ITEMSET_WEAPON_COUNT_RACE3 == 0x14);
CTR_STATIC_ASSERT(ITEMSET_WEAPON_COUNT_RACE4 == 0x13);
CTR_STATIC_ASSERT(ITEMSET_WEAPON_COUNT_BATTLE_DEFAULT == 0x14);
CTR_STATIC_ASSERT(ITEMSET_WEAPON_COUNT_BOSS == 0x14);
CTR_STATIC_ASSERT(ITEMSET_RNG_RANDOM_SHIFT == 3);
CTR_STATIC_ASSERT(ITEMSET_BOSS_LOSSES_REPLACE_MASK_CLOCK_WARPBALL == 3);
CTR_STATIC_ASSERT(ITEMSET_BOSS_LOSSES_REPLACE_MASK_CLOCK == 4);
CTR_STATIC_ASSERT(ITEMSET_BOSS_LOSSES_REPLACE_CLOCK == 5);
CTR_STATIC_ASSERT(ITEMSET_THREE_MISSILES_MIN_PLAYERS == 3);
CTR_STATIC_ASSERT(ITEMSET_THREE_MISSILES_HELD_LIMIT == 2);

// Itemset infographic (outdated):
// https://discord.com/channels/330945093416779787/550106151887568906/734368526294450267
// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80060f0c-0x80061488.
void VehPhysGeneral_SetHeldItem(struct Driver *driver)
{
	struct GameTracker *gGT = sdata->gGT;

	int itemSet = ITEMSET_Invalid;

	if ((gGT->gameMode1 & BATTLE_MODE) != 0)
	{
		// 6th Itemset (Battle Mode Custom Itemset)
		itemSet = ITEMSET_BattleCustom;

		// 5th Itemset (Battle Mode Default Itemset)
		if (gGT->battleSetup.enabledWeapons == BATTLE_DEFAULT_WEAPON_FLAGS)
		{
			itemSet = ITEMSET_BattleDefault;
		}
	}

	// Not in Battle Mode
	else
	{
		if ((gGT->gameMode1 & CRYSTAL_CHALLENGE) != 0)
		{
			// 7th Itemset (Crystal Challenge)
			itemSet = ITEMSET_CrystalChallenge;
		}
		else
		{
			// Choose Itemset based on number of Drivers
			int mode = gGT->numPlyrCurrGame + gGT->numBotsNextGame;

			switch (mode)
			{
			// if boss race
			case 2:

				// boss race, last place
				itemSet = ITEMSET_BossRace;

				// if in first place
				if (driver->driverRank == 0)
				{
				Itemset1:
					// 1st Itemset
					itemSet = ITEMSET_Race1;
				}
				break;

			// 3P VS race
			case 3:

				// if first place
				if (driver->driverRank == 0)
				{
					goto Itemset1;
				}

				// default (2nd or 3rd place)
				itemSet = ITEMSET_Race4;

				// 50/50 chance of an upgrade,
				// while in 2nd place

				if (driver->driverRank == 1)
				{
					itemSet = ITEMSET_Race3;
					s32 rng = MixRNG_Scramble();
					if (rng & 1)
					{
						goto Itemset2;
					}
				}

				break;
			case 4:
				itemSet = driver->driverRank;
				break;
			case 5:
				itemSet = driver->driverRank;
				// 5th rank is 4th Itemset
				if (itemSet == 4)
				{
					itemSet = 3;
				}
				break;

			// 2P Arcade
			case 6:

				// careful, dont get confused by names
				itemSet = driver->driverRank;

				// if 1st place, ItemSet1
				if (itemSet == 0)
				{
					goto Itemset1;
				}

				// if 6th place, ItemSet4
				if (itemSet == 5)
				{
					itemSet = ITEMSET_Race4;
				}

				// 2nd, 3rd place, gets 2nd Itemset
				// 4th, 5th place, gets 3rd Itemset
				else
				{
					itemSet = (itemSet - 1) / 2 + 1;
				}

				break;

			// 1P Arcade
			case 8:

				// if in 2nd place, get itemSet2
				if (driver->driverRank == 1)
				{
				Itemset2:
					itemSet = ITEMSET_Race2;
				}
				else
				{
					itemSet = CTR_MipsSra(CTR_MipsAddLo(driver->driverRank, (u32)driver->driverRank >> 31), 1);
				}
			}
		}

		// if you have 4th-place itemset on first lap,
		// then override to 3rd place
		if (itemSet == ITEMSET_Race4 && driver->lapIndex == 0)
		{
			itemSet = ITEMSET_Race3;
		}
	}

	// Decide item for Driver
	s32 rng = CTR_MipsSra(MixRNG_Scramble(), ITEMSET_RNG_RANDOM_SHIFT);
	rng = CTR_MipsSubLo(rng, CTR_MipsMulLo(CTR_MipsDiv(rng, ITEMSET_RNG_BUCKET_COUNT), ITEMSET_RNG_BUCKET_COUNT));

	DriverHeldItem item;
	switch (itemSet)
	{
	case ITEMSET_Race1:
	case ITEMSET_Race2:
	case ITEMSET_Race3:
	case ITEMSET_Race4:
	case ITEMSET_BattleDefault:
	case ITEMSET_BossRace:
		driver->heldItemID = itemSetWeaponTables[itemSet][(rng * itemSetWeaponCounts[itemSet]) / ITEMSET_RNG_BUCKET_COUNT];
		break;

	// uses int array instead of char,
	// should fix that later, requires 230 rewrite
	case ITEMSET_BattleCustom:
		driver->heldItemID = gGT->battleSetup.RNG_itemSetCustom[(rng * gGT->battleSetup.numWeapons) / ITEMSET_RNG_BUCKET_COUNT];
		break;

	case ITEMSET_CrystalChallenge:
		// Item is bomb at Rocky Road, Nitro Court
		// Item is turbo at Skull Rock and Rampage Ruins
		item = HELD_ITEM_BOMB_1X;
		if (gGT->levelID != SKULL_ROCK && gGT->levelID != RAMPAGE_RUINS)
		{
			goto SetItem;
		}
		driver->heldItemID = HELD_ITEM_TURBO;
		break;

	// "-1st place": Undecided rank
	default:
	{
		s32 fallbackRng = MixRNG_Scramble();
		item = (u8)CTR_MipsSubLo(fallbackRng, CTR_MipsMulLo(CTR_MipsDiv(fallbackRng, ITEMSET_FALLBACK_ITEM_COUNT), ITEMSET_FALLBACK_ITEM_COUNT));
	}
	SetItem:
		driver->heldItemID = item;
	}

	// In Boss race
	if (gGT->gameMode1 & ADVENTURE_BOSS)
	{
		s8 bossFails = sdata->advProgress.timesLostBossRace[gGT->bossID];

		if (bossFails < ITEMSET_BOSS_LOSSES_REPLACE_MASK_CLOCK_WARPBALL)
		{
			// Replace Clock, Mask,  with 3 Missiles
			if ((u32)driver->heldItemID - HELD_ITEM_MASK < (HELD_ITEM_WARPBALL - HELD_ITEM_MASK + 1))
			{
				driver->heldItemID = HELD_ITEM_MISSILE_3X;
			}
		}

		else if (bossFails < ITEMSET_BOSS_LOSSES_REPLACE_MASK_CLOCK)
		{
			// Replace Clock, Mask with 3 Missiles
			if ((u32)driver->heldItemID - HELD_ITEM_MASK < (HELD_ITEM_CLOCK - HELD_ITEM_MASK + 1))
			{
				driver->heldItemID = HELD_ITEM_MISSILE_3X;
			}
		}

		else if (bossFails < ITEMSET_BOSS_LOSSES_REPLACE_CLOCK && driver->heldItemID == HELD_ITEM_CLOCK)
		{
			// Replace Clock with 3 Missiles
			driver->heldItemID = HELD_ITEM_MISSILE_3X;
		}

		// Replace 3 Missiles with 1 Missile if racing Komodo Joe
		if (gGT->levelID == DRAGON_MINES && driver->heldItemID == HELD_ITEM_MISSILE_3X)
		{
			driver->heldItemID = HELD_ITEM_MISSILE_1X;
		}
	}

	// Replace unused Spring item with Turbo
	if (driver->heldItemID == HELD_ITEM_SPRING)
	{
		driver->heldItemID = HELD_ITEM_TURBO;
	}

	// Make sure only 1 Warpball is instanced at once
	if (driver->heldItemID == HELD_ITEM_WARPBALL)
	{
		// if nobody has warpball, then set flag that somebody has it
		if ((gGT->gameMode1 & WARPBALL_HELD) == 0)
		{
			gGT->gameMode1 |= WARPBALL_HELD;
		}

		// if somebody has warpball already, then give 3 missiles
		else
		{
			driver->heldItemID = HELD_ITEM_MISSILE_3X;
		}
	}

	if (
	    // if you got 3 missiles
	    driver->heldItemID == HELD_ITEM_MISSILE_3X &&

	    // if three or more players
	    gGT->numPlyrCurrGame >= ITEMSET_THREE_MISSILES_MIN_PLAYERS &&

	    // if not in battle mode
	    ((gGT->gameMode1 & BATTLE_MODE) == 0))
	{
		// if less than 2 drivers have 3 missiles, then increase number of drivers that have it
		if (gGT->numPlayersWith3Missiles < ITEMSET_THREE_MISSILES_HELD_LIMIT)
		{
			gGT->numPlayersWith3Missiles++;
		}

		// if 2 drivers already have 3 missiles, now you have 1 missile
		else
		{
			driver->heldItemID = HELD_ITEM_MISSILE_1X;
		}
	}

	// Set number of held items
	if ((u32)driver->heldItemID - HELD_ITEM_BOMB_3X < (HELD_ITEM_MISSILE_3X - HELD_ITEM_BOMB_3X + 1))
	{
		driver->numHeldItems = HELD_ITEM_STACK_COUNT;
	}

	return;
}

u8 *itemSetWeaponTables[ITEMSET_Count] = {(u8 *)&data.RNG_itemSetRace1[0],
                                          (u8 *)&data.RNG_itemSetRace2[0],
                                          (u8 *)&data.RNG_itemSetRace3[0],
                                          (u8 *)&data.RNG_itemSetRace4[0],
                                          (u8 *)&data.RNG_itemSetBattleDefault[0],
                                          (u8 *)&sdata_static.gameTracker.battleSetup.RNG_itemSetCustom[0],
                                          NULL,
                                          (u8 *)&data.RNG_itemSetBossrace[0]};

// NOTE(aalhendi): Race4 storage is 0x14 bytes, but retail samples 0x13 entries.
u8 itemSetWeaponCounts[ITEMSET_Count] = {
    ITEMSET_WEAPON_COUNT_RACE1, ITEMSET_WEAPON_COUNT_RACE2, ITEMSET_WEAPON_COUNT_RACE3, ITEMSET_WEAPON_COUNT_RACE4, ITEMSET_WEAPON_COUNT_BATTLE_DEFAULT, 0, 0,
    ITEMSET_WEAPON_COUNT_BOSS};

enum
{
	VEH_BASE_SPEED_MAX_WUMPA = 9,
	VEH_BASE_SPEED_MAX_TURBO_MULTIPLIER = 5,
	VEH_BASE_SPEED_STAT_BLEND_SHIFT = 0xc,
	VEH_BASE_SPEED_STAT_DIVISOR = 5,
	VEH_BASE_SPEED_STAT_OFFSET = 1,
	VEH_BASE_SPEED_WUMPA_DIVISOR = 10,
	VEH_BASE_SPEED_DAMAGE_HALF_SHIFT = 1,
	VEH_BASE_SPEED_CLOCK_RANK_BASE = 0x14,
	VEH_BASE_SPEED_CLOCK_DAMAGE_SHIFT = 4,
	VEH_BASE_SPEED_NET_CAP = 0x6400,
};

CTR_STATIC_ASSERT(VEH_BASE_SPEED_MAX_WUMPA == 9);
CTR_STATIC_ASSERT(VEH_BASE_SPEED_MAX_TURBO_MULTIPLIER == 5);
CTR_STATIC_ASSERT(VEH_BASE_SPEED_STAT_BLEND_SHIFT == 0xc);
CTR_STATIC_ASSERT(VEH_BASE_SPEED_STAT_DIVISOR == 5);
CTR_STATIC_ASSERT(VEH_BASE_SPEED_STAT_OFFSET == 1);
CTR_STATIC_ASSERT(VEH_BASE_SPEED_WUMPA_DIVISOR == 10);
CTR_STATIC_ASSERT(VEH_BASE_SPEED_DAMAGE_HALF_SHIFT == 1);
CTR_STATIC_ASSERT(VEH_BASE_SPEED_CLOCK_RANK_BASE == 0x14);
CTR_STATIC_ASSERT(VEH_BASE_SPEED_CLOCK_DAMAGE_SHIFT == 4);
CTR_STATIC_ASSERT(VEH_BASE_SPEED_NET_CAP == 0x6400);

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80061488-0x8006163c.
int VehPhysGeneral_GetBaseSpeed(struct Driver *driver)
{
	int statAdditional = (int)driver->const_Speed_ClassStat;

	int netWumpaFruitCount = (int)driver->numWumpas;
	if (netWumpaFruitCount > VEH_BASE_SPEED_MAX_WUMPA)
	{
		netWumpaFruitCount = VEH_BASE_SPEED_MAX_WUMPA;
	}

	int turboMultiplier = (int)driver->turboConst;
	if (turboMultiplier > VEH_BASE_SPEED_MAX_TURBO_MULTIPLIER)
	{
		turboMultiplier = VEH_BASE_SPEED_MAX_TURBO_MULTIPLIER;
	}

	int netSpeedStat = CTR_MipsSubLo(
	    CTR_MipsDiv(CTR_MipsSll(CTR_MipsSubLo(driver->const_AccelSpeed_ClassStat, driver->const_Speed_ClassStat), VEH_BASE_SPEED_STAT_BLEND_SHIFT),
	                VEH_BASE_SPEED_STAT_DIVISOR),
	    VEH_BASE_SPEED_STAT_OFFSET);

	int speedAdditional = CTR_MipsSra(
	    CTR_MipsAddLo(CTR_MipsDiv(CTR_MipsMulLo(netWumpaFruitCount, netSpeedStat), VEH_BASE_SPEED_WUMPA_DIVISOR), CTR_MipsMulLo(turboMultiplier, netSpeedStat)),
	    VEH_BASE_SPEED_STAT_BLEND_SHIFT);

	if ((driver->actionsFlagSet & ACTION_MASK_WEAPON) != 0)
	{
		speedAdditional = CTR_MipsAddLo(speedAdditional, driver->const_MaskSpeed);
	}

	if (driver->reserves != 0)
	{
		statAdditional = CTR_MipsAddLo(statAdditional, driver->fireSpeedCap);

		int netSpeedCap = CTR_MipsSubLo(
		    CTR_MipsAddLo(driver->const_SingleTurboSpeed, CTR_MipsSll(CTR_MipsSubLo(driver->const_SacredFireSpeed, driver->const_SingleTurboSpeed), 1)),
		    driver->fireSpeedCap);
		if (netSpeedCap < 0)
		{
			netSpeedCap = 0;
		}

		if (netSpeedCap < speedAdditional)
		{
			speedAdditional = netSpeedCap;
		}
	}

	int subtract = 0;

	if (driver->instTntRecv != 0)
	{
		subtract = CTR_MipsSra(driver->const_DamagedSpeed, VEH_BASE_SPEED_DAMAGE_HALF_SHIFT);
	}

	if (
	    // burn, squish, or raincloud
	    (driver->burnTimer != 0) || (driver->squishTimer != 0) || (driver->rainCloudEffect == RAIN_CLOUD_EFFECT_SLOW))
	{
		subtract = driver->const_DamagedSpeed;
	}

	if (driver->clockReceive != 0)
	{
		// NOTE(aalhendi): Retail scales clock damage by rank: stronger near the front, still nonzero near the back.
		int clockEffect = CTR_MipsSra(CTR_MipsMulLo(driver->const_DamagedSpeed, CTR_MipsSubLo(VEH_BASE_SPEED_CLOCK_RANK_BASE, driver->driverRank)),
		                              VEH_BASE_SPEED_CLOCK_DAMAGE_SHIFT);

		if (subtract < clockEffect)
		{
			subtract = clockEffect;
		}
	}

	int netSpeed = CTR_MipsSubLo(CTR_MipsAddLo(statAdditional, speedAdditional), subtract);

	if (VEH_BASE_SPEED_NET_CAP < netSpeed)
	{
		netSpeed = VEH_BASE_SPEED_NET_CAP;
	}

	return netSpeed;
}
