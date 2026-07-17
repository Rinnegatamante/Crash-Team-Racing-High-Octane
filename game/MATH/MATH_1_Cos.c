#include <common.h>

int MATH_Cos(u32 angle)
{
	// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003d1c0-0x8003d214.
	u32 trig = CTR_ReadU32LE(&data.trigApprox[ANG_MODULO_HALF_PI(angle)]);
	s32 cosine;

	if ((angle & ANG_QUADRANT_BIT) != 0)
	{
		cosine = (s32)(trig << 0x10) >> 0x10;
		if ((angle & ANG_SIGN_BIT) == 0)
		{
			cosine = -cosine;
		}
	}
	else
	{
		cosine = (s32)trig >> 0x10;
		if ((angle & ANG_SIGN_BIT) != 0)
		{
			cosine = -cosine;
		}
	}

	return cosine;
}
