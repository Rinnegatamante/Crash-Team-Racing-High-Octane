#ifndef CTR_NATIVE_OVR_232_H
#define CTR_NATIVE_OVR_232_H


struct MaskHint
{
	struct MaskHint *self; // why?
	s16 scale;

	// end of struct
};

enum AdventureHubCounts
{
	AH_BOSS_KEY_COUNT = 4,
	AH_HUB_TRACK_COUNT = 4,
	AH_WOOD_DOOR_KEY_COUNT = 4,
};

CTR_STATIC_ASSERT(AH_BOSS_KEY_COUNT == 4);
CTR_STATIC_ASSERT(AH_HUB_TRACK_COUNT == 4);
CTR_STATIC_ASSERT(AH_WOOD_DOOR_KEY_COUNT == 4);

enum AHHintMenuConstants
{
	AH_HINTMENU_ARROW_COUNT = 5,
	AH_HINTMENU_ARROW_SPACING = 0x32,
	AH_HINTMENU_ARROW_START_X = 0x95,
	AH_HINTMENU_ARROW_Y_OFFSET = 4,
	AH_HINTMENU_VISIBLE_ROWS = 5,
	AH_HINTMENU_SCROLL_MARGIN = AH_HINTMENU_VISIBLE_ROWS - 1,
	AH_HINTMENU_VIEW_COOLDOWN_FRAMES = 30,
	AH_HINTMENU_HINT_STRING_COUNT = 32,
	AH_HINTMENU_HINT_LNG_FIRST = LNG_GREETINGS,
	AH_HINTMENU_INPUT_VIEW_EXIT = BTN_CROSS_one | BTN_CIRCLE | BTN_SQUARE_one | BTN_TRIANGLE,
	AH_HINTMENU_INPUT_NAV = BTN_UP | BTN_DOWN | BTN_CROSS_one | BTN_CIRCLE | BTN_SQUARE_one | BTN_TRIANGLE,
	AH_HINTMENU_INPUT_CONFIRM = BTN_CROSS_one | BTN_CIRCLE,
	AH_HINTMENU_INPUT_BACK = BTN_TRIANGLE | BTN_SQUARE_one,
	AH_HINTMENU_INPUT_CLOSE = BTN_START | BTN_SQUARE_one | BTN_TRIANGLE,
};

CTR_STATIC_ASSERT(AH_HINTMENU_ARROW_COUNT == 5);
CTR_STATIC_ASSERT(AH_HINTMENU_ARROW_SPACING == 0x32);
CTR_STATIC_ASSERT(AH_HINTMENU_ARROW_START_X == 0x95);
CTR_STATIC_ASSERT(AH_HINTMENU_ARROW_Y_OFFSET == 4);
CTR_STATIC_ASSERT(AH_HINTMENU_VISIBLE_ROWS == 5);
CTR_STATIC_ASSERT(AH_HINTMENU_SCROLL_MARGIN == 4);
CTR_STATIC_ASSERT(AH_HINTMENU_VIEW_COOLDOWN_FRAMES == 30);
CTR_STATIC_ASSERT(AH_HINTMENU_HINT_STRING_COUNT == 32);
CTR_STATIC_ASSERT(AH_HINTMENU_HINT_LNG_FIRST == 0x17b);
CTR_STATIC_ASSERT(AH_HINTMENU_INPUT_VIEW_EXIT == 0x40070);
CTR_STATIC_ASSERT(AH_HINTMENU_INPUT_NAV == 0x40073);
CTR_STATIC_ASSERT(AH_HINTMENU_INPUT_CONFIRM == 0x50);
CTR_STATIC_ASSERT(AH_HINTMENU_INPUT_BACK == 0x40020);
CTR_STATIC_ASSERT(AH_HINTMENU_INPUT_CLOSE == 0x41020);

enum AHMaskHintState
{
	AH_MASKHINT_STATE_IDLE = 0,
	AH_MASKHINT_STATE_REPEAT_PROMPT = 5,
};

