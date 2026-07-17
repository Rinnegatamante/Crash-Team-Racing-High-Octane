#include <common.h>

enum CreditsRuntimeConstants
{
	CS_CREDITS_RELIC_TRACK_COUNT = ADV_REWARD_RELIC_TRACK_COUNT,
	CS_CREDITS_THREAD_FLAGS = SMALL | OTHER,
	CS_CREDITS_CONFETTI_PARTICLE_COUNT = 250,
	CS_CREDITS_COUNTDOWN_FRAMES = 360,
	CS_CREDITS_TEXT_START_STRING_INDEX = 0x14,
	CS_CREDITS_LINE_HEIGHT = 20,
	CS_CREDITS_NAME_START_Y = 340,
	CS_CREDITS_NAME_END_Y = 0x114,
	CS_CREDITS_NAME_FADE_END_START_Y = 0x83,
	CS_CREDITS_NAME_FADE_BOTTOM_Y = 0x96,
	CS_CREDITS_NAME_DIRECTIVE_STRIDE = 3,
	CS_CREDITS_NAME_DIRECTIVE_DIGIT_BASE = '0',
	CS_CREDITS_NAME_DIRECTIVE_COLOR_LIMIT = 50,
	CS_CREDITS_NAME_DIRECTIVE_STYLE_52 = 52,
	CS_CREDITS_NAME_DIRECTIVE_STYLE_53 = 53,
	CS_CREDITS_NAME_DIRECTIVE_STYLE_52_FLAG = 0x2000,
	CS_CREDITS_NAME_DIRECTIVE_STYLE_53_FLAG = 0x1000,
	CS_CREDITS_EPILOGUE_DURATION_FRAMES = 200,
	CS_CREDITS_EPILOGUE_FADE_OUT_START_FRAMES = 0xb5,
	CS_CREDITS_EPILOGUE_CENTER_X = 0x100,
	CS_CREDITS_EPILOGUE_Y = 0xaf,
	CS_CREDITS_EPILOGUE_WIDTH = 0x1cc,
	CS_CREDITS_EPILOGUE_UNUSED_POS_X = 0x200,
	CS_CREDITS_GHOST_BASE_SCALE = FP_ONE,
	CS_CREDITS_GHOST_SCALE_STEP = 300,
	CS_CREDITS_GHOST_ALPHA_STEP = 630,
	CS_CREDITS_GHOST_TRAIL_SCALE_STEP = 0x4b,
	CS_CREDITS_GHOST_TRAIL_ALPHA_STEP = 0x9d,
};

