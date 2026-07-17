#ifndef CTR_NATIVE_OVR_230_H
#define CTR_NATIVE_OVR_230_H

enum
{
	SCRAP_INIT = 0,
	SCRAP_LOAD = 1,
	SCRAP_PLAY = 2,
	SCRAP_STOP = 3,
	SCRAP_EXIT = 4,
};
typedef s32 ScrapbookState;

CTR_STATIC_ASSERT(sizeof(ScrapbookState) == 0x4);
CTR_STATIC_ASSERT(SCRAP_INIT == 0);
CTR_STATIC_ASSERT(SCRAP_EXIT == 4);

enum TransitionState
{
	ENTERING_MENU,
	IN_MENU,
	EXITING_MENU,
};

CTR_STATIC_ASSERT(ENTERING_MENU == 0);
CTR_STATIC_ASSERT(IN_MENU == 1);
CTR_STATIC_ASSERT(EXITING_MENU == 2);
CTR_STATIC_ASSERT(sizeof(enum TransitionState) == 0x4);

enum TitleMenuState
{
	TITLE_MENU_STATE_INTRO = 0,
	TITLE_MENU_STATE_IN_MENU = 1,
	TITLE_MENU_STATE_EXITING = 2,
	TITLE_MENU_STATE_RETURNING = 3,
};

CTR_STATIC_ASSERT(sizeof(enum TitleMenuState) == 0x4);

enum
{
	MM_EXIT_ROUTE_ADV_NEW = 0,
	MM_EXIT_ROUTE_ADV_LOAD = 1,
	MM_EXIT_ROUTE_CHARACTER_SELECT = 2,
	MM_EXIT_ROUTE_HIGH_SCORE = 3,
	MM_EXIT_ROUTE_DEMO = 4,
	MM_EXIT_ROUTE_SCRAPBOOK = 5,
};
typedef s32 MainMenuExitRoute;

enum TitleIntroFrames
{
	TITLE_INTRO_MENU_READY_FRAME = 230,
	TITLE_INTRO_TM_DRAW_MIN_FRAME = TITLE_INTRO_MENU_READY_FRAME - 1,
	TITLE_INTRO_END_FRAME = 245,
	TITLE_INTRO_SKIP_FRAME = 1000,
};

enum TitleIntroConstants
{
	TITLE_INSTANCE_COUNT = 6,
	TITLE_INSTANCE_COUNT_JPN = 7,
	TITLE_INTRO_SKIP_INPUT = BTN_CROSS_one | BTN_CIRCLE | BTN_SQUARE_one | BTN_TRIANGLE,
	TITLE_DEMO_IDLE_FRAMES = CTR_SECONDS_TO_FRAMES(30),
	TITLE_DEMO_RACE_FRAMES = CTR_SECONDS_TO_FRAMES(60),
	TITLE_DEFAULT_DISTANCE_TO_SCREEN = 0x100,
	TITLE_INITIAL_EVENT_TIME = 0x2a300,
	TITLE_CAMERA_MOVE_FRAMES = 0xf,
	TITLE_TROPHY_HIDE_START_FRAME = 138,
	TITLE_TROPHY_HIDE_FRAMES = 62,
	TITLE_TROPHY_ANIM_START_FRAME = 200,
	TITLE_MATRIX_SCALE = 0x5000,
	TITLE_SPEC_LIGHT_Y = 0x1000,
	TITLE_DEMO_DRIVER_COUNT = 8,
	TITLE_DEMO_INDEX_MASK = TITLE_DEMO_DRIVER_COUNT - 1,
	TITLE_INTRO_DISTANCE_TO_SCREEN = 450,
	TITLE_CAMERA_RESET_X = 2000,

#if BUILD != JpnRetail
	TITLE_SOUND_COUNT = 8,
#else
	TITLE_SOUND_COUNT = 7,
#endif
};

CTR_STATIC_ASSERT(TITLE_INSTANCE_COUNT == 6);
CTR_STATIC_ASSERT(TITLE_INSTANCE_COUNT_JPN == 7);
CTR_STATIC_ASSERT(sizeof(MainMenuExitRoute) == 0x4);
CTR_STATIC_ASSERT(MM_EXIT_ROUTE_ADV_NEW == 0);
CTR_STATIC_ASSERT(MM_EXIT_ROUTE_SCRAPBOOK == 5);
CTR_STATIC_ASSERT(TITLE_INTRO_SKIP_INPUT == 0x40070);
CTR_STATIC_ASSERT(TITLE_DEMO_IDLE_FRAMES == 900);
CTR_STATIC_ASSERT(TITLE_DEMO_RACE_FRAMES == 1800);
CTR_STATIC_ASSERT(TITLE_DEFAULT_DISTANCE_TO_SCREEN == 0x100);
CTR_STATIC_ASSERT(TITLE_INITIAL_EVENT_TIME == 0x2a300);
CTR_STATIC_ASSERT(TITLE_CAMERA_MOVE_FRAMES == 0xf);
CTR_STATIC_ASSERT(TITLE_TROPHY_HIDE_START_FRAME == 138);
CTR_STATIC_ASSERT(TITLE_TROPHY_HIDE_FRAMES == 62);
CTR_STATIC_ASSERT(TITLE_TROPHY_ANIM_START_FRAME == 200);
CTR_STATIC_ASSERT(TITLE_MATRIX_SCALE == 0x5000);
CTR_STATIC_ASSERT(TITLE_SPEC_LIGHT_Y == 0x1000);
CTR_STATIC_ASSERT(TITLE_DEMO_DRIVER_COUNT == 8);
CTR_STATIC_ASSERT(TITLE_DEMO_INDEX_MASK == 7);
CTR_STATIC_ASSERT(TITLE_INTRO_DISTANCE_TO_SCREEN == 450);
CTR_STATIC_ASSERT(TITLE_CAMERA_RESET_X == 2000);
#if BUILD != JpnRetail
CTR_STATIC_ASSERT(TITLE_SOUND_COUNT == 8);
#else
CTR_STATIC_ASSERT(TITLE_SOUND_COUNT == 7);
#endif

enum ScrapbookConstants
{
	SCRAPBOOK_STREAM_FRAMES = 0x1148,
	SCRAPBOOK_VIDEO_WIDTH = 0x200,
	SCRAPBOOK_VIDEO_HEIGHT = 0xd0,
	SCRAPBOOK_FRAME_VBLANKS = 4,
	SCRAPBOOK_SKIP_INPUT = TITLE_INTRO_SKIP_INPUT | BTN_START,
};

CTR_STATIC_ASSERT(SCRAPBOOK_STREAM_FRAMES == 0x1148);
CTR_STATIC_ASSERT(SCRAPBOOK_VIDEO_WIDTH == 0x200);
CTR_STATIC_ASSERT(SCRAPBOOK_VIDEO_HEIGHT == 0xd0);
CTR_STATIC_ASSERT(SCRAPBOOK_FRAME_VBLANKS == 4);
CTR_STATIC_ASSERT(SCRAPBOOK_SKIP_INPUT == 0x41070);

enum MainMenuCheatConstants
{
	MM_CHEAT_SUCCESS_SFX = 0x67,
	MM_CHEAT_BUTTON_HISTORY_COUNT = 10,

#if BUILD == JpnRetail
	MM_CHEAT_COUNT = 0x15,
#else
	MM_CHEAT_COUNT = 0x16,
#endif
};

enum CharacterSelectDirection
{
	CHARACTER_SELECT_DIR_UP = 0,
	CHARACTER_SELECT_DIR_DOWN = 1,
	CHARACTER_SELECT_DIR_LEFT = 2,
	CHARACTER_SELECT_DIR_RIGHT = 3,
	CHARACTER_SELECT_DIRECTION_COUNT = 4,
};

enum CharacterSelectUnlockRequirement
{
	MM_CHARACTER_UNLOCK_ALWAYS = -1,
};

CTR_STATIC_ASSERT(MM_CHEAT_SUCCESS_SFX == 0x67);
CTR_STATIC_ASSERT(MM_CHEAT_BUTTON_HISTORY_COUNT == 10);
#if BUILD == JpnRetail
CTR_STATIC_ASSERT(MM_CHEAT_COUNT == 0x15);
#else
CTR_STATIC_ASSERT(MM_CHEAT_COUNT == 0x16);
#endif
CTR_STATIC_ASSERT(CHARACTER_SELECT_DIR_UP == 0);
CTR_STATIC_ASSERT(CHARACTER_SELECT_DIR_DOWN == 1);
CTR_STATIC_ASSERT(CHARACTER_SELECT_DIR_LEFT == 2);
CTR_STATIC_ASSERT(CHARACTER_SELECT_DIR_RIGHT == 3);
CTR_STATIC_ASSERT(CHARACTER_SELECT_DIRECTION_COUNT == 4);
CTR_STATIC_ASSERT(MM_CHARACTER_UNLOCK_ALWAYS == -1);

