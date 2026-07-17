#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ac9fc-0x800aca34.
void MM_Cheat_MaxWumpa(void)
{
	sdata->gGT->gameMode2 |= CHEAT_WUMPA;
	OtherFX_Play(MM_CHEAT_SUCCESS_SFX, 1);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800aca34-0x800aca6c.
void MM_Cheat_UnlockRoo(void)
{
	sdata->gameProgress.unlockFlags |= UNLOCK_ROO;
	OtherFX_Play(MM_CHEAT_SUCCESS_SFX, 1);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800aca6c-0x800acaa4.
void MM_Cheat_UnlockPapu(void)
{
	sdata->gameProgress.unlockFlags |= UNLOCK_PAPU;
	OtherFX_Play(MM_CHEAT_SUCCESS_SFX, 1);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800acaa4-0x800acadc.
void MM_Cheat_UnlockJoe(void)
{
	sdata->gameProgress.unlockFlags |= UNLOCK_JOE;
	OtherFX_Play(MM_CHEAT_SUCCESS_SFX, 1);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800acadc-0x800acb14.
void MM_Cheat_UnlockPinstripe(void)
{
	sdata->gameProgress.unlockFlags |= UNLOCK_PINSTRIPE;
	OtherFX_Play(MM_CHEAT_SUCCESS_SFX, 1);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800acb14-0x800acb4c.
void MM_Cheat_UnlockFakeCrash(void)
{
	sdata->gameProgress.unlockFlags |= UNLOCK_FAKE_CRASH;
	OtherFX_Play(MM_CHEAT_SUCCESS_SFX, 1);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800acb4c-0x800acb84.
void MM_Cheat_UnlockPenta(void)
{
	sdata->gameProgress.unlockFlags |= UNLOCK_PENTA;
	OtherFX_Play(MM_CHEAT_SUCCESS_SFX, 1);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800acb84-0x800acbbc.
void MM_Cheat_UnlockTropy(void)
{
	sdata->gameProgress.unlockFlags |= UNLOCK_TROPY;
	OtherFX_Play(MM_CHEAT_SUCCESS_SFX, 1);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800acbbc-0x800acbf4.
void MM_Cheat_UnlockScrapbook(void)
{
	UNLOCK_ADV_BIT(sdata->gameProgress.unlocks, GAME_UNLOCK_BIT_SCRAPBOOK);
	OtherFX_Play(MM_CHEAT_SUCCESS_SFX, 1);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800acbf4-0x800acc2c.
void MM_Cheat_UnlockTracks(void)
{
	sdata->gameProgress.unlockFlags |= GAME_UNLOCK_TRACKS_MASK;
	OtherFX_Play(MM_CHEAT_SUCCESS_SFX, 1);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800acc2c-0x800acc64.
void MM_Cheat_InfiniteMasks(void)
{
	sdata->gGT->gameMode2 |= CHEAT_MASK;
	OtherFX_Play(MM_CHEAT_SUCCESS_SFX, 1);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800acc64-0x800acc9c.
void MM_Cheat_MaxTurbos(void)
{
	sdata->gGT->gameMode2 |= CHEAT_TURBO;
	OtherFX_Play(MM_CHEAT_SUCCESS_SFX, 1);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800acc9c-0x800accd4.
void MM_Cheat_MaxInvisibility(void)
{
	sdata->gGT->gameMode2 |= CHEAT_INVISIBLE;
	OtherFX_Play(MM_CHEAT_SUCCESS_SFX, 1);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800accd4-0x800acd10.
void MM_Cheat_MaxEngine(void)
{
	sdata->gGT->gameMode2 |= CHEAT_ENGINE;
	OtherFX_Play(MM_CHEAT_SUCCESS_SFX, 1);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800acd10-0x800acd4c.
void MM_Cheat_MaxBombs(void)
{
	sdata->gGT->gameMode2 |= CHEAT_BOMBS;
	OtherFX_Play(MM_CHEAT_SUCCESS_SFX, 1);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800acd4c-0x800acd88.
void MM_Cheat_AdvDifficulty(void)
{
	sdata->gGT->gameMode2 |= CHEAT_ADV;
	OtherFX_Play(MM_CHEAT_SUCCESS_SFX, 1);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800acd88-0x800acdc4.
void MM_Cheat_SuperHard(void)
{
	sdata->gGT->gameMode2 |= CHEAT_SUPERHARD;
	OtherFX_Play(MM_CHEAT_SUCCESS_SFX, 1);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800acdc4-0x800ace00.
void MM_Cheat_IcyTracks(void)
{
	sdata->gGT->gameMode2 |= CHEAT_ICY;
	OtherFX_Play(MM_CHEAT_SUCCESS_SFX, 1);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ace00-0x800ace3c.
void MM_Cheat_SuperTurboPads(void)
{
	sdata->gGT->gameMode2 |= CHEAT_TURBOPAD;
	OtherFX_Play(MM_CHEAT_SUCCESS_SFX, 1);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ace3c-0x800ace78.
void MM_Cheat_OneLap(void)
{
	sdata->gGT->gameMode2 |= CHEAT_ONELAP;
	OtherFX_Play(MM_CHEAT_SUCCESS_SFX, 1);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ace78-0x800aceb4.
void MM_Cheat_TurboCounter(void)
{
	sdata->gGT->gameMode2 |= CHEAT_TURBOCOUNT;
	OtherFX_Play(MM_CHEAT_SUCCESS_SFX, 1);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800aceb4-0x800acff4.
void MM_ParseCheatCodes(void)
{
	struct GamepadBuffer *gpad = &sdata->gGamepads->gamepad[0];

	// if not holding L1 and R1
	if ((gpad->buttonsHeldCurrFrame & (BTN_L1 | BTN_R1)) != (BTN_L1 | BTN_R1))
	{
		// skip function
		return;
	}

	u32 tap = (u32)gpad->buttonsTapped;
	if (tap == 0)
	{
		return;
	}

	// at this point, must be holding L1 and R1,
	// and also must have tapped a buttons

	// shift the loop
	for (s32 historyIndex = MM_CHEAT_BUTTON_HISTORY_COUNT - 1; historyIndex > 0; historyIndex--)
	{
		D230.cheatButtonHistory[historyIndex] = D230.cheatButtonHistory[historyIndex - 1];
	}

	// add to input
	D230.cheatButtonHistory[0] = tap;

	// loop through all cheats
	for (s32 cheatIndex = 0; cheatIndex < MM_CHEAT_COUNT; cheatIndex++)
	{
		b32 cheatMatches = true;

		// check if buttons match this cheat
		for (s32 buttonIndex = 0; buttonIndex < D230.cheats[cheatIndex].buttonCount; buttonIndex++)
		{
			// remember, inputButtons is backward
			s32 expectedButtonIndex = D230.cheats[cheatIndex].buttonCount - buttonIndex - 1;
			if ((D230.cheatButtonHistory[buttonIndex] & D230.cheats[cheatIndex].buttons[expectedButtonIndex]) == 0)
			{
				cheatMatches = false;
				break;
			}
		}

		// skip to next cheat if needed
		if (!cheatMatches)
		{
			continue;
		}

		if (D230.cheats[cheatIndex].handler != NULL)
		{
			D230.cheats[cheatIndex].handler();
		}
	}

	return;
}