enum AHMaskHintConstants
{
	AH_MASKHINT_SHORT_SPAWN_FRAMES = 20,
	AH_MASKHINT_LONG_SPAWN_FRAMES = CTR_SECONDS_TO_FRAMES(3),
	AH_MASKHINT_CAMERA_DELAY_FRAMES = CTR_SECONDS_TO_FRAMES(2),
	AH_MASKHINT_INTERRUPT_DONE_DELAY_FRAMES = CTR_SECONDS_TO_FRAMES(1),
	AH_MASKHINT_MAX_START_SPEED = 0x31,
	AH_MASKHINT_FULL_BLEND = FP_ONE,
	AH_MASKHINT_SPAWN_RING_FRAMES = 20,
	AH_MASKHINT_SPAWN_SPIRAL_RADIUS = 50,
	AH_MASKHINT_SPAWN_PARTICLES = 3,
	AH_MASKHINT_LEAVE_PARTICLES = 0x18,
	AH_MASKHINT_VANISH_PARTICLES = 20,
	AH_MASKHINT_SFX_SPAWN = 0x100,
	AH_MASKHINT_SFX_VANISH = 0x101,
};

CTR_STATIC_ASSERT(AH_MASKHINT_STATE_IDLE == 0);
CTR_STATIC_ASSERT(AH_MASKHINT_STATE_REPEAT_PROMPT == 5);
CTR_STATIC_ASSERT(AH_MASKHINT_SHORT_SPAWN_FRAMES == 20);
CTR_STATIC_ASSERT(AH_MASKHINT_LONG_SPAWN_FRAMES == 90);
CTR_STATIC_ASSERT(AH_MASKHINT_CAMERA_DELAY_FRAMES == 60);
CTR_STATIC_ASSERT(AH_MASKHINT_INTERRUPT_DONE_DELAY_FRAMES == 30);
CTR_STATIC_ASSERT(AH_MASKHINT_MAX_START_SPEED == 0x31);
CTR_STATIC_ASSERT(AH_MASKHINT_FULL_BLEND == 0x1000);
CTR_STATIC_ASSERT(AH_MASKHINT_SPAWN_RING_FRAMES == 20);
CTR_STATIC_ASSERT(AH_MASKHINT_SPAWN_SPIRAL_RADIUS == 50);
CTR_STATIC_ASSERT(AH_MASKHINT_SPAWN_PARTICLES == 3);
CTR_STATIC_ASSERT(AH_MASKHINT_LEAVE_PARTICLES == 0x18);
CTR_STATIC_ASSERT(AH_MASKHINT_VANISH_PARTICLES == 20);
CTR_STATIC_ASSERT(AH_MASKHINT_SFX_SPAWN == 0x100);
CTR_STATIC_ASSERT(AH_MASKHINT_SFX_VANISH == 0x101);

enum BossGarageDoorDirectionID
{
	BOSS_GARAGE_DOOR_CLOSING = -1,
	BOSS_GARAGE_DOOR_STOPPED = 0,
	BOSS_GARAGE_DOOR_OPENING = 1,
};

typedef s32 BossGarageDoorDirection;

CTR_STATIC_ASSERT(BOSS_GARAGE_DOOR_CLOSING == -1);
CTR_STATIC_ASSERT(BOSS_GARAGE_DOOR_STOPPED == 0);
CTR_STATIC_ASSERT(BOSS_GARAGE_DOOR_OPENING == 1);
CTR_STATIC_ASSERT(sizeof(BossGarageDoorDirection) == 0x4);

struct BossGarageDoor
{
	BossGarageDoorDirection direction;

	// so you can't spam open/close
	int cooldown;

	struct Instance *garageTopInst;

	SVec3 rot;
	s16 _pad_rot;

	// 0x14 bytes large
};

CTR_STATIC_ASSERT(sizeof(struct BossGarageDoor) == 0x14);

#if 0
struct AdvPause {

};
#endif

enum WoodDoorCamFlags
{
	WdCam_None = 0,
	WdCam_FlyingOut = 1,
	WdCam_FullyOut = 2,
	WdCam_FlyingIn = 4,
	WdCam_CutscenePlaying = 0x10
};

typedef s16 WoodDoorCamFlagSet;

