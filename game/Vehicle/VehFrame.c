#include <common.h>

enum
{
	VEH_FRAME_ANIM_DRIVE = 0,
	VEH_FRAME_ANIM_STATIONARY = 1,
	VEH_FRAME_ANIM_MATRIX_FIRST = 2,
	VEH_FRAME_ANIM_AIRBORNE = 3,
	VEH_FRAME_ANIM_END_START = 4,
	VEH_FRAME_MATRIX_ANIM_COUNT = 2,
	VEH_FRAME_NUM_FRAMES_MASK = 0x7fff,
	VEH_FRAME_BURN_SMOKE_ICON_GROUP = 1,
	VEH_FRAME_JUMP_AIR_THRESHOLD = 0x600,
	VEH_FRAME_AIR_GAP_THRESHOLD = 0x8000,
	VEH_FRAME_BURN_TIMER_LIMIT = 0x1e0,
	VEH_FRAME_BURN_TIMER_SHIFT = 5,
	VEH_FRAME_BURN_FRAME_PERIOD = 5,
	VEH_FRAME_BURN_FRAME_SHIFT = 2,
	VEH_FRAME_BURN_FRAME_BIAS = 8,
	VEH_FRAME_TURN_ACCEL_PREVENTION_LIMIT = 0x40,
	VEH_FRAME_TRANSITION_DEFAULT_SPEED = 2,
	VEH_FRAME_TRANSITION_DRIVE_SPEED = 6,
	VEH_FRAME_TRANSITION_MATRIX_SPEED = 1,
	VEH_FRAME_INTERP_SPEED_NORMAL = 1,
	VEH_FRAME_SPIN_INTERP_SPEED = 4,
	VEH_FRAME_LAST_SPIN_INTERP_SPEED = 3,
	VEH_FRAME_AIRBORNE_MATRIX_BASE = BAKED_GTE_MATRIX_JUMP_BASE,
	VEH_FRAME_OXIDE_MATRIX_ARRAY = BAKED_GTE_MATRIX_JUMP_OXIDE,
};

