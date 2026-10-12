#ifndef CTR_NATIVE_POSTFX_H
#define CTR_NATIVE_POSTFX_H

enum
{
	NATIVE_POSTFX_OFF = 0,
	NATIVE_POSTFX_CRT,
	NATIVE_POSTFX_CRT_BLEEDING,
	NATIVE_POSTFX_DALTONIC,
	NATIVE_POSTFX_FXAA,
	NATIVE_POSTFX_GAMMA,
	NATIVE_POSTFX_GREYSCALE,
	NATIVE_POSTFX_HUE,
	NATIVE_POSTFX_LCD3X,
	NATIVE_POSTFX_NEGATIVE,
	NATIVE_POSTFX_SEPIA,
	NATIVE_POSTFX_SHARP_GRID,
	NATIVE_POSTFX_SHARP_SCANLINES,
	NATIVE_POSTFX_VIGNETTE,
	NATIVE_POSTFX_MODE_COUNT,
	NATIVE_POSTFX_PARAMETER_COUNT = 9
};

extern int gNativePostFXMode;
extern int gNativePostFXParameterValues[NATIVE_POSTFX_PARAMETER_COUNT];
extern unsigned gNativePostFXRevision;

const char *NativePostFX_GetName(int mode);
const char *NativePostFX_GetLabel(int mode);
void NativePostFX_SetMode(int mode);
void NativePostFX_SetParameter(int index, int centValue);
int NativePostFX_CalculateCrop(int x, int y, int width, int height, int displayWidth, int displayHeight, float crop[4]);

#if !defined(__EMSCRIPTEN__)
unsigned NativePostFX_TargetFramebuffer(int width, int height);
int NativePostFX_Finish(int x, int y, int width, int height, int displayWidth, int displayHeight);
void NativePostFX_Shutdown(void);
#endif

#endif
