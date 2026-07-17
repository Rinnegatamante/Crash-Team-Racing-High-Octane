#include <common.h>

enum
{
	VEH_EMITTER_AXIS_COUNT = 3,
	VEH_EMITTER_EXHAUST_ICON_LOW = 1,
	VEH_EMITTER_EXHAUST_ICON_WATER = 7,
	VEH_EMITTER_EXHAUST_WATER_Y_LIMIT = FP8_ONE,
	VEH_EMITTER_EXHAUST_VEL_Y = 0x400,
	VEH_EMITTER_EXHAUST_VEL_Z = -0x400,
	VEH_EMITTER_EXHAUST_POS_X_NUM = 9,
	VEH_EMITTER_EXHAUST_POS_X_SHIFT = 3,
	VEH_EMITTER_EXHAUST_POS_Y_NUM = 7,
	VEH_EMITTER_EXHAUST_POS_Y_SHIFT = 1,
	VEH_EMITTER_EXHAUST_POS_Z_NUM = -0x38,
	VEH_EMITTER_EXHAUST_POS_Z_SHIFT = 4,
	VEH_EMITTER_EXHAUST_POS_SECOND_X_NUM = -0x12,
	VEH_EMITTER_EXHAUST_POS_SECOND_X_SHIFT = 4,
	VEH_EMITTER_TURBO_METER_COLOR_MIN = 129,
	VEH_EMITTER_TURBO_ROOM_WARNING_PAD = 2,
	VEH_EMITTER_TURBO_ROOM_WARNING_SCALE = 32,
	VEH_EMITTER_GROUND_SPARK_COUNT = 10,
	VEH_EMITTER_GROUND_SPARK_RNG_MASK = 0x7ff,
	VEH_EMITTER_TERRAIN_MIN_SPEED = 0x300,
	VEH_EMITTER_TERRAIN_NORMAL_TIRE_COUNT = 2,
	VEH_EMITTER_TERRAIN_DRIFT_TIRE_COUNT = 4,
	VEH_EMITTER_TIRE_LEFT_X = -0x1e,
	VEH_EMITTER_TIRE_RIGHT_X = 0x1e,
	VEH_EMITTER_TIRE_BACK_Z = -0x14,
	VEH_EMITTER_TIRE_FRONT_Z = 0x28,
	VEH_EMITTER_TERRAIN_TIRE_Y = 0x0a,
	VEH_EMITTER_SKID_LATERAL_SCALE = 15,
	VEH_EMITTER_SKID_WIDTH_SCALE = 10,
	VEH_EMITTER_SKID_COLOR_BIAS = 2,
	VEH_EMITTER_SKID_PREVIOUS_WIDTH_SHIFT = 1,
	VEH_EMITTER_SKID_HALF_LATERAL_SHIFT = 1,
	VEH_EMITTER_MUD_SPLASH_NORMAL_COUNT = 1,
	VEH_EMITTER_MUD_SPLASH_LANDING_COUNT = 10,
	VEH_EMITTER_MUD_SPLASH_ICON = 0x0d,
	VEH_EMITTER_MUD_SPLASH_VELOCITY_SHIFT = 4,
	VEH_EMITTER_MUD_SPLASH_VELOCITY_SCALE = 1 << VEH_EMITTER_MUD_SPLASH_VELOCITY_SHIFT,
	VEH_EMITTER_PARTICLE_WORLD_SCALE = FP8_ONE,
	VEH_EMITTER_MUD_MIN_SPEED = FP8_ONE * 5,
	VEH_EMITTER_LANDING_SPARK_MIN_SPEED = FP8_ONE * 6,
	VEH_EMITTER_LANDING_SPARK_MIN_JUMP = FP8_ONE * 22,
	VEH_EMITTER_TERRAIN_AUDIO_SPEED_MAX = 5000,
	VEH_EMITTER_TERRAIN_AUDIO_VOLUME_MAX = 200,
	VEH_EMITTER_TERRAIN_AUDIO_DISTORT_SPEED_MAX = 12000,
	VEH_EMITTER_TERRAIN_AUDIO_DISTORT_MIN = 0x6c,
	VEH_EMITTER_TERRAIN_AUDIO_DISTORT_MAX = 0xd2,
	VEH_EMITTER_TERRAIN_RUMBLE_MIN_SPEED = FP8_ONE * 2,
	VEH_EMITTER_LANDING_RUMBLE_FRAMES = 3,
	VEH_EMITTER_RUMBLE_FORCE_MAX = 0xff,
	VEH_EMITTER_WALL_RUB_TIMER_FULL = 0xf0,
	VEH_EMITTER_WALL_RUB_SFX = 0x14,
	VEH_EMITTER_WALL_RUB_VOLUME_STEP = 0x14,
	VEH_EMITTER_SKID_AUDIO_MIN_SPEED = (FP8_ONE * 2) + 1,
	VEH_EMITTER_SKID_AUDIO_SPEED_MIN = 2000,
	VEH_EMITTER_SKID_AUDIO_SPEED_MAX = 12000,
	VEH_EMITTER_SKID_AUDIO_VOLUME_MIN = 0x14,
	VEH_EMITTER_SKID_AUDIO_VOLUME_MAX = 0xaa,
	VEH_EMITTER_SKID_AUDIO_DISTORT_SLOW = 0x92,
	VEH_EMITTER_SKID_AUDIO_DISTORT_FAST = 0x78,
	VEH_EMITTER_WALL_SPARK_MIN_SPEED = FP8_ONE * 2,
	VEH_EMITTER_WALL_RUMBLE_FRAME_LIMIT = 450,
	VEH_EMITTER_WALL_RUMBLE_FRAMES = 8,
	VEH_EMITTER_WALL_RUMBLE_FORCE = 0x7f,
	VEH_EMITTER_WALL_SPARK_LEFT_X = -0x2200,
	VEH_EMITTER_WALL_SPARK_RIGHT_X = 0x2200,
	VEH_EMITTER_WALL_SPARK_Y = 0x0a00,
	VEH_EMITTER_WALL_SPARK_REVERSE_Z = -0x1400,
	VEH_EMITTER_WALL_SPARK_FORWARD_Z = 0x2800,
	VEH_EMITTER_WALL_SPARK_SCRATCH_HALF_COUNT = 6,
	VEH_EMITTER_ALPHA_FULL = 0x1000,
	VEH_EMITTER_JOG_GROUND = 0x27,
	VEH_EMITTER_JOG_WOBBLE_ALT = 0xf0,
	VEH_EMITTER_JOG_WOBBLE_TIMER_MASK = 3,
	VEH_EMITTER_JOG_WOBBLE_DURATION = 0x100,
	VEH_EMITTER_JOG_AIR_LANDING_BOOST_MAX = 0x80,
	VEH_EMITTER_JOG_AIR_TURN_NEGATIVE = 0x12,
	VEH_EMITTER_JOG_AIR_TURN_POSITIVE = 0x22,
	VEH_EMITTER_JOG_AIR_DURATION = 0x20,
};

