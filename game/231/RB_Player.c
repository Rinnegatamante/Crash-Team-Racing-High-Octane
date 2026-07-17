#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800abbb4-0x800abefc.
void RB_Player_KillPlayer(struct Driver *attacker, struct Driver *victim)
{
	struct GameTracker *gGT = sdata->gGT;
	u32 gameMode = gGT->gameMode1;
	u8 numPlyr = gGT->numPlyrCurrGame;

	if ((gameMode & BATTLE_MODE) == 0)
	{
		return;
	}

	if (attacker == NULL)
	{
		return;
	}

	if (victim == NULL)
	{
		return;
	}

	if ((gameMode & POINT_LIMIT) != 0)
	{
		int attackerTeam = attacker->BattleHUD.teamID;
		int victimTeam = victim->BattleHUD.teamID;

		if (victimTeam == attackerTeam)
		{
			int score = gGT->battleSetup.pointsPerTeam[victimTeam] - 1;

			if (score < -9)
			{
				return;
			}

			gGT->battleSetup.pointsPerTeam[victimTeam] = score;
			return;
		}

		int score = gGT->battleSetup.pointsPerTeam[attackerTeam] + 1;

		if (score < 100)
		{
			gGT->battleSetup.pointsPerTeam[attackerTeam] = score;
		}

		if (gGT->battleSetup.pointsPerTeam[attackerTeam] != gGT->battleSetup.killLimit)
		{
			return;
		}

		if ((gameMode & TIME_LIMIT) != 0)
		{
			return;
		}

		for (int i = 0; i < numPlyr; i++)
		{
			gGT->drivers[i]->actionsFlagSet |= ACTION_RACE_FINISHED;
		}
	}
	else
	{
		if ((gameMode & LIFE_LIMIT) == 0)
		{
			return;
		}

		int lives = victim->BattleHUD.numLives - 1;

		if (lives > 0)
		{
			victim->BattleHUD.numLives = lives;
			return;
		}

		s16 isTeamAlive[4];
		memset(isTeamAlive, 0, sizeof(isTeamAlive));

		int deadPlayers = 0;
		s16 teamsAlive = 0;

		victim->funcPtrs[DRIVER_FUNC_INIT] = VehStuckProc_RIP_Init;
		victim->BattleHUD.numLives = 0;
		victim->actionsFlagSet |= ACTION_RACE_FINISHED;

		for (int i = 0; i < numPlyr; i++)
		{
			struct Driver *driver = gGT->drivers[i];

			if ((driver->actionsFlagSet & ACTION_RACE_FINISHED) == 0)
			{
				isTeamAlive[driver->BattleHUD.teamID] = 1;
			}
			else
			{
				deadPlayers++;
			}
		}

		int victimTeam = victim->BattleHUD.teamID;

		if (((gGT->battleSetup.teamFlags & (1 << victimTeam)) != 0) && (isTeamAlive[victimTeam] == 0))
		{
			int remainingPlayers = numPlyr - deadPlayers;

			if (remainingPlayers < 3)
			{
				gGT->standingsPoints[victimTeam * 3 + remainingPlayers]++;
			}

			gGT->battleSetup.finishedRankOfEachTeam[victimTeam] = remainingPlayers;
		}

		for (int team = 0; team < 4; team++)
		{
			if (((gGT->battleSetup.teamFlags & (1 << team)) != 0) && (isTeamAlive[team] != 0))
			{
				teamsAlive++;
			}
		}

		if (teamsAlive != 1)
		{
			return;
		}

		for (int i = 0; i < numPlyr; i++)
		{
			gGT->drivers[i]->actionsFlagSet |= ACTION_RACE_FINISHED;
		}
	}

	MainGameEnd_Initialize();
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800abefc-0x800abfec.
void RB_Player_ModifyWumpa(struct Driver *driver, int wumpaDelta)
{
	s8 numWumpaOriginal = driver->numWumpas;

	// if using unlimited wumpa, quit
	if ((sdata->gGT->gameMode2 & CHEAT_WUMPA) != 0)
	{
		return;
	}

	if (
	    // if wumpa is being subtracted
	    (wumpaDelta < 0) &&

	    // using mask weapon
	    ((driver->actionsFlagSet & ACTION_MASK_WEAPON) != 0))
	{
		// quit, dont lose wumpa
		return;
	}

	if (
	    // wumpa increasing
	    (wumpaDelta > 0) &&

	    // driver is not an AI
	    ((driver->actionsFlagSet & ACTION_BOT) == 0))
	{
		// for end-of-race comments
		driver->numTimesWumpa += wumpaDelta;
	}

	// works for positive and negative delta
	driver->numWumpas += wumpaDelta;

	// dont allow negatives
	if (driver->numWumpas < 0)
	{
		driver->numWumpas = 0;
	}

	// cap at max wumpa count
	if (driver->numWumpas > DRIVER_WUMPA_MAX_COUNT)
	{
		driver->numWumpas = DRIVER_WUMPA_MAX_COUNT;
	}

	if (
	    // if did not have juiced wumpa before
	    (numWumpaOriginal < DRIVER_WUMPA_JUICED_COUNT) &&

	    // if have juiced wumpa now
	    (driver->numWumpas == DRIVER_WUMPA_JUICED_COUNT))
	{
		// Play "juiced up" sound
		OtherFX_Play(0x41, 1);

		driver->BattleHUD.juicedUpCooldown = DRIVER_WUMPA_JUICED_HUD_COOLDOWN_FRAMES;
	}
}

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 231 0x800b0dbc-0x800b0e68.
void RB_Player_ToggleInvisible(void)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Driver *d;
	struct Thread *t;

	// loop through player threads
	for (t = gGT->threadBuckets[PLAYER].thread; t != NULL; t = t->siblingThread)
	{
		// driver object
		d = t->object;

		// if driver is invisible
		if (d->invisibleTimer != 0)
		{
			// loop through InstanceDrawPerPlayer
			for (int i = 0; i < gGT->numPlyrCurrGame; i++)
			{
				// if this is not the screen of the invisible driver
				if (i != d->driverID)
				{
					struct InstDrawPerPlayer *idpp = INST_GETIDPP(d->instSelf);

					// make driver instance invisible on this screen
					idpp[i].instFlags &= 0xffffffbf;
				}
			}
		}
	}
	return;
}

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 231 0x800b0e68-0x800b0f1c.
void RB_Player_ToggleFlicker(void)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Thread *t;
	struct Driver *d;

	for (t = gGT->threadBuckets[PLAYER].thread; t != NULL; t = t->siblingThread)
	{
		// driver object
		d = t->object;

		if (
		    // invincible timer
		    (0x2a0 < d->invincibleTimer) &&

		    // odd number frames
		    ((gGT->timer & 1) != 0))
		{
			struct InstDrawPerPlayer *idpp = INST_GETIDPP(d->instSelf);

			// on all screens
			for (int i = 0; i < gGT->numPlyrCurrGame; i++)
			{
				// make driver invisible
				idpp[i].instFlags &= 0xffffffbf;
			}
		}
	}
	return;
}
