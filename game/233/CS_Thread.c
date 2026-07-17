#include <common.h>

struct CSThreadParentFrameScratch
{
	SVec3Slot parentPos;
	u8 pad_110[0x08];
	SVec3Slot parentRot;
};

enum CsPathModelKind
{
	CS_PATH_MODEL_PPOINT_THING_INTRO = STATIC_PPOINTTHINGINTRO - STATIC_PPOINTTHINGINTRO,
	CS_PATH_MODEL_PR_THING_INTRO = STATIC_PRTHINGINTRO - STATIC_PPOINTTHINGINTRO,
	CS_PATH_MODEL_OXIDE_LIL_SHIP = STATIC_OXIDELILSHIP - STATIC_PPOINTTHINGINTRO,
	CS_PATH_MODEL_COCO_SELECT = STATIC_COCOSELECT - STATIC_PPOINTTHINGINTRO,
	CS_PATH_MODEL_END_OXIDE_BIG_SHIP = STATIC_ENDOXIDEBIGSHIP - STATIC_PPOINTTHINGINTRO,
	CS_PATH_MODEL_END_OXIDE_LIL_SHIP = STATIC_ENDOXIDELILSHIP - STATIC_PPOINTTHINGINTRO,
	CS_PATH_MODEL_OXIDE_SPEAKER = STATIC_OXIDESPEAKER - STATIC_PPOINTTHINGINTRO,
	CS_PATH_MODEL_KIND_COUNT = STATIC_OXIDESPEAKER - STATIC_PPOINTTHINGINTRO + 1,
};

enum CsThreadConstants
{
	CS_DECODED_OPCODE_WORD_COUNT = 5,
	CS_PODIUM_SECOND_HIDE_START_FRAME = 0x65,
	CS_PODIUM_SECOND_HIDE_FRAME_COUNT = 0x87,
	CS_PODIUM_SECOND_DEPTH_BIAS_RESTORE = 2,
	CS_PODIUM_FIRST_HIDE_START_FRAME = 0x83,
	CS_PODIUM_FIRST_HIDE_FRAME_COUNT = 0x69,
	CS_PODIUM_FIRST_DEPTH_BIAS_RESTORE = 6,
	CS_FADE_FROM_BLACK_CURRENT = 0x1fff,
	CS_FADE_FROM_BLACK_TARGET = FP_ONE,
	CS_FADE_FROM_BLACK_STEP = -0x2aa,
	CS_RANDOM_DURATION_SHIFT = 2,
	CS_RANDOM_DURATION_MASK = ANG_TWO_PI - 1,
	CS_FRAME32_SHIFT = 5,
	CS_FRAME32_UNIT = 1 << CS_FRAME32_SHIFT,
	CS_FRAME32_MASK = CS_FRAME32_UNIT - 1,
	CS_CAMERA_PATH_TIME_NUMERATOR_SHIFT = 11,
	CS_CAMERA_PATH_TIME_DENOMINATOR_SHIFT = 16,
	CS_CAMERA_DISTANCE_DEFAULT = 0x100,
	CS_CAMERA_DISTANCE_CLOSE = 0x50,
	CS_CAMERA_DISTANCE_OXIDE_SHIP = 0x278,
	CS_CAMERA_DISTANCE_INTRO_WIDE = 0x1eb,
	CS_CAMERA_DISTANCE_INTRO_MEDIUM = 0x14d,
	CS_ND_CRATE_CAMERA_DISTANCE = 0x140,
	CS_ND_CRATE_SKIP_MIN_FRAME32 = 0xb5,
	CS_CREDITS_LEVEL_COUNT = SCRAPBOOK - CREDITS_CRASH,
	CS_OXIDE_ENDING_LEVEL_COUNT = OXIDE_TRUE_ENDING - OXIDE_ENDING + 1,
	CS_ADV_HUB_LEVEL_COUNT = INTRO_RACE_TODAY - GEM_STONE_VALLEY,
	CS_INTRO_MODEL_SCRIPT_COUNT = 0x10,
	CS_BOX_MODEL_SCRIPT_COUNT = 0x2b,
	CS_DANCE_MODEL_SCRIPT_COUNT = 0x10,
	CS_TAWNA_MODEL_SCRIPT_COUNT = 4,
	CS_PINHEAD_DEFAULT_SCRIPT_COUNT = 5,
	CS_KART_FRAME_OVERRIDE_COUNT = 4,
	CS_MASK_KART_MODEL_COUNT = 2,
	CS_CREDITS_ND_CRATE_SCRIPT_OFFSET = 0x18,
	CS_TRUE_ENDING_SCRIPT_OFFSET = 0x30,
	CS_DEFAULT_SCALE = FP_ONE,
	CS_PODIUM_CHARACTER_SCALE = 0x2800,
	CS_DEFAULT_PARTICLE_ID = 0xff,
	CS_DEFAULT_COLOR_WORD = 0x2e808080,
	CS_RANDOM_BYTE_MASK = 0xff,
	CS_ANIM_INDEX_LOW_BYTE_MASK = 0xff,
	CS_CONDITION_RESULT_MASK = 0xffff,
	CS_RANDOM_ALPHA_MASK = 0x7ff,
	CS_RANDOM_ALPHA_BASE = 0x400,
	CS_MODEL_LOD_DISTANCE_VISIBLE = 20000,
	CS_RANDOM_CLEAR_BOX_MASK = 0xf,
	CS_INTERPOLATE_LINE_COLOR_CODE = 0x42000000,
	CS_INTERPOLATE_LINE_OT_TAG = 0x05000000,
	CS_INTERPOLATE_LINE_DEPTH_MIN = 1,
	CS_INTERPOLATE_LINE_DEPTH_RANGE = 0x11ff,
	CS_INTERPOLATE_LINE_FADE_START_DEPTH = 0xa00,
	CS_INTERPOLATE_LINE_FADE_END_DEPTH = 0x1200,
	CS_INTERPOLATE_LINE_MAX_COLOR = 0x3f,
	CS_INTERPOLATE_LINE_FADE_SHIFT = 11,
	CS_INTERPOLATE_LINE_OT_SHIFT = 6,
	CS_INTERPOLATE_LINE_MAX_OT_INDEX = 0x3ff,
	CS_SUBTITLE_TEXT_MAX_WIDTH = 460,
	CS_SUBTITLE_BOX_HALF_WIDTH = 236,
	CS_SUBTITLE_BOX_Y_OFFSET = 4,
	CS_SUBTITLE_BOX_WIDTH = 472,
	CS_SUBTITLE_BOX_HEIGHT_PADDING = 8,
	CS_SUBTITLE_BOX_DRAW_STYLE = 4,
	CS_XA_ANIM_SYNC_NUMERATOR = 0x1e00,
	CS_XA_ANIM_SYNC_DENOMINATOR = 0xac44,
	CS_AUDIO_VOLUME_FX_BYTE_OFFSET = 2,
	CS_AUDIO_VOLUME_MUSIC_BYTE_OFFSET = 4,
	CS_AUDIO_VOLUME_VOICE_BYTE_OFFSET = 6,
	CS_BOSS_END_OVERLAY_TRANSITION_FRAMES = 3,
};

CTR_STATIC_ASSERT(offsetof(struct CSThreadParentFrameScratch, parentPos) == 0x00);
CTR_STATIC_ASSERT(offsetof(struct CSThreadParentFrameScratch, parentRot) == 0x10);
CTR_STATIC_ASSERT(CS_PATH_MODEL_KIND_COUNT == 63);
CTR_STATIC_ASSERT(CS_DECODED_OPCODE_WORD_COUNT == 5);
CTR_STATIC_ASSERT((u16)CS_FADE_FROM_BLACK_STEP == 0xfd56);
CTR_STATIC_ASSERT(CS_FRAME32_UNIT == 0x20);
CTR_STATIC_ASSERT(CS_CREDITS_LEVEL_COUNT == 20);
CTR_STATIC_ASSERT(CS_OXIDE_ENDING_LEVEL_COUNT == 2);
CTR_STATIC_ASSERT(CS_ADV_HUB_LEVEL_COUNT == 5);
CTR_STATIC_ASSERT(CS_INTRO_MODEL_SCRIPT_COUNT ==
                  sizeof(((struct OverlayRDATA_233 *)0)->introModelScripts) / sizeof(((struct OverlayRDATA_233 *)0)->introModelScripts[0]));
CTR_STATIC_ASSERT(CS_BOX_MODEL_SCRIPT_COUNT ==
                  sizeof(((struct OverlayRDATA_233 *)0)->boxModelScripts) / sizeof(((struct OverlayRDATA_233 *)0)->boxModelScripts[0]));
CTR_STATIC_ASSERT(CS_DANCE_MODEL_SCRIPT_COUNT ==
                  sizeof(((struct OverlayRDATA_233 *)0)->danceFirstScripts) / sizeof(((struct OverlayRDATA_233 *)0)->danceFirstScripts[0]));
CTR_STATIC_ASSERT(CS_DANCE_MODEL_SCRIPT_COUNT ==
                  sizeof(((struct OverlayRDATA_233 *)0)->danceOtherScripts) / sizeof(((struct OverlayRDATA_233 *)0)->danceOtherScripts[0]));
CTR_STATIC_ASSERT(CS_KART_FRAME_OVERRIDE_COUNT ==
                  sizeof(((struct OverlayDATA_233 *)0)->cs_initMatrixTable) / sizeof(((struct OverlayDATA_233 *)0)->cs_initMatrixTable[0]));