CTR_STATIC_ASSERT(VEH_EMITTER_AXIS_COUNT == 3);
CTR_STATIC_ASSERT(VEH_EMITTER_EXHAUST_ICON_LOW == 1);
CTR_STATIC_ASSERT(VEH_EMITTER_EXHAUST_ICON_WATER == 7);
CTR_STATIC_ASSERT(VEH_EMITTER_EXHAUST_WATER_Y_LIMIT == 0x100);
CTR_STATIC_ASSERT(VEH_EMITTER_EXHAUST_VEL_Y == 0x400);
CTR_STATIC_ASSERT(VEH_EMITTER_EXHAUST_VEL_Z == -0x400);
CTR_STATIC_ASSERT(VEH_EMITTER_EXHAUST_POS_X_NUM == 9);
CTR_STATIC_ASSERT(VEH_EMITTER_EXHAUST_POS_X_SHIFT == 3);
CTR_STATIC_ASSERT(VEH_EMITTER_EXHAUST_POS_Y_NUM == 7);
CTR_STATIC_ASSERT(VEH_EMITTER_EXHAUST_POS_Y_SHIFT == 1);
CTR_STATIC_ASSERT(VEH_EMITTER_EXHAUST_POS_Z_NUM == -0x38);
CTR_STATIC_ASSERT(VEH_EMITTER_EXHAUST_POS_Z_SHIFT == 4);
CTR_STATIC_ASSERT(VEH_EMITTER_EXHAUST_POS_SECOND_X_NUM == -0x12);
CTR_STATIC_ASSERT(VEH_EMITTER_EXHAUST_POS_SECOND_X_SHIFT == 4);
CTR_STATIC_ASSERT(VEH_EMITTER_TURBO_METER_COLOR_MIN == 129);
CTR_STATIC_ASSERT(VEH_EMITTER_TURBO_ROOM_WARNING_PAD == 2);
CTR_STATIC_ASSERT(VEH_EMITTER_TURBO_ROOM_WARNING_SCALE == 32);
CTR_STATIC_ASSERT(VEH_EMITTER_GROUND_SPARK_COUNT == 10);
CTR_STATIC_ASSERT(VEH_EMITTER_GROUND_SPARK_RNG_MASK == 0x7ff);
CTR_STATIC_ASSERT(VEH_EMITTER_TERRAIN_MIN_SPEED == 0x300);
CTR_STATIC_ASSERT(VEH_EMITTER_TERRAIN_NORMAL_TIRE_COUNT == 2);
CTR_STATIC_ASSERT(VEH_EMITTER_TERRAIN_DRIFT_TIRE_COUNT == 4);
CTR_STATIC_ASSERT(VEH_EMITTER_TIRE_LEFT_X == -0x1e);
CTR_STATIC_ASSERT(VEH_EMITTER_TIRE_RIGHT_X == 0x1e);
CTR_STATIC_ASSERT(VEH_EMITTER_TIRE_BACK_Z == -0x14);
CTR_STATIC_ASSERT(VEH_EMITTER_TIRE_FRONT_Z == 0x28);
CTR_STATIC_ASSERT(VEH_EMITTER_TERRAIN_TIRE_Y == 0x0a);
CTR_STATIC_ASSERT(VEH_EMITTER_SKID_LATERAL_SCALE == 15);
CTR_STATIC_ASSERT(VEH_EMITTER_SKID_WIDTH_SCALE == 10);
CTR_STATIC_ASSERT(VEH_EMITTER_SKID_COLOR_BIAS == 2);
CTR_STATIC_ASSERT(VEH_EMITTER_SKID_PREVIOUS_WIDTH_SHIFT == 1);
CTR_STATIC_ASSERT(VEH_EMITTER_SKID_HALF_LATERAL_SHIFT == 1);
CTR_STATIC_ASSERT(VEH_EMITTER_MUD_SPLASH_NORMAL_COUNT == 1);
CTR_STATIC_ASSERT(VEH_EMITTER_MUD_SPLASH_LANDING_COUNT == 10);
CTR_STATIC_ASSERT(VEH_EMITTER_MUD_SPLASH_ICON == 0x0d);
CTR_STATIC_ASSERT(VEH_EMITTER_MUD_SPLASH_VELOCITY_SHIFT == 4);
CTR_STATIC_ASSERT(VEH_EMITTER_MUD_SPLASH_VELOCITY_SCALE == 0x10);
CTR_STATIC_ASSERT(VEH_EMITTER_PARTICLE_WORLD_SCALE == 0x100);
CTR_STATIC_ASSERT(VEH_EMITTER_MUD_MIN_SPEED == 0x500);
CTR_STATIC_ASSERT(VEH_EMITTER_LANDING_SPARK_MIN_SPEED == 0x600);
CTR_STATIC_ASSERT(VEH_EMITTER_LANDING_SPARK_MIN_JUMP == 0x1600);
CTR_STATIC_ASSERT(VEH_EMITTER_TERRAIN_AUDIO_SPEED_MAX == 5000);
CTR_STATIC_ASSERT(VEH_EMITTER_TERRAIN_AUDIO_VOLUME_MAX == 200);
CTR_STATIC_ASSERT(VEH_EMITTER_TERRAIN_AUDIO_DISTORT_SPEED_MAX == 12000);
CTR_STATIC_ASSERT(VEH_EMITTER_TERRAIN_AUDIO_DISTORT_MIN == 0x6c);
CTR_STATIC_ASSERT(VEH_EMITTER_TERRAIN_AUDIO_DISTORT_MAX == 0xd2);
CTR_STATIC_ASSERT(VEH_EMITTER_TERRAIN_RUMBLE_MIN_SPEED == 0x200);
CTR_STATIC_ASSERT(VEH_EMITTER_LANDING_RUMBLE_FRAMES == 3);
CTR_STATIC_ASSERT(VEH_EMITTER_RUMBLE_FORCE_MAX == 0xff);
CTR_STATIC_ASSERT(VEH_EMITTER_WALL_RUB_TIMER_FULL == 0xf0);
CTR_STATIC_ASSERT(VEH_EMITTER_WALL_RUB_SFX == 0x14);
CTR_STATIC_ASSERT(VEH_EMITTER_WALL_RUB_VOLUME_STEP == 0x14);
CTR_STATIC_ASSERT(VEH_EMITTER_SKID_AUDIO_MIN_SPEED == 0x201);
CTR_STATIC_ASSERT(VEH_EMITTER_SKID_AUDIO_SPEED_MIN == 2000);
CTR_STATIC_ASSERT(VEH_EMITTER_SKID_AUDIO_SPEED_MAX == 12000);
CTR_STATIC_ASSERT(VEH_EMITTER_SKID_AUDIO_VOLUME_MIN == 0x14);
CTR_STATIC_ASSERT(VEH_EMITTER_SKID_AUDIO_VOLUME_MAX == 0xaa);
CTR_STATIC_ASSERT(VEH_EMITTER_SKID_AUDIO_DISTORT_SLOW == 0x92);
CTR_STATIC_ASSERT(VEH_EMITTER_SKID_AUDIO_DISTORT_FAST == 0x78);
CTR_STATIC_ASSERT(VEH_EMITTER_WALL_SPARK_MIN_SPEED == 0x200);
CTR_STATIC_ASSERT(VEH_EMITTER_WALL_RUMBLE_FRAME_LIMIT == 450);
CTR_STATIC_ASSERT(VEH_EMITTER_WALL_RUMBLE_FRAMES == 8);
CTR_STATIC_ASSERT(VEH_EMITTER_WALL_RUMBLE_FORCE == 0x7f);
CTR_STATIC_ASSERT(VEH_EMITTER_WALL_SPARK_LEFT_X == -0x2200);
CTR_STATIC_ASSERT(VEH_EMITTER_WALL_SPARK_RIGHT_X == 0x2200);
CTR_STATIC_ASSERT(VEH_EMITTER_WALL_SPARK_Y == 0x0a00);
CTR_STATIC_ASSERT(VEH_EMITTER_WALL_SPARK_REVERSE_Z == -0x1400);
CTR_STATIC_ASSERT(VEH_EMITTER_WALL_SPARK_FORWARD_Z == 0x2800);
CTR_STATIC_ASSERT(VEH_EMITTER_WALL_SPARK_SCRATCH_HALF_COUNT == 6);
CTR_STATIC_ASSERT(VEH_EMITTER_ALPHA_FULL == 0x1000);
CTR_STATIC_ASSERT(VEH_EMITTER_JOG_GROUND == 0x27);
CTR_STATIC_ASSERT(VEH_EMITTER_JOG_WOBBLE_ALT == 0xf0);
CTR_STATIC_ASSERT(VEH_EMITTER_JOG_WOBBLE_TIMER_MASK == 3);
CTR_STATIC_ASSERT(VEH_EMITTER_JOG_WOBBLE_DURATION == 0x100);
CTR_STATIC_ASSERT(VEH_EMITTER_JOG_AIR_LANDING_BOOST_MAX == 0x80);
CTR_STATIC_ASSERT(VEH_EMITTER_JOG_AIR_TURN_NEGATIVE == 0x12);
CTR_STATIC_ASSERT(VEH_EMITTER_JOG_AIR_TURN_POSITIVE == 0x22);
CTR_STATIC_ASSERT(VEH_EMITTER_JOG_AIR_DURATION == 0x20);

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80059100-0x80059344.
struct Particle *VehEmitter_Exhaust(struct Driver *d, VECTOR *exhaustPos, VECTOR *exhaustVel)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Instance *dInst = d->instSelf;

	if (d->invisibleTimer != 0)
	{
		return 0;
	}

	if ((dInst->flags & HIDE_MODEL) != 0)
	{
		return 0;
	}

	// low LOD exhaust (4p or ai car)
	int exhaustType = VEH_EMITTER_EXHAUST_ICON_LOW;
	struct ParticleEmitter *emSet = &data.emSet_Exhaust_Low[0];

	u8 numPlyr = gGT->numPlyrCurrGame;

	// equivalent of (d->driverID < numPlyr),
	// because modelIndex is not set to DYNAMIC_ROBOT_CAR
	// for human players after BOTS_Driver_Convert is called
	if (dInst->thread->modelIndex != DYNAMIC_ROBOT_CAR)
	{
		switch (numPlyr)
		{
		case 1:
			// 1P mode, high LOD exhaust
			emSet = &data.emSet_Exhaust_High[0];
			break;
		case 2:
			// 2P mode, med LOD exhaust
			emSet = &data.emSet_Exhaust_Med[0];
			break;
		}
	}

	if (((dInst->flags & SPLIT_LINE) != 0) && ((exhaustPos->vy - exhaustVel->vy) + d->posCurr.y < VEH_EMITTER_EXHAUST_WATER_Y_LIMIT))
	{
		// bubble texture
		exhaustType = VEH_EMITTER_EXHAUST_ICON_WATER;
		emSet = &data.emSet_Exhaust_Water[0];
	}

	struct Particle *p = Particle_Init(0, gGT->iconGroup[exhaustType], emSet);

	if (p == NULL)
	{
		return p;
	}

	p->axis[0].startVal += exhaustPos->vx - exhaustVel->vx;
	p->axis[0].velocity = (s16)exhaustVel->vx;
	p->axis[1].startVal += exhaustPos->vy - exhaustVel->vy;
	p->axis[1].velocity = (s16)exhaustVel->vy;
	p->axis[2].startVal += exhaustPos->vz - exhaustVel->vz;
	p->axis[2].velocity = (s16)exhaustVel->vz;

	p->driverInst = dInst;
	p->otIndexOffset = dInst->depthBiasNormal;

	if (exhaustType == VEH_EMITTER_EXHAUST_ICON_WATER)
	{
		p->funcPtr = Particle_FuncPtr_ExhaustUnderwater;
	}

	// if engine revving
	if (d->kartState == KS_ENGINE_REVVING)
	{
		if (d->revEngineState != 1)
		{
			return p;
		}
	}

	// if not engine revving
	else
	{
		s16 meterLeft = d->turbo_MeterRoomLeft;
		if ((meterLeft < VEH_EMITTER_TURBO_METER_COLOR_MIN) ||
		    (((d->const_turboLowRoomWarning + VEH_EMITTER_TURBO_ROOM_WARNING_PAD) * VEH_EMITTER_TURBO_ROOM_WARNING_SCALE) < meterLeft))
		{
			return p;
		}
	}

	p->flagsSetColor &= ~PARTICLE_SET_COLOR_FLAG_DRAW_MODE_MASK;
	p->flagsSetColor |= PARTICLE_SET_COLOR_FLAG_DRAW_MODE_40;

	return p;
}

