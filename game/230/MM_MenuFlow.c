#include <common.h>

#if defined(CTR_NATIVE)
#include "platform/native_leaderboard.h"
#if defined(__vita__)
#include "platform/native_adhoc.h"
#endif
#endif

#include "platform/native_user_id.h"

#ifdef CTR_NATIVE
int gNativeOnlineLeaderboardMode;

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

#if defined(__vita__)
enum MMNativeAdhocMenuStage
{
	MM_NATIVE_ADHOC_STAGE_MAIN = 0,
	MM_NATIVE_ADHOC_STAGE_MODE,
	MM_NATIVE_ADHOC_STAGE_ROLE,
	MM_NATIVE_ADHOC_STAGE_WAIT,
	MM_NATIVE_ADHOC_STAGE_GAME_FLOW,
	MM_NATIVE_ADHOC_MAIN_ROW = 6,
};

static const char *s_nativeAdhocHostText[MM_NATIVE_LANGUAGE_COUNT] =
{
	"HOST GAME",
	"CREER PARTIE",
	"SPIEL HOSTEN",
	"CREA PARTITA",
	"CREAR PARTIDA",
	"SPEL HOSTEN",
};

static const char *s_nativeAdhocJoinText[MM_NATIVE_LANGUAGE_COUNT] =
{
	"JOIN GAME",
	"REJOINDRE",
	"BEITRETEN",
	"UNISCITI",
	"UNIRSE",
	"DEELNEMEN",
};

static s16 s_nativeAdhocMenuStage = MM_NATIVE_ADHOC_STAGE_MAIN;
static s16 s_nativeAdhocSuppressInputFrames;
static s16 s_nativeAdhocGameMode = NATIVE_ADHOC_GAME_MODE_ARCADE;
#endif

