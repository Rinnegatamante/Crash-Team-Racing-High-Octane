#include <common.h>


#ifdef CTR_NATIVE
enum MMNativeLanguageConstants
{
	MM_NATIVE_LANGUAGE_COUNT = 6,
	MM_NATIVE_LANGUAGE_TIMEOUT_FRAMES = CTR_SECONDS_TO_FRAMES(30),
	MM_NATIVE_LANGUAGE_DRAWSTYLE_WIDESCREEN = 0x200,
};

static const s16 s_nativeLanguageFileIndex[MM_NATIVE_LANGUAGE_COUNT] =
{
	2, // English (PAL/UK)
	3, // French
	4, // German
	5, // Italian
	6, // Spanish
	7, // Dutch
};


enum MMNativeExtraDifficultyConstants
{
	MM_NATIVE_DIFFICULTY_EASY = 0,
	MM_NATIVE_DIFFICULTY_MEDIUM,
	MM_NATIVE_DIFFICULTY_HARD,
	MM_NATIVE_DIFFICULTY_SUPER_HARD,
	MM_NATIVE_DIFFICULTY_ULTRA_HARD,
	MM_NATIVE_DIFFICULTY_COUNT,
};

static const char *s_nativeExtraDifficultyText[MM_NATIVE_LANGUAGE_COUNT][2] =
{
	{"SUPER HARD", "ULTRA HARD"},
	{"SUPER DIFFICILE", "ULTRA DIFFICILE"},
	{"SUPER SCHWER", "ULTRA SCHWER"},
	{"SUPER DIFFICILE", "ULTRA DIFFICILE"},
	{"SUPER DIFICIL", "ULTRA DIFICIL"},
	{"SUPER MOEILIJK", "ULTRA MOEILIJK"},
};

static const char *s_nativeMirrorModeText[MM_NATIVE_LANGUAGE_COUNT][2] =
{
	{"MIRROR: OFF", "MIRROR: ON"},
	{"MIROIR: NON", "MIROIR: OUI"},
	{"SPIEGEL: AUS", "SPIEGEL: EIN"},
	{"SPECCHIO: NO", "SPECCHIO: SI"},
	{"ESPEJO: NO", "ESPEJO: SI"},
	{"SPIEGEL: UIT", "SPIEGEL: AAN"},
};

static struct MenuRow s_nativeExtraDifficultyRows[MM_NATIVE_DIFFICULTY_COUNT + 1] =
{
	{LNG_EASY, 0, 1, 0, 0},
	{LNG_MEDIUM, 0, 2, 1, 1},
	{LNG_HARD, 1, 3, 2, 2},
	{LNG_NA_241, 2, 4, 3, 3},
	{LNG_NA_242, 3, 4, 4, 4},
	{RECTMENU_STRING_NONE},
};

static struct RectMenu s_nativeExtraDifficultyMenu =
{
	.stringIndexTitle = LNG_DIFFICULTY,
	.state = CENTER_ON_X | USE_SMALL_FONT | BIG_TEXT_IN_TITLE,
	.rows = s_nativeExtraDifficultyRows,
	.funcPtr = MM_MenuProc_Difficulty,
};

static struct MenuRow s_nativeLanguageRows[MM_NATIVE_LANGUAGE_COUNT + 1] =
{
	{LNG_ENGLISH, 0, 1, 0, 0},
	{LNG_FRENCH, 0, 2, 1, 1},
	{LNG_GERMAN, 1, 3, 2, 2},
	{LNG_ITALIAN, 2, 4, 3, 3},
	{LNG_SPANISH, 3, 5, 4, 4},
	{LNG_DUTCH, 4, 5, 5, 5},
	{RECTMENU_STRING_NONE},
};

static struct MenuRow s_nativeMainMenuBasic[] =
{
	{LNG_ADVENTURE, 0, 1, 0, 0},
	{LNG_TIME_TRIAL, 0, 2, 1, 1},
	{LNG_ARCADE, 1, 3, 2, 2},
	{LNG_VS, 2, 4, 3, 3},
	{LNG_BATTLE, 3, 5, 4, 4},
	{LNG_HIGH_SCORE, 4, 6, 5, 5},
	{LNG_LANGUAGE, 5, 7, 6, 6},
	{LNG_NA_243, 6, 7, 7, 7},
	{RECTMENU_STRING_NONE},
};