CTR_STATIC_ASSERT(WdCam_None == 0);
CTR_STATIC_ASSERT(WdCam_FlyingOut == 1);
CTR_STATIC_ASSERT(WdCam_FullyOut == 2);
CTR_STATIC_ASSERT(WdCam_FlyingIn == 4);
CTR_STATIC_ASSERT(WdCam_CutscenePlaying == 0x10);
CTR_STATIC_ASSERT(sizeof(WoodDoorCamFlagSet) == 0x2);

enum AdventureHubDoorID
{
	AH_DOOR_BEACH_TO_GLACIER_PARK = 4,
	AH_DOOR_BEACH_TO_GEMSTONE_VALLEY = 5,
};

typedef s16 AdventureHubDoorID;

CTR_STATIC_ASSERT(AH_DOOR_BEACH_TO_GLACIER_PARK == 4);
CTR_STATIC_ASSERT(AH_DOOR_BEACH_TO_GEMSTONE_VALLEY == 5);
CTR_STATIC_ASSERT(sizeof(AdventureHubDoorID) == 0x2);

struct WoodDoor
{
	struct Instance *otherDoor;
	struct Instance *keyInst[AH_WOOD_DOOR_KEY_COUNT];

	// 0x14 (5)
	SVec3 doorRot;
	s16 _pad_doorRot;

	// 0x1c (7)
	WoodDoorCamFlagSet camFlags;
	s16 camTimer_unused;

	// 0x20 (8)
	int hudFlags;

	// 0x24 (9)
	s16 frameCount_unused;
	s16 frameCount_doorOpenAnim;

	// 0x28 (10)
	SVec3 keyRot;
	s16 _pad_keyRot;

	// 0x30 (12)
	s16 keyOrbit;
	s16 keyShrinkFrame;

	// 0x34
	AdventureHubDoorID doorID;
	s16 padding_0x36;

	// 0x38 bytes large
};

CTR_STATIC_ASSERT(sizeof(struct WoodDoor) == 0x38);

enum WarpPadInstanceSet
{
	// instances that appear
	// when warppad is closed
	WPIS_CLOSED_ITEM = 0,
	WPIS_CLOSED_X,
	WPIS_CLOSED_1S,
	WPIS_CLOSED_10S,

	// istances that appear
	// when warppad is open
	WPIS_OPEN_BEAM,
	WPIS_OPEN_RING1,
	WPIS_OPEN_RING2,
	WPIS_OPEN_PRIZE1,
	WPIS_OPEN_PRIZE2,
	WPIS_OPEN_PRIZE3,

	WPIS_NUM_INSTANCES
};

enum AdventureHubItemTypeID
{
	// Boss-garage and marker values are data IDs. Route values select lock behavior,
	// not a unique source/destination pair.
	AH_HUB_ITEM_RIPPER_ROO_GARAGE = 0,
	AH_HUB_ITEM_PAPU_PAPU_GARAGE,
	AH_HUB_ITEM_KOMODO_JOE_GARAGE,
	AH_HUB_ITEM_PINSTRIPE_GARAGE,
	AH_HUB_ITEM_OXIDE_WARPPAD,
	AH_HUB_ITEM_SAVE_LOAD_MARKER = 100,
	AH_HUB_ITEM_ROUTE_KEY1_IF_BEACH = -1,
	AH_HUB_ITEM_ROUTE_OPEN_A = -2,
	AH_HUB_ITEM_ROUTE_OPEN_B = -3,
	AH_HUB_ITEM_ROUTE_KEY2 = -4,
	AH_HUB_ITEM_ROUTE_KEY3 = -5,
};

typedef s16 AdventureHubItemType;

enum AdventureHubItemList
{
	AH_HUB_ITEM_LIST_END_POS_X = -1,
};

