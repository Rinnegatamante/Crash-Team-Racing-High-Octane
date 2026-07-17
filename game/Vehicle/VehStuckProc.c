#include <common.h>

enum
{
	VEH_STUCK_MASK_BSP_PROBE_HEIGHT = 0x100,
	VEH_STUCK_RESPAWN_Y_OFFSET = 0x80,
	VEH_STUCK_RESPAWN_PLAYER_CLEARANCE_XZ = 0x2000,
	VEH_STUCK_MASK_HEAD_Y_OFFSET = 0x140,
	VEH_STUCK_REV_MASK_RELEASE_HEIGHT = 0x4000,
	VEH_STUCK_REV_MASK_GRAB_HEIGHT_TRIGGER = 0x1000,
	VEH_STUCK_REV_SMALL_BOOST_FIRE_LEVEL = 0x20,
	VEH_STUCK_REV_BIG_BOOST_FIRE_LEVEL = 0x80,
	VEH_STUCK_REV_START_BOOST_RESERVES = 960,
	VEH_STUCK_REV_MASK_DESCENT_STEP = 0x200,
	VEH_STUCK_REV_CAMERA_HEIGHT_OFFSET = 0x40,
	VEH_STUCK_REV_OVERREV_FX = 0xf,
	VEH_STUCK_REV_FILL_STEP_MAX = 5000,
	VEH_STUCK_REV_STEP_MIN = 0x100,
	VEH_STUCK_REV_OVERREV_TIMEOUT = 192,
	VEH_STUCK_REV_RELEASE_COOLDOWN = 0x100,
	VEH_STUCK_REV_DECAY_STEP_MAX_NORMAL = 1000,
	VEH_STUCK_REV_DECAY_STEP_MAX_LOCKED = 3000,
	VEH_STUCK_REV_EMPTY_COOLDOWN = 0xc0,
	VEH_STUCK_REV_TURBO_ROOM_SHIFT = 5,
	VEH_STUCK_REV_SQUISH_SHIFT = 6,
	VEH_STUCK_REV_SQUISH_LIMIT = 0x401,
	VEH_STUCK_REV_SQUISH_MAX = 0x400,
	VEH_STUCK_REV_MODEL_BASE_SCALE = 3276,
	VEH_STUCK_REV_MODEL_WIDTH_SQUISH_NUMERATOR = 6,
	VEH_STUCK_REV_MODEL_WIDTH_SQUISH_DENOMINATOR = 10,
	VEH_STUCK_MASK_GRAB_WHISTLE_FX = 0x55,
	VEH_STUCK_MASK_GRAB_WHISTLE_TIME = 960,
	VEH_STUCK_MASK_GRAB_CRASH_TIME = 1296,
	VEH_STUCK_MASK_GRAB_SQUISH_DECREASE_LIMIT = 0x3c1,
	VEH_STUCK_MASK_GRAB_SQUISH_DECREASE_STEP = 800,
	VEH_STUCK_MASK_GRAB_SQUISH_INCREASE_STEP = 0x2d0,
	VEH_STUCK_MASK_GRAB_SQUISH_MAX = 8000,
	VEH_STUCK_MASK_GRAB_LIFT_SHIFT = 7,
	VEH_STUCK_MASK_GRAB_SCALE_FULL_TIME = 721,
	VEH_STUCK_MASK_GRAB_SCALE_START_TIME = VEH_STUCK_MASK_GRAB_WHISTLE_TIME,
	VEH_STUCK_MASK_GRAB_SCALE_RAMP_TIME = 0xf0,
	VEH_STUCK_MASK_GRAB_INITIAL_TIMER = 1440,
	VEH_STUCK_MASK_GRAB_FALL_HEIGHT_THRESHOLD = 0x8000,
	VEH_STUCK_MASK_GRAB_CRASH_ANIM_INDEX = 2,
	VEH_STUCK_MASK_GRAB_CRASH_FRAME_THRESHOLD = 3,
	VEH_STUCK_MASK_GRAB_CRASH_FRAME_OFFSET = 5,
	VEH_STUCK_MASK_GRAB_CRASH_FRAME_CLAMP = 7,
	VEH_STUCK_MASK_GRAB_CRASH_FREEZE_FRAME = 12,
	VEH_STUCK_PLANT_CAMERA_INIT_TIME = 2880,
	VEH_STUCK_PLANT_CAMERA_SIDE_OFFSET = 250,
	VEH_STUCK_PLANT_CAMERA_FORWARD_OFFSET = 750,
	VEH_STUCK_PLANT_CAMERA_Y_OFFSET = 0xc0,
	VEH_STUCK_PLANT_CAMERA_PITCH_BASE = 0x800,
	VEH_TUMBLE_FORCED_JUMP_MS = 0x60,
	VEH_TUMBLE_JUMP_VELOCITY_TIMER_SHIFT = 1,
	VEH_TUMBLE_JUMP_VELOCITY_BASE = 6000,
	VEH_TUMBLE_STEERING_FRAME_SENTINEL = 10000,
	VEH_TUMBLE_ANGULAR_DAMP_SHIFT = 3,
	VEH_TUMBLE_TURN_WRAP_BIAS = 0x800,
	VEH_TUMBLE_TURN_MASK = 0xfff,
	VEH_TUMBLE_SPIN_RATE_SHIFT = 0xd,
	VEH_TUMBLE_ROT_W_INTERP_SHIFT = 5,
	VEH_TUMBLE_ANIM_FRAME_TIME_SHIFT = 5,
	VEH_TUMBLE_ANIM_REVERSE_OFFSET = 1,
	VEH_TUMBLE_WUMPA_PENALTY = 3,
	VEH_TUMBLE_BACKWARDS_RNG_MASK = 4,
	VEH_TUMBLE_RUMBLE_WEAK = 0x19,
	VEH_TUMBLE_RUMBLE_STRONG = 0x29,
	VEH_TUMBLE_RUMBLE_DURATION = 0x60,
};

CTR_STATIC_ASSERT(VEH_STUCK_MASK_BSP_PROBE_HEIGHT == 0x100);
CTR_STATIC_ASSERT(VEH_STUCK_RESPAWN_Y_OFFSET == 0x80);
CTR_STATIC_ASSERT(VEH_STUCK_RESPAWN_PLAYER_CLEARANCE_XZ == 0x2000);
CTR_STATIC_ASSERT(VEH_STUCK_MASK_HEAD_Y_OFFSET == 0x140);
CTR_STATIC_ASSERT(VEH_STUCK_REV_MASK_RELEASE_HEIGHT == 0x4000);
CTR_STATIC_ASSERT(VEH_STUCK_REV_MASK_GRAB_HEIGHT_TRIGGER == 0x1000);
CTR_STATIC_ASSERT(VEH_STUCK_REV_SMALL_BOOST_FIRE_LEVEL == 0x20);
CTR_STATIC_ASSERT(VEH_STUCK_REV_BIG_BOOST_FIRE_LEVEL == 0x80);
CTR_STATIC_ASSERT(VEH_STUCK_REV_START_BOOST_RESERVES == 960);
CTR_STATIC_ASSERT(VEH_STUCK_REV_MASK_DESCENT_STEP == 0x200);
CTR_STATIC_ASSERT(VEH_STUCK_REV_CAMERA_HEIGHT_OFFSET == 0x40);
CTR_STATIC_ASSERT(VEH_STUCK_REV_OVERREV_FX == 0xf);
CTR_STATIC_ASSERT(VEH_STUCK_REV_FILL_STEP_MAX == 5000);
CTR_STATIC_ASSERT(VEH_STUCK_REV_STEP_MIN == 0x100);
CTR_STATIC_ASSERT(VEH_STUCK_REV_OVERREV_TIMEOUT == 192);
CTR_STATIC_ASSERT(VEH_STUCK_REV_RELEASE_COOLDOWN == 0x100);
CTR_STATIC_ASSERT(VEH_STUCK_REV_DECAY_STEP_MAX_NORMAL == 1000);
CTR_STATIC_ASSERT(VEH_STUCK_REV_DECAY_STEP_MAX_LOCKED == 3000);
CTR_STATIC_ASSERT(VEH_STUCK_REV_EMPTY_COOLDOWN == 0xc0);
CTR_STATIC_ASSERT(VEH_STUCK_REV_TURBO_ROOM_SHIFT == 5);
CTR_STATIC_ASSERT(VEH_STUCK_REV_SQUISH_SHIFT == 6);
CTR_STATIC_ASSERT(VEH_STUCK_REV_SQUISH_LIMIT == 0x401);
CTR_STATIC_ASSERT(VEH_STUCK_REV_SQUISH_MAX == 0x400);
CTR_STATIC_ASSERT(VEH_STUCK_REV_MODEL_BASE_SCALE == 3276);
CTR_STATIC_ASSERT(VEH_STUCK_REV_MODEL_WIDTH_SQUISH_NUMERATOR == 6);
CTR_STATIC_ASSERT(VEH_STUCK_REV_MODEL_WIDTH_SQUISH_DENOMINATOR == 10);
CTR_STATIC_ASSERT(VEH_STUCK_MASK_GRAB_WHISTLE_FX == 0x55);
CTR_STATIC_ASSERT(VEH_STUCK_MASK_GRAB_WHISTLE_TIME == 960);
CTR_STATIC_ASSERT(VEH_STUCK_MASK_GRAB_CRASH_TIME == 1296);
CTR_STATIC_ASSERT(VEH_STUCK_MASK_GRAB_SQUISH_DECREASE_LIMIT == 0x3c1);
CTR_STATIC_ASSERT(VEH_STUCK_MASK_GRAB_SQUISH_DECREASE_STEP == 800);
CTR_STATIC_ASSERT(VEH_STUCK_MASK_GRAB_SQUISH_INCREASE_STEP == 0x2d0);
CTR_STATIC_ASSERT(VEH_STUCK_MASK_GRAB_SQUISH_MAX == 8000);
CTR_STATIC_ASSERT(VEH_STUCK_MASK_GRAB_LIFT_SHIFT == 7);
CTR_STATIC_ASSERT(VEH_STUCK_MASK_GRAB_SCALE_FULL_TIME == 721);
CTR_STATIC_ASSERT(VEH_STUCK_MASK_GRAB_SCALE_START_TIME == 960);
CTR_STATIC_ASSERT(VEH_STUCK_MASK_GRAB_SCALE_RAMP_TIME == 0xf0);
CTR_STATIC_ASSERT(VEH_STUCK_MASK_GRAB_INITIAL_TIMER == 1440);
CTR_STATIC_ASSERT(VEH_STUCK_MASK_GRAB_FALL_HEIGHT_THRESHOLD == 0x8000);
CTR_STATIC_ASSERT(VEH_STUCK_MASK_GRAB_CRASH_ANIM_INDEX == 2);
CTR_STATIC_ASSERT(VEH_STUCK_MASK_GRAB_CRASH_FRAME_THRESHOLD == 3);
CTR_STATIC_ASSERT(VEH_STUCK_MASK_GRAB_CRASH_FRAME_OFFSET == 5);
CTR_STATIC_ASSERT(VEH_STUCK_MASK_GRAB_CRASH_FRAME_CLAMP == 7);
CTR_STATIC_ASSERT(VEH_STUCK_MASK_GRAB_CRASH_FREEZE_FRAME == 12);
CTR_STATIC_ASSERT(VEH_STUCK_PLANT_CAMERA_INIT_TIME == 2880);
CTR_STATIC_ASSERT(VEH_STUCK_PLANT_CAMERA_SIDE_OFFSET == 250);
CTR_STATIC_ASSERT(VEH_STUCK_PLANT_CAMERA_FORWARD_OFFSET == 750);
CTR_STATIC_ASSERT(VEH_STUCK_PLANT_CAMERA_Y_OFFSET == 0xc0);
CTR_STATIC_ASSERT(VEH_STUCK_PLANT_CAMERA_PITCH_BASE == 0x800);
CTR_STATIC_ASSERT(VEH_TUMBLE_FORCED_JUMP_MS == 0x60);
CTR_STATIC_ASSERT(VEH_TUMBLE_JUMP_VELOCITY_TIMER_SHIFT == 1);
CTR_STATIC_ASSERT(VEH_TUMBLE_JUMP_VELOCITY_BASE == 6000);
CTR_STATIC_ASSERT(VEH_TUMBLE_STEERING_FRAME_SENTINEL == 10000);
CTR_STATIC_ASSERT(VEH_TUMBLE_ANGULAR_DAMP_SHIFT == 3);
CTR_STATIC_ASSERT(VEH_TUMBLE_TURN_WRAP_BIAS == 0x800);
CTR_STATIC_ASSERT(VEH_TUMBLE_TURN_MASK == 0xfff);
CTR_STATIC_ASSERT(VEH_TUMBLE_SPIN_RATE_SHIFT == 0xd);
CTR_STATIC_ASSERT(VEH_TUMBLE_ROT_W_INTERP_SHIFT == 5);
CTR_STATIC_ASSERT(VEH_TUMBLE_ANIM_FRAME_TIME_SHIFT == 5);
CTR_STATIC_ASSERT(VEH_TUMBLE_ANIM_REVERSE_OFFSET == 1);
CTR_STATIC_ASSERT(VEH_TUMBLE_WUMPA_PENALTY == 3);
CTR_STATIC_ASSERT(VEH_TUMBLE_BACKWARDS_RNG_MASK == 4);
CTR_STATIC_ASSERT(VEH_TUMBLE_RUMBLE_WEAK == 0x19);
CTR_STATIC_ASSERT(VEH_TUMBLE_RUMBLE_STRONG == 0x29);
CTR_STATIC_ASSERT(VEH_TUMBLE_RUMBLE_DURATION == 0x60);