enum MainMenuFlowConstants
{
	MM_TRANSITION_SWISH_SFX = 0x65,
	MM_TRANSITION_SWISH_FRAME = 4,
	MM_DEFAULT_LAP_COUNT = 3,
	MM_ONE_LAP_CHEAT_COUNT = 1,
	MM_MENU_RESET_DONE_FRAMES = 0xf,
	MM_PLAYER_1P2P_SELECTABLE_ROWS = 2,
	MM_PLAYER_2P3P4P_SELECTABLE_ROWS = 3,
	MM_RACE_TYPE_SELECTABLE_ROWS = 2,
	MM_ADV_NEW_LOAD_ROUTE_COUNT = 2,
	MM_DIFFICULTY_COUNT = 3,
	MM_CUP_TRACK_COUNT = 4,
	MM_TITLE_TM_X = 0x10e,
	MM_TITLE_TM_Y = 0x9c,
	MM_TITLE_TM_OT_INDEX = 3,

#if BUILD == EurRetail
	MM_MENU_RESET_COUNT = 10,
	MM_LANGUAGE_MENU_TIMEOUT_FRAMES = CTR_SECONDS_TO_FRAMES(30),
#else
	MM_MENU_RESET_COUNT = 9,
#endif
};

CTR_STATIC_ASSERT(MM_TRANSITION_SWISH_SFX == 0x65);
CTR_STATIC_ASSERT(MM_TRANSITION_SWISH_FRAME == 4);
CTR_STATIC_ASSERT(MM_DEFAULT_LAP_COUNT == 3);
CTR_STATIC_ASSERT(MM_ONE_LAP_CHEAT_COUNT == 1);
CTR_STATIC_ASSERT(MM_MENU_RESET_DONE_FRAMES == 0xf);
CTR_STATIC_ASSERT(MM_PLAYER_1P2P_SELECTABLE_ROWS == 2);
CTR_STATIC_ASSERT(MM_PLAYER_2P3P4P_SELECTABLE_ROWS == 3);
CTR_STATIC_ASSERT(MM_RACE_TYPE_SELECTABLE_ROWS == 2);
CTR_STATIC_ASSERT(MM_ADV_NEW_LOAD_ROUTE_COUNT == 2);
CTR_STATIC_ASSERT(MM_DIFFICULTY_COUNT == 3);
CTR_STATIC_ASSERT(MM_CUP_TRACK_COUNT == 4);
CTR_STATIC_ASSERT(MM_TITLE_TM_X == 0x10e);
CTR_STATIC_ASSERT(MM_TITLE_TM_Y == 0x9c);
CTR_STATIC_ASSERT(MM_TITLE_TM_OT_INDEX == 3);
#if BUILD == EurRetail
CTR_STATIC_ASSERT(MM_MENU_RESET_COUNT == 10);
CTR_STATIC_ASSERT(MM_LANGUAGE_MENU_TIMEOUT_FRAMES == 900);
#else
CTR_STATIC_ASSERT(MM_MENU_RESET_COUNT == 9);
#endif

enum MainMenuCupSelectConstants
{
	MM_CUP_SELECT_INITIAL_TRANSITION_FRAMES = 0xc,
	MM_CUP_SELECT_LERP_FRAMES = 8,
	MM_CUP_SELECT_TRANSITION_OUT_DONE_FRAME = 12,
	MM_CUP_SELECT_DRIVER_SLOT_COUNT = 8,
	MM_CUP_SELECT_TITLE_META_INDEX = 4,
	MM_CUP_SELECT_TITLE_X_OFFSET = 0x100,
	MM_CUP_SELECT_TITLE_Y_OFFSET = 0x10,
	MM_CUP_SELECT_COLUMN_WIDTH = 200,
	MM_CUP_SELECT_ROW_HEIGHT = 0x54,
	MM_CUP_SELECT_NAME_X_OFFSET = 0xa2,
	MM_CUP_SELECT_NAME_Y_OFFSET = 0x44,
	MM_CUP_SELECT_CONTENT_X_OFFSET = 0x4e,
	MM_CUP_SELECT_CONTENT_Y_OFFSET = 0x29,
	MM_CUP_SELECT_STAR_ICON_GROUP = 5,
	MM_CUP_SELECT_STAR_ICON_ID = 0x37,
	MM_CUP_SELECT_STAR_COLUMN_BIAS = 0xca,
	MM_CUP_SELECT_STAR_X_OFFSET = -0x16,
	MM_CUP_SELECT_STAR_Y_OFFSET = 0x10,
	MM_CUP_SELECT_STAR_Y_STEP = 0x10,
	MM_CUP_SELECT_TRACK_X_STEP = 0x54,
	MM_CUP_SELECT_TRACK_Y_STEP = 0x23,
	MM_CUP_SELECT_HIGHLIGHT_X_OFFSET = -3,
	MM_CUP_SELECT_HIGHLIGHT_Y_OFFSET = -2,
	MM_CUP_SELECT_HIGHLIGHT_WIDTH = 174,
	MM_CUP_SELECT_HIGHLIGHT_HEIGHT = 74,
	MM_CUP_SELECT_BACKGROUND_X_OFFSET = -6,
	MM_CUP_SELECT_BACKGROUND_Y_OFFSET = -4,
	MM_CUP_SELECT_BACKGROUND_WIDTH = 180,
	MM_CUP_SELECT_BACKGROUND_HEIGHT = 78,
	MM_CUP_SELECT_FLASH_FRAME_BIT = 2,
	MM_CUP_SELECT_FLASH_COLOR_BIT = 4,
};

#define MM_CUP_SELECT_TEXT_COLOR 0xffff8000u

CTR_STATIC_ASSERT(MM_CUP_SELECT_INITIAL_TRANSITION_FRAMES == 0xc);
CTR_STATIC_ASSERT(MM_CUP_SELECT_LERP_FRAMES == 8);
CTR_STATIC_ASSERT(MM_CUP_SELECT_TRANSITION_OUT_DONE_FRAME == 12);
CTR_STATIC_ASSERT(MM_CUP_SELECT_DRIVER_SLOT_COUNT == 8);
CTR_STATIC_ASSERT(MM_CUP_SELECT_TITLE_META_INDEX == 4);
CTR_STATIC_ASSERT(MM_CUP_SELECT_TITLE_X_OFFSET == 0x100);
CTR_STATIC_ASSERT(MM_CUP_SELECT_TITLE_Y_OFFSET == 0x10);
CTR_STATIC_ASSERT(MM_CUP_SELECT_COLUMN_WIDTH == 200);
CTR_STATIC_ASSERT(MM_CUP_SELECT_ROW_HEIGHT == 0x54);
CTR_STATIC_ASSERT(MM_CUP_SELECT_NAME_X_OFFSET == 0xa2);
CTR_STATIC_ASSERT(MM_CUP_SELECT_NAME_Y_OFFSET == 0x44);
CTR_STATIC_ASSERT(MM_CUP_SELECT_CONTENT_X_OFFSET == 0x4e);
CTR_STATIC_ASSERT(MM_CUP_SELECT_CONTENT_Y_OFFSET == 0x29);
CTR_STATIC_ASSERT(MM_CUP_SELECT_STAR_ICON_GROUP == 5);
CTR_STATIC_ASSERT(MM_CUP_SELECT_STAR_ICON_ID == 0x37);
CTR_STATIC_ASSERT(MM_CUP_SELECT_STAR_COLUMN_BIAS == 0xca);
CTR_STATIC_ASSERT(MM_CUP_SELECT_STAR_X_OFFSET == -0x16);
CTR_STATIC_ASSERT(MM_CUP_SELECT_STAR_Y_OFFSET == 0x10);
CTR_STATIC_ASSERT(MM_CUP_SELECT_STAR_Y_STEP == 0x10);
CTR_STATIC_ASSERT(MM_CUP_SELECT_TRACK_X_STEP == 0x54);
CTR_STATIC_ASSERT(MM_CUP_SELECT_TRACK_Y_STEP == 0x23);
CTR_STATIC_ASSERT(MM_CUP_SELECT_HIGHLIGHT_X_OFFSET == -3);
CTR_STATIC_ASSERT(MM_CUP_SELECT_HIGHLIGHT_Y_OFFSET == -2);
CTR_STATIC_ASSERT(MM_CUP_SELECT_HIGHLIGHT_WIDTH == 174);
CTR_STATIC_ASSERT(MM_CUP_SELECT_HIGHLIGHT_HEIGHT == 74);
CTR_STATIC_ASSERT(MM_CUP_SELECT_BACKGROUND_X_OFFSET == -6);
CTR_STATIC_ASSERT(MM_CUP_SELECT_BACKGROUND_Y_OFFSET == -4);
CTR_STATIC_ASSERT(MM_CUP_SELECT_BACKGROUND_WIDTH == 180);
CTR_STATIC_ASSERT(MM_CUP_SELECT_BACKGROUND_HEIGHT == 78);
CTR_STATIC_ASSERT(MM_CUP_SELECT_FLASH_FRAME_BIT == 2);
CTR_STATIC_ASSERT(MM_CUP_SELECT_FLASH_COLOR_BIT == 4);

