#ifndef CTR_NATIVE_NAMESPACE_DECAL_H
#define CTR_NATIVE_NAMESPACE_DECAL_H

// transparent parameter of DrawPoly funcs
enum BlendModeDecal
{
	// 50% transparency
	TRANS_50_DECAL = 1,

	// additive blending
	ADD_DECAL = 2,

	// subtractive blending
	SUBTRACT_DECAL = 3,

	// additive blending on 25% transparency
	ADD_25_DECAL = 4
};

enum Font
{
	// erased in final builds
	FONT_DEBUG = 0,

	// most gameplay
	FONT_BIG = 1,

	// good for debugging
	FONT_SMALL = 2,

	// same as "BIG" but less spacing
	FONT_CREDITS = 3,

	// total amount of enum entries
	FONT_NUM = 4,
};

// DecalFont_DrawLineStrlen receives one packed style word: low bits select the
// ptrColors slot, high bits select alignment/style.
enum DecalFontStyle
{
	// normal UI text colors
	ORANGE,
	PERIWINKLE,
	ORANGE_DARKENED,
	RED,

	// used for blinking
	WHITE,

	// driver colors
	// these are flat colors instead of gradients
	// used for minimap marker and time trial records
	CRASH_BLUE,
	CORTEX_RED,
	TINY_GREEN,
	COCO_MAGENTA,
	N_GIN_PURPLE,
	DINGODILE_OLIVE,
	POLAR_CYAN,
	PURA_VIOLET,
	PINSTRIPE_PALE_DARK_BLUE, // different in sep3
	PAPU_YELLOW,
	ROO_ORANGE,
	JOE_COLOR,
	TROPY_LIGHT_BLUE,
	PENTA_WHITE,
	FAKE_CRASH_GRAY,
	OXIDE_LIGHT_GREEN,

	// pitch black, used for text shadows
	BLACK,

	// the only color in ptrColors that isn't a flat color or a top-to-bottom gradient
	// used for silver stars in time trial menu and platinum relic text in relic races
	SILVER,

	// gray #808080, identical to fake crash's
	// the neutral vertex color for the PSX
	// used for sprites that don't need coloring, as a result
	GRAY,

	// colors assigned to each player in multiplayer modes
	PLAYER_BLUE,
	PLAYER_RED,
	PLAYER_GREEN,
	PLAYER_YELLOW,

	DARK_RED,
	LIGHT_GREEN,
	FOREST_GREEN,
	CREDITS_FADE, // written dynamically while credits are running, makes text fade to black bg

// Only in USA, EUR, JPN, not Sep3
#if BUILD >= UsaRetail
	BLUE,
	LIME_GREEN,
	ORANGE_RED,
#endif

	// total amount of enum entries
	NUM_COLORS,

	// TODO(alhendi): nicer way of doing this or...?
	// end text at X position
	JUSTIFY_RIGHT = 0x4000,

	// center text
	JUSTIFY_CENTER = 0x8000
};

typedef u16 DecalFontStyle;

CTR_STATIC_ASSERT(sizeof(DecalFontStyle) == 0x2);
CTR_STATIC_ASSERT(ORANGE == 0);
CTR_STATIC_ASSERT(NUM_COLORS <= 0xfff);
CTR_STATIC_ASSERT(JUSTIFY_RIGHT == 0x4000);
CTR_STATIC_ASSERT(JUSTIFY_CENTER == 0x8000);

struct TextureLayout
{
	// 0x14
	// top left corner
	u8 u0;
	u8 v0;

	// 0x16
	// holds X and Y with bit shifting
	u16 clut;

	// 0x18
	// top right corner
	u8 u1;
	u8 v1;

	// 0x1a
	// holds X and Y with bit shifting
	u16 tpage;

	// 0x1c
	// bottom left corner
	u8 u2;
	u8 v2;

	// 0x1e
	// bottom right corner
	u8 u3;
	u8 v3;
};

struct Icon
{
	// https://github.com/DCxDemo/CTR-tools/blob/master/ctr-tools/CTRFramework/CTRFramework.Code/lev/TextureLayout.cs

	// 0x0
	char name[0x10];

	// 0x10
	int global_IconArray_Index;

	struct TextureLayout texLayout;
};

struct IconGroup
{
	// 0x0
	char name[16];

	// 0x10
	s16 groupID;

	// 0x12
	s16 numIcons;

	// 0x14
	// struct Icon* icons[0];
};
#define ICONGROUP_GETICONS(x) (struct Icon **)((u32)x + sizeof(struct IconGroup))

CTR_STATIC_ASSERT(sizeof(struct TextureLayout) == 0xC);
CTR_STATIC_ASSERT(sizeof(struct Icon) == 0x20);
CTR_STATIC_ASSERT(sizeof(((struct Icon *)0)->name) == 0x10);
CTR_STATIC_ASSERT(sizeof(struct IconGroup) == 0x14);
CTR_STATIC_ASSERT(OFFSETOF(struct IconGroup, name) == 0x0);
CTR_STATIC_ASSERT(OFFSETOF(struct IconGroup, groupID) == 0x10);
CTR_STATIC_ASSERT(OFFSETOF(struct IconGroup, numIcons) == 0x12);

#define setIconUV(p, icon)                                                                                                                              \
	CtrGpu_WritePackedUVWord(&(p)->u0, CTR_ReadU32LE(&(icon)->texLayout.u0)), CtrGpu_WritePackedUVWord(&(p)->u1, CTR_ReadU32LE(&(icon)->texLayout.u1)), \
	    CtrGpu_WritePackedUV(&(p)->u2, CTR_ReadU16LE(&(icon)->texLayout.u2)), CtrGpu_WritePackedUV(&(p)->u3, CTR_ReadU16LE(&(icon)->texLayout.u3))

#endif
