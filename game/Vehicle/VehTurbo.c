#include <common.h>

enum
{
	TURBO_FIRE_SIZE_MIN = 4,
	TURBO_FIRE_SIZE_MAX = 8,
	TURBO_FIRE_MATRIX_SCALE_SHIFT = 3,
	TURBO_FIRE_LEFT_X_NUMERATOR = 9,
	TURBO_FIRE_LEFT_X_SHIFT = 0xb,
	TURBO_FIRE_RIGHT_X_NUMERATOR = -0x12,
	TURBO_FIRE_RIGHT_X_SHIFT = 0xc,
	TURBO_FIRE_Y_NUMERATOR = 3,
	TURBO_FIRE_Y_SHIFT = 8,
	TURBO_FIRE_Z_NUMERATOR = -0x34,
	TURBO_FIRE_Z_SHIFT = 0xc,
	TURBO_COOLDOWN_SIGN_SCALE = 0x10000,
	TURBO_ALPHA_RUMBLE_THRESHOLD = 2500,
	TURBO_RUMBLE_FRAMES = 4,
	TURBO_RUMBLE_FORCE = 4,
	TURBO_SECONDARY_MODEL_FRAME_OFFSET = 3,
	TURBO_ANIM_FRAME_COUNT = 8,
	TURBO_ANIM_FRAME_MASK = 7,
	TURBO_AUDIO_SLOT = 3,
	TURBO_AUDIO_VOLUME_BASE = 0x100,
	TURBO_AUDIO_ALPHA_SHIFT = 4,
	TURBO_AUDIO_VOLUME_MAX = 0x82,
	TURBO_AUDIO_DISTORT_STEP = 0x10,
	TURBO_AUDIO_DISTORT_MAX = 0x80,
	TURBO_AUDIO_DISTORT_INCREMENT_LIMIT = 0xc0,
	TURBO_AUDIO_SFX_ID = 0xe,
	TURBO_RESERVES_DISAPPEAR_THRESHOLD = 0x10,
	TURBO_ALPHA_FULL_MINUS_ONE = 0xfff,
	TURBO_FADE_FAST_STEP = 0x100,
	TURBO_FADE_SLOW_STEP = 0x40,
	TURBO_STOP_SFX_ID = -1,
};