CTR_STATIC_ASSERT(CS_CREDITS_ND_CRATE_SCRIPT_OFFSET == 0x18);
CTR_STATIC_ASSERT(CS_TRUE_ENDING_SCRIPT_OFFSET == 0x30);
CTR_STATIC_ASSERT(CS_INTERPOLATE_LINE_FADE_END_DEPTH - CS_INTERPOLATE_LINE_FADE_START_DEPTH == 0x800);
CTR_STATIC_ASSERT(OFFSETOF(union CsOpcodeMeta, animIndex) == CS_AUDIO_VOLUME_FX_BYTE_OFFSET);
CTR_STATIC_ASSERT(OFFSETOF(union CsOpcodeMeta, frameStart) == CS_AUDIO_VOLUME_MUSIC_BYTE_OFFSET);
CTR_STATIC_ASSERT(OFFSETOF(union CsOpcodeMeta, frameEnd) == CS_AUDIO_VOLUME_VOICE_BYTE_OFFSET);

static const u32 CS_INTERPOLATE_LINE_DRAW_MODE = 0xe1000a20u;

static void CS_SaveDecodedOpcode(const struct CutsceneObj *cs, int out[CS_DECODED_OPCODE_WORD_COUNT])
{
	out[0] = cs->decodedOpcode.words[0];
	out[1] = cs->decodedOpcode.words[1];
	out[2] = cs->decodedOpcode.words[2];
	out[3] = cs->decodedOpcode.words[3];
	out[4] = cs->decodedOpcode.words[4];
}

