#include <common.h>

#include "platform/native_memcard.h"

#define NATIVE_REVERSE_TRACK_FIRST_LOGICAL_ID 18
#define NATIVE_REVERSE_TRACK_COUNT 8
#define NATIVE_REVERSE_SCORE_SAVE_NAME "BASCUS-94426R"
#define NATIVE_REVERSE_SCORE_MAGIC 0x31565248u
#define NATIVE_REVERSE_SCORE_VERSION 1u

struct NativeReverseTrackDef
{
	s16 logicalId;
	s16 physicalId;
};

struct NativeReverseScoreSave
{
	u32 magic;
	u32 version;
	u32 checksum;
	struct HighScoreTrack tracks[NATIVE_REVERSE_TRACK_COUNT];
};

static const struct NativeReverseTrackDef s_nativeReverseTrackDefs[NATIVE_REVERSE_TRACK_COUNT] =
{
	{18, CRASH_COVE},
	{19, ROO_TUBES},
	{20, TIGER_TEMPLE},
	{21, COCO_PARK},
	{22, DRAGON_MINES},
	{23, TINY_ARENA},
	{24, SLIDE_COLISEUM},
	{25, TURBO_TRACK},
};

int gNativeReverseTrackEnabled = 0;
static s16 s_nativeReverseTrackLogicalId = -1;
static s16 s_nativeReverseTrackPhysicalId = -1;
static struct HighScoreTrack s_nativeReverseHighScores[NATIVE_REVERSE_TRACK_COUNT];
static b32 s_nativeReverseHighScoresLoaded;

static int NativeReverseTrack_FindLogicalIndex(s16 logicalId)
{
	for (int i = 0; i < NATIVE_REVERSE_TRACK_COUNT; i++)
	{
		if (s_nativeReverseTrackDefs[i].logicalId == logicalId)
		{
			return i;
		}
	}
	return -1;
}

static int NativeReverseTrack_FindPhysicalIndex(s16 physicalId)
{
	for (int i = 0; i < NATIVE_REVERSE_TRACK_COUNT; i++)
	{
		if (s_nativeReverseTrackDefs[i].physicalId == physicalId)
		{
			return i;
		}
	}
	return -1;
}

b32 NativeReverseTrack_IsLogicalReverse(s16 logicalId)
{
	return NativeReverseTrack_FindLogicalIndex(logicalId) >= 0;
}

b32 NativeReverseTrack_IsPhysicalSupported(s16 physicalId)
{
	return NativeReverseTrack_FindPhysicalIndex(physicalId) >= 0;
}

s16 NativeReverseTrack_GetPhysicalFromLogical(s16 logicalId)
{
	int index = NativeReverseTrack_FindLogicalIndex(logicalId);
	return (index >= 0) ? s_nativeReverseTrackDefs[index].physicalId : logicalId;
}

s16 NativeReverseTrack_GetLogicalFromPhysical(s16 physicalId)
{
	int index = NativeReverseTrack_FindPhysicalIndex(physicalId);
	return (index >= 0) ? s_nativeReverseTrackDefs[index].logicalId : physicalId;
}

void NativeReverseTrack_ClearSelection(void)
{
	gNativeReverseTrackEnabled = 0;
	s_nativeReverseTrackLogicalId = -1;
	s_nativeReverseTrackPhysicalId = -1;
}

void NativeReverseTrack_SelectPhysical(s16 physicalId, b32 reverse)
{
	if (!reverse)
	{
		gNativeReverseTrackEnabled = 0;
		s_nativeReverseTrackLogicalId = physicalId;
		s_nativeReverseTrackPhysicalId = physicalId;
		return;
	}

	int index = NativeReverseTrack_FindPhysicalIndex(physicalId);
	if (index < 0)
	{
		NativeReverseTrack_ClearSelection();
		return;
	}

	gNativeReverseTrackEnabled = 1;
	s_nativeReverseTrackLogicalId = s_nativeReverseTrackDefs[index].logicalId;
	s_nativeReverseTrackPhysicalId = physicalId;
}

void NativeReverseTrack_SelectLogical(s16 logicalId)
{
	int index = NativeReverseTrack_FindLogicalIndex(logicalId);
	if (index >= 0)
	{
		gNativeReverseTrackEnabled = 1;
		s_nativeReverseTrackLogicalId = logicalId;
		s_nativeReverseTrackPhysicalId = s_nativeReverseTrackDefs[index].physicalId;
		return;
	}

	NativeReverseTrack_SelectPhysical(logicalId, false);
}

s16 NativeReverseTrack_GetSelectedLogicalTrackId(void)
{
	if (s_nativeReverseTrackLogicalId >= 0)
	{
		return s_nativeReverseTrackLogicalId;
	}
	return (sdata != NULL && sdata->gGT != NULL) ? sdata->gGT->levelID : -1;
}

s16 NativeReverseTrack_GetTrackIdForPhysical(s16 physicalId)
{
	if (gNativeReverseTrackEnabled && (s_nativeReverseTrackPhysicalId == physicalId))
	{
		return s_nativeReverseTrackLogicalId;
	}
	return physicalId;
}

