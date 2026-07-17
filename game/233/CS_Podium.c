#include <common.h>

enum PodiumDanceModelOffset
{
	PODIUM_DANCE_CRASH = STATIC_CRASHDANCE - STATIC_CRASHDANCE,
	PODIUM_DANCE_CORTEX = STATIC_CORTEXDANCE - STATIC_CRASHDANCE,
	PODIUM_DANCE_COCO = STATIC_COCODANCE - STATIC_CRASHDANCE,
	PODIUM_DANCE_NGIN = STATIC_NGINDANCE - STATIC_CRASHDANCE,
	PODIUM_DANCE_POLAR = STATIC_POLARDANCE - STATIC_CRASHDANCE,
	PODIUM_DANCE_PURA = STATIC_PURADANCE - STATIC_CRASHDANCE,
	PODIUM_DANCE_PINSTRIPE = STATIC_PINSTRIPEDANCE - STATIC_CRASHDANCE,
	PODIUM_DANCE_PAPU = STATIC_PAPUDANCE - STATIC_CRASHDANCE,
	PODIUM_DANCE_ROO = STATIC_ROODANCE - STATIC_CRASHDANCE,
	PODIUM_DANCE_JOE = STATIC_JOEDANCE - STATIC_CRASHDANCE,
	PODIUM_DANCE_NTROPY = STATIC_NTROPYDANCE - STATIC_CRASHDANCE,
	PODIUM_DANCE_PENTA = STATIC_PENDANCE - STATIC_CRASHDANCE,
	PODIUM_DANCE_FAKE_CRASH = STATIC_FAKEDANCE - STATIC_CRASHDANCE,
};

