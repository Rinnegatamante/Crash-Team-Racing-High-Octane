#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b3dd8-0x800b3f88.
void AH_MaskHint_Start(s16 hintId, u16 bool_interruptWarppad)
{
	// copy parameters
	D232.maskWarppadBoolInterrupt = bool_interruptWarppad;
	D232.maskHintID = hintId;

	sdata->boolDraw3D_AdvMask = 1;

	struct AdvProgress *adv = &sdata->advProgress;
	int bitIndex = (int)hintId + ADV_REWARD_FIRST_HINT;
	UNLOCK_ADV_BIT(adv->rewards, bitIndex);

	// If this is "welcome to adventure arena"
	if (hintId == ADV_MASK_HINT_ID_WELCOME_TO_ARENA)
	{
		// "Using a Warppad" and "Map Information"
		UNLOCK_ADV_BIT(adv->rewards, ADV_REWARD_HINT_USING_WARP_PAD);
		UNLOCK_ADV_BIT(adv->rewards, ADV_REWARD_HINT_MAP_INFORMATION);
	}

	struct Driver *d = sdata->gGT->drivers[0];
	d->funcPtrs[DRIVER_FUNC_INIT] = VehPhysProc_FreezeEndEvent_Init;

	// If Aku / Uka model pointer is nullptr
	if (sdata->modelMaskHints3D == NULL)
	{
		LOAD_TalkingMask(LOAD_GetAdvPackIndex(), !VehPickupItem_MaskBoolGoodGuy(d));

		// 3.0s to spawn mask
		D232.maskSpawnFrame = AH_MASKHINT_LONG_SPAWN_FRAMES;
	}

	// if model is not nullptr
	else
	{
		// 0.667s to spawn mask
		D232.maskSpawnFrame = AH_MASKHINT_SHORT_SPAWN_FRAMES;
	}

	int offsetSlot = bool_interruptWarppad & AH_MASKHINT_OFFSET_WARPPAD_INTERRUPT;

	CTR_COPY_VEC3(D232.maskOffsetPos.v, D232.maskHintOffsets.pos[offsetSlot].v);
	CTR_COPY_VEC3(D232.maskOffsetRot.v, D232.maskHintOffsets.rot[offsetSlot].v);

	for (int i = 0; i < 3; i++)
	{
		// 4 bytes for 4 volumes
		// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b3f3c-0x800b3f54 for mask-hint volume backup.
		D232.audioBackup[i] = howl_VolumeGet(i);
	}

	return;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b3f88-0x800b3f98.
b32 AH_MaskHint_boolCanSpawn(void)
{
	// 0 - aku is gone,
	// 1 - aku is speaking

	// return 0, if aku is speaking -- can't spawn
	// return 1, if aku is gone -- can spawn

	return sdata->AkuAkuHintState == 0;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b3f98-0x800b42b4.
void AH_MaskHint_SetAnim(int scale)
{
	MATRIX *m;
	struct GameTracker *gGT = sdata->gGT;
	struct PushBuffer *pb = &gGT->pushBuffer[0];

	m = &pb->matrix_Camera;
	gte_SetRotMatrix(m);
	gte_SetTransMatrix(m);

	CTR_GteLoadSVec3V0(&D232.maskOffsetPos);
	gte_rt();

	Vec3 posEndInt;
	SVec3 posEnd;

	CTR_GteStoreMAC(posEndInt.v);

	posEnd.x = posEndInt.x;
	posEnd.y = posEndInt.y;
	posEnd.z = posEndInt.z;

	SVec3 rotEnd;
	rotEnd.x = pb->rot.x - D232.maskOffsetRot.x;
	rotEnd.y = pb->rot.y + D232.maskOffsetRot.y;
	rotEnd.z = pb->rot.z - D232.maskOffsetRot.z;

	SVec3 posCurr;
	SVec3 rotCurr;
	CAM_ProcessTransition(&posCurr, &rotCurr, &D232.maskCamPosStart, &D232.maskCamRotStart, &posEnd, &rotEnd, scale);

	int rot = AH_MASKHINT_FULL_BLEND;
	if (D232.maskSpawnFrame - AH_MASKHINT_SPAWN_RING_FRAMES < D232.maskFrameCurr)
	{
		rot = ((D232.maskSpawnFrame - D232.maskFrameCurr) * rot) / AH_MASKHINT_SPAWN_RING_FRAMES;
	}

	// 4096->50
	rot = (rot * AH_MASKHINT_SPAWN_SPIRAL_RADIUS) >> 0xc;

	int angle = (scale << 0xf) >> 0xc;
	D232.maskAngle = angle;

	int sin = MATH_Sin(angle);
	int cos = MATH_Cos(angle);

	struct Instance *mhInst = sdata->instMaskHints3D;
	posCurr.x += (s16)((sin * rot) >> 0xc);
	posCurr.z += (s16)((cos * rot) >> 0xc);

	rotCurr.y += angle;
	ConvertRotToMatrix(&mhInst->matrix, &rotCurr);

	((struct MaskHint *)mhInst->thread->object)->scale = scale * 4 - 1;

	angle = (sdata->frameCounter + gGT->timer) * 0x20;
	sin = MATH_Sin(angle);
	posCurr.y += (s16)(((sin << 4) >> 0xc) * scale >> 0xc);

	mhInst->matrix.t[0] = posCurr.x;
	mhInst->matrix.t[1] = posCurr.y;
	mhInst->matrix.t[2] = posCurr.z;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b42b4-0x800b43cc.
void AH_MaskHint_SpawnParticles(s16 numParticles, struct ParticleEmitter *emSet, int maskAnim)

{
	struct Particle *particle;
	struct Instance *maskInst;
	int i, j;

	maskAnim = maskAnim + AH_MASKHINT_FULL_BLEND;
	if (maskAnim > 0x3fff)
	{
		maskAnim = 0x3fff;
	}

	// "hubdustpuff"
	struct IconGroup *ig = sdata->gGT->iconGroup[0x10];

	// talking mask instance
	maskInst = sdata->instMaskHints3D;

	for (i = 0; i < numParticles; i++)
	{
		particle = Particle_Init(0, ig, emSet);
		if (particle == NULL)
		{
			continue;
		}

		for (j = 0; j < 3; j++)
		{
			particle->axis[j].startVal += maskInst->matrix.t[j] * 0x100;
		}

		particle->axis[5].startVal = (particle->axis[5].startVal * maskAnim) >> 0xc;
		particle->axis[5].velocity = (particle->axis[5].velocity * maskAnim) >> 0xc;

		particle->otIndexOffset -= 5;
	}

	return;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b43cc-0x800b4470.
void AH_MaskHint_LerpVol(int blend)
{
	int diff;
	int volume;
	u8 backup;

	for (s32 i = 0; i < 3; i++)
	{
		backup = D232.audioBackup[i];

		diff = D232.maskAudioTargetVolume[i] - backup;
		volume = backup + ((diff * blend) >> 12);

		// restore backups of Volume settings,
		// that were originally saved in AH_MaskHint_Start
		howl_VolumeSet(i, volume & 0xFF);
	}
}

force_inline void AH_MaskHint_DrawRepeatPrompt(void)
{
	int lngIndex = 0;
	b32 boolFound = false;

	if (sdata->AkuAkuHintState != AH_MASKHINT_STATE_REPEAT_PROMPT)
	{
		return;
	}

	const s16 *ptrLngID = &D232.hintMenuLngIndex[0];
	struct GameTracker *gGT = sdata->gGT;
	struct Driver *d = gGT->drivers[0];

	for (/**/; *ptrLngID > -1; ptrLngID++)
	{
		if (D232.maskHintID == (ptrLngID[0] - AH_HINTMENU_HINT_LNG_FIRST) / 2)
		{
			boolFound = true;
			break;
		}
	}

	if (!boolFound)
	{
		return;
	}

	// Retail finds the hint subtitle entry above, but the shipped path draws a
	// generic "press start to repeat" prompt instead of that hint text.
	if (VehPickupItem_MaskBoolGoodGuy(d))
	{
		lngIndex = LNG_AKU_HINT_REPEAT_INSTRUCTIONS;
	}
	else
	{
		lngIndex = LNG_TO_HEAR_THIS_HINT_AGAIN_PRESS_THE_START;
	}

	RECT r;
	r.x = -10;
	r.y = 0xb0;
	r.w = 0x214;
	r.h = 8 + DecalFont_DrawMultiLine(sdata->lngStrings[lngIndex], 0x100, 0xb4, 400, 2, 0xffff8000);

	RECTMENU_DrawInnerRect(&r, 4, gGT->backBuffer->otMem.uiOT);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 overlay 232 0x800b4470-0x800b4c80.
void AH_MaskHint_Update()
{
	struct GameTracker *gGT = sdata->gGT;
	struct Driver *d = gGT->drivers[0];
	struct CameraAngleAxisScratch angleAxisWork;
	SVec3 pos;
	SVec3 rot;

	switch (sdata->AkuAkuHintState - 1)
	{
	case 0:
		if (sdata->XA_State != 0)
		{
			return;
		}

		sdata->AkuAkuHintState++;
		break;

	case 1:
	{
		int absSpeedApprox = d->speedApprox;
		if (absSpeedApprox < 0)
		{
			absSpeedApprox = -absSpeedApprox;
		}
		if (absSpeedApprox > AH_MASKHINT_MAX_START_SPEED)
		{
			return;
		}

		if ((D232.maskWarppadBoolInterrupt & 1) == 0)
		{
			struct CameraDC *cdc = &gGT->cameraDC[0];

			CTR_COPY_VEC3(cdc->driverOffset_CamEyePos.v, D232.eyePos.v);

			CTR_COPY_VEC3(cdc->driverOffset_CamLookAtPos.v, D232.lookAtPos.v);

			cdc->flags |= 8;

			// NOTE(aalhendi): Retail passes a stack work buffer here, not 0x1f800108.
			CAM_FollowDriver_AngleAxis(cdc, d, &angleAxisWork, &pos, &rot);
			CAM_SetDesiredPosRot(cdc, &pos, &rot);
		}

		D232.maskWarppadDelayFrames = AH_MASKHINT_CAMERA_DELAY_FRAMES;

		sdata->AkuAkuHintState++;
	}
	break;

	case 2:

		if (((D232.maskWarppadBoolInterrupt & 1) == 0) && ((gGT->cameraDC[0].flags & CAMERA_FLAG_TRANSITION_HOLD) == 0) &&
		    (D232.maskSpawnFrame != AH_MASKHINT_SHORT_SPAWN_FRAMES))
		{
			return;
		}

		struct Instance *dInst = d->instSelf;
		sdata->instMaskHints3D = VehTalkMask_Init();
		struct Instance *mhInst = sdata->instMaskHints3D;

		SVECTOR matrixRot;
		CTR_MatrixToRot(&matrixRot, &dInst->matrix, 0x11);

		// not a typo
		D232.maskCamRotStart.x = matrixRot.vy & 0xfff;
		D232.maskCamRotStart.z = matrixRot.vz & 0xfff;
		D232.maskCamRotStart.y = matrixRot.vx & 0xfff;

		CTR_COPY_VEC3(D232.maskCamPosStart.v, dInst->matrix.t);

		((struct MaskHint *)mhInst->thread->object)->scale = 0;

		AH_MaskHint_SetAnim(0);

		D232.maskFrameCurr = 0;

		sdata->AkuAkuHintState++;
		break;

	case 3:

		// first frame "whoosh" sound
		if (D232.maskFrameCurr == 0)
		{
			// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b46d4-0x800b46e4 for mask spawn-start SFX.
			OtherFX_Play_LowLevel(AH_MASKHINT_SFX_SPAWN, 1, HOWL_SFX_DEFAULT_FLAGS);
		}

		// if 3-second spawn, play more sounds
		if (D232.maskSpawnFrame == AH_MASKHINT_LONG_SPAWN_FRAMES)
		{
			if (D232.maskFrameCurr == 10)
			{
				// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b470c-0x800b4774 for mask spawn pulse 10 SFX.
				OtherFX_Play_LowLevel(AH_MASKHINT_SFX_SPAWN, 0, 0xd78a80);
			}
			else if (D232.maskFrameCurr == 20)
			{
				// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b4728-0x800b4774 for mask spawn pulse 20 SFX.
				OtherFX_Play_LowLevel(AH_MASKHINT_SFX_SPAWN, 1, 0xaf9480);
			}
			else if (D232.maskFrameCurr == 25)
			{
				// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b4744-0x800b4774 for mask spawn pulse 25 SFX.
				OtherFX_Play_LowLevel(AH_MASKHINT_SFX_SPAWN, 0, 0x879e80);
			}
			else if (D232.maskFrameCurr == 30)
			{
				// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b4760-0x800b4774 for mask spawn pulse 30 SFX.
				OtherFX_Play_LowLevel(AH_MASKHINT_SFX_SPAWN, 1, 0x5fa880);
			}
		}

		int timer4096 = (D232.maskFrameCurr << 0xc) / D232.maskSpawnFrame;

		AH_MaskHint_SetAnim(timer4096);

		AH_MaskHint_SpawnParticles(AH_MASKHINT_SPAWN_PARTICLES, &D232.emSet_maskSpawn[0], timer4096);

		// if not finished spawning
		if (D232.maskFrameCurr < D232.maskSpawnFrame)
		{
			D232.maskFrameCurr++;

			timer4096 = (D232.maskFrameCurr << 0xc) / D232.maskSpawnFrame;

			AH_MaskHint_LerpVol(timer4096);
			break;
		}

		// NOTE(aalhendi): Retail only waits for the mask model pointer.
		if (sdata->modelMaskHints3D == 0)
		{
			AH_MaskHint_LerpVol(AH_MASKHINT_FULL_BLEND);
			break;
		}

		if (((D232.maskWarppadBoolInterrupt & 1) != 0) || ((gGT->cameraDC[0].flags & CAMERA_FLAG_TRANSITION_HOLD) != 0))
		{
			AH_MaskHint_LerpVol(AH_MASKHINT_FULL_BLEND);

			AH_MaskHint_SpawnParticles(AH_MASKHINT_LEAVE_PARTICLES, &D232.emSet_maskLeave[0], AH_MASKHINT_FULL_BLEND);

			VehTalkMask_PlayXA(sdata->instMaskHints3D, D232.maskHintID);

			if (((gGT->gameMode1 & ADVENTURE_ARENA) != 0) &&

			    // Not "Welcome to Adventure" or "You need a Boss Key"
			    (D232.maskHintID != ADV_MASK_HINT_ID_WELCOME_TO_ARENA) && (D232.maskHintID != ADV_MASK_HINT_ID_MAP_INFORMATION))
			{
				// hide UI map
				gGT->hudFlags |= HUD_FLAG_HIDE_ADVENTURE_MAP;
			}

			sdata->AkuAkuHintState++;
			break;
		}
		break;

	case 4:
	{
		// NOTE(aalhendi): Native draws only this shared prompt earlier from
		// AH_Map_Main so synchronous DrawOTag sees it; the rest of this state
		// remains retail-timed here.
#if !defined(CTR_NATIVE)
		AH_MaskHint_DrawRepeatPrompt();
#endif

		AH_MaskHint_SetAnim(AH_MASKHINT_FULL_BLEND);

		b32 delayComplete = D232.maskWarppadDelayFrames == 0;
		if (!delayComplete)
		{
			delayComplete = D232.maskWarppadDelayFrames == 1;
			D232.maskWarppadDelayFrames--;
		}

		if ((delayComplete && (VehTalkMask_boolNoXA() || ((sdata->gGamepads->gamepad[0].buttonsTapped & BTN_TRIANGLE) != 0))) &&
		    (sdata->AkuAkuHintState++,

		     // If you're in Adventure Arena
		     ((gGT->gameMode1 & ADVENTURE_ARENA) != 0)))
		{
			// show map again
			gGT->hudFlags &= ~HUD_FLAG_HIDE_ADVENTURE_MAP;
		}
	}
	break;

	case 5:

		AH_MaskHint_SpawnParticles(AH_MASKHINT_VANISH_PARTICLES, &D232.emSet_maskLeave[0], AH_MASKHINT_FULL_BLEND);

		// vanish sound
		// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b4b24-0x800b4b2c for mask vanish SFX.
		OtherFX_Play(AH_MASKHINT_SFX_VANISH, 1);

		VehTalkMask_End();

		if ((D232.maskWarppadBoolInterrupt & 1) == 0)
		{
			// transition back to player
			gGT->cameraDC[0].flags |= CAMERA_FLAG_TRANSITION_BACK;
		}

		sdata->AkuAkuHintState++;
		break;

	case 6:

		AH_MaskHint_LerpVol(AH_MASKHINT_FULL_BLEND - gGT->cameraDC[0].transitionBlend);

		if (((gGT->cameraDC[0].flags & CAMERA_FLAG_TRANSITION_AWAY) == 0) || ((D232.maskWarppadBoolInterrupt & 1) != 0))
		{
			AH_MaskHint_SetAnim(0);
			AH_MaskHint_LerpVol(0);

			D232.maskWarppadDelayFrames = 0;
			if ((D232.maskWarppadBoolInterrupt & 1) != 0)
			{
				D232.maskWarppadDelayFrames = AH_MASKHINT_INTERRUPT_DONE_DELAY_FRAMES;
			}

			sdata->AkuAkuHintState++;
		}
		break;

	case 7:

		AH_MaskHint_LerpVol(0);

		D232.maskWarppadDelayFrames--;

		if (D232.maskWarppadDelayFrames < 1)
		{
			RECTMENU_ClearInput();

			sdata->AkuAkuHintState = 0;
			sdata->boolDraw3D_AdvMask = 0;

			gGT->gameMode2 &= ~(VEH_FREEZE_DOOR);
			d->funcPtrs[DRIVER_FUNC_INIT] = VehPhysProc_Driving_Init;
		}

		break;
	}
}