s16 NativeReverseTrack_GetCurrentLogicalTrackId(void)
{
	if (gNativeReverseTrackEnabled && (s_nativeReverseTrackLogicalId >= 0))
	{
		return s_nativeReverseTrackLogicalId;
	}
	return (sdata != NULL && sdata->gGT != NULL) ? sdata->gGT->levelID : -1;
}

static b32 NativeReverseTrack_IsAllowedMode(void)
{
	if ((sdata == NULL) || (sdata->gGT == NULL))
	{
		return false;
	}

	u32 gameMode = (u32)sdata->gGT->gameMode1;
	return (gNativeGhostReplayMode != 0) ||
	       ((gameMode & TIME_TRIAL) != 0) ||
	       ((gNativeRelicRaceMode != 0) && ((gameMode & RELIC_RACE) != 0));
}

s16 NativeReverseTrack_ResolveLoadLevel(s16 requestedLevelId)
{
	if (!gNativeReverseTrackEnabled)
	{
		return requestedLevelId;
	}

	if (!NativeReverseTrack_IsAllowedMode())
	{
		s16 fallbackLevelId = (requestedLevelId == s_nativeReverseTrackLogicalId)
		                          ? s_nativeReverseTrackPhysicalId
		                          : requestedLevelId;
		NativeReverseTrack_ClearSelection();
		return fallbackLevelId;
	}

	if ((requestedLevelId == s_nativeReverseTrackLogicalId) ||
	    (requestedLevelId == s_nativeReverseTrackPhysicalId))
	{
		return s_nativeReverseTrackPhysicalId;
	}

	NativeReverseTrack_ClearSelection();
	return requestedLevelId;
}

static u32 NativeReverseTrack_Checksum(const void *source, int size)
{
	const u8 *bytes = (const u8 *)source;
	u32 hash = 2166136261u;
	for (int i = 0; i < size; i++)
	{
		hash ^= bytes[i];
		hash *= 16777619u;
	}
	return hash;
}

static void NativeReverseTrack_ResetHighScores(void)
{
	for (int trackIndex = 0; trackIndex < NATIVE_REVERSE_TRACK_COUNT; trackIndex++)
	{
		struct HighScoreTrack *track = &s_nativeReverseHighScores[trackIndex];
		memset(track, 0, sizeof(*track));

		for (int mode = 0; mode < MEMCARD_HIGH_SCORE_MODE_COUNT; mode++)
		{
			for (int entryIndex = 0; entryIndex < MEMCARD_HIGH_SCORE_ENTRIES_PER_MODE; entryIndex++)
			{
				int physicalId = s_nativeReverseTrackDefs[trackIndex].physicalId;
				int characterId = physicalId + mode + entryIndex;
				characterId = characterId - PENTA_PENGUIN * (characterId / PENTA_PENGUIN);

				struct HighScoreEntry *entry = &track->scoreEntry[mode * MEMCARD_HIGH_SCORE_ENTRIES_PER_MODE + entryIndex];
				entry->time = MEMCARD_HIGH_SCORE_DEFAULT_TIME;
				entry->characterID = characterId;
				if ((sdata != NULL) && (sdata->lngStrings != NULL))
				{
					strcpy(entry->name, sdata->lngStrings[data.MetaDataCharacters[characterId].name_LNG_short]);
				}
				else
				{
					strcpy(entry->name, "CTR");
				}
			}
		}
	}
}

static void NativeReverseTrack_EnsureHighScoresLoaded(void)
{
	if (s_nativeReverseHighScoresLoaded)
	{
		return;
	}

	struct NativeReverseScoreSave save;
	memset(&save, 0, sizeof(save));
	if ((NativeMemcard_ReadSaveData(NATIVE_REVERSE_SCORE_SAVE_NAME, (u8 *)&save, sizeof(save), 0) == NATIVE_MEMCARD_OK) &&
	    (save.magic == NATIVE_REVERSE_SCORE_MAGIC) &&
	    (save.version == NATIVE_REVERSE_SCORE_VERSION) &&
	    (save.checksum == NativeReverseTrack_Checksum(save.tracks, sizeof(save.tracks))))
	{
		memcpy(s_nativeReverseHighScores, save.tracks, sizeof(s_nativeReverseHighScores));
	}
	else
	{
		NativeReverseTrack_ResetHighScores();
	}

	s_nativeReverseHighScoresLoaded = true;
}

void NativeReverseTrack_SaveHighScores(void)
{
	if (!s_nativeReverseHighScoresLoaded)
	{
		return;
	}

	struct NativeReverseScoreSave save;
	memset(&save, 0, sizeof(save));
	save.magic = NATIVE_REVERSE_SCORE_MAGIC;
	save.version = NATIVE_REVERSE_SCORE_VERSION;
	memcpy(save.tracks, s_nativeReverseHighScores, sizeof(save.tracks));
	save.checksum = NativeReverseTrack_Checksum(save.tracks, sizeof(save.tracks));
	NativeMemcard_WriteSaveData(NATIVE_REVERSE_SCORE_SAVE_NAME, "", 0, (const u8 *)&save, sizeof(save));
}

