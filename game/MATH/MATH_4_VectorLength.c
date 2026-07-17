#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003d328-0x8003d378
s32 MATH_VectorLength(SVec3 *vector)
{
	u32 xy = CTR_ReadU32LE(&vector->x);
	s32 z = vector->z;

	CTC2(xy, 0);
	CTC2((u32)z, 1);

	MTC2(xy, 0);
	MTC2(z, 1);
	gte_mvmva(0, 0, 0, 3, 0);

	return SquareRoot0_stub((s32)MFC2(25));
}