static struct MenuRow s_nativeMainMenuWithScrapbook[] =
{
	{LNG_ADVENTURE, 0, 1, 0, 0},
	{LNG_TIME_TRIAL, 0, 2, 1, 1},
	{LNG_ARCADE, 1, 3, 2, 2},
	{LNG_VS, 2, 4, 3, 3},
	{LNG_BATTLE, 3, 5, 4, 4},
	{LNG_HIGH_SCORE, 4, 6, 5, 5},
	{LNG_LANGUAGE, 5, 7, 6, 6},
	{LNG_NA_243, 6, 8, 7, 7},
	{LNG_SCRAPBOOK, 7, 8, 8, 8},
	{RECTMENU_STRING_NONE},
};

static void MM_NativeLanguageBootMenuProc(struct RectMenu *menu);
static void MM_NativeLanguageMainMenuProc(struct RectMenu *menu);

static struct RectMenu s_nativeLanguageBootMenu =
{
	.stringIndexTitle = RECTMENU_STRING_NONE,
	.posX_curr = 256,
	.posY_curr = 118,
	.state = RECTMENU_STATE_EXEC_CENTERED,
	.rows = s_nativeLanguageRows,
	.funcPtr = MM_NativeLanguageBootMenuProc,
#if CTR_VITA_WIDESCREEN
	.drawStyle = MM_NATIVE_LANGUAGE_DRAWSTYLE_WIDESCREEN,
#endif
};

static struct RectMenu s_nativeLanguageMainMenu =
{
	.stringIndexTitle = RECTMENU_STRING_NONE,
	.state = CENTER_ON_X,
	.rows = s_nativeLanguageRows,
	.funcPtr = MM_NativeLanguageMainMenuProc,
};

s32 s_nativeLanguageChosen = 0;
static s32 s_nativeLanguageTimer;
static s16 s_nativeLanguageRow;
extern int cfg_language;
extern int gNativeMirrorModeEnabled;
extern void save_config();

static void MM_NativeMirrorModeApplyText(void)
{
	s16 languageRow = 0;

	if ((cfg_language >= 2) && (cfg_language <= 7))
	{
		languageRow = (s16)(cfg_language - 2);
	}

	if ((sdata->lngStrings != NULL) && (sdata->numLngStrings > LNG_NA_243))
	{
		sdata->lngStrings[LNG_NA_243] =
			(char *)s_nativeMirrorModeText[languageRow][gNativeMirrorModeEnabled != 0];
	}
}

static void MM_NativeExtraDifficultyApplyText(void)
{
	s16 languageRow = 0;

	if ((cfg_language >= 2) && (cfg_language <= 7))
	{
		languageRow = (s16)(cfg_language - 2);
	}

	if ((sdata->lngStrings != NULL) && (sdata->numLngStrings > LNG_NA_242))
	{
		sdata->lngStrings[LNG_NA_241] = (char *)s_nativeExtraDifficultyText[languageRow][0];
		sdata->lngStrings[LNG_NA_242] = (char *)s_nativeExtraDifficultyText[languageRow][1];
	}
}

static void MM_NativeExtraDifficultyPrepare(void)
{
	MM_NativeExtraDifficultyApplyText();

	s_nativeExtraDifficultyMenu = (struct RectMenu)
	{
		.stringIndexTitle = LNG_DIFFICULTY,
		.state = CENTER_ON_X | USE_SMALL_FONT | BIG_TEXT_IN_TITLE,
		.rows = s_nativeExtraDifficultyRows,
		.funcPtr = MM_MenuProc_Difficulty,
	};
}

static void MM_NativeLanguageLoad(s16 row)
{
	if ((u16)row >= MM_NATIVE_LANGUAGE_COUNT)
	{
		row = 0;
	}

	cfg_language = s_nativeLanguageFileIndex[row];
	LOAD_LangFile((int)sdata->ptrBigfile1, cfg_language);
	MM_NativeExtraDifficultyApplyText();
	MM_NativeMirrorModeApplyText();

	s_nativeLanguageRow = row;
	s_nativeLanguageChosen = 1;
	save_config();
}