// MainMenu
struct Title
{
	// 0x0
	struct Thread *t;

	// 0x4
	struct Instance *i[TITLE_INSTANCE_COUNT];

	// 0x1c
	SVec3 cameraPosOffset;

	// 0x22
	s16 UnusedPadding;

	// 0x24 -- size of struct
};

CTR_STATIC_ASSERT(sizeof(struct Title) == 0x24);

struct TitleInstanceMeta
{
	s16 modelID;
	s16 animStartFrame;

	// Stored as TITLE_INTRO_MENU_READY_FRAME for every NTSC-U row; no current NTSC-U use site is known.
	s16 unusedMenuReadyFrame;

	u16 isTrophy;
};

CTR_STATIC_ASSERT(sizeof(struct TitleInstanceMeta) == 0x8);

enum MMVideoFlags
{
	MM_VIDEO_FLAG_RGB24 = 0x1,
	MM_VIDEO_FLAG_HAS_XA_AUDIO = 0x2,
	MM_VIDEO_FLAG_LOOP = 0x4,
	MM_VIDEO_FLAG_SCRAPBOOK = 0x8,
};

CTR_STATIC_ASSERT(MM_VIDEO_FLAG_RGB24 == 0x1);
CTR_STATIC_ASSERT(MM_VIDEO_FLAG_HAS_XA_AUDIO == 0x2);
CTR_STATIC_ASSERT(MM_VIDEO_FLAG_LOOP == 0x4);
CTR_STATIC_ASSERT(MM_VIDEO_FLAG_SCRAPBOOK == 0x8);

enum MMVideoConstants
{
	MM_VIDEO_VLC_WAIT_FRAMES = 10,
	MM_VIDEO_STALL_RECOVERY_FRAMES = 400,
	MM_VIDEO_STALLED_BACKLOC_FRAMES = 0x40,
	MM_VIDEO_DEFAULT_RING_SECTORS = 0x40,
	MM_VIDEO_BACKLOC_NONE = -1,
	MM_VIDEO_DCT_OUTPUT_TIMEOUT_POLLS = 40000,
	MM_VIDEO_CD_READY_POLL_INTERVAL = 0x28,
	MM_VIDEO_GPU_IDLE_TIMEOUT = 10000,
};

#define MM_VIDEO_STREAM_END_FRAME_NONE 0xffffffffu

CTR_STATIC_ASSERT(MM_VIDEO_VLC_WAIT_FRAMES == 10);
CTR_STATIC_ASSERT(MM_VIDEO_STALL_RECOVERY_FRAMES == 400);
CTR_STATIC_ASSERT(MM_VIDEO_STALLED_BACKLOC_FRAMES == 0x40);
CTR_STATIC_ASSERT(MM_VIDEO_DEFAULT_RING_SECTORS == 0x40);
CTR_STATIC_ASSERT(MM_VIDEO_BACKLOC_NONE == -1);
CTR_STATIC_ASSERT(MM_VIDEO_DCT_OUTPUT_TIMEOUT_POLLS == 40000);
CTR_STATIC_ASSERT(MM_VIDEO_CD_READY_POLL_INTERVAL == 0x28);
CTR_STATIC_ASSERT(MM_VIDEO_GPU_IDLE_TIMEOUT == 10000);
CTR_STATIC_ASSERT(MM_VIDEO_STREAM_END_FRAME_NONE == 0xffffffffu);

enum MainMenuTrackUnlockRequirement
{
	MM_TRACK_UNLOCK_ALWAYS = -1,
	MM_TRACK_UNLOCK_1P_ONLY = -2,
};

CTR_STATIC_ASSERT(MM_TRACK_UNLOCK_ALWAYS == -1);
CTR_STATIC_ASSERT(MM_TRACK_UNLOCK_1P_ONLY == -2);

// array at 0x800b53b0
struct MainMenu_LevelRow
{
	// 0 - dingo canyon
	// 3 - crash cove
	// etc
	s16 levID;

// NTSC-J also has this,
// but changing it now without proper refactor will break pointer compiles
#if BUILD >= EurRetail
	// Do not represent AI Difficulty
	// Number of Wumpas drawn representing levels of difficulty on NTSC-J
	// Left unused on PAL
	s16 difficulty_level;
#endif

	// texture that shows before video plays
	s16 videoThumbnail;

	// which black+white map draws
	s16 mapTextureID;

	// MM_TRACK_UNLOCK_ALWAYS for unlock by default,
	// otherwise has a flag for what is needed.
	// MM_TRACK_UNLOCK_1P_ONLY means only show in 1P mode (Oxide Station).
	s16 unlock;

#if BUILD >= EurRetail
	s16 unlock_padding;
#endif

	// BIGFILE entry index for this track's preview video
	s32 previewVideoFileIndex;

	// how long preview video plays before looping
	s32 previewVideoFrameCount;

	// Struct is 16 bytes large in NTSC-U, and 20 bytes in PAL & NTSC-J
	// this structure is now complete
};

struct CharacterSelectMeta
{
	// 0x0
	s16 posX;
	s16 posY;

	// 0x4
	// up, down, left, right
	u8 nextIconByDirection[CHARACTER_SELECT_DIRECTION_COUNT];

	// 0x8
	s16 characterID;
	u16 unlockFlags;

	// 0xC -- size
};

struct CupDifficultyTables
{
	// 0x00
	s16 firstUnlockBit[4];

	// 0x08
	s16 stringIndex[4];

	// 0x10
	s16 speed[4];
};

struct CharacterSelectLayoutTables
{
	// 0x00
	s16 windowW[6];

	// 0x0c
	s16 windowH[6];

	// 0x18
	s16 driverPosZ[6];

	// 0x24
	s16 driverPosY[6];

	// 0x30
	s16 textY[6];
};

struct CharacterSelectPlayerState
{
	// 0x00
	s16 modelMoveDir[4];

	// 0x08
	s16 desiredCharacterID[4];

	// 0x10
	s16 currentCharacterID[4];

	// 0x18
	s16 angle[4];
};

struct CharacterSelectDriverModelConfig
{
	// 0x00
	SVec3 pos;
	s16 _pad_pos;

	// 0x08
	SVec3 rot;
	s16 moveFrames;

	// 0x10
	s32 slideDistance;
};

struct TimeTrialStarTables
{
	// 0x00
	u16 colorIndex[2];

	// 0x04
	u16 beatenFlagBit[2];
};

struct CupSelectStarTables
{
	// 0x00
	u16 colorIndex[4];

	// 0x08
	s16 winBitBase[4];
};

struct BattleSetupTables
{
	// 0x00
	u32 typeModeFlags[3];

	// 0x0c
	u8 timeLimitMinutes[4];

	// 0x10
	s8 lifeModeTimeLimitMinutes[4];

	// 0x14
	u8 lifeLimitValues[4];

	// 0x18
	u8 pointLimitValues[4];
};

struct HighScoreGhostStarTables
{
	// 0x00
	u16 colorIndex[2];

	// 0x04
	u16 beatenFlagBit[2];
};

struct LapCountMenuRow
{
	// 0x00
	u8 lapCount;

	// 0x01
	u8 padding;
};

struct HighScoreSelectionState
{
	// 0x00
	s16 targetTrack;

	// 0x02
	s16 targetRow;

	// 0x04
	s16 currentTrack;

	// 0x06
	s16 currentRow;
};

struct HighScoreTransitionState
{
	// 0x00
	s16 state;

	// 0x02
	union
	{
		s16 frame[3];
		struct
		{
			s16 mainFrame;
			s16 trackFrame;
			s16 rowFrame;
		};
	};

	// 0x08
	// -1 for negative direction and 1 for positive direction.
	union
	{
		s16 horizontalMove[2];
		struct
		{
			s16 activeHorizontalMove;
			s16 pendingHorizontalMove;
		};
	};

	// 0x0c
	union
	{
		s16 verticalMove[2];
		struct
		{
			s16 activeVerticalMove;
			s16 pendingVerticalMove;
		};
	};
};

struct MenuExitTransitionState
{
	// 0x00
	s16 state;

	// 0x02
	s16 startAfterExit;