static void VehStuckProc_MaskGrab_SearchBsp(struct Driver *d, struct ScratchpadStruct *sps)
{
	struct GameTracker *gGT = sdata->gGT;
	s16 topX = (s16)CTR_MipsSra(d->posCurr.x, FRACTIONAL_BITS_8);
	s16 topY = (s16)CTR_MipsSra(d->posCurr.y, FRACTIONAL_BITS_8);
	s16 topZ = (s16)CTR_MipsSra(d->posCurr.z, FRACTIONAL_BITS_8);
	s16 bottomY = (s16)CTR_MipsSubLo(topY, VEH_STUCK_MASK_BSP_PROBE_HEIGHT);

	sps->Input1.pos.x = topX;
	sps->Input1.pos.y = bottomY;
	sps->Input1.pos.z = topZ;

	sps->Union.QuadBlockColl.pos.x = topX;
	sps->Union.QuadBlockColl.pos.y = topY;
	sps->Union.QuadBlockColl.pos.z = topZ;

	sps->Union.QuadBlockColl.searchFlags = 0;
	if (gGT->numPlyrCurrGame < 3)
	{
		sps->Union.QuadBlockColl.searchFlags = COLL_SEARCH_HIGH_LOD;
	}

	sps->boolDidTouchQuadblock = 0;
	sps->numTrianglesTested = 0;
	sps->hitFraction = COLL_FRACTION_ONE;
	sps->collision.stepFlags = 0;

	sps->bbox.min.x = topX;
	sps->bbox.max.x = topX;
	sps->bbox.min.y = (bottomY < topY) ? bottomY : topY;
	sps->bbox.max.y = (topY < bottomY) ? bottomY : topY;
	sps->bbox.min.z = topZ;
	sps->bbox.max.z = topZ;

	sps->Union.QuadBlockColl.hitPos = sps->Input1.pos;

	COLL_SearchBSP_CallbackPARAM(sps->ptr_mesh_info->bspRoot, &sps->bbox, COLL_FIXED_BSPLEAF_TestQuadblocks, sps);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8006677c-0x80066cb0.
void VehStuckProc_MaskGrab_FindDestPos(struct Driver *d, struct QuadBlock *quad)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Level *level = gGT->level1;
	struct mesh_info *mesh = level->ptr_mesh_info;

	if ((level->cnt_restart_points < 1) || (level->ptr_restart_points == NULL) || (quad->checkpointIndex == 0xff))
	{
		struct LevVertex *verts = mesh->ptrVertexArray;
		struct LevVertex *v0 = &verts[quad->index[0]];
		struct LevVertex *v3 = &verts[quad->index[3]];

		d->posCurr.x = CTR_MipsSll(CTR_MipsAddLo(v0->pos.x, v3->pos.x), 7);
		d->posCurr.y = CTR_MipsSll(CTR_MipsAddLo(CTR_MipsAddLo(v0->pos.y, v3->pos.y), VEH_STUCK_RESPAWN_Y_OFFSET), 7);
		d->posCurr.z = CTR_MipsSll(CTR_MipsAddLo(v0->pos.z, v3->pos.z), 7);
	}
	else
	{
		struct ScratchpadStruct *sps = CTR_SCRATCHPAD_PTR(struct ScratchpadStruct, 0x108);
		struct Thread *driverThread = d->instSelf->thread;
		struct CheckpointNode *respawn = &level->ptr_restart_points[quad->checkpointIndex];
		struct CheckpointNode *nextRespawn;

		sps->Input1.hitRadius = driverThread->driverHitRadius;
		sps->Input1.hitRadiusSquared = driverThread->driverHitRadiusSquared;
		sps->Union.QuadBlockColl.hitRadius = driverThread->driverHitRadius;
		sps->Union.QuadBlockColl.hitRadiusSquared = driverThread->driverHitRadiusSquared;
		sps->ptr_mesh_info = mesh;
		sps->Union.QuadBlockColl.quadFlagsIgnored = QUADBLOCK_FLAG_NO_CAMERA_RESPAWN_PROBE | QUADBLOCK_FLAG_NO_COLLISION_RESPONSE;
		sps->Union.QuadBlockColl.quadFlagsWanted = QUADBLOCK_FLAG_GROUND;
		d->distanceDrivenBackwards = 0;

		do
		{
			do
			{
				nextRespawn = &level->ptr_restart_points[respawn->nextIndex_forward];

				d->posCurr.x = CTR_MipsSll(respawn->pos.x, FRACTIONAL_BITS_8);
				d->posCurr.y = CTR_MipsSll(CTR_MipsAddLo(respawn->pos.y, VEH_STUCK_RESPAWN_Y_OFFSET), FRACTIONAL_BITS_8);
				d->posCurr.z = CTR_MipsSll(respawn->pos.z, FRACTIONAL_BITS_8);

				d->rotCurr.x = 0;
				d->rotCurr.y = ratan2(CTR_MipsSubLo(nextRespawn->pos.x, respawn->pos.x), CTR_MipsSubLo(nextRespawn->pos.z, respawn->pos.z));
				d->rotCurr.z = 0;

				VehStuckProc_MaskGrab_SearchBsp(d, sps);
				respawn = nextRespawn;
			} while ((sps->boolDidTouchQuadblock == 0) || ((sps->collision.stepFlags & COLL_STEP_FLAG_KILL_PLANE) != 0));

			struct Thread *playerThread = gGT->threadBuckets[PLAYER].thread;
			while (playerThread != NULL)
			{
				struct Driver *other = playerThread->object;

				if (other != d)
				{
					int diffX = CTR_MipsSubLo(d->posCurr.x, other->posCurr.x);
					int diffZ = CTR_MipsSubLo(d->posCurr.z, other->posCurr.z);

					if (diffX < 0)
					{
						diffX = CTR_MipsNegLo(diffX);
					}

					if (diffX < VEH_STUCK_RESPAWN_PLAYER_CLEARANCE_XZ)
					{
						break;
					}

					if (diffZ < 0)
					{
						diffZ = CTR_MipsNegLo(diffZ);
					}

					if (diffZ < VEH_STUCK_RESPAWN_PLAYER_CLEARANCE_XZ)
					{
						break;
					}
				}

				playerThread = playerThread->siblingThread;
			}

			if (playerThread == NULL)
			{
				break;
			}
		} while (1);
	}

	gGT->cameraDC[d->driverID].flags |= 1;
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80066cb0-0x80066d4c.
void VehStuckProc_MaskGrab_Particles(struct Driver *d)
{
	struct Particle *p;

	for (int i = 10; i > 0; i--)
	{
		// Create instance in particle pool
		p = Particle_Init(0, sdata->gGT->iconGroup[0], &data.emSet_Maskgrab[0]);

		if (p == NULL)
		{
			return;
		}

		// position variables
		p->axis[0].startVal = CTR_MipsAddLo(p->axis[0].startVal, d->posCurr.x);
		p->axis[1].startVal = CTR_MipsAddLo(p->axis[1].startVal, d->posCurr.y);
		p->axis[2].startVal = CTR_MipsAddLo(p->axis[2].startVal, d->posCurr.z);
	}
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80066d4c-0x80066e3c.
void VehStuckProc_MaskGrab_Update(struct Thread *t, struct Driver *d)
{
	struct GameTracker *gGT = sdata->gGT;

	d->NoInputTimer = (s16)CTR_MipsSubLo((u16)d->NoInputTimer, (u16)gGT->elapsedTimeMS);

	if (d->NoInputTimer < 0)
	{
		d->NoInputTimer = 0;
	}

	if (d->NoInputTimer != 0)
	{
		return;
	}

	// when input is allowed,
	// which is when driver is spawned back over track

	struct MaskHeadWeapon *mask = d->KartStates.MaskGrab.maskObj;

	if (mask != NULL)
	{
		// mask rotZ
		mask->rot.z &= ~MASK_HEAD_ROT_WORLD_SPACE;

		// scale = 100%
		mask->scale = MASK_HEAD_SCALE_NORMAL;
	}


	// CameraDC flag
	gGT->cameraDC[d->driverID].flags |= CAMERA_FLAG_DIRECTION_CHANGED;


	VehStuckProc_MaskGrab_FindDestPos(d, d->lastValid);

	VehBirth_TeleportSelf(d, 0, VEH_STUCK_RESPAWN_Y_OFFSET);

	VehStuckProc_RevEngine_Init(t, d);
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80066e3c-0x80066e8c.
void VehStuckProc_MaskGrab_PhysLinear(struct Thread *t, struct Driver *d)
{
	VehPhysProc_Driving_PhysLinear(t, d);

	d->baseSpeed = 0;
	d->fireSpeed = 0;
	d->jump_TenBuffer = 0;

	// reset turning state
	d->simpTurnState = 0;

	d->actionsFlagSet &= ~(ACTION_REVERSING_ENGINE | ACTION_BRAKE_WITH_ACCEL | ACTION_JUMP_BUTTON_HELD);
	d->actionsFlagSet |= ACTION_ACCEL_PREVENTION;
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80066e8c-0x800671b0.
void VehStuckProc_MaskGrab_Animate(struct Thread *t, struct Driver *d)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Instance *inst = t->inst;

	// if driver touched ground before mask grab
	if (d->KartStates.MaskGrab.boolStillFalling == false)
	{
		d->matrixArray = BAKED_GTE_MATRIX_NONE;
		d->matrixIndex = 0;
		inst->animIndex = 0;

		int numFrames = VehFrameInst_GetNumAnimFrames(inst, 0);

		inst->animFrame = VehFrameInst_GetStartFrame(0, numFrames);

		d->AxisAngle2_normalVec = d->KartStates.MaskGrab.AngleAxis_NormalVec;
	}

	// if driver did not touch ground (and is falling)
	else
	{
		if (
		    // if whistle sound has not played
		    (d->KartStates.MaskGrab.boolWhistle == false) &&

		    // no input less than 1 sec
		    (d->NoInputTimer < VEH_STUCK_MASK_GRAB_WHISTLE_TIME))

		{
			// whistle sound has played
			d->KartStates.MaskGrab.boolWhistle = true;

			// "falling" sound, like a whistle
			OtherFX_Play(VEH_STUCK_MASK_GRAB_WHISTLE_FX, 1);
		}


		// Crashing animation at a frozen frame
		// makes it look like the driver is falling
		d->matrixArray = BAKED_GTE_MATRIX_CRASH_FALL;
		inst->animIndex = VEH_STUCK_MASK_GRAB_CRASH_ANIM_INDEX;


		int maskGrabAnimFrame = d->KartStates.MaskGrab.animFrame;


		// logic specific to matrix set
		if (maskGrabAnimFrame < VEH_STUCK_MASK_GRAB_CRASH_FRAME_THRESHOLD)
		{
			d->matrixIndex = VEH_STUCK_MASK_GRAB_CRASH_FRAME_CLAMP;
		}
		else
		{
			d->matrixIndex = maskGrabAnimFrame + VEH_STUCK_MASK_GRAB_CRASH_FRAME_OFFSET;
		}


		// logic specific to instance
		int frame = VEH_STUCK_MASK_GRAB_CRASH_FRAME_CLAMP;
		if (VEH_STUCK_MASK_GRAB_CRASH_FRAME_THRESHOLD <= maskGrabAnimFrame)
		{
			frame = maskGrabAnimFrame + VEH_STUCK_MASK_GRAB_CRASH_FRAME_OFFSET;
		}
		inst->animFrame = frame;


		// logic specific to maskgrab
		frame = maskGrabAnimFrame + 1;

		if (frame > VEH_STUCK_MASK_GRAB_CRASH_FRAME_CLAMP)
		{
			frame = VEH_STUCK_MASK_GRAB_CRASH_FRAME_CLAMP;
		}
		d->KartStates.MaskGrab.animFrame = frame;

		// no input is less than 1.35 s
		if (d->NoInputTimer < VEH_STUCK_MASK_GRAB_CRASH_TIME)
		{
			// Crashing
			d->matrixArray = BAKED_GTE_MATRIX_CRASH_FALL;

			d->matrixIndex = VEH_STUCK_MASK_GRAB_CRASH_FREEZE_FRAME;

			// set animation
			inst->animIndex = VEH_STUCK_MASK_GRAB_CRASH_ANIM_INDEX;

			// set animation frame
			inst->animFrame = VEH_STUCK_MASK_GRAB_CRASH_FREEZE_FRAME;

			if (d->NoInputTimer < VEH_STUCK_MASK_GRAB_SQUISH_DECREASE_LIMIT)
			{
				d->jumpSquishStretch = (s16)CTR_MipsSubLo((u16)d->jumpSquishStretch, VEH_STUCK_MASK_GRAB_SQUISH_DECREASE_STEP);
				if (d->jumpSquishStretch < 0)
				{
					d->jumpSquishStretch = 0;
				}
			}
			else
			{
				// if particles are not spawned
				if (d->KartStates.MaskGrab.boolParticlesSpawned == false)
				{
					VehStuckProc_MaskGrab_Particles(d);

					// now they are spawned
					d->KartStates.MaskGrab.boolParticlesSpawned = true;
				}

				d->jumpSquishStretch = (s16)CTR_MipsAddLo((u16)d->jumpSquishStretch, VEH_STUCK_MASK_GRAB_SQUISH_INCREASE_STEP);
				if (d->jumpSquishStretch > VEH_STUCK_MASK_GRAB_SQUISH_MAX)
				{
					d->jumpSquishStretch = VEH_STUCK_MASK_GRAB_SQUISH_MAX;
				}
			}
		}
		else
		{
			// reset Speed and Speed Approximate
			d->speed = 0;
			d->speedApprox = 0;

			// position backups
			d->posCurr = d->posPrev;
		}
	}

	struct MaskHeadWeapon *mask = d->KartStates.MaskGrab.maskObj;

	// if maskObj
	if (mask == 0)
	{
		return;
	}

	// set mask duration
	mask->duration = MASK_HEAD_DURATION_NORMAL;

	// less than 0.5s after player fell
	if (d->NoInputTimer > VEH_STUCK_MASK_GRAB_SCALE_START_TIME)
	{
		// scale = 0%
		mask->scale = 0;
		return;
	}

	// if more than 0.5s after player fell

	// if not lifting player
	if (d->KartStates.MaskGrab.boolLiftingPlayer == false)
	{
		// decrease mask posY by elapsed time
		mask->pos.y = (s16)CTR_MipsSubLo((u16)mask->pos.y, (u16)gGT->elapsedTimeMS);
	}

	// if lifting player (if driver isn't falling infinitely)
	else
	{
		d->speed = 0;

		// increase driver height, both posCurr and posPrev
		d->posCurr.y = CTR_MipsAddLo(d->posCurr.y, CTR_MipsSll(gGT->elapsedTimeMS, VEH_STUCK_MASK_GRAB_LIFT_SHIFT));
		d->posPrev.y = d->posCurr.y;
	}

	// maskPosX = driverPosX
	mask->pos.x = (s16)CTR_MipsSra(d->posCurr.x, FRACTIONAL_BITS_8);

	// set mask posZ
	mask->pos.z = (s16)CTR_MipsSra(d->posCurr.z, FRACTIONAL_BITS_8);

	// if mask posY < driver posY
	if (mask->pos.y < (s16)CTR_MipsSra(d->posCurr.y, FRACTIONAL_BITS_8))
	{
		// mask posY = driver posY
		mask->pos.y = (s16)CTR_MipsSra(d->posCurr.y, FRACTIONAL_BITS_8);

		d->KartStates.MaskGrab.boolLiftingPlayer = true;
	}

	// if more than halfway through mask pickup
	if (d->NoInputTimer < VEH_STUCK_MASK_GRAB_SCALE_FULL_TIME)
	{
		// scale = 100%
		mask->scale = MASK_HEAD_SCALE_NORMAL;
	}

	// if less than half
	else
	{
		// interpolate scale
		mask->scale =
		    (s16)(CTR_MipsSll(CTR_MipsSubLo(VEH_STUCK_MASK_GRAB_SCALE_START_TIME, d->NoInputTimer), FRACTIONAL_BITS) / VEH_STUCK_MASK_GRAB_SCALE_RAMP_TIME);
	}
}


extern DriverFunc PlayerMaskGrabFuncTable[DRIVER_FUNC_COUNT];

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800671b0-0x8006749c.
void VehStuckProc_MaskGrab_Init(struct Thread *t, struct Driver *d)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Instance *inst = t->inst;

	d->kartState = KS_MASK_GRABBED;

	d->KartStates.MaskGrab.animFrame = 0;

	d->KartStates.MaskGrab.boolParticlesSpawned = false;
	d->KartStates.MaskGrab.boolStillFalling = false;
	d->KartStates.MaskGrab.boolLiftingPlayer = false;
	d->KartStates.MaskGrab.boolWhistle = false;

	d->KartStates.MaskGrab.maskObj = VehPickupItem_MaskUseWeapon(d, true);

	d->matrixArray = BAKED_GTE_MATRIX_NONE;
	d->matrixIndex = 0;

	d->turbo_MeterRoomLeft = 0;
	d->turbo_outsideTimer = 0;
	d->reserves = 0;

	d->NoInputTimer = VEH_STUCK_MASK_GRAB_INITIAL_TIMER;

	d->actionsFlagSet &= ~(ACTION_AIRBORNE | ACTION_HIGH_JUMP);

	if (LOAD_IsOpen_RacingOrBattle() && ((gGT->gameMode1 & ADVENTURE_ARENA) == 0))
	{
		RB_Player_ModifyWumpa(d, -2);
	}

	if (CTR_MipsAddLo(d->quadBlockHeight, VEH_STUCK_MASK_GRAB_FALL_HEIGHT_THRESHOLD) < d->posCurr.y)
	{
		d->numTimesMaskGrab++;

		if ((d->posCurr.y < -VEH_STUCK_MASK_GRAB_FALL_HEIGHT_THRESHOLD) && ((gGT->level1->configFlags & 2) != 0))
		{
			d->KartStates.MaskGrab.AngleAxis_NormalVec = d->AxisAngle2_normalVec;

			for (int i = 10; i > 0; i--)
			{
				struct Particle *p = Particle_Init(0, gGT->iconGroup[9], &data.emSet_Falling[0]);
				if (p == NULL)
				{
					break;
				}

				p->otIndexOffset = d->instSelf->depthBiasNormal;
				p->driverInst = d->instSelf;
				p->driverID = d->driverID;
			}
		}
		else
		{
			d->KartStates.MaskGrab.boolStillFalling = true;
		}
	}
	else
	{
		d->KartStates.MaskGrab.AngleAxis_NormalVec = d->AxisAngle2_normalVec;
	}

	d->posCurr.x = CTR_MipsSll(inst->matrix.t[0], FRACTIONAL_BITS_8);
	d->posCurr.y = CTR_MipsSll(inst->matrix.t[1], FRACTIONAL_BITS_8);
	d->posCurr.z = CTR_MipsSll(inst->matrix.t[2], FRACTIONAL_BITS_8);

	d->posPrev = d->posCurr;

	for (int i = 0; i < DRIVER_FUNC_COUNT; i++)
	{
		d->funcPtrs[i] = PlayerMaskGrabFuncTable[i];
	}

	struct MaskHeadWeapon *mask = d->KartStates.MaskGrab.maskObj;
	if (mask == NULL)
	{
		return;
	}

	mask->rot.z |= MASK_HEAD_ROT_WORLD_SPACE;

	mask->pos.x = (s16)CTR_MipsSra(d->posCurr.x, FRACTIONAL_BITS_8);
	mask->pos.y = (s16)CTR_MipsAddLo(CTR_MipsSra(d->posCurr.y, FRACTIONAL_BITS_8), VEH_STUCK_MASK_HEAD_Y_OFFSET);
	mask->pos.z = (s16)CTR_MipsSra(d->posCurr.z, FRACTIONAL_BITS_8);
}


DriverFunc PlayerMaskGrabFuncTable[DRIVER_FUNC_COUNT] = {NULL,
                                                         VehStuckProc_MaskGrab_Update,
                                                         VehStuckProc_MaskGrab_PhysLinear,
                                                         VehPhysProc_Driving_Audio,
                                                         VehPhysGeneral_PhysAngular,
                                                         VehPhysForce_OnApplyForces,
                                                         COLL_MOVED_PlayerSearch,
                                                         VehPhysForce_CollideDrivers,
                                                         COLL_FIXED_PlayerSearch,
                                                         VehPhysGeneral_JumpAndFriction,
                                                         VehPhysForce_TranslateMatrix,
                                                         VehStuckProc_MaskGrab_Animate,
                                                         VehEmitter_DriverMain};


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8006749c-0x80067554.
void VehStuckProc_PlantEaten_Update(struct Thread *t, struct Driver *d)
{
	d->NoInputTimer = (s16)CTR_MipsSubLo((u16)d->NoInputTimer, (u16)sdata->gGT->elapsedTimeMS);

	if (d->NoInputTimer <= 0)
	{
		d->NoInputTimer = 0;

		// respawn driver at last valid quadblock
		VehStuckProc_MaskGrab_FindDestPos(d, d->lastValid);
		VehBirth_TeleportSelf(d, 0, VEH_STUCK_RESPAWN_Y_OFFSET);

		// enable collision, make visible
		t->flags &= ~THREAD_FLAG_DISABLE_COLLISION;
		t->inst->flags &= ~(HIDE_MODEL);

		// this lets you rev engine while falling
		VehStuckProc_RevEngine_Init(t, d);
	}
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80067554-0x800675c0.
void VehStuckProc_PlantEaten_PhysLinear(struct Thread *t, struct Driver *d)
{
	VehPhysProc_Driving_PhysLinear(t, d);

	d->simpTurnState = 0;

	// reset two speed variables
	d->fireSpeed = 0;
	d->baseSpeed = 0;

	// reset jump variable
	d->jump_TenBuffer = 0;

	// acceleration prevention,
	// drop jump-button, gas+brake, and reversing engine bits.
	d->actionsFlagSet &= ~(ACTION_REVERSING_ENGINE | ACTION_BRAKE_WITH_ACCEL | ACTION_JUMP_BUTTON_HELD);
	d->actionsFlagSet |= ACTION_ACCEL_PREVENTION;

	d->timeSpentEaten = CTR_MipsAddLo(d->timeSpentEaten, sdata->gGT->elapsedTimeMS);
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800675c0-0x800677d0.
void VehStuckProc_PlantEaten_Animate(struct Thread *t, struct Driver *d)
{
	(void)t;
	s32 dist;
	struct Instance *inst;
	SVECTOR plantVector;
	VECTOR camVec;
	s32 gteFlags[2];

	struct GameTracker *gGT = sdata->gGT;

	struct Thread *plant = d->plantEatingMe;

	// if any plant is eating me
	if (((plant != NULL) &&

	     // if not initialized
	     (d->KartStates.EatenByPlant.boolInited == false)) &&

	    // if more than 0.5s since player death
	    (d->NoInputTimer < VEH_STUCK_PLANT_CAMERA_INIT_TIME))
	{
		// get instance from thread
		inst = plant->inst;

		// initialized, player eaten
		d->KartStates.EatenByPlant.boolInited = true;

		plantVector.vx = (((struct Plant *)plant->object)->side == 0) ? VEH_STUCK_PLANT_CAMERA_SIDE_OFFSET : -VEH_STUCK_PLANT_CAMERA_SIDE_OFFSET;
		plantVector.vy = 0;
		plantVector.vz = VEH_STUCK_PLANT_CAMERA_FORWARD_OFFSET;

		SetRotMatrix(&inst->matrix);

		SetTransMatrix(&inst->matrix);

		camVec.vx = 0;
		camVec.vy = 0;
		camVec.vz = 0;
		gteFlags[0] = 0;
		gteFlags[1] = 0;

		RotTrans(&plantVector, &camVec, gteFlags);

		struct PushBuffer *pb = &gGT->pushBuffer[d->driverID];

		pb->pos.x = camVec.vx;
		pb->pos.y = CTR_MipsAddLo(inst->matrix.t[1], VEH_STUCK_PLANT_CAMERA_Y_OFFSET);
		pb->pos.z = camVec.vz;

		int camX = CTR_MipsSubLo(camVec.vx, inst->matrix.t[0]);
		int camZ = CTR_MipsSubLo(camVec.vz, inst->matrix.t[2]);

		pb->rot.y = (s16)ratan2(camX, camZ);

		// get distance between car and camera
		dist = SquareRoot0_stub(CTR_MipsAddLo(CTR_MipsMulLo(camX, camX), CTR_MipsMulLo(camZ, camZ)));

		pb->rot.x = CTR_MipsSubLo(VEH_STUCK_PLANT_CAMERA_PITCH_BASE, ratan2(CTR_MipsSubLo(pb->pos.y, inst->matrix.t[1]), dist));

		pb->rot.z = 0;
	}
}


extern DriverFunc PlayerEatenFuncTable[DRIVER_FUNC_COUNT];

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800677d0-0x80067930.
// when eaten by plant on papu pyramid
void VehStuckProc_PlantEaten_Init(struct Thread *t, struct Driver *d)
{
	// when this function executes, you are lifted
	// above the track by the mask, where you respawn

	struct Instance *inst = t->inst;

	// set state to mask grab, so nobody hits you with weapon
	d->kartState = KS_MASK_GRABBED;

	d->KartStates.EatenByPlant.boolInited = false;

	d->turbo_MeterRoomLeft = 0;
	d->turbo_outsideTimer = 0;
	d->reserves = 0;

	// drop bits for airborne and high-jump state
	d->actionsFlagSet &= ~(ACTION_AIRBORNE | ACTION_HIGH_JUMP);

	// "cloud" is the raincloud after hitting red potion

	// if thread of "cloud" exists
	if (d->thCloud != NULL)
	{
		((struct RainCloud *)d->thCloud->object)->timeMS = 0;

		d->thCloud->funcThTick = RB_RainCloud_FadeAway;
		d->thCloud = NULL;
	}

	if (LOAD_IsOpen_RacingOrBattle() && ((sdata->gGT->gameMode1 & ADVENTURE_ARENA) == 0))
	{
		RB_Player_ModifyWumpa(d, -2);
	}

	// allow this thread to ignore all collisions
	t->flags |= THREAD_FLAG_DISABLE_COLLISION;

	// make invisible
	inst->flags |= HIDE_MODEL;

	OtherFX_Stop1((int)d->driverAudioPtrs[1]);
	d->driverAudioPtrs[1] = 0;
	OtherFX_Stop1((int)d->driverAudioPtrs[2]);
	d->driverAudioPtrs[2] = 0;
	OtherFX_Stop1((int)d->driverAudioPtrs[0]);
	d->driverAudioPtrs[0] = 0;

	for (int i = 0; i < DRIVER_FUNC_COUNT; i++)
	{
		d->funcPtrs[i] = PlayerEatenFuncTable[i];
	}
}

DriverFunc PlayerEatenFuncTable[DRIVER_FUNC_COUNT] = {
    NULL,
    VehStuckProc_PlantEaten_Update,
    VehStuckProc_PlantEaten_PhysLinear,
    VehPhysProc_Driving_Audio,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    VehStuckProc_PlantEaten_Animate,
    NULL,
};


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80067930-0x80067960.
void VehStuckProc_RIP_Init(struct Thread *t, struct Driver *d)
{
	VehStuckProc_PlantEaten_Init(t, d);
	d->invisibleTimer = 0;
	d->funcPtrs[DRIVER_FUNC_UPDATE] = NULL;
	d->funcPtrs[DRIVER_FUNC_ANIMATE] = NULL;
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80067960-0x80067a74.
void VehStuckProc_RevEngine_Update(struct Thread *t, struct Driver *d)
{
	int revFireLevel;

	// If race has not started
	if (d->KartStates.RevEngine.boolMaskGrab == false)
	{
		// If Traffic Lights are not done counting down
		if (0 < sdata->gGT->trafficLightsTimer)
		{
			// Dont continue with the function,
			// let your kart stay in a revving state
			return;
		}
	}

	// If race has started
	else
	{
		// If mask grab has not lowered you close
		// enough to the track to let you go
		if (CTR_MipsAddLo(d->quadBlockHeight, VEH_STUCK_REV_MASK_RELEASE_HEIGHT) <= d->posCurr.y)
		{
			// Dont continue with the function,
			// let your kart stay in a revving state
			return;
		}
	}

	// Assume it's time to transition out of being
	// frozen, and into driving, last iteration of
	// this function

	if ((d->KartStates.RevEngine.boolMaskGrab == true) && (d->KartStates.RevEngine.maskObj != NULL))
	{
		d->KartStates.RevEngine.maskObj->duration = 0;
	}

	if ((d->const_AccelSpeed_ClassStat < d->KartStates.RevEngine.fireLevel) && (d->KartStates.RevEngine.lockoutFlags & REV_ENGINE_LOCKOUT_ALL) == 0)
	{
		// While not moving, if you rev'd your engine less than...
		if (d->KartStates.RevEngine.boostMeter < CTR_MipsAddLo(d->const_AccelSpeed_ClassStat, d->const_SacredFireSpeed))
		{
			// You get a small boost
			revFireLevel = VEH_STUCK_REV_SMALL_BOOST_FIRE_LEVEL;
		}

		// if you rev'd your engine high
		else
		{
			// you get a big boost
			revFireLevel = VEH_STUCK_REV_BIG_BOOST_FIRE_LEVEL;
		}

		// one full second of reserves
		VehFire_Increment(d, VEH_STUCK_REV_START_BOOST_RESERVES, 0, revFireLevel);
	}

	// full meter
	d->turbo_MeterRoomLeft = 0;
	d->revEngineState = 0;

	VehPhysProc_Driving_Init(t, d);
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80067a74-0x80067b7c.
void VehStuckProc_RevEngine_PhysLinear(struct Thread *t, struct Driver *d)
{
	u32 cooldownTimer;

	struct GameTracker *gGT = sdata->gGT;

	cooldownTimer = (u16)d->KartStates.RevEngine.releaseCooldownTimerMS;
	cooldownTimer = CTR_MipsSubLo(cooldownTimer, (u16)gGT->elapsedTimeMS);
	if ((cooldownTimer & 0x8000) != 0)
	{
		cooldownTimer = 0;
	}
	d->KartStates.RevEngine.releaseCooldownTimerMS = (s16)cooldownTimer;

	cooldownTimer = (u16)d->KartStates.RevEngine.emptyCooldownTimerMS;
	cooldownTimer = CTR_MipsSubLo(cooldownTimer, (u16)gGT->elapsedTimeMS);
	if ((cooldownTimer & 0x8000) != 0)
	{
		cooldownTimer = 0;
	}
	d->KartStates.RevEngine.emptyCooldownTimerMS = (s16)cooldownTimer;

	VehPhysProc_Driving_PhysLinear(t, d);

	if (d->KartStates.RevEngine.boolMaskGrab == 0)
	{
		return;
	}

	d->posCurr.y = CTR_MipsSubLo(d->posCurr.y, VEH_STUCK_REV_MASK_DESCENT_STEP);

	// if maskObj exists
	if (d->KartStates.RevEngine.maskObj != 0)
	{
		d->KartStates.RevEngine.maskObj->duration = MASK_HEAD_DURATION_NORMAL;
	}

	struct CameraDC *cDC = &gGT->cameraDC[d->driverID];
	cDC->flags |= CAMERA_FLAG_MASK_GRAB;
	cDC->maskGrabHeightOffset = VEH_STUCK_REV_CAMERA_HEIGHT_OFFSET;
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80067b7c-0x80067f4c.
void VehStuckProc_RevEngine_Animate(struct Thread *t, struct Driver *d)
{
	struct Instance *inst = t->inst;

	if ((d->fireSpeed > 0) && (d->KartStates.RevEngine.releaseCooldownTimerMS == 0) && ((d->KartStates.RevEngine.lockoutFlags & REV_ENGINE_LOCKOUT_ALL) == 0))
	{
		int revDelta = CTR_MipsSubLo(d->KartStates.RevEngine.fireLevel, d->KartStates.RevEngine.boostMeter);
		if (revDelta < 0)
		{
			revDelta = CTR_MipsNegLo(revDelta);
		}

		revDelta = CTR_MipsSra(revDelta, 1);

		int fillStep = revDelta;

		// Speed of filling the meter changes
		// depending on how full the meter is,
		// there are two speeds
		if (VEH_STUCK_REV_FILL_STEP_MAX < revDelta)
		{
			fillStep = VEH_STUCK_REV_FILL_STEP_MAX;
		}

		if (revDelta < VEH_STUCK_REV_STEP_MIN)
		{
			fillStep = VEH_STUCK_REV_STEP_MIN;
		}

		int revLevel = VehCalc_InterpBySpeed(d->KartStates.RevEngine.fireLevel, fillStep, d->KartStates.RevEngine.boostMeter);

		d->KartStates.RevEngine.fireLevel = revLevel;
		d->KartStates.RevEngine.chargeState = REV_ENGINE_CHARGE_ACTIVE;

		if (revLevel < d->KartStates.RevEngine.boostMeter)
		{
			d->KartStates.RevEngine.overRevTimerMS = 0;
		}
		else
		{
			s16 overRevTimerMS = (s16)CTR_MipsAddLo((u16)d->KartStates.RevEngine.overRevTimerMS, (u16)sdata->gGT->elapsedTimeMS);
			d->KartStates.RevEngine.overRevTimerMS = overRevTimerMS;

			if (VEH_STUCK_REV_OVERREV_TIMEOUT < overRevTimerMS)
			{
				d->KartStates.RevEngine.chargeState = REV_ENGINE_CHARGE_IDLE;
				d->KartStates.RevEngine.lockoutFlags |= REV_ENGINE_LOCKOUT_ALL;

				OtherFX_Play_Echo(VEH_STUCK_REV_OVERREV_FX, 1, d->actionsFlagSet & ACTION_ENGINE_ECHO);
			}
		}
		goto LAB_80067dec;
	}
	d->KartStates.RevEngine.overRevTimerMS = 0;

	if (d->KartStates.RevEngine.chargeState == REV_ENGINE_CHARGE_ACTIVE)
	{
		d->KartStates.RevEngine.releaseCooldownTimerMS = VEH_STUCK_REV_RELEASE_COOLDOWN;
		d->KartStates.RevEngine.chargeState = REV_ENGINE_CHARGE_IDLE;

		if (d->const_AccelSpeed_ClassStat < d->KartStates.RevEngine.fireLevel)
		{
			d->KartStates.RevEngine.chargeState = REV_ENGINE_CHARGE_RELEASED_ABOVE_ACCEL;
		}
	}

	if ((d->KartStates.RevEngine.chargeState != REV_ENGINE_CHARGE_IDLE) && (d->KartStates.RevEngine.fireLevel < d->const_AccelSpeed_ClassStat))
	{
		d->KartStates.RevEngine.chargeState = REV_ENGINE_CHARGE_IDLE;

		int boostMeter = VehCalc_InterpBySpeed(d->KartStates.RevEngine.boostMeter, CTR_MipsAddLo(d->const_SacredFireSpeed / 3, 3),
		                                       CTR_MipsAddLo(d->const_SacredFireSpeed, d->const_AccelSpeed_ClassStat));
		d->KartStates.RevEngine.boostMeter = boostMeter;
	}

	if (d->KartStates.RevEngine.fireLevel < 1)
	{
		d->KartStates.RevEngine.lockoutFlags &= ~REV_ENGINE_LOCKOUT_REV_DECAY;
		d->KartStates.RevEngine.boostMeter = CTR_MipsAddLo(d->const_AccelSpeed_ClassStat, d->const_SacredFireSpeed / 3);
	}
	else
	{
		u32 decayStep = CTR_MipsSra(d->KartStates.RevEngine.fireLevel, 1);
		u8 decayBelowMinimum;

		if ((d->KartStates.RevEngine.lockoutFlags & REV_ENGINE_LOCKOUT_REV_DECAY) == 0)
		{
			decayBelowMinimum = (int)decayStep < VEH_STUCK_REV_STEP_MIN;

			if (VEH_STUCK_REV_DECAY_STEP_MAX_NORMAL < (int)decayStep)
			{
				decayStep = VEH_STUCK_REV_DECAY_STEP_MAX_NORMAL;
				decayBelowMinimum = decayStep < VEH_STUCK_REV_STEP_MIN;
			}
		}
		else
		{
			decayBelowMinimum = (int)decayStep < VEH_STUCK_REV_STEP_MIN;

			if (VEH_STUCK_REV_DECAY_STEP_MAX_LOCKED < (int)decayStep)
			{
				decayStep = VEH_STUCK_REV_DECAY_STEP_MAX_LOCKED;
				decayBelowMinimum = decayStep < VEH_STUCK_REV_STEP_MIN;
			}
		}

		if (decayBelowMinimum)
		{
			decayStep = VEH_STUCK_REV_STEP_MIN;
		}

		int revLevel = CTR_MipsSubLo(d->KartStates.RevEngine.fireLevel, decayStep);
		d->KartStates.RevEngine.fireLevel = revLevel;

		if (revLevel < 1)
		{
			d->KartStates.RevEngine.emptyCooldownTimerMS = VEH_STUCK_REV_EMPTY_COOLDOWN;
			d->KartStates.RevEngine.fireLevel = 0;
		}
	}

	if (d->fireSpeed < 1)
	{
		d->KartStates.RevEngine.lockoutFlags &= ~REV_ENGINE_LOCKOUT_PEDAL_HELD;
	}

LAB_80067dec:;

	u32 packedStatus = ((u32)(u16)d->KartStates.RevEngine.emptyCooldownTimerMS) | ((u32)d->KartStates.RevEngine.chargeState << 16) |
	                   ((u32)d->KartStates.RevEngine.lockoutFlags << 24);
	if ((packedStatus & REV_ENGINE_PACKED_BUSY_MASK) == 0)
	{
		if (d->KartStates.RevEngine.fireLevel < d->const_AccelSpeed_ClassStat)
		{
			d->revEngineState = 0;
		}
		else
		{
			d->revEngineState = 1;
		}
	}
	else
	{
		d->revEngineState = 2;
	}

	int accelClassStat = d->const_AccelSpeed_ClassStat;

	d->speedometerNeedleValue = d->KartStates.RevEngine.fireLevel;

	u8 meterRoomStart;
	int meterRoomEnd;
	int meterMin;
	int meterMax;

	if (d->KartStates.RevEngine.fireLevel < accelClassStat)
	{
		// 476 and 447 can be absolutely any value,
		// by default they are 15 and 30, but as long as
		// they are proportional (1 and 2, 4 and 8), they
		// behave the same as 15 and 30

		meterRoomStart = d->const_turboMaxRoom;

		// 477 changes when meter turns red
		meterRoomEnd = CTR_MipsAddLo(CTR_MipsSll((u8)d->const_turboLowRoomWarning, VEH_STUCK_REV_TURBO_ROOM_SHIFT), 1);

		meterMin = 0;
		meterMax = accelClassStat;
	}
	else
	{
		// 477 changes when meter turns red
		meterRoomStart = d->const_turboLowRoomWarning;

		meterRoomEnd = 1;

		meterMin = accelClassStat;
		meterMax = CTR_MipsAddLo(accelClassStat, d->const_SacredFireSpeed);
	}

	s16 meterRoomLeft =
	    VehCalc_MapToRange(d->KartStates.RevEngine.fireLevel, meterMin, meterMax, CTR_MipsSll(meterRoomStart, VEH_STUCK_REV_TURBO_ROOM_SHIFT), meterRoomEnd);
	d->turbo_MeterRoomLeft = meterRoomLeft;

	d->distanceDrivenBackwards = 0;
	int squishScale = CTR_MipsSra((s16)d->speedometerNeedleValue, VEH_STUCK_REV_SQUISH_SHIFT);

	if (squishScale < VEH_STUCK_REV_SQUISH_LIMIT)
	{
		if (squishScale < 0)
		{
			squishScale = 0;
		}
	}
	else
	{
		squishScale = VEH_STUCK_REV_SQUISH_MAX;
	}

	// Set the scale of the car while revving the engine,
	// this is a basic "squash and stretch" concept of animation, before motion

	// Reduce height a little
	inst->scale.y = (s16)CTR_MipsSubLo(VEH_STUCK_REV_MODEL_BASE_SCALE, squishScale);
	inst->scale.x = (s16)CTR_MipsAddLo(CTR_MipsMulLo(squishScale, VEH_STUCK_REV_MODEL_WIDTH_SQUISH_NUMERATOR) / VEH_STUCK_REV_MODEL_WIDTH_SQUISH_DENOMINATOR,
	                                   VEH_STUCK_REV_MODEL_BASE_SCALE);
	inst->scale.z = (s16)CTR_MipsAddLo(CTR_MipsMulLo(squishScale, VEH_STUCK_REV_MODEL_WIDTH_SQUISH_NUMERATOR) / VEH_STUCK_REV_MODEL_WIDTH_SQUISH_DENOMINATOR,
	                                   VEH_STUCK_REV_MODEL_BASE_SCALE);

	d->jumpSquishStretch = squishScale;
}


extern DriverFunc PlayerRevEngineFuncTable[DRIVER_FUNC_COUNT];

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80067f4c-0x8006809c.
void VehStuckProc_RevEngine_Init(struct Thread *t, struct Driver *d)
{
	(void)t;
	// spawn function that waits for traffic lights

	// kart state to rev
	d->kartState = KS_ENGINE_REVVING;
	d->revEngineState = 0;

	// assume reason for revving is: start of race
	d->KartStates.RevEngine.boolMaskGrab = false;
	d->KartStates.RevEngine.maskObj = NULL;
	d->KartStates.RevEngine.fireLevel = 0;

	// if this is a mask grab
	if (CTR_MipsAddLo(d->quadBlockHeight, VEH_STUCK_REV_MASK_GRAB_HEIGHT_TRIGGER) < d->posCurr.y)
	{
		// assume reason for revving is: mask grab
		d->KartStates.RevEngine.boolMaskGrab = true;
		d->KartStates.RevEngine.maskObj = VehPickupItem_MaskUseWeapon(d, false);

		d->actionsFlagSet &= ~ACTION_TOUCH_GROUND;

		// CameraDC flag
		sdata->gGT->cameraDC[d->driverID].flags |= CAMERA_FLAG_DIRECTION_CHANGED;
	}

	for (s32 i = 0; i < DRIVER_FUNC_COUNT; i++)
	{
		d->funcPtrs[i] = PlayerRevEngineFuncTable[i];
	}

	d->boolFirstFrameSinceRevEngine = true;

	d->KartStates.RevEngine.overRevTimerMS = 0;
	d->KartStates.RevEngine.releaseCooldownTimerMS = 0;
	d->KartStates.RevEngine.emptyCooldownTimerMS = 0;
	d->KartStates.RevEngine.chargeState = REV_ENGINE_CHARGE_IDLE;
	d->KartStates.RevEngine.lockoutFlags = 0;

	d->KartStates.RevEngine.boostMeter = CTR_MipsAddLo(d->const_AccelSpeed_ClassStat, d->const_SacredFireSpeed / 3);
}

DriverFunc PlayerRevEngineFuncTable[DRIVER_FUNC_COUNT] = {
    NULL, VehStuckProc_RevEngine_Update, VehStuckProc_RevEngine_PhysLinear, VehPhysProc_Driving_Audio, NULL, NULL, NULL, NULL, NULL,
    NULL, VehPhysForce_TranslateMatrix,  VehStuckProc_RevEngine_Animate,    VehEmitter_DriverMain,
};


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8006809c-0x800680d0.
void VehStuckProc_Tumble_Update(struct Thread *thread, struct Driver *driver)
{
	if (driver->NoInputTimer != 0)
	{
		return;
	}

	driver->matrixArray = BAKED_GTE_MATRIX_NONE;
	driver->matrixIndex = 0;
	VehPhysProc_Driving_Init(thread, driver);
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800680d0-0x80068150.
void VehStuckProc_Tumble_PhysLinear(struct Thread *thread, struct Driver *driver)
{
	driver->NoInputTimer = (s16)CTR_MipsSubLo((u16)driver->NoInputTimer, (u16)sdata->gGT->elapsedTimeMS);

	if (driver->NoInputTimer < 0)
	{
		driver->NoInputTimer = 0;
	}

	VehPhysProc_Driving_PhysLinear(thread, driver);

	driver->jump_ForcedMS = VEH_TUMBLE_FORCED_JUMP_MS;
	driver->baseSpeed = 0;
	driver->fireSpeed = 0;
	driver->actionsFlagSet |= ACTION_WARP | ACTION_FRONT_SKID | ACTION_BACK_SKID | ACTION_ACCEL_PREVENTION;
	driver->jump_InitialVelY = (s16)CTR_MipsAddLo(CTR_MipsSll((u16)driver->NoInputTimer, VEH_TUMBLE_JUMP_VELOCITY_TIMER_SHIFT), VEH_TUMBLE_JUMP_VELOCITY_BASE);
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80068150-0x80068244.
void VehStuckProc_Tumble_PhysAngular(struct Thread *thread, struct Driver *driver)
{
	(void)thread;
	int elapsedTimeMS = sdata->gGT->elapsedTimeMS;

	driver->numFramesSpentSteering = VEH_TUMBLE_STEERING_FRAME_SENTINEL;

	driver->turnWobbleAngle = (s16)CTR_MipsSubLo((u16)driver->turnWobbleAngle, CTR_MipsSra(driver->turnWobbleAngle, VEH_TUMBLE_ANGULAR_DAMP_SHIFT));
	driver->rotationSpinRate = (s16)CTR_MipsSubLo((u16)driver->rotationSpinRate, CTR_MipsSra(driver->rotationSpinRate, VEH_TUMBLE_ANGULAR_DAMP_SHIFT));
	driver->turnAngleLerpVel = (s16)CTR_MipsSubLo((u16)driver->turnAngleLerpVel, CTR_MipsSra(driver->turnAngleLerpVel, VEH_TUMBLE_ANGULAR_DAMP_SHIFT));

	driver->ampTurnState = driver->rotationSpinRate;

	driver->turnAngleCurr = (s16)CTR_MipsSubLo(
	    CTR_MipsAddLo(CTR_MipsAddLo((u16)driver->turnAngleCurr, (u16)driver->turnAngleLerpVel), VEH_TUMBLE_TURN_WRAP_BIAS) & VEH_TUMBLE_TURN_MASK,
	    VEH_TUMBLE_TURN_WRAP_BIAS);

	driver->angle = (s16)(CTR_MipsAddLo((u16)driver->angle, CTR_MipsSra(CTR_MipsMulLo(driver->rotationSpinRate, elapsedTimeMS), VEH_TUMBLE_SPIN_RATE_SHIFT)) &
	                      VEH_TUMBLE_TURN_MASK);

	(driver->rotCurr).y = (s16)CTR_MipsAddLo(CTR_MipsAddLo((u16)driver->turnWobbleAngle, (u16)driver->angle), (u16)driver->turnAngleCurr);

	(driver->rotCurr).w = VehCalc_InterpBySpeed((int)(driver->rotCurr).w,
	                                            CTR_MipsSra(CTR_MipsSll(elapsedTimeMS, VEH_TUMBLE_ROT_W_INTERP_SHIFT), VEH_TUMBLE_ROT_W_INTERP_SHIFT), 0);

	VehPhysForce_RotAxisAngle(&driver->matrixMovingDir, driver->AxisAngle1_normalVec.v, driver->angle);
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80068244-0x800682a4.
void VehStuckProc_Tumble_Animate(struct Thread *thread, struct Driver *driver)
{
	(void)thread;
	int matrixIndex;
	int arrLength;
	int quotient;

	driver->matrixArray = BAKED_GTE_MATRIX_BLASTED;
	arrLength = data.bakedGteMath[BAKED_GTE_MATRIX_BLASTED].numEntries;

	// divide by 32ms to get frame index
	matrixIndex = CTR_MipsSra(driver->NoInputTimer, VEH_TUMBLE_ANIM_FRAME_TIME_SHIFT);

	// modulus to wrap repeat animation
	quotient = CTR_MipsDiv(matrixIndex, arrLength);
	matrixIndex = CTR_MipsSubLo(matrixIndex, CTR_MipsMulLo(quotient, arrLength));

	if (driver->KartStates.Blasted.boolPlayBackwards != 0)
	{
		matrixIndex = CTR_MipsSubLo(arrLength, CTR_MipsAddLo(matrixIndex, VEH_TUMBLE_ANIM_REVERSE_OFFSET));
	}

	driver->matrixIndex = matrixIndex;
}


DriverFunc PlayerBlastedFuncTable[DRIVER_FUNC_COUNT] = {NULL,
                                                        VehStuckProc_Tumble_Update,
                                                        VehStuckProc_Tumble_PhysLinear,
                                                        VehPhysProc_Driving_Audio,
                                                        VehStuckProc_Tumble_PhysAngular,
                                                        VehPhysForce_OnApplyForces,
                                                        COLL_MOVED_PlayerSearch,
                                                        VehPhysForce_CollideDrivers,
                                                        COLL_FIXED_PlayerSearch,
                                                        VehPhysGeneral_JumpAndFriction,
                                                        VehPhysForce_TranslateMatrix,
                                                        VehStuckProc_Tumble_Animate,
                                                        VehEmitter_DriverMain};

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800682a4-0x800683f4.
void VehStuckProc_Tumble_Init(struct Thread *thread, struct Driver *driver)
{
	(void)thread;
	driver->kartState = KS_BLASTED;
	driver->turbo_MeterRoomLeft = 0;

	if (LOAD_IsOpen_RacingOrBattle() && ((sdata->gGT->gameMode1 & ADVENTURE_ARENA) == 0))
	{
		RB_Player_ModifyWumpa(driver, -VEH_TUMBLE_WUMPA_PENALTY);
	}

	driver->instSelf->animIndex = 0;

	int numAnimFrames = VehFrameInst_GetNumAnimFrames(driver->instSelf, 0);
	int animFrame = VehFrameInst_GetStartFrame(0, numAnimFrames);

	driver->instSelf->animFrame = (s16)animFrame;

	int rng = MixRNG_Scramble();
	driver->KartStates.Blasted.boolPlayBackwards = rng & VEH_TUMBLE_BACKWARDS_RNG_MASK;

	s8 simpTurnState = driver->simpTurnState;
	int rumbleStrength;
	if (simpTurnState < 1)
	{
		rumbleStrength = VEH_TUMBLE_RUMBLE_WEAK;
	}
	else
	{
		rumbleStrength = VEH_TUMBLE_RUMBLE_STRONG;
	}

	for (int i = 0; i < DRIVER_FUNC_COUNT; i++)
	{
		driver->funcPtrs[i] = PlayerBlastedFuncTable[i];
	}

	GAMEPAD_JogCon1(driver, rumbleStrength, VEH_TUMBLE_RUMBLE_DURATION);
}

enum
{
	VEH_WARP_DUST_SEGMENTS = 16,
	VEH_WARP_DUST_POINT_STRIDE_SHORTS = 4,
	VEH_WARP_DUST_RANDOM_MASK = 0xfff,
	VEH_WARP_DUST_RANDOM_SHIFT = FRACTIONAL_BITS,
	VEH_WARP_DUST_RECURSION_MIN_SPAN = 2,
	VEH_WARP_DUST_RADIUS_DECAY = 0xc00,
	VEH_WARP_DUST_SPAWN_TIMER_BIT = 1,
	VEH_WARP_DUST_JITTER_SCALE_SHIFT = 5,
	VEH_WARP_DUST_CAMERA_OFFSET_SHIFT = 10,
	VEH_WARP_DUST_POSITION_SHIFT = FRACTIONAL_BITS_8,
	VEH_WARP_DUST_RING_COUNT = 6,
	VEH_WARP_DUST_RING_ANGLE_SHIFT = 12,
	VEH_WARP_DUST_HIDDEN_BASE_OFFSET_SHIFT = 5,
	VEH_WARP_DUST_VISIBLE_BASE_OFFSET_SHIFT = 6,
	VEH_WARP_DUST_VISIBLE_ENDPOINT_OFFSET_SHIFT = 8,
	VEH_WARP_DUST_MOVE_RADIUS = 0x100,
	VEH_WARP_DUST_WAVE_ANGLE_SHIFT = 7,
	VEH_WARP_DUST_WAVE_HEIGHT_SHIFT = 7,
	VEH_WARP_DUST_OT_DEPTH_SHIFT = 6,
	VEH_WARP_BEAM_HEIGHT_OFFSET = 0x100,
	VEH_WARP_DUST_ANGLE_STEP = 100,
	VEH_WARP_TIMER_STEP = 26,
	VEH_WARP_TIMER_MAX = 800,
	VEH_WARP_EXPAND_SCALE_SPEED = 120,
	VEH_WARP_EXPAND_SCALE_TARGET_XZ = 4800,
	VEH_WARP_LIFT_HEIGHT_LIMIT = 0x8000,
	VEH_WARP_LIFT_STEP = 0x800,
	VEH_WARP_LAUNCH_REV_STATE = 2,
	VEH_WARP_SHRINK_SCALE_SPEED_XZ = 600,
	VEH_WARP_SHRINK_SCALE_SPEED_Y = 3200,
	VEH_WARP_SHRINK_SCALE_TARGET_Y = 24000,
	VEH_WARP_FLARE_HEIGHT_OFFSET = 0x40,
	VEH_WARP_HEIGHT_OFFSET_STEP = 0x1800,
	VEH_WARP_TURN_WRAP_BIAS = 0x800,
	VEH_WARP_TURN_MASK = 0xfff,
	VEH_WARP_INITIAL_TIMER = 0x3c,
	VEH_WARP_SOUND_ID = 0x97,
	VEH_WARP_ENGINE_AUDIO_STRIDE = 4,
	VEH_WARP_POSITION_SHIFT = FRACTIONAL_BITS_8,
};

CTR_STATIC_ASSERT(VEH_WARP_DUST_SEGMENTS == 16);
CTR_STATIC_ASSERT(VEH_WARP_DUST_POINT_STRIDE_SHORTS == 4);
CTR_STATIC_ASSERT(VEH_WARP_DUST_RANDOM_MASK == 0xfff);
CTR_STATIC_ASSERT(VEH_WARP_DUST_RANDOM_SHIFT == 0xc);
CTR_STATIC_ASSERT(VEH_WARP_DUST_RECURSION_MIN_SPAN == 2);
CTR_STATIC_ASSERT(VEH_WARP_DUST_RADIUS_DECAY == 0xc00);
CTR_STATIC_ASSERT(VEH_WARP_DUST_SPAWN_TIMER_BIT == 1);
CTR_STATIC_ASSERT(VEH_WARP_DUST_JITTER_SCALE_SHIFT == 5);
CTR_STATIC_ASSERT(VEH_WARP_DUST_CAMERA_OFFSET_SHIFT == 10);
CTR_STATIC_ASSERT(VEH_WARP_DUST_POSITION_SHIFT == 8);
CTR_STATIC_ASSERT(VEH_WARP_DUST_RING_COUNT == 6);
CTR_STATIC_ASSERT(VEH_WARP_DUST_RING_ANGLE_SHIFT == 12);
CTR_STATIC_ASSERT(VEH_WARP_DUST_HIDDEN_BASE_OFFSET_SHIFT == 5);
CTR_STATIC_ASSERT(VEH_WARP_DUST_VISIBLE_BASE_OFFSET_SHIFT == 6);
CTR_STATIC_ASSERT(VEH_WARP_DUST_VISIBLE_ENDPOINT_OFFSET_SHIFT == 8);
CTR_STATIC_ASSERT(VEH_WARP_DUST_MOVE_RADIUS == 0x100);
CTR_STATIC_ASSERT(VEH_WARP_DUST_WAVE_ANGLE_SHIFT == 7);
CTR_STATIC_ASSERT(VEH_WARP_DUST_WAVE_HEIGHT_SHIFT == 7);
CTR_STATIC_ASSERT(VEH_WARP_DUST_OT_DEPTH_SHIFT == 6);
CTR_STATIC_ASSERT(VEH_WARP_BEAM_HEIGHT_OFFSET == 0x100);
CTR_STATIC_ASSERT(VEH_WARP_DUST_ANGLE_STEP == 100);
CTR_STATIC_ASSERT(VEH_WARP_TIMER_STEP == 26);
CTR_STATIC_ASSERT(VEH_WARP_TIMER_MAX == 800);
CTR_STATIC_ASSERT(VEH_WARP_EXPAND_SCALE_SPEED == 120);
CTR_STATIC_ASSERT(VEH_WARP_EXPAND_SCALE_TARGET_XZ == 4800);
CTR_STATIC_ASSERT(VEH_WARP_LIFT_HEIGHT_LIMIT == 0x8000);
CTR_STATIC_ASSERT(VEH_WARP_LIFT_STEP == 0x800);
CTR_STATIC_ASSERT(VEH_WARP_LAUNCH_REV_STATE == 2);
CTR_STATIC_ASSERT(VEH_WARP_SHRINK_SCALE_SPEED_XZ == 600);
CTR_STATIC_ASSERT(VEH_WARP_SHRINK_SCALE_SPEED_Y == 3200);
CTR_STATIC_ASSERT(VEH_WARP_SHRINK_SCALE_TARGET_Y == 24000);
CTR_STATIC_ASSERT(VEH_WARP_FLARE_HEIGHT_OFFSET == 0x40);
CTR_STATIC_ASSERT(VEH_WARP_HEIGHT_OFFSET_STEP == 0x1800);
CTR_STATIC_ASSERT(VEH_WARP_TURN_WRAP_BIAS == 0x800);
CTR_STATIC_ASSERT(VEH_WARP_TURN_MASK == 0xfff);
CTR_STATIC_ASSERT(VEH_WARP_INITIAL_TIMER == 0x3c);
CTR_STATIC_ASSERT(VEH_WARP_SOUND_ID == 0x97);
CTR_STATIC_ASSERT(VEH_WARP_ENGINE_AUDIO_STRIDE == 4);
CTR_STATIC_ASSERT(VEH_WARP_POSITION_SHIFT == 8);
CTR_STATIC_ASSERT(sizeof(SVECTOR) == VEH_WARP_DUST_POINT_STRIDE_SHORTS * sizeof(s16));

static const u32 VEH_WARP_DUST_DRAW_MODE = 0xe1000a20u;
static const u32 VEH_WARP_DUST_POLY_G4_CODE = 0x3a000000u;
static const u32 VEH_WARP_DUST_EDGE_COLOR = 0x007f1f3fu;
static const u32 VEH_WARP_DUST_PACKET_TAG = 0x11000000u;

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800683f4-0x800685b0.
void VehStuckProc_Warp_MoveDustPuff(s16 *points, int span, int radius, s16 *jitterScale)
{
	int radiusHalf = CTR_MipsSra(radius, 1);

	int jitterX = CTR_MipsSra(CTR_MipsMulLo(MixRNG_Scramble() & VEH_WARP_DUST_RANDOM_MASK, radius), VEH_WARP_DUST_RANDOM_SHIFT);
	if (jitterX < radiusHalf)
	{
		jitterX = CTR_MipsSubLo(jitterX, radius);
	}

	int jitterY = CTR_MipsSra(CTR_MipsMulLo(MixRNG_Scramble() & VEH_WARP_DUST_RANDOM_MASK, radius), VEH_WARP_DUST_RANDOM_SHIFT);
	if (jitterY < radiusHalf)
	{
		jitterY = CTR_MipsSubLo(jitterY, radius);
	}

	int jitterZ = CTR_MipsSra(CTR_MipsMulLo(MixRNG_Scramble() & VEH_WARP_DUST_RANDOM_MASK, radius), VEH_WARP_DUST_RANDOM_SHIFT);
	if (jitterZ < radiusHalf)
	{
		jitterZ = CTR_MipsSubLo(jitterZ, radius);
	}

	s16 *end = points + span * VEH_WARP_DUST_POINT_STRIDE_SHORTS;
	int halfSpan = CTR_MipsSra(span, 1);
	s16 *mid = points + halfSpan * VEH_WARP_DUST_POINT_STRIDE_SHORTS;

	mid[0] =
	    (s16)CTR_MipsAddLo(CTR_MipsSra(CTR_MipsAddLo(points[0], end[0]), 1), CTR_MipsSra(CTR_MipsMulLo(jitterScale[0], jitterX), VEH_WARP_DUST_RANDOM_SHIFT));
	mid[1] =
	    (s16)CTR_MipsAddLo(CTR_MipsSra(CTR_MipsAddLo(points[1], end[1]), 1), CTR_MipsSra(CTR_MipsMulLo(jitterScale[1], jitterY), VEH_WARP_DUST_RANDOM_SHIFT));
	mid[2] =
	    (s16)CTR_MipsAddLo(CTR_MipsSra(CTR_MipsAddLo(points[2], end[2]), 1), CTR_MipsSra(CTR_MipsMulLo(jitterScale[2], jitterZ), VEH_WARP_DUST_RANDOM_SHIFT));

	if (span > VEH_WARP_DUST_RECURSION_MIN_SPAN)
	{
		int nextRadius = CTR_MipsSra(CTR_MipsMulLo(radius, VEH_WARP_DUST_RADIUS_DECAY), VEH_WARP_DUST_RANDOM_SHIFT);
		VehStuckProc_Warp_MoveDustPuff(points, halfSpan, nextRadius, jitterScale);
		VehStuckProc_Warp_MoveDustPuff(mid, halfSpan, nextRadius, jitterScale);
	}
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800685b0-0x80068644.
void VehStuckProc_Warp_AddDustPuff1(struct ScratchpadStruct *sps)
{
	struct GameTracker *gGT = sdata->gGT;

	// if even frame don't spawn
	if (gGT->timer & VEH_WARP_DUST_SPAWN_TIMER_BIT)
	{
		return;
	}

	struct Particle *p = Particle_Init(0, gGT->iconGroup[1], &data.emSet_Warppad[0]);

	if (p == NULL)
	{
		return;
	}

	// position variables
	for (s32 i = 0; i < 3; i++)
	{
		p->axis[i].startVal = CTR_MipsAddLo(p->axis[i].startVal, CTR_MipsSll(sps->Input1.pos.v[i], FRACTIONAL_BITS_8));
	}
}


struct VehWarpDustProjected
{
	u32 sxy0;
	u32 sxy1;
	u32 sxy2;
	u32 depth;
};

struct VehWarpDustScratch
{
	u8 pad_000[0x108];
	SVECTOR points[VEH_WARP_DUST_SEGMENTS + 1];
	SVECTOR projectLeft;
	SVECTOR projectRight;
	struct VehWarpDustProjected prev;
	struct VehWarpDustProjected curr;
	SVec3 jitterScale;
};

struct VehWarpDustG4Body
{
	u32 color0AndCode;
	u32 xy0;
	u32 color1;
	u32 xy1;
	u32 color2;
	u32 xy2;
	u32 color3;
	u32 xy3;
};

struct VehWarpDustPacket
{
	u32 tag;
	u32 drawMode;
	struct VehWarpDustG4Body leftStrip;
	struct VehWarpDustG4Body rightStrip;
};

CTR_STATIC_ASSERT(sizeof(struct VehWarpDustG4Body) == 0x20);
CTR_STATIC_ASSERT(offsetof(struct VehWarpDustG4Body, color0AndCode) == 0x00);
CTR_STATIC_ASSERT(offsetof(struct VehWarpDustG4Body, xy0) == 0x04);
CTR_STATIC_ASSERT(offsetof(struct VehWarpDustG4Body, color1) == 0x08);
CTR_STATIC_ASSERT(offsetof(struct VehWarpDustG4Body, xy1) == 0x0C);
CTR_STATIC_ASSERT(offsetof(struct VehWarpDustG4Body, color2) == 0x10);
CTR_STATIC_ASSERT(offsetof(struct VehWarpDustG4Body, xy2) == 0x14);
CTR_STATIC_ASSERT(offsetof(struct VehWarpDustG4Body, color3) == 0x18);
CTR_STATIC_ASSERT(offsetof(struct VehWarpDustG4Body, xy3) == 0x1C);

CTR_STATIC_ASSERT(sizeof(struct VehWarpDustPacket) == 0x48);
CTR_STATIC_ASSERT(offsetof(struct VehWarpDustPacket, tag) == 0x00);
CTR_STATIC_ASSERT(offsetof(struct VehWarpDustPacket, drawMode) == 0x04);
CTR_STATIC_ASSERT(offsetof(struct VehWarpDustPacket, leftStrip) == 0x08);
CTR_STATIC_ASSERT(offsetof(struct VehWarpDustPacket, rightStrip) == 0x28);
CTR_STATIC_ASSERT(offsetof(struct VehWarpDustScratch, points) == 0x108);
CTR_STATIC_ASSERT(offsetof(struct VehWarpDustScratch, projectLeft) == 0x190);
CTR_STATIC_ASSERT(offsetof(struct VehWarpDustScratch, projectRight) == 0x198);
CTR_STATIC_ASSERT(offsetof(struct VehWarpDustScratch, prev) == 0x1a0);
CTR_STATIC_ASSERT(offsetof(struct VehWarpDustScratch, curr) == 0x1b0);
CTR_STATIC_ASSERT(offsetof(struct VehWarpDustScratch, jitterScale) == 0x1c0);

static s16 VehWarpDust_AddHalf(s16 value, int delta)
{
	return (s16)CTR_MipsAddLo((u16)value, delta);
}

static void VehWarpDust_Project(struct VehWarpDustScratch *scratch, SVECTOR *point, int offsetX, int offsetY, int offsetZ, struct VehWarpDustProjected *out)
{
	SVECTOR *left = &scratch->projectLeft;
	SVECTOR *right = &scratch->projectRight;

	left->vx = VehWarpDust_AddHalf(point->vx, offsetX);
	left->vy = VehWarpDust_AddHalf(point->vy, offsetY);
	left->vz = VehWarpDust_AddHalf(point->vz, offsetZ);

	right->vx = VehWarpDust_AddHalf(point->vx, CTR_MipsNegLo(offsetX));
	right->vy = VehWarpDust_AddHalf(point->vy, CTR_MipsNegLo(offsetY));
	right->vz = VehWarpDust_AddHalf(point->vz, CTR_MipsNegLo(offsetZ));

	CTR_GteLoadSV3(left, point, right);
	gte_rtpt_b();

	out->sxy0 = MFC2(12);
	out->sxy1 = MFC2(13);
	out->sxy2 = MFC2(14);
	out->depth = MFC2(17);
}

static void VehWarpDust_EmitSegment(u32 **primCursor, struct PushBuffer *pb, const struct VehWarpDustProjected *prev, const struct VehWarpDustProjected *curr)
{
	struct VehWarpDustPacket *packet = (struct VehWarpDustPacket *)*primCursor;
	u32 *ot = pb->ptrOT + CTR_MipsSra((s32)curr->depth, VEH_WARP_DUST_OT_DEPTH_SHIFT);

	packet->drawMode = VEH_WARP_DUST_DRAW_MODE;

	packet->leftStrip.color0AndCode = VEH_WARP_DUST_POLY_G4_CODE;
	packet->leftStrip.xy0 = curr->sxy0;
	packet->leftStrip.color1 = VEH_WARP_DUST_EDGE_COLOR;
	packet->leftStrip.xy1 = curr->sxy1;
	packet->leftStrip.color2 = 0;
	packet->leftStrip.xy2 = prev->sxy0;
	packet->leftStrip.color3 = VEH_WARP_DUST_EDGE_COLOR;
	packet->leftStrip.xy3 = prev->sxy1;

	packet->rightStrip.color0AndCode = VEH_WARP_DUST_POLY_G4_CODE;
	packet->rightStrip.xy0 = curr->sxy2;
	packet->rightStrip.color1 = VEH_WARP_DUST_EDGE_COLOR;
	packet->rightStrip.xy1 = curr->sxy1;
	packet->rightStrip.color2 = 0;
	packet->rightStrip.xy2 = prev->sxy2;
	packet->rightStrip.color3 = VEH_WARP_DUST_EDGE_COLOR;
	packet->rightStrip.xy3 = prev->sxy1;

	CtrGpu_LinkPacket24(ot, &packet->tag, packet, VEH_WARP_DUST_PACKET_TAG);
	*primCursor = (u32 *)(packet + 1);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80068644-0x80068be8.
void VehStuckProc_Warp_AddDustPuff2(struct Driver *d, struct DriverWarpState *warp)
{
	struct GameTracker *gGT = sdata->gGT;
	struct PushBuffer *pb = &gGT->pushBuffer[d->driverID];
	struct DB *backBuffer = gGT->backBuffer;
	u32 *prim = backBuffer->primMem.cursor;
	struct VehWarpDustScratch *scratch = CTR_SCRATCHPAD_PTR(struct VehWarpDustScratch, 0);
	SVECTOR *points = scratch->points;
	SVECTOR *endpoint = &points[VEH_WARP_DUST_SEGMENTS];
	s16 *jitterScale = scratch->jitterScale.v;
	int offsetX;
	int offsetY;
	int offsetZ;

	gte_SetRotMatrix(&pb->matrix_ViewProj);
	gte_SetTransMatrix(&pb->matrix_ViewProj);

	jitterScale[0] = (s16)CTR_MipsSra(CTR_MipsAddLo(pb->matrix_CameraTranspose.m[0][0], pb->matrix_CameraTranspose.m[0][1]), VEH_WARP_DUST_JITTER_SCALE_SHIFT);
	jitterScale[1] = (s16)CTR_MipsSra(CTR_MipsAddLo(pb->matrix_CameraTranspose.m[1][0], pb->matrix_CameraTranspose.m[1][1]), VEH_WARP_DUST_JITTER_SCALE_SHIFT);
	jitterScale[2] = (s16)CTR_MipsSra(CTR_MipsAddLo(pb->matrix_CameraTranspose.m[2][0], pb->matrix_CameraTranspose.m[2][1]), VEH_WARP_DUST_JITTER_SCALE_SHIFT);

	offsetX = CTR_MipsSra(pb->matrix_CameraTranspose.m[0][0], VEH_WARP_DUST_CAMERA_OFFSET_SHIFT);
	offsetY = CTR_MipsSra(pb->matrix_CameraTranspose.m[1][0], VEH_WARP_DUST_CAMERA_OFFSET_SHIFT);
	offsetZ = CTR_MipsSra(pb->matrix_CameraTranspose.m[2][0], VEH_WARP_DUST_CAMERA_OFFSET_SHIFT);

	if ((d->instSelf->flags & HIDE_MODEL) != 0)
	{
		endpoint->vx = (s16)CTR_MipsSra(d->posCurr.x, VEH_WARP_DUST_POSITION_SHIFT);
		endpoint->vy = (s16)CTR_MipsSra(warp->beamHeight, VEH_WARP_DUST_POSITION_SHIFT);
		endpoint->vz = (s16)CTR_MipsSra(d->posCurr.z, VEH_WARP_DUST_POSITION_SHIFT);
		VehStuckProc_Warp_AddDustPuff1((struct ScratchpadStruct *)endpoint);
	}

	for (int ring = 0; ring < VEH_WARP_DUST_RING_COUNT; ring++)
	{
		int baseAngle = CTR_MipsAddLo((CTR_MipsSll(ring, VEH_WARP_DUST_RING_ANGLE_SHIFT) / VEH_WARP_DUST_RING_COUNT), warp->dustAngle);
		struct VehWarpDustProjected *prev = &scratch->prev;
		struct VehWarpDustProjected *curr = &scratch->curr;

		points[0].vx = (s16)CTR_MipsSubLo(CTR_MipsSra(d->posCurr.x, VEH_WARP_DUST_POSITION_SHIFT),
		                                  CTR_MipsSra(MATH_Sin(baseAngle), VEH_WARP_DUST_HIDDEN_BASE_OFFSET_SHIFT));
		points[0].vy = (s16)CTR_MipsSra(warp->quadHeight, VEH_WARP_DUST_POSITION_SHIFT);
		points[0].vz = (s16)CTR_MipsSubLo(CTR_MipsSra(d->posCurr.z, VEH_WARP_DUST_POSITION_SHIFT),
		                                  CTR_MipsSra(MATH_Cos(baseAngle), VEH_WARP_DUST_HIDDEN_BASE_OFFSET_SHIFT));

		endpoint->vx = (s16)CTR_MipsSra(d->posCurr.x, VEH_WARP_DUST_POSITION_SHIFT);
		endpoint->vy = (s16)CTR_MipsSra(warp->beamHeight, VEH_WARP_DUST_POSITION_SHIFT);
		endpoint->vz = (s16)CTR_MipsSra(d->posCurr.z, VEH_WARP_DUST_POSITION_SHIFT);

		if ((d->instSelf->flags & HIDE_MODEL) == 0)
		{
			points[0].vx = VehWarpDust_AddHalf(points[0].vx, CTR_MipsNegLo(CTR_MipsSra(MATH_Sin(baseAngle), VEH_WARP_DUST_VISIBLE_BASE_OFFSET_SHIFT)));
			points[0].vz = VehWarpDust_AddHalf(points[0].vz, CTR_MipsNegLo(CTR_MipsSra(MATH_Cos(baseAngle), VEH_WARP_DUST_VISIBLE_BASE_OFFSET_SHIFT)));
			endpoint->vx = VehWarpDust_AddHalf(endpoint->vx, CTR_MipsSra(MATH_Sin(baseAngle), VEH_WARP_DUST_VISIBLE_ENDPOINT_OFFSET_SHIFT));
			endpoint->vz = VehWarpDust_AddHalf(endpoint->vz, CTR_MipsSra(MATH_Cos(baseAngle), VEH_WARP_DUST_VISIBLE_ENDPOINT_OFFSET_SHIFT));
		}
		else
		{
			VehStuckProc_Warp_AddDustPuff1((struct ScratchpadStruct *)points);
		}

		VehStuckProc_Warp_MoveDustPuff((s16 *)points, VEH_WARP_DUST_SEGMENTS, VEH_WARP_DUST_MOVE_RADIUS, jitterScale);

		for (int i = 1; i < VEH_WARP_DUST_SEGMENTS; i++)
		{
			points[i].vy =
			    VehWarpDust_AddHalf(points[i].vy, CTR_MipsSra(MATH_Sin(CTR_MipsSll(i, VEH_WARP_DUST_WAVE_ANGLE_SHIFT)), VEH_WARP_DUST_WAVE_HEIGHT_SHIFT));
		}

		VehWarpDust_Project(scratch, &points[0], offsetX, offsetY, offsetZ, prev);

		for (int seg = 0; seg < VEH_WARP_DUST_SEGMENTS; seg++)
		{
			struct VehWarpDustProjected *tmp;

			VehWarpDust_Project(scratch, &points[seg + 1], offsetX, offsetY, offsetZ, curr);
			VehWarpDust_EmitSegment(&prim, pb, prev, curr);

			tmp = prev;
			prev = curr;
			curr = tmp;
		}
	}

	backBuffer->primMem.cursor = prim;
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80068be8-0x80068e04.
void VehStuckProc_Warp_PhysAngular(struct Thread *th, struct Driver *d)
{
	(void)th;
	int warpTimer;
	SVec4 flarePos;

	// get instance from driver object
	struct Instance *inst = d->instSelf;

	// if driver is visible
	if ((inst->flags & HIDE_MODEL) == 0)
	{
		// beam starts just above the kart
		int beamHeight = CTR_MipsAddLo(d->posCurr.y, VEH_WARP_BEAM_HEIGHT_OFFSET);

		if (beamHeight < d->KartStates.Warp.quadHeight)
		{
			beamHeight = d->KartStates.Warp.quadHeight;
		}

		d->KartStates.Warp.beamHeight = beamHeight;

		d->KartStates.Warp.dustAngle = CTR_MipsSubLo(d->KartStates.Warp.dustAngle, VEH_WARP_DUST_ANGLE_STEP);

		// add dust puff
		VehStuckProc_Warp_AddDustPuff2(d, &d->KartStates.Warp);
	}

	warpTimer = d->KartStates.Warp.timer;
	warpTimer = CTR_MipsAddLo(warpTimer, VEH_WARP_TIMER_STEP);

	if (warpTimer <= VEH_WARP_TIMER_MAX)
	{
		for (s32 i = 0; i < 3; i++)
		{
			inst->scale.v[i] = VehCalc_InterpBySpeed(inst->scale.v[i], VEH_WARP_EXPAND_SCALE_SPEED, VEH_WARP_EXPAND_SCALE_TARGET_XZ >> (i & 1));
		}

		if (d->posCurr.y < CTR_MipsAddLo(d->quadBlockHeight, VEH_WARP_LIFT_HEIGHT_LIMIT))
		{
			d->posCurr.y = CTR_MipsAddLo(d->posCurr.y, VEH_WARP_LIFT_STEP);
		}
	}
	else
	{
		// cap to final warp phase
		warpTimer = VEH_WARP_TIMER_MAX;

		d->revEngineState = VEH_WARP_LAUNCH_REV_STATE;

		for (s32 i = 0; i < 3; i++)
		{
			inst->scale.v[i] = VehCalc_InterpBySpeed(inst->scale.v[i], (i == 1) ? VEH_WARP_SHRINK_SCALE_SPEED_Y : VEH_WARP_SHRINK_SCALE_SPEED_XZ,
			                                         VEH_WARP_SHRINK_SCALE_TARGET_Y * (i & 1));
		}

		// if scale shrinks to zero
		if (inst->scale.x == 0)
		{
			// if car is visible
			if ((inst->flags & HIDE_MODEL) == 0)
			{
				// position above kart
				flarePos.x = (s16)CTR_MipsSra(d->posCurr.x, VEH_WARP_POSITION_SHIFT);
				flarePos.y = (s16)CTR_MipsAddLo(CTR_MipsSra(d->KartStates.Warp.quadHeight, VEH_WARP_POSITION_SHIFT), VEH_WARP_FLARE_HEIGHT_OFFSET);
				flarePos.z = (s16)CTR_MipsSra(d->posCurr.z, VEH_WARP_POSITION_SHIFT);

				FLARE_Init(flarePos.v);
			}

			// make invisible
			inst->flags |= HIDE_MODEL;
		}

		else
		{
			d->KartStates.Warp.heightOffset = CTR_MipsSubLo(d->KartStates.Warp.heightOffset, VEH_WARP_HEIGHT_OFFSET_STEP);
			d->posCurr.y = CTR_MipsAddLo(d->posCurr.y, d->KartStates.Warp.heightOffset);
		}
	}

	// drift angle = ((drift angle + warp timer + 0x800) & 0xfff) - 0x800
	s16 wrappedTurnAngle = (s16)CTR_MipsSubLo(CTR_MipsAddLo(CTR_MipsAddLo((u16)d->turnAngleCurr, (u16)warpTimer), VEH_WARP_TURN_WRAP_BIAS) & VEH_WARP_TURN_MASK,
	                                          VEH_WARP_TURN_WRAP_BIAS);
	d->turnAngleCurr = wrappedTurnAngle;

	// cameraRotY = wobble angle + kart angle + wrapped warp turn angle
	d->rotCurr.y = (s16)CTR_MipsAddLo(CTR_MipsAddLo((u16)d->turnWobbleAngle, (u16)d->angle), (u16)wrappedTurnAngle);

	// driver is warping
	d->actionsFlagSet |= ACTION_WARP;

	d->KartStates.Warp.timer = warpTimer;
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80068e04-0x80068f90.
void VehStuckProc_Warp_Init(struct Thread *th, struct Driver *d)
{
	(void)th;
	if (d->kartState == KS_WARP_PAD)
	{
		return;
	}

	// If you are not in a warp pad

	d->KartStates.Warp.timer = VEH_WARP_INITIAL_TIMER;
	d->KartStates.Warp.heightOffset = 0;
	d->KartStates.Warp.quadHeight = d->quadBlockHeight;

	// Warp sound?
	OtherFX_Play(VEH_WARP_SOUND_ID, 1);

	OtherFX_Stop1((int)d->driverAudioPtrs[1]);
	d->driverAudioPtrs[1] = 0;
	OtherFX_Stop1((int)d->driverAudioPtrs[2]);
	d->driverAudioPtrs[2] = 0;
	OtherFX_Stop1((int)d->driverAudioPtrs[0]);
	d->driverAudioPtrs[0] = 0;

	u8 playerID = d->driverID;

	int engine = data.MetaDataCharacters[data.characterIDs[playerID]].engineID;

	EngineAudio_Stop((engine * VEH_WARP_ENGINE_AUDIO_STRIDE) + playerID);

	// CameraDC, freecam mode
	sdata->gGT->cameraDC[playerID].cameraMode = CAMERA_MODE_FREECAM;

	// driver -> instSelf
	struct Instance *inst = d->instSelf;

	// instance flags, now reflective
	inst->flags |= REFLECTIVE;

	// vertical line for split or reflection
	inst->vertSplit = (s16)CTR_MipsSra(d->quadBlockHeight, VEH_WARP_POSITION_SHIFT);

	// you are now in a warp pad
	d->kartState = KS_WARP_PAD;

	d->speed = 0;
	d->speedApprox = 0;

	d->funcPtrs[DRIVER_FUNC_INIT] = NULL;
	d->funcPtrs[DRIVER_FUNC_UPDATE] = NULL;
	d->funcPtrs[DRIVER_FUNC_PHYS_LINEAR] = NULL;
	d->funcPtrs[DRIVER_FUNC_AUDIO] = VehPhysProc_Driving_Audio;
	d->funcPtrs[DRIVER_FUNC_PHYS_ANGULAR] = VehStuckProc_Warp_PhysAngular;
	d->funcPtrs[DRIVER_FUNC_APPLY_FORCES] = NULL;
	d->funcPtrs[DRIVER_FUNC_COLL_MOVED] = NULL;
	d->funcPtrs[DRIVER_FUNC_COLLIDE_DRIVERS] = NULL;
	d->funcPtrs[DRIVER_FUNC_COLL_FIXED] = NULL;
	d->funcPtrs[DRIVER_FUNC_JUMP_FRICTION] = NULL;
	d->funcPtrs[DRIVER_FUNC_TRANSLATE_MATRIX] = VehPhysForce_TranslateMatrix;
	d->funcPtrs[DRIVER_FUNC_ANIMATE] = VehFrameProc_Driving;
	d->funcPtrs[DRIVER_FUNC_PARTICLES] = VehEmitter_DriverMain;

	// driver is warping
	d->actionsFlagSet |= ACTION_WARP;
}