CTR_STATIC_ASSERT(VEH_FRAME_ANIM_DRIVE == 0);
CTR_STATIC_ASSERT(VEH_FRAME_ANIM_STATIONARY == 1);
CTR_STATIC_ASSERT(VEH_FRAME_ANIM_MATRIX_FIRST == 2);
CTR_STATIC_ASSERT(VEH_FRAME_ANIM_AIRBORNE == 3);
CTR_STATIC_ASSERT(VEH_FRAME_ANIM_END_START == 4);
CTR_STATIC_ASSERT(VEH_FRAME_MATRIX_ANIM_COUNT == 2);
CTR_STATIC_ASSERT(VEH_FRAME_NUM_FRAMES_MASK == 0x7fff);
CTR_STATIC_ASSERT(VEH_FRAME_BURN_SMOKE_ICON_GROUP == 1);
CTR_STATIC_ASSERT(VEH_FRAME_JUMP_AIR_THRESHOLD == 0x600);
CTR_STATIC_ASSERT(VEH_FRAME_AIR_GAP_THRESHOLD == 0x8000);
CTR_STATIC_ASSERT(VEH_FRAME_BURN_TIMER_LIMIT == 0x1e0);
CTR_STATIC_ASSERT(VEH_FRAME_BURN_TIMER_SHIFT == 5);
CTR_STATIC_ASSERT(VEH_FRAME_BURN_FRAME_PERIOD == 5);
CTR_STATIC_ASSERT(VEH_FRAME_BURN_FRAME_SHIFT == 2);
CTR_STATIC_ASSERT(VEH_FRAME_BURN_FRAME_BIAS == 8);
CTR_STATIC_ASSERT(VEH_FRAME_TURN_ACCEL_PREVENTION_LIMIT == 0x40);
CTR_STATIC_ASSERT(VEH_FRAME_TRANSITION_DEFAULT_SPEED == 2);
CTR_STATIC_ASSERT(VEH_FRAME_TRANSITION_DRIVE_SPEED == 6);
CTR_STATIC_ASSERT(VEH_FRAME_TRANSITION_MATRIX_SPEED == 1);
CTR_STATIC_ASSERT(VEH_FRAME_INTERP_SPEED_NORMAL == 1);
CTR_STATIC_ASSERT(VEH_FRAME_SPIN_INTERP_SPEED == 4);
CTR_STATIC_ASSERT(VEH_FRAME_LAST_SPIN_INTERP_SPEED == 3);
CTR_STATIC_ASSERT(VEH_FRAME_AIRBORNE_MATRIX_BASE == 7);
CTR_STATIC_ASSERT(VEH_FRAME_OXIDE_MATRIX_ARRAY == 7);

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005b0c4-0x8005b0f4.
int VehFrameInst_GetStartFrame(int animIndex, int numFrames)
{
	switch (animIndex)
	{
	// midpoint
	case VEH_FRAME_ANIM_DRIVE:
		return numFrames >> 1;

	// end
	case VEH_FRAME_ANIM_END_START:
		return (numFrames - 1);

	// start
	default:
		return 0;
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005b0f4-0x8005b178.
u32 VehFrameInst_GetNumAnimFrames(struct Instance *inst, int animIndex)
{
	if (inst->model == NULL)
	{
		return 0;
	}
	if (inst->model->numHeaders <= 0)
	{
		return 0;
	}
	if (inst->model->headers == NULL)
	{
		return 0;
	}

	struct ModelHeader *mh = inst->model->headers;

	if (animIndex >= (int)mh->numAnimations)
	{
		return 0;
	}
	if (mh->ptrAnimations == NULL)
	{
		return 0;
	}

	struct ModelAnim *anim = mh->ptrAnimations[animIndex];

	if (anim == NULL)
	{
		return 0;
	}

	return anim->numFrames & VEH_FRAME_NUM_FRAMES_MASK;
}

static void VehFrameProc_Driving_SpawnBurnSmoke(struct Driver *d)
{
	struct Particle *p = Particle_Init(0, sdata->gGT->iconGroup[VEH_FRAME_BURN_SMOKE_ICON_GROUP], &data.emSet_BurnSmoke[0]);

	if (p != NULL)
	{
		p->otIndexOffset = d->instSelf->depthBiasNormal;
		p->driverInst = d->instSelf;
		p->driverID = d->driverID;
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005b178-0x8005b510
void VehFrameProc_Driving(struct Thread *t, struct Driver *d)
{
	struct Instance *inst = t->inst;
	u8 desiredAnim = VEH_FRAME_ANIM_DRIVE;

	if ((d->instTntRecv == NULL) && (d->kartState != KS_WARP_PAD))
	{
		if (d->fireSpeed < 0)
		{
			desiredAnim = (d->speedApprox < 1) ? VEH_FRAME_ANIM_STATIONARY : VEH_FRAME_ANIM_DRIVE;
		}

		if (((d->jumpHeightCurr > VEH_FRAME_JUMP_AIR_THRESHOLD) || (inst->animIndex == VEH_FRAME_ANIM_AIRBORNE)) &&
		    (d->posCurr.y - d->quadBlockHeight > VEH_FRAME_AIR_GAP_THRESHOLD))
		{
			desiredAnim = VEH_FRAME_ANIM_AIRBORNE;
		}
	}

	int numFrames = VehFrameInst_GetNumAnimFrames(inst, inst->animIndex);
	if (numFrames <= 0)
	{
		return;
	}

	if (desiredAnim != inst->animIndex)
	{
		u8 currAnim = inst->animIndex;
		int startFrame;

		if (currAnim == VEH_FRAME_ANIM_MATRIX_FIRST)
		{
			startFrame = VehFrameInst_GetNumAnimFrames(inst, VEH_FRAME_ANIM_MATRIX_FIRST) - 1;
		}
		else
		{
			startFrame = VehFrameInst_GetStartFrame(currAnim, numFrames);
		}

		if (inst->animFrame == startFrame)
		{
			numFrames = VehFrameInst_GetNumAnimFrames(inst, desiredAnim);
			if (numFrames <= 0)
			{
				return;
			}

			inst->animIndex = desiredAnim;
			inst->animFrame = VehFrameInst_GetStartFrame(desiredAnim, numFrames);
			d->matrixArray = BAKED_GTE_MATRIX_NONE;
			d->matrixIndex = 0;
		}
		else
		{
			int speed = VEH_FRAME_TRANSITION_DEFAULT_SPEED;

			if (currAnim == VEH_FRAME_ANIM_DRIVE)
			{
				speed = VEH_FRAME_TRANSITION_DRIVE_SPEED;
			}
			else if (currAnim == VEH_FRAME_ANIM_MATRIX_FIRST)
			{
				speed = VEH_FRAME_TRANSITION_MATRIX_SPEED;
				d->matrixIndex = inst->animFrame;
			}

			inst->animFrame = VehCalc_InterpBySpeed(inst->animFrame, speed, startFrame);

			if ((u32)(inst->animIndex - VEH_FRAME_ANIM_MATRIX_FIRST) < VEH_FRAME_MATRIX_ANIM_COUNT)
			{
				d->matrixIndex = (u8)inst->animFrame;
				if (d->matrixIndex == 0)
				{
					d->matrixArray = BAKED_GTE_MATRIX_NONE;
					d->matrixIndex = 0;
				}
			}

			return;
		}
	}

	if (desiredAnim == VEH_FRAME_ANIM_DRIVE)
	{
		int targetFrame = numFrames >> 1;

		if (d->instTntRecv == NULL)
		{
			s16 burnTimer = d->burnTimer;

			if ((burnTimer != 0) && (burnTimer < VEH_FRAME_BURN_TIMER_LIMIT))
			{
				targetFrame +=
				    (((burnTimer >> VEH_FRAME_BURN_TIMER_SHIFT) % VEH_FRAME_BURN_FRAME_PERIOD) << VEH_FRAME_BURN_FRAME_SHIFT) - VEH_FRAME_BURN_FRAME_BIAS;
				inst->animFrame = targetFrame;
				VehFrameProc_Driving_SpawnBurnSmoke(d);
			}
			else
			{
				int turnMin = -VEH_FRAME_TURN_ACCEL_PREVENTION_LIMIT;
				int turnMax = VEH_FRAME_TURN_ACCEL_PREVENTION_LIMIT;
				int turnState = d->ampTurnState;

				if ((d->actionsFlagSet & ACTION_ACCEL_PREVENTION) == 0)
				{
					turnMax = (u8)d->const_TurnRate;
					turnMin = -turnMax;
					turnState = d->simpTurnState;
				}

				targetFrame = VehCalc_MapToRange(-turnState, turnMin, turnMax, 0, numFrames - 1);
			}
		}

		inst->animFrame = VehCalc_InterpBySpeed(inst->animFrame, VEH_FRAME_INTERP_SPEED_NORMAL, targetFrame);
		return;
	}

	if (desiredAnim == VEH_FRAME_ANIM_AIRBORNE)
	{
		s16 characterID;
		u8 matrixArray;

		inst->animFrame = VehCalc_InterpBySpeed(inst->animFrame, VEH_FRAME_INTERP_SPEED_NORMAL, numFrames - 1);

		if (d->kartState == KS_MASK_GRABBED)
		{
			return;
		}

		characterID = data.characterIDs[d->driverID];
		if (characterID == PENTA_PENGUIN)
		{
			characterID = COCO_BANDICOOT;
		}
		if (characterID == FAKE_CRASH)
		{
			characterID = CRASH_BANDICOOT;
		}

		matrixArray = characterID + VEH_FRAME_AIRBORNE_MATRIX_BASE;
		if (characterID == NITROS_OXIDE)
		{
			matrixArray = VEH_FRAME_OXIDE_MATRIX_ARRAY;
		}

		d->matrixArray = matrixArray;
		d->matrixIndex = (u8)inst->animFrame;
		return;
	}

	inst->animFrame = VehCalc_InterpBySpeed(inst->animFrame, VEH_FRAME_INTERP_SPEED_NORMAL, numFrames - 1);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005b510-0x8005b5fc.
void VehFrameProc_Spinning(struct Thread *t, struct Driver *d)
{
	struct Instance *inst = t->inst;
	int numFrames = VehFrameInst_GetNumAnimFrames(inst, inst->animIndex);
	int targetFrame;

	if (numFrames <= 0)
	{
		return;
	}

	if (inst->animIndex != VEH_FRAME_ANIM_DRIVE)
	{
		targetFrame = VehFrameInst_GetStartFrame(inst->animIndex, numFrames);

		if ((u32)(inst->animIndex - VEH_FRAME_ANIM_MATRIX_FIRST) < VEH_FRAME_MATRIX_ANIM_COUNT)
		{
			d->matrixArray = BAKED_GTE_MATRIX_NONE;
			d->matrixIndex = 0;
		}

		if (inst->animFrame == targetFrame)
		{
			numFrames = VehFrameInst_GetNumAnimFrames(inst, VEH_FRAME_ANIM_DRIVE);
			if (numFrames <= 0)
			{
				return;
			}

			inst->animIndex = VEH_FRAME_ANIM_DRIVE;
		}

		if (inst->animIndex != VEH_FRAME_ANIM_DRIVE)
		{
			inst->animFrame = VehCalc_InterpBySpeed(inst->animFrame, VEH_FRAME_SPIN_INTERP_SPEED, targetFrame);
			return;
		}
	}

	targetFrame = 0;
	if (d->KartStates.Spinning.spinDir >= 0)
	{
		targetFrame = numFrames - 1;
	}

	inst->animFrame = VehCalc_InterpBySpeed(inst->animFrame, VEH_FRAME_SPIN_INTERP_SPEED, targetFrame);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005b5fc-0x8005b6b8.
void VehFrameProc_LastSpin(struct Thread *t, struct Driver *d)
{
	struct Instance *inst = t->inst;

	if (inst->animIndex != VEH_FRAME_ANIM_DRIVE)
	{
		VehFrameProc_Spinning(t, d);
		return;
	}

	int numFrames = VehFrameInst_GetNumAnimFrames(inst, VEH_FRAME_ANIM_DRIVE);
	if (numFrames <= 0)
	{
		return;
	}

	int targetFrame = inst->animFrame;

	if (d->turnAngleCurr > 0)
	{
		if (d->turnAngleLerpVel < 0)
		{
			targetFrame = numFrames - 1;
		}
	}

	if ((d->turnAngleCurr < 0) && (d->turnAngleLerpVel > 0))
	{
		targetFrame = 0;
	}

	inst->animFrame = VehCalc_InterpBySpeed(inst->animFrame, VEH_FRAME_LAST_SPIN_INTERP_SPEED, targetFrame);
}
