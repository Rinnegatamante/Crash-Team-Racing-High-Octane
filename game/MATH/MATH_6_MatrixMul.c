#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003d460-0x8003d4e4.
void MATH_MatrixMul(MATRIX *output, MATRIX *input, MATRIX *transform)
{
	MatrixRotate(output, input, transform);
	VECTOR transformT = {transform->t[0], transform->t[1], transform->t[2], 0};
	VECTOR outputT;
	ApplyMatrixLV_stub(&transformT, &outputT);
	output->t[0] = outputT.vx;
	output->t[1] = outputT.vy;
	output->t[2] = outputT.vz;

	output->t[0] += input->t[0];
	output->t[1] += input->t[1];
	output->t[2] += input->t[2];
}