static struct MenuRow s_nativeExtraDifficultyRows[MM_NATIVE_DIFFICULTY_COUNT + 1] =
{
	{LNG_EASY, 0, 1, 0, 0},
	{LNG_MEDIUM, 0, 2, 1, 1},
	{LNG_HARD, 1, 3, 2, 2},
	{NATIVE_MENU_STRING_SUPER_HARD, 2, 4, 3, 3},
	{NATIVE_MENU_STRING_ULTRA_HARD, 3, 4, 4, 4},
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

#if defined(__vita__)
static void MM_NativeAdhocModeProc(struct RectMenu *menu);
static void MM_NativeAdhocRoleProc(struct RectMenu *menu);
static void MM_NativeAdhocWaitProc(struct RectMenu *menu);

static struct MenuRow s_nativeAdhocModeRows[] =
{
	{LNG_ARCADE, 0, 1, 0, 0},
	{LNG_VS, 0, 1, 1, 1},
	{RECTMENU_STRING_NONE},
};

static struct MenuRow s_nativeAdhocRoleRows[] =
{
	{LNG_NA_241, 0, 1, 0, 0},
	{LNG_NA_242, 0, 1, 1, 1},
	{RECTMENU_STRING_NONE},
};

static struct MenuRow s_nativeAdhocWaitRows[] =
{
	{LNG_CANCEL, 0, 0, 0, 0},
	{RECTMENU_STRING_NONE},
};

static struct RectMenu s_nativeAdhocModeMenu =
{
	.stringIndexTitle = NATIVE_MENU_STRING_ADHOC,
	.state = CENTER_ON_X | USE_SMALL_FONT | BIG_TEXT_IN_TITLE,
	.rows = s_nativeAdhocModeRows,
	.funcPtr = MM_NativeAdhocModeProc,
};

static struct RectMenu s_nativeAdhocRoleMenu =
{
	.stringIndexTitle = NATIVE_MENU_STRING_ADHOC,
	.state = CENTER_ON_X | USE_SMALL_FONT | BIG_TEXT_IN_TITLE,
	.rows = s_nativeAdhocRoleRows,
	.funcPtr = MM_NativeAdhocRoleProc,
};

static struct RectMenu s_nativeAdhocWaitMenu =
{
	.stringIndexTitle = LNG_NA_241,
	.state = CENTER_ON_X | BIG_TEXT_IN_TITLE,
	.rows = s_nativeAdhocWaitRows,
	.funcPtr = MM_NativeAdhocWaitProc,
};
#endif

static struct MenuRow s_nativeMainMenuBasic[] =
{
	{LNG_ADVENTURE, 0, 1, 0, 0},
	{LNG_TIME_TRIAL, 0, 2, 1, 1},
	{LNG_ARCADE, 1, 3, 2, 2},
	{LNG_VS, 2, 4, 3, 3},
	{LNG_BATTLE, 3, 5, 4, 4},
	{NATIVE_MENU_STRING_BOSS_FIGHT, 4, 6, 5, 5},
#if defined(__vita__)
	{NATIVE_MENU_STRING_ADHOC, 5, 7, 6, 6},
	{LNG_OPTIONS, 6, 7, 7, 7},
#else
	{LNG_OPTIONS, 5, 6, 6, 6},
#endif
	{RECTMENU_STRING_NONE},
};

static struct MenuRow s_nativeMainMenuWithScrapbook[] =
{
	{LNG_ADVENTURE, 0, 1, 0, 0},
	{LNG_TIME_TRIAL, 0, 2, 1, 1},
	{LNG_ARCADE, 1, 3, 2, 2},
	{LNG_VS, 2, 4, 3, 3},
	{LNG_BATTLE, 3, 5, 4, 4},
	{NATIVE_MENU_STRING_BOSS_FIGHT, 4, 6, 5, 5},
#if defined(__vita__)
	{NATIVE_MENU_STRING_ADHOC, 5, 7, 6, 6},
	{LNG_OPTIONS, 6, 8, 7, 7},
	{LNG_SCRAPBOOK, 7, 8, 8, 8},
#else
	{LNG_OPTIONS, 5, 7, 6, 6},
	{LNG_SCRAPBOOK, 6, 7, 7, 7},
#endif
	{RECTMENU_STRING_NONE},
};

static struct MenuRow s_nativeTimeTrialRows[] =
{
#if CTR_NATIVE_HAS_LEADERBOARD
	{LNG_TIME_TRIAL, 4, 1, 0, 0},
	{LNG_RELIC_RACE, 0, 2, 1, 1},
	{NATIVE_MENU_STRING_GHOST_REPLAY, 1, 3, 2, 2},
	{LNG_HIGH_SCORE, 2, 4, 3, 3},
	{NATIVE_MENU_STRING_ONLINE_LEADERBOARD, 3, 0, 4, 4},
#else
	{LNG_TIME_TRIAL, 3, 1, 0, 0},
	{LNG_RELIC_RACE, 0, 2, 1, 1},
	{NATIVE_MENU_STRING_GHOST_REPLAY, 1, 3, 2, 2},
	{LNG_HIGH_SCORE, 2, 0, 3, 3},
#endif
	{RECTMENU_STRING_NONE},
};

static struct MenuRow s_nativeOptionsRows[] =
{
#ifdef __vita__
	{LNG_LANGUAGE, 3, 1, 0, 0},
	{NATIVE_MENU_STRING_MIRROR_MODE, 0, 2, 1, 1},
	{NATIVE_MENU_STRING_FRAME_RATE, 1, 3, 2, 2},
	{NATIVE_MENU_STRING_DEFAULT_CAMERA, 2, 0, 3, 3},
#else
	{LNG_LANGUAGE, 5, 1, 0, 0},
	{NATIVE_MENU_STRING_MIRROR_MODE, 0, 2, 1, 1},
	{NATIVE_MENU_STRING_FRAME_RATE, 1, 3, 2, 2},
	{NATIVE_MENU_STRING_DEFAULT_CAMERA, 2, 4, 3, 3},
	{NATIVE_MENU_STRING_ANTI_ALIASING, 3, 5, 4, 4},
	{NATIVE_MENU_STRING_BORDERLESS, 4, 0, 5, 5},
#endif
	{RECTMENU_STRING_NONE},
};

static struct MenuRow s_nativeBossFightRows[] =
{
	{LNG_RIPPER_ROO, 0, 1, 0, 0},
	{LNG_PAPU_PAPU, 0, 2, 1, 1},
	{LNG_KOMODO_JOE, 1, 3, 2, 2},
	{LNG_PINSTRIPE, 2, 4, 3, 3},
	{LNG_N_OXIDE_FULL, 3, 5, 4, 4},
	{NATIVE_MENU_STRING_OXIDE_FINAL, 4, 5, 5, 5},
	{RECTMENU_STRING_NONE},
};

static void MM_NativeLanguageBootMenuProc(struct RectMenu *menu);
static void MM_NativeLanguageMainMenuProc(struct RectMenu *menu);
static void MM_NativeTimeTrialMenuProc(struct RectMenu *menu);
static void MM_NativeOptionsMenuProc(struct RectMenu *menu);
static void MM_NativeBossFightMenuProc(struct RectMenu *menu);

static struct RectMenu s_nativeLanguageBootMenu =
{
	.stringIndexTitle = RECTMENU_STRING_NONE,
	.posX_curr = 256,
	.posY_curr = 118,
	.state = RECTMENU_STATE_EXEC_CENTERED,
	.rows = s_nativeLanguageRows,
	.funcPtr = MM_NativeLanguageBootMenuProc,
#if CTR_NATIVE_WIDESCREEN
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

static void MM_NativeTimeTrialRefreshOnlineRow(void)
{
#if CTR_NATIVE_HAS_LEADERBOARD
	s16 onlineString = NATIVE_MENU_STRING_ONLINE_LEADERBOARD;
	if (!NativeLeaderboard_IsInternetConnected())
	{
		onlineString |= MENU_ROW_LOCKED;
	}
	s_nativeTimeTrialRows[4].stringIndex = onlineString;
#endif
}
static struct RectMenu s_nativeTimeTrialMenu =
{
	.stringIndexTitle = LNG_TIME_TRIAL,
	.state = CENTER_ON_X,
	.rows = s_nativeTimeTrialRows,
	.funcPtr = MM_NativeTimeTrialMenuProc,
};

static struct RectMenu s_nativeOptionsMenu =
{
	.stringIndexTitle = LNG_OPTIONS,
	.state = CENTER_ON_X,
	.rows = s_nativeOptionsRows,
	.funcPtr = MM_NativeOptionsMenuProc,
};

static struct RectMenu s_nativeBossFightMenu =
{
	.stringIndexTitle = NATIVE_MENU_STRING_BOSS_FIGHT,
	.posX_curr = 256,
	.posY_curr = 82,
	.state = RECTMENU_STATE_EXEC_CENTERED | USE_SMALL_FONT | BIG_TEXT_IN_TITLE,
	.rows = s_nativeBossFightRows,
	.funcPtr = MM_NativeBossFightMenuProc,
};

s32 s_nativeLanguageChosen = 0;
static s32 s_nativeLanguageTimer;
static s16 s_nativeLanguageRow;
extern int cfg_language;
extern int gNativeMirrorModeEnabled;
extern int gNative60FpsEnabled;
extern int gNativeDefaultCameraFar;
#ifndef __vita__
extern int gNativeAntiAliasingEnabled;
extern int gNativeBorderlessEnabled;
#endif
extern int gNativeGhostReplayMode;
extern void save_config();

static void MM_NativeExtraDifficultyPrepare(void)
{
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
			s_nativeLanguageTimer = FPS_DOUBLE(MM_NATIVE_LANGUAGE_TIMEOUT_FRAMES);
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

#if defined(__vita__)
static s16 MM_NativeAdhocLanguageRow(void)
{
	if ((cfg_language >= 2) && (cfg_language <= 7))
	{
		return (s16)(cfg_language - 2);
	}
	return 0;
}

static void MM_NativeAdhocApplyText(void)
{
	s16 languageRow = MM_NativeAdhocLanguageRow();

	if ((sdata->lngStrings == NULL) || (sdata->numLngStrings <= LNG_NA_242))
	{
		return;
	}

	switch (s_nativeAdhocMenuStage)
	{
	case MM_NATIVE_ADHOC_STAGE_ROLE:
		sdata->lngStrings[LNG_NA_241] = (char *)s_nativeAdhocHostText[languageRow];
		sdata->lngStrings[LNG_NA_242] = (char *)s_nativeAdhocJoinText[languageRow];
		break;

	case MM_NATIVE_ADHOC_STAGE_WAIT:
		if (NativeAdhoc_GetRole() == NATIVE_ADHOC_ROLE_CLIENT)
		{
			sdata->lngStrings[LNG_NA_241] = (char *)NativeAdhoc_GetStatusText();
			sdata->lngStrings[LNG_NA_242] = (char *)s_nativeAdhocJoinText[languageRow];
			s_nativeAdhocWaitMenu.stringIndexTitle = LNG_NA_241;
		}
		else
		{
			sdata->lngStrings[LNG_NA_241] = (char *)s_nativeAdhocHostText[languageRow];
			sdata->lngStrings[LNG_NA_242] = (char *)NativeAdhoc_GetStatusText();
			s_nativeAdhocWaitMenu.stringIndexTitle = LNG_NA_242;
		}
		break;

	default:
		break;
	}
}

static void MM_NativeAdhocSetMainBreadcrumb(s16 stringIndex)
{
	s_nativeMainMenuBasic[MM_NATIVE_ADHOC_MAIN_ROW].stringIndex = stringIndex;
	s_nativeMainMenuWithScrapbook[MM_NATIVE_ADHOC_MAIN_ROW].stringIndex = stringIndex;
}

static void MM_NativeAdhocResetHierarchy(void)
{
	D230.menuMainMenu.state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
	D230.menuMainMenu.ptrNextBox_InHierarchy = NULL;

	s_nativeAdhocModeMenu.state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
	s_nativeAdhocModeMenu.ptrNextBox_InHierarchy = NULL;
	s_nativeAdhocModeMenu.ptrPrevBox_InHierarchy = NULL;
	s_nativeAdhocModeMenu.rowSelected = 0;

	s_nativeAdhocRoleMenu.state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
	s_nativeAdhocRoleMenu.ptrNextBox_InHierarchy = NULL;
	s_nativeAdhocRoleMenu.ptrPrevBox_InHierarchy = NULL;
	s_nativeAdhocRoleMenu.rowSelected = 0;

	s_nativeAdhocWaitMenu.state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
	s_nativeAdhocWaitMenu.ptrNextBox_InHierarchy = NULL;
	s_nativeAdhocWaitMenu.ptrPrevBox_InHierarchy = NULL;
	s_nativeAdhocWaitMenu.rowSelected = 0;
	s_nativeAdhocWaitMenu.stringIndexTitle = LNG_NA_241;
	s_nativeAdhocSuppressInputFrames = 0;
}

static void MM_NativeAdhocReturnToMain(void)
{
	NativeAdhoc_Shutdown();
	MM_NativeAdhocResetHierarchy();
	MM_NativeAdhocSetMainBreadcrumb(NATIVE_MENU_STRING_ADHOC);
	s_nativeAdhocGameMode = NATIVE_ADHOC_GAME_MODE_ARCADE;
	s_nativeAdhocMenuStage = MM_NATIVE_ADHOC_STAGE_MAIN;
	RECTMENU_ClearInput();
	sdata->ptrDesiredMenu = &D230.menuMainMenu;
}

static int MM_NativeAdhocPollWait(struct GameTracker *gGT)
{
	int dialogWasRunning;

	if (s_nativeAdhocMenuStage != MM_NATIVE_ADHOC_STAGE_WAIT)
	{
		return 0;
	}

	dialogWasRunning = NativeAdhoc_IsDialogRunning();
	NativeAdhoc_Update();
	MM_NativeAdhocApplyText();

	if (dialogWasRunning || NativeAdhoc_IsDialogRunning())
	{
		s_nativeAdhocSuppressInputFrames = 2;
	}
	if (s_nativeAdhocSuppressInputFrames > 0)
	{
		RECTMENU_ClearInput();
		s_nativeAdhocSuppressInputFrames--;
	}

	if (NativeAdhoc_IsConnected())
	{
		int adhocGameMode = NativeAdhoc_GetGameMode();
		if ((adhocGameMode != NATIVE_ADHOC_GAME_MODE_ARCADE) && (adhocGameMode != NATIVE_ADHOC_GAME_MODE_VS))
		{
			MM_NativeAdhocReturnToMain();
			return 1;
		}

		s_nativeAdhocGameMode = (s16)adhocGameMode;
		gGT->gameMode1 &= ~(BATTLE_MODE | ADVENTURE_MODE | TIME_TRIAL | RELIC_RACE | ADVENTURE_ARENA | ARCADE_MODE | ADVENTURE_CUP);
		gGT->gameMode2 &= ~(CUP_ANY_KIND);
		if (adhocGameMode == NATIVE_ADHOC_GAME_MODE_ARCADE)
		{
			gGT->gameMode1 |= ARCADE_MODE;
		}
		gGT->numPlyrNextGame = 2;
		gGT->numLaps = MM_DEFAULT_LAP_COUNT;
		if ((gGT->gameMode2 & CHEAT_ONELAP) != 0)
		{
			gGT->numLaps = MM_ONE_LAP_CHEAT_COUNT;
		}

		MM_NativeAdhocResetHierarchy();
		MM_NativeAdhocSetMainBreadcrumb(adhocGameMode == NATIVE_ADHOC_GAME_MODE_ARCADE ? LNG_ARCADE : LNG_VS);
		s_nativeAdhocMenuStage = MM_NATIVE_ADHOC_STAGE_GAME_FLOW;
		RECTMENU_ClearInput();

		if (adhocGameMode == NATIVE_ADHOC_GAME_MODE_VS)
		{
			D230.characterSelectTransitionState = EXITING_MENU;
			D230.titleMenuState = TITLE_MENU_STATE_EXITING;
			D230.desiredMenuIndex = MM_EXIT_ROUTE_CHARACTER_SELECT;
			return 1;
		}

		MM_NativeExtraDifficultyPrepare();
		s_nativeExtraDifficultyMenu.ptrPrevBox_InHierarchy = &D230.menuMainMenu;
		s_nativeExtraDifficultyMenu.ptrNextBox_InHierarchy = NULL;
		s_nativeExtraDifficultyMenu.rowSelected = 0;

		D230.menuMainMenu.ptrNextBox_InHierarchy = &s_nativeExtraDifficultyMenu;
		D230.menuMainMenu.state |= ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY;
		D230.characterSelectTransitionState = IN_MENU;
		return 1;
	}

	if (NativeAdhoc_GetStatus() == NATIVE_ADHOC_STATUS_ERROR)
	{
		MM_NativeAdhocReturnToMain();
		return 1;
	}

	return 0;
}

static void MM_NativeAdhocModeProc(struct RectMenu *menu)
{
	MM_NativeAdhocApplyText();

	if (menu->funcState != RECTMENU_FUNC_STATE_INPUT)
	{
		return;
	}

	if (menu->rowSelected < 0)
	{
		s_nativeAdhocMenuStage = MM_NATIVE_ADHOC_STAGE_MAIN;
		if (menu->ptrPrevBox_InHierarchy != NULL)
		{
			menu->ptrPrevBox_InHierarchy->state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
		}
		return;
	}

	if ((menu->rowSelected == 0) || (menu->rowSelected == 1))
	{
		s_nativeAdhocGameMode = menu->rowSelected == 0 ? NATIVE_ADHOC_GAME_MODE_ARCADE : NATIVE_ADHOC_GAME_MODE_VS;
		s_nativeAdhocMenuStage = MM_NATIVE_ADHOC_STAGE_ROLE;
		MM_NativeAdhocApplyText();
		s_nativeAdhocRoleMenu.rowSelected = 0;
		menu->ptrNextBox_InHierarchy = &s_nativeAdhocRoleMenu;
		menu->state |= ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY;
	}
}

static void MM_NativeAdhocRoleProc(struct RectMenu *menu)
{
	MM_NativeAdhocApplyText();

	if (menu->funcState != RECTMENU_FUNC_STATE_INPUT)
	{
		return;
	}

	if (menu->rowSelected < 0)
	{
		s_nativeAdhocMenuStage = MM_NATIVE_ADHOC_STAGE_MODE;
		if (menu->ptrPrevBox_InHierarchy != NULL)
		{
			menu->ptrPrevBox_InHierarchy->state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
		}
		return;
	}

	if ((menu->rowSelected == 0) || (menu->rowSelected == 1))
	{
		int role = menu->rowSelected == 0 ? NATIVE_ADHOC_ROLE_HOST : NATIVE_ADHOC_ROLE_CLIENT;
		if (!NativeAdhoc_Begin(role, s_nativeAdhocGameMode))
		{
			MM_NativeAdhocReturnToMain();
			return;
		}

		s_nativeAdhocMenuStage = MM_NATIVE_ADHOC_STAGE_WAIT;
		s_nativeAdhocSuppressInputFrames = 2;
		MM_NativeAdhocApplyText();
		s_nativeAdhocWaitMenu.rowSelected = 0;
		menu->ptrNextBox_InHierarchy = &s_nativeAdhocWaitMenu;
		menu->state |= ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY;
	}
}

static void MM_NativeAdhocWaitProc(struct RectMenu *menu)
{
	if ((menu->funcState == RECTMENU_FUNC_STATE_INPUT) && (menu->rowSelected <= 0))
	{
		MM_NativeAdhocReturnToMain();
	}
}
#endif

static void MM_NativeTimeTrialMenuProc(struct RectMenu *menu)
{
	if (menu->funcState == RECTMENU_FUNC_STATE_UPDATE)
	{
		MM_NativeTimeTrialRefreshOnlineRow();
		return;
	}

	if (menu->funcState != RECTMENU_FUNC_STATE_INPUT)
	{
		return;
	}

	struct RectMenu *parent = menu->ptrPrevBox_InHierarchy;
	if (menu->rowSelected < 0)
	{
		if (parent != NULL)
		{
			parent->state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
		}
		return;
	}

	struct GameTracker *gGT = sdata->gGT;
	s16 choose = menu->rows[menu->rowSelected].stringIndex & MENU_ROW_LNG_MASK;

	gGT->gameMode1 &= ~(TIME_TRIAL | RELIC_RACE);
	gNativeGhostReplayMode = 0;
	gNativeOnlineLeaderboardMode = 0;
	gNativeRelicRaceMode = 0;
	gNativeRelicRaceResultTier = -1;
	NativeGhostInput_ClearSelection();

	if (choose == LNG_TIME_TRIAL)
	{
		D230.titleMenuState = TITLE_MENU_STATE_EXITING;
		D230.desiredMenuIndex = MM_EXIT_ROUTE_CHARACTER_SELECT;
		gGT->numPlyrNextGame = 1;
		gGT->gameMode1 |= TIME_TRIAL;
		gGT->gameMode2 &= ~(CHEAT_WUMPA | CHEAT_MASK | CHEAT_TURBO | CHEAT_ENGINE | CHEAT_BOMBS);
		return;
	}

	if (choose == LNG_RELIC_RACE)
	{
		gNativeRelicRaceMode = 1;
		D230.titleMenuState = TITLE_MENU_STATE_EXITING;
		D230.desiredMenuIndex = MM_EXIT_ROUTE_CHARACTER_SELECT;
		gGT->numPlyrNextGame = 1;
		gGT->gameMode1 &= ~TIME_TRIAL;
		gGT->gameMode1 |= RELIC_RACE;
		gGT->gameMode2 &= ~(CHEAT_WUMPA | CHEAT_MASK | CHEAT_TURBO | CHEAT_ENGINE | CHEAT_BOMBS);
		return;
	}

	if (choose == NATIVE_MENU_STRING_GHOST_REPLAY)
	{
		gNativeGhostReplayMode = 1;
		D230.titleMenuState = TITLE_MENU_STATE_EXITING;
		D230.desiredMenuIndex = MM_EXIT_ROUTE_GHOST_REPLAY;
		gGT->numPlyrNextGame = 1;
		gGT->gameMode1 |= TIME_TRIAL;
		gGT->gameMode2 &= ~(CHEAT_WUMPA | CHEAT_MASK | CHEAT_TURBO | CHEAT_ENGINE | CHEAT_BOMBS);
		return;
	}

	if (choose == LNG_HIGH_SCORE)
	{
		D230.desiredMenuIndex = MM_EXIT_ROUTE_HIGH_SCORE;
		D230.titleMenuState = TITLE_MENU_STATE_EXITING;
		return;
	}

#if CTR_NATIVE_HAS_LEADERBOARD
	if (choose == NATIVE_MENU_STRING_ONLINE_LEADERBOARD)
	{
		gNativeOnlineLeaderboardMode = 1;
		NativeLeaderboard_RequestRefresh();
		D230.desiredMenuIndex = MM_EXIT_ROUTE_HIGH_SCORE;
		D230.titleMenuState = TITLE_MENU_STATE_EXITING;
	}
#endif
}

static void MM_NativeOptionsMenuProc(struct RectMenu *menu)
{
	if (menu->funcState == RECTMENU_FUNC_STATE_UPDATE)
	{
			return;
	}

	if (menu->funcState != RECTMENU_FUNC_STATE_INPUT)
	{
		return;
	}

	struct RectMenu *parent = menu->ptrPrevBox_InHierarchy;
	if (menu->rowSelected < 0)
	{
		if (parent != NULL)
		{
			parent->state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
		}
		return;
	}

	s16 choose = menu->rows[menu->rowSelected].stringIndex & MENU_ROW_LNG_MASK;

	if (choose == LNG_LANGUAGE)
	{
		s_nativeLanguageMainMenu.rowSelected = s_nativeLanguageRow;
		s_nativeLanguageMainMenu.ptrNextBox_InHierarchy = NULL;
		s_nativeLanguageMainMenu.ptrPrevBox_InHierarchy = menu;

		menu->ptrNextBox_InHierarchy = &s_nativeLanguageMainMenu;
		menu->state |= ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY;
		return;
	}

	if (choose == NATIVE_MENU_STRING_MIRROR_MODE)
	{
		gNativeMirrorModeEnabled ^= 1;
		save_config();
		return;
	}

	if (choose == NATIVE_MENU_STRING_FRAME_RATE)
	{
		gNative60FpsEnabled ^= 1;
		save_config();
		return;
	}

	if (choose == NATIVE_MENU_STRING_DEFAULT_CAMERA)
	{
		gNativeDefaultCameraFar ^= 1;
		save_config();
		return;
	}
#ifndef __vita__
	if (choose == NATIVE_MENU_STRING_ANTI_ALIASING)
	{
		gNativeAntiAliasingEnabled ^= 1;
		save_config();
		return;
	}

	if (choose == NATIVE_MENU_STRING_BORDERLESS)
	{
		Platform_SetBorderless(!gNativeBorderlessEnabled);
		save_config();
	}
#endif
}

static void MM_NativeBossFightPrepareMenu(void)
{
	s_nativeBossFightMenu.stringIndexTitle = NATIVE_MENU_STRING_BOSS_FIGHT;
	s_nativeBossFightMenu.posX_curr = 256;
	s_nativeBossFightMenu.posY_curr = 82;
	s_nativeBossFightMenu.state = RECTMENU_STATE_EXEC_CENTERED | USE_SMALL_FONT | BIG_TEXT_IN_TITLE;
	s_nativeBossFightMenu.rows = s_nativeBossFightRows;
	s_nativeBossFightMenu.funcPtr = MM_NativeBossFightMenuProc;
	s_nativeBossFightMenu.rowSelected = (s16)gNativeBossFightBossID;
	s_nativeBossFightMenu.ptrNextBox_InHierarchy = NULL;
	s_nativeBossFightMenu.ptrPrevBox_InHierarchy = NULL;
}

void MM_NativeBossFight_OpenBossSelect(void)
{
	MM_NativeBossFightPrepareMenu();
	sdata->ptrDesiredMenu = &s_nativeBossFightMenu;
}

void MM_NativeBossFight_JumpToBossSelect(void)
{
	MM_NativeBossFightPrepareMenu();
	sdata->ptrActiveMenu = &s_nativeBossFightMenu;
}

static void MM_NativeBossFightMenuProc(struct RectMenu *menu)
{
	if (menu->funcState != RECTMENU_FUNC_STATE_INPUT)
	{
		return;
	}

	if (menu->rowSelected < 0)
	{
		sdata->ptrDesiredMenu = &D230.menuCharacterSelect;
		MM_Characters_RestoreIDs();
		return;
	}

	NativeBossFight_SelectBoss(menu->rowSelected);
	sdata->ptrDesiredMenu = &D230.menuTrackSelect;
	MM_TrackSelect_Init();
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
#if defined(__vita__)
	if ((mainMenu->funcState == RECTMENU_FUNC_STATE_UPDATE) && MM_NativeAdhocPollWait(gGT))
	{
		return;
	}

	if (((mainMenu->state & DRAW_NEXT_MENU_IN_HIERARCHY) == 0) &&
	    (s_nativeAdhocMenuStage != MM_NATIVE_ADHOC_STAGE_GAME_FLOW))
	{
		if (NativeAdhoc_IsActive())
		{
			NativeAdhoc_Shutdown();
		}
		MM_NativeAdhocSetMainBreadcrumb(NATIVE_MENU_STRING_ADHOC);
		s_nativeAdhocMenuStage = MM_NATIVE_ADHOC_STAGE_MAIN;
	}
#endif

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
#if defined(CTR_NATIVE)
		MM_NativeTimeTrialRefreshOnlineRow();
#endif
		MM_Title_MenuUpdate();

		if (
		    // main menu, "title" exists, and timer >= 230
		    (D230.titleMenuState == TITLE_MENU_STATE_IN_MENU) && (D230.titleObj != NULL) && (TITLE_INTRO_TM_DRAW_MIN_FRAME < D230.titleIntroFrame))
		{
			DecalFont_DrawLineOT(sdata->lngStrings[LNG_TM], MM_TITLE_TM_X, MM_TITLE_TM_Y, FONT_SMALL, ORANGE,
			                     &gGT->backBuffer->otMem.uiOT[MM_TITLE_TM_OT_INDEX]);

#if defined(CTR_NATIVE)
			if ((D230.menuMainMenu.state & DRAW_NEXT_MENU_IN_HIERARCHY) == 0)
			{
				const char *userId = NativeUserId_GetDisplayString();
				if (userId != NULL)
				{
					char userIdText[32];
					snprintf(userIdText, sizeof(userIdText), "USER ID: %s", userId);
					DecalFont_DrawLineOT(userIdText, 8, 0xc8, FONT_SMALL, WHITE, &gGT->backBuffer->otMem.uiOT[MM_TITLE_TM_OT_INDEX]);
				}
			}
#endif
		}

		if ((D230.menuMainMenu.state & DRAW_NEXT_MENU_IN_HIERARCHY) == 0)
		{
#if defined(__vita__)
			if ((s_nativeAdhocMenuStage == MM_NATIVE_ADHOC_STAGE_GAME_FLOW) && NativeAdhoc_IsConnected())
			{
				gGT->numPlyrNextGame = 2;
			}
			else
#endif
			{
				gGT->numPlyrNextGame = 1;
			}

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
				gGT->demoCountdownTimer = FPS_DOUBLE(TITLE_DEMO_IDLE_FRAMES);
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
	gGT->gameMode1 &= ~(BATTLE_MODE | ADVENTURE_MODE | TIME_TRIAL | RELIC_RACE | ADVENTURE_ARENA | ARCADE_MODE | ADVENTURE_CUP);

	// clear more game mode flags
	gGT->gameMode2 &= ~(CUP_ANY_KIND);

	mainMenu->state |= ONLY_DRAW_TITLE;

	// Default to 3,
	// this intentionally disables the 1-lap cheat
	// in Time Trial and Adventure, DONT change it
	gGT->numLaps = MM_DEFAULT_LAP_COUNT;

	// get LNG index of row selected
	s16 choose = mainMenu->rows[mainMenu->rowSelected].stringIndex;

	gNativeGhostReplayMode = 0;
	gNativeRelicRaceMode = 0;
	gNativeRelicRaceResultTier = -1;
	NativeGhostInput_ClearSelection();
	NativeBossFight_Clear();

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
#if defined(CTR_NATIVE)
		s_nativeTimeTrialMenu.rowSelected = 0;
		s_nativeTimeTrialMenu.state = CENTER_ON_X;
		s_nativeTimeTrialMenu.ptrNextBox_InHierarchy = NULL;
		s_nativeTimeTrialMenu.ptrPrevBox_InHierarchy = mainMenu;

		mainMenu->ptrNextBox_InHierarchy = &s_nativeTimeTrialMenu;
		mainMenu->state |= DRAW_NEXT_MENU_IN_HIERARCHY;
		return;
#else
		D230.titleMenuState = TITLE_MENU_STATE_EXITING;
		D230.desiredMenuIndex = MM_EXIT_ROUTE_CHARACTER_SELECT;
		gGT->numPlyrNextGame = 1;
		gGT->gameMode1 |= TIME_TRIAL;
		gGT->gameMode2 &= ~(CHEAT_WUMPA | CHEAT_MASK | CHEAT_TURBO | CHEAT_ENGINE | CHEAT_BOMBS);
		return;
#endif
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

	if (choose == NATIVE_MENU_STRING_BOSS_FIGHT)
	{
		gNativeBossFightMode = 1;
		NativeBossFight_SelectBoss(0);
		gGT->numPlyrNextGame = 1;
		gGT->gameMode1 |= ADVENTURE_BOSS;
		gGT->gameMode2 &= ~(CHEAT_WUMPA | CHEAT_MASK | CHEAT_TURBO | CHEAT_ENGINE | CHEAT_BOMBS);
		D230.titleMenuState = TITLE_MENU_STATE_EXITING;
		D230.desiredMenuIndex = MM_EXIT_ROUTE_CHARACTER_SELECT;
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
#if defined(__vita__)
	if (choose == NATIVE_MENU_STRING_ADHOC)
	{
		s_nativeAdhocModeMenu.state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
		s_nativeAdhocModeMenu.ptrNextBox_InHierarchy = NULL;
		s_nativeAdhocRoleMenu.state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
		s_nativeAdhocRoleMenu.ptrNextBox_InHierarchy = NULL;
		s_nativeAdhocWaitMenu.state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
		s_nativeAdhocWaitMenu.ptrNextBox_InHierarchy = NULL;
		s_nativeAdhocSuppressInputFrames = 0;
		s_nativeAdhocGameMode = NATIVE_ADHOC_GAME_MODE_ARCADE;
		s_nativeAdhocMenuStage = MM_NATIVE_ADHOC_STAGE_MODE;
		s_nativeAdhocModeMenu.rowSelected = 0;
		s_nativeAdhocModeMenu.ptrPrevBox_InHierarchy = mainMenu;

		mainMenu->ptrNextBox_InHierarchy = &s_nativeAdhocModeMenu;
		mainMenu->state |= DRAW_NEXT_MENU_IN_HIERARCHY;
		return;
	}
#endif

	// Options
	if (choose == LNG_OPTIONS)
	{
		s_nativeOptionsMenu.rowSelected = 0;
		s_nativeOptionsMenu.state = CENTER_ON_X;
		s_nativeOptionsMenu.ptrNextBox_InHierarchy = NULL;
		s_nativeOptionsMenu.ptrPrevBox_InHierarchy = mainMenu;

		mainMenu->ptrNextBox_InHierarchy = &s_nativeOptionsMenu;
		mainMenu->state |= DRAW_NEXT_MENU_IN_HIERARCHY;
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
	s_nativeExtraDifficultyRows[MM_NATIVE_DIFFICULTY_SUPER_HARD].stringIndex = NATIVE_MENU_STRING_SUPER_HARD | hardLockFlag;
	s_nativeExtraDifficultyRows[MM_NATIVE_DIFFICULTY_ULTRA_HARD].stringIndex = NATIVE_MENU_STRING_ULTRA_HARD | hardLockFlag;
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

	D230.titleMenuTransitionFrame = FPS_DOUBLE(D230.titleMenuTransitionDurationFrames);
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
		s_nativeLanguageTimer = FPS_DOUBLE(MM_NATIVE_LANGUAGE_TIMEOUT_FRAMES);
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
		D230.langMenuTimer = FPS_DOUBLE(MM_LANGUAGE_MENU_TIMEOUT_FRAMES);
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
	gGT->gameMode1 &= ~(TIME_TRIAL | RELIC_RACE);
	gNativeRelicRaceMode = 0;
	gNativeRelicRaceResultTier = -1;
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
