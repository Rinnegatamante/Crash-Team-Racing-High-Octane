#include <common.h>

#if defined(CTR_NATIVE)
#include <platform/native_custom_racer.h>
#endif

enum
{
	UI_ICON_FIXED_SHIFT = 0xc,
	UI_ICON_QUAD_COUNT = 4,
	UI_ICON_TPAGE_BLEND_MASK = 0x60,
	UI_ICON_TPAGE_BLEND_STEP = 0x20,
	UI_ICON_SEMI_TRANS_CODE_BIT = 2,
	UI_WEAPON_SHINE_RESULT_SHIFT = 0xd,
	UI_WEAPON_SHINE_BRIGHT_YELLOW_BASE = 0x7f,
	UI_WEAPON_SHINE_ORANGE_GREEN_BASE = 0x32,
	UI_WEAPON_SHINE_DARK_RED_BASE = 0x21,
	UI_WEAPON_SHINE_DARK_GREEN_BASE = 0x10,
	UI_WEAPON_SHINE_GRAY_BASE = 0x5f,
	UI_WEAPON_SHINE_RESULT_SCALE = 0xff,
	UI_WEAPON_SHINE_RESULT_BASE = 0x80,
	UI_WEAPON_SHINE_COLOR_BRIGHT_ROW = 0,
	UI_WEAPON_SHINE_COLOR_MID_ROW = 1,
	UI_WEAPON_SHINE_COLOR_DARK_ROW = 2,
	UI_WEAPON_SHINE_SECOND_COLOR_TRANSPARENCY = 3,
	UI_TRACKER_BG_SHINE_THETA_STEP = 0x100,
	UI_DRIVER_ICON_NTSC_CLIP_LIMIT = 166,
	UI_DRIVER_ICON_NTSC_CLIP_MAX = 165,
	UI_DRIVER_ICON_EUR_CLIP_LIMIT = 176,
	UI_DRIVER_ICON_EUR_CLIP_MAX = 175,
	UI_DRIVER_ICON_FT4_CODE = 0x2c,
};

#if CTR_NATIVE_WIDESCREEN
static s16 UI_IconWidescreenX(int x, int centerX)
{
	return (s16)(centerX + CTR_WIDESCREEN_SCALE_X(x - centerX));
}