CTR_STATIC_ASSERT(AH_HUB_ITEM_RIPPER_ROO_GARAGE == 0);
CTR_STATIC_ASSERT(AH_HUB_ITEM_PAPU_PAPU_GARAGE == 1);
CTR_STATIC_ASSERT(AH_HUB_ITEM_KOMODO_JOE_GARAGE == 2);
CTR_STATIC_ASSERT(AH_HUB_ITEM_PINSTRIPE_GARAGE == 3);
CTR_STATIC_ASSERT(AH_HUB_ITEM_OXIDE_WARPPAD == 4);
CTR_STATIC_ASSERT(AH_HUB_ITEM_SAVE_LOAD_MARKER == 100);
CTR_STATIC_ASSERT(AH_HUB_ITEM_ROUTE_KEY1_IF_BEACH == -1);
CTR_STATIC_ASSERT(AH_HUB_ITEM_ROUTE_OPEN_A == -2);
CTR_STATIC_ASSERT(AH_HUB_ITEM_ROUTE_OPEN_B == -3);
CTR_STATIC_ASSERT(AH_HUB_ITEM_ROUTE_KEY2 == -4);
CTR_STATIC_ASSERT(AH_HUB_ITEM_ROUTE_KEY3 == -5);
CTR_STATIC_ASSERT(sizeof(AdventureHubItemType) == 0x2);
CTR_STATIC_ASSERT(AH_HUB_ITEM_LIST_END_POS_X == -1);

struct HubItem
{
	// 0x0
	s16 posX;
	s16 posY;

	// 0x4
	s16 angle;

	// 0x6
	AdventureHubItemType iconType;

	// 0x8 -- size
};

CTR_STATIC_ASSERT(sizeof(struct HubItem) == 0x8);
CTR_STATIC_ASSERT(OFFSETOF(struct HubItem, posX) == 0x0);
CTR_STATIC_ASSERT(OFFSETOF(struct HubItem, posY) == 0x2);
CTR_STATIC_ASSERT(OFFSETOF(struct HubItem, angle) == 0x4);
CTR_STATIC_ASSERT(OFFSETOF(struct HubItem, iconType) == 0x6);

enum AHWarpPadVisualState
{
	AH_WP_VISUAL_LOCKED = 0,
	AH_WP_VISUAL_TROPHY_OPEN,
	AH_WP_VISUAL_COMPLETE,
	AH_WP_VISUAL_RELIC_TOKEN_OPEN,
	AH_WP_VISUAL_COLOR_CYCLE_OPEN,
	AH_WP_VISUAL_COUNT,
};

CTR_STATIC_ASSERT(AH_WP_VISUAL_LOCKED == 0);
CTR_STATIC_ASSERT(AH_WP_VISUAL_TROPHY_OPEN == 1);
CTR_STATIC_ASSERT(AH_WP_VISUAL_COMPLETE == 2);
CTR_STATIC_ASSERT(AH_WP_VISUAL_RELIC_TOKEN_OPEN == 3);
CTR_STATIC_ASSERT(AH_WP_VISUAL_COLOR_CYCLE_OPEN == 4);
CTR_STATIC_ASSERT(AH_WP_VISUAL_COUNT == 5);

enum AHPausePageType
{
	AH_PAUSE_PAGE_HUB = 0,
	AH_PAUSE_PAGE_TOKEN_TOTALS,
	AH_PAUSE_PAGE_RELIC_TOTALS,
};

enum AHPauseIconIndex
{
	AH_PAUSE_ICON_NONE = -1,
	AH_PAUSE_ICON_FIRST_GEM = 0,
	AH_PAUSE_ICON_BOSS_KEY = 5,
	AH_PAUSE_ICON_SAPPHIRE_RELIC = 6,
	AH_PAUSE_ICON_GOLD_RELIC = 7,
	AH_PAUSE_ICON_PLATINUM_RELIC = 8,
	AH_PAUSE_ICON_FIRST_TOKEN = 9,
	AH_PAUSE_ICON_TROPHY = 14,
};

enum AHPauseConstants
{
	AH_PAUSE_MENU_PAGE_COUNT = 7,
	AH_PAUSE_MEMBER_COUNT = 0xe,
	AH_PAUSE_ICON_COUNT = 15,
	AH_PAUSE_GEM_ICON_COUNT = 5,
	AH_PAUSE_TOKEN_ICON_COUNT = 5,
	AH_PAUSE_PURPLE_TOKEN_INDEX = 4,
	AH_PAUSE_RELIC_ICON_COUNT = 3,
	AH_PAUSE_CTR_TOKEN_TRACK_COUNT = 0x10,
	AH_PAUSE_RELIC_TRACK_COUNT = 0x12,
	AH_PAUSE_PURPLE_TOKEN_COUNT = 4,
};