	// 0x04
	s16 frame;
};

struct TrackSelectRuntimeState
{
	// 0x00
	s32 trackChangeFrames;

	// 0x04
	s16 currentTrack;

	// 0x06
	s16 trackChangeDirection;

	// 0x08
	s16 lapBoxOpen;

	// 0x0a
	struct MenuExitTransitionState transition;

	// 0x10
	s16 videoPreviewFrames;

	// 0x12
	s16 videoMemAllocated;

	// 0x14
	s16 videoStateCurr;

	// 0x16
	s16 videoStatePrev;
};

struct TransitionMeta
{
	s16 distX;
	s16 distY;

	// change when each one starts to move
	s16 headStart;

	s16 currX;
	s16 currY;

	// 0xA -- size
};

struct BattleWeaponMenuItem
{
	u32 enabledWeaponFlag;
	s32 iconID;
};

CTR_STATIC_ASSERT(sizeof(struct BattleWeaponMenuItem) == 0x8);

struct TitleSoundCue
{
	s16 frameToPlay;
	s16 soundID;
};

CTR_STATIC_ASSERT(sizeof(struct TitleSoundCue) == 0x4);
CTR_STATIC_ASSERT(offsetof(struct TitleSoundCue, frameToPlay) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct TitleSoundCue, soundID) == 0x2);

struct TitleCameraPathFrame
{
	SVec3 pos;
	SVec3 rot;
};

CTR_STATIC_ASSERT(sizeof(struct TitleCameraPathFrame) == 0xc);
CTR_STATIC_ASSERT(offsetof(struct TitleCameraPathFrame, pos) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct TitleCameraPathFrame, rot) == 0x6);

struct MainMenuCheatCode
{
#if BUILD == JpnRetail
	s32 holdCode;
#endif
	s32 buttonCount;
	u32 buttons[MM_CHEAT_BUTTON_HISTORY_COUNT];
	void (*handler)(void);
};

#if BUILD == JpnRetail
CTR_STATIC_ASSERT(sizeof(struct MainMenuCheatCode) == 0x34);
CTR_STATIC_ASSERT(offsetof(struct MainMenuCheatCode, holdCode) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct MainMenuCheatCode, buttonCount) == 0x4);
CTR_STATIC_ASSERT(offsetof(struct MainMenuCheatCode, buttons) == 0x8);
CTR_STATIC_ASSERT(offsetof(struct MainMenuCheatCode, handler) == 0x30);
#else
CTR_STATIC_ASSERT(sizeof(struct MainMenuCheatCode) == 0x30);
CTR_STATIC_ASSERT(offsetof(struct MainMenuCheatCode, buttonCount) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct MainMenuCheatCode, buttons) == 0x4);
CTR_STATIC_ASSERT(offsetof(struct MainMenuCheatCode, handler) == 0x2c);
#endif

// 800ab9f0 - UsaRetail
// 800abe04 - EurRetail
// 800aef2c - JpnRetail
struct OverlayRDATA_230
{
	// (tag given by compiler, meaningless to game)
	// do not compile this "tag" in the struct when we're actually building the overlay
	s32 overlayTag;

	// 800ab9f4
	u32 jmpPtrs_Title_MenuUpdate[6];

	// 800aba0c - UsaRetail
	// 800abe20 - EurRetail
	// 800aef48 - JpnRetail
	// "title"
	char s_title[8];

	// 800aba14 - UsaRetail
	// 800abe28 - EurRetail
	// 800aef50 - JpnRetail
	// 0,1,2,3,4,5,6,7
	u32 packedDefaultCharacterIDWords[2];

	// strings for the player numbers in the character select screen
	//
	// 800abe30 - EurRetail
	// 800aef58 - JpnRetail

	// "4"
	char s_4[4];
	// "3"
	char s_3[4];
	// "2"
	char s_2[4];
	// "1"
	char s_1[4];

	// 800aba2c
	u32 jmpPtrs_Characters_MenuProc[6];

	// 800aba44 - UsaRetail
	// 800abe58 - EurRetail
	// 800aef80 - JpnRetail
	char s_loaded_ghost_data[0x18];

	// 800aba6c
	u32 jmpPtrs_Battle_MenuProc[11];

	// 800aba88 - UsaRetail
	// 800abe9c - EurRetail
	// 800aefc4 - JpnRetail
	// " test.str 1"
	char s_teststr1[12];

	// 800aba94
	u32 ptr_MM_TrackSelect_boolTrackOpen;
};

// 800b44e4 - UsaRetail
// 800b4c4c - EurRetail
// 800b7f68 - JpnRetail
struct OverlayDATA_230
{
	// =========== Main Menu CONST =============

	// 800b44e4 - UsaRetail
	// 800b4c4c - EurRetail
	// 800b7f68 - JpnRetail
#if BUILD == EurRetail
	struct MenuRow rowsMainMenuBasic[8];
#else
	struct MenuRow rowsMainMenuBasic[7];
	char padding800b450E[2];
#endif

	// 800b4510 - UsaRetail
	// 800b4c7c - EurRetail
	// 800b7f94 - JpnRetail
#if BUILD == EurRetail
	struct MenuRow rowsMainMenuWithScrapbook[9];
	char padding800b4cb2[2];
#else
	struct MenuRow rowsMainMenuWithScrapbook[8];
#endif

	// 800b4540 - UsaRetail
	// 800b4cb4 - EurRetail
	// 800b7fc4 - JpnRetail
	struct RectMenu menuMainMenu;

	// 800b456c - UsaRetail
	// 800b4ce0 - EurRetail
	// 800b7ff0 - JpnRetail
	struct MenuRow rowsPlayers1P2P[MM_PLAYER_1P2P_SELECTABLE_ROWS + 1];

// ????
#if (BUILD == EurRetail) || (BUILD == UsaRetail)
	char padding800b4cf2[2];
#endif

	// 800b4580 - UsaRetail
	// 800b4cf4 - EurRetail
	// 800b7ffc - JpnRetail
	struct RectMenu menuPlayers1P2P;

	// 800b45ac - UsaRetail
	// 800b4d20 - EurRetail
	// 800b8030 - JpnRetail
	struct MenuRow rowsPlayers2P3P4P[MM_PLAYER_2P3P4P_SELECTABLE_ROWS + 1];

	// 800b45c4 - UsaRetail
	// 800b4d38 - EurRetail
	// 800b8042 - JpnRetail
	struct RectMenu menuPlayers2P3P4P;

	// 800b45f0 - UsaRetail
	// 800b4d64 - EurRetail
	// 800b8074 - JpnRetail
	struct MenuRow rowsDifficulty[4];

	// 800b4608 - UsaRetail
	// 800b4d7c - EurRetail
	// 800b808c - JpnRetail
	struct RectMenu menuDifficulty;

	// 800b4634 - UsaRetail
	// 800b4da8 - EurRetail
	// 800b80b8 - JpnRetail
	struct MenuRow rowsRaceType[3];

#if (BUILD == EurRetail) || (BUILD == UsaRetail)
	char padding800b4dba[2];
#endif

	// 800b4648 - UsaRetail
	// 800b4dbc - EurRetail
	// 800b80cc - JpnRetail
	struct RectMenu menuRaceType;

	// 800b4674 - UsaRetail
	// 800b4de8 - EurRetail
	// 800b80f8 - JpnRetail
	struct MenuRow rowsAdventure[3];

// ???
#if (BUILD == EurRetail) || (BUILD == UsaRetail)
	char padding800b4dfa[2];
#endif

	// 800b4688 - UsaRetail
	// 800b4dfc - EurRetail
	// 800b810c - JpnRetail
	struct RectMenu menuAdventure;

#if BUILD == EurRetail
	// 800b4e28
	s16 langIndex[6];

	// 800b4e34
	struct MenuRow rowsLanguage[7];

	char padding800b4e5e[2];

	// 800b4e60
	struct RectMenu menuLanguage;
#endif

	// 800b46b4 - UsaRetail
	// 800b4e8c - EurRetail
	// 800b8138 - JpnRetail
	struct RectMenu menuCharacterSelect;

	// 800b46e0 - UsaRetail
	// 800b4eb8 - EurRetail
	// 800b8164 - JpnRetail
	struct RectMenu menuTrackSelect;

	// 800b470c - UsaRetail
	// 800b4ee4 - EurRetail
	// 800b8190 - JpnRetail
	struct MenuRow rowsCupSelect[5];

// ????
#if (BUILD == EurRetail) || (BUILD == UsaRetail)
	char padding800b4f02[2];
#endif

	// 800b472c - UsaRetail
	// 800b4f04 - EurRetail
	// 800b81b0 - JpnRetail
	struct RectMenu menuCupSelect;