static int UI_IconQuadCenterX(int x0, int x1, int x2, int x3)
{
	int minX = x0;
	int maxX = x0;
	const int x[3] = {x1, x2, x3};
	for (int i = 0; i < 3; i++)
	{
		if (x[i] < minX)
			minX = x[i];
		if (x[i] > maxX)
			maxX = x[i];
	}
	return (minX + maxX) / 2;
}
#endif

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8004e0e0-0x8004e37c.
void UI_WeaponBG_AnimateShine(void)
{
	int sine;
	u32 brightYellow;
	u32 orangeGreen;
	u32 gray;

	sine = MATH_Sin((int)sdata->wumpaShineTheta);
	sine = (sine < 0) ? -sine : sine;

	brightYellow = ((sine * UI_WEAPON_SHINE_BRIGHT_YELLOW_BASE) >> UI_ICON_FIXED_SHIFT) + UI_WEAPON_SHINE_BRIGHT_YELLOW_BASE;
	sdata->wumpaShineColor1[0][0] = brightYellow;
	sdata->wumpaShineColor1[0][1] = brightYellow;
	CTR_WriteU16LE(&sdata->wumpaShineColor1[0][2], 0);

	orangeGreen = ((sine * UI_WEAPON_SHINE_ORANGE_GREEN_BASE) >> UI_ICON_FIXED_SHIFT) + UI_WEAPON_SHINE_ORANGE_GREEN_BASE;
	sdata->wumpaShineColor1[1][0] = brightYellow;
	sdata->wumpaShineColor1[1][1] = orangeGreen;
	CTR_WriteU16LE(&sdata->wumpaShineColor1[1][2], 0);

	sdata->wumpaShineColor1[2][0] = ((sine * UI_WEAPON_SHINE_DARK_RED_BASE) >> UI_ICON_FIXED_SHIFT) + UI_WEAPON_SHINE_DARK_RED_BASE;
	sdata->wumpaShineColor1[2][1] = ((sine * UI_WEAPON_SHINE_DARK_GREEN_BASE) >> UI_ICON_FIXED_SHIFT) + UI_WEAPON_SHINE_DARK_GREEN_BASE;
	CTR_WriteU16LE(&sdata->wumpaShineColor1[2][2], 0);

	gray = ((sine * UI_WEAPON_SHINE_GRAY_BASE) >> UI_ICON_FIXED_SHIFT) + UI_WEAPON_SHINE_GRAY_BASE;
	sdata->wumpaShineColor2[0][0] = gray;
	sdata->wumpaShineColor2[0][1] = gray;
	CTR_WriteU16LE(&sdata->wumpaShineColor2[0][2], (u16)gray);

	gray = CTR_ReadU32LE(&sdata->wumpaShineColor2[0][0]);
	CTR_WriteU32LE(&sdata->wumpaShineColor2[1][0], gray);
	CTR_WriteU32LE(&sdata->wumpaShineColor2[2][0], gray);

	sdata->wumpaShineResult = (sine * UI_WEAPON_SHINE_RESULT_SCALE >> UI_WEAPON_SHINE_RESULT_SHIFT) + UI_WEAPON_SHINE_RESULT_BASE;
	return;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8004e37c-0x8004e660.
void UI_WeaponBG_DrawShine(struct Icon *icon, s16 posX, s16 posY, struct PrimMem *primMem, uint32_t *ot, char transparency, s16 angleX, s16 angleY,
                           int unusedColor)
{
	s16 rightX;
	s16 bottomY;
	s16 widthOffset;
	s16 heightOffset;
	s16 altX;
	s16 altY;
	POLY_GT4 *p;
	int quadIndex;
	s16 topY;
	s16 leftX;

	(void)unusedColor;

	u8 *shineColors = &sdata->wumpaShineColor1[0][0];

	if (transparency == UI_WEAPON_SHINE_SECOND_COLOR_TRANSPARENCY)
	{
		shineColors = &sdata->wumpaShineColor2[0][0];
	}

	widthOffset = (s16)(((icon->texLayout.u1 - icon->texLayout.u0) * (int)angleX) >> UI_ICON_FIXED_SHIFT);
	rightX = posX + widthOffset;
	angleX = angleX >> UI_ICON_FIXED_SHIFT;
	leftX = rightX - angleX;

	heightOffset = (s16)(((icon->texLayout.v2 - icon->texLayout.v0) * (int)angleY) >> UI_ICON_FIXED_SHIFT);
	bottomY = posY + heightOffset;
	angleY = angleY >> UI_ICON_FIXED_SHIFT;
	topY = bottomY - angleY;

#if CTR_NATIVE_WIDESCREEN
	const int shineAltX = (posX + widthOffset * 2) - angleX;
	const int shineCenterX = UI_IconQuadCenterX(posX, rightX, shineAltX, leftX);
#endif

	for (quadIndex = 0; quadIndex < UI_ICON_QUAD_COUNT; quadIndex++)
	{
		p = primMem->cursor;
		CtrGpu_WritePackedUVWord(&p->u0, CTR_ReadU32LE(&icon->texLayout.u0));
		CtrGpu_WritePackedUVWord(&p->u1, CTR_ReadU32LE(&icon->texLayout.u1));
		CtrGpu_WritePackedUVWord(&p->u2, CTR_ReadU32LE(&icon->texLayout.u2));
		CtrGpu_WritePackedUV(&p->u3, CTR_ReadU16LE(&icon->texLayout.u3));

		switch (quadIndex)
		{
		case 0:
			p->x0 = posX;
			p->y0 = posY;

			p->x1 = rightX;
			p->y1 = posY;
			p->x2 = posX;
			p->y2 = bottomY;
			p->x3 = rightX;
			p->y3 = bottomY;
			break;

		case 1:
			altX = (posX + widthOffset * 2) - angleX;
			p->x0 = altX;
			p->y0 = posY;

			p->x1 = leftX;
			p->y1 = posY;
			p->x2 = altX;
			p->y2 = bottomY;
			p->x3 = leftX;
			p->y3 = bottomY;

			break;

		case 2:
			altY = (posY + heightOffset * 2) - angleY;
			p->x0 = posX;
			p->y0 = altY;

			p->x1 = rightX;
			p->y1 = altY;
			p->x2 = posX;
			p->y2 = topY;
			p->x3 = rightX;
			p->y3 = topY;

			break;

		case 3:
			altX = (posX + widthOffset * 2) - angleX;
			altY = (posY + heightOffset * 2) - angleY;
			p->x0 = altX;
			p->y0 = altY;

			p->x1 = leftX;
			p->y1 = altY;
			p->x2 = altX;
			p->y2 = topY;
			p->x3 = leftX;
			p->y3 = topY;

			break;
		}

#if CTR_NATIVE_WIDESCREEN
		p->x0 = UI_IconWidescreenX(p->x0, shineCenterX);
		p->x1 = UI_IconWidescreenX(p->x1, shineCenterX);
		p->x2 = UI_IconWidescreenX(p->x2, shineCenterX);
		p->x3 = UI_IconWidescreenX(p->x3, shineCenterX);
#endif

		CtrGpu_WriteColorCode(&p->r0, CTR_ReadU32LE(&shineColors[UI_WEAPON_SHINE_COLOR_DARK_ROW * sizeof(u32)]));
		CtrGpu_WriteColorCode(&p->r1, CTR_ReadU32LE(&shineColors[UI_WEAPON_SHINE_COLOR_MID_ROW * sizeof(u32)]));
		CtrGpu_WriteColorCode(&p->r2, CTR_ReadU32LE(&shineColors[UI_WEAPON_SHINE_COLOR_MID_ROW * sizeof(u32)]));
		CtrGpu_WriteColorCode(&p->r3, CTR_ReadU32LE(&shineColors[UI_WEAPON_SHINE_COLOR_BRIGHT_ROW * sizeof(u32)]));

		setPolyGT4(p);

		if (transparency != 0)
		{
			p->tpage = (p->tpage & ~UI_ICON_TPAGE_BLEND_MASK) | (((u16)transparency - 1) * UI_ICON_TPAGE_BLEND_STEP);
			p->code |= UI_ICON_SEMI_TRANS_CODE_BIT;
		}

		AddPrim(ot, p);

		primMem->cursor = p + 1;
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8004e660-0x8004e8d8.
void UI_TrackerBG(struct Icon *targetIcon, s16 centerX, s16 centerY, struct PrimMem *primMem, uint32_t *ot, char transparency, s16 angleX, s16 angleY,
                  int color)
{
	s16 rightX;
	s16 bottomY;
	s16 heightOffset;
	POLY_FT4 *p;
	int widthOffset;
	int quadIndex;
	s16 topY;
	s16 leftX;

	sdata->wumpaShineTheta += FPS_HALF(UI_TRACKER_BG_SHINE_THETA_STEP);

	widthOffset = ((targetIcon->texLayout.u1 - targetIcon->texLayout.u0) * angleX) >> UI_ICON_FIXED_SHIFT;
	heightOffset = ((targetIcon->texLayout.v2 - targetIcon->texLayout.v0) * angleY) >> UI_ICON_FIXED_SHIFT;

	rightX = centerX + widthOffset;
	angleX >>= UI_ICON_FIXED_SHIFT;
	leftX = rightX - angleX;

	bottomY = centerY + heightOffset;
	angleY >>= UI_ICON_FIXED_SHIFT;
	topY = bottomY - angleY;

	int altX = (centerX + (widthOffset * 2)) - angleX;
	int altY = (centerY + (heightOffset * 2)) - angleY;

#if CTR_NATIVE_WIDESCREEN
	const int trackerCenterX = UI_IconQuadCenterX(centerX, rightX, altX, leftX);
#endif

	for (quadIndex = 0; quadIndex < UI_ICON_QUAD_COUNT; quadIndex++)
	{
		p = primMem->cursor;
		primMem->cursor = (p + 1);

		CtrGpu_WriteColorCode(&p->r0, (u32)color);
		CtrGpu_WritePackedUVWord(&p->u0, CTR_ReadU32LE(&targetIcon->texLayout.u0));
		CtrGpu_WritePackedUVWord(&p->u1, CTR_ReadU32LE(&targetIcon->texLayout.u1));
		CtrGpu_WritePackedUV(&p->u2, CTR_ReadU16LE(&targetIcon->texLayout.u2));
		CtrGpu_WritePackedUV(&p->u3, CTR_ReadU16LE(&targetIcon->texLayout.u3));

		setPolyFT4(p);

		if (transparency != 0)
		{
			p->tpage = (p->tpage & ~UI_ICON_TPAGE_BLEND_MASK) | (((u16)transparency - 1) * UI_ICON_TPAGE_BLEND_STEP);
			p->code |= UI_ICON_SEMI_TRANS_CODE_BIT;
		}

		const int widescreenOffset = 0;

		p->x0 = centerX + widescreenOffset;
		p->x1 = rightX;
		p->y0 = centerY;
		p->y2 = bottomY;

		switch (quadIndex)
		{
		case 1:
			p->x0 = altX - widescreenOffset;
			p->x1 = leftX;
			break;

		case 2:
			p->y0 = altY;
			p->y2 = topY;
			break;

		case 3:
			p->x0 = altX - widescreenOffset;
			p->y0 = altY;

			p->x1 = leftX;
			p->y2 = topY;
			break;
		}

		p->x2 = p->x0;
		p->y1 = p->y0;

		p->x3 = p->x1;
		p->y3 = p->y2;

#if CTR_NATIVE_WIDESCREEN
		p->x0 = UI_IconWidescreenX(p->x0, trackerCenterX);
		p->x1 = UI_IconWidescreenX(p->x1, trackerCenterX);
		p->x2 = UI_IconWidescreenX(p->x2, trackerCenterX);
		p->x3 = UI_IconWidescreenX(p->x3, trackerCenterX);
#endif

		AddPrim(ot, p);
	}
	return;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8004e8d8-0x8004eaa8.
void UI_DrawDriverIcon(struct Icon *icon, s16 posX, s16 posY, struct PrimMem *primMem, uint32_t *ot, char transparency, s16 scale, u32 color)
{
	PolyFT4 *p = primMem->cursor;
	const PrimCode primCode = {.poly = {.renderCode = RenderCode_Polygon, .quad = 1, .textured = 1}};
	p->colorCode.self = color;
	p->colorCode.code = primCode;

	int width = icon->texLayout.u1 - icon->texLayout.u0;
	int height = icon->texLayout.v2 - icon->texLayout.v0;
	int scaledWidth = FP_Mult(width, scale);
	int scaledHeight = FP_Mult(height, scale);
	int topX = posX;
	int bottomX = topX + scaledWidth;
#if CTR_NATIVE_WIDESCREEN
	const int centerX = (topX + bottomX) / 2;
	topX = UI_IconWidescreenX(topX, centerX);
	bottomX = UI_IconWidescreenX(bottomX, centerX);
#endif
#if BUILD != EurRetail
	int topY = (posY < UI_DRIVER_ICON_NTSC_CLIP_LIMIT) ? posY : UI_DRIVER_ICON_NTSC_CLIP_MAX;
	int bottomY = ((posY + scaledHeight) < UI_DRIVER_ICON_NTSC_CLIP_LIMIT) ? (posY + scaledHeight) : UI_DRIVER_ICON_NTSC_CLIP_MAX;
#else
	int topY = (posY < UI_DRIVER_ICON_EUR_CLIP_LIMIT) ? posY : UI_DRIVER_ICON_EUR_CLIP_MAX;
	int bottomY = ((posY + scaledHeight) < UI_DRIVER_ICON_EUR_CLIP_LIMIT) ? (posY + scaledHeight) : UI_DRIVER_ICON_EUR_CLIP_MAX;
#endif

	p->tag.size = (sizeof(*p) - sizeof(p->tag)) / sizeof(u32);
	p->colorCode.code.code = UI_DRIVER_ICON_FT4_CODE;

	p->v[0].pos.x = topX;
	p->v[0].pos.y = topY;
	p->v[1].pos.x = bottomX;
	p->v[1].pos.y = topY;
	p->v[2].pos.x = topX;
	p->v[2].pos.y = bottomY;
	p->v[3].pos.x = bottomX;
	p->v[3].pos.y = bottomY;

	p->polyClut.self = icon->texLayout.clut;
	p->polyTpage.self = icon->texLayout.tpage;
	p->v[2].clut.self = (icon->texLayout.v3 << 8) | icon->texLayout.u3;

	if (transparency)
	{
		p->polyTpage.semiTransparency = transparency - 1;
		p->colorCode.code.poly.semiTransparency = 1;
	}

	u8 bottomV = (u8)((icon->texLayout.v0 + bottomY) - posY);
	p->v[0].texCoords.u = icon->texLayout.u0;
	p->v[0].texCoords.v = icon->texLayout.v0;
	p->v[1].texCoords.u = icon->texLayout.u1;
	p->v[1].texCoords.v = icon->texLayout.v1;
	p->v[2].texCoords.u = icon->texLayout.u2;
	p->v[2].texCoords.v = bottomV;
	p->v[3].texCoords.u = icon->texLayout.u3;
	p->v[3].texCoords.v = bottomV;

	AddPrimitive(p, ot);
	primMem->cursor = p + 1;
}

#if defined(CTR_NATIVE)
internal u32 UI_NativeResolveDriverPortraitTexture(int driverID, struct Icon *icon, int *textureWidth, int *textureHeight)
{
	if ((driverID < 0) || (driverID >= LOAD_CHARACTER_ID_COUNT) || (icon == NULL))
		return 0;

	const int customIndex = NativeCustomRacer_GetPlayerSelection(driverID);
	if (customIndex >= 0)
		return NativeCustomRacer_GetPortraitTexture(customIndex, icon, textureWidth, textureHeight);

	return 0;
}

internal void UI_NativeDrawDriverIconTexture(u32 texture, int textureWidth, int textureHeight,
	struct Icon *templateIcon, s16 posX, s16 posY, struct PrimMem *primMem, uint32_t *ot,
	s16 scale, u32 color)
{
	if ((texture == 0) || (textureWidth <= 0) || (textureHeight <= 0) ||
	    (textureWidth > 255) || (textureHeight > 255) || (templateIcon == NULL))
		return;

	u32 oldTag = *ot;
	DR_PSYX_TEX *setTexture = (DR_PSYX_TEX *)primMem->cursor;
	PolyFT4 *poly = (PolyFT4 *)(setTexture + 1);
	DR_PSYX_TEX *resetTexture = (DR_PSYX_TEX *)(poly + 1);
	struct Icon nativeIcon = *templateIcon;

	nativeIcon.texLayout.u0 = 0;
	nativeIcon.texLayout.v0 = 0;
	nativeIcon.texLayout.u1 = (u8)textureWidth;
	nativeIcon.texLayout.v1 = 0;
	nativeIcon.texLayout.u2 = 0;
	nativeIcon.texLayout.v2 = (u8)textureHeight;
	nativeIcon.texLayout.u3 = (u8)textureWidth;
	nativeIcon.texLayout.v3 = (u8)textureHeight;
	nativeIcon.texLayout.clut = 0;

	SetPsyXTexture(setTexture, texture, textureWidth, textureHeight);
	primMem->cursor = poly;
	// The RGBA cache only stores transparent/opaque pixels, not the PS1 STP bit.
	UI_DrawDriverIcon(&nativeIcon, posX, posY, primMem, ot, 0, scale, color);

	SetPsyXTexture(resetTexture, 0, 0, 0);
	setTexture->tag = CtrGpu_PackOTTag(CtrGpu_PrimToOTLink24(poly), 0x02000000);
	poly->tag.self = CtrGpu_PackOTTag(CtrGpu_PrimToOTLink24(resetTexture), 0x09000000);
	resetTexture->tag = CtrGpu_PackOTTag(oldTag, 0x02000000);
	*ot = CtrGpu_PrimToOTLink24(setTexture);
	primMem->cursor = resetTexture + 1;
}

internal void UI_NativeDrawDriverIconDecalTexture(u32 texture, int textureWidth, int textureHeight,
	struct Icon *templateIcon, s16 posX, s16 posY, struct PrimMem *primMem, uint32_t *ot, s16 scale)
{
	if ((texture == 0) || (textureWidth <= 0) || (textureHeight <= 0) ||
	    (textureWidth > 255) || (textureHeight > 255) || (templateIcon == NULL))
		return;

	u32 oldTag = *ot;
	DR_PSYX_TEX *setTexture = (DR_PSYX_TEX *)primMem->cursor;
	POLY_FT4 *poly = (POLY_FT4 *)(setTexture + 1);
	DR_PSYX_TEX *resetTexture = (DR_PSYX_TEX *)(poly + 1);
	struct Icon nativeIcon = *templateIcon;

	nativeIcon.texLayout.u0 = 0;
	nativeIcon.texLayout.v0 = 0;
	nativeIcon.texLayout.u1 = (u8)textureWidth;
	nativeIcon.texLayout.v1 = 0;
	nativeIcon.texLayout.u2 = 0;
	nativeIcon.texLayout.v2 = (u8)textureHeight;
	nativeIcon.texLayout.u3 = (u8)textureWidth;
	nativeIcon.texLayout.v3 = (u8)textureHeight;
	nativeIcon.texLayout.clut = 0;

	SetPsyXTexture(setTexture, texture, textureWidth, textureHeight);
	primMem->cursor = poly;
	DecalHUD_DrawPolyFT4(&nativeIcon, posX, posY, primMem, ot, 0, scale);

	SetPsyXTexture(resetTexture, 0, 0, 0);
	setTexture->tag = CtrGpu_PackOTTag(CtrGpu_PrimToOTLink24(poly), 0x02000000);
	poly->tag = CtrGpu_PackOTTag(CtrGpu_PrimToOTLink24(resetTexture), 0x09000000);
	resetTexture->tag = CtrGpu_PackOTTag(oldTag, 0x02000000);
	*ot = CtrGpu_PrimToOTLink24(setTexture);
	primMem->cursor = resetTexture + 1;
}

internal void UI_NativeDrawDriverIconGT4Texture(u32 texture, int textureWidth, int textureHeight,
	struct Icon *templateIcon, s16 posX, s16 posY, struct PrimMem *primMem, uint32_t *ot,
	u32 color0, u32 color1, u32 color2, u32 color3, s16 scale)
{
	if ((texture == 0) || (textureWidth <= 0) || (textureHeight <= 0) ||
	    (textureWidth > 255) || (textureHeight > 255) || (templateIcon == NULL))
		return;

	u32 oldTag = *ot;
	DR_PSYX_TEX *setTexture = (DR_PSYX_TEX *)primMem->cursor;
	POLY_GT4 *poly = (POLY_GT4 *)(setTexture + 1);
	DR_PSYX_TEX *resetTexture = (DR_PSYX_TEX *)(poly + 1);
	struct Icon nativeIcon = *templateIcon;

	nativeIcon.texLayout.u0 = 0;
	nativeIcon.texLayout.v0 = 0;
	nativeIcon.texLayout.u1 = (u8)textureWidth;
	nativeIcon.texLayout.v1 = 0;
	nativeIcon.texLayout.u2 = 0;
	nativeIcon.texLayout.v2 = (u8)textureHeight;
	nativeIcon.texLayout.u3 = (u8)textureWidth;
	nativeIcon.texLayout.v3 = (u8)textureHeight;
	nativeIcon.texLayout.clut = 0;

	SetPsyXTexture(setTexture, texture, textureWidth, textureHeight);
	primMem->cursor = poly;
	DecalHUD_DrawPolyGT4(&nativeIcon, posX, posY, primMem, ot,
		color0, color1, color2, color3, 0, scale);

	SetPsyXTexture(resetTexture, 0, 0, 0);
	setTexture->tag = CtrGpu_PackOTTag(CtrGpu_PrimToOTLink24(poly), 0x02000000);
	poly->tag = CtrGpu_PackOTTag(CtrGpu_PrimToOTLink24(resetTexture), 0x0c000000);
	resetTexture->tag = CtrGpu_PackOTTag(oldTag, 0x02000000);
	*ot = CtrGpu_PrimToOTLink24(setTexture);
	primMem->cursor = resetTexture + 1;
}
#endif

void UI_DrawDriverIconForDriver(int driverID, struct Icon *icon, s16 posX, s16 posY,
	struct PrimMem *primMem, uint32_t *ot, char transparency, s16 scale, u32 color)
{
#if defined(CTR_NATIVE)
	if ((driverID >= 0) && (driverID < LOAD_CHARACTER_ID_COUNT) && (icon != NULL))
	{
		int textureWidth = 0;
		int textureHeight = 0;
		const u32 texture = UI_NativeResolveDriverPortraitTexture(driverID, icon, &textureWidth, &textureHeight);

		if (texture != 0)
		{
			UI_NativeDrawDriverIconTexture(texture, textureWidth, textureHeight, icon,
				posX, posY, primMem, ot, scale, color);
			return;
		}
	}
#endif

	UI_DrawDriverIcon(icon, posX, posY, primMem, ot, transparency, scale, color);
}

void UI_DrawDriverIconDecalForDriver(int driverID, struct Icon *icon, s16 posX, s16 posY,
	struct PrimMem *primMem, uint32_t *ot, char transparency, s16 scale)
{
#if defined(CTR_NATIVE)
	if ((driverID >= 0) && (driverID < LOAD_CHARACTER_ID_COUNT) && (icon != NULL))
	{
		int textureWidth = 0;
		int textureHeight = 0;
		const u32 texture = UI_NativeResolveDriverPortraitTexture(driverID, icon, &textureWidth, &textureHeight);
		if (texture != 0)
		{
			UI_NativeDrawDriverIconDecalTexture(texture, textureWidth, textureHeight, icon,
				posX, posY, primMem, ot, scale);
			return;
		}
	}
#endif

	DecalHUD_DrawPolyFT4(icon, posX, posY, primMem, ot, transparency, scale);
}

void UI_DrawDriverIconGT4ForDriver(int driverID, struct Icon *icon, s16 posX, s16 posY,
	struct PrimMem *primMem, uint32_t *ot, u32 color0, u32 color1, u32 color2, u32 color3,
	char transparency, s16 scale)
{
#if defined(CTR_NATIVE)
	if ((driverID >= 0) && (driverID < LOAD_CHARACTER_ID_COUNT) && (icon != NULL))
	{
		int textureWidth = 0;
		int textureHeight = 0;
		const u32 texture = UI_NativeResolveDriverPortraitTexture(driverID, icon, &textureWidth, &textureHeight);
		if (texture != 0)
		{
			UI_NativeDrawDriverIconGT4Texture(texture, textureWidth, textureHeight, icon,
				posX, posY, primMem, ot, color0, color1, color2, color3, scale);
			return;
		}
	}
#endif

	DecalHUD_DrawPolyGT4(icon, posX, posY, primMem, ot,
		color0, color1, color2, color3, transparency, scale);
}
