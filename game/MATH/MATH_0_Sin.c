#include <common.h>

int MATH_Sin(u32 angle)
{
	// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003d184-0x8003d1c0.
	u32 trig = CTR_ReadU32LE(&data.trigApprox[ANG_MODULO_HALF_PI(angle)]);
	if ((angle & ANG_QUADRANT_BIT) == 0)
	{
		trig <<= 0x10;
	}

	s32 sine = (s32)trig >> 0x10;
	if ((angle & ANG_SIGN_BIT) != 0)
	{
		sine = -sine;
	}

	return sine;
}
