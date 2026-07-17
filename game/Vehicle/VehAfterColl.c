#include <common.h>

enum
{
	VEH_AFTER_COLL_SCRUB_COUNT = len(data.MetaDataScrub),
	VEH_AFTER_COLL_TERRAIN_COUNT = len(data.MetaDataTerrain),
};

CTR_STATIC_ASSERT(VEH_AFTER_COLL_SCRUB_COUNT == 7);
CTR_STATIC_ASSERT(VEH_AFTER_COLL_TERRAIN_COUNT == TERRAIN_SLOWDIRT + 1);

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80057c44-0x80057c68.
struct Scrub *VehAfterColl_GetSurface(u32 scrubId)
{
	struct Scrub *sc = &data.MetaDataScrub[0];

	if (scrubId < VEH_AFTER_COLL_SCRUB_COUNT)
	{
		return &sc[scrubId];
	}

	return sc;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80057c68-0x80057c8c.
struct Terrain *VehAfterColl_GetTerrain(u8 terrainType)
{
	struct Terrain *ter = &data.MetaDataTerrain[0];

	// if terrain is valid, max 20
	if (terrainType < VEH_AFTER_COLL_TERRAIN_COUNT)
	{
		return &ter[terrainType];
	}

	return ter;
}