static void MM_NativeLanguageBootMenuProc(struct RectMenu *menu)
{
	if (menu->funcState == RECTMENU_FUNC_STATE_UPDATE)
	{
		if (sdata->gGamepads->anyoneHeldCurr != 0)
		{
			s_nativeLanguageTimer = MM_NATIVE_LANGUAGE_TIMEOUT_FRAMES;
		}
		else if (s_nativeLanguageTimer > 0)
		{
			s_nativeLanguageTimer--;
		}

		if (s_nativeLanguageTimer == 0)
		{
			MM_NativeLanguageLoad(menu->rowSelected);
			sdata->ptrDesiredMenu = &D230.menuMainMenu;
		}
		return;
	}

	if ((menu->funcState != RECTMENU_FUNC_STATE_INPUT) || (menu->rowSelected < 0))
	{
		return;
	}

	MM_NativeLanguageLoad(menu->rowSelected);
	sdata->ptrDesiredMenu = &D230.menuMainMenu;
}

static void MM_NativeLanguageMainMenuProc(struct RectMenu *menu)
{
	if (menu->funcState != RECTMENU_FUNC_STATE_INPUT)
	{
		return;
	}

	struct RectMenu *parent = menu->ptrPrevBox_InHierarchy;
	if (parent == NULL)
	{
		return;
	}

	if (menu->rowSelected >= 0)
	{
		MM_NativeLanguageLoad(menu->rowSelected);
	}

	parent->state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
}
#endif