static const SVECTOR sparkGround_inX = {0x1800, 0, 0, 0};
static const SVECTOR sparkGround_inZ = {0, 0, -0x1800, 0};
static const SVECTOR sparkGround_inZ2 = {0, 0, -0x200, 0};

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80059344-0x80059558.
void VehEmitter_Sparks_Ground(struct Driver *d, struct ParticleEmitter *emSet)
{
	struct GameTracker *gGT = sdata->gGT;

	Vec3 outX;
	Vec3 outZ;
	Vec3 outZ2;

	CTR_GteLoadSV0(&sparkGround_inX);
	gte_rtv0();
	CTR_GteStoreMAC(outX.v);

	CTR_GteLoadSV0(&sparkGround_inZ);
	gte_rtv0();
	CTR_GteStoreMAC(outZ.v);

	CTR_GteLoadSV0(&sparkGround_inZ2);
	gte_rtv0();
	CTR_GteStoreMAC(outZ2.v);

	for (int i = 0; i < VEH_EMITTER_GROUND_SPARK_COUNT; i++)
	{
		// Create instance in particle pool
		struct Particle *p = Particle_Init(0, gGT->iconGroup[0], emSet);

		if (p == NULL)
		{
			continue;
		}

		u32 rng = (u32)(RngDeadCoed(&gGT->deadcoed_struct) & VEH_EMITTER_GROUND_SPARK_RNG_MASK);

		if ((rng & 1) != 0)
		{
			rng = -rng;
		}

		for (int j = 0; j < VEH_EMITTER_AXIS_COUNT; j++)
		{
			p->axis[j].velocity += (s16)outZ2.v[j] + (s16)((rng * outX.v[j]) >> FRACTIONAL_BITS);
			p->axis[j].startVal += outZ.v[j] + p->axis[j].velocity;
		}

		p->driverInst = d->instSelf;
		p->otIndexOffset = d->instSelf->depthBiasNormal;
	}
}

