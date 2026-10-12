#ifndef CTR_NATIVE_POSTFX_PARAMS_H
#define CTR_NATIVE_POSTFX_PARAMS_H

#include <stddef.h>

typedef struct NativePostFXParameter
{
	const char *effect;
	const char *label;
	const char *uniform;
	int defaultCent;
	int minimumCent;
	int maximumCent;
	int stepCent;
} NativePostFXParameter;

extern const NativePostFXParameter native_postfx_parameters[NATIVE_POSTFX_PARAMETER_COUNT];

#endif