struct AHPauseMember
{
	// 0x0
	s16 iconIndex;
	s16 unlockFlags;

	// 0x4
	SVec3 rot;
	s16 _pad_rot;

	// 0xC
	struct Instance *inst;

	// 0x10 -- size
};

struct PauseObject
{
	// 0x0
	struct AHPauseMember members[AH_PAUSE_MEMBER_COUNT];

	// 0xE0
	struct Thread *t;

	// 0xe4 -- size
};

CTR_STATIC_ASSERT(sizeof(struct AHPauseMember) == 0x10);
CTR_STATIC_ASSERT(offsetof(struct AHPauseMember, iconIndex) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct AHPauseMember, unlockFlags) == 0x2);
CTR_STATIC_ASSERT(offsetof(struct AHPauseMember, rot) == 0x4);
CTR_STATIC_ASSERT(offsetof(struct AHPauseMember, inst) == 0xc);
CTR_STATIC_ASSERT(sizeof(struct PauseObject) == 0xe4);
CTR_STATIC_ASSERT(offsetof(struct PauseObject, members) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct PauseObject, t) == 0xe0);

struct AHPausePage
{
	// can be -1 if not hub page
	s16 hubID;

	// can be -1 for hubs, which then get name from MetaDataLev
	s16 titleLng;

	// enum AHPausePageType, stored as s16 to match retail layout
	s16 type;

	s16 characterID_Boss;
};

struct AHPauseInstance
{
	// 0x0
	s16 modelID;
	s16 scale;

	// 0x4
	u32 color;

	// 0x8
	u32 instFlags;

	// 0xC
	SVec3 lightDir;
	s16 _pad_lightDir;
};

CTR_STATIC_ASSERT(sizeof(struct AHPausePage) == 0x8);
CTR_STATIC_ASSERT(offsetof(struct AHPausePage, hubID) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct AHPausePage, titleLng) == 0x2);
CTR_STATIC_ASSERT(offsetof(struct AHPausePage, type) == 0x4);
CTR_STATIC_ASSERT(offsetof(struct AHPausePage, characterID_Boss) == 0x6);
CTR_STATIC_ASSERT(sizeof(struct AHPauseInstance) == 0x14);
CTR_STATIC_ASSERT(offsetof(struct AHPauseInstance, modelID) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct AHPauseInstance, scale) == 0x2);
CTR_STATIC_ASSERT(offsetof(struct AHPauseInstance, color) == 0x4);
CTR_STATIC_ASSERT(offsetof(struct AHPauseInstance, instFlags) == 0x8);
CTR_STATIC_ASSERT(offsetof(struct AHPauseInstance, lightDir) == 0xc);

struct WarpPad
{
	// 0x0
	struct Instance *inst[WPIS_NUM_INSTANCES];

	// 0x28
	SVec3 spinRot_Prize;
	s16 _pad_spinRot_Prize;

	// 0x30
	SVec3 spinRot_Wisp[2];
	s16 _pad_spinRot_Wisp[2];

	// 0x40
	SVec3 spinRot_Beam;
	s16 _pad_spinRot_Beam;

	// 0x48
	SVec3 spinRot_Rewards;
	s16 _pad_spinRot_Rewards;

	// 0x50
	SVec3 lightDirGem;
	s16 _pad_lightDirGem;

	// 0x58
	SVec3 lightDirRelic;
	s16 _pad_lightDirRelic;

	// 0x60
	SVec3 lightDirToken;
	s16 _pad_lightDirToken;

	// 0x68
	s16 digit10s;

	// 0x6a
	s16 digit1s;

	// 0x6c (1b*4)
	s16 levelID;

	// 0x6e
	// 0/3    1/3     2/3
	// 0x0    0x555   0xAAA
	u16 thirds[3];

	// 0x74
	s16 boolEnteredWarppad;

	// 0x76
	s16 framesWarping;

	// 0x78 -- size
};

enum AHSaveObjFlags
{
	AH_SAVEOBJ_FLAG_NONE = 0,
	AH_SAVEOBJ_FLAG_INTERACTION_ACTIVE = 1,
	AH_SAVEOBJ_FLAG_MENU_SHOWN = 2,
	AH_SAVEOBJ_FLAG_HUD_RESTORED = 4,
};

