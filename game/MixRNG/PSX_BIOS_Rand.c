#include <common.h>

static u32 psxRandSeed = 1;

// NOTE(aalhendi): Retail `rand` is the PSX BIOS A(2F) stub at 0x80078be8,
// not the host C runtime. Keep this global symbol so retail call sites that
// use `rand()` resolve to the PSX BIOS sequence on native too.
int rand(void)
{
	psxRandSeed = psxRandSeed * 0x41c64e6dU + 0x3039U;
	return (int)((psxRandSeed >> 16) & 0x7fffU);
}

void srand(unsigned int seed)
{
	psxRandSeed = (u32)seed;
}

u32 PSX_BIOS_GetRandSeed(void)
{
	return psxRandSeed;
}

void PSX_BIOS_SetRandSeed(u32 seed)
{
	psxRandSeed = seed;
}
