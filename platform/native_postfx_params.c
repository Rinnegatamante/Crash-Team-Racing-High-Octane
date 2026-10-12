#include <platform/native_postfx.h>
#include <platform/native_postfx_params.h>

const NativePostFXParameter native_postfx_parameters[NATIVE_POSTFX_PARAMETER_COUNT] =
{
	{"daltonic", "Protanopia", "Protanopia", 200, 0, 200, 10},
	{"daltonic", "Deuteranopia", "Deuteranopia", 100, 0, 200, 10},
	{"daltonic", "Tritanopia", "Tritanopia", 100, 0, 200, 10},
	{"gamma", "Gamma", "Gamma", 100, 10, 200, 5},
	{"hue", "Hue", "Hue", 100, 0, 200, 5},
	{"hue", "Saturation", "Saturation", 100, 0, 200, 5},
	{"vignette", "Radius", "Radius", 100, 0, 200, 5},
	{"vignette", "Softness", "Softness", 100, 5, 200, 5},
	{"vignette", "Opacity", "Opacity", 100, 0, 200, 5}
};