typedef u16 AHSaveObjFlagSet;

enum AHSaveObjConstants
{
	AH_SAVEOBJ_SCANLINE_START_FRAME = 0xf,
	AH_SAVEOBJ_INTERACTION_DIST_SQ = 0x8ffff,
	AH_SAVEOBJ_ENTRY_SPEED_MAX = 0x80,
	AH_SAVEOBJ_EXIT_SPEED_MAX = 0x101,
	AH_SAVEOBJ_CAMERA_FORWARD_OFFSET = 0x19,
	AH_SAVEOBJ_SCAN_SFX_ID = 0x99,
	AH_SAVEOBJ_SCAN_SFX_NEAR_DIST = 300,
	AH_SAVEOBJ_SCAN_SFX_FAR_DIST = 6000,
	AH_SAVEOBJ_SCAN_DEPTH_BIAS = 0xf8,
};

struct SaveObj
{
	// 0x0
	struct Instance *inst;
	// 0x4
	AHSaveObjFlagSet flags;
	// 0x6
	s16 scanlineFrame;
	// 0x8
	// Retail writes a word here and restores the low byte.
	u32 hudFlagBackup;

	// 0xc bytes large
};

CTR_STATIC_ASSERT(AH_SAVEOBJ_FLAG_NONE == 0);
CTR_STATIC_ASSERT(AH_SAVEOBJ_FLAG_INTERACTION_ACTIVE == 1);
CTR_STATIC_ASSERT(AH_SAVEOBJ_FLAG_MENU_SHOWN == 2);
CTR_STATIC_ASSERT(AH_SAVEOBJ_FLAG_HUD_RESTORED == 4);
CTR_STATIC_ASSERT(AH_SAVEOBJ_SCANLINE_START_FRAME == 0xf);
CTR_STATIC_ASSERT(AH_SAVEOBJ_INTERACTION_DIST_SQ == 0x8ffff);
CTR_STATIC_ASSERT(AH_SAVEOBJ_ENTRY_SPEED_MAX == 0x80);
CTR_STATIC_ASSERT(AH_SAVEOBJ_EXIT_SPEED_MAX == 0x101);
CTR_STATIC_ASSERT(AH_SAVEOBJ_CAMERA_FORWARD_OFFSET == 0x19);
CTR_STATIC_ASSERT(AH_SAVEOBJ_SCAN_SFX_ID == 0x99);
CTR_STATIC_ASSERT(AH_SAVEOBJ_SCAN_SFX_NEAR_DIST == 300);
CTR_STATIC_ASSERT(AH_SAVEOBJ_SCAN_SFX_FAR_DIST == 6000);
CTR_STATIC_ASSERT(AH_SAVEOBJ_SCAN_DEPTH_BIAS == 0xf8);
CTR_STATIC_ASSERT(sizeof(AHSaveObjFlagSet) == 0x2);
CTR_STATIC_ASSERT(sizeof(struct SaveObj) == 0xc);

enum AHMaskHintOffsetSlot
{
	AH_MASKHINT_OFFSET_DEFAULT = 0,
	AH_MASKHINT_OFFSET_WARPPAD_INTERRUPT = 1,
	AH_MASKHINT_OFFSET_COUNT = 2,
};

struct AHMaskHintOffsets
{
	// 0x0
	SVec3 pos[AH_MASKHINT_OFFSET_COUNT];

	// 0xc
	SVec3 rot[AH_MASKHINT_OFFSET_COUNT];
};

CTR_STATIC_ASSERT(AH_MASKHINT_OFFSET_DEFAULT == 0);
CTR_STATIC_ASSERT(AH_MASKHINT_OFFSET_WARPPAD_INTERRUPT == 1);
CTR_STATIC_ASSERT(AH_MASKHINT_OFFSET_COUNT == 2);
CTR_STATIC_ASSERT(sizeof(struct AHMaskHintOffsets) == 0x18);
CTR_STATIC_ASSERT(offsetof(struct AHMaskHintOffsets, pos) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct AHMaskHintOffsets, rot) == 0xc);

