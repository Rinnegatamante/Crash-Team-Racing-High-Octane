#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8006c684-0x8006c6c8
int RngDeadCoed(struct RngDeadCoedState *state)
{
	u32 state0 = state->state0;
	u32 state1 = state->state1;
	u32 shifted1 = state1 >> 8;
	u32 shifted0 = (state0 >> 8) | (state1 << 24);
	u32 mixed = (shifted1 | ((state0 + shifted1 + (shifted0 >> 8)) << 24)) ^ 0xdeadc0ed;

	state->state1 = mixed;
	state->state0 = shifted0;
	return mixed;
}
