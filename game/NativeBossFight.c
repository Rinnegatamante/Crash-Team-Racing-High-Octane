#include <common.h>

enum
{
    NATIVE_BOSS_FIGHT_COUNT = 6,
    NATIVE_BOSS_FIGHT_STANDARD_META_COUNT = 5,
    NATIVE_BOSS_FIGHT_OXIDE_META_COUNT = 13,
};

int gNativeBossFightMode = 0;
int gNativeBossFightBossID = 0;

static const s16 s_nativeBossFightCharacters[NATIVE_BOSS_FIGHT_COUNT] =
{
    RIPPER_ROO,
    PAPU_PAPU,
    KOMODO_JOE,
    PINSTRIPE,
    NITROS_OXIDE,
    NITROS_OXIDE,
};

int NativeBossFight_GetBossCharacter(int bossID)
{
    if ((u32)bossID >= NATIVE_BOSS_FIGHT_COUNT)
    {
        bossID = 0;
    }

    return s_nativeBossFightCharacters[bossID];
}

void NativeBossFight_SelectBoss(int bossID)
{
    if ((u32)bossID >= NATIVE_BOSS_FIGHT_COUNT)
    {
        bossID = 0;
    }

    gNativeBossFightBossID = bossID;
    sdata->gGT->bossID = bossID;
    data.characterIDs[1] = NativeBossFight_GetBossCharacter(bossID);
}

void NativeBossFight_ArmGameplay(void)
{
    if ((gNativeBossFightMode == 0) || (sdata == NULL) || (sdata->gGT == NULL))
    {
        return;
    }

    struct GameTracker *gGT = sdata->gGT;
    gGT->gameMode1 &= ~ARCADE_MODE;
    gGT->gameMode1 |= ADVENTURE_BOSS;
    gGT->bossID = gNativeBossFightBossID;
    data.characterIDs[1] = NativeBossFight_GetBossCharacter(gNativeBossFightBossID);
}

void NativeBossFight_BeginPostRace(void)
{
    if ((gNativeBossFightMode == 0) || (sdata == NULL) || (sdata->gGT == NULL))
    {
        return;
    }

    sdata->gGT->gameMode1 &= ~ADVENTURE_BOSS;
    sdata->gGT->gameMode1 |= ARCADE_MODE;
}

void NativeBossFight_Clear(void)
{
    gNativeBossFightMode = 0;
    gNativeBossFightBossID = 0;

    if ((sdata != NULL) && (sdata->gGT != NULL))
    {
        sdata->gGT->gameMode1 &= ~(ADVENTURE_BOSS | ARCADE_MODE);
    }
}

struct MetaDataBOSS *NativeBossFight_GetWeaponMeta(int bossID)
{
    if ((u32)bossID >= NATIVE_BOSS_FIGHT_COUNT)
    {
        bossID = 0;
    }

    if (bossID >= 4)
    {
        return data.bossWeaponMetaPtr[0];
    }

    return data.bossWeaponMetaPtr[bossID + 1];
}

int NativeBossFight_GetWeaponMetaCount(int bossID)
{
    if (bossID >= 4)
    {
        return NATIVE_BOSS_FIGHT_OXIDE_META_COUNT;
    }

    return NATIVE_BOSS_FIGHT_STANDARD_META_COUNT;
}

