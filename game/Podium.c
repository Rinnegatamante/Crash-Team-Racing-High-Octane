#include <common.h>

enum PodiumConstants
{
	PODIUM_DRIVER_COUNT = 8,
	PODIUM_RANK_FIRST = 0,
	PODIUM_RANK_SECOND = 1,
	PODIUM_RANK_THIRD = 2,
};

CTR_STATIC_ASSERT(PODIUM_DRIVER_COUNT == 8);
CTR_STATIC_ASSERT(PODIUM_RANK_FIRST == 0);
CTR_STATIC_ASSERT(PODIUM_RANK_SECOND == 1);
CTR_STATIC_ASSERT(PODIUM_RANK_THIRD == 2);

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80041c84-0x80041dc0.
void Podium_InitModels(struct GameTracker *gGT)
{
	gGT->podium_modelIndex_First = 0;
	gGT->podium_modelIndex_Second = 0;
	gGT->podium_modelIndex_Third = 0;
	gGT->podium_modelIndex_tawna = STATIC_TAWNA1;

	u8 *podiumModelIndexArr = &gGT->podium_modelIndex_First;

	for (int i = 0; i < PODIUM_DRIVER_COUNT; i++)
	{
		struct Driver *driver = gGT->drivers[i];

		if (driver == NULL)
		{
			continue;
		}

		s16 rank = driver->driverRank;

		switch (rank)
		{
		case PODIUM_RANK_FIRST:
		case PODIUM_RANK_SECOND:
		case PODIUM_RANK_THIRD:
		{
			u8 characterID = data.characterIDs[driver->driverID];
			podiumModelIndexArr[rank] = characterID + STATIC_CRASHDANCE;

			if (rank != PODIUM_RANK_FIRST)
			{
				break;
			}

			switch (characterID)
			{
			case CRASH_BANDICOOT:
			case COCO_BANDICOOT:
				gGT->podium_modelIndex_tawna = STATIC_TAWNA2;
				break;

			case POLAR:
			case PURA:
				gGT->podium_modelIndex_tawna = STATIC_TAWNA3;
				break;

			case NEO_CORTEX:
			case N_GIN:
				gGT->podium_modelIndex_tawna = STATIC_TAWNA4;
				break;

			default:
				gGT->podium_modelIndex_tawna = STATIC_TAWNA1;
				break;
			}
			break;
		}

		default:
			break;
		}
	}
}