// NOTE(aalhendi): ASM-verified against retail 230 0x800abaf0-0x800abcac.
u8 MM_TransitionInOut(struct TransitionMeta *meta, int framesPassed, int numFrames)
{
	u8 allTransitionsDone = 1;
	int transitionIndex = 0;

	// last member of array is null-terminated with 0xFFFF
	for (/**/; meta->headStart > -1; meta++, transitionIndex++)
	{
		s16 start = meta->headStart;
		s16 framesLeft = ((s16)framesPassed - start);

		if ((framesLeft == MM_TRANSITION_SWISH_FRAME) && (transitionIndex == 0))
		{
			// Play "swoosh" sound for menu transition
			OtherFX_Play(MM_TRANSITION_SWISH_SFX, 0);
		}

		if (framesLeft < 1)
		{
			allTransitionsDone = 0;
			meta->currX = 0;
			meta->currY = 0;
			continue;
		}

		// else if
		if (framesLeft < (s16)numFrames)
		{
			allTransitionsDone = 0;
			meta->currX = framesLeft * meta->distX / (s16)numFrames;
			meta->currY = framesLeft * meta->distY / (s16)numFrames;
			continue;
		}

		// else
		meta->currX = meta->distX;
		meta->currY = meta->distY;
	}
	return allTransitionsDone;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800acff4-0x800ad448.
void MM_MenuProc_Main(struct RectMenu *mainMenu)
{
	struct GameTracker *gGT = sdata->gGT;

#if defined(CTR_NATIVE)
	MM_NativeMirrorModeApplyText();

	if (CHECK_ADV_BIT(sdata->gameProgress.unlocks, GAME_UNLOCK_BIT_SCRAPBOOK))
	{
		mainMenu->rows = s_nativeMainMenuWithScrapbook;
	}
	else
	{
		mainMenu->rows = s_nativeMainMenuBasic;
	}
#else
	// if scrapbook is unlocked, change "rows" to extended array
	if (CHECK_ADV_BIT(sdata->gameProgress.unlocks, GAME_UNLOCK_BIT_SCRAPBOOK))
	{
		mainMenu->rows = &D230.rowsMainMenuWithScrapbook[0];
	}
#endif

	MM_ParseCheatCodes();
	MM_ToggleRows_Difficulty();
	MM_ToggleRows_PlayerCount();

	// If you are at the highest hierarchy level of main menu
	if (mainMenu->funcState == RECTMENU_FUNC_STATE_UPDATE)
	{
		MM_Title_MenuUpdate();

		if (
		    // main menu, "title" exists, and timer >= 230
		    (D230.titleMenuState == TITLE_MENU_STATE_IN_MENU) && (D230.titleObj != NULL) && (TITLE_INTRO_TM_DRAW_MIN_FRAME < D230.titleIntroFrame))
		{
			DecalFont_DrawLineOT(sdata->lngStrings[LNG_TM], MM_TITLE_TM_X, MM_TITLE_TM_Y, FONT_SMALL, ORANGE,
			                     &gGT->backBuffer->otMem.uiOT[MM_TITLE_TM_OT_INDEX]);
		}

		if ((D230.menuMainMenu.state & DRAW_NEXT_MENU_IN_HIERARCHY) == 0)
		{
			gGT->numPlyrNextGame = 1;

			// if no buttons pressed, check demo mode
			if (sdata->gGamepads->anyoneHeldCurr == 0)
			{
				gGT->demoCountdownTimer--;

				// If time runs out
				if (gGT->demoCountdownTimer < 1)
				{
					// Transition out of main menu
					D230.titleMenuState = TITLE_MENU_STATE_EXITING;

					// Go to a cutscene of some kind, either the Oxide intro
					// or a demo-mode race.
					D230.desiredMenuIndex = MM_EXIT_ROUTE_DEMO;
				}
			}

			// if button pressed, reset timer
			else
			{
				gGT->demoCountdownTimer = TITLE_DEMO_IDLE_FRAMES;
			}
		}
	}

	MM_Title_Init();

	// if drawing ptrNextBox_InHierarchy
	if ((mainMenu->state & DRAW_NEXT_MENU_IN_HIERARCHY) != 0)
	{
		D230.titleIntroFrame = TITLE_INTRO_SKIP_FRAME;
	}

	// if funcPtr is null
	if ((mainMenu->state & EXECUTE_FUNCPTR) == 0)
	{
		return;
	}

	struct Title *titleObj = D230.titleObj;

	// if "title" object exists
	if (titleObj != NULL)
	{
		// CameraPosOffset X
		titleObj->cameraPosOffset.x = 0;
	}

	// if you are at highest level of menu hierarchy
	if (mainMenu->funcState != RECTMENU_FUNC_STATE_INPUT)
	{
		// leave the function
		return;
	}

	// If you are here, then you must not be
	// at the highest level of menu hierarchy

	// if row is negative, do nothing
	if ((mainMenu->rowSelected) < 0)
	{
		return;
	}

	// clear flags from game mode
	gGT->gameMode1 &= ~(BATTLE_MODE | ADVENTURE_MODE | TIME_TRIAL | ADVENTURE_ARENA | ARCADE_MODE | ADVENTURE_CUP);

	// clear more game mode flags
	gGT->gameMode2 &= ~(CUP_ANY_KIND);

	mainMenu->state |= ONLY_DRAW_TITLE;

	// Default to 3,
	// this intentionally disables the 1-lap cheat
	// in Time Trial and Adventure, DONT change it
	gGT->numLaps = MM_DEFAULT_LAP_COUNT;

	// get LNG index of row selected
	s16 choose = mainMenu->rows[mainMenu->rowSelected].stringIndex;

	// Adventure Mode
	if (choose == LNG_ADVENTURE)
	{
		// Turn on Adventure Mode, turn off item cheats
		gGT->gameMode1 |= ADVENTURE_MODE;
		gGT->gameMode2 &= ~(CHEAT_WUMPA | CHEAT_MASK | CHEAT_TURBO | CHEAT_ENGINE | CHEAT_BOMBS);

		// menu for new/load
		mainMenu->ptrNextBox_InHierarchy = &D230.menuAdventure;
		mainMenu->state |= DRAW_NEXT_MENU_IN_HIERARCHY;
		return;
	}

	// Time Trial
	if (choose == LNG_TIME_TRIAL)
	{
		// Leave main menu hierarchy
		D230.titleMenuState = TITLE_MENU_STATE_EXITING;

		// Leave through the normal character-select flow.
		D230.desiredMenuIndex = MM_EXIT_ROUTE_CHARACTER_SELECT;

		// set game mode to Time Trial Mode
		gGT->numPlyrNextGame = 1;
		gGT->gameMode1 |= TIME_TRIAL;
		gGT->gameMode2 &= ~(CHEAT_WUMPA | CHEAT_MASK | CHEAT_TURBO | CHEAT_ENGINE | CHEAT_BOMBS);

		return;
	}

	// Arcade Mode
	if (choose == LNG_ARCADE)
	{
		// DONT change, should only work in Arcade, and VS
		if ((gGT->gameMode2 & CHEAT_ONELAP) != 0)
		{
			gGT->numLaps = MM_ONE_LAP_CHEAT_COUNT;
		}

		// set game mode to Arcade Mode
		gGT->gameMode1 |= ARCADE_MODE;

		// set next menu
		mainMenu->ptrNextBox_InHierarchy = &D230.menuRaceType;
		mainMenu->state |= DRAW_NEXT_MENU_IN_HIERARCHY;
		return;
	}

	// Versus
	if (choose == LNG_VS)
	{
		// DONT change, should only work in Arcade, and VS
		if ((gGT->gameMode2 & CHEAT_ONELAP) != 0)
		{
			gGT->numLaps = MM_ONE_LAP_CHEAT_COUNT;
		}

		// next menu is choosing single+cup
		mainMenu->ptrNextBox_InHierarchy = &D230.menuRaceType;
		mainMenu->state |= DRAW_NEXT_MENU_IN_HIERARCHY;
		return;
	}

	// Battle
	if (choose == LNG_BATTLE)
	{
		D230.characterSelectTransitionState = EXITING_MENU;

		// set game mode to Battle Mode
		gGT->gameMode1 |= BATTLE_MODE;

		// set next menu to 2P,3P,4P
		mainMenu->ptrNextBox_InHierarchy = &D230.menuPlayers2P3P4P;
		mainMenu->state |= DRAW_NEXT_MENU_IN_HIERARCHY;
		return;
	}

	// High Score
	if (choose == LNG_HIGH_SCORE)
	{
		// Set next stage to high score menu
		D230.desiredMenuIndex = MM_EXIT_ROUTE_HIGH_SCORE;

		// Leave main menu hierarchy
		D230.titleMenuState = TITLE_MENU_STATE_EXITING;

		return;
	}

#if defined(CTR_NATIVE)
	// Language
	if (choose == LNG_LANGUAGE)
	{
		s_nativeLanguageMainMenu.rowSelected = s_nativeLanguageRow;
		s_nativeLanguageMainMenu.ptrNextBox_InHierarchy = NULL;
		s_nativeLanguageMainMenu.ptrPrevBox_InHierarchy = mainMenu;

		mainMenu->ptrNextBox_InHierarchy = &s_nativeLanguageMainMenu;
		mainMenu->state |= DRAW_NEXT_MENU_IN_HIERARCHY;
		return;
	}

	// Mirror Mode
	if (choose == LNG_NA_243)
	{
		gNativeMirrorModeEnabled ^= 1;
		MM_NativeMirrorModeApplyText();
		save_config();
		mainMenu->state &= ~ONLY_DRAW_TITLE;
		return;
	}
#endif

	// Scrapbook
	if (choose == LNG_SCRAPBOOK)
	{
		// Set next stage to Scrapbook
		D230.desiredMenuIndex = MM_EXIT_ROUTE_SCRAPBOOK;

		// Leave main menu hierarchy
		D230.titleMenuState = TITLE_MENU_STATE_EXITING;

		return;
	}
}

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 230 0x800ad448-0x800ad560.
void MM_ToggleRows_PlayerCount(void)
{
	for (s32 rowIndex = 0; rowIndex < MM_PLAYER_1P2P_SELECTABLE_ROWS; rowIndex++)
	{
		struct MenuRow *row = &D230.rowsPlayers1P2P[rowIndex];

		// unlock row
		row->stringIndex &= MENU_ROW_LNG_MASK;

		if (!MainFrame_HaveAllPads(rowIndex + 1))
		{
			// lock row
			row->stringIndex |= MENU_ROW_LOCKED;
		}
	}

	for (s32 rowIndex = 0; rowIndex < MM_PLAYER_2P3P4P_SELECTABLE_ROWS; rowIndex++)
	{
		struct MenuRow *row = &D230.rowsPlayers2P3P4P[rowIndex];

		// unlock row
		row->stringIndex &= MENU_ROW_LNG_MASK;

		if (!MainFrame_HaveAllPads(rowIndex + 2))
		{
			// lock row
			row->stringIndex |= MENU_ROW_LOCKED;
		}
	}
}

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 230 0x800ad560-0x800ad5e8.
void MM_MenuProc_1p2p(struct RectMenu *menu)
{
	struct GameTracker *gGT = sdata->gGT;
	s16 row = menu->rowSelected;

	// if uninitialized
	if (row == -1)
	{
		menu->ptrPrevBox_InHierarchy->state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);

		gGT->numPlyrNextGame = 1;

		D230.characterSelectTransitionState = ENTERING_MENU;
	}

	else
	{
		// if on row 0 or 1
		if ((row >= 0) && (row < MM_PLAYER_1P2P_SELECTABLE_ROWS))
		{
			// row 0 is 1P, row 1 is 2P
			gGT->numPlyrNextGame = menu->rowSelected + 1;

			// go to difficulty box
#if defined(CTR_NATIVE)
			MM_NativeExtraDifficultyPrepare();
			menu->ptrNextBox_InHierarchy = &s_nativeExtraDifficultyMenu;
#else
			menu->ptrNextBox_InHierarchy = &D230.menuDifficulty;
#endif

			menu->state |= ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY;
			return;
		}
	}
	return;
}

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 230 0x800ad5e8-0x800ad678.
void MM_MenuProc_2p3p4p(struct RectMenu *menu)
{
	struct GameTracker *gGT = sdata->gGT;
	s16 row = menu->rowSelected;

	// if uninitialized
	if (row == -1)
	{
		menu->ptrPrevBox_InHierarchy->state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);

		gGT->numPlyrNextGame = 1;

		D230.characterSelectTransitionState = ENTERING_MENU;
	}
	else
	{
		// row is 0, 1, 2
		if ((row >= 0) && (row < MM_PLAYER_2P3P4P_SELECTABLE_ROWS))
		{
			// row 0 is 2P, row 1 is 3P, row 2 is 4P
			gGT->numPlyrNextGame = menu->rowSelected + 2;

			D230.titleMenuState = TITLE_MENU_STATE_EXITING;
			D230.desiredMenuIndex = MM_EXIT_ROUTE_CHARACTER_SELECT;

			menu->state |= ONLY_DRAW_TITLE;
			return;
		}
	}
	return;
}

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 230 0x800ad678-0x800ad7a4.
void MM_ToggleRows_Difficulty(void)
{
	struct GameTracker *gGT = sdata->gGT;

	// check 3 mods (easy, medium, hard)
	for (s32 difficultyIndex = 0; difficultyIndex < MM_DIFFICULTY_COUNT; difficultyIndex++)
	{
		s16 bitIndex = D230.cupDifficulty.firstUnlockBit[difficultyIndex];

		// if -1 (for EASY row), skip
		if (-1 == bitIndex)
		{
			continue;
		}

		// assume unlocked
		u32 isUnlocked = 1;

		// check 4 bits starting at bitIndex,
		// one for each track in cup
		for (s32 trackIndex = 0; trackIndex < MM_CUP_TRACK_COUNT; trackIndex++)
		{
			b32 shouldCheckNextTrack = (isUnlocked != 0);
			isUnlocked = 0;

			// if not determined locked
			if (shouldCheckNextTrack)
			{
				s32 unlockBit = (s32)bitIndex + trackIndex;

				// check what is unlocked
				isUnlocked = CHECK_ADV_BIT(sdata->gameProgress.unlocks, unlockBit);
			}
		}

		// get current value of lng index,
		// for easy, medium, hard
		u16 lngIndex = D230.cupDifficulty.stringIndex[difficultyIndex];

		if (
		    // if locked
		    (isUnlocked == 0) &&

		    // If you're in Arcade mode
		    ((gGT->gameMode1 & ARCADE_MODE) != 0) &&

		    // if you are in Arcade or VS cup
		    ((gGT->gameMode2 & CUP_ANY_KIND) != 0))
		{
			// use high bits for "LOCKED"
			lngIndex |= MENU_ROW_LOCKED;
		}

		// save new value
		D230.rowsDifficulty[difficultyIndex].stringIndex = lngIndex;
	}

#if defined(CTR_NATIVE)
	for (s32 difficultyIndex = 0; difficultyIndex < MM_DIFFICULTY_COUNT; difficultyIndex++)
	{
		s_nativeExtraDifficultyRows[difficultyIndex].stringIndex = D230.rowsDifficulty[difficultyIndex].stringIndex;
	}

	u16 hardLockFlag = D230.rowsDifficulty[MM_NATIVE_DIFFICULTY_HARD].stringIndex & MENU_ROW_LOCKED;
	s_nativeExtraDifficultyRows[MM_NATIVE_DIFFICULTY_SUPER_HARD].stringIndex = LNG_NA_241 | hardLockFlag;
	s_nativeExtraDifficultyRows[MM_NATIVE_DIFFICULTY_ULTRA_HARD].stringIndex = LNG_NA_242 | hardLockFlag;
#endif
}

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 230 0x800ad7a4-0x800ad828.
void MM_MenuProc_Difficulty(struct RectMenu *menu)
{
	s16 row = menu->rowSelected;

	// if uninitialized
	if (row == -1)
	{
		menu->ptrPrevBox_InHierarchy->state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
	}

	else
	{
		// if you are on a valid row
		if ((row >= 0) &&
#if defined(CTR_NATIVE)
		    (row < MM_NATIVE_DIFFICULTY_COUNT)
#else
		    (row < MM_DIFFICULTY_COUNT)
#endif
		)
		{
#if defined(CTR_NATIVE)
			if (row == MM_NATIVE_DIFFICULTY_SUPER_HARD)
			{
				sdata->gGT->arcadeDifficulty = 0x140;
			}
			else if (row == MM_NATIVE_DIFFICULTY_ULTRA_HARD)
			{
				sdata->gGT->arcadeDifficulty = 0x280;
			}
			else
#endif
			{
				// set difficulty to value, from array of fixed difficulty values
				sdata->gGT->arcadeDifficulty = D230.cupDifficulty.speed[row];
			}

			D230.titleMenuState = TITLE_MENU_STATE_EXITING;
			D230.desiredMenuIndex = MM_EXIT_ROUTE_CHARACTER_SELECT;

			menu->state |= ONLY_DRAW_TITLE;
			return;
		}
	}
	return;
}

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 230 0x800ad828-0x800ad8f0.
void MM_MenuProc_SingleCup(struct RectMenu *menu)
{
	struct GameTracker *gGT = sdata->gGT;
	s16 row = menu->rowSelected;

	if (row == -1)
	{
		menu->ptrPrevBox_InHierarchy->state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
		return;
	}

	if ((row >= 0) && (row < MM_RACE_TYPE_SELECTABLE_ROWS))
	{
		// disable Cup mode
		gGT->gameMode2 &= ~(CUP_ANY_KIND);

		// if you choose cup mode
		if (menu->rowSelected != 0)
		{
			// enable cup mode
			gGT->gameMode2 |= CUP_ANY_KIND;
		}

		menu->state |= ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY;

		// if mode is Arcade
		if ((gGT->gameMode1 & ARCADE_MODE) != 0)
		{
			// set next menu to 1P+2P select
			menu->ptrNextBox_InHierarchy = &D230.menuPlayers1P2P;
			D230.characterSelectTransitionState = IN_MENU;
			return;
		}

		// if mode is VS

		// set next menu to 2P+3P+4P (vs or battle)
		menu->ptrNextBox_InHierarchy = &D230.menuPlayers2P3P4P;
		D230.characterSelectTransitionState = EXITING_MENU;
	}
}

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 230 0x800ad8f0-0x800ad980.
void MM_MenuProc_NewLoad(struct RectMenu *menu)
{
	// row number
	s16 row = menu->rowSelected;

	if (row == -1)
	{
		menu->ptrPrevBox_InHierarchy->state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
		return;
	}

	if ((row < 0) || (row >= MM_ADV_NEW_LOAD_ROUTE_COUNT))
	{
		return;
	}

	// if Load was chosen
	D230.desiredMenuIndex = row;

	// MM_Title transitioning out
	D230.titleMenuState = TITLE_MENU_STATE_EXITING;

	menu->state |= ONLY_DRAW_TITLE;
	return;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ad980-0x800ad98c.
struct RectMenu *MM_AdvNewLoad_GetMenuPtr(void)
{
	// menu for new/load
	return &D230.menuAdventure;
}

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 230 0x800b42b0-0x800b4334.
void MM_ResetAllMenus(void)
{
	for (s32 menuIndex = 0; menuIndex < MM_MENU_RESET_COUNT; menuIndex++)
	{
		struct RectMenu *menu = D230.arrayMenuPtrs[menuIndex];

// NOTE(aalhendi): Retail resets one menu per array slot; native walks chained
// menus because overlay 230 data is not reloaded.
#ifdef CTR_NATIVE
		do
		{
			struct RectMenu *next = menu->ptrNextBox_InHierarchy;
#endif

			// Close menu
			menu->state |= RECTMENU_CLOSE_TRANSIENT;
			menu->state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);

			// Reset ptrNext and ptrPrev
			menu->ptrNextBox_InHierarchy = 0;
			menu->ptrPrevBox_InHierarchy = 0;

#ifdef CTR_NATIVE
			menu = next;
		} while (menu != 0);
#endif
	}

	// unused
	sdata->framesRemainingInMenu = MM_MENU_RESET_DONE_FRAMES;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b4334-0x800b4364.
void MM_JumpTo_Title_Returning(void)
{
	// return to main menu from another menu
	D230.titleMenuState = TITLE_MENU_STATE_RETURNING;

	// return to main menu
	sdata->ptrDesiredMenu = &D230.menuMainMenu;

	D230.titleMenuTransitionFrame = D230.titleMenuTransitionDurationFrames;
}

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 230 0x800b4364-0x800b43f4.
void MM_JumpTo_Title_FirstTime(void)
{
	struct GameTracker *gGT = sdata->gGT;

	MM_ResetAllMenus();

	MainStats_ClearBattleVS();

#if defined(CTR_NATIVE)
	if (s_nativeLanguageChosen == 0)
	{
		s_nativeLanguageBootMenu.state = RECTMENU_STATE_EXEC_CENTERED;
		s_nativeLanguageBootMenu.rowSelected = s_nativeLanguageRow;
		s_nativeLanguageBootMenu.ptrNextBox_InHierarchy = 0;
		s_nativeLanguageBootMenu.ptrPrevBox_InHierarchy = 0;
		s_nativeLanguageTimer = MM_NATIVE_LANGUAGE_TIMEOUT_FRAMES;
		sdata->ptrActiveMenu = &s_nativeLanguageBootMenu;
	}
	else
	{
		sdata->ptrActiveMenu = &D230.menuMainMenu;
	}
#elif BUILD == EurRetail
	// if you have not chose a language or skipped the language menu
	if (sdata->boolLangChosen == 0)
	{
		sdata->ptrActiveMenu = &D230.menuLngBoot;
		D230.langMenuTimer = MM_LANGUAGE_MENU_TIMEOUT_FRAMES;
	}
	else
	{
		// if not set to normal main menu
		sdata->ptrActiveMenu = &D230.menuMainMenu;
	}
#else
	// open Main Menu for the first time
	sdata->ptrActiveMenu = &D230.menuMainMenu;
#endif

	D230.titleIntroFrame = 0;

	// first time in main menu
	// (play crash trophy anim)
	D230.titleMenuState = TITLE_MENU_STATE_INTRO;

	// reset countdown clock for battle or crystal challenge
	gGT->originalEventTime = TITLE_INITIAL_EVENT_TIME;

	D230.menuMainMenu.state &= ~(EXECUTE_FUNCPTR | ONLY_DRAW_TITLE);
	D230.menuMainMenu.state |= DISABLE_INPUT_ALLOW_FUNCPTRS;

	// distance to screen (perspective)
	gGT->pushBuffer[0].distanceToScreen_PREV = TITLE_DEFAULT_DISTANCE_TO_SCREEN;
	gGT->pushBuffer[0].distanceToScreen_CURR = TITLE_DEFAULT_DISTANCE_TO_SCREEN;
	gGT->gameMode1 &= ~(TIME_TRIAL);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b43f4-0x800b4430.
void MM_JumpTo_BattleSetup(void)
{
	// Go to battle setup
	sdata->ptrActiveMenu = &D230.menuBattleWeapons;

	D230.menuBattleWeapons.state &= ~(ONLY_DRAW_TITLE);

	MM_Battle_Init();
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b4430-0x800b446c.
void MM_JumpTo_TrackSelect(void)
{
	// return to track selection
	sdata->ptrActiveMenu = &D230.menuTrackSelect;

	D230.menuTrackSelect.state &= ~(ONLY_DRAW_TITLE);

	MM_TrackSelect_Init();
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b446c-0x800b44a8.
void MM_JumpTo_Characters(void)
{
	// return to character selection
	sdata->ptrActiveMenu = &D230.menuCharacterSelect;

	D230.menuCharacterSelect.state &= ~(ONLY_DRAW_TITLE);

	MM_Characters_RestoreIDs();
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 overlay 230 0x800b44a8-0x800b44e4.
void MM_JumpTo_Scrapbook(void)
{
	// go to scrapbook
	sdata->ptrActiveMenu = &D230.menuScrapbook;

	D230.menuScrapbook.state &= ~(ONLY_DRAW_TITLE);

	MM_Scrapbook_Init();
}
