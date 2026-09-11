#include <common.h>

#if defined(CTR_NATIVE) && defined(CTR_INTERNAL)
#include <platform/native_checkpoint.h>
#endif

#if defined(CTR_NATIVE)
enum
{
	NATIVE_AI_RANDOMIZER_DRIVER_COUNT = LOAD_CHARACTER_ID_COUNT,
	NATIVE_AI_RANDOMIZER_CHARACTER_COUNT = NITROS_OXIDE + 1,
	NATIVE_AI_RANDOMIZER_2P_AI_COUNT = LOAD_2P_AI_SET_RACER_COUNT,
};

static DriverModelExtraSlot s_nativeAIRandomizerModels[NATIVE_AI_RANDOMIZER_DRIVER_COUNT];
static void *s_nativeAIRandomizer2PBuffers[NATIVE_AI_RANDOMIZER_2P_AI_COUNT];
static struct Model *s_nativeAIRandomizer2PModels[NATIVE_AI_RANDOMIZER_2P_AI_COUNT];

static b32 NativeAIRandomizer_ShouldUse(const struct GameTracker *gGT)
{
	if ((gGT == NULL) || (gNativeBossFightMode != 0) || (gGT->boolDemoMode != 0))
	{
		return false;
	}

	if ((gGT->levelID >= GEM_STONE_VALLEY) || (gGT->numPlyrCurrGame < 1) || (gGT->numPlyrCurrGame > 2))
	{
		return false;
	}

	if ((gGT->gameMode1 &
	     (ADVENTURE_MODE | ADVENTURE_ARENA | ADVENTURE_BOSS | ADVENTURE_CUP | BATTLE_MODE | TIME_TRIAL | RELIC_RACE | CRYSTAL_CHALLENGE |
	      MAIN_MENU | GAME_CUTSCENE)) != 0)
	{
		return false;
	}

	if ((gGT->gameMode2 & (CREDITS | TOKEN_RACE)) != 0)
	{
		return false;
	}

#if defined(__vita__)
	if (NativeAdhoc_IsActive())
	{
		return false;
	}
#endif

	return true;
}

static b32 NativeAIRandomizer_ShouldPreserveCupLineup(const struct GameTracker *gGT)
{
	return NativeAIRandomizer_ShouldUse(gGT) && ((gGT->gameMode2 & CUP_ANY_KIND) != 0) && (gGT->cup.trackIndex != 0);
}

static u32 NativeAIRandomizer_Next(u32 *state)
{
	*state = (*state * 1664525u) + 1013904223u;
	return *state;
}

static u32 NativeAIRandomizer_Seed(const struct GameTracker *gGT, int humanCount)
{
	u32 state = (u32)Timer_GetTime_Total() ^ (u32)sdata->randomNumber;
	state ^= (u32)gGT->currLEV * 0x9e3779b9u;
	state ^= (u32)gGT->numLaps * 0x85ebca6bu;

	for (int i = 0; i < humanCount; i++)
	{
		state = (state * 33u) ^ (u32)data.characterIDs[i];
	}

	return state != 0 ? state : 0x4354524eu;
}

static b32 NativeAIRandomizer_CharacterIsTaken(int driverIndex, int characterID)
{
	for (int i = 0; i < driverIndex; i++)
	{
		if (data.characterIDs[i] == characterID)
		{
			return true;
		}
	}

	return false;
}

static void NativeAIRandomizer_SetCharacters(struct GameTracker *gGT, int firstAI, int driverCount)
{
	if (NativeAIRandomizer_ShouldPreserveCupLineup(gGT))
	{
		return;
	}

	u32 state = NativeAIRandomizer_Seed(gGT, firstAI);
	for (int driverIndex = firstAI; driverIndex < driverCount; driverIndex++)
	{
		int characterID;
		do
		{
			characterID = (int)(NativeAIRandomizer_Next(&state) % NATIVE_AI_RANDOMIZER_CHARACTER_COUNT);
		} while (NativeAIRandomizer_CharacterIsTaken(driverIndex, characterID));

		data.characterIDs[driverIndex] = characterID;
	}
}

static void NativeAIRandomizer_2PModelLoaded(struct LoadQueueSlot *lqs)
{
	int characterID = lqs->subfileIndex - BI_RACERMODELMED;

	for (int i = 0; i < NATIVE_AI_RANDOMIZER_2P_AI_COUNT; i++)
	{
		int driverIndex = 2 + i;
		if (data.characterIDs[driverIndex] == characterID)
		{
			s_nativeAIRandomizer2PModels[i] = (struct Model *)lqs->ptrDestination;
			return;
		}
	}
}