static const SVECTOR terrainEmitterPos[4] = {
    {VEH_EMITTER_TIRE_RIGHT_X, VEH_EMITTER_TERRAIN_TIRE_Y, VEH_EMITTER_TIRE_BACK_Z, 0},
    {VEH_EMITTER_TIRE_LEFT_X, VEH_EMITTER_TERRAIN_TIRE_Y, VEH_EMITTER_TIRE_BACK_Z, 0},
    {VEH_EMITTER_TIRE_RIGHT_X, VEH_EMITTER_TERRAIN_TIRE_Y, VEH_EMITTER_TIRE_FRONT_Z, 0},
    {VEH_EMITTER_TIRE_LEFT_X, VEH_EMITTER_TERRAIN_TIRE_Y, VEH_EMITTER_TIRE_FRONT_Z, 0},
};

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80059558-0x80059780.
void VehEmitter_Terrain_Ground(struct Driver *d, struct ParticleEmitter *emSet)
{
	int speed;
	int numTires;
	Vec3 pos;
	Vec3 vel;

	int flags = d->actionsFlagSet;

	if ((flags & ACTION_TOUCH_GROUND) == 0)
	{
		return;
	}

	if ((flags & ACTION_ACCEL_PREVENTION) != 0)
	{
		return;
	}

	speed = d->fireSpeed;
	if (speed < 0)
	{
		speed = -speed;
	}
	if (speed < VEH_EMITTER_TERRAIN_MIN_SPEED)
	{
		speed = d->speedApprox;
		if (speed < 0)
		{
			speed = -speed;
		}
		if (speed < VEH_EMITTER_TERRAIN_MIN_SPEED)
		{
			return;
		}
	}

	// if sliding, spawn on 4 tires, otherwise just 2
	numTires = (d->kartState == KS_DRIFTING) ? VEH_EMITTER_TERRAIN_DRIFT_TIRE_COUNT : VEH_EMITTER_TERRAIN_NORMAL_TIRE_COUNT;

	struct Instance *dInst = d->instSelf;
	struct IconGroup *ig = sdata->gGT->iconGroup[0];

	// spawn particles on wheels
	for (; numTires != 0; numTires--)
	{
		CTR_GteLoadSV0(&terrainEmitterPos[numTires - 1]);
		gte_rtv0();
		CTR_GteStoreMAC(pos.v);

		struct Particle *p = Particle_Init(0, ig, emSet);

		if (p == NULL)
		{
			continue;
		}

		SVECTOR velInput = {p->axis[0].velocity, p->axis[1].velocity, p->axis[2].velocity, 0};

		CTR_GteLoadSV0(&velInput);
		gte_rtv0();
		CTR_GteStoreMAC(vel.v);

		for (int i = 0; i < VEH_EMITTER_AXIS_COUNT; i++)
		{
			p->axis[i].startVal += pos.v[i] * VEH_EMITTER_PARTICLE_WORLD_SCALE;
			p->axis[i].velocity = (s16)vel.v[i];
		}

		p->driverInst = dInst;
		p->otIndexOffset = dInst->depthBiasNormal;
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80059780-0x80059a18.
void VehEmitter_Sparks_Wall(struct Driver *d, struct ParticleEmitter *emSet)
{
	int speedAbs = d->speedApprox;
	if (speedAbs < 0)
	{
		speedAbs = -speedAbs;
	}

	// must have speed, or gas pedal, for vibration
	if (((d->fireSpeed != 0) || (speedAbs > VEH_EMITTER_WALL_SPARK_MIN_SPEED)) && (d->frameAgainstWall < VEH_EMITTER_WALL_RUMBLE_FRAME_LIMIT))
	{
		// both gamepad vibration
		GAMEPAD_ShockFreq(d, VEH_EMITTER_WALL_RUMBLE_FRAMES, 0);
		GAMEPAD_ShockForce1(d, VEH_EMITTER_WALL_RUMBLE_FRAMES, VEH_EMITTER_WALL_RUMBLE_FORCE);

		d->frameAgainstWall++;
	}
	else
	{
		d->frameAgainstWall = 0;
	}

	// must reach minimum speed for sparks
	if (speedAbs <= VEH_EMITTER_WALL_SPARK_MIN_SPEED)
	{
		return;
	}

	union VehEmitterWallScratch *scratch = CTR_SCRATCHPAD_PTR(union VehEmitterWallScratch, 0);
	s32 *tireLeftOutWord = &scratch->word[0];
	s32 *tireRightOutWord = &scratch->word[3];
	s16 *tireLeftOutHalf = &scratch->half[0];
	s16 *tireRightOutHalf = &scratch->half[3];
	s16 *distIn4 = &scratch->half[6];
	s32 *distOut4 = &scratch->word[3];

	// s16[3] array
	tireLeftOutWord[0] = (s32)CTR_PackS16Pair(VEH_EMITTER_WALL_SPARK_LEFT_X, VEH_EMITTER_WALL_SPARK_Y);
	tireRightOutWord[0] = (s32)CTR_PackS16Pair(VEH_EMITTER_WALL_SPARK_RIGHT_X, VEH_EMITTER_WALL_SPARK_Y);

	int valZ = VEH_EMITTER_WALL_SPARK_REVERSE_Z;
	if (d->speedApprox > 0)
	{
		valZ = VEH_EMITTER_WALL_SPARK_FORWARD_Z;
	}

	// s16[3] array
	tireLeftOutWord[1] = valZ;
	tireRightOutWord[1] = valZ;

	CTR_GteLoadS16TripletV0(tireLeftOutHalf);
	gte_rtv0();
	CTR_GteStoreMAC(&tireLeftOutWord[0]);

	CTR_GteLoadS16TripletV0(tireRightOutHalf);
	gte_rtv0();
	CTR_GteStoreMAC(&tireRightOutWord[0]);

	// this compresses TireLeft and TireRight from int to s16,
	// which then doubles in usage as a matrix (3x2)
	for (int i = 0; i < VEH_EMITTER_WALL_SPARK_SCRATCH_HALF_COUNT; i++)
	{
		tireLeftOutHalf[i] = (u16)scratch->word[i];
	}

#ifdef CTR_NATIVE

#define gte_SetLightMatrix3x2(r0)                  \
	{                                              \
		CTC2(CTR_ReadU32LE((char *)(r0)), 8);      \
		CTC2(CTR_ReadU32LE((char *)(r0) + 4), 9);  \
		CTC2(CTR_ReadU32LE((char *)(r0) + 8), 10); \
	}

#else

#define gte_SetLightMatrix3x2(r0)        \
	__asm__ volatile("lw		$t0, 0( %0 );" \
	                 "lw		$t1, 4( %0 );" \
	                 "lw		$t2, 8( %0 );" \
	                 "ctc2	$t0, $8;"     \
	                 "ctc2	$t1, $9;"     \
	                 "ctc2	$t2, $10;"    \
	                 :                   \
	                 : "r"(r0)           \
	                 : "$t2")

#endif

	gte_SetLightMatrix3x2(&scratch->half[0]);

	// dist4 is actual distance
	distIn4[0] = (d->posWallColl.x * VEH_EMITTER_PARTICLE_WORLD_SCALE) - d->posCurr.x;
	distIn4[1] = (d->posWallColl.y * VEH_EMITTER_PARTICLE_WORLD_SCALE) - d->posCurr.y;
	distIn4[2] = (d->posWallColl.z * VEH_EMITTER_PARTICLE_WORLD_SCALE) - d->posCurr.z;

	CTR_GteLoadS16TripletV0(&distIn4[0]);
	gte_llv0();
	CTR_GteStoreMAC(&distOut4[0]);
	if (distOut4[0] < distOut4[1])
	{
		tireLeftOutHalf = tireRightOutHalf;
	}

	// Create instance in particle pool
	struct Particle *p = Particle_Init(0, sdata->gGT->iconGroup[0], emSet);

	if (p == NULL)
	{
		return;
	}

	for (int i = 0; i < VEH_EMITTER_AXIS_COUNT; i++)
	{
		p->axis[i].startVal += tireLeftOutHalf[i];
		distIn4[i] = p->axis[i].velocity;
	}

	// dist4 now determines velocity
	CTR_GteLoadS16TripletV0(&distIn4[0]);
	gte_rtv0();
	CTR_GteStoreMAC(&distOut4[0]);

	p->axis[0].velocity = (s16)distOut4[0];
	p->axis[1].velocity = (s16)distOut4[1];
	p->axis[2].velocity = (s16)distOut4[2];

	p->driverInst = d->instSelf;
}

static void VehEmitter_SetRotTransMatrix(MATRIX *m)
{
	gte_SetRotMatrix(m);
	gte_SetTransMatrix(m);
}

static void VehEmitter_RotVec(const SVECTOR *in, VECTOR *out)
{
	CTR_GteLoadSV0(in);
	gte_rtv0();
	CTR_GteStoreMAC(&out->vx);
}

static void VehEmitter_RotTransVec(const SVECTOR *in, VECTOR *out)
{
	CTR_GteLoadSV0(in);
	gte_rt();
	CTR_GteStoreMAC(&out->vx);
}

static union VehEmitterSkidmark *VehEmitter_GetSkidmark(struct Driver *d, u8 frameIndex, int tireIndex)
{
	return &d->skidmarks[frameIndex & DRIVER_SKIDMARK_FRAME_INDEX_MASK][tireIndex];
}

static void VehEmitter_WriteSkidmark(struct Driver *d, u8 frameIndex, int tireIndex, int x, int y, int z, int widthX, int widthZ, u8 color, u8 flags)
{
	union VehEmitterSkidmark *mark = VehEmitter_GetSkidmark(d, frameIndex, tireIndex);

	mark->edge0.x = (s16)(x + widthX);
	mark->edge0.y = (s16)y;
	mark->edge0.z = (s16)(z + widthZ);
	mark->color = color;
	mark->flags = flags;
	mark->edge1.x = (s16)(x - widthX);
	mark->edge1.y = (s16)y;
	mark->edge1.z = (s16)(z - widthZ);
}

static void VehEmitter_WriteSkidmarkPair(struct Driver *d, int tireIndex, int x, int y, int z, int lateralX, int lateralZ, int widthX, int widthZ, u8 color,
                                         u8 flags)
{
	u8 frame = (u8)d->skidmarkFrameIndex;

	VehEmitter_WriteSkidmark(d, frame, tireIndex, x, y, z, widthX, widthZ, color, flags);

	x += lateralX;
	z += lateralZ;
	VehEmitter_WriteSkidmark(d, (u8)(frame - 1), tireIndex, x, y, z, widthX >> VEH_EMITTER_SKID_PREVIOUS_WIDTH_SHIFT,
	                         widthZ >> VEH_EMITTER_SKID_PREVIOUS_WIDTH_SHIFT, color, flags);
}

static void VehEmitter_Skidmarks(struct Thread *thread, struct Driver *d, TerrainFlags terrainFlags)
{
	struct Instance *inst = thread->inst;
	MATRIX *m = &inst->matrix;
	u8 color = ((inst->flags & SPLIT_LINE) == 0) ? inst->depthBiasNormal : inst->depthBiasSecondary;
	u8 flags = ((terrainFlags & TERRAIN_FLAG_FORCE_SKIDMARKS) == 0) ? 0 : 1;
	int sin = MATH_Sin(d->axisRotationX);
	int cos = MATH_Cos(d->axisRotationX);
	int lateralX = (sin * VEH_EMITTER_SKID_LATERAL_SCALE) >> FRACTIONAL_BITS;
	int lateralZ = (cos * VEH_EMITTER_SKID_LATERAL_SCALE) >> FRACTIONAL_BITS;
	int widthX = (cos * VEH_EMITTER_SKID_WIDTH_SCALE) >> FRACTIONAL_BITS;
	int widthZ = (-sin * VEH_EMITTER_SKID_WIDTH_SCALE) >> FRACTIONAL_BITS;
	VECTOR pos;

	color += VEH_EMITTER_SKID_COLOR_BIAS;

	VehEmitter_SetRotTransMatrix(m);

	if ((d->actionsFlagSet & ACTION_BACK_SKID) != 0)
	{
		d->skidmarkEnableFlags |= DRIVER_SKIDMARK_BACK_LEFT;

		SVECTOR local = {VEH_EMITTER_TIRE_LEFT_X, 0, VEH_EMITTER_TIRE_BACK_Z, 0};
		VehEmitter_RotTransVec(&local, &pos);
		VehEmitter_WriteSkidmarkPair(d, 0, pos.vx - (lateralX >> VEH_EMITTER_SKID_HALF_LATERAL_SHIFT), pos.vy,
		                             pos.vz - (lateralZ >> VEH_EMITTER_SKID_HALF_LATERAL_SHIFT), lateralX, lateralZ, widthX, widthZ, color, flags);

		d->skidmarkEnableFlags |= DRIVER_SKIDMARK_BACK_RIGHT;

		local.vx = VEH_EMITTER_TIRE_RIGHT_X;
		VehEmitter_RotTransVec(&local, &pos);
		VehEmitter_WriteSkidmarkPair(d, 1, pos.vx - (lateralX >> VEH_EMITTER_SKID_HALF_LATERAL_SHIFT), pos.vy,
		                             pos.vz - (lateralZ >> VEH_EMITTER_SKID_HALF_LATERAL_SHIFT), lateralX, lateralZ, widthX, widthZ, color, flags);
	}

	if ((d->actionsFlagSet & ACTION_FRONT_SKID) != 0)
	{
		d->skidmarkEnableFlags |= DRIVER_SKIDMARK_FRONT_LEFT;

		SVECTOR local = {VEH_EMITTER_TIRE_LEFT_X, 0, VEH_EMITTER_TIRE_FRONT_Z, 0};
		VehEmitter_RotTransVec(&local, &pos);
		VehEmitter_WriteSkidmarkPair(d, 2, pos.vx - (lateralX >> VEH_EMITTER_SKID_HALF_LATERAL_SHIFT), pos.vy,
		                             pos.vz - (lateralZ >> VEH_EMITTER_SKID_HALF_LATERAL_SHIFT), lateralX, lateralZ, widthX, widthZ, color, flags);

		d->skidmarkEnableFlags |= DRIVER_SKIDMARK_FRONT_RIGHT;

		local.vx = VEH_EMITTER_TIRE_RIGHT_X;
		VehEmitter_RotTransVec(&local, &pos);
		VehEmitter_WriteSkidmarkPair(d, 3, pos.vx - (lateralX >> VEH_EMITTER_SKID_HALF_LATERAL_SHIFT), pos.vy,
		                             pos.vz - (lateralZ >> VEH_EMITTER_SKID_HALF_LATERAL_SHIFT), lateralX, lateralZ, widthX, widthZ, color, flags);
	}
}

static void VehEmitter_MudSplash(struct Driver *d)
{
	struct GameTracker *gGT = sdata->gGT;
	int count = ((d->actionsFlagSet & ACTION_STARTED_TOUCH_GROUND) == 0) ? VEH_EMITTER_MUD_SPLASH_NORMAL_COUNT : VEH_EMITTER_MUD_SPLASH_LANDING_COUNT;

	for (; count != 0; count--)
	{
		struct Particle *p = Particle_Init(0, gGT->iconGroup[VEH_EMITTER_MUD_SPLASH_ICON], &data.emSet_MudSplash[0]);

		if (p == NULL)
		{
			continue;
		}

		p->otIndexOffset = d->instSelf->depthBiasNormal;
		p->driverInst = d->instSelf;
		p->driverID = d->driverID;

		p->axis[0].startVal += (int)p->axis[0].velocity * VEH_EMITTER_MUD_SPLASH_VELOCITY_SCALE;
		p->axis[2].startVal += (int)p->axis[2].velocity * VEH_EMITTER_MUD_SPLASH_VELOCITY_SCALE;
		p->axis[0].accel -= p->axis[0].velocity >> VEH_EMITTER_MUD_SPLASH_VELOCITY_SHIFT;
		p->axis[2].accel -= p->axis[2].velocity >> VEH_EMITTER_MUD_SPLASH_VELOCITY_SHIFT;
	}
}

static void VehEmitter_TerrainEffects(struct Thread *thread, struct Driver *d, struct Terrain *terrain, TerrainFlags terrainFlags, int absSpeedApprox)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Instance *inst = thread->inst;
	MATRIX *m = &inst->matrix;

	if (gGT->numPlyrCurrGame >= 2)
	{
		return;
	}

	int absSpeed = d->speed;
	if (absSpeed < 0)
	{
		absSpeed = -absSpeed;
	}

	if ((absSpeed > VEH_EMITTER_MUD_MIN_SPEED) && (d->currentTerrain == TERRAIN_MUD))
	{
		VehEmitter_MudSplash(d);
	}

	if (((terrainFlags & TERRAIN_FLAG_LANDING_SPARKS) != 0) && ((d->actionsFlagSet & ACTION_STARTED_TOUCH_GROUND) != 0) &&
	    (absSpeedApprox > VEH_EMITTER_LANDING_SPARK_MIN_SPEED))
	{
		int absJump = d->jumpHeightPrev;
		if (absJump < 0)
		{
			absJump = -absJump;
		}

		if (absJump > VEH_EMITTER_LANDING_SPARK_MIN_JUMP)
		{
			VehEmitter_SetRotTransMatrix(m);
			VehEmitter_Sparks_Ground(d, &data.emSet_GroundSparks[0]);
		}
	}

	if (terrain->em_OddFrame != NULL)
	{
		struct ParticleEmitter *emSet = terrain->em_OddFrame;

		if ((terrain->em_EvenFrame != NULL) && ((gGT->timer & 1) != 0))
		{
			emSet = terrain->em_EvenFrame;
		}

		VehEmitter_SetRotTransMatrix(m);
		VehEmitter_Terrain_Ground(d, emSet);
	}

	int wallSound = VEH_EMITTER_WALL_RUB_SFX;
	s32 engineVol = (s16)d->engineVol;

	if ((d->wallRubTimer == VEH_EMITTER_WALL_RUB_TIMER_FULL) && (d->kartState != KS_MASK_GRABBED))
	{
		VehEmitter_SetRotTransMatrix(m);
		VehEmitter_Sparks_Wall(d, &data.emSet_WallSparks[0]);

		engineVol += VEH_EMITTER_WALL_RUB_VOLUME_STEP;
		if (engineVol > HOWL_SFX_VOLUME_MAX)
		{
			engineVol = HOWL_SFX_VOLUME_MAX;
		}
	}
	else
	{
		if (d->wallRubTimer == 0)
		{
			d->frameAgainstWall = 0;
		}

		engineVol -= VEH_EMITTER_WALL_RUB_VOLUME_STEP;
		if (engineVol < 0)
		{
			engineVol = 0;
		}

		if (engineVol == 0)
		{
			wallSound = -1;
		}
	}

	d->engineVol = (u16)engineVol;

	if (thread->modelIndex == DYNAMIC_PLAYER)
	{
		u32 echo = ((d->actionsFlagSet & ACTION_ENGINE_ECHO) != 0);
		u32 flags = HowlSfx_Pack(HOWL_SFX_LR_CENTER, HOWL_SFX_DISTORTION_NONE, (u32)(s16)d->engineVol, echo);

		OtherFX_RecycleNew(&d->driverAudioPtrs[2], wallSound, flags);
	}
}

static void VehEmitter_TerrainAudioAndFeedback(struct Thread *thread, struct Driver *d, struct Terrain *terrain, TerrainFlags terrainFlags, int absSpeedApprox)
{
	if (thread->modelIndex != DYNAMIC_PLAYER)
	{
		return;
	}

	int soundID = -1;

	if (((d->actionsFlagSet & ACTION_TOUCH_GROUND) != 0) && ((terrainFlags & TERRAIN_FLAG_ONESHOT_GROUND_SOUND) == 0))
	{
		soundID = terrain->sound;
	}

	int vol = VehCalc_MapToRange(absSpeedApprox, 0, VEH_EMITTER_TERRAIN_AUDIO_SPEED_MAX, 0, VEH_EMITTER_TERRAIN_AUDIO_VOLUME_MAX);
	int distort = VehCalc_MapToRange(absSpeedApprox, 0, VEH_EMITTER_TERRAIN_AUDIO_DISTORT_SPEED_MAX, VEH_EMITTER_TERRAIN_AUDIO_DISTORT_MIN,
	                                 VEH_EMITTER_TERRAIN_AUDIO_DISTORT_MAX);
	u32 echo = ((d->actionsFlagSet & ACTION_ENGINE_ECHO) != 0);
	OtherFX_RecycleNew(&d->driverAudioPtrs[1], soundID, HowlSfx_Pack(HOWL_SFX_LR_CENTER, (u32)distort, (u32)vol, echo));

	if ((d->actionsFlagSet & ACTION_BOT) == 0)
	{
		if (absSpeedApprox > VEH_EMITTER_TERRAIN_RUMBLE_MIN_SPEED)
		{
			GAMEPAD_ShockFreq(d, terrain->vibrationData[0], terrain->vibrationData[1]);
			GAMEPAD_ShockForce2(d, terrain->vibrationData[2], terrain->vibrationData[3]);
		}

		if ((d->actionsFlagSet & ACTION_STARTED_TOUCH_GROUND) != 0)
		{
			int absJump = d->jumpHeightPrev;
			if (absJump < 0)
			{
				absJump = -absJump;
			}

			if (absJump > VEH_EMITTER_LANDING_SPARK_MIN_JUMP)
			{
				GAMEPAD_ShockForce1(d, VEH_EMITTER_LANDING_RUMBLE_FRAMES, VEH_EMITTER_RUMBLE_FORCE_MAX);
			}
		}
	}
}

static void VehEmitter_SkidmarkAudio(struct Thread *thread, struct Driver *d, struct Terrain *terrain, TerrainFlags terrainFlags, int absSpeedApprox)
{
	if (((d->actionsFlagSet & ACTION_TOUCH_GROUND) == 0) || ((d->actionsFlagSet & (ACTION_BACK_SKID | ACTION_FRONT_SKID)) == 0) ||
	    (absSpeedApprox < VEH_EMITTER_SKID_AUDIO_MIN_SPEED))
	{
		if (d->driverAudioPtrs[0] != 0)
		{
			OtherFX_Stop1((int)d->driverAudioPtrs[0]);
			d->driverAudioPtrs[0] = 0;
		}
		return;
	}

	if (thread->modelIndex == DYNAMIC_PLAYER)
	{
		int absTurn = d->simpTurnState;
		if (absTurn < 0)
		{
			absTurn = -absTurn;
		}

		int vol = VehCalc_MapToRange(absSpeedApprox, VEH_EMITTER_SKID_AUDIO_SPEED_MIN, VEH_EMITTER_SKID_AUDIO_SPEED_MAX, VEH_EMITTER_SKID_AUDIO_VOLUME_MIN,
		                             VEH_EMITTER_SKID_AUDIO_VOLUME_MAX);
		int distort = VehCalc_MapToRange(absSpeedApprox, VEH_EMITTER_SKID_AUDIO_SPEED_MIN, VEH_EMITTER_SKID_AUDIO_SPEED_MAX,
		                                 VEH_EMITTER_SKID_AUDIO_DISTORT_SLOW, VEH_EMITTER_SKID_AUDIO_DISTORT_FAST);

		if (d->kartState == KS_DRIFTING)
		{
			int drift = d->turnWobbleAngle;
			if (drift < 0)
			{
				drift = -drift;
			}

			distort -= drift;
			if (distort < 0)
			{
				distort = 0;
			}
		}

		distort += absTurn;
		if (distort > VEH_EMITTER_SKID_AUDIO_DISTORT_SLOW)
		{
			distort = VEH_EMITTER_SKID_AUDIO_DISTORT_SLOW;
		}

		int panOffset = CTR_MipsSra(CTR_MipsSll((u8)d->simpTurnState, 24), 26);
		u32 lr = (u32)CTR_MipsSubLo(HOWL_SFX_LR_CENTER, panOffset);
		u32 echo = ((d->actionsFlagSet & ACTION_ENGINE_ECHO) != 0);
		u32 flags = HowlSfx_Pack(lr, (u32)distort, (u32)(vol + (absTurn >> 1)), echo);

		OtherFX_RecycleNew(&d->driverAudioPtrs[0], terrain->skidSound, flags);
	}

	VehEmitter_Skidmarks(thread, d, terrainFlags);
}

static int VehEmitter_ShouldSkipExhaust(struct Thread *thread, struct Driver *d)
{
	struct GameTracker *gGT = sdata->gGT;

	if (thread->modelIndex == DYNAMIC_ROBOT_CAR)
	{
		if ((gGT->timer & 3) != (d->driverID & 3))
		{
			return 1;
		}
	}
	else
	{
		if (d->revEngineState == 2)
		{
			return 1;
		}

		u32 numPlyr = gGT->numPlyrCurrGame;

		if (numPlyr > 1)
		{
			if (((numPlyr != 2) || ((gGT->timer & 1) != d->driverID)) && ((gGT->timer & 3) != d->driverID))
			{
				return 1;
			}
		}

		if (d->failedBoostExhaustTimer == 0)
		{
			int meterLeft = d->turbo_MeterRoomLeft;

			if ((meterLeft < VEH_EMITTER_TURBO_METER_COLOR_MIN) ||
			    (((d->const_turboLowRoomWarning + VEH_EMITTER_TURBO_ROOM_WARNING_PAD) * VEH_EMITTER_TURBO_ROOM_WARNING_SCALE) < meterLeft))
			{
				if (PROC_SearchForModel(thread->childThread, STATIC_TURBO_EFFECT) != NULL)
				{
					return 1;
				}
			}
		}
	}

	if (d->failedBoostExhaustTimer != 0)
	{
		d->failedBoostExhaustTimer--;
	}

	return 0;
}

static void VehEmitter_ExhaustPair(struct Thread *thread, struct Driver *d)
{
	struct Instance *inst = thread->inst;
	MATRIX *m = &inst->matrix;
	SVECTOR local;
	VECTOR exhaustPos;
	VECTOR exhaustVel;

	gte_SetRotMatrix(m);

	local.vx = 0;
	local.vy = VEH_EMITTER_EXHAUST_VEL_Y;
	local.vz = VEH_EMITTER_EXHAUST_VEL_Z;
	local.pad = 0;
	VehEmitter_RotVec(&local, &exhaustVel);

	local.vx = (s16)((inst->scale.x * VEH_EMITTER_EXHAUST_POS_X_NUM) >> VEH_EMITTER_EXHAUST_POS_X_SHIFT);
	local.vy = (s16)((inst->scale.y * VEH_EMITTER_EXHAUST_POS_Y_NUM) >> VEH_EMITTER_EXHAUST_POS_Y_SHIFT);
	local.vz = (s16)((inst->scale.z * VEH_EMITTER_EXHAUST_POS_Z_NUM) >> VEH_EMITTER_EXHAUST_POS_Z_SHIFT);
	VehEmitter_RotVec(&local, &exhaustPos);
	VehEmitter_Exhaust(d, &exhaustPos, &exhaustVel);

	local.vx = (s16)((inst->scale.x * VEH_EMITTER_EXHAUST_POS_SECOND_X_NUM) >> VEH_EMITTER_EXHAUST_POS_SECOND_X_SHIFT);
	VehEmitter_RotVec(&local, &exhaustPos);
	VehEmitter_Exhaust(d, &exhaustPos, &exhaustVel);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80059a18-0x8005ab24
void VehEmitter_DriverMain(struct Thread *thread, struct Driver *d)
{
	struct Terrain *terrain = d->terrainMeta1;
	TerrainFlags terrainFlags = terrain->flags;
	int absSpeedApprox = d->speedApprox;

	d->skidmarkEnableFlags = (d->skidmarkEnableFlags & DRIVER_SKIDMARK_HISTORY_MASK) << DRIVER_SKIDMARK_HISTORY_SHIFT;
	d->skidmarkFrameIndex = (d->skidmarkFrameIndex - 1) & DRIVER_SKIDMARK_FRAME_INDEX_MASK;

	if (absSpeedApprox < 0)
	{
		absSpeedApprox = -absSpeedApprox;
	}

	VehEmitter_TerrainAudioAndFeedback(thread, d, terrain, terrainFlags, absSpeedApprox);
	VehEmitter_TerrainEffects(thread, d, terrain, terrainFlags, absSpeedApprox);

	if ((terrainFlags & TERRAIN_FLAG_FORCE_SKIDMARKS) != 0)
	{
		d->actionsFlagSet |= ACTION_BACK_SKID | ACTION_FRONT_SKID;
	}

	if ((d->matrixArray > BAKED_GTE_MATRIX_NONE) && (d->matrixArray <= BAKED_GTE_MATRIX_WHEELIE_RECOVER))
	{
		if (d->matrixArray == BAKED_GTE_MATRIX_WHEELIE_START)
		{
			d->actionsFlagSet |= ACTION_BACK_SKID;
		}

		d->actionsFlagSet &= ~ACTION_FRONT_SKID;
	}

	VehEmitter_SkidmarkAudio(thread, d, terrain, terrainFlags, absSpeedApprox);

	if (!VehEmitter_ShouldSkipExhaust(thread, d))
	{
		VehEmitter_ExhaustPair(thread, d);
	}

	if (d->burnTimer != 0)
	{
		d->alphaScaleBackup = VEH_EMITTER_ALPHA_FULL;
		d->instSelf->alphaScale = VEH_EMITTER_ALPHA_FULL;
	}

	if (d->invisibleTimer != 0)
	{
		thread->inst->alphaScale = VEH_EMITTER_ALPHA_FULL;
	}

	if ((d->kartState != KS_NORMAL) && (d->kartState != KS_DRIFTING))
	{
		d->actionsFlagSet &= ~ACTION_AIRBORNE;
	}

	if ((d->kartState == KS_ENGINE_REVVING) || (d->kartState == KS_MASK_GRABBED) || ((d->actionsFlagSet & ACTION_TOUCH_GROUND) != 0))
	{
		GAMEPAD_JogCon2(d, VEH_EMITTER_JOG_GROUND, 0);

		if (d->turnWobbleAngle == 0)
		{
			return;
		}

		int jogValue = ((sdata->gGT->timer & VEH_EMITTER_JOG_WOBBLE_TIMER_MASK) == 0) ? VEH_EMITTER_JOG_GROUND : VEH_EMITTER_JOG_WOBBLE_ALT;
		GAMEPAD_JogCon2(d, jogValue, VEH_EMITTER_JOG_WOBBLE_DURATION);
		return;
	}

	if (d->jump_LandingBoost < VEH_EMITTER_JOG_AIR_LANDING_BOOST_MAX)
	{
		int jogValue = VEH_EMITTER_JOG_AIR_TURN_NEGATIVE;

		if ((d->simpTurnState < 0) || ((jogValue = VEH_EMITTER_JOG_AIR_TURN_POSITIVE), (d->simpTurnState > 0)))
		{
			GAMEPAD_JogCon1(d, jogValue, VEH_EMITTER_JOG_AIR_DURATION);
		}
	}

	GAMEPAD_JogCon2(d, 0, 0);
}