CTR_STATIC_ASSERT(TURBO_FIRE_SIZE_MIN == 4);
CTR_STATIC_ASSERT(TURBO_FIRE_SIZE_MAX == 8);
CTR_STATIC_ASSERT(TURBO_FIRE_MATRIX_SCALE_SHIFT == 3);
CTR_STATIC_ASSERT(TURBO_FIRE_LEFT_X_NUMERATOR == 9);
CTR_STATIC_ASSERT(TURBO_FIRE_LEFT_X_SHIFT == 0xb);
CTR_STATIC_ASSERT(TURBO_FIRE_RIGHT_X_NUMERATOR == -0x12);
CTR_STATIC_ASSERT(TURBO_FIRE_RIGHT_X_SHIFT == 0xc);
CTR_STATIC_ASSERT(TURBO_FIRE_Y_NUMERATOR == 3);
CTR_STATIC_ASSERT(TURBO_FIRE_Y_SHIFT == 8);
CTR_STATIC_ASSERT(TURBO_FIRE_Z_NUMERATOR == -0x34);
CTR_STATIC_ASSERT(TURBO_FIRE_Z_SHIFT == 0xc);
CTR_STATIC_ASSERT(TURBO_COOLDOWN_SIGN_SCALE == 0x10000);
CTR_STATIC_ASSERT(TURBO_ALPHA_RUMBLE_THRESHOLD == 2500);
CTR_STATIC_ASSERT(TURBO_RUMBLE_FRAMES == 4);
CTR_STATIC_ASSERT(TURBO_RUMBLE_FORCE == 4);
CTR_STATIC_ASSERT(TURBO_SECONDARY_MODEL_FRAME_OFFSET == 3);
CTR_STATIC_ASSERT(TURBO_ANIM_FRAME_COUNT == 8);
CTR_STATIC_ASSERT(TURBO_ANIM_FRAME_MASK == 7);
CTR_STATIC_ASSERT(TURBO_AUDIO_SLOT == 3);
CTR_STATIC_ASSERT(TURBO_AUDIO_VOLUME_BASE == 0x100);
CTR_STATIC_ASSERT(TURBO_AUDIO_ALPHA_SHIFT == 4);
CTR_STATIC_ASSERT(TURBO_AUDIO_VOLUME_MAX == 0x82);
CTR_STATIC_ASSERT(TURBO_AUDIO_DISTORT_STEP == 0x10);
CTR_STATIC_ASSERT(TURBO_AUDIO_DISTORT_MAX == 0x80);
CTR_STATIC_ASSERT(TURBO_AUDIO_DISTORT_INCREMENT_LIMIT == 0xc0);
CTR_STATIC_ASSERT(TURBO_AUDIO_SFX_ID == 0xe);
CTR_STATIC_ASSERT(TURBO_RESERVES_DISAPPEAR_THRESHOLD == 0x10);
CTR_STATIC_ASSERT(TURBO_ALPHA_FULL_MINUS_ONE == 0xfff);
CTR_STATIC_ASSERT(TURBO_FADE_FAST_STEP == 0x100);
CTR_STATIC_ASSERT(TURBO_FADE_SLOW_STEP == 0x40);
CTR_STATIC_ASSERT(TURBO_STOP_SFX_ID == -1);

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80069284-0x80069370.
void VehTurbo_ProcessBucket(struct Thread *turboThread)
{
	while (turboThread != NULL)
	{
		struct Instance *primaryInst = turboThread->inst;
		struct Turbo *turbo = (struct Turbo *)turboThread->object;
		struct Instance *secondaryInst = turbo->inst;
		struct Instance *driverInst = turbo->driver->instSelf;

		struct InstDrawPerPlayer *primary = INST_GETIDPP(primaryInst);
		struct InstDrawPerPlayer *secondary = INST_GETIDPP(secondaryInst);
		struct InstDrawPerPlayer *driver = INST_GETIDPP(driverInst);

		for (int i = 0; i < sdata->gGT->numPlyrCurrGame; i++)
		{
			if ((driver->instFlags & PUSHBUFFER_EXISTS) == 0)
			{
				u32 driverDrawFlag = driver->instFlags | ~DRAW_SUCCESSFUL;

				secondary->instFlags &= driverDrawFlag;
				primary->instFlags &= driverDrawFlag;

				secondary->otRangeNormal = driver->otRangeNormal;
				primary->otRangeNormal = driver->otRangeNormal;
				secondary->otRangeSecondary = driver->otRangeSecondary;
				primary->otRangeSecondary = driver->otRangeSecondary;

				secondary->depthOffset[0] = driver->depthOffset[0];
				primary->depthOffset[0] = driver->depthOffset[0];
				secondary->depthOffset[1] = driver->depthOffset[1];
				primary->depthOffset[1] = driver->depthOffset[1];
			}

			primary++;
			secondary++;
			driver++;
		}

		turboThread = turboThread->siblingThread;
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80069370-0x800693c8.
void VehTurbo_ThDestroy(struct Thread *t)
{
	struct Turbo *turboObj = t->object;
	struct Driver *d = turboObj->driver;
	d->actionsFlagSet &= ~ACTION_TURBO_ITEM;

	INSTANCE_Death(turboObj->inst);
	INSTANCE_Death(t->inst);
}

static void VehTurbo_TransformOffset(struct Instance *driverInst, s16 x, s16 y, s16 z, s32 *out)
{
	SVECTOR offset = {x, y, z, 0};

	// NOTE(aalhendi): Native expression of retail VXY0/VZ0 loads before gte_rt.
	gte_SetRotMatrix(&driverInst->matrix.m[0][0]);
	gte_SetTransMatrix(&driverInst->matrix.m[0][0]);
	CTR_GteLoadSV0(&offset);
	gte_rt();
	CTR_GteStoreIR(out);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800693c8-0x80069bb0.
void VehTurbo_ThTick(struct Thread *turboThread)
{
	struct GameTracker *gGT = sdata->gGT;

	struct Turbo *turbo = (struct Turbo *)turboThread->object;
	struct Driver *driver = turbo->driver;
	struct Instance *instance = turboThread->inst;
	struct Instance *instanceDriver = driver->instSelf;

	if ((
	        // if not burnt
	        (driver->burnTimer == 0) &&

	        // if alpha of turbo is zero
	        (instance->alphaScale == 0)) &&


	    (instanceDriver->thread->modelIndex != DYNAMIC_GHOST))
	{
		// cut driverInst transparency in half
		instanceDriver->alphaScale = instanceDriver->alphaScale >> 1;
	}

	// if instance is not split by water
	if ((instanceDriver->flags & SPLIT_LINE) == 0)
	{
		// instance flags
		instance->flags &= ~SPLIT_LINE;
		turbo->inst->flags &= ~SPLIT_LINE;
	}

	// if instance is split by water
	else
	{
		// turbos are now split by water, set vertical split height
		instance->flags |= SPLIT_LINE;
		instance->vertSplit = instanceDriver->vertSplit;
		turbo->inst->flags |= SPLIT_LINE;
		turbo->inst->vertSplit = instanceDriver->vertSplit;
	}

	// if driver instance is not reflective
	if ((instanceDriver->flags & REFLECTIVE) == 0)
	{
		// remove reflection from turbo instances
		instance->flags &= ~REFLECTIVE;
		turbo->inst->flags &= ~REFLECTIVE;
	}

	// if driver instance is reflective
	else
	{
		// make turbo instances reflective
		// copy reflection height axis to instance
		instance->flags |= REFLECTIVE;
		instance->vertSplit = instanceDriver->vertSplit;
		turbo->inst->flags |= REFLECTIVE;
		turbo->inst->vertSplit = instanceDriver->vertSplit;
	}

	int fireSize = (int)turbo->fireSize;
	if (TURBO_FIRE_SIZE_MAX < (int)turbo->fireSize)
	{
		fireSize = TURBO_FIRE_SIZE_MAX;
	}
	if ((int)turbo->fireSize < TURBO_FIRE_SIZE_MIN)
	{
		fireSize = TURBO_FIRE_SIZE_MIN;
	}

	// matrix of first turbo instance
	instance->matrix.m[0][0] = (s16)(instanceDriver->matrix.m[0][0] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
	instance->matrix.m[0][1] = (s16)(instanceDriver->matrix.m[0][1] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
	instance->matrix.m[0][2] = (s16)(instanceDriver->matrix.m[0][2] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
	instance->matrix.m[1][0] = (s16)(instanceDriver->matrix.m[1][0] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
	instance->matrix.m[1][1] = (s16)(instanceDriver->matrix.m[1][1] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
	instance->matrix.m[1][2] = (s16)(instanceDriver->matrix.m[1][2] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
	instance->matrix.m[2][0] = (s16)(instanceDriver->matrix.m[2][0] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
	instance->matrix.m[2][1] = (s16)(instanceDriver->matrix.m[2][1] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
	instance->matrix.m[2][2] = (s16)(instanceDriver->matrix.m[2][2] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);

	VehTurbo_TransformOffset(instanceDriver, instanceDriver->scale.x * TURBO_FIRE_LEFT_X_NUMERATOR >> TURBO_FIRE_LEFT_X_SHIFT,
	                         instanceDriver->scale.y * TURBO_FIRE_Y_NUMERATOR >> TURBO_FIRE_Y_SHIFT,
	                         instanceDriver->scale.z * TURBO_FIRE_Z_NUMERATOR >> TURBO_FIRE_Z_SHIFT, &instance->matrix.t[0]);

	// matrix of second turbo instance, negate X axis
	turbo->inst->matrix.m[0][0] = (s16)(-(int)instanceDriver->matrix.m[0][0] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
	turbo->inst->matrix.m[0][1] = (s16)(instanceDriver->matrix.m[0][1] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
	turbo->inst->matrix.m[0][2] = (s16)(instanceDriver->matrix.m[0][2] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
	turbo->inst->matrix.m[1][0] = (s16)(-(int)instanceDriver->matrix.m[1][0] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
	turbo->inst->matrix.m[1][1] = (s16)(instanceDriver->matrix.m[1][1] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
	turbo->inst->matrix.m[1][2] = (s16)(instanceDriver->matrix.m[1][2] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
	turbo->inst->matrix.m[2][0] = (s16)(-(int)instanceDriver->matrix.m[2][0] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
	turbo->inst->matrix.m[2][1] = (s16)(instanceDriver->matrix.m[2][1] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
	turbo->inst->matrix.m[2][2] = (s16)(instanceDriver->matrix.m[2][2] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);

	VehTurbo_TransformOffset(instanceDriver, instanceDriver->scale.x * TURBO_FIRE_RIGHT_X_NUMERATOR >> TURBO_FIRE_RIGHT_X_SHIFT,
	                         instanceDriver->scale.y * TURBO_FIRE_Y_NUMERATOR >> TURBO_FIRE_Y_SHIFT,
	                         instanceDriver->scale.z * TURBO_FIRE_Z_NUMERATOR >> TURBO_FIRE_Z_SHIFT, &turbo->inst->matrix.t[0]);

	// decrease turbo visibility cooldown by elapsed milliseconds per frame, ~32
	s16 elapsedTime = turbo->fireVisibilityCooldown - gGT->elapsedTimeMS;
	turbo->fireVisibilityCooldown = elapsedTime;

	// don't allow negatives
	if (elapsedTime * TURBO_COOLDOWN_SIGN_SCALE < 0)
	{
		turbo->fireVisibilityCooldown = 0;
	}

	if (turbo->fireVisibilityCooldown == 0)
	{
		// make fire visible now that there's no cooldown
		instance->flags &= ~HIDE_MODEL;
		turbo->inst->flags &= ~HIDE_MODEL;
	}

	if (instance->alphaScale < TURBO_ALPHA_RUMBLE_THRESHOLD)
	{
		// gamepad vibration
		GAMEPAD_ShockFreq(driver, TURBO_RUMBLE_FRAMES, TURBO_RUMBLE_FORCE);
	}

	// set new model pointer, one of eight
	instance->model = gGT->modelPtr[(int)turbo->fireAnimIndex + STATIC_TURBO_EFFECT];

	// set new model pointer, one of eight

	// STATIC_TURBO_EFFECT
	// STATIC_TURBO_EFFECT1
	// STATIC_TURBO_EFFECT2
	// STATIC_TURBO_EFFECT3
	// STATIC_TURBO_EFFECT4
	// STATIC_TURBO_EFFECT5
	// STATIC_TURBO_EFFECT6
	// STATIC_TURBO_EFFECT7
	turbo->inst->model = gGT->modelPtr[(((int)turbo->fireAnimIndex + TURBO_SECONDARY_MODEL_FRAME_OFFSET) & TURBO_ANIM_FRAME_MASK) + STATIC_TURBO_EFFECT];

	turbo->fireAnimIndex++;

	// if eight or higher, back to zero
	if (turbo->fireAnimIndex >= TURBO_ANIM_FRAME_COUNT)
	{
		turbo->fireAnimIndex = 0;
	}

	if (turbo->fireDisappearCountdown > 0)
	{
		turbo->fireDisappearCountdown--;
	}

	// player of any kind
	if (instanceDriver->thread->modelIndex == DYNAMIC_PLAYER)
	{
		int fireSfxVolume = TURBO_AUDIO_VOLUME_BASE - (u32)(instance->alphaScale >> TURBO_AUDIO_ALPHA_SHIFT);

		if (fireSfxVolume < 0)
		{
			fireSfxVolume = 0;
		}
		else
		{
			if (TURBO_AUDIO_VOLUME_MAX < fireSfxVolume)
			{
				fireSfxVolume = TURBO_AUDIO_VOLUME_MAX;
			}
		}

		u32 fireAudioDistort = (u32)turbo->fireAudioDistort + TURBO_AUDIO_DISTORT_STEP;

		if ((int)fireAudioDistort < 0)
		{
			fireAudioDistort = 0;
		}
		else
		{
			if (fireAudioDistort > TURBO_AUDIO_DISTORT_MAX)
			{
				fireAudioDistort = TURBO_AUDIO_DISTORT_MAX;
			}
		}

		// if echo is required
		u32 echo = ((driver->actionsFlagSet & ACTION_ENGINE_ECHO) != 0);

		// driver audio
		OtherFX_RecycleNew(&driver->driverAudioPtrs[TURBO_AUDIO_SLOT], TURBO_AUDIO_SFX_ID,
		                   HowlSfx_Pack(HOWL_SFX_LR_CENTER, fireAudioDistort, fireSfxVolume, echo));

		// manipulate turbo audio distort to change sound each frame
		if (turbo->fireAudioDistort < TURBO_AUDIO_DISTORT_INCREMENT_LIMIT)
		{
			turbo->fireAudioDistort++;
		}
	}

	u8 kartState = driver->kartState;

	if (
	    // if this is a ghost
	    (instanceDriver->thread->modelIndex == DYNAMIC_GHOST) ||

	    ((kartState != KS_MASK_GRABBED) && (kartState != KS_CRASHING)

// lol they found a glitch with this
#if BUILD > SepReview
	     && (kartState != KS_WARP_PAD)
#endif
	         ))
	{
		// if reserves are nearing zero
		if ((driver->reserves < TURBO_RESERVES_DISAPPEAR_THRESHOLD) || (turbo->fireDisappearCountdown == 0))
		{
			// if fully transparent, skip lines
			if (TURBO_ALPHA_FULL_MINUS_ONE < instance->alphaScale)
			{
				goto LAB_80069b50;
			}

			if (turbo->fireDisappearCountdown == 0)
			{
				// increase transparency
				instance->alphaScale += TURBO_FADE_FAST_STEP;
				turbo->inst->alphaScale += TURBO_FADE_FAST_STEP;
			}
			else
			{
				// increase transparency
				instance->alphaScale += TURBO_FADE_SLOW_STEP;
				turbo->inst->alphaScale += TURBO_FADE_SLOW_STEP;
			}
		}
		else
		{
			// if scale is big, skip lines
			if (TURBO_ALPHA_FULL_MINUS_ONE < instance->alphaScale)
			{
				goto LAB_80069b50;
			}
		}
	}

	// if not a ghost, and
	// kart state is mask grab, crashed, or warped
	else
	{
		// restore backup of alpha
		instanceDriver->alphaScale = driver->alphaScaleBackup;
	LAB_80069b50:

		// player of any kind
		if (instanceDriver->thread->modelIndex == DYNAMIC_PLAYER)
		{
			// volume, distortion, left/right
			u32 stopSfxParams = HOWL_SFX_CENTER_NO_DISTORTION;

			// if echo is required
			if ((driver->actionsFlagSet & ACTION_ENGINE_ECHO) != 0)
			{
				// add echo, volume, distortion, left/right
				stopSfxParams = HOWL_SFX_CENTER_NO_DISTORTION | HOWL_SFX_ECHO_FLAG;
			}

			// driver audio
			OtherFX_RecycleNew(&driver->driverAudioPtrs[TURBO_AUDIO_SLOT], TURBO_STOP_SFX_ID, stopSfxParams);
		}

		// 0x800 = this thread needs to be deleted
		turboThread->flags |= THREAD_FLAG_DEAD;
	}

	// do not use infinite loop optimization,
	// modern GCC "without" the $RA skip is more
	// optimized than PSYQ "with" the $RA skip
}