	// 800b4758 - UsaRetail
	// 800b4f30 - EurRetail
	// 800b81dc - JpnRetail
	struct RectMenu menuBattleWeapons;

	// 800b4784 - UsaRetail
	// 800b4f5c - EurRetail
	// 800b8208 - JpnRetail
	struct RectMenu menuHighScores;

	// 800b47b0 - UsaRetail
	// 800b4f88 - EurRetail
	// 800b8234 - JpnRetail
	struct RectMenu menuScrapbook;

	// 800b47dc - UsaRetail
	// 800b4fb4 - EurRetail
	// 800b8260 - JpnRetail
	// array of menu pointers
#if BUILD == EurRetail
	struct RectMenu *arrayMenuPtrs[MM_MENU_RESET_COUNT];
#else
	struct RectMenu *arrayMenuPtrs[MM_MENU_RESET_COUNT];
#endif

#if BUILD == JpnRetail
	// unknown 2 big structures in NTSC-J
	char unk800b8284[0x570];
#endif

	// 800B4800 - UsaRetail
	// 800b4fdc - EurRetail
	// 800b87f4 - JpnRetail
	struct TitleInstanceMeta
#if BUILD == JpnRetail
	    titleInstances[TITLE_INSTANCE_COUNT_JPN];
#else
	    titleInstances[TITLE_INSTANCE_COUNT];
#endif

	// 800B4830 - UsaRetail
	// 800b500c - EurRetail
	// 800b882c - JpnRetail
	SVec3 titleCameraPos;
	s16 _pad_titleCameraPos;
	SVec3 titleCameraRot;
	s16 _pad_titleCameraRot;

	// 800B4840 - UsaRetail
	// 800b501c - EurRetail
	// 800b883c - JpnRetail
	// random stuff related to the title animation,
	// come up with better names later
#if BUILD == JpnRetail
	char title_OtherStuff[0x8C];
#else

	// Full block is 0x84 bytes

	// 800B4840
	s32 titleMenuTransitionDurationFrames;

	// 800B4844
	s32 titleMenuTransitionStep;

	// 800B4848
	SVec2 titleMainMenuPos;

	// 800B484c
	SVec2 titleAdventureMenuPos;

	// 800B4850
	SVec2 titleRaceTypeMenuPos;

	// 800B4854
	SVec2 titlePlayersMenuPos;

	// 800B4858
	SVec2 titleDifficultyMenuPos;

	// 800B485c
	SVec3 titleBaseCameraPos;
	s16 _pad_titleBaseCameraPos;

	// 800B4864
	union
	{
		struct TransitionMeta transitionMeta_Menu[8];
		struct
		{
			struct TransitionMeta titleMainMenuTransition;
			struct TransitionMeta titleAdventureTransition;
			struct TransitionMeta titleRaceTypeTransition;
			struct TransitionMeta titlePlayersTransition;
			struct TransitionMeta titleDifficultyTransition;
			struct TransitionMeta titleCameraXYTransition;
			struct TransitionMeta titleCameraZTransition;
			struct TransitionMeta titleTransitionEnd;
		};
	};

	// 800B48B4
	char padding_afterTitleTransitions[0x10];

#endif

	// 800b48c4 - UsaRetail
	// 800b50a0 - EurRetail
	// 800b88c8 - JpnRetail
#if BUILD != JpnRetail
	struct TitleSoundCue titleSounds[8];
#else
	struct TitleSoundCue titleSounds[7];

	char unkTitleData[0x18];
#endif

	// 800b48e4 - UsaRetail
	// 800b50c0 - EurRetail
	// 800b88fc - JpnRetail
	struct MainMenuCheatCode cheats[MM_CHEAT_COUNT];

	// 800B4D04 - UsaRetail
	// 800b54e0 - EurRetail
	// 800B8D40 - JpnRetail
	u32 cheatButtonHistory[MM_CHEAT_BUTTON_HISTORY_COUNT];

	// 800B4D2C - UsaRetail
	// 800b5508 - EurRetail
	// 800B8D68 - JpnRetail
	struct CupDifficultyTables cupDifficulty;

	// ============= Character Select CONST ================

	// 800B4D44 - UsaRetail
	// 800b5520 - EurRetail
	// 800b8d80 - JpnRetail
	SVec2 characterSelectWindowPos[0xD];

	// pointer
	// 800b4d78 - UsaRetail
	// 800b5554 - EurRetail
	SVec2 *characterSelectWindowPosByLayout[6];

	// 800B4D90 - UsaRetail
	// 800b556c - EurRetail
	struct CharacterSelectLayoutTables characterSelectLayout;

	// 800b4dcc - UsaRetail
	// 800b55a8 - EurRetail
	struct CharacterSelectMeta characterSelectMeta1P2PLimited[0xF];

	// 800b4e80 - UsaRetail
	// 800b565c - EurRetail
	struct CharacterSelectMeta characterSelectMeta1P2P[0xF];

	// 800b4f34 - UsaRetail
	// 800b5710 - EurRetail
	struct CharacterSelectMeta characterSelectMeta3P[0xF];

	// 800b4fe8 - UsaRetail
	// 800b57c4 - EurRetail
	struct CharacterSelectMeta characterSelectMeta4P[0xF];

	// 800b509C - UsaRetail
	// 800b5878 - EurRetail
	struct CharacterSelectMeta *characterSelectMetaByLayout[6];

	// 800b50B4 - UsaRetail
	// 800b5890 - EurRetail
	// the character select menu has a different order for playable characters
	// this array contains the IDs used for each character in the character select menu
	// each member of the array corresponds to the character order used in the rest of the game, see enum Characters
	s16 characterMenuID[0x10];

	// 0x15 for transition meta array:
	// 14 character icons + title text + 4 kart screens + 2 more?

	// 800b50D4 - UsaRetail
	// 800b58b0 - EurRetail
	// 1P/2P mode
	struct TransitionMeta characterSelectTransition1P2P[0x15];

	// 0x2 byte padding
	s16 padding800b51A6;

	// 3P mode
	// 800b51A8 - UsaRetail
	// 800b5984 - EurRetail
	struct TransitionMeta characterSelectTransition3P[0x15];

	// 0x2 byte padding
	s16 padding800B527A;

	// 4P mode
	// 800b527c - UsaRetail
	// 800b5a58 - EurRetail
	struct TransitionMeta characterSelectTransition4P[0x15];

	// 0x2 byte padding
	s16 padding800B534E;

	// 800B5350 - UsaRetail
	// 800b5b2c - EurRetail
	struct TransitionMeta *characterSelectTransitionByPlayerCount[4];

	// 800B5360 - UsaRetail
	// 800b5b3c - EurRetail
	struct CharacterSelectDriverModelConfig characterSelectDriverModel;

	// 800b5374 - UsaRetail
	// 800b5b50 - EurRetail
	// points to s_1, s_2, s_3, s_4
	char *playerNumberStrings[4];

	// 800b5384
	u8 characterSelectFallbackDirection1[CHARACTER_SELECT_DIRECTION_COUNT];

	// 800b5388
	u8 characterSelectFallbackDirection2[CHARACTER_SELECT_DIRECTION_COUNT];

	// 800b538c -- UsaRetail
	Color characterSelect_Outline;

	// 800b5390 - UsaRetail
	// 800b5b6c - EurRetail
	Color characterSelect_NeutralColor;

	// 800b5394 - UsaRetail
	// 800b5b70 - EurRetail
	Color characterSelect_ChosenColor;

	// 800b5398 - UsaRetail
	// 800b5b74 - EurRetail
	char characterSelect_BlueRectColors[0x18];
	// u32 characterSelect_BlueRectColors[6];

	// =========== Track Select CONST ============

	// 800b53b0 - UsaRetail
	// 800b5b8c - EurRetail
	// 800b93ec - JpnRetail
	struct MainMenu_LevelRow arcadeTracks[0x12];

	// 800b54d0 - UsaRetail
	// 800b5cf4 - EurRetail
	// 800b9554 - JpnRetail
	struct MainMenu_LevelRow battleTracks[0x7];

	// 800b5540 - UsaRetail
	// 800b5d80 - EurRetail
	// 800b95e0 - JpnRetail
	union
	{
		struct TransitionMeta transitionMeta_trackSel[5];
		struct
		{
			struct TransitionMeta trackSelect_rowListTransition;
			struct TransitionMeta trackSelect_previewTransition;
			struct TransitionMeta trackSelect_lapMenuTransition;
			struct TransitionMeta trackSelect_titleTransition;
			struct TransitionMeta trackSelect_transitionEnd;
		};
	};
	s16 padding_800B5572;

	// 800b5574
	struct LapCountMenuRow lapCountByRow[4];

	// 800b557c
	struct MenuRow rowsLapSel[4];

