#include <common.h>

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 231 0x800abfec-0x800ac0e4.
void RB_MinePool_Init(void)
{
	int i;

	LIST_Clear(&D231.minePoolTaken);
	LIST_Clear(&D231.minePoolFree);

	int gameMode = sdata->gGT->gameMode1;

	// default
	int numMines = 10;

	if ((gameMode & CRYSTAL_CHALLENGE) != 0)
	{
		// naughty dog bug, should be 50,
		// this caused nitro court challenge bug
		numMines = 40;
	}

	// boss race
	if ((gameMode & ADVENTURE_BOSS) != 0)
	{
		if (sdata->gGT->levelID == DRAGON_MINES)
		{
			numMines = 3;
		}
		if (sdata->gGT->levelID == ROO_TUBES)
		{
			numMines = 7;
		}
	}

	// add all mines
	for (i = 0; i < numMines; i++)
	{
		LIST_AddFront(&D231.minePoolFree, &D231.minePoolItem[i].item);
	}
}

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 231 0x800ac0e4-0x800ac13c.
void RB_MinePool_Remove(struct MineWeapon *mw)
{
	struct WeaponSlot231 *ws = mw->weaponSlot231;

	if (ws == NULL)
	{
		return;
	}

	// remove from taken, add to free
	LIST_RemoveMember(&D231.minePoolTaken, (struct Item *)ws);
	LIST_AddFront(&D231.minePoolFree, (struct Item *)ws);

	mw->boolDestroyed = 1;
	mw->weaponSlot231 = NULL;
}

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 231 0x800ac13c-0x800ac1b0.
void RB_MinePool_Add(struct MineWeapon *mw)
{
	// if no more items on free list
	if ((&D231.minePoolFree)->count == 0)
	{
		// remove oldest mine
		RB_MinePool_Remove(((struct WeaponSlot231 *)(&D231.minePoolTaken)->last)->mineWeapon);
	}

	// LIST_RemoveBack free list
	struct WeaponSlot231 *ws = (struct WeaponSlot231 *)LIST_RemoveBack(&D231.minePoolFree);

	// link together
	ws->mineWeapon = mw;
	mw->weaponSlot231 = ws;

	// LIST_AddFront to taken list
	LIST_AddFront(&D231.minePoolTaken, (struct Item *)ws);
}