enum PodiumPrizeConstants
{
	PODIUM_CAMERA_DISTANCE_TO_SCREEN = 0x100,
	PODIUM_VOLUME_BACKUP_MASK = 0xff,
	PODIUM_CONFETTI_PARTICLE_COUNT = 200,
	PODIUM_SCENE_SPAWN_Y_OFFSET = 0x80,
	PODIUM_THIRD_POS_X = 299,
	PODIUM_THIRD_POS_Y = -85,
	PODIUM_THIRD_POS_Z = 0,
	PODIUM_THIRD_YAW_OFFSET = 0x600,
	PODIUM_SECOND_POS_X = -299,
	PODIUM_SECOND_POS_Y = -42,
	PODIUM_SECOND_POS_Z = 0,
	PODIUM_SECOND_YAW_OFFSET = 0x200,
	PODIUM_FIRST_POS_X = 0,
	PODIUM_FIRST_POS_Y = 0,
	PODIUM_FIRST_POS_Z = 0,
	PODIUM_TAWNA_POS_X = 0x1a8,
	PODIUM_TAWNA_POS_Y = -128,
	PODIUM_TAWNA_POS_Z = 0x140,
	PODIUM_TAWNA_YAW_OFFSET = -0x2aa,
	PODIUM_VICTORY_CAMERA_THREAD_FLAGS = SIZE_RELATIVE_POOL_BUCKET(sizeof(struct CsPodiumCameraThreadObj), NONE, SMALL, CAMERA),
	PODIUM_PRIZE_HUD_CENTER_X = 0x100,
	PODIUM_PRIZE_HUD_CENTER_Y = 0x6c,
	PODIUM_PRIZE_HUD_Y_OFFSET = 0x3c,
	PODIUM_PRIZE_FLY_TO_HUD_FRAMES = 0xf,
	PODIUM_PRIZE_DEFAULT_TARGET_Z = -0x200,
	PODIUM_PRIZE_TROPHY_TARGET_Z = -200,
	PODIUM_PRIZE_ROT_Y_STEP = 100,
	PODIUM_PRIZE_SPEC_LIGHT_PHASE_STEP = 0x3f,
	PODIUM_PRIZE_SPEC_LIGHT_RATIO_SHIFT = 11,
	PODIUM_PRIZE_SCALE_DOWN_STEP = 0x4b0,
	PODIUM_PRIZE_SCALE_CLAMP_THRESHOLD = FP_ONE + 1,
	PODIUM_PRIZE_BOUNCE_FRAME_COUNT = 5,
	PODIUM_PRIZE_BOUNCE_SCALE_UP_BASE = 800,
	PODIUM_PRIZE_BOUNCE_SCALE_UP_STEP = 400,
	PODIUM_PRIZE_BOUNCE_SCALE_DOWN_STEP = 800,
	PODIUM_PRIZE_BOUNCE_LIMIT_BASE = 0x2000,
	PODIUM_PRIZE_BOUNCE_LIMIT_STEP = 0x28a,
	PODIUM_PRIZE_HEIGHT_INTERP_STEP = 0x14,
	PODIUM_PRIZE_ORBIT_INTERP_STEP = 1,
	PODIUM_PRIZE_UI_DEPTH_BIAS = 0x80,
	PODIUM_PRIZE_INITIAL_SCALE = FP_ONE * 2,
	PODIUM_PRIZE_TROPHY_INITIAL_SCALE = FP_ONE * 4,
	PODIUM_PRIZE_ORBIT_RADIUS = 0x40,
	PODIUM_PRIZE_HEIGHT_OFFSET = 0x200,
	PODIUM_PRIZE_PODIUM_Y_OFFSET = 0x1c0,
	PODIUM_REWARD_UNLOCK_SFX = 0x67,
	PODIUM_PRIZE_STOP_DINGOFIRE_FX = 0xaf,
	PODIUM_PRIZE_STOP_AKUMOUTH_FX = 0xae,
	PODIUM_PRIZE_FLY_TO_HUD_SFX = 0x9a,
	PODIUM_GEM_SPEC_LIGHT_VERTICAL_START = 0x5d3,
	PODIUM_GEM_SPEC_LIGHT_HORIZONTAL_START = 0x718,
	PODIUM_GEM_SPEC_LIGHT_VERTICAL_END = 0x590,
	PODIUM_GEM_SPEC_LIGHT_HORIZONTAL_END = 0x609,
	PODIUM_RELIC_SPEC_LIGHT_VERTICAL_START = 0x2ab,
	PODIUM_RELIC_SPEC_LIGHT_HORIZONTAL_START = 0x436,
	PODIUM_RELIC_SPEC_LIGHT_VERTICAL_END = 0x1eb,
	PODIUM_RELIC_SPEC_LIGHT_HORIZONTAL_END = 0x670,
	PODIUM_KEY_SPEC_LIGHT_VERTICAL_START = 0x1d9,
	PODIUM_KEY_SPEC_LIGHT_HORIZONTAL_START = 0x5db,
	PODIUM_KEY_SPEC_LIGHT_VERTICAL_END = 0x2da,
	PODIUM_KEY_SPEC_LIGHT_HORIZONTAL_END = 0x54b,
	PODIUM_MUSIC_POLAR = 7,
	PODIUM_MUSIC_CORTEX = 8,
	PODIUM_MUSIC_ROO = 9,
	PODIUM_MUSIC_CRASH = 10,
	PODIUM_MUSIC_PINSTRIPE = 0xb,
	PODIUM_MUSIC_TINY = 0xc,
};

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ac714-0x800ac840
void CS_DestroyPodium_StartDriving(void)
{
	struct Instance *inst;
	struct Driver *d;
	struct GameTracker *gGT = sdata->gGT;
	struct Thread *t = gGT->threadBuckets[OTHER].thread;

	// enable HUD
	gGT->hudFlags |= HUD_FLAG_RACE_HUD;

	// loop through all threads
	while (t != NULL)
	{
		if (t->funcThDestroy != CS_Podium_Prize_ThDestroy)
		{
			t->flags |= THREAD_FLAG_DEAD;
		}

		t = t->siblingThread;
	}

	d = gGT->drivers[0];

	// enable collisions for thread,
	// and make instance visible
	inst = d->instSelf;
	inst->thread->flags &= ~THREAD_FLAG_DISABLE_COLLISION;
	inst->flags &= ~(HIDE_MODEL);

	d->kartState = KS_ENGINE_REVVING;
	d->funcPtrs[DRIVER_FUNC_INIT] = VehPhysProc_Driving_Init;

	// if cutscene changed audio, restore backup
	if (D233.CutsceneManipulatesAudio != 0)
	{
		// restore backup of volume variables
		howl_VolumeSet(HOWL_VOLUME_TYPE_FX, D233.FXVolumeBackup);
		howl_VolumeSet(HOWL_VOLUME_TYPE_MUSIC, D233.MusicVolumeBackup);
		howl_VolumeSet(HOWL_VOLUME_TYPE_VOICE, D233.VoiceVolumeBackup);
	}

	// cam mode be zero to follow you
	gGT->cameraDC[0].cameraMode = 0;
	gGT->pushBuffer[0].distanceToScreen_PREV = PODIUM_CAMERA_DISTANCE_TO_SCREEN;
	gGT->pushBuffer[0].distanceToScreen_CURR = PODIUM_CAMERA_DISTANCE_TO_SCREEN;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b021c-0x800b0248
void CS_Podium_Stand_ThTick(struct Thread *t)
{
	if (D233.isCutsceneOver != 0)
	{
		t->flags |= THREAD_FLAG_DEAD;
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b0248-0x800b0300
void CS_Podium_Stand_Init(struct CsThreadInitData *podiumData)
{
	struct Instance *inst = INSTANCE_BirthWithThread(STATIC_PODIUM, R233.s_podium, SMALL, OTHER, CS_Podium_Stand_ThTick, 0, 0);

	// if the instance was built
	if (inst == NULL)
	{
		return;
	}

	// set funcThDestroy to remove instance from instance pool
	inst->thread->funcThDestroy = PROC_DestroyInstance;

	inst->matrix.t[0] = podiumData->podiumPos.x;
	inst->matrix.t[1] = podiumData->podiumPos.y;
	inst->matrix.t[2] = podiumData->podiumPos.z;

	inst->depthBiasSecondary += 2;
	inst->depthBiasNormal += 2;

	podiumData->derivedRot.x = podiumData->rot.x;
	podiumData->derivedRot.y = podiumData->rot.y;
	podiumData->derivedRot.z = podiumData->rot.z;

	ConvertRotToMatrix(&inst->matrix, &podiumData->derivedRot.vec);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800af7c0-0x800af994
void CS_Podium_Prize_Spin(struct Instance *inst, struct Prize *prize)
{
	struct GamepadSystem *gGS;
	u32 trigApprox;
	s16 prevAngle;
	int ratio;
	u32 angle;
	SVec3 lightDir;

	prize->rot.y += PODIUM_PRIZE_ROT_Y_STEP;
	const SVec3 *prizeRot = &prize->rot;
	ConvertRotToMatrix(&inst->matrix, prizeRot);

	gGS = sdata->gGamepads;

	if ((inst->flags & USE_SPECULAR_LIGHT) == 0)
	{
		return;
	}

	prevAngle = prize->specLightPhase;
	prize->specLightPhase = prevAngle + PODIUM_PRIZE_SPEC_LIGHT_PHASE_STEP;

	if ((gGS->gamepad[1].buttonsHeldCurrFrame & BTN_L1) != 0)
	{
		prize->specLightPhase = prevAngle;
	}

	ratio = (prize->specLightPhase & (ANG_TWO_PI - 1)) - ANG_PI;
	if (ratio < 0)
	{
		ratio = -ratio;
	}

	angle = prize->specLightVerticalStart + (((prize->specLightVerticalEnd - prize->specLightVerticalStart) * ratio) >> PODIUM_PRIZE_SPEC_LIGHT_RATIO_SHIFT);

	{
		s16 sine1;
		s16 cos1;

		trigApprox = CTR_ReadU32LE(&data.trigApprox[ANG_MODULO_HALF_PI(angle)]);
		if (IS_ANG_FIRST_OR_THIRD_QUADRANT(angle))
		{
			cos1 = (s16)(trigApprox >> 16);
			sine1 = (s16)trigApprox;
		}
		else
		{
			cos1 = -(s16)trigApprox;
			sine1 = (s16)(trigApprox >> 16);
		}
		if (IS_ANG_THIRD_OR_FOURTH_QUADRANT(angle))
		{
			cos1 = -cos1;
			sine1 = -sine1;
		}
		lightDir.y = cos1;

		ratio = (prize->specLightPhase & (ANG_TWO_PI - 1)) - ANG_PI;
		if (ratio < 0)
		{
			ratio = -ratio;
		}

		angle = prize->specLightHorizontalStart +
		        (((prize->specLightHorizontalEnd - prize->specLightHorizontalStart) * ratio) >> PODIUM_PRIZE_SPEC_LIGHT_RATIO_SHIFT);

		s16 sine2;
		s16 cos2;

		trigApprox = CTR_ReadU32LE(&data.trigApprox[ANG_MODULO_HALF_PI(angle)]);
		if (IS_ANG_FIRST_OR_THIRD_QUADRANT(angle))
		{
			cos2 = (s16)(trigApprox >> 16);
			sine2 = (s16)trigApprox;
		}
		else
		{
			cos2 = -(s16)trigApprox;
			sine2 = (s16)(trigApprox >> 16);
		}
		if (IS_ANG_THIRD_OR_FOURTH_QUADRANT(angle))
		{
			cos2 = -cos2;
			sine2 = -sine2;
		}
		lightDir.x = (sine1 * cos2) >> FRACTIONAL_BITS;
		lightDir.z = (sine1 * sine2) >> FRACTIONAL_BITS;
	}

	Vector_SpecLightSpin3D(inst, prizeRot, &lightDir);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800af994-0x800afbc8
void CS_Podium_Prize_ThTick3(struct Thread *th)
{
	struct GameTracker *gGT;
	struct Instance *inst = th->inst;
	struct Prize *prize = th->object;
	s16 framesLeft;

	framesLeft = prize->flyToHudFramesLeft - 1;
	prize->flyToHudFramesLeft = framesLeft;

	if (framesLeft != 0)
	{
		int frameMax = prize->flyToHudFramesTotal;
		int xInterp = framesLeft * (PODIUM_PRIZE_HUD_CENTER_X - prize->targetScreenPos.x);
		int yInterp = framesLeft * (PODIUM_PRIZE_HUD_CENTER_Y - prize->targetScreenPos.y);
		int x;
		int y;
		s16 scale;

		x = (prize->targetScreenPos.x + xInterp / frameMax - PODIUM_PRIZE_HUD_CENTER_X) * -inst->matrix.t[2];
		if (x < 0)
		{
			x += FP8_ONE - 1;
		}

		inst->matrix.t[0] = x >> FRACTIONAL_BITS_8;

		y = (prize->targetScreenPos.y + yInterp / frameMax - PODIUM_PRIZE_HUD_CENTER_Y) * inst->matrix.t[2];
		if (y < 0)
		{
			y += FP8_ONE - 1;
		}

		inst->matrix.t[1] = y >> FRACTIONAL_BITS_8;

		scale = inst->scale.x - PODIUM_PRIZE_SCALE_DOWN_STEP;
		if (scale < PODIUM_PRIZE_SCALE_CLAMP_THRESHOLD)
		{
			scale = FP_ONE;
		}

		inst->scale.x = scale;
		inst->scale.y = scale;
		inst->scale.z = scale;

		CS_Podium_Prize_Spin(inst, prize);
		return;
	}

	if (!CS_Camera_BoolGotoBoss())
	{
		u32 rewards = sdata->advProgress.hintFlags;
		s16 hintID = 0;

		if ((rewards & ADV_REWARD_HINT_MAP_INFORMATION_MASK) == 0)
		{
			hintID = ADV_MASK_HINT_ID_MAP_INFORMATION;
		}
		else if ((rewards & ADV_REWARD_HINT_WUMPA_FRUIT_MASK) == 0)
		{
			hintID = ADV_MASK_HINT_ID_WUMPA_FRUIT;
		}
		else if ((rewards & ADV_REWARD_HINT_TNT_MASK) == 0)
		{
			hintID = ADV_MASK_HINT_ID_TNT;
		}
		else if ((rewards & ADV_REWARD_HINT_HANG_TIME_TURBO_MASK) == 0)
		{
			hintID = ADV_MASK_HINT_ID_HANG_TIME_TURBO;
		}
		else if ((rewards & ADV_REWARD_HINT_POWER_SLIDE_MASK) == 0)
		{
			hintID = ADV_MASK_HINT_ID_POWER_SLIDE;
		}
		else if ((rewards & ADV_REWARD_HINT_TURBO_BOOST_MASK) == 0)
		{
			hintID = ADV_MASK_HINT_ID_TURBO_BOOST;
		}
		else if ((rewards & ADV_REWARD_HINT_BRAKE_SLIDE_MASK) == 0)
		{
			hintID = ADV_MASK_HINT_ID_BRAKE_SLIDE;
		}

		if (hintID != 0)
		{
			MainFrame_RequestMaskHint(hintID, 0);
		}
	}

	gGT = sdata->gGT;
	gGT->overlayTransition = 2;
	gGT->gameMode2 &= ~VEH_FREEZE_PODIUM;

	OtherFX_Play(PODIUM_REWARD_UNLOCK_SFX, 1);

	th->flags |= THREAD_FLAG_DEAD;
}

// Make the trophy bounce 3 times
// Then start ThTick3
// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800afbc8-0x800afcc4
void CS_Podium_Prize_ThTick2(struct Thread *th)
{
	int currScale;

	struct Prize *prize = th->object;

	// get instance from thread
	struct Instance *inst = th->inst;

	s16 frameIndex = prize->bounceFrameIndex;

	// bouncing scale animation
	if (frameIndex < PODIUM_PRIZE_BOUNCE_FRAME_COUNT)
	{
		// if even frame
		if ((frameIndex & 1) == 0)
		{
			// scaleX
			currScale = inst->scale.x + PODIUM_PRIZE_BOUNCE_SCALE_UP_BASE + frameIndex * PODIUM_PRIZE_BOUNCE_SCALE_UP_STEP;

			if ((frameIndex + 1) * PODIUM_PRIZE_BOUNCE_LIMIT_STEP + PODIUM_PRIZE_BOUNCE_LIMIT_BASE < currScale)
			{
				// frame counter
				frameIndex += 1;
			}
		}
		else
		{
			// scaleX
			currScale = inst->scale.x - PODIUM_PRIZE_BOUNCE_SCALE_DOWN_STEP;

			if (currScale < PODIUM_PRIZE_SCALE_CLAMP_THRESHOLD)
			{
				// frame counter
				frameIndex += 1;
			}
		}

		prize->bounceFrameIndex = frameIndex;

		// scaleY and scaleZ
		inst->scale.x = currScale;
		inst->scale.y = currScale;
		inst->scale.z = currScale;

		CS_Podium_Prize_Spin(inst, prize);
	}
	else
	{
		ThTick_SetAndExec(th, CS_Podium_Prize_ThTick3);
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800afcc4-0x800afe58
void CS_Podium_Prize_ThTick1(struct Thread *th)
{
	struct Instance *inst = th->inst;
	struct Prize *prize = th->object;
	int trig;

	if (D233.podiumPrizeDropReady != 0)
	{
		if (th->modelIndex != STATIC_BIG1)
		{
			inst->flags &= ~HIDE_MODEL;
		}

		prize->heightOffset = VehCalc_InterpBySpeed(prize->heightOffset, PODIUM_PRIZE_HEIGHT_INTERP_STEP, 0);
		prize->orbitRadius = VehCalc_InterpBySpeed(prize->orbitRadius, PODIUM_PRIZE_ORBIT_INTERP_STEP, 0);
	}

	trig = MATH_Sin(prize->rot.y);
	inst->matrix.t[0] = prize->posStart.x + ((prize->orbitRadius * trig) >> FRACTIONAL_BITS);
	inst->matrix.t[1] = prize->posStart.y + prize->heightOffset;

	trig = MATH_Cos(prize->rot.y);
	inst->matrix.t[2] = prize->posStart.z + ((prize->orbitRadius * trig) >> FRACTIONAL_BITS);

	if (D233.isCutsceneOver == 0)
	{
		CS_Podium_Prize_Spin(inst, prize);
		return;
	}

	prize->flyToHudFramesTotal = PODIUM_PRIZE_FLY_TO_HUD_FRAMES;
	prize->flyToHudFramesLeft = PODIUM_PRIZE_FLY_TO_HUD_FRAMES;
	prize->bounceFrameIndex = 0;

	inst->depthBiasNormal = PODIUM_PRIZE_UI_DEPTH_BIAS;
	inst->depthBiasSecondary = PODIUM_PRIZE_UI_DEPTH_BIAS;

	inst->scale.x = FP_ONE;
	inst->scale.y = FP_ONE;
	inst->scale.z = FP_ONE;

	inst->matrix.t[0] = 0;
	inst->matrix.t[1] = 0;
	inst->matrix.t[2] = prize->targetScreenPos.z;

	{
		struct InstDrawPerPlayer *idpp = INST_GETIDPP(inst);
		idpp[0].pushBuffer = &sdata->gGT->pushBuffer_UI;
	}

	OtherFX_Stop2(PODIUM_PRIZE_STOP_DINGOFIRE_FX);
	OtherFX_Stop2(PODIUM_PRIZE_STOP_AKUMOUTH_FX);
	OtherFX_Play(PODIUM_PRIZE_FLY_TO_HUD_SFX, 1);

	ThTick_SetAndExec(th, CS_Podium_Prize_ThTick2);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800afe58-0x800afe90
void CS_Podium_Prize_ThDestroy(struct Thread *t)
{
	// remove bits
	sdata->gGT->gameMode2 &= ~(INC_RELIC | INC_KEY | INC_TROPHY);
	PROC_DestroyInstance(t);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800afe90-0x800b021c
void CS_Podium_Prize_Init(u32 prizeModel, const char *prizeName, const SVec3Slot *podiumPos)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Instance *inst;
	struct Prize *prize;
	s32 transformedOffset[3];

	inst = INSTANCE_BirthWithThread(prizeModel, prizeName, MEDIUM, OTHER, CS_Podium_Prize_ThTick1, sizeof(struct Prize), NULL);

	if (inst == NULL)
	{
		if (D233.cutsceneState < CS_WAIT_INPUT)
		{
			D233.cutsceneState = CS_WAIT_INPUT;
		}

		gGT->gameMode2 &= ~VEH_FREEZE_PODIUM;
		return;
	}

	inst->scale.x = PODIUM_PRIZE_INITIAL_SCALE;
	inst->scale.y = PODIUM_PRIZE_INITIAL_SCALE;
	inst->scale.z = PODIUM_PRIZE_INITIAL_SCALE;
	inst->flags |= HIDE_MODEL;

	prize = inst->thread->object;
	inst->thread->funcThDestroy = CS_Podium_Prize_ThDestroy;

	prize->orbitRadius = PODIUM_PRIZE_ORBIT_RADIUS;
	prize->heightOffset = PODIUM_PRIZE_HEIGHT_OFFSET;
	prize->rot.x = 0;
	prize->rot.y = 0;
	prize->rot.z = 0;

	MTC2(0, 0);
	MTC2(PODIUM_PRIZE_ORBIT_RADIUS, 1);
	gte_llv0();

	CTR_GteStoreMAC(transformedOffset);

	prize->posStart.x = podiumPos->x + (s16)transformedOffset[0];
	prize->posStart.y = podiumPos->y + (s16)transformedOffset[1] + PODIUM_PRIZE_PODIUM_Y_OFFSET;
	prize->posStart.z = podiumPos->z + (s16)transformedOffset[2];
	prize->targetScreenPos.z = PODIUM_PRIZE_DEFAULT_TARGET_Z;

	switch (prizeModel)
	{
	case STATIC_BIG1:
		inst->flags |= HIDE_MODEL;
		goto center_target;

	case STATIC_GEM:
	{
		s16 *gemColor = data.AdvCups[gGT->cup.cupID].color;

		inst->colorRGBA = (gemColor[0] << 20) | (gemColor[1] << 12) | (gemColor[2] << 4);
		prize->specLightVerticalStart = PODIUM_GEM_SPEC_LIGHT_VERTICAL_START;
		prize->specLightHorizontalStart = PODIUM_GEM_SPEC_LIGHT_HORIZONTAL_START;
		prize->specLightVerticalEnd = PODIUM_GEM_SPEC_LIGHT_VERTICAL_END;
		prize->specLightHorizontalEnd = PODIUM_GEM_SPEC_LIGHT_HORIZONTAL_END;
		inst->flags |= USE_SPECULAR_LIGHT;
		goto center_target;
	}

	default:
	center_target:
		prize->targetScreenPos.x = PODIUM_PRIZE_HUD_CENTER_X;
		prize->targetScreenPos.y = PODIUM_PRIZE_HUD_CENTER_Y;
		return;

	case STATIC_RELIC:
	{
		struct UiElement2D *hud = data.hudStructPtr[0];
		u32 bitIndex = gGT->prevLEV + ADV_REWARD_FIRST_PLATINUM_RELIC;
		u32 relicColor;

		prize->targetScreenPos.x = hud[UI_HUD_SLOT_RELIC].x;
		prize->targetScreenPos.y = hud[UI_HUD_SLOT_RELIC].y - PODIUM_PRIZE_HUD_Y_OFFSET;

		if (!CHECK_ADV_BIT(sdata->advProgress.rewards, bitIndex))
		{
			bitIndex = gGT->prevLEV + ADV_REWARD_FIRST_GOLD_RELIC;

			if (!CHECK_ADV_BIT(sdata->advProgress.rewards, bitIndex))
			{
				relicColor = INST_COLOR_SAPPHIRE_RELIC;
			}
			else
			{
				relicColor = INST_COLOR_GOLD_RELIC;
			}
		}
		else
		{
			relicColor = INST_COLOR_PLATINUM_RELIC;
		}

		inst->colorRGBA = relicColor;
		prize->specLightVerticalStart = PODIUM_RELIC_SPEC_LIGHT_VERTICAL_START;
		prize->specLightHorizontalStart = PODIUM_RELIC_SPEC_LIGHT_HORIZONTAL_START;
		prize->specLightVerticalEnd = PODIUM_RELIC_SPEC_LIGHT_VERTICAL_END;
		prize->specLightHorizontalEnd = PODIUM_RELIC_SPEC_LIGHT_HORIZONTAL_END;
		inst->flags |= USE_SPECULAR_LIGHT;

		gGT->gameMode2 |= INC_RELIC;
		return;
	}

	case STATIC_TROPHY:
	{
		struct UiElement2D *hud = data.hudStructPtr[0];

		prize->targetScreenPos.x = hud[UI_HUD_SLOT_TROPHY].x;
		prize->targetScreenPos.y = hud[UI_HUD_SLOT_TROPHY].y - PODIUM_PRIZE_HUD_Y_OFFSET;
		prize->targetScreenPos.z = PODIUM_PRIZE_TROPHY_TARGET_Z;

		inst->scale.x = PODIUM_PRIZE_TROPHY_INITIAL_SCALE;
		inst->scale.y = PODIUM_PRIZE_TROPHY_INITIAL_SCALE;
		inst->scale.z = PODIUM_PRIZE_TROPHY_INITIAL_SCALE;

		gGT->gameMode2 |= INC_TROPHY;
		return;
	}

	case STATIC_KEY:
	{
		struct UiElement2D *hud = data.hudStructPtr[0];

		inst->colorRGBA = INST_COLOR_KEY;
		prize->specLightVerticalStart = PODIUM_KEY_SPEC_LIGHT_VERTICAL_START;
		prize->specLightHorizontalStart = PODIUM_KEY_SPEC_LIGHT_HORIZONTAL_START;
		prize->specLightVerticalEnd = PODIUM_KEY_SPEC_LIGHT_VERTICAL_END;
		prize->specLightHorizontalEnd = PODIUM_KEY_SPEC_LIGHT_HORIZONTAL_END;
		inst->flags |= USE_SPECULAR_LIGHT;

		prize->targetScreenPos.x = hud[UI_HUD_SLOT_KEY].x;
		prize->targetScreenPos.y = hud[UI_HUD_SLOT_KEY].y - PODIUM_PRIZE_HUD_Y_OFFSET;

		gGT->gameMode2 |= INC_KEY;
		return;
	}
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b0300-0x800b06ac
void CS_Podium_FullScene_Init(void)
{
	struct Instance *driverInstSelf;
	struct Thread *victoryCamThread;
	u32 podiumMusic;
	struct CsThreadInitData InitData = {0};
	MATRIX podiumMatrix;

	struct SpawnPosRot *posRot;

	struct GameTracker *gGT = sdata->gGT;

	// assume cutscene did not manipulate audio
	D233.CutsceneManipulatesAudio = 0;

	// Make a backup of FX volume, masked to a byte
	D233.FXVolumeBackup = howl_VolumeGet(HOWL_VOLUME_TYPE_FX);
	D233.FXVolumeBackup &= PODIUM_VOLUME_BACKUP_MASK;

	// Make a backup of Music volume, masked to a byte
	D233.MusicVolumeBackup = howl_VolumeGet(HOWL_VOLUME_TYPE_MUSIC);
	D233.MusicVolumeBackup &= PODIUM_VOLUME_BACKUP_MASK;

	// Make a backup of Voice volume, masked to a byte
	D233.VoiceVolumeBackup = howl_VolumeGet(HOWL_VOLUME_TYPE_VOICE);
	D233.VoiceVolumeBackup &= PODIUM_VOLUME_BACKUP_MASK;

	// Cutscene is now starting
	D233.isCutsceneOver = 0;
	D233.cutsceneState = CS_CAMERA_PAN;

	D233.podiumPrizeDropReady = 0;

	driverInstSelf = gGT->drivers[0]->instSelf;

	D233.podiumCameraFrame = 0;

	driverInstSelf->flags |= HIDE_MODEL;

	VehPhysProc_FreezeEndEvent_Init(driverInstSelf->thread, gGT->drivers[0]);

	// Number of Winners = 1
	// this means Draw Confetti on one window
	gGT->numWinners = 1;

	// Set winnerIndex[0] to 0, to draw
	// confetti on the first pushBuffer
	gGT->winnerIndex[0] = 0;

	gGT->confetti.numParticles_max = PODIUM_CONFETTI_PARTICLE_COUNT;
	gGT->confetti.vanishRate = PODIUM_CONFETTI_PARTICLE_COUNT;
	gGT->hudFlags &= HUD_FLAG_CLEAR_RACE_HUD_MASK;

	// Draw Confetti
	gGT->renderFlags |= RENDER_FLAG_CONFETTI;

	gGT->gameMode2 |= VEH_FREEZE_PODIUM;

	// position and rotation of podium scene
	// Y coordinate (podiumPos.y) has added height
	posRot = gGT->level1->ptrSpawnType2_PosRot[1].posRot;
	InitData.podiumPos.x = posRot->pos.x;
	InitData.podiumPos.y = posRot->pos.y + PODIUM_SCENE_SPAWN_Y_OFFSET;
	InitData.podiumPos.z = posRot->pos.z;
	InitData.rot.x = posRot->rot.x;
	InitData.rot.y = posRot->rot.y;
	InitData.rot.z = posRot->rot.z;

	// convert 3 rotation shorts into rotation matrix
	ConvertRotToMatrix(&podiumMatrix, &InitData.rot.vec);
	// Move position of trophy girl
	gte_SetLightMatrix(&podiumMatrix);

	// CameraDC, this makes the camera stop following you as it does while racing, it must be zero to follow you
	gGT->cameraDC[0].cameraMode = CAMERA_MODE_FREECAM;

	// if someone placed third
	if (gGT->podium_modelIndex_Third != '\0')
	{
		InitData.characterPos.x = PODIUM_THIRD_POS_X;
		InitData.characterPos.y = PODIUM_THIRD_POS_Y;
		InitData.characterPos.z = PODIUM_THIRD_POS_Z;

		// create thread for "third"
		CS_Thread_Init(gGT->podium_modelIndex_Third, &R233.s_third[0], &InitData, PODIUM_THIRD_YAW_OFFSET, 0);
	}

	// if someone placed second
	if (gGT->podium_modelIndex_Second != '\0')
	{
		InitData.characterPos.x = PODIUM_SECOND_POS_X;
		InitData.characterPos.y = PODIUM_SECOND_POS_Y;
		InitData.characterPos.z = PODIUM_SECOND_POS_Z;

		// create thread for "second"
		CS_Thread_Init(gGT->podium_modelIndex_Second, &R233.s_second[0], &InitData, PODIUM_SECOND_YAW_OFFSET, 0);
	}

	InitData.characterPos.x = PODIUM_FIRST_POS_X;
	InitData.characterPos.y = PODIUM_FIRST_POS_Y;
	InitData.characterPos.z = PODIUM_FIRST_POS_Z;

	// create thread for "first"
	CS_Thread_Init(gGT->podium_modelIndex_First, &R233.s_first[0], &InitData, 0, 0);

	InitData.characterPos.x = PODIUM_TAWNA_POS_X;
	InitData.characterPos.y = PODIUM_TAWNA_POS_Y;
	InitData.characterPos.z = PODIUM_TAWNA_POS_Z;

	// create thread for trophy girl (internally called "tawna")
	CS_Thread_Init(gGT->podium_modelIndex_tawna, &R233.s_tawna[0], &InitData, PODIUM_TAWNA_YAW_OFFSET, 0);

	CS_Podium_Prize_Init(gGT->podiumRewardID, &R233.s_prize[0], &InitData.podiumPos);

	CS_Podium_Stand_Init(&InitData);

	victoryCamThread = (struct Thread *)PROC_BirthWithObject(PODIUM_VICTORY_CAMERA_THREAD_FLAGS, (void *)CS_Camera_ThTick_Podium, R233.s_victorycam, NULL);

	// if it allocated correctly
	if (victoryCamThread != 0)
	{
		struct CsPodiumCameraThreadObj *podiumCamera = victoryCamThread->object;
		podiumCamera->pathFrame32 = 0;
	}

	// change victory music based on who is first in the podium
	switch (gGT->podium_modelIndex_First - STATIC_CRASHDANCE)
	{
	// Crash, Coco, Fake Crash
	case PODIUM_DANCE_CRASH:
	case PODIUM_DANCE_COCO:
	case PODIUM_DANCE_FAKE_CRASH:
		// Crash's music
		podiumMusic = PODIUM_MUSIC_CRASH;
		break;

	// Cortex, NGin, NTrophy
	case PODIUM_DANCE_CORTEX:
	case PODIUM_DANCE_NGIN:
	case PODIUM_DANCE_NTROPY:
		// Cortex's music
		podiumMusic = PODIUM_MUSIC_CORTEX;
		break;

	// Polar Pura
	case PODIUM_DANCE_POLAR:
	case PODIUM_DANCE_PURA:
		// Polar and Pura's music
		podiumMusic = PODIUM_MUSIC_POLAR;
		break;

	// pinstripe kjoe
	case PODIUM_DANCE_PINSTRIPE:
	case PODIUM_DANCE_JOE:
		// Pinstripe's music
		podiumMusic = PODIUM_MUSIC_PINSTRIPE;
		break;

	// papu, roo, penta
	case PODIUM_DANCE_PAPU:
	case PODIUM_DANCE_ROO:
	case PODIUM_DANCE_PENTA:
		// Ripper Roo's music
		podiumMusic = PODIUM_MUSIC_ROO;
		break;

	// Tiny, Dingo, Oxide
	default:
		// Default music is Tiny Tiger's
		podiumMusic = PODIUM_MUSIC_TINY;
		break;
	}

	CDSYS_XAPlay(CDSYS_XA_TYPE_MUSIC, podiumMusic);

	return;
}
