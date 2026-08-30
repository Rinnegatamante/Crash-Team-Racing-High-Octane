#ifndef NATIVE_RENDERER_H
#define NATIVE_RENDERER_H

#include <platform/native_renderer_types.h>

int NativeRenderer_InitialiseRender(char *windowName, int width, int height, int fullscreen);
int NativeRenderer_InitialisePSX(void);
void NativeRenderer_Shutdown(void);
void NativeRenderer_ResetDevice(void);
void NativeRenderer_BeginScene(void);
void NativeRenderer_EndScene(void);
void NativeRenderer_EndGpuFrame(void);
void NativeRenderer_FinishGpuMeasurements(void);
void NativeRenderer_UpdateSwapIntervalState(int swapInterval);
void NativeRenderer_SwapWindow(void);
void NativeRenderer_StoreFrameBuffer(int x, int y, int w, int h);
void NativeRenderer_PresentMainRenderTarget(void);
void NativeRenderer_PresentVRAMDisplay(void);
void NativeRenderer_PresentVRAMRect(int x, int y, int w, int h);
void NativeRenderer_PresentStreamingTexture(TextureID texture, int contentHeight, int displayHeight);
void NativeRenderer_SaveVRAM(const char *outputFileName, int x, int y, int width, int height, int readFromFramebuffer);
void NativeRenderer_Clear(int x, int y, int w, int h, u8 r, u8 g, u8 b);
void NativeRenderer_ClearVRAM(int x, int y, int w, int h, u8 r, u8 g, u8 b);
void NativeRenderer_CopyVRAM(u16 *src, int x, int y, int w, int h, int dstX, int dstY);
void NativeRenderer_ReadVRAM(u16 *dst, int x, int y, int dstW, int dstH);
void NativeRenderer_SyncVRAMToCPU(int x, int y, int w, int h);
void NativeRenderer_UpdateVRAM(void);
int NativeRenderer_GetVRAMStateSize(void);
int NativeRenderer_CaptureVRAMState(void *dst, int dstSize);
int NativeRenderer_RestoreVRAMState(const void *src, int srcSize);
TextureID NativeRenderer_GetVRAMTexture(void);
TextureID NativeRenderer_GetWhiteTexture(void);
#ifdef __vita__
#define NATIVE_PALETTE_HAS_TRANSPARENT 0x1
#define NATIVE_PALETTE_HAS_OPAQUE      0x2
#define NATIVE_PALETTE_HAS_STP         0x4
int NativeRenderer_GetPaletteProperties(TexFormat format, int clut);
TextureID NativeRenderer_GetCachedP4Texture(int page, int clut);
#endif
TextureID NativeRenderer_CreateStreamingTexture(int width, int height);
void NativeRenderer_UpdateStreamingTexture(TextureID texture, int width, int height, const u8 *rgbaPixels);
void NativeRenderer_DestroyStreamingTexture(TextureID texture);
void NativeRenderer_SetBlendMode(BlendMode blendMode);
void NativeRenderer_SetMixedSTPBlendMode(BlendMode blendMode);
void NativeRenderer_SetDepthState(int enable, int write);
void NativeRenderer_SetStencilMode(int drawPrim);
void NativeRenderer_SetOffscreenState(const RECT16 *offscreenRect, int enable);
void NativeRenderer_SetProjection(const RECT16 *drawRect, const DISPENV *displayEnv, int offscreen);
void NativeRenderer_SetupClipMode(const RECT16 *clipRect, const DISPENV *displayEnv, int enable);
void NativeRenderer_SetTexture(TextureID texture, TexFormat texFormat, int semiTransPass, BlendMode blendMode, int textured,
                               int textureFullyOpaque, int cachedP4);
void NativeRenderer_SetOverrideTextureSize(int width, int height);
void NativeRenderer_SetPSXTextureOutputSTP(int enabled);
void NativeRenderer_SetPSXDrawMaskSet(int maskSet);
GrVertex *NativeRenderer_AllocateVertexBuffer(int count);
void NativeRenderer_UpdateVertexBuffer(const GrVertex *vertices, int count);
void NativeRenderer_DrawTriangles(int startVertex, int triangles);
void NativeRenderer_PushDebugLabel(const char *label);
void NativeRenderer_PopDebugLabel(void);

#endif