struct HighScoreTrack *NativeReverseTrack_GetHighScoreTrack(s16 logicalId)
{
	int index = NativeReverseTrack_FindLogicalIndex(logicalId);
	if (index >= 0)
	{
		NativeReverseTrack_EnsureHighScoresLoaded();
		return &s_nativeReverseHighScores[index];
	}

	if ((logicalId >= 0) && (logicalId < MEMCARD_HIGH_SCORE_TRACK_COUNT))
	{
		return &sdata->gameProgress.highScoreTracks[logicalId];
	}
	return NULL;
}

static void NativeReverseTrack_ReflectDriverSpawns(struct Level *level)
{
	struct CheckpointNode *nodes = level->ptr_restart_points;
	u8 previousIndex = nodes[0].nextIndex_backward;
	if ((previousIndex == 0xff) || (previousIndex >= level->cnt_restart_points))
	{
		return;
	}

	s64 tangentX = (s64)nodes[0].pos.x - nodes[previousIndex].pos.x;
	s64 tangentZ = (s64)nodes[0].pos.z - nodes[previousIndex].pos.z;
	s64 lengthSquared = tangentX * tangentX + tangentZ * tangentZ;
	if (lengthSquared == 0)
	{
		return;
	}

	// Mirror the retail grid across the finish plane at checkpoint 0, keeping its original offset from the line.
	s64 linePointX2 = (s64)nodes[0].pos.x * 2;
	s64 linePointZ2 = (s64)nodes[0].pos.z * 2;

	for (int i = 0; i < 8; i++)
	{
		s64 deltaX2 = (s64)level->DriverSpawn[i].pos.x * 2 - linePointX2;
		s64 deltaZ2 = (s64)level->DriverSpawn[i].pos.z * 2 - linePointZ2;
		s64 projection2 = deltaX2 * tangentX + deltaZ2 * tangentZ;

		level->DriverSpawn[i].pos.x = (s16)((s64)level->DriverSpawn[i].pos.x - (projection2 * tangentX) / lengthSquared);
		level->DriverSpawn[i].pos.z = (s16)((s64)level->DriverSpawn[i].pos.z - (projection2 * tangentZ) / lengthSquared);
		level->DriverSpawn[i].rot.y = ANG_MODULO_TWO_PI(level->DriverSpawn[i].rot.y + ANG_PI);
	}
}

void NativeReverseTrack_ApplyToLevel(struct Level *level)
{
	if (!gNativeReverseTrackEnabled || !NativeReverseTrack_IsAllowedMode() || (level == NULL) ||
	    (sdata == NULL) || (sdata->gGT == NULL) ||
	    (sdata->gGT->levelID != s_nativeReverseTrackPhysicalId) ||
	    (level->ptr_restart_points == NULL) || (level->cnt_restart_points <= 0))
	{
		return;
	}

	struct CheckpointNode *nodes = level->ptr_restart_points;
	u16 trackLength = nodes[0].distToFinish;
	if (trackLength == 0)
	{
		return;
	}

	NativeReverseTrack_ReflectDriverSpawns(level);

	for (int i = 0; i < level->cnt_restart_points; i++)
	{
		u8 oldForward = nodes[i].nextIndex_forward;
		u8 oldLeft = nodes[i].nextIndex_left;
		u16 oldDistance = nodes[i].distToFinish;

		nodes[i].nextIndex_forward = nodes[i].nextIndex_backward;
		nodes[i].nextIndex_backward = oldForward;
		nodes[i].nextIndex_left = nodes[i].nextIndex_right;
		nodes[i].nextIndex_right = oldLeft;

		if (i == 0)
		{
			nodes[i].distToFinish = trackLength;
		}
		else if (oldDistance <= trackLength)
		{
			nodes[i].distToFinish = trackLength - oldDistance;
		}
	}

	printf("[CTR Reverse] Applied logical=%d physical=%d checkpoints=%d\n",
	       s_nativeReverseTrackLogicalId, s_nativeReverseTrackPhysicalId, level->cnt_restart_points);
}

const char *NativeReverseTrack_GetSuffix(void)
{
	extern int cfg_language;
	static const char *suffix[6] =
	{
		" REVERSE",
		" INVERSE",
		" RUECKWAERTS",
		" INVERSA",
		" INVERSA",
		" OMGEKEERD",
	};
	int languageRow = ((cfg_language >= 2) && (cfg_language <= 7)) ? cfg_language - 2 : 0;
	return suffix[languageRow];
}

void NativeReverseTrack_FormatName(s16 logicalId, char *dst, int dstSize)
{
	if ((dst == NULL) || (dstSize <= 0))
	{
		return;
	}

	s16 physicalId = NativeReverseTrack_GetPhysicalFromLogical(logicalId);
	const char *baseName = sdata->lngStrings[data.metaDataLEV[physicalId].name_LNG];
	if (NativeReverseTrack_IsLogicalReverse(logicalId))
	{
		snprintf(dst, dstSize, "%s%s", baseName, NativeReverseTrack_GetSuffix());
	}
	else
	{
		snprintf(dst, dstSize, "%s", baseName);
	}
}