	// 800B5594
	struct RectMenu menuLapSel;

	// 800B55C0
	Color videoCol;

	// 800B55C4
	struct TimeTrialStarTables timeTrialStars;

#if BUILD == JpnRetail
	// 800b966c
	char wumpaShadowRGBA[4];
#endif

	// 800b55cc -- UsaRetail
	// 800b9670 -- JpnRetail
	struct
	{
		s16 offsetX;
		s16 offsetY;
		s16 type;
	} drawMapOffset[6];

	// ============== Cup Select ==================

	// 800b55f0 - UsaRetail
	// 800b5e30 - EurRetail
	// 800b9694 - JpnRetail
	struct TransitionMeta transitionMeta_cupSel[0x6];

	// 800b562c - UsaRetail
	// 800b5e6c - EurRetail
	// 800b96d0 - JpnRetail
	struct CupSelectStarTables cupSelectStars;

	// 800b563c - UsaRetail
	// 800b5e7c - EurRetail
	// 800b96e0 - JpnRetail
	Color cupSel_Color;

	// ============= Battle CONST ================

	// 800b5640 - UsaRetail
	// 800b5e80 - EurRetail
	// 800b96e4 - JpnRetail
	struct TransitionMeta transitionMeta_battle[0xB];

	// 2 byte padding
	s16 padding800b56ae;

	// 800b56b0 - UsaRetail
	// 800b5ef0 - EurRetail
	// 800b9754 - JpnRetail
	struct MenuRow rowsBattleType[4];

	// 800b56c8 - UsaRetail
	// 800b5f08 - EurRetail
	// 800b976c - JpnRetail
	struct RectMenu menuBattleType;

	// 800b56f4 - UsaRetail
	// 800b5f34 - EurRetail
	// 800b9798 - JpnRetail
	struct MenuRow rowsBattleLengthLifeTime[4];

	// 800b570c - UsaRetail
	// 800b5f4c - EurRetail
	// 800b97b0 - JpnRetail
	struct RectMenu menuBattleLengthLifeTime;

	// 800b5738 - UsaRetail
	// 800b5f78 - EurRetail
	// 800b97dc - JpnRetail
	struct MenuRow rowsBattleLengthTimeTime[4];

	// 800b5750 - UsaRetail
	// 800b5f90 - EurRetail
	// 800b97f4 - JpnRetail
	struct RectMenu menuBattleLengthTimeTime;

	// 800b577c - UsaRetail
	// 800b5fbc - EurRetail
	// 800b9820 - JpnRetail
	struct MenuRow rowsBattleLengthPoints[4];

	// 800b5794 - UsaRetail
	// 800b5fd4 - EurRetail
	// 800b9838 - JpnRetail
	struct RectMenu menuBattleLengthPoints;

	// 800b57c0 - UsaRetail
	// 800b6000 - EurRetail
	// 800b9864 - JpnRetail
	struct MenuRow rowsBattleLengthLifeLife[4];

	// 800b57d8 - UsaRetail
	// 800b6018 - EurRetail
	// 800b987c - JpnRetail
	struct RectMenu menuBattleLengthLifeLife;

	// 800b5804 - UsaRetail
	// 800b6044 - EurRetail
	// 800b98a8 - JpnRetail
	struct MenuRow rowsBattleStartGame[2];

	// 800b5810 - UsaRetail
	// 800b6050 - EurRetail
	// 800b98b4 - JpnRetail
	struct RectMenu menuBattleStartGame;

	// 800b583c - UsaRetail
	// 800b607c - EurRetail
	// 800b98e0 - JpnRetail
	struct RectMenu *battleMenuArray[5];

	// 800b5850 - UsaRetail
	// 800b6090 - EurRetail
	// 800b98f4 - JpnRetail
	struct BattleWeaponMenuItem battleWeaponItems[11];

	// 800b58a8 - UsaRetail
	// 800b60e8 - EurRetail
	// 800b994c - JpnRetail
	struct BattleSetupTables battleSetupTables;

	// 800b58c4 - UsaRetail
	// 800b6104 - EurRetail
	// 800b9968 - JpnRetail
	Color battleWeaponEnabledColor;
	Color battleWeaponDisabledColor;
	Color battleWeaponPanelColor;

	// ================ High Score CONST ==================

	// 800b58d0 - UsaRetail
	// 800b6110 - EurRetail
	// 800b9974 - JpnRetail
	struct TransitionMeta transitionMeta_HighScores[0xC];

	// 800b5948 - UsaRetail
	// 800b6188 - EurRetail
	// 800b99ec - JpnRetail
	struct HighScoreGhostStarTables highScoreGhostStars;

	// 800b5950 - UsaRetail
	// 800b6190 - EurRetail
	// 800b99f4 - JpnRetail
	Color highscore_iconColor;

	// 800b5954 - UsaRetail
	// 800b6194 - EurRetail
	// 800b99f8 - JpnRetail
	struct MenuRow rowsHighScore[4];

	// 800b596c - UsaRetail
	// 800b61ac - EurRetail
	// 800b9a10 - JpnRetail
	struct RectMenu menuHighScore;

	// 800B5998 - UsaRetail
	// 800b61d8 - EurRetail
	// 800b9a3c - JpnRetail
	struct HighScoreSelectionState highScoreSelection;

#if BUILD == EurRetail

	// 800b61e0 - EurRetail
	// why not just use the one at 800b4e28?
	s16 fileIndexLngBoot[6];

	// 800b61ec - EurRetail
	struct MenuRow rowsLngBoot[7];

	char padding800b6216[2];

	// 800b6218 - EurRetail
	// Language menu on game start
	struct RectMenu menuLngBoot;

#elif BUILD == JpnRetail
	// 800b9a44 - JpnRetail
	char unk800b9a44[8];
#endif

	// 800b59a0 - UsaRetail
	// 800b6244 - EurRetail
	// 800b9a4c - JpnRetail
	struct Title *titleObj;

	// ============== Track Select DYN ====================

	// 800b59a4 - UsaRetail
	// 800b6248 - EurRetail
	// 800b9a50 - JpnRetail
	struct TrackSelectRuntimeState trackSelect;

	// ============== Cup Select DYN ========================

	// 800b59bc - UsaRetail
	// 800b6260 - EurRetail
	struct MenuExitTransitionState cupSelectTransition;

	// ============== Battle DYN ==========================

	// 800b59c2 - UsaRetail
	// 800b6266 - EurRetail
	struct MenuExitTransitionState battleTransition;

	// =============== High Score DYN ===================

	// 800b59c8 - UsaRetail
	// 800b626c - EurRetail
	struct HighScoreTransitionState highScoreTransition;

	// =============== Scrapbook ===================

	// 800b59d8 - UsaRetail
	// 800b627c - EurRetail
	ScrapbookState scrapbookState;

	// ============= Character Select DYN ====================

	// 800b59dc - UsaRetail
	// 800b6280 - EurRetail
	s16 characterSelectWindowHeight;
	s16 pad_afterCharacterSelectWindowHeight;

	// 800b59e0 - UsaRetail
	// 800b6284 - EurRetail
	// 800b9a8c - JpnRetail
	MainMenuExitRoute desiredMenuIndex;

	// 800b59e4 -- UsaRetail
	// 800b6288 -- EurRetail
	// 800b9a90 -- JpnRetail
	s16 characterSelectExitsForward;
	s16 pad_afterCharacterSelectExitsForward;

	// 800b59e8 -- UsaRetail
	// 800b628c -- EurRetail
	struct CharacterSelectPlayerState characterSelectPlayerState;

	// 800b5a08 - UsaRetail
	// 800b62ac - EurRetail
	// 800b9ab4 - JpnRetail
	enum TransitionState characterSelectTransitionState;

	// 800b5a0c - UsaRetail
	// 800b62b0 - EurRetail
	SVec2 *activeCharacterSelectWindowPos;

	// 800b5a10 - UsaRetail
	// 800b62b4 - EurRetail
	s32 characterSelectLayoutIndex;

	// 800b5a14 - UsaRetail
	// 800b62b8 - EurRetail
	// 800b9ac0 - JpnRetail
	s32 titleIntroFrame;

	// 800b5a18 - UsaRetail
	// 800b62bc - EurRetail
	struct CharacterSelectMeta *activeCharacterSelectMeta;

	// 800b5a1c - UsaRetail
	// 800b62c0 - EurRetail
	// 800b9ac8 - JpnRetail
	enum TitleMenuState titleMenuState;

	// 800b5a20 - UsaRetail
	// 800b62c4 - EurRetail
	// 800b9acc - JpnRetail
	// 0 = transitioning in, 1 = in focus/still, 2 = transitioning out
	enum TransitionState characterSelectMenuState;

