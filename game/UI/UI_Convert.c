#include <common.h>

enum
{
	UI_CONVERT_CENTER_X = 0x100,
	UI_CONVERT_CENTER_Y = 0x6c,
	UI_CONVERT_NEGATIVE_ROUND_BIAS = 0xff,
	UI_CONVERT_FIXED_SHIFT = 8,
};

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8004caa8-0x8004cac8.
int UI_ConvertX_2(int posX, int scale)
{
	int scaledPosX = (posX - UI_CONVERT_CENTER_X) * scale;

	if (scaledPosX < 0)
	{
		scaledPosX += UI_CONVERT_NEGATIVE_ROUND_BIAS;
	}

	return scaledPosX >> UI_CONVERT_FIXED_SHIFT;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8004cac8-0x8004cae8.
int UI_ConvertY_2(int posY, int scale)
{
	int scaledPosY = (posY - UI_CONVERT_CENTER_Y) * scale;

	if (scaledPosY < 0)
	{
		scaledPosY += UI_CONVERT_NEGATIVE_ROUND_BIAS;
	}

	return scaledPosY >> UI_CONVERT_FIXED_SHIFT;
}