static b32 NativeAIRandomizer_Queue2PModels(struct BigHeader *bigfile)
{
	struct BigEntry *entries = BIG_GETENTRY(bigfile);

	for (int i = 0; i < NATIVE_AI_RANDOMIZER_2P_AI_COUNT; i++)
	{
		int driverIndex = 2 + i;
		int fileIndex = BI_RACERMODELMED + data.characterIDs[driverIndex];
		u32 readSize = (entries[fileIndex].size + LOAD_CD_DATA_SECTOR_ROUND_MASK) & ~LOAD_CD_DATA_SECTOR_ROUND_MASK;

		s_nativeAIRandomizer2PBuffers[i] = malloc((size_t)readSize);
		if (s_nativeAIRandomizer2PBuffers[i] == NULL)
		{
			for (int j = 0; j < i; j++)
			{
				free(s_nativeAIRandomizer2PBuffers[j]);
				s_nativeAIRandomizer2PBuffers[j] = NULL;
			}
			return false;
		}
	}

	for (int i = 0; i < NATIVE_AI_RANDOMIZER_2P_AI_COUNT; i++)
	{
		int driverIndex = 2 + i;
		int fileIndex = BI_RACERMODELMED + data.characterIDs[driverIndex];
		LOAD_AppendQueue(bigfile, LT_GETADDR, fileIndex, s_nativeAIRandomizer2PBuffers[i], NativeAIRandomizer_2PModelLoaded);
	}

	return true;
}

static void NativeAIRandomizer_ResetModels(void)
{
	for (int i = 0; i < NATIVE_AI_RANDOMIZER_2P_AI_COUNT; i++)
	{
		if (s_nativeAIRandomizer2PBuffers[i] != NULL)
		{
			free(s_nativeAIRandomizer2PBuffers[i]);
			s_nativeAIRandomizer2PBuffers[i] = NULL;
		}
		s_nativeAIRandomizer2PModels[i] = NULL;
	}

	memset(s_nativeAIRandomizerModels, 0, sizeof(s_nativeAIRandomizerModels));
}

static void NativeAIRandomizer_QueueDriverModel(struct BigHeader *bigfile, int driverIndex, int bigfileIndex)
{
	LOAD_AppendQueue(bigfile, LT_GETADDR, bigfileIndex,
	                 &s_nativeAIRandomizerModels[driverIndex].fileBase, LOAD_QUEUE_CALLBACK_SET_POINTER);
}

static void NativeAIRandomizer_QueueCPUModels(struct BigHeader *bigfile, int firstAI, int driverCount, int packDriverIndex, int modelBaseIndex)
{
	for (int driverIndex = firstAI; driverIndex < driverCount; driverIndex++)
	{
		if (driverIndex == packDriverIndex)
		{
			continue;
		}

		NativeAIRandomizer_QueueDriverModel(bigfile, driverIndex, modelBaseIndex + data.characterIDs[driverIndex]);
	}
}

void NativeAIRandomizer_FinalizeModels(void)
{
	for (int driverIndex = 0; driverIndex < NATIVE_AI_RANDOMIZER_DRIVER_COUNT; driverIndex++)
	{
		if (s_nativeAIRandomizerModels[driverIndex].fileBase != NULL)
		{
			s_nativeAIRandomizerModels[driverIndex].model =
			    (struct Model *)((u8 *)s_nativeAIRandomizerModels[driverIndex].fileBase + LOAD_MODEL_FILE_HEADER_BYTES);
		}
	}
}

struct Model *NativeAIRandomizer_GetDriverModel(int driverIndex)
{
	if ((u32)driverIndex >= NATIVE_AI_RANDOMIZER_DRIVER_COUNT)
	{
		return NULL;
	}

	if ((driverIndex >= 2) && (driverIndex < 2 + NATIVE_AI_RANDOMIZER_2P_AI_COUNT))
	{
		struct Model *model = s_nativeAIRandomizer2PModels[driverIndex - 2];
		if (model != NULL)
		{
			return model;
		}
	}