	// 800b5a24 - UsaRetail
	// 800b62c8 - EurRetail
	// 800b9ad0 - JpnRetail
	s16 characterSelectModelMoveTimer[4];

#if BUILD == EurRetail
	// 800b62d0
	// starts at 900 frames (30 secs). If ran out (-1),
	// menu will automatically choose current selection
	s32 langMenuTimer;
#endif

	// 800b5a2c - UsaRetail
	// 800b62d4 - EurRetail
	// 800b9ad8 - JpnRetail
	b32 characterSelectRosterExpanded;

	// 800b5a30 - UsaRetail
	// 800b62d8 - EurRetail
	// 800b9adc - JpnRetail
	s32 characterSelectWindowWidth;

	// 800b5a34 - UsaRetail
	// 800b62dc - EurRetail
	// 800b9ae0 - JpnRetail
	struct TitleCameraPathFrame *titleIntroCameraPath;

	// 800b5a38 - UsaRetail
	// 800b62e0 - EurRetail
	// 800b9ae4 - JpnRetail
	s32 characterSelectNameTextY;

	// 800b5a3c - UsaRetail
	// 800b62e4 - EurRetail
	// 800b9ae8 - JpnRetail
	struct TransitionMeta *characterSelectTransitionMeta;

	// 800b5a40 - UsaRetail
	// 800b62e8 - EurRetail
	// 800b9aec - JpnRetail
	s32 titleMenuTransitionFrame;

	// 800b5a44 - UsaRetail
	// 800b62ec - EurRetail
	// 800b9af0 - JpnRetail
	s32 characterSelectTransitionFrame;

	// ================= Video RDATA ===========================

	// 800b5a48 - UsaRetail
	// 800b62f0 - EurRetail
	// 800b9af4 - JpnRetail
	char s_SliceBuf[0xC];
	char s_VlcBuf[8];
	char s_RingBuf[8];

	// 800b5a64 MM_Video_DecDCToutCallbackFunc
};

// starts at 800b67ac
struct OVR_230_VideoBSS
{
	s32 loopStartBackloc;
	s32 loopEndBackloc;
	s32 loopWrapPending;
	s32 cdRetryState;

	// 800b67bc
	s16 finalSliceIndex;

	// 800b67be
	s16 sliceIndex;

	// 800b67c0
	s16 dctMode;

	// 800b67c2
	s16 drawNextFrame;

	// 800b67c4
	s16 endOfStream;
	s16 decodeState;

	// 800b67c8
	s16 vlcBufferIndex;

	// 800b67ca
	s16 dctOutBufferIndex;

	// 800b67cc
	s16 cdKickState;
	s16 stalledBacklocFrames;

	// 800b67d0
	s32 stallRecoveryFrames;

	// 800b67d4
	s32 streamFrameCount;

	// 800b67d8
	s32 sectorFrameCount;
	s32 lastSectorFrameCount;
	s32 lastBackloc;
	s32 unused_0x38;

	// 800b67e8
	// 1 - IS_RGB24
	// 2 - has Audio
	// 4 - is looping
	// 8 - is Scrapbook (?)
	u32 flags;

	// 800b67ec
	s32 ringSectorCount;

	// 800b67f0
	u32 dctOutputDone;

	s32 vlcBufferSize;

	char pad800b67f8[4];

	// 30,31...
	// 800b67fc
	uint32_t *in_Buf[2];

	s32 dctOutSliceSize;

	char pad800b6808[4];

	// 800b680c
	uint32_t *out_Buf[4];

	// 800b681c
	RECT slice;

	// 800b6824
	CdlLOC cdLocation1;

	// 800b6828
	CdlLOC cdLocation2;

	// 800b682c
	CdlLOC cdLocation3;

	// 800b6830
	CdlLOC *ptrCdLoc;
};

CTR_STATIC_ASSERT(sizeof(struct OVR_230_VideoBSS) == 0x88);
CTR_STATIC_ASSERT(offsetof(struct OVR_230_VideoBSS, loopStartBackloc) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct OVR_230_VideoBSS, loopEndBackloc) == 0x4);
CTR_STATIC_ASSERT(offsetof(struct OVR_230_VideoBSS, loopWrapPending) == 0x8);
CTR_STATIC_ASSERT(offsetof(struct OVR_230_VideoBSS, cdRetryState) == 0xc);
CTR_STATIC_ASSERT(offsetof(struct OVR_230_VideoBSS, finalSliceIndex) == 0x10);
CTR_STATIC_ASSERT(offsetof(struct OVR_230_VideoBSS, sliceIndex) == 0x12);
CTR_STATIC_ASSERT(offsetof(struct OVR_230_VideoBSS, dctMode) == 0x14);
CTR_STATIC_ASSERT(offsetof(struct OVR_230_VideoBSS, dctOutBufferIndex) == 0x1e);
CTR_STATIC_ASSERT(offsetof(struct OVR_230_VideoBSS, stallRecoveryFrames) == 0x24);
CTR_STATIC_ASSERT(offsetof(struct OVR_230_VideoBSS, streamFrameCount) == 0x28);
CTR_STATIC_ASSERT(offsetof(struct OVR_230_VideoBSS, sectorFrameCount) == 0x2c);
CTR_STATIC_ASSERT(offsetof(struct OVR_230_VideoBSS, unused_0x38) == 0x38);
CTR_STATIC_ASSERT(offsetof(struct OVR_230_VideoBSS, ringSectorCount) == 0x40);
CTR_STATIC_ASSERT(offsetof(struct OVR_230_VideoBSS, dctOutputDone) == 0x44);

extern struct OverlayRDATA_230 R230;
extern struct OverlayDATA_230 D230;
extern struct OVR_230_VideoBSS V230;