struct OverlayRDATA_232
{
	// 0x800aba3c
	s16 battleTrackPurpleTokenOffset[8];

	// 0x800aba4c
	s16 bossRaceLevelIDs[6];

	// 0x800aba58
	s16 bossCharacterIDs[6];

	// 0x800aba64
	char s_garage[8];
	char s_garagetop[0xc];
	char s_saveobj[8];
	char s_scan[8];
	char s_key[4];

	// 0x800aba8c
	s16 doorKeyShrinkScale[0xc];

	// 0x800abaa4
	char s_door[8];

	// 0x800abaac
	u32 warppadColorJumpTable[AH_WP_VISUAL_COUNT];

	// 0x800abac0
	u32 unk_800abac0;

	// 0x800abac4
	char s_format[8];

	// 0x800abacc
	char s_PAUSE[8];

	// 0x800abad4
	char s_pause[8];
};

struct OverlayDATA_232
{
	// 800b4ddc (5 light directions plus padding)
	SVec3 lightDirGem[5];
	s16 _pad_lightDirGemTable;

	// 800b4dfc (5 light directions plus padding)
	SVec3 lightDirRelic[5];
	s16 _pad_lightDirRelicTable;

	// 800b4e1c (5 light directions plus padding)
	SVec3 lightDirToken[5];
	s16 _pad_lightDirTokenTable;

	// 800b4e3c
	struct MenuRow rowsTokenRelic[3];

	// 800b4e50
	struct RectMenu menuTokenRelic;

	// 800b4e7c
	s16 keysNeededByHub[5];

	// 800b4e86
	s16 levelID;

	// 800b4e88
	int battleCrystalEventTime[7];

	// 800b4ea4
	SVec3 saveObjCameraOffset;
	s16 _pad_saveObjCameraOffset;

	// 800b4eac
	SVec2 loadSavePrimOffset[5];

	// 800b4ec0
	SVec2 hubArrowPrimOffset[5];

	// 800b4ed4
	// 2 arrows, boss, save/load, null(0xFFFF)
	struct HubItem hubItems_hub1[5],
	    // 800b4efc
	    hubItems_hub2[5],
	    // 800b4f24
	    hubItems_hub3[5],
	    // 800b4f4c (3 arrows)
	    hubItems_hub4[6],
	    // 800b4f7c (1 arrow)
	    hubItems_hub5[4];

	// 800b4f9c -- array of pointers:
	//		800b4ed4 800b4efc 800b4f24
	//		800b4f4c 800b4f7c
	struct HubItem *hubItemsXY_ptrArray[5];

	// 800b4fb0
	SVec2 hubArrowInnerOffset[3];

	// 800b4fbc
	SVec2 hubArrowOuterOffset[4];

	// 800b4fcc
	SVec2 loadSavePos[4];

	// 800b4fdc
	u32 loadSave_col[4];

	// 800b4fec
	SVec2 hubArrowPos[3];

	// 800B4FF8
	u32 hubArrow_col1[3];

	// 800b5004
	u32 hubArrow_col2[3];

	// 800b5010
	u32 hubArrowGray1[3];
	u32 hubArrowGray2[3];

	// 800b5028
	struct AHPausePage advPausePages[AH_PAUSE_MENU_PAGE_COUNT];

	// 0x800B5060
	// 0,1,2,3,4: Gems
	// 5: Key (for all pages)
	// 6,7,8: 3-Relic page
	// 9,10,11,12,13: Tokens
	// 14: Trophy
	struct AHPauseInstance advPauseInst[AH_PAUSE_ICON_COUNT];

	// 0x800B518C
	struct RectMenu menuHintMenu;

	// 0x800B51B8
	SVec2 fiveArrowPos[3];

	// 0x800b51c4
	u32 fiveArrow_col1[3];

	// 0x800b51d0
	u32 fiveArrow_col2[3];

	// 0x800b51dc
	SVec3 maskPos;
	s16 _pad_maskPos;

	// 0x800b51e4
	SVec3 maskRot;
	s16 _pad_maskRot;

	// 0x800b51ec
	s16 maskScale;

	// 0x800b51ee
	s16 maskCooldown;

	// 0x800b51f0
	SVec3 maskOffsetPos;
	s16 _pad_maskOffsetPos;

