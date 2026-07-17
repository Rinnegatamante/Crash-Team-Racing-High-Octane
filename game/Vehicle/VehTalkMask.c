#include <common.h>

enum
{
	TALK_MASK_3D_MODEL_SCALE = 0x2000,
	TALK_MASK_STATIC_MODEL_SCALE = FP(1.0),
	TALK_MASK_SCALE_SHIFT = 0xc,

	TALK_MASK_MOUTH_SAMPLE_MULTIPLIER = 7,
	TALK_MASK_MOUTH_NEGATIVE_ROUND_BIAS = 0x3fff,
	TALK_MASK_MOUTH_SAMPLE_SHIFT = 0xe,
	TALK_MASK_MOUTH_SILENCE_FRAME_THRESHOLD = 2,
	TALK_MASK_MOUTH_SNAP_TARGET_THRESHOLD = 4,
	TALK_MASK_MOUTH_SNAP_DELTA_THRESHOLD = 4,
	TALK_MASK_MOUTH_FINAL_LERP_DELTA_THRESHOLD = 6,
	TALK_MASK_MOUTH_LERP_STEP = 1,

	TALK_MASK_INIT_MODEL = STATIC_AKUAKU,
	TALK_MASK_INIT_OBJECT_SIZE = 6,
	TALK_MASK_XA_TYPE = CDSYS_XA_TYPE_EXTRA,
};