	return s_nativeAIRandomizerModels[driverIndex].model;
}
#endif

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800326b4-0x80032700.
void LOAD_RunPtrMap(char *origin, int *patchArr, int numPtrs)
{
	int *ptrCurrOffset = patchArr;

	for (ptrCurrOffset = &patchArr[0]; ptrCurrOffset < &patchArr[numPtrs]; ptrCurrOffset++)
	{
		int offset = (*ptrCurrOffset >> 2) << 2;
		*(int *)&origin[offset] = *(int *)&origin[offset] + (int)origin;
#if defined(CTR_NATIVE) && defined(CTR_INTERNAL)
		NativeCheckpoint_RegisterPointerSlot(&origin[offset]);
#endif
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80032700-0x800327dc.
void LOAD_Robots2P(struct BigHeader *bigfile, int p1, int p2, void (*callback)(struct LoadQueueSlot *))
{
	int setIndex;
	u8 *robotSet;
	b32 boolFoundRepeat = false;

	// 8 sets, but only check 7 because the last is the Gem Cups pack (4 bosses).
	for (setIndex = 0; setIndex < LOAD_2P_AI_SET_COUNT; setIndex++)
	{
		robotSet = data.characterIDs_2P_AIs[setIndex];

		boolFoundRepeat = false;
		for (int racerIndex = 0; racerIndex < LOAD_2P_AI_SET_RACER_COUNT; racerIndex++)
		{
			if ((robotSet[racerIndex] == p1) || (robotSet[racerIndex] == p2))
			{
				boolFoundRepeat = true;
				break;
			}
		}

		if (!boolFoundRepeat)
		{
			break;
		}
	}

	if (setIndex >= LOAD_2P_AI_SET_COUNT)
	{
		return;
	}

#if defined(CTR_NATIVE)
	if (NativeAIRandomizer_ShouldUse(sdata->gGT))
	{
		NativeAIRandomizer_SetCharacters(sdata->gGT, 2, 2 + LOAD_2P_AI_SET_RACER_COUNT);

		if (!NativeAIRandomizer_Queue2PModels(bigfile))
		{
			for (int i = 0; i < LOAD_2P_AI_SET_RACER_COUNT; i++)
			{
				data.characterIDs[2 + i] = robotSet[i];
			}
		}

		LOAD_AppendQueue(bigfile, LT_GETADDR, BI_2PARCADEPACK + setIndex, NULL, callback);
		return;
	}
#endif

	data.characterIDs[2] = robotSet[0];
	data.characterIDs[3] = robotSet[1];
	data.characterIDs[4] = robotSet[2];
	data.characterIDs[5] = robotSet[3];

	LOAD_AppendQueue(bigfile, LT_GETADDR, BI_2PARCADEPACK + setIndex, NULL, callback);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800327dc-0x8003282c.
void LOAD_Robots1P(int characterID)
{
	int newCharacterID = 0;

	data.characterIDs[0] = characterID;

#if defined(CTR_NATIVE)
	if (NativeAIRandomizer_ShouldPreserveCupLineup(sdata->gGT))
	{
		return;
	}
#endif

	for (int i = 1; i < LOAD_CHARACTER_ID_COUNT; i++, newCharacterID++)
	{
		if (newCharacterID == characterID)
		{
			newCharacterID++;
		}

		data.characterIDs[i] = newCharacterID;
	}

#if defined(CTR_NATIVE)
	if (NativeAIRandomizer_ShouldUse(sdata->gGT))
	{
		NativeAIRandomizer_SetCharacters(sdata->gGT, 1, LOAD_CHARACTER_ID_COUNT);
	}
#endif
}

static void (*const LOAD_DriverMPK_SetPointer)(struct LoadQueueSlot *) = LOAD_QUEUE_CALLBACK_SET_POINTER;

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003282c-0x80032b50.
int LOAD_DriverMPK(struct BigHeader *bigfile, int levelLOD, void (*callback)(struct LoadQueueSlot *))
{
	int i;
	int gameMode1;

	struct GameTracker *gGT = sdata->gGT;
#if defined(CTR_NATIVE)
	NativeAIRandomizer_ResetModels();
#endif
#if defined(__vita__)
	if (NativeAdhoc_EnforcePreparedRaceConfig(gGT))
	{
		levelLOD = LOAD_LEVEL_LOD_2P;
	}
#endif
	gameMode1 = gGT->gameMode1;

	int lastFileIndexMPK;

	// 3P/4P
	if ((u32)(levelLOD - LOAD_LEVEL_LOD_3P) < LOAD_LEVEL_LOD_3P4P_COUNT)
	{
		for (i = 0; i < LOAD_DRIVER_MODEL_EXTRA_COUNT; i++)
		{
			// low lod CTR model
			LOAD_AppendQueue(bigfile, LT_GETADDR, BI_RACERMODELLOW + data.characterIDs[i], &data.driverModelExtras[i].fileBase, LOAD_DriverMPK_SetPointer);
		}

		// load 4P MPK of fourth player
		lastFileIndexMPK = BI_4PARCADEPACK + data.characterIDs[3];
	}

	else if (levelLOD == LOAD_LEVEL_LOD_1P)
	{
		if ((gameMode1 & (TIME_TRIAL | MAIN_MENU)) == TIME_TRIAL)
		{
			goto LoadHighAndPack;
		}

		if (
		    // adv/cutscene mpk when we just need text from MPK
		    ((gameMode1 & (GAME_CUTSCENE | ADVENTURE_ARENA)) != 0) ||

		    // credits
		    ((gGT->gameMode2 & CREDITS) != 0) ||

		    // adventure character select
		    (gGT->levelID == ADVENTURE_GARAGE))
		{
			lastFileIndexMPK = BI_ADVENTUREPACK + data.characterIDs[0];
			goto QueueLastPack;
		}

		if ((gameMode1 & ADVENTURE_BOSS) != 0)
		{
			goto LoadHighAndPack;
		}

		if (
		    // If you are in Adventure cup
		    ((gameMode1 & ADVENTURE_CUP) != 0) &&

		    // purple gem cup
		    (gGT->cup.cupID == 4))
		{
			// high lod model
			LOAD_AppendQueue(bigfile, LT_GETADDR, BI_RACERMODELHI + data.characterIDs[0], &data.driverModelExtras[0].fileBase, LOAD_DriverMPK_SetPointer);

			// pack of four AIs with bosses
			LOAD_AppendQueue(bigfile, LT_GETADDR, BI_2PARCADEPACK + LOAD_PURPLE_GEM_CUP_AI_SET_INDEX, NULL, callback);

			data.characterIDs[1] = RIPPER_ROO;
			data.characterIDs[2] = PAPU_PAPU;
			data.characterIDs[3] = KOMODO_JOE;
			data.characterIDs[4] = PINSTRIPE;

			return sdata->ptrMPK;
		}

		if ((gameMode1 & (TIME_TRIAL | MAIN_MENU)) != MAIN_MENU)
		{
			LOAD_Robots1P(data.characterIDs[0]);
		}

#if defined(CTR_NATIVE)
		if (NativeAIRandomizer_ShouldUse(gGT))
		{
			int packDriverIndex = LOAD_CHARACTER_ID_COUNT - 1;

			NativeAIRandomizer_QueueDriverModel(bigfile, 0, BI_RACERMODELHI + data.characterIDs[0]);
			NativeAIRandomizer_QueueCPUModels(bigfile, 1, LOAD_CHARACTER_ID_COUNT, packDriverIndex, BI_RACERMODELHI);
			lastFileIndexMPK = BI_4PARCADEPACK + data.characterIDs[packDriverIndex];
		}
		else
#endif
		{
			// arcade mpk
			lastFileIndexMPK = BI_1PARCADEPACK + data.characterIDs[0];
		}
	}

	else if ((levelLOD == LOAD_LEVEL_LOD_RELIC) || ((gameMode1 & TIME_TRIAL) != 0))
	{
	LoadHighAndPack:
		// Do NOT switch the order to optimize Relic,
		// if HI+IDs[1] and PACK+IDs[0] is loaded,
		// then mask-grab breaks for all characters
		// on Hot Air Skyway (except Crash Bandicoot)

		// Load Player 1 [0]
		LOAD_AppendQueue(bigfile, LT_GETADDR, BI_RACERMODELHI + data.characterIDs[0], &data.driverModelExtras[0].fileBase, LOAD_DriverMPK_SetPointer);

		// Load boss or ghost [1]
		lastFileIndexMPK = BI_TIMETRIALPACK + data.characterIDs[1];
	}

	// else if (levelLOD == LOAD_LEVEL_LOD_2P)
	else
	{
		// med models
		for (i = 0; i < LOAD_MED_LOD_DRIVER_MODEL_EXTRA_COUNT; i++)
		{
			// med lod CTR model
			LOAD_AppendQueue(bigfile, LT_GETADDR, BI_RACERMODELMED + data.characterIDs[i], &data.driverModelExtras[i].fileBase, LOAD_DriverMPK_SetPointer);
		}

		LOAD_Robots2P(bigfile, data.characterIDs[0], data.characterIDs[1], callback);
		return sdata->ptrMPK;
	}

QueueLastPack:
	LOAD_AppendQueue(bigfile, LT_GETADDR, lastFileIndexMPK, NULL, callback);
	return sdata->ptrMPK;
}

struct LngFile
{
	int numStrings;
	int offsetToPtrArr;
	char strings[1];
};

// param_1 - Pointer to "cd position of bigfile"
// param_2 - language index - 0 ja, 1 en, 2 en2, 3 fr, 4 de, 5 it, 6 es, 7 ne
// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80032b50-0x80032c24
void LOAD_LangFile(int bigfilePtr, int lang)
{
	struct LngFile *lngFile;
	u32 size;

	int i;
	int numStrings;
	char **strArray;

#if BUILD == EurRetail
	// This is to turn the screen black for a bit (optional)
	CTR_ErrorScreen(0, 0, 0);
	VSync(0);
#endif

	if (sdata->lngFile == 0)
	{
		struct BigHeader *bigfile = (struct BigHeader *)bigfilePtr;
		struct BigEntry *entries = BIG_GETENTRY(bigfile);
		u32 langBufferSize = (u32)sdata->langBufferSize;

		for (int i = 0; i < (BI_RACERMODELHI - BI_LANGUAGEFILE); i++)
		{
			u32 fileSize = (u32)entries[BI_LANGUAGEFILE + i].size;
			u32 readSize = (fileSize + LOAD_CD_DATA_SECTOR_ROUND_MASK) & ~LOAD_CD_DATA_SECTOR_ROUND_MASK;

			if (langBufferSize < readSize)
			{
				langBufferSize = readSize;
			}
		}

		sdata->langBufferSize = (int)langBufferSize;
		sdata->lngFile = MEMPACK_AllocMem(sdata->langBufferSize /* "lang buffer" */);
	}

	lngFile = sdata->lngFile;

	lngFile = LOAD_ReadFile_ex((struct BigHeader *)bigfilePtr, LT_SETADDR, BI_LANGUAGEFILE + lang, lngFile, &size, NULL);
	if (lngFile == NULL)
	{
		return;
	}

	numStrings = lngFile->numStrings;
	strArray = (char **)((u32)lngFile + lngFile->offsetToPtrArr);

	sdata->numLngStrings = numStrings;
	sdata->lngStrings = strArray;

	for (i = 0; i < numStrings; i++)
	{
		strArray[i] = (char *)((u32)strArray[i] + (u32)lngFile);
	}
#if defined(CTR_NATIVE)
	NativeAudio_SetVoiceLanguage(lang);
#elif BUILD == EurRetail
	// set voicelines to new lang
	CDSYS_SetXAToLang(lang);
#endif
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80032c24-0x80032d30.
int LOAD_GetBigfileIndex(u32 levelID, int lod, int fileIndexInGroup)
{
	if (levelID < NITRO_COURT)
	{
		return BI_ARCADETRACKS + levelID * LOAD_TRACK_FILES_PER_LOD_GROUP + sdata->levBigLodIndex[lod - 1] + fileIndexInGroup;
	}

	if ((u32)(levelID - NITRO_COURT) < LOAD_BATTLE_TRACK_COUNT)
	{
		return BI_BATTLETRACKS + (levelID - NITRO_COURT) * LOAD_TRACK_FILES_PER_LOD_GROUP + sdata->levBigLodIndex[lod - 1] + fileIndexInGroup;
	}

	if ((u32)(levelID - INTRO_RACE_TODAY) < LOAD_INTRO_CUTSCENE_COUNT)
	{
		return BI_CUTSCENES_INTRO + (levelID - INTRO_RACE_TODAY) * LOAD_CUTSCENE_FILES_PER_LEVEL + fileIndexInGroup;
	}

	if ((u32)(levelID - OXIDE_ENDING) < LOAD_OUTRO_CUTSCENE_COUNT)
	{
		return BI_CUTSCENES_OUTRO + (levelID - OXIDE_ENDING) * LOAD_OUTRO_FILES_PER_LEVEL + fileIndexInGroup;
	}

	if (levelID == ADVENTURE_GARAGE)
	{
		return BI_MAINMENUFILE + LOAD_MAIN_MENU_GARAGE_FILE_OFFSET + fileIndexInGroup;
	}

	if (levelID == NAUGHTY_DOG_CRATE)
	{
		return BI_NDBOX + fileIndexInGroup;
	}

	if ((u32)(levelID - CREDITS_CRASH) < LOAD_CREDIT_LEVEL_COUNT)
	{
		return BI_CREDITS + (levelID - CREDITS_CRASH) * LOAD_CUTSCENE_FILES_PER_LEVEL + fileIndexInGroup;
	}

	if (levelID == MAIN_MENU_LEVEL)
	{
		return BI_MAINMENUFILE + fileIndexInGroup;
	}

	if (levelID == SCRAPBOOK)
	{
		return BI_SCRAPBOOK + fileIndexInGroup;
	}

	return BI_ADVENTUREHUB + (levelID - GEM_STONE_VALLEY) * LOAD_CUTSCENE_FILES_PER_LEVEL + fileIndexInGroup;
}