static void CS_RestoreDecodedOpcode(struct CutsceneObj *cs, const int in[CS_DECODED_OPCODE_WORD_COUNT])
{
	cs->decodedOpcode.words[0] = in[0];
	cs->decodedOpcode.words[1] = in[1];
	cs->decodedOpcode.words[2] = in[2];
	cs->decodedOpcode.words[3] = in[3];
	cs->decodedOpcode.words[4] = in[4];
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ac840-0x800ade8c
int CS_Thread_UseOpcode(struct Instance *instance, struct CutsceneObj *cs)
{
	u8 numPlayers;
	int frameBoundaryHit;
	s16 numCamPathPoints;
	s16 gameModeTarget;
	u16 clockEffectFlags;
	u16 cutsceneFlags;
	u32 conditionMet;
	int lodIndexState;
	char *const *cutsceneOpcodes;
	s16 levelToLoad;
	int distanceToScreen;
	struct Thread *dancerThread;
	char *opcodeAt;
	int animFrame32;
	union CsOpcodeMeta *opcodeMeta;
	s16 *opcodeMetaShorts;
	struct CsInitMatrixEntry *frameData;
	int nextFrameTime;
	int lodIndex;
	struct ModelHeader *modelHeader;
	int metadataBackup[CS_DECODED_OPCODE_WORD_COUNT];
	SVec3 camRot;
	SVec3 camPos;
	s16 camPathFlags[2];
	int animIndex;
	int opcodeDuration;
	int opcodeChanged;
	int elapsedTimeRemaining;

	struct GameTracker *gGT = sdata->gGT;
	CS_SaveDecodedOpcode(cs, metadataBackup);

	if (instance != 0)
	{
		if ((instance->flags & SPLIT_LINE) != 0)
		{
			instance->vertSplit = D233.VertSplitLine;
		}

		if ((int)instance->model->id == (int)(u8)gGT->podium_modelIndex_Second)
		{
			if ((u32)(D233.podiumCameraFrame - CS_PODIUM_SECOND_HIDE_START_FRAME) < CS_PODIUM_SECOND_HIDE_FRAME_COUNT)
			{
				instance->flags |= HIDE_MODEL;
			}
			else
			{
				if ((instance->flags & HIDE_MODEL) == 0)
				{
					goto afterPodiumSecondModelCheck;
				}
				instance->depthBiasNormal -= CS_PODIUM_SECOND_DEPTH_BIAS_RESTORE;
				instance->depthBiasSecondary -= CS_PODIUM_SECOND_DEPTH_BIAS_RESTORE;
				instance->flags &= ~HIDE_MODEL;
			}
		}
	afterPodiumSecondModelCheck:

		if ((int)instance->model->id == (int)(u8)gGT->podium_modelIndex_First)
		{
			if ((u32)(D233.podiumCameraFrame - CS_PODIUM_FIRST_HIDE_START_FRAME) < CS_PODIUM_FIRST_HIDE_FRAME_COUNT)
			{
				instance->flags |= HIDE_MODEL;
			}
			else
			{
				if ((instance->flags & HIDE_MODEL) == 0)
				{
					goto afterPodiumFirstModelCheck;
				}
				instance->depthBiasNormal -= CS_PODIUM_FIRST_DEPTH_BIAS_RESTORE;
				instance->depthBiasSecondary -= CS_PODIUM_FIRST_DEPTH_BIAS_RESTORE;
				instance->flags &= ~HIDE_MODEL;
			}
		}
	afterPodiumFirstModelCheck:

		if ((cs->flags & CS_FLAG_ADV_CHAR_SELECT_LOGIC) != 0)
		{
			if (((int)instance->model->id - STATIC_CRASHSELECT == (int)gGarage.garageCharacterIDs[sdata->advCharSelectIndex_curr]) &&
			    (gGarage.boolSelected == 1))
			{
				if ((cs->flags & CS_FLAG_ADV_CHAR_SELECT_SELECTED) == 0)
				{
					gGT->pushBuffer[0].fadeFromBlack_currentValue = CS_FADE_FROM_BLACK_CURRENT;
					gGT->pushBuffer[0].fadeFromBlack_desiredResult = CS_FADE_FROM_BLACK_TARGET;
					gGT->pushBuffer[0].fade_step = CS_FADE_FROM_BLACK_STEP;
					cs->flags |= CS_FLAG_ADV_CHAR_SELECT_SELECTED;
					CS_ScriptCmd_OpcodeAt(cs, R233.advCharSelectSelectOpcodes[(int)instance->model->id - STATIC_CRASHSELECT]);
					CS_SaveDecodedOpcode(cs, metadataBackup);
				reloadAdvCharSelectOpcodeState:
					cs->animFrame32 = cs->decodedOpcode.words[2];
					int rng = MixRNG_Scramble();
					opcodeMeta = cs->metadataMeta;
					opcodeMetaShorts = (s16 *)opcodeMeta;
					cs->opcodeDuration = opcodeMeta->frameStart + (s16)((int)(((rng >> CS_RANDOM_DURATION_SHIFT) & CS_RANDOM_DURATION_MASK) *
					                                                          (((int)opcodeMeta->frameEnd - (int)opcodeMeta->frameStart) + 1)) >>
					                                                    FRACTIONAL_BITS);
				}
			}
			else
			{
				if ((cs->flags & CS_FLAG_ADV_CHAR_SELECT_SELECTED) != 0)
				{
					cs->flags &= ~CS_FLAG_ADV_CHAR_SELECT_SELECTED;
					CS_ScriptCmd_OpcodeAt(cs, R233.advCharSelectDeselectOpcodes[(int)instance->model->id - STATIC_CRASHSELECT]);
					CS_SaveDecodedOpcode(cs, metadataBackup);
					goto reloadAdvCharSelectOpcodeState;
				}
			}
		}
	}

	opcodeDuration = (int)cs->opcodeDuration;
	animFrame32 = cs->animFrame32;
	lodIndexState = (int)cs->lodIndex;
	elapsedTimeRemaining = gGT->elapsedTimeMS;
	opcodeMeta = cs->metadataMeta;
	opcodeMetaShorts = (s16 *)opcodeMeta;
	animIndex = (int)opcodeMeta->animIndex;

	if (instance == 0)
	{
		numCamPathPoints = CAM_Path_GetNumPoints();
		if ((int)numCamPathPoints != 0)
		{
			int cameraPathFrame = ((s32)((u32)gGT->msInThisLEV << CS_CAMERA_PATH_TIME_NUMERATOR_SHIFT)) >> CS_CAMERA_PATH_TIME_DENOMINATOR_SHIFT;
			if (cameraPathFrame < (int)numCamPathPoints + -1)
			{
				CAM_Path_Move(cameraPathFrame, camPos.v, camRot.v, camPathFlags);
				gGT->pushBuffer[0].pos = camPos;
				gGT->pushBuffer[0].rot = camRot;
			}
			else
			{
				if (opcodeMeta->opcode == CS_OPCODE_YIELD)
				{
					CS_ScriptCmd_OpcodeNext(cs);
				}
				CAM_Path_Move((int)(s16)(numCamPathPoints + -1), gGT->pushBuffer[0].pos.v, gGT->pushBuffer[0].rot.v, camPathFlags);
			}

			clockEffectFlags = gGT->clockEffectEnabled;
			gGT->clockEffectEnabled = clockEffectFlags & ~CAM_PATH_FLAG_CLOCK_EFFECT;
			if ((camPathFlags[0] & CAM_PATH_FLAG_CLOCK_EFFECT) != 0)
			{
				gGT->clockEffectEnabled = (clockEffectFlags & ~CAM_PATH_FLAG_CLOCK_EFFECT) | CAM_PATH_FLAG_CLOCK_EFFECT;
			}

			if ((cs->flags & CS_FLAG_CAMERA_DISTANCE_OVERRIDE) == 0)
			{
				gGT->pushBuffer[0].distanceToScreen_PREV = CS_CAMERA_DISTANCE_DEFAULT;
				if ((camPathFlags[0] & CAM_PATH_FLAG_DISTANCE_TO_SCREEN_0x50) != 0)
				{
					gGT->pushBuffer[0].distanceToScreen_PREV = CS_CAMERA_DISTANCE_CLOSE;
				}
				if ((camPathFlags[0] & CAM_PATH_FLAG_DISTANCE_TO_SCREEN_0x278) != 0)
				{
					gGT->pushBuffer[0].distanceToScreen_PREV = CS_CAMERA_DISTANCE_OXIDE_SHIP;
				}
				if ((camPathFlags[0] & CAM_PATH_FLAG_DISTANCE_TO_SCREEN_0x1EB) != 0)
				{
					gGT->pushBuffer[0].distanceToScreen_PREV = CS_CAMERA_DISTANCE_INTRO_WIDE;
				}
				if ((camPathFlags[0] & CAM_PATH_FLAG_DISTANCE_TO_SCREEN_0x14D) != 0)
				{
					gGT->pushBuffer[0].distanceToScreen_PREV = CS_CAMERA_DISTANCE_INTRO_MEDIUM;
				}
			}

			if (((camPathFlags[0] & CAM_PATH_FLAG_RANDOM_CLEAR_BOX) != 0) && ((MixRNG_Scramble() & CS_RANDOM_CLEAR_BOX_MASK) == 0))
			{
				CTR_Box_DrawClearBox(&R233.introClearBoxRect, &R233.introClearBoxColor, 1, gGT->backBuffer->otMem.uiOT);
			}

			if (gGT->levelID == NAUGHTY_DOG_CRATE)
			{
				gGT->pushBuffer[0].distanceToScreen_PREV = CS_ND_CRATE_CAMERA_DISTANCE;
			}

			gGT->pushBuffer[0].distanceToScreen_CURR = gGT->pushBuffer[0].distanceToScreen_PREV;
		}

		if ((sdata->gGamepads->gamepad[0].buttonsTapped & BTN_START) != 0)
		{
			gGT->clockEffectEnabled &= ~CAM_PATH_FLAG_CLOCK_EFFECT;
			if ((u32)(gGT->levelID - CREDITS_CRASH) >= CS_CREDITS_LEVEL_COUNT)
			{
				if (gGT->levelID == NAUGHTY_DOG_CRATE)
				{
					if ((u32)gGT->msInThisLEV >> CS_FRAME32_SHIFT < CS_ND_CRATE_SKIP_MIN_FRAME32)
					{
						goto afterCameraAndSkipChecks;
					}
					RaceFlag_SetCanDraw(1);
					if (!RaceFlag_IsTransitioning() && !RaceFlag_IsFullyOnScreen())
					{
						RaceFlag_SetFullyOffScreen();
					}
				}
				else
				{
					RaceFlag_SetCanDraw(1);
					if (!RaceFlag_IsTransitioning() && !RaceFlag_IsFullyOnScreen())
					{
						RaceFlag_SetFullyOffScreen();
					}
					levelToLoad = CREDITS_CRASH;
					if ((u32)(gGT->levelID - OXIDE_ENDING) < CS_OXIDE_ENDING_LEVEL_COUNT)
					{
						goto requestSkipLevelLoad;
					}
				}
				CseqMusic_StopAll();
				CDSYS_XAPauseRequest();
				RaceFlag_SetDrawOrder(0);
				levelToLoad = MAIN_MENU_LEVEL;
			requestSkipLevelLoad:
				MainRaceTrack_RequestLoad(levelToLoad);
				D233.isCutsceneOver = 1;
				gGT->gameMode2 &= ~VEH_FREEZE_PODIUM;
				CS_RestoreDecodedOpcode(cs, metadataBackup);
				return 1;
			}
			CS_Credits_End();
		}
	}

afterCameraAndSkipChecks:
	opcodeChanged = 0;
	if (elapsedTimeRemaining == 0)
	{
	updateInstanceAndReturn:
		cs->animFrame32 = animFrame32;
		cs->animIndex = (char)animIndex;
		cs->lodIndex = (s16)lodIndexState;
		cs->opcodeDuration = (s16)opcodeDuration;
		int rotY = (int)opcodeMeta->rotStart;
		int animFrame = animFrame32 >> CS_FRAME32_SHIFT;
		if (rotY != (int)opcodeMeta->rotEnd)
		{
			int rotInterpStartFrame = opcodeMeta->arg0.i;
			if (opcodeMeta->arg1.i != rotInterpStartFrame)
			{
				int rotInterpNumerator = (((((int)opcodeMeta->rotEnd - rotY) + (u32)ANG_PI) & (ANG_TWO_PI - 1)) - ANG_PI) * (animFrame - rotInterpStartFrame);
				int rotInterpFrameRange = opcodeMeta->arg1.i - rotInterpStartFrame;
				if (rotInterpFrameRange < 0)
				{
					rotInterpFrameRange = -rotInterpFrameRange;
				}
				rotY = rotY + rotInterpNumerator / rotInterpFrameRange;
			}
		}
		rotY = rotY + cs->baseRotY;
		if ((rotY != (int)cs->rot.y) && (cs->rot.y = (s16)rotY, instance != 0))
		{
			ConvertRotToMatrix(&instance->matrix, &cs->rot);
		}
		int checkedAnimFrame = CS_Instance_SafeCheckAnimFrame(instance, animIndex, lodIndexState, animFrame);
		if (animFrame != checkedAnimFrame)
		{
			animIndex &= ~CS_ANIM_INDEX_LOW_BYTE_MASK;
			animFrame = 0;
		}
		if (instance != 0)
		{
			instance->animFrame = (s16)animFrame;
			instance->animIndex = (char)animIndex;
		}
		if (cs->frameOverrideRoot != 0)
		{
			frameData = &cs->frameOverrideRoot->data[animFrame];
			CTR_WriteU32LE((u8 *)&instance->matrix + 0x00, CTR_ReadU32LE(&frameData->rotScaleOrMatrix[0]));
			CTR_WriteU32LE((u8 *)&instance->matrix + 0x04, CTR_ReadU32LE(&frameData->rotScaleOrMatrix[2]));
			CTR_WriteU32LE((u8 *)&instance->matrix + 0x08, CTR_ReadU32LE(&frameData->rotScaleOrMatrix[4]));
			CTR_WriteU32LE((u8 *)&instance->matrix + 0x0c, CTR_ReadU32LE(&frameData->rotScaleOrMatrix[6]));
			CTR_WriteU32LE((u8 *)&instance->matrix + 0x10, CTR_ReadU32LE(&frameData->rotScaleOrMatrix[8]));
			instance->matrix.t[0] = frameData->offset[0];
			instance->matrix.t[1] = frameData->offset[1];
			instance->matrix.t[2] = frameData->offset[2];
		}
		return 0;
	}

processOpcode:
	switch (opcodeMeta->opcode)
	{
	case CS_OPCODE_ANIM_ROT_RANGE:
	case CS_OPCODE_ANIM_SYNC_MARKER:
	case CS_OPCODE_ANIM_RANGE:
		if (instance != 0)
		{
			cutsceneFlags = cs->flags;
			if ((cutsceneFlags & CS_FLAG_XA_SYNC_ANIMATION) != 0)
			{
				if (((cutsceneFlags & CS_FLAG_XA_PLAYBACK_STARTED) == 0) && (sdata->XA_State == XA_PLAYING))
				{
					cs->flags = cutsceneFlags | CS_FLAG_XA_PLAYBACK_STARTED;
				}
				if (sdata->XA_State != XA_IDLE)
				{
					if ((cs->flags & CS_FLAG_XA_PLAYBACK_STARTED) == 0)
					{
						animFrame32 = 0;
					}
					else
					{
						animFrame32 = CS_Instance_SafeCheckAnimFrame(instance, animIndex, lodIndexState,
						                                             (sdata->XA_CurrOffset * CS_XA_ANIM_SYNC_NUMERATOR) / CS_XA_ANIM_SYNC_DENOMINATOR);
						animFrame32 = animFrame32 << CS_FRAME32_SHIFT;
					}
					if (opcodeMeta->arg1.i << CS_FRAME32_SHIFT < animFrame32)
					{
						break;
					}
					goto updateInstanceAndReturn;
				}
				break;
			}
		}
		if (opcodeChanged != 0)
		{
			animIndex = (int)opcodeMeta->animIndex;
			animFrame32 = CS_Instance_SafeCheckAnimFrame(instance, animIndex, lodIndexState, opcodeMeta->arg0.i);
			animFrame32 = animFrame32 << CS_FRAME32_SHIFT;
			int rng = MixRNG_Scramble();
			opcodeChanged = 0;
			opcodeDuration =
			    ((int)(((rng >> CS_RANDOM_DURATION_SHIFT) & CS_RANDOM_DURATION_MASK) * (((int)opcodeMeta->frameEnd - (int)opcodeMeta->frameStart) + 1)) >>
			     FRACTIONAL_BITS) +
			    (int)opcodeMeta->frameStart;
		}
		frameBoundaryHit = 0;
		if (opcodeMeta->arg1.i < opcodeMeta->arg0.i)
		{
			int targetFrameTime = opcodeMeta->arg1.i * CS_FRAME32_UNIT;
			animFrame32 = animFrame32 - elapsedTimeRemaining;
			if (animFrame32 < targetFrameTime)
			{
				elapsedTimeRemaining = targetFrameTime - animFrame32;
			markAnimationBoundary:
				frameBoundaryHit = 1;
			}
		}
		else
		{
			int endFrame = CS_Instance_SafeCheckAnimFrame(instance, animIndex, lodIndexState, opcodeMeta->arg1.i);
			nextFrameTime = (endFrame + 1) * CS_FRAME32_UNIT;
			animFrame32 = animFrame32 + elapsedTimeRemaining;
			if (nextFrameTime <= animFrame32)
			{
				frameBoundaryHit = 1;
				elapsedTimeRemaining = 0;
				if (nextFrameTime != 0)
				{
					elapsedTimeRemaining = animFrame32 + (endFrame + 1) * -CS_FRAME32_UNIT;
					goto markAnimationBoundary;
				}
			}
		}
		if ((frameBoundaryHit) || (opcodeDuration < 1))
		{
			opcodeDuration = opcodeDuration + -1;
			if (opcodeDuration < 1)
			{
				CS_ScriptCmd_OpcodeNext(cs);
				opcodeChanged = 1;
			}
			else
			{
				animFrame32 = CS_Instance_SafeCheckAnimFrame(instance, animIndex, lodIndexState, opcodeMeta->arg0.i);
				animFrame32 = animFrame32 << CS_FRAME32_SHIFT;
			}
		}
		else
		{
			elapsedTimeRemaining = 0;
		}
		goto finishOpcodeStep;

	case CS_OPCODE_GOTO:
		opcodeChanged = 1;
		CS_ScriptCmd_OpcodeAt(cs, opcodeMeta->arg1.ptr);
		goto finishOpcodeStep;

	case CS_OPCODE_HIDE_INSTANCE_AND_END_THREAD:
		if (instance != 0)
		{
			instance->flags |= HIDE_MODEL;
		}
		CS_RestoreDecodedOpcode(cs, metadataBackup);
		return 1;

	case CS_OPCODE_SPAWN_CHILD:
		if (instance != 0)
		{
			// Retail builds this opcode 3 init data at scratchpad 0x1f800108.
			struct CsThreadInitData *initData = CTR_SCRATCHPAD_PTR(struct CsThreadInitData, 0x108);
			int spawnModelID = opcodeMeta->arg1.i;

			CS_Instance_GetFrameData(instance, (int)opcodeMeta->animIndex, opcodeMeta->arg0.i, &initData->podiumPos.vec, &initData->rot.vec, 0);

			initData->podiumPos.x += (s16)instance->matrix.t[0];
			initData->podiumPos.y += (s16)instance->matrix.t[1];
			initData->podiumPos.z += (s16)instance->matrix.t[2];
			initData->characterPos.x = 0;
			initData->characterPos.y = 0;
			initData->characterPos.z = 0;

			if (spawnModelID == NDI_BOX_PARTICLES_01)
			{
				initData->rot.x = 0;
				initData->rot.y = 0;
				initData->rot.z = 0;
			}

			CS_Thread_Init(spawnModelID, R233.s_spawn, initData, 0, instance->thread);
		}
		break;

	case CS_OPCODE_BRANCH_IF_RANDOM_LE:
	{
		int rng = MixRNG_Scramble();
		if (opcodeMeta->arg0.i < (int)((rng >> CS_RANDOM_DURATION_SHIFT) & CS_RANDOM_BYTE_MASK))
		{
			CS_ScriptCmd_OpcodeNext(cs);
		}
		else
		{
			CS_ScriptCmd_OpcodeAt(cs, opcodeMeta->arg1.ptr);
		}
		opcodeChanged = 1;
		goto finishOpcodeStep;
	}

	case CS_OPCODE_PLAY_CONTEXT_FX:
		if (gGT->levelID == ADVENTURE_GARAGE)
		{
			if (instance != 0)
			{
				Garage_PlayFX(opcodeMeta->arg1.u, (int)instance->model->id - STATIC_CRASHSELECT);
			}
		}
		else
		{
			if (CS_Instance_BoolPlaySound(cs, instance))
			{
				OtherFX_Play((u32)(u16)opcodeMetaShorts[6], 1);
			}
		}
		break;

	case CS_OPCODE_STOP_FX:
		OtherFX_Stop2((u32)(u16)opcodeMetaShorts[6]);
		break;

	case CS_OPCODE_START_MUSIC:
		CseqMusic_Start((u32)(u16)opcodeMetaShorts[6], 0, 0, 0, opcodeMeta->arg0.i);
		break;

	case CS_OPCODE_RESTART_MUSIC:
		CseqMusic_Restart((u32)(u16)opcodeMetaShorts[6], 1);
		break;

	case CS_OPCODE_SET_VISIBLE_LOD:
		if (instance != 0)
		{
			int numHeaders = (int)instance->model->numHeaders;
			if ((numHeaders != 0) && (modelHeader = instance->model->headers, modelHeader != 0))
			{
				lodIndex = opcodeMeta->arg1.i;
				lodIndexState = lodIndex;
				if (numHeaders <= lodIndex)
				{
					lodIndex = numHeaders + -1;
					lodIndexState = lodIndex;
				}
				while (lodIndex != 0)
				{
					modelHeader->maxDistanceLOD = 0;
					lodIndex = lodIndex + -1;
					modelHeader++;
				}
				modelHeader->maxDistanceLOD = CS_MODEL_LOD_DISTANCE_VISIBLE;
			}
		}
		break;

	case CS_OPCODE_SET_PATH_MOTION:
		if (opcodeMeta->arg1.i == -1)
		{
			cutsceneFlags = cs->flags | CS_FLAG_PATH_MOTION_DISABLED;
		}
		else
		{
			cs->pathProgress32 = 0;
			cutsceneFlags = cs->flags & ~CS_FLAG_PATH_MOTION_DISABLED;
		}
		cs->flags = cutsceneFlags;
		break;

	case CS_OPCODE_SCALE_TO:
		cs->desiredScale = opcodeMetaShorts[4];
		cs->scaleSpeed = opcodeMetaShorts[6];
		CS_ScriptCmd_OpcodeNext(cs);
		goto finishOpcodeStep;

	case CS_OPCODE_FADE_FROM_BLACK:
		gGT->pushBuffer[0].fadeFromBlack_currentValue = CS_FADE_FROM_BLACK_CURRENT;
		gGT->pushBuffer[0].fadeFromBlack_desiredResult = CS_FADE_FROM_BLACK_TARGET;
		gGT->pushBuffer[0].fade_step = CS_FADE_FROM_BLACK_STEP;
		CS_ScriptCmd_OpcodeNext(cs);
		goto finishOpcodeStep;

	case CS_OPCODE_SET_CUTSCENE_FLAGS:
		cutsceneFlags = cs->flags | opcodeMetaShorts[6];
		goto setFlagsAndAdvanceOpcode;

	case CS_OPCODE_CLEAR_CUTSCENE_FLAGS:
		cs->flags &= ~opcodeMetaShorts[6];
		CS_ScriptCmd_OpcodeNext(cs);
		goto finishOpcodeStep;

	case CS_OPCODE_RESELECT_LEVEL_SCRIPT:
	{
		gGT->bool_AdvHub_NeedToSwapLEV = 1;
		int scriptIndex;
		if ((gGT->gameMode2 & CREDITS) == 0)
		{
			cutsceneOpcodes = R233.introCutsceneOpcodes;
			scriptIndex = gGT->levelID - INTRO_RACE_TODAY;
		}
		else
		{
			cutsceneOpcodes = R233.creditsCutsceneOpcodes;
			scriptIndex = gGT->levelID - CREDITS_CRASH;
		}
		CS_ScriptCmd_OpcodeAt(cs, cutsceneOpcodes[scriptIndex]);
		goto updateInstanceAndReturn;
	}

	case CS_OPCODE_REQUEST_LEVEL:
	{
		int requestedLevelID = opcodeMeta->arg1.i;
		gGT->levelID = requestedLevelID;
		if (requestedLevelID == INTRO_RACE_TODAY)
		{
			RaceFlag_SetCanDraw(0);
		requestDirectLevelLoad:
			gGT->gameMode2 &= ~VEH_FREEZE_PODIUM;
			MainRaceTrack_RequestLoad((int)(s16)requestedLevelID);
		}
		else
		{
			if (requestedLevelID < INTRO_COCO)
			{
				if (requestedLevelID == GEM_STONE_VALLEY)
				{
					levelToLoad = GEM_STONE_VALLEY;
				requestMappedLevelLoad:
					gGT->gameMode2 &= ~VEH_FREEZE_PODIUM;
					MainRaceTrack_RequestLoad(levelToLoad);
					break;
				}
			}
			else
			{
				if (requestedLevelID == MAIN_MENU_LEVEL)
				{
					RaceFlag_SetDrawOrder(0);
					levelToLoad = MAIN_MENU_LEVEL;
					goto requestMappedLevelLoad;
				}
				if (requestedLevelID == CREDITS_CRASH)
				{
					goto requestDirectLevelLoad;
				}
			}
			D233.boolLoadNextSwap = 1;
			LOAD_Hub_ReadFile(sdata->ptrBigfileCdPos_2, requestedLevelID, 3 - (int)gGT->activeMempackIndex);
		}
		break;
	}

	case CS_OPCODE_WAIT_SWAP_QUEUE:
		if ((D233.boolLoadNextSwap == 0) || (sdata->queueReady == 0) || (sdata->queueLength != 0))
		{
			goto updateInstanceAndReturn;
		}
		break;

	case CS_OPCODE_PLAY_XA:
		CDSYS_XAPlay(opcodeMeta->arg0.i, opcodeMeta->arg1.i);
		if (sdata->XA_State != XA_IDLE)
		{
			cs->flags = (cs->flags & ~CS_FLAG_XA_PLAYBACK_STARTED) | CS_FLAG_XA_SYNC_ANIMATION;
		}
		break;

	case CS_OPCODE_WAIT_XA_DONE:
		if (sdata->XA_State == XA_IDLE)
		{
			cutsceneFlags = cs->flags & ~CS_FLAG_XA_SYNC_ANIMATION;
			goto setFlagsAndAdvanceOpcode;
		}

	case CS_OPCODE_YIELD:
		goto updateInstanceAndReturn;

	case CS_OPCODE_LOAD_LEVEL_STARS:
		numPlayers = gGT->numPlyrCurrGame;
		gGT->stars.numStars = (s16)((int)gGT->level1->stars.numStars / (int)(u32)numPlayers);
		gGT->stars.spread = gGT->level1->stars.spread;
		gGT->stars.seed = gGT->level1->stars.seed;
		gGT->stars.distance = gGT->level1->stars.distance;
		D233.boolLoadNextSwap = 0;
		CS_ScriptCmd_OpcodeNext(cs);
		goto finishOpcodeStep;

	case CS_OPCODE_RACEFLAG_TRANSITION1_IF_OFFSCREEN:
	{
		b32 raceFlagState = RaceFlag_IsFullyOffScreen();
		if (raceFlagState)
		{
			RaceFlag_SetCanDraw(1);
			RaceFlag_BeginTransition(1);
		}
		break;
	}

	case CS_OPCODE_ADVANCE_IF_RACEFLAG_ONSCREEN:
		conditionMet = RaceFlag_IsFullyOnScreen();
		goto advanceIfConditionMet;

	case CS_OPCODE_RACEFLAG_TRANSITION2_IF_ONSCREEN:
	{
		b32 raceFlagState = RaceFlag_IsFullyOnScreen();
		if (raceFlagState)
		{
			RaceFlag_BeginTransition(2);
		}
		break;
	}

	case CS_OPCODE_SET_PARTICLE_ID:
		cs->particleID = opcodeMetaShorts[6];
		CS_ScriptCmd_OpcodeNext(cs);
		goto finishOpcodeStep;

	case CS_OPCODE_HIDE_INSTANCE:
		if (instance != 0)
		{
			instance->flags |= HIDE_MODEL;
		}
		break;

	case CS_OPCODE_SHOW_INSTANCE:
		if (instance != 0)
		{
			instance->flags &= ~HIDE_MODEL;
		}
		break;

	case CS_OPCODE_ADD_INSTANCE_DEPTH_BIAS:
		if (instance != 0)
		{
			instance->depthBiasNormal += (char)opcodeMeta->arg1.i;
			instance->depthBiasSecondary += (char)opcodeMeta->arg1.i;
		}
		break;

	case CS_OPCODE_SET_INSTANCE_FLAGS:
		if (instance != 0)
		{
			instance->flags |= opcodeMeta->arg1.u;
		}
		break;

	case CS_OPCODE_CLEAR_INSTANCE_FLAGS:
		if (instance != 0)
		{
			instance->flags &= ~opcodeMeta->arg1.u;
		}
		break;

	case CS_OPCODE_SET_UNK4_1333:
		cs->unk4 = 0x1333;
		break;

	case CS_OPCODE_END_BOSS_CUTSCENE:
		D233.isCutsceneOver = 1;
		CS_DestroyPodium_StartDriving();
		D233.bossCutsceneIndex = -1;
		gGT->overlayTransition = CS_BOSS_END_OVERLAY_TRANSITION_FRAMES;
		gGT->gameMode2 &= ~VEH_FREEZE_PODIUM;
		CS_ScriptCmd_OpcodeNext(cs);
		goto finishOpcodeStep;

	case CS_OPCODE_SET_BOSS_CUTSCENE_INDEX:
		D233.bossCutsceneIndex = opcodeMeta->arg1.i;
		if ((D233.bossCutsceneIndex == 0) && (gGT->currAdvProfile.numRelics >= ADV_OXIDE_FINAL_RELIC_COUNT))
		{
			D233.bossCutsceneIndex = 9;
		}
		D233.cutsceneState = CS_WAIT_INPUT;
		break;

	case CS_OPCODE_SET_CAMERA_DISTANCE:
		distanceToScreen = opcodeMeta->arg1.i;
		gGT->pushBuffer[0].distanceToScreen_PREV = distanceToScreen;
		gGT->pushBuffer[0].distanceToScreen_CURR = distanceToScreen;
		cutsceneFlags = cs->flags | CS_FLAG_CAMERA_DISTANCE_OVERRIDE;
	setFlagsAndAdvanceOpcode:
		cs->flags = cutsceneFlags;
		CS_ScriptCmd_OpcodeNext(cs);
		goto finishOpcodeStep;

	case CS_OPCODE_ADVANCE_IF_CREDITS_TEXT_VALID:
		conditionMet = CS_Credits_IsTextValid();
		goto advanceIfConditionMet;

	case CS_OPCODE_CREDITS_DANCER:
	{
		// Retail builds this credits dancer init data at scratchpad 0x1f800108.
		struct CsThreadInitData *initData = CTR_SCRATCHPAD_PTR(struct CsThreadInitData, 0x108);
		int dancerModelID = opcodeMeta->arg1.i;

		initData->podiumPos.x = 0;
		initData->podiumPos.y = 0;
		initData->podiumPos.z = 0;
		initData->rot.x = 0;
		initData->rot.y = 0;
		initData->rot.z = 0;
		initData->characterPos.x = 0;
		initData->characterPos.y = 0;
		initData->characterPos.z = 0;

		gGT->podium_modelIndex_First = (u8)dancerModelID;
		gGT->podium_modelIndex_Second = 0;
		gGT->podium_modelIndex_Third = 0;

		if (dancerModelID == STATIC_OXIDEDANCE)
		{
			gGT->podium_modelIndex_First = 0;
			gGT->podium_modelIndex_Second = STATIC_OXIDEDANCE;
		}
		if (dancerModelID == STATIC_CRASHDANCE)
		{
			initData->rot.y += ANG_PI;
		}

		initData->rot.x += R233.creditsDancerRotOffset.x;
		initData->rot.y += R233.creditsDancerRotOffset.y;
		initData->rot.z += R233.creditsDancerRotOffset.z;

		dancerThread = (struct Thread *)CS_Thread_Init(dancerModelID, R233.s_g_dancer, initData, 0, 0);
		CS_Credits_NewDancer(dancerThread, (int)opcodeMetaShorts[6]);
	}
	break;

	case CS_OPCODE_ADVANCE_IF_CREDITS_GHOSTS_READY:
		conditionMet = CS_Credits_NewCreditGhosts();
	advanceIfConditionMet:
		conditionMet &= CS_CONDITION_RESULT_MASK;
		if (conditionMet == 0)
		{
			goto updateInstanceAndReturn;
		}
		break;

	case CS_OPCODE_BRANCH_ADV_CHAR_SELECT:
		if (opcodeMeta->frameEnd == 0)
		{
			if ((opcodeMeta->arg0.i != (int)gGarage.garageCharacterIDs[sdata->advCharSelectIndex_curr]) || (gGarage.boolSelected == 0))
			{
				opcodeAt = opcodeMeta->arg1.ptr;
			branchToGarageOpcode:
				opcodeChanged = 1;
				CS_ScriptCmd_OpcodeAt(cs, opcodeAt);
			}
		}
		else
		{
			if ((opcodeMeta->arg0.i == (int)gGarage.garageCharacterIDs[sdata->advCharSelectIndex_curr]) && (gGarage.boolSelected == 1))
			{
				opcodeAt = opcodeMeta->arg1.ptr;
				goto branchToGarageOpcode;
			}
		}
		break;

	case CS_OPCODE_ADVANCE_IF_LEVEL_TIME_REACHED:
		if ((u32)gGT->msInThisLEV >> CS_FRAME32_SHIFT < opcodeMeta->arg1.u)
		{
			goto updateInstanceAndReturn;
		}
		break;

	case CS_OPCODE_SYNC_ANIM_FRAME:
		animFrame32 = CS_Instance_SafeCheckAnimFrame(instance, animIndex, lodIndexState, animFrame32 >> CS_FRAME32_SHIFT);
		animFrame32 = animFrame32 << CS_FRAME32_SHIFT;
		goto updateInstanceAndReturn;

	case CS_OPCODE_END_CREDITS:
		CS_Credits_End();
		CS_RestoreDecodedOpcode(cs, metadataBackup);
		return 1;

	case CS_OPCODE_SET_GAME_MODE_FLAGS:
		gameModeTarget = opcodeMeta->animIndex;
		if (gameModeTarget == CS_GAME_MODE_TARGET_GAME_MODE2)
		{
			gGT->gameMode2 |= opcodeMeta->arg1.u;
		}
		else
		{
			if (gameModeTarget < CS_GAME_MODE_TARGET_RENDER_FLAGS_SET)
			{
				if (gameModeTarget == CS_GAME_MODE_TARGET_GAME_MODE1)
				{
					gGT->gameMode1 |= opcodeMeta->arg1.u;
				}
			}
			else
			{
				if (gameModeTarget == CS_GAME_MODE_TARGET_RENDER_FLAGS_SET)
				{
					gGT->renderFlags |= opcodeMeta->arg1.u;
				}
				else
				{
					if (gameModeTarget == CS_GAME_MODE_TARGET_RENDER_FLAGS_CLEAR)
					{
						gGT->renderFlags &= ~opcodeMeta->arg1.u;
					}
				}
			}
		}
		break;

	case CS_OPCODE_SET_SUBTITLE:
		cs->Subtitles.textPos.x = opcodeMeta->animIndex;
		cs->Subtitles.textPos.y = opcodeMeta->frameStart;
		cs->Subtitles.lngIndex = opcodeMeta->frameEnd;
		cs->Subtitles.font = opcodeMeta->rotStart;
		cs->Subtitles.colors = opcodeMeta->rotEnd;
		CS_ScriptCmd_OpcodeNext(cs);
		goto finishOpcodeStep;

	case CS_OPCODE_UI_FADE_TO_BLACK:
		gGT->pushBuffer_UI.fadeFromBlack_desiredResult = 0;
		gGT->pushBuffer_UI.fade_step = CS_FADE_FROM_BLACK_STEP;
		CS_ScriptCmd_OpcodeNext(cs);
		goto finishOpcodeStep;

	case CS_OPCODE_WAIT_UI_FADE:
		if (0 < gGT->pushBuffer_UI.fadeFromBlack_currentValue)
		{
			goto updateInstanceAndReturn;
		}
		break;

	case CS_OPCODE_SET_AUDIO_VOLUME:
	{
		const u8 *opcodeBytes = (const u8 *)opcodeMeta;

		D233.CutsceneManipulatesAudio = 1;
		howl_VolumeSet(HOWL_VOLUME_TYPE_FX, opcodeBytes[CS_AUDIO_VOLUME_FX_BYTE_OFFSET]);
		howl_VolumeSet(HOWL_VOLUME_TYPE_MUSIC, opcodeBytes[CS_AUDIO_VOLUME_MUSIC_BYTE_OFFSET]);
		howl_VolumeSet(HOWL_VOLUME_TYPE_VOICE, opcodeBytes[CS_AUDIO_VOLUME_VOICE_BYTE_OFFSET]);
		break;
	}

	default:
		CS_RestoreDecodedOpcode(cs, metadataBackup);
		return 0;
	}

	CS_ScriptCmd_OpcodeNext(cs);

finishOpcodeStep:
	if ((elapsedTimeRemaining != 0) || (opcodeChanged != 0))
	{
		goto processOpcode;
	}
	goto updateInstanceAndReturn;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ae2b8-0x800ae318
void CS_Thread_AnimateScale(struct Thread *t)
{
	struct Instance *inst = t->inst;
	struct CutsceneObj *cs = t->object;

	if (!inst)
	{
		return;
	}

	if (cs->scaleSpeed == 0)
	{
		return;
	}

	int newScale = (int)inst->scale.x + (int)cs->scaleSpeed;
	int desiredScale = (int)cs->desiredScale;

	if (cs->scaleSpeed > 0)
	{
		if (newScale >= desiredScale)
		{
			newScale = desiredScale;
			cs->scaleSpeed = 0;
		}
	}
	else
	{
		if (newScale <= desiredScale)
		{
			newScale = desiredScale;
			cs->scaleSpeed = 0;
		}
	}

	inst->scale.x = (s16)newScale;
	inst->scale.y = (s16)newScale;
	inst->scale.z = (s16)newScale;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ade8c-0x800ae2b8
void CS_Thread_MoveOnPath(struct Thread *t)
{
	struct CutsceneObj *cs = t->object;
	struct Instance *inst = t->inst;
	struct Level *level;
	struct GameTracker *gGT;
	s16 modelID;
	int pathModelKind;
	int pathIndex;
	struct SpawnType2 *spawnEntry;
	SVec3 *pathPoints;
	SVec3 *currPoint;
	SVec3 *nextPoint;
	struct SpawnPosRot *posRot;
	u16 pathFrame32;
	int segmentIndex;
	u16 segmentFrac32;
	SVec3 rot;

	if ((cs->flags & CS_FLAG_PATH_MOTION_DISABLED) != 0)
	{
		return;
	}

	if (inst == 0)
	{
		return;
	}

	modelID = inst->model->id;
	pathModelKind = (s16)(modelID - STATIC_PPOINTTHINGINTRO);

	if ((u32)pathModelKind >= CS_PATH_MODEL_KIND_COUNT)
	{
		return;
	}

	gGT = sdata->gGT;
	level = gGT->level1;

	switch (pathModelKind)
	{
	case CS_PATH_MODEL_PPOINT_THING_INTRO:
	case CS_PATH_MODEL_OXIDE_SPEAKER:

		pathIndex = (u8)inst->name[strlen(inst->name) - 1] - '0';

		if (level->numSpawnType2 <= pathIndex)
		{
			return;
		}

		spawnEntry = &level->ptrSpawnType2[pathIndex];
		pathPoints = spawnEntry->positions;

		if (pathPoints == 0)
		{
			return;
		}

		pathFrame32 = cs->pathProgress32;
		segmentIndex = (s16)pathFrame32 >> CS_FRAME32_SHIFT;
		cs->pathProgress32 = (u16)(pathFrame32 + (u16)gGT->elapsedTimeMS);
		segmentFrac32 = pathFrame32 & CS_FRAME32_MASK;

		if (segmentIndex >= spawnEntry->numCoords - 1)
		{
			segmentIndex = 0;

			if (modelID == STATIC_OXIDESPEAKER)
			{
				segmentIndex = spawnEntry->numCoords - 2;
				cs->pathProgress32 = segmentIndex << CS_FRAME32_SHIFT;
			}
			else
			{
				cs->pathProgress32 = 0;
			}
		}

		currPoint = &pathPoints[segmentIndex];
		nextPoint = &currPoint[1];

		inst->matrix.t[0] = currPoint->x + ((segmentFrac32 * (nextPoint->x - currPoint->x)) >> CS_FRAME32_SHIFT);
		inst->matrix.t[1] = currPoint->y + ((segmentFrac32 * (nextPoint->y - currPoint->y)) >> CS_FRAME32_SHIFT);
		inst->matrix.t[2] = currPoint->z + ((segmentFrac32 * (nextPoint->z - currPoint->z)) >> CS_FRAME32_SHIFT);

		if (segmentIndex >= spawnEntry->numCoords - 1)
		{
			return;
		}

		if (modelID == STATIC_OXIDESPEAKER)
		{
			return;
		}

		rot.x = cs->rot.x;
		rot.y = cs->rot.y + ratan2(nextPoint->x - currPoint->x, nextPoint->z - currPoint->z);
		rot.z = cs->rot.z;

		ConvertRotToMatrix(&inst->matrix, &rot);
		return;

	case CS_PATH_MODEL_PR_THING_INTRO:
	case CS_PATH_MODEL_OXIDE_LIL_SHIP:
	case CS_PATH_MODEL_END_OXIDE_BIG_SHIP:
	case CS_PATH_MODEL_END_OXIDE_LIL_SHIP:

		pathIndex = (u8)inst->name[strlen(inst->name) - 1] - '0';

		if (level->numSpawnType2_PosRot <= pathIndex)
		{
			return;
		}

		spawnEntry = &level->ptrSpawnType2_PosRot[pathIndex];
		posRot = spawnEntry->posRot;

		if (posRot == 0)
		{
			return;
		}

		pathFrame32 = cs->pathProgress32;
		cs->pathProgress32 = (u16)(pathFrame32 + (u16)gGT->elapsedTimeMS);
		segmentIndex = (s16)pathFrame32 >> CS_FRAME32_SHIFT;

		if (segmentIndex >= spawnEntry->numCoords - 1)
		{
			segmentIndex = 0;
			cs->pathProgress32 = 0;
		}

		{
			struct SpawnPosRot *frame = &posRot[segmentIndex];

			inst->matrix.t[0] = frame->pos.x;
			inst->matrix.t[1] = frame->pos.y;
			inst->matrix.t[2] = frame->pos.z;

			rot = frame->rot;
		}

		break;

	case CS_PATH_MODEL_COCO_SELECT:

		if (level->numSpawnType2 <= 0)
		{
			return;
		}

		spawnEntry = level->ptrSpawnType2;
		pathPoints = spawnEntry->positions;

		if (pathPoints == 0)
		{
			return;
		}

		{
			int prog = 0;

			if (cs->animIndex == 3)
			{
				prog = cs->animFrame32;
			}

			segmentFrac32 = prog & CS_FRAME32_MASK;
			int numCoords = spawnEntry->numCoords;
			segmentIndex = prog >> CS_FRAME32_SHIFT;

			if (segmentIndex < numCoords - 1)
			{
				if (segmentIndex >= 0)
				{
					currPoint = &pathPoints[segmentIndex];
					nextPoint = &currPoint[1];
				}
				else
				{
					currPoint = &pathPoints[0];
					nextPoint = currPoint;
				}
			}
			else
			{
				currPoint = &pathPoints[numCoords - 1];
				nextPoint = currPoint;
			}

			inst->matrix.t[0] = currPoint->x + ((segmentFrac32 * (nextPoint->x - currPoint->x)) >> CS_FRAME32_SHIFT);
			inst->matrix.t[1] = currPoint->y + ((segmentFrac32 * (nextPoint->y - currPoint->y)) >> CS_FRAME32_SHIFT);
			inst->matrix.t[2] = currPoint->z + ((segmentFrac32 * (nextPoint->z - currPoint->z)) >> CS_FRAME32_SHIFT);
		}

		return;

	default:
		return;
	}

	ConvertRotToMatrix(&inst->matrix, &rot);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800abdd4-0x800abf70
void CS_Thread_Particles(struct Thread *t)
{
	struct CutsceneObj *cs = t->object;
	struct Instance *inst = t->inst;
	const struct CsParticleConfig *entry;
	s8 particleID;

	if (inst == NULL)
	{
		return;
	}

	if ((inst->flags & HIDE_MODEL) != 0)
	{
		return;
	}

	particleID = cs->particleID;
	if ((u8)particleID >= 9)
	{
		return;
	}

	entry = &R233.particleConfigs[(int)particleID];

	while (1)
	{
		int iconGroupIndex = entry->meta.iconGroupIndex;
		int frameOffset = entry->meta.frameOffset;
		int count = entry->meta.count;
		int flags = entry->meta.flags;
		s8 modelDelta = entry->spawn.modelDelta;

		for (int i = 0; i < count; i++)
		{
			struct Particle *p = Particle_Init(0, sdata->gGT->iconGroup[iconGroupIndex], entry->emitter);

			if (p != NULL)
			{
				SVec3 pos;

				CS_Instance_GetFrameData(inst, inst->animIndex, inst->animFrame, &pos, NULL, frameOffset);

				p->axis[0].startVal += (pos.x + inst->matrix.t[0]) << 8;
				p->axis[1].startVal += (pos.y + inst->matrix.t[1]) << 8;
				p->axis[2].startVal += (pos.z + inst->matrix.t[2]) << 8;
				p->otIndexOffset = inst->depthBiasNormal + modelDelta;
			}
		}

		if ((flags & 1) == 0)
		{
			break;
		}

		entry++;
	}
}

struct CSInterpolateLinePacket
{
	u32 tag;
	u32 drawMode;
	u32 pad;
	u32 colorAndCode;
	u32 xy0;
	u32 xy1;
};

CTR_STATIC_ASSERT(sizeof(struct CSInterpolateLinePacket) == 0x18);
CTR_STATIC_ASSERT(offsetof(struct CSInterpolateLinePacket, tag) == 0x00);
CTR_STATIC_ASSERT(offsetof(struct CSInterpolateLinePacket, drawMode) == 0x04);
CTR_STATIC_ASSERT(offsetof(struct CSInterpolateLinePacket, pad) == 0x08);
CTR_STATIC_ASSERT(offsetof(struct CSInterpolateLinePacket, colorAndCode) == 0x0C);
CTR_STATIC_ASSERT(offsetof(struct CSInterpolateLinePacket, xy0) == 0x10);
CTR_STATIC_ASSERT(offsetof(struct CSInterpolateLinePacket, xy1) == 0x14);

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ae318-0x800ae54c
void CS_Thread_InterpolateFramesMS(struct Thread *t)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Instance *inst = t->inst;
	struct PrimMem *primMem;
	struct CSInterpolateLinePacket *packet;
	void *end;
	SVec3 curr;
	SVec3 next;
	int depth;

	CS_Instance_GetFrameData(inst, inst->animIndex, inst->animFrame, &curr, NULL, 0);
	CS_Instance_GetFrameData(inst, inst->animIndex, inst->animFrame, &next, NULL, 1);

	curr.x = (s16)((u16)curr.x + (u16)inst->matrix.t[0]);
	curr.y = (s16)((u16)curr.y + (u16)inst->matrix.t[1]);
	curr.z = (s16)((u16)curr.z + (u16)inst->matrix.t[2]);

	next.x = (s16)((u16)next.x + (u16)inst->matrix.t[0]);
	next.y = (s16)((u16)next.y + (u16)inst->matrix.t[1]);
	next.z = (s16)((u16)next.z + (u16)inst->matrix.t[2]);

	primMem = &gGT->backBuffer->primMem;
	packet = primMem->cursor;
	end = primMem->guardEnd;

	if ((uintptr_t)(packet + 1) >= (uintptr_t)end)
	{
		return;
	}

	gte_SetRotMatrix(&gGT->pushBuffer[0].matrix_ViewProj);
	gte_SetTransMatrix(&gGT->pushBuffer[0].matrix_ViewProj);

	MTC2(CTR_PackS16Pair(curr.x, curr.y), 0);
	MTC2(CTR_PackS16Pair(curr.z, 0), 1);
	MTC2(CTR_PackS16Pair(next.x, next.y), 2);
	MTC2(CTR_PackS16Pair(next.z, 0), 3);
	gte_rtpt();

	packet->xy0 = MFC2(12);
	packet->xy1 = MFC2(13);

	depth = MFC2(17);
	if ((u32)(depth - CS_INTERPOLATE_LINE_DEPTH_MIN) < CS_INTERPOLATE_LINE_DEPTH_RANGE)
	{
		u32 color = CS_INTERPOLATE_LINE_MAX_COLOR;
		int otIndex;
		u32 *ot;

		packet->drawMode = CS_INTERPOLATE_LINE_DRAW_MODE;
		packet->pad = 0;

		if (depth > CS_INTERPOLATE_LINE_FADE_START_DEPTH)
		{
			int fade = (CS_INTERPOLATE_LINE_FADE_END_DEPTH - depth) * CS_INTERPOLATE_LINE_MAX_COLOR;

			color = fade >> CS_INTERPOLATE_LINE_FADE_SHIFT;
			if (fade < 0)
			{
				color = (fade + ((1 << CS_INTERPOLATE_LINE_FADE_SHIFT) - 1)) >> CS_INTERPOLATE_LINE_FADE_SHIFT;
			}
		}

		packet->colorAndCode = color | (color << 8) | (color << 16) | CS_INTERPOLATE_LINE_COLOR_CODE;

		otIndex = depth >> CS_INTERPOLATE_LINE_OT_SHIFT;
		if (otIndex > CS_INTERPOLATE_LINE_MAX_OT_INDEX)
		{
			otIndex = CS_INTERPOLATE_LINE_MAX_OT_INDEX;
		}

		ot = (u32 *)&gGT->pushBuffer[0].ptrOT[otIndex];
		packet->tag = CtrGpu_PackOTTag(*ot, CS_INTERPOLATE_LINE_OT_TAG);
		*ot = CtrGpu_PrimToOTLink24(packet);
		packet++;
	}

	primMem->cursor = packet;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b06ac-0x800b087c
void CS_Thread_LInB(struct Instance *inst)
{
	struct Thread *t;
	struct CutsceneObj *cs;
	s16 modelID;
	char *scriptPtr;

	D233.isCutsceneOver = 0;

	if (inst->thread != 0)
	{
		goto check_polar;
	}

	t = PROC_BirthWithObject(SIZE_RELATIVE_POOL_BUCKET(sizeof(struct CutsceneObj), NONE, MEDIUM, STATIC), CS_Thread_ThTick, R233.s_introguy, 0);

	inst->thread = t;

	if (t == 0)
	{
		return;
	}

	cs = t->object;

	t->inst = inst;

	cs->metadataMeta = &cs->decodedOpcode;
	cs->prevOpcode = (char *)-1;
	cs->Subtitles.lngIndex = -1;

	modelID = inst->model->id;

	if (modelID < NDI_BOX_BOX_01)
	{
		if ((u16)(modelID - STATIC_CRASHINTRO) < CS_INTRO_MODEL_SCRIPT_COUNT)
		{
			scriptPtr = R233.introModelScripts[modelID - STATIC_CRASHINTRO];
		}
		else
		{
			scriptPtr = (char *)R233.script_default;
		}
	}
	else
	{
		scriptPtr = R233.boxModelScripts[modelID - NDI_BOX_BOX_01];
	}

	CS_ScriptCmd_OpcodeAt(cs, scriptPtr);

	cs->animFrame32 = cs->metadata[2];

	{
		int rng = MixRNG_Scramble();
		s16 *meta = cs->metadataShorts;
		s16 frameStart = meta[2];
		s16 frameEnd = meta[3];

		cs->baseRotY = 0;
		cs->rot.x = 0;
		cs->rot.y = 0;
		cs->rot.z = 0;
		cs->pathProgress32 = 0;
		cs->lodIndex = 0;
		cs->flags = 0;
		cs->scaleSpeed = 0;
		cs->frameOverrideRoot = 0;
		cs->desiredScale = CS_DEFAULT_SCALE;
		cs->particleID = CS_DEFAULT_PARTICLE_ID;

		cs->opcodeDuration =
		    frameStart + (s16)((((rng >> CS_RANDOM_DURATION_SHIFT) & CS_RANDOM_DURATION_MASK) * ((frameEnd - frameStart) + 1)) >> FRACTIONAL_BITS);

		struct GameTracker *gGT = sdata->gGT;

		cs->unk4 = 0;
		cs->unk6 = 0;
		cs->unk8 = CS_DEFAULT_COLOR_WORD;
		cs->unk_C = 0;
		cs->unk_E = 0;

		cs->ptrIcons = (struct IconGroup *)((char *)gGT->iconGroup[0] + sizeof(struct IconGroup));
	}

check_polar:
	if (sdata->gGT->levelID == INTRO_POLAR)
	{
		inst->vertSplit = 0;
		inst->flags |= REFLECTIVE;
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ae54c-0x800ae81c
void CS_Thread_ThTick(struct Thread *t)
{
	// Retail uses scratchpad 0x1f800108/0x1f800118 for parent frame-data temporaries.
	struct CSThreadParentFrameScratch *parentFrame = CTR_SCRATCHPAD_PTR(struct CSThreadParentFrameScratch, 0x108);
	SVec3 bonePos;
	struct CutsceneObj *cs = t->object;
	struct Instance *inst = t->inst;
	struct Instance *parentInst;
	struct Thread *parentThread;

	if (CS_Thread_UseOpcode(inst, cs))
	{
		t->flags |= THREAD_FLAG_DEAD;

		if ((sdata->gGT->gameMode2 & CREDITS) != 0)
		{
			return;
		}
	}

	CS_Thread_MoveOnPath(t);
	CS_Thread_AnimateScale(t);
	CS_Thread_Particles(t);

	if ((cs->flags & CS_FLAG_INTERPOLATE_FRAMES_MS) != 0)
	{
		CS_Thread_InterpolateFramesMS(t);
	}

	// ASM: 0x800ae5dc - parent-thread frameOverrideRoot processing
	if (inst != 0)
	{
		parentThread = t->parentThread;

		if (parentThread != 0)
		{
			if ((cs->flags & CS_FLAG_SKIP_PARENT_FRAME_TRANSFORM) == 0)
			{
				parentInst = parentThread->inst;

				CS_Instance_GetFrameData(parentInst, parentInst->animIndex, parentInst->animFrame, &parentFrame->parentPos.vec, &parentFrame->parentRot.vec, 0);

				inst->matrix.t[0] = parentInst->matrix.t[0] + parentFrame->parentPos.x;
				inst->matrix.t[1] = parentInst->matrix.t[1] + parentFrame->parentPos.y;
				inst->matrix.t[2] = parentInst->matrix.t[2] + parentFrame->parentPos.z;

				if ((cs->flags & CS_FLAG_SKIP_PARENT_ROTATION) == 0)
				{
					ConvertRotToMatrix(&inst->matrix, &parentFrame->parentRot.vec);
				}
			}
		}

		inst = t->inst;
		if (inst == 0)
		{
			goto thTick_subtitles;
		}

		// ASM: 0x800ae6b4 - CS_FLAG_WRITE_VERT_SPLIT_LINE writes bone Y to overlay-233 mutable state.
		if ((cs->flags & CS_FLAG_WRITE_VERT_SPLIT_LINE) != 0)
		{
			CS_Instance_GetFrameData(inst, inst->animIndex, inst->animFrame, &bonePos, 0, 0);

			D233.VertSplitLine = bonePos.y;

			inst = t->inst;
			if (inst == 0)
			{
				goto thTick_subtitles;
			}
		}

		// ASM: 0x800ae6fc - CS_FLAG_RANDOM_ALPHA_SCALE jitters alphaScale for fade effect.
		if ((cs->flags & CS_FLAG_RANDOM_ALPHA_SCALE) != 0)
		{
			inst->alphaScale = 0;

			if ((sdata->gGT->timer & 0x1) != 0)
			{
				inst->alphaScale = (MixRNG_Scramble() & CS_RANDOM_ALPHA_MASK) + CS_RANDOM_ALPHA_BASE;
			}
		}
	}

	// ASM: 0x800ae744 - subtitle rendering
thTick_subtitles:
	if (cs->Subtitles.lngIndex > 0)
	{
		struct GameTracker *gGT = sdata->gGT;
		int textHeight;
		RECT textRect;

		textHeight = DecalFont_DrawMultiLine(sdata->lngStrings[cs->Subtitles.lngIndex], cs->Subtitles.textPos.x, cs->Subtitles.textPos.y,
		                                     CS_SUBTITLE_TEXT_MAX_WIDTH, cs->Subtitles.font, cs->Subtitles.colors);

		textRect.x = (s16)((u16)cs->Subtitles.textPos.x - CS_SUBTITLE_BOX_HALF_WIDTH);
		textRect.y = (s16)((u16)cs->Subtitles.textPos.y - CS_SUBTITLE_BOX_Y_OFFSET);
		textRect.w = CS_SUBTITLE_BOX_WIDTH;
		textRect.h = (s16)textHeight + CS_SUBTITLE_BOX_HEIGHT_PADDING;

		RECTMENU_DrawInnerRect(&textRect, CS_SUBTITLE_BOX_DRAW_STYLE, gGT->backBuffer->otMem.uiOT);
	}

	// ASM: 0x800ae7dc - check isCutsceneOver, re-apply death flag
	if (D233.isCutsceneOver != 0)
	{
		t->flags |= THREAD_FLAG_DEAD;
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800af328-0x800af7c0
struct Thread *CS_Thread_Init(s16 modelID, const char *name, struct CsThreadInitData *initData, s16 yawOffset, struct Thread *parent)
{
	struct GameTracker *gGT = sdata->gGT;
	struct CutsceneObj *cs;
	struct Instance *inst;
	struct Thread *t;
	char *scriptPtr;
	u32 bucket;
	s16 *meta;

	if (modelID == NOFUNC)
	{
		inst = NULL;

		t = PROC_BirthWithObject(SIZE_RELATIVE_POOL_BUCKET(sizeof(struct CutsceneObj), NONE, MEDIUM, CAMERA), CS_Thread_ThTick, name, parent);

		if (t == NULL)
		{
			return NULL;
		}
	}
	else
	{
		bucket = OTHER;

		if ((u32)(modelID - NDI_KART6) < CS_MASK_KART_MODEL_COUNT)
		{
			bucket = AKUAKU;
		}

		if ((u32)(modelID - NDI_KART0) < CS_KART_FRAME_OVERRIDE_COUNT)
		{
			bucket = GHOST;
		}

		inst = INSTANCE_BirthWithThread(modelID, name, MEDIUM, bucket, CS_Thread_ThTick, sizeof(struct CutsceneObj), parent);

		if (inst == NULL)
		{
			return NULL;
		}

		t = inst->thread;
		t->funcThDestroy = PROC_DestroyInstance;
	}

	cs = t->object;

	cs->metadataMeta = &cs->decodedOpcode;
	cs->frameOverrideRoot = NULL;
	cs->prevOpcode = (char *)-1;
	cs->Subtitles.lngIndex = -1;

	if (modelID == NOFUNC)
	{
		int level = gGT->levelID;

		if (level == NAUGHTY_DOG_CRATE)
		{
			scriptPtr = (char *)&R233.creditsOpcodeData[CS_CREDITS_ND_CRATE_SCRIPT_OFFSET];
		}
		else if (level == OXIDE_ENDING)
		{
			scriptPtr = (char *)&R233.introEndingOpcodeData[0];
		}
		else if (level == OXIDE_TRUE_ENDING)
		{
			scriptPtr = (char *)&R233.introEndingOpcodeData[CS_TRUE_ENDING_SCRIPT_OFFSET];
		}
		else if ((gGT->gameMode2 & CREDITS) == 0)
		{
			scriptPtr = R233.introCutsceneOpcodes[level - INTRO_RACE_TODAY];
		}
		else
		{
			scriptPtr = R233.creditsCutsceneOpcodes[level - CREDITS_CRASH];
		}
	}
	else
	{
		if (modelID >= NDI_BOX_BOX_01)
		{
			if ((u32)(modelID - NDI_BOX_BOX_01) < CS_BOX_MODEL_SCRIPT_COUNT)
			{
				scriptPtr = R233.boxModelScripts[modelID - NDI_BOX_BOX_01];
			}
			else
			{
				scriptPtr = (char *)R233.script_default;
			}

			CS_ScriptCmd_OpcodeAt(cs, scriptPtr);

			if ((u32)(modelID - NDI_KART0) < CS_KART_FRAME_OVERRIDE_COUNT)
			{
				cs->frameOverrideRoot = &D233.cs_initMatrixTable[modelID - NDI_KART0];
			}

			goto after_opcode;
		}

		if ((u32)(modelID - STATIC_PINHEAD) < CS_PINHEAD_DEFAULT_SCRIPT_COUNT)
		{
			scriptPtr = (char *)R233.script_default;
		}
		else if (modelID == STATIC_DINGOFIRE)
		{
			scriptPtr = (char *)R233.script_dingofire;
		}
		else if ((u32)(modelID - STATIC_TAWNA1) < CS_TAWNA_MODEL_SCRIPT_COUNT)
		{
			if (gGT->gameMode2 & CREDITS)
			{
				scriptPtr = (char *)R233.script_tawnaCredits;
			}
			else
			{
				scriptPtr = (char *)R233.script_tawnaNormal;
			}
		}
		else if ((u32)(modelID - STATIC_CRASHDANCE) < CS_DANCE_MODEL_SCRIPT_COUNT)
		{
			char *const *base;
			int off = (modelID - STATIC_CRASHDANCE);

			if (modelID == gGT->podium_modelIndex_First)
			{
				base = R233.danceFirstScripts;
			}
			else
			{
				base = R233.danceOtherScripts;
			}

			scriptPtr = base[off];
		}
		else
		{
			scriptPtr = (char *)R233.script_default;
		}
	}

	CS_ScriptCmd_OpcodeAt(cs, scriptPtr);

after_opcode:

	cs->animFrame32 = cs->metadata[2];

	meta = cs->metadataShorts;
	cs->opcodeDuration =
	    meta[2] + (s16)((((MixRNG_Scramble() >> CS_RANDOM_DURATION_SHIFT) & CS_RANDOM_DURATION_MASK) * ((meta[3] - meta[2]) + 1)) >> FRACTIONAL_BITS);

	if (inst != NULL)
	{
		s32 transformedCharacterPos[3];

		MTC2(CTR_PackS16Pair(initData->characterPos.x, initData->characterPos.y), 0);
		MTC2(CTR_PackS16Pair(initData->characterPos.z, initData->characterPos.w), 1);
		gte_llv0();

		CTR_GteStoreMAC(transformedCharacterPos);

		inst->matrix.t[0] = transformedCharacterPos[0] + initData->podiumPos.x;
		inst->matrix.t[1] = transformedCharacterPos[1] + initData->podiumPos.y;
		inst->matrix.t[2] = transformedCharacterPos[2] + initData->podiumPos.z;

		if (gGT->levelID != NAUGHTY_DOG_CRATE)
		{
			inst->scale.x = CS_PODIUM_CHARACTER_SCALE;
			inst->scale.y = CS_PODIUM_CHARACTER_SCALE;
			inst->scale.z = CS_PODIUM_CHARACTER_SCALE;
		}

		if ((u32)(gGT->levelID - GEM_STONE_VALLEY) < CS_ADV_HUB_LEVEL_COUNT)
		{
			inst->depthBiasNormal -= 4;
			inst->depthBiasSecondary -= 4;
		}

		initData->derivedRot.x = initData->rot.x;
		initData->derivedRot.z = initData->rot.z;
		initData->derivedRot.y = initData->rot.y + yawOffset;

		ConvertRotToMatrix(&inst->matrix, &initData->derivedRot.vec);

		cs->baseRotY = ANG_MODULO_TWO_PI(initData->derivedRot.y);
		cs->rot.x = ANG_MODULO_TWO_PI(initData->derivedRot.x);
		cs->rot.y = ANG_MODULO_TWO_PI(initData->derivedRot.y);
		cs->rot.z = ANG_MODULO_TWO_PI(initData->derivedRot.z);
	}

	cs->particleID = CS_DEFAULT_PARTICLE_ID;
	cs->pathProgress32 = 0;
	cs->lodIndex = 0;
	cs->flags = 0;
	cs->scaleSpeed = 0;
	cs->desiredScale = CS_PODIUM_CHARACTER_SCALE;

	cs->unk4 = 0;
	cs->unk6 = 0;
	cs->unk8 = CS_DEFAULT_COLOR_WORD;
	cs->unk_C = 0;
	cs->unk_E = 0;

	cs->ptrIcons = (struct IconGroup *)((char *)gGT->iconGroup[0] + sizeof(struct IconGroup));

	return t;
}