CTR_STATIC_ASSERT(TALK_MASK_3D_MODEL_SCALE == 0x2000);
CTR_STATIC_ASSERT(TALK_MASK_STATIC_MODEL_SCALE == 0x1000);
CTR_STATIC_ASSERT(TALK_MASK_MOUTH_SAMPLE_MULTIPLIER == 7);
CTR_STATIC_ASSERT(TALK_MASK_MOUTH_NEGATIVE_ROUND_BIAS == 0x3fff);
CTR_STATIC_ASSERT(TALK_MASK_MOUTH_SAMPLE_SHIFT == 0xe);
CTR_STATIC_ASSERT((s32)TALK_MASK_INIT_MODEL == (s32)STATIC_AKUAKU);
CTR_STATIC_ASSERT((s32)STATIC_UKAUKA - 1 == (s32)STATIC_AKUAKU);
CTR_STATIC_ASSERT(TALK_MASK_INIT_OBJECT_SIZE == 6);
CTR_STATIC_ASSERT(TALK_MASK_XA_TYPE == 1);
CTR_STATIC_ASSERT(ADV_MASK_HINT_UKA_UKA_XA_OFFSET == 0x1f);

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80068f90-0x80069178.
void VehTalkMask_ThTick(struct Thread *t)
{
	struct GameTracker *gGT = sdata->gGT;

	struct MaskHint *mhObj = t->object;
	struct Instance *mhInst = t->inst;

	s32 modelScale = TALK_MASK_3D_MODEL_SCALE;

	if (sdata->modelMaskHints3D != 0)
	{
		mhInst->model = sdata->modelMaskHints3D;
	}
	else
	{
		modelScale = TALK_MASK_STATIC_MODEL_SCALE;

		if (gGT->drivers[0] != 0)
		{
			b32 boolGoodGuy = VehPickupItem_MaskBoolGoodGuy(gGT->drivers[0]);

			s32 modelID = STATIC_UKAUKA - boolGoodGuy;
			mhInst->model = gGT->modelPtr[modelID];
		}
	}

	s32 scale = (mhObj->scale * modelScale) >> TALK_MASK_SCALE_SHIFT;
	mhInst->scale.x = scale;
	mhInst->scale.y = scale;
	mhInst->scale.z = scale;

	u32 lastFrame = VehFrameInst_GetNumAnimFrames(mhInst, 0) - 1;

	sdata->talkMaskXASamplePeak = sdata->XA_MaxSampleValInArr;

	s32 targetMouthFrame = sdata->talkMaskXASamplePeak * TALK_MASK_MOUTH_SAMPLE_MULTIPLIER;

	if (targetMouthFrame < 0)
	{
		targetMouthFrame += TALK_MASK_MOUTH_NEGATIVE_ROUND_BIAS;
	}

	targetMouthFrame >>= TALK_MASK_MOUTH_SAMPLE_SHIFT;

	if (sdata->talkMaskMaxMouthFrame < targetMouthFrame)
	{
		sdata->talkMaskMaxMouthFrame = targetMouthFrame;
	}

	s32 desiredMouthFrame = targetMouthFrame;
	if (targetMouthFrame < TALK_MASK_MOUTH_SILENCE_FRAME_THRESHOLD)
	{
		desiredMouthFrame = 0;
	}

	s32 currentMouthFrame = mhInst->animFrame;

	if (targetMouthFrame >= TALK_MASK_MOUTH_SNAP_TARGET_THRESHOLD)
	{
		s32 mouthFrameDelta = currentMouthFrame - desiredMouthFrame;

		if (mouthFrameDelta < 0)
		{
			mouthFrameDelta = -mouthFrameDelta;
		}

		if (mouthFrameDelta >= TALK_MASK_MOUTH_SNAP_DELTA_THRESHOLD)
		{
			mhInst->animFrame = (s16)desiredMouthFrame;

			goto SkipLerp;
		}
	}

	mhInst->animFrame = EngineSound_VolumeAdjust(desiredMouthFrame, currentMouthFrame, TALK_MASK_MOUTH_LERP_STEP);

SkipLerp:

	currentMouthFrame = mhInst->animFrame;

	// animFrame
	s32 mouthFrameDelta = currentMouthFrame - desiredMouthFrame;

	if (mouthFrameDelta < 0)
	{
		mouthFrameDelta = -mouthFrameDelta;
	}

	if (mouthFrameDelta < TALK_MASK_MOUTH_FINAL_LERP_DELTA_THRESHOLD)
	{
		mhInst->animFrame = EngineSound_VolumeAdjust(desiredMouthFrame, currentMouthFrame, TALK_MASK_MOUTH_LERP_STEP);
	}
	else
	{
		mhInst->animFrame = (s16)desiredMouthFrame;
	}

	// animation frame goes back and forth
	// 0x00: mouth close
	// 0x0C: mouth open

	if (mhInst->animFrame < 0)
	{
		mhInst->animFrame = 0;
	}
	else if (lastFrame < (u32)mhInst->animFrame)
	{
		mhInst->animFrame = (s16)lastFrame;
	}

	if (sdata->talkMask_boolDead != 0)
	{
		sdata->talkMask_boolDead = 0;

		// dead thread
		t->flags |= THREAD_FLAG_DEAD;
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80069178-0x800691e4.
struct Instance *VehTalkMask_Init(void)
{
	sdata->boolIsMaskThreadAlive = 1;
	sdata->talkMask_boolDead = 0;

	struct Instance *mhInst = INSTANCE_BirthWithThread(TALK_MASK_INIT_MODEL, sdata->s_head, SMALL, AKUAKU, VehTalkMask_ThTick, TALK_MASK_INIT_OBJECT_SIZE, 0);

	struct Thread *mhTh = mhInst->thread;
	mhTh->funcThDestroy = PROC_DestroyInstance;

	((struct MaskHint *)mhTh->object)->scale = 0;

	return mhInst;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800691e4-0x8006924c.
void VehTalkMask_PlayXA(struct Instance *i, s32 id)
{
	(void)i;
	struct Driver *d = sdata->gGT->drivers[0];

	if (d != 0)
	{
		b32 boolGoodGuy = VehPickupItem_MaskBoolGoodGuy(d);

		if (boolGoodGuy == 0)
		{
			id += ADV_MASK_HINT_UKA_UKA_XA_OFFSET;
		}
	}

	CDSYS_XAPlay(TALK_MASK_XA_TYPE, id);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8006924c-0x8006925c.
b32 VehTalkMask_boolNoXA(void)
{
	return sdata->XA_State == 0;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8006925c-0x80069284.
void VehTalkMask_End(void)
{
	CDSYS_XAPauseRequest();

	sdata->boolIsMaskThreadAlive = 0;
	sdata->talkMask_boolDead = 1;
}