CTR_STATIC_ASSERT(CS_CREDITS_RELIC_TRACK_COUNT == 0x12);
CTR_STATIC_ASSERT(CS_CREDITS_THREAD_FLAGS == 0x30d);
CTR_STATIC_ASSERT(CS_CREDITS_NAME_DIRECTIVE_STYLE_52_FLAG == 0x2000);
CTR_STATIC_ASSERT(CS_CREDITS_NAME_DIRECTIVE_STYLE_53_FLAG == 0x1000);

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b8810-0x800b885c
char *CS_Credits_GetNextString(char *str)
{
#if defined(CTR_NATIVE)
	if (str == NULL)
	{
		// NOTE(aalhendi): Retail blindly reads the input pointer. Native
		// returns the same "no next string" result when credits epilogue text
		// reaches the end and the next pointer is PS1 null-space.
		return NULL;
	}
#endif

	char c = *str;
	while (c != '\0')
	{
		if (c == '\r')
		{
			return str + 1;
		}
		str++;
		c = *str;
	}
	if (*str != '\r')
	{
		return 0;
	}
	return str + 1;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b885c-0x800b88c8
void CS_Credits_DestroyCreditGhost(void)
{
	for (int i = 0; i < CS_CREDITS_GHOST_COUNT; i++)
	{
		INSTANCE_Death(creditsBSS.creditsObj.creditGhostInst[i]);
	}

	MEMPACK_ClearHighMem();
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b8668-0x800b8810
void CS_Credits_AnimateCreditGhost(struct Instance *dst, struct Instance *src, int index)
{
	struct CreditsObj *co = &creditsBSS.creditsObj;

	dst->animFrame = src->animFrame;
	dst->animIndex = src->animIndex;

	dst->matrix = src->matrix;

	s16 scale = CS_CREDITS_GHOST_BASE_SCALE + (index + 1) * CS_CREDITS_GHOST_SCALE_STEP;

	dst->scale.x = scale;
	dst->scale.y = scale;
	dst->scale.z = scale;

	dst->flags &= ~HIDE_MODEL;
	if ((int)dst->model == 0)
	{
		dst->flags |= HIDE_MODEL;
	}

	dst->alphaScale = (index + 1) * CS_CREDITS_GHOST_ALPHA_STEP;

	struct Model *localModel = &co->creditGhostModelCopies[index];
	dst->model = localModel;

	struct Model *srcModel = src->model;
	int *dstModelInts = (int *)localModel;
	int *srcModelInts = (int *)srcModel;
	dstModelInts[0] = srcModelInts[0];
	dstModelInts[1] = srcModelInts[1];
	dstModelInts[2] = srcModelInts[2];
	dstModelInts[3] = srcModelInts[3];
	dstModelInts[4] = srcModelInts[4];
	dstModelInts[5] = srcModelInts[5];

	localModel->headers = co->creditGhostHeaders[index];

	s16 srcNumHeaders = srcModel->numHeaders;
	if (srcNumHeaders > 0)
	{
		struct ModelHeader *dstHeaders = localModel->headers;
		struct ModelHeader *srcHeaders = srcModel->headers;

		for (int i = 0; i < srcNumHeaders; i++)
		{
			int *d = (int *)&dstHeaders[i];
			int *s = (int *)&srcHeaders[i];
			d[0] = s[0];
			d[1] = s[1];
			d[2] = s[2];
			d[3] = s[3];
			d[4] = s[4];
			d[5] = s[5];
			d[6] = s[6];
			d[7] = s[7];
			d[8] = s[8];
			d[9] = s[9];
			d[10] = s[10];
			d[11] = s[11];
			d[12] = s[12];
			d[13] = s[13];
			d[14] = s[14];
			d[15] = s[15];
		}
	}
}

// NOTE(aalhendi): Native copies of retail credits rdata names at
// 0x800b8644-0x800b8678.
static char cs_creditsRData[] = "credits\0creditghost\0credit strings";
static char *const cs_creditsThreadName = &cs_creditsRData[0];
static char *const cs_creditGhostName = &cs_creditsRData[8];

// NOTE(aalhendi): Retail stores the no-op return stub at 0x800b8f84 as the
// credits thread destroy callback.
static void CS_Credits_ThDestroy_NoOp(struct Thread *self)
{
	(void)self;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b8f8c-0x800b92a0
void CS_Credits_Init(void)
{
	int i;
	int bitIndex;
	struct Instance *inst;

	b32 boolAllGold;
	struct GameTracker *gGT;
	struct AdvProgress *advProg;
	struct CreditsObj *creditsObj;
	struct CreditsLevHeader *CLH;
	struct CreditsLevHeader *creditsDst;
	struct Thread *creditThread;

	gGT = sdata->gGT;
	advProg = &sdata->advProgress;
	creditsObj = &creditsBSS.creditsObj;

	void **pointers = ST1_GETPOINTERS(gGT->level1->ptrSpawnType1);
	CLH = pointers[ST1_CREDITS];

	creditsBSS.dancerThread = 0;

	creditsBSS.boolAllBlue = 1;
	boolAllGold = true;

	for (i = 0; i < CS_CREDITS_RELIC_TRACK_COUNT; i++)
	{
		if (creditsBSS.boolAllBlue != 0)
		{
			bitIndex = i + ADV_REWARD_FIRST_SAPPHIRE_RELIC;
			creditsBSS.boolAllBlue = CHECK_ADV_BIT(advProg->rewards, bitIndex);
		}

		if (boolAllGold != 0)
		{
			bitIndex = i + ADV_REWARD_FIRST_GOLD_RELIC;
			boolAllGold = CHECK_ADV_BIT(advProg->rewards, bitIndex);
		}
	}

	if (boolAllGold != 0)
	{
		gGT->numWinners = 1;
		gGT->winnerIndex[0] = 0;
		gGT->confetti.numParticles_max = CS_CREDITS_CONFETTI_PARTICLE_COUNT;
		gGT->confetti.vanishRate = CS_CREDITS_CONFETTI_PARTICLE_COUNT;
		gGT->renderFlags |= RENDER_FLAG_CONFETTI;
	}

	creditThread = PROC_BirthWithObject(CS_CREDITS_THREAD_FLAGS, CS_Credits_ThTick, cs_creditsThreadName, NULL);
	creditThread->funcThDestroy = CS_Credits_ThDestroy_NoOp;
	creditsBSS.creditThread = creditThread;

	memset(creditsObj, 0, sizeof(struct CreditsObj));
	creditsObj->countdown = CS_CREDITS_COUNTDOWN_FRAMES;

	// credit ghost instances
	for (i = 0; i < CS_CREDITS_GHOST_COUNT; i++)
	{
		// STATIC_AKUAKU for some reason?
		inst = INSTANCE_Birth3D(gGT->modelPtr[STATIC_AKUAKU], cs_creditGhostName, creditThread);

		// save instance
		creditsObj->creditGhostInst[(CS_CREDITS_GHOST_COUNT - 1) - i] = inst;

		CTR_MatrixSetRotIdentity(&inst->matrix);

		inst->flags |= SCREENSPACE_INSTANCE;

		struct InstDrawPerPlayer *idpp = INST_GETIDPP(inst);
		idpp[0].pushBuffer = &gGT->pushBuffer_UI;

		for (int j = 1; j < gGT->numPlyrCurrGame; j++)
		{
			idpp[j].pushBuffer = NULL;
		}
	}

	creditsBSS.dancerInst_invisible = NULL;

	creditsDst = MEMPACK_AllocHighMem(CLH->size /* "credits strings" */);

	memcpy(creditsDst, CLH, CLH->size);

	creditsBSS.numStrings = creditsDst->numStrings;

	char **ptrStrings = (char **)CREDITSHEADER_GETSTRINGS(creditsDst);
	creditsBSS.ptrStrings = ptrStrings;

	for (i = 0; i < creditsBSS.numStrings; i++)
	{
		ptrStrings[i] = (char *)((u32)ptrStrings[i] + (u32)creditsDst);
	}

	creditsObj->creditsPosY = CS_CREDITS_NAME_START_Y;
	creditsObj->creditsTopString = ptrStrings[CS_CREDITS_TEXT_START_STRING_INDEX];
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b92a0-0x800b92cc
b32 CS_Credits_IsTextValid(void)
{
	struct CreditsObj *creditsObj = &creditsBSS.creditsObj;

	if (creditsObj->epilogueTopString != 0)
	{
		return 0;
	}

	creditsObj->countdown = CS_CREDITS_COUNTDOWN_FRAMES;
	return 1;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b92cc-0x800b9398
void CS_Credits_NewDancer(struct Thread *dancerTh, int dancerModelID)
{
	struct CreditsObj *creditsObj = &creditsBSS.creditsObj;

	// kill any living thread
	struct Thread *oldDancerThread = creditsBSS.dancerThread;
	if (oldDancerThread != 0)
	{
		creditsBSS.dancerThread = 0;
		oldDancerThread->flags |= THREAD_FLAG_DEAD;
	}

	// store globally, make instance invisible
	creditsBSS.dancerThread = dancerTh;
	creditsBSS.dancerInst_invisible = dancerTh->inst;
	creditsBSS.dancerInst_invisible->flags |= HIDE_MODEL;

	creditsObj->countdown = CS_CREDITS_COUNTDOWN_FRAMES;

	char **ptrStrings = creditsBSS.ptrStrings;

	// less than TAWNA1
	if (dancerModelID < STATIC_TAWNA1)
	{
		// subtract CRASHDANCE
		creditsObj->epilogueTopString = ptrStrings[dancerModelID - STATIC_CRASHDANCE];
	}

	// TAWNA
	else
	{
		// subtract an extra cause of GARAGE_TOP
		creditsObj->epilogueTopString = ptrStrings[(dancerModelID - STATIC_CRASHDANCE) - 1];
	}

	creditsObj->epilogueFramesLeft = CS_CREDITS_EPILOGUE_DURATION_FRAMES;

	creditsObj->epilogueNextString = CS_Credits_GetNextString(creditsObj->epilogueTopString);

	creditsObj->epiloguePosX_unused = CS_CREDITS_EPILOGUE_UNUSED_POS_X;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b9398-0x800b93f4
int CS_Credits_NewCreditGhosts(void)
{
	struct Model *model = creditsBSS.dancerInst_invisible->model;
	int i;

	for (i = 0; i < CS_CREDITS_GHOST_COUNT; i++)
	{
		if (creditsBSS.creditsObj.creditGhostModel[i] != model)
		{
			return 0;
		}
	}

	return 1;
}

#if defined(CTR_NATIVE)
static void CS_Credits_RestorePodiumAudioForNativeHandoff(void)
{
	if ((D233.CutsceneManipulatesAudio == 0) || (howl_VolumeGet(HOWL_VOLUME_TYPE_FX) != 0) || (D233.FXVolumeBackup == 0))
	{
		return;
	}

	// NOTE(aalhendi): Boss scripts can mute HOWL with opcode 0x30; normal
	// podium exits restore it in CS_DestroyPodium_StartDriving. Credits do not
	// return to driving, and their scripts do not apply this mute. Native can
	// still reach this handoff with a leaked FX mute, so restore the podium
	// backup before loading the hub/Scrapbook. A zero backup preserves
	// user-muted SFX.
	howl_VolumeSet(HOWL_VOLUME_TYPE_FX, (u8)D233.FXVolumeBackup);
	howl_VolumeSet(HOWL_VOLUME_TYPE_MUSIC, (u8)D233.MusicVolumeBackup);
	howl_VolumeSet(HOWL_VOLUME_TYPE_VOICE, (u8)D233.VoiceVolumeBackup);
}
#endif

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b93f4-0x800b9488
void CS_Credits_End(void)
{
	int levID;
	struct GameTracker *gGT = sdata->gGT;

	// erase 5 instances
	CS_Credits_DestroyCreditGhost();

	// kill thread
	creditsBSS.creditThread->flags |= THREAD_FLAG_DEAD;

#if defined(CTR_NATIVE)
	CS_Credits_RestorePodiumAudioForNativeHandoff();
#endif

	// go to gemstone valley
	if (creditsBSS.boolAllBlue == 0)
	{
		levID = GEM_STONE_VALLEY;

		gGT->gameMode1 |= ADVENTURE_MODE;
	}

	// go to scrapbook
	else
	{
		sdata->mainMenuState = MAIN_MENU_SCRAPBOOK;

		levID = SCRAPBOOK;
	}

	MainRaceTrack_RequestLoad(levID);

	gGT->renderFlags &= ~RENDER_FLAG_CONFETTI;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b88c8-0x800b8bd0
void CS_Credits_DrawNames(struct CreditsObj *co)
{
	if (co->creditsTopString == 0)
	{
		return;
	}

	co->creditsPosY--;

	if (co->creditsPosY < -CS_CREDITS_LINE_HEIGHT)
	{
		co->creditsTopString = CS_Credits_GetNextString(co->creditsTopString);
		co->creditsPosY += CS_CREDITS_LINE_HEIGHT;
	}

	int posY = co->creditsPosY;
	char *str = co->creditsTopString;
	int charId = 0;

	while (posY < CS_CREDITS_NAME_END_Y)
	{
		u16 textFlags = 0;

		if (*str == '~')
		{
			char *p = str + 2;

			do
			{
				int digit1 = (u8)p[-1] - CS_CREDITS_NAME_DIRECTIVE_DIGIT_BASE;
				int digit2 = (u8)p[0];
				p += CS_CREDITS_NAME_DIRECTIVE_STRIDE;
				int value = digit2 + digit1 * 10 - CS_CREDITS_NAME_DIRECTIVE_DIGIT_BASE;
				str += CS_CREDITS_NAME_DIRECTIVE_STRIDE;

				if (value < CS_CREDITS_NAME_DIRECTIVE_COLOR_LIMIT)
				{
					charId = value;
				}
				else if (value == CS_CREDITS_NAME_DIRECTIVE_STYLE_52)
				{
					textFlags |= CS_CREDITS_NAME_DIRECTIVE_STYLE_52_FLAG;
				}
				else if (value == CS_CREDITS_NAME_DIRECTIVE_STYLE_53)
				{
					textFlags |= CS_CREDITS_NAME_DIRECTIVE_STYLE_53_FLAG;
				}
			} while (*str == '~');
		}

		char *nextStr = CS_Credits_GetNextString(str);
		s16 strLen;

		if (nextStr == 0)
		{
			strLen = strlen(str);
		}
		else
		{
			strLen = (s16)(nextStr - str) - 1;
		}

		int clampedY = posY;
		int fadeAmount = CS_CREDITS_LINE_HEIGHT;

		if (clampedY < CS_CREDITS_NAME_FADE_END_START_Y)
		{
			if (clampedY < CS_CREDITS_LINE_HEIGHT)
			{
				fadeAmount = clampedY;
			}
		}
		else
		{
			fadeAmount = CS_CREDITS_NAME_FADE_BOTTOM_Y - clampedY;
		}

		int colorSlot = charId;

		if ((fadeAmount < CS_CREDITS_LINE_HEIGHT) && (creditsBSS.boolAllBlue != 0))
		{
			if (fadeAmount < 1)
			{
				colorSlot = -1;
			}
			else
			{
				colorSlot = CREDITS_FADE;

				int fade8 = (fadeAmount << 8) / CS_CREDITS_LINE_HEIGHT;
				char *src = (char *)data.ptrColor[charId];
				char *dst = (char *)&data.colors[CREDITS_FADE];

				for (int i = 0; i < 4; i++)
				{
					int stride = i * 4;
					dst[stride + 0] = (char)((u8)src[stride + 0] * fade8 >> 8);
					dst[stride + 1] = (char)((u8)src[stride + 1] * fade8 >> 8);
					dst[stride + 2] = (char)((u8)src[stride + 2] * fade8 >> 8);
				}
			}
		}

		if (colorSlot >= 0)
		{
			DecalFont_DrawLineStrlen(str, strLen, creditsBSS.creditTextPosX, posY, FONT_CREDITS, colorSlot | textFlags);
		}

		posY += CS_CREDITS_LINE_HEIGHT;

		if (nextStr == 0)
		{
			return;
		}

		str = nextStr;
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b8bd0-0x800b8dc8
void CS_Credits_DrawEpilogue(struct CreditsObj *co)
{
	if (co->epilogueTopString == 0)
	{
		return;
	}

	co->epilogueFramesLeft--;

	if (co->epilogueFramesLeft <= 0)
	{
		co->epilogueFramesLeft = CS_CREDITS_EPILOGUE_DURATION_FRAMES;
		co->epilogueTopString = co->epilogueNextString;
		co->epilogueNextString = CS_Credits_GetNextString(co->epilogueNextString);
	}

	if (co->epilogueTopString == 0)
	{
		return;
	}

	s16 timeRemaining = co->epilogueFramesLeft;
	s16 fadeAmount = CS_CREDITS_LINE_HEIGHT;

	if (timeRemaining < CS_CREDITS_EPILOGUE_FADE_OUT_START_FRAMES)
	{
		if (timeRemaining < CS_CREDITS_LINE_HEIGHT)
		{
			fadeAmount = timeRemaining;
		}
	}
	else
	{
		fadeAmount = CS_CREDITS_EPILOGUE_DURATION_FRAMES - timeRemaining;
	}

	s16 colorSlot = WHITE;

	if (fadeAmount < CS_CREDITS_LINE_HEIGHT)
	{
		if (fadeAmount > 0)
		{
			colorSlot = CREDITS_FADE;

			int fade8 = (fadeAmount << 8) / CS_CREDITS_LINE_HEIGHT;
			char *dst = (char *)&data.colors[CREDITS_FADE];
			char *src = (char *)data.ptrColor[WHITE];

			for (int i = 0; i < 4; i++)
			{
				int stride = i * 4;
				dst[stride + 0] = (char)((u8)src[stride + 0] * fade8 >> 8);
				dst[stride + 1] = (char)((u8)src[stride + 1] * fade8 >> 8);
				dst[stride + 2] = (char)((u8)src[stride + 2] * fade8 >> 8);
			}
		}
		else
		{
			colorSlot = -1;
		}
	}

	if ((colorSlot >= 0) && (creditsBSS.boolAllBlue != 0))
	{
		s16 strLen = -1;

		if (co->epilogueNextString != 0)
		{
			strLen = (s16)(co->epilogueNextString - co->epilogueTopString) - 1;
		}

		DecalFont_DrawMultiLineStrlen(co->epilogueTopString, strLen, CS_CREDITS_EPILOGUE_CENTER_X, CS_CREDITS_EPILOGUE_Y, CS_CREDITS_EPILOGUE_WIDTH, FONT_SMALL,
		                              colorSlot | JUSTIFY_CENTER);
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b8dc8-0x800b8f8c
void CS_Credits_ThTick(void)
{
	struct CreditsObj *co = &creditsBSS.creditsObj;
	struct Instance *danceInst = creditsBSS.dancerInst_invisible;

	co->creditDanceInst = danceInst;

	if (danceInst != NULL)
	{
		danceInst->flags |= HIDE_MODEL;

		danceInst->matrix.t[0] = (int)creditsBSS.creditGhostPos.x;
		danceInst->matrix.t[1] = (int)creditsBSS.creditGhostPos.y;
		danceInst->matrix.t[2] = (int)creditsBSS.creditGhostPos.z;

		struct GameTracker *gGT = sdata->gGT;

		if ((gGT->timer & 3) == 0)
		{
			for (int i = CS_CREDITS_GHOST_COUNT - 1; i > 0; i--)
			{
				CS_Credits_AnimateCreditGhost(co->creditGhostInst[i], co->creditGhostInst[i - 1], i);
				co->creditGhostModel[i] = co->creditGhostModel[i - 1];
			}

			CS_Credits_AnimateCreditGhost(co->creditGhostInst[0], co->creditDanceInst, 0);
			co->creditGhostModel[0] = co->creditDanceInst->model;
		}
		else
		{
			CS_Credits_AnimateCreditGhost(co->creditGhostInst[0], co->creditDanceInst, 0);

			for (int i = 1; i < CS_CREDITS_GHOST_COUNT; i++)
			{
				struct Instance *ghost = co->creditGhostInst[i];
				ghost->scale.x += CS_CREDITS_GHOST_TRAIL_SCALE_STEP;
				ghost->scale.y += CS_CREDITS_GHOST_TRAIL_SCALE_STEP;
				ghost->scale.z += CS_CREDITS_GHOST_TRAIL_SCALE_STEP;
				ghost->alphaScale += CS_CREDITS_GHOST_TRAIL_ALPHA_STEP;
			}
		}
	}

	if (co->countdown > 0)
	{
		co->countdown--;
	}

	CS_Credits_DrawNames(co);
	CS_Credits_DrawEpilogue(co);
}