	// 0x800b51f8
	SVec3 maskOffsetRot;
	s16 _pad_maskOffsetRot;

	// 0x800b5200
	struct AHMaskHintOffsets maskHintOffsets;

	// 0x800b5218
	int maskFrameCurr;

	// 0x800b521c
	struct ParticleEmitter emSet_maskSpawn[0xA];

	// 0x800b5384
	struct ParticleEmitter emSet_maskLeave[0xA];

	// 0x800b54ec
	s16 maskAudioTargetVolume[4];

	// 800b54f4
	// 20 hints, last two entries are null
	s16 hintMenuLngIndex[22];

	// 800b5520
	SVec3 eyePos;
	s16 _pad_eyePos;

	// 800b5528
	SVec3 lookAtPos;
	s16 _pad_lookAtPos;

	// 800b5530
	u32 colorQuad[4];

	// 800b5540
	u32 colorTri[3];

	// 800b554c
	s16 pausePageDir;

	// 800b554e
	s16 pausePageTimer;

	// 800b5550
	s16 pausePagePrev;

	// 800b5552
	s16 pausePageCurr;

	// 800b5554
	s16 pausePageDir_dup;
	s16 padding3;

	// 800b5558
	s16 maskHintID;
	s16 padding_maskHintID;

	// 800b555c
	int maskAngle;

	// 800b5560
	SVec3 maskCamPosStart;
	s16 _pad_maskCamPosStart;

	// 800b5568
	SVec3 maskCamRotStart;
	s16 _pad_maskCamRotStart;

	// 800b5570
	s16 maskWarppadDelayFrames;
	s16 padding_maskWarppadDelayFrames;

	// 800b5574
	s16 maskWarppadBoolInterrupt;
	s16 padding_maskWarppadBoolInterrupt;

	// 800b5578
	struct PauseObject *ptrPauseObject;

	struct PauseObject pauseObject; // 800b557c

	// 800B5660
	int hintMenu_boolViewHint;

	// 800B5664
	int hintMenu_scrollIndex;

	// 800B5668
	char audioBackup[4];

	// 800B566c
	s16 maskSpawnFrame;
	s16 padding4;

	// 800b5670
	// Suppresses lower-priority hub-route arrows when an open boss/warppad arrow is drawn.
	s16 mapPriorityArrowDrawn;
	s16 padding_800b5672;
};

#define OFFSETOF_D232(ELEMENT) ((u32)0x800b4ddc + OFFSETOF(struct OverlayDATA_232, ELEMENT))

CTR_STATIC_ASSERT(OFFSETOF_D232(saveObjCameraOffset) == 0x800b4ea4);
CTR_STATIC_ASSERT(OFFSETOF_D232(loadSavePrimOffset) == 0x800b4eac);
CTR_STATIC_ASSERT(OFFSETOF_D232(hubArrowPrimOffset) == 0x800b4ec0);
CTR_STATIC_ASSERT(OFFSETOF_D232(hubArrowInnerOffset) == 0x800b4fb0);
CTR_STATIC_ASSERT(OFFSETOF_D232(hubArrowOuterOffset) == 0x800b4fbc);
CTR_STATIC_ASSERT(OFFSETOF_D232(loadSavePos) == 0x800b4fcc);
CTR_STATIC_ASSERT(OFFSETOF_D232(hubArrowPos) == 0x800b4fec);
CTR_STATIC_ASSERT(OFFSETOF_D232(fiveArrowPos) == 0x800b51b8);
CTR_STATIC_ASSERT(OFFSETOF_D232(maskHintOffsets) == 0x800b5200);
CTR_STATIC_ASSERT(OFFSETOF_D232(maskWarppadDelayFrames) == 0x800b5570);
CTR_STATIC_ASSERT(OFFSETOF_D232(maskWarppadBoolInterrupt) == 0x800b5574);
CTR_STATIC_ASSERT(OFFSETOF_D232(ptrPauseObject) == 0x800b5578);
CTR_STATIC_ASSERT(OFFSETOF_D232(pauseObject) == 0x800b557c);

extern struct OverlayRDATA_232 R232;
extern struct OverlayDATA_232 D232;

#endif