#if BUILD >= EurRetail
CTR_STATIC_ASSERT(sizeof(struct MainMenu_LevelRow) == 0x14);
CTR_STATIC_ASSERT(offsetof(struct MainMenu_LevelRow, previewVideoFileIndex) == 0xc);
CTR_STATIC_ASSERT(offsetof(struct MainMenu_LevelRow, previewVideoFrameCount) == 0x10);
#else
CTR_STATIC_ASSERT(sizeof(struct MainMenu_LevelRow) == 0x10);
CTR_STATIC_ASSERT(offsetof(struct MainMenu_LevelRow, previewVideoFileIndex) == 0x8);
CTR_STATIC_ASSERT(offsetof(struct MainMenu_LevelRow, previewVideoFrameCount) == 0xc);
#endif
CTR_STATIC_ASSERT(offsetof(struct OverlayRDATA_230, overlayTag) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct OverlayRDATA_230, s_title) == 0x1c);
CTR_STATIC_ASSERT(offsetof(struct OverlayRDATA_230, packedDefaultCharacterIDWords) == 0x24);
CTR_STATIC_ASSERT(sizeof(struct CharacterSelectMeta) == 0xC);
CTR_STATIC_ASSERT(offsetof(struct CharacterSelectMeta, nextIconByDirection) == 0x4);
CTR_STATIC_ASSERT(offsetof(struct CharacterSelectMeta, characterID) == 0x8);
CTR_STATIC_ASSERT(offsetof(struct CharacterSelectMeta, unlockFlags) == 0xa);
CTR_STATIC_ASSERT(sizeof(struct CupDifficultyTables) == 0x18);
CTR_STATIC_ASSERT(offsetof(struct CupDifficultyTables, firstUnlockBit) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct CupDifficultyTables, stringIndex) == 0x8);
CTR_STATIC_ASSERT(offsetof(struct CupDifficultyTables, speed) == 0x10);
CTR_STATIC_ASSERT(sizeof(struct CharacterSelectLayoutTables) == 0x3c);
CTR_STATIC_ASSERT(offsetof(struct CharacterSelectLayoutTables, windowW) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct CharacterSelectLayoutTables, windowH) == 0xc);
CTR_STATIC_ASSERT(offsetof(struct CharacterSelectLayoutTables, driverPosZ) == 0x18);
CTR_STATIC_ASSERT(offsetof(struct CharacterSelectLayoutTables, driverPosY) == 0x24);
CTR_STATIC_ASSERT(offsetof(struct CharacterSelectLayoutTables, textY) == 0x30);
CTR_STATIC_ASSERT(sizeof(struct CharacterSelectPlayerState) == 0x20);
CTR_STATIC_ASSERT(offsetof(struct CharacterSelectPlayerState, modelMoveDir) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct CharacterSelectPlayerState, desiredCharacterID) == 0x8);
CTR_STATIC_ASSERT(offsetof(struct CharacterSelectPlayerState, currentCharacterID) == 0x10);
CTR_STATIC_ASSERT(offsetof(struct CharacterSelectPlayerState, angle) == 0x18);
CTR_STATIC_ASSERT(sizeof(struct CharacterSelectDriverModelConfig) == 0x14);
CTR_STATIC_ASSERT(offsetof(struct CharacterSelectDriverModelConfig, pos) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct CharacterSelectDriverModelConfig, rot) == 0x8);
CTR_STATIC_ASSERT(offsetof(struct CharacterSelectDriverModelConfig, moveFrames) == 0xe);
CTR_STATIC_ASSERT(offsetof(struct CharacterSelectDriverModelConfig, slideDistance) == 0x10);
CTR_STATIC_ASSERT(sizeof(struct TimeTrialStarTables) == 0x8);
CTR_STATIC_ASSERT(offsetof(struct TimeTrialStarTables, colorIndex) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct TimeTrialStarTables, beatenFlagBit) == 0x4);
CTR_STATIC_ASSERT(sizeof(struct CupSelectStarTables) == 0x10);
CTR_STATIC_ASSERT(offsetof(struct CupSelectStarTables, colorIndex) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct CupSelectStarTables, winBitBase) == 0x8);
CTR_STATIC_ASSERT(sizeof(struct BattleSetupTables) == 0x1c);
CTR_STATIC_ASSERT(offsetof(struct BattleSetupTables, typeModeFlags) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct BattleSetupTables, timeLimitMinutes) == 0xc);
CTR_STATIC_ASSERT(offsetof(struct BattleSetupTables, lifeModeTimeLimitMinutes) == 0x10);
CTR_STATIC_ASSERT(offsetof(struct BattleSetupTables, lifeLimitValues) == 0x14);
CTR_STATIC_ASSERT(offsetof(struct BattleSetupTables, pointLimitValues) == 0x18);
CTR_STATIC_ASSERT(sizeof(struct HighScoreGhostStarTables) == 0x8);
CTR_STATIC_ASSERT(offsetof(struct HighScoreGhostStarTables, colorIndex) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct HighScoreGhostStarTables, beatenFlagBit) == 0x4);
CTR_STATIC_ASSERT(sizeof(struct LapCountMenuRow) == 0x2);
CTR_STATIC_ASSERT(offsetof(struct LapCountMenuRow, lapCount) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct LapCountMenuRow, padding) == 0x1);
CTR_STATIC_ASSERT(sizeof(struct HighScoreSelectionState) == 0x8);
CTR_STATIC_ASSERT(offsetof(struct HighScoreSelectionState, targetTrack) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct HighScoreSelectionState, targetRow) == 0x2);
CTR_STATIC_ASSERT(offsetof(struct HighScoreSelectionState, currentTrack) == 0x4);
CTR_STATIC_ASSERT(offsetof(struct HighScoreSelectionState, currentRow) == 0x6);
CTR_STATIC_ASSERT(sizeof(struct HighScoreTransitionState) == 0x10);
CTR_STATIC_ASSERT(offsetof(struct HighScoreTransitionState, state) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct HighScoreTransitionState, frame) == 0x2);
CTR_STATIC_ASSERT(offsetof(struct HighScoreTransitionState, mainFrame) == 0x2);
CTR_STATIC_ASSERT(offsetof(struct HighScoreTransitionState, trackFrame) == 0x4);
CTR_STATIC_ASSERT(offsetof(struct HighScoreTransitionState, rowFrame) == 0x6);
CTR_STATIC_ASSERT(offsetof(struct HighScoreTransitionState, horizontalMove) == 0x8);
CTR_STATIC_ASSERT(offsetof(struct HighScoreTransitionState, activeHorizontalMove) == 0x8);
CTR_STATIC_ASSERT(offsetof(struct HighScoreTransitionState, pendingHorizontalMove) == 0xa);
CTR_STATIC_ASSERT(offsetof(struct HighScoreTransitionState, verticalMove) == 0xc);
CTR_STATIC_ASSERT(offsetof(struct HighScoreTransitionState, activeVerticalMove) == 0xc);
CTR_STATIC_ASSERT(offsetof(struct HighScoreTransitionState, pendingVerticalMove) == 0xe);
CTR_STATIC_ASSERT(sizeof(struct MenuExitTransitionState) == 0x6);
CTR_STATIC_ASSERT(offsetof(struct MenuExitTransitionState, state) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct MenuExitTransitionState, startAfterExit) == 0x2);
CTR_STATIC_ASSERT(offsetof(struct MenuExitTransitionState, frame) == 0x4);
CTR_STATIC_ASSERT(sizeof(struct TrackSelectRuntimeState) == 0x18);
CTR_STATIC_ASSERT(offsetof(struct TrackSelectRuntimeState, trackChangeFrames) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct TrackSelectRuntimeState, currentTrack) == 0x4);
CTR_STATIC_ASSERT(offsetof(struct TrackSelectRuntimeState, trackChangeDirection) == 0x6);
CTR_STATIC_ASSERT(offsetof(struct TrackSelectRuntimeState, lapBoxOpen) == 0x8);
CTR_STATIC_ASSERT(offsetof(struct TrackSelectRuntimeState, transition) == 0xa);
CTR_STATIC_ASSERT(offsetof(struct TrackSelectRuntimeState, videoPreviewFrames) == 0x10);
CTR_STATIC_ASSERT(offsetof(struct TrackSelectRuntimeState, videoMemAllocated) == 0x12);
CTR_STATIC_ASSERT(offsetof(struct TrackSelectRuntimeState, videoStateCurr) == 0x14);
CTR_STATIC_ASSERT(offsetof(struct TrackSelectRuntimeState, videoStatePrev) == 0x16);
CTR_STATIC_ASSERT(sizeof(struct TransitionMeta) == 0xA);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelectWindowPos) == 0x860);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelectWindowPosByLayout) == 0x894);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelectMeta1P2PLimited) == 0x8e8);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelectMeta1P2P) == 0x99c);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelectMeta3P) == 0xa50);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelectMeta4P) == 0xb04);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelectMetaByLayout) == 0xbb8);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterMenuID) == 0xbd0);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelectTransition1P2P) == 0xbf0);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelectTransition3P) == 0xcc4);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelectTransition4P) == 0xd98);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelectTransitionByPlayerCount) == 0xe6c);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelectDriverModel) == 0xe7c);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, playerNumberStrings) == 0xe90);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelectFallbackDirection1) == 0xea0);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelectFallbackDirection2) == 0xea4);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelect_Outline) == 0xea8);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelect_NeutralColor) == 0xeac);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelect_ChosenColor) == 0xeb0);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, titleMenuTransitionDurationFrames) == 0x35c);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, titleMenuTransitionStep) == 0x360);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, titleMainMenuPos) == 0x364);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, titleAdventureMenuPos) == 0x368);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, titleRaceTypeMenuPos) == 0x36c);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, titlePlayersMenuPos) == 0x370);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, titleDifficultyMenuPos) == 0x374);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, padding_afterTitleTransitions) == 0x3d0);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, titleSounds) == 0x3e0);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, cheats) == 0x400);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, cheatButtonHistory) == 0x820);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, cupDifficulty) == 0x848);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelectLayout) == 0x8ac);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, lapCountByRow) == 0x1090);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, timeTrialStars) == 0x10e0);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, cupSelectStars) == 0x1148);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, battleSetupTables) == 0x13c4);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, highScoreGhostStars) == 0x1464);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, highScoreSelection) == 0x14b4);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, trackSelect) == 0x14c0);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, cupSelectTransition) == 0x14d8);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, battleTransition) == 0x14de);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, highScoreTransition) == 0x14e4);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelectWindowHeight) == 0x14f8);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, desiredMenuIndex) == 0x14fc);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelectExitsForward) == 0x1500);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelectTransitionState) == 0x1524);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, activeCharacterSelectWindowPos) == 0x1528);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelectLayoutIndex) == 0x152c);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, titleIntroFrame) == 0x1530);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, activeCharacterSelectMeta) == 0x1534);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, titleMenuState) == 0x1538);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelectMenuState) == 0x153c);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelectModelMoveTimer) == 0x1540);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelectRosterExpanded) == 0x1548);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelectWindowWidth) == 0x154c);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, titleIntroCameraPath) == 0x1550);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelectNameTextY) == 0x1554);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelectTransitionMeta) == 0x1558);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, titleMenuTransitionFrame) == 0x155c);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelectTransitionFrame) == 0x1560);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, videoCol) == 0x10dc);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, cupSel_Color) == 0x1158);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, battleWeaponEnabledColor) == 0x13e0);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, battleWeaponDisabledColor) == 0x13e4);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, battleWeaponPanelColor) == 0x13e8);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, highscore_iconColor) == 0x146c);
CTR_STATIC_ASSERT(offsetof(struct OverlayDATA_230, characterSelectPlayerState) == 0x1504);

#endif
