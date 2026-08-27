/*
 * Derived from REDRIVER2/PsyCross MIT source:
 * externals/PsyCross/src/render/PsyX_render.cpp
 * See THIRD_PARTY_NOTICES.md for copyright and license details.
 */

#include <macros.h>
#include "platform/native_renderer_types.h"
#include <SDL3/SDL.h>

#include "platform/native_gpu.h"
#include "platform/native_glad.h"
#include "platform/native_log.h"
#include "platform/native_perf.h"
#include "platform/native_renderer.h"

#include <assert.h>
#include <string.h>

#ifdef _WIN32
#include "platform/native_win32.h"

__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

#endif // def WIN32

#ifdef __vita__
#define VRAM_FORMAT          GL_RGBA
#define VRAM_INTERNAL_FORMAT GL_RGBA
#else
#define VRAM_FORMAT          GL_RG
// NOTE(penta3): VRAM holds packed 16-bit PSX pixels as two bytes (R=low,
// G=high), uploaded as GL_RG + GL_UNSIGNED_BYTE. RG8 is the faithful storage:
// 2 bytes/texel = real PS1's 1MB VRAM, vs RG32F's 8 bytes/texel (4MB). With
// NEAREST sampling RG8 returns byte/255 exactly, identical to what the shader
// got from RG32F, so CLUT/texture-page reconstruction is unchanged.
#define VRAM_INTERNAL_FORMAT GL_RG8
#endif

#define NATIVE_RENDER_TARGET_TYPE GL_UNSIGNED_BYTE

extern SDL_Window *g_window;

#define NATIVE_RENDERER_LOG(fmt, ...)   Platform_Log("[CTR Renderer] " fmt, __VA_ARGS__)
#define NATIVE_RENDERER_ERROR(fmt, ...) Platform_LogError("[CTR Renderer] [%s] - " fmt, __func__, __VA_ARGS__)

#ifdef __vita__
#define MAX_NUM_VERTEX_BUFFERS (1)
#else
#define MAX_NUM_VERTEX_BUFFERS (2)
#endif
#define PSX_SCREEN_ASPECT               (240.0f / 320.0f) // PSX screen is mapped always to this aspect

#if defined(CTR_INTERNAL)
#ifndef GL_TIME_ELAPSED
#define GL_TIME_ELAPSED 0x88BF
#endif
#define NATIVE_GPU_TIMER_QUERY_COUNT 8

struct NativeGpuTimerQuery
{
	GLuint id;
	u32 frameIndex;
	b32 pending;
};

global_variable struct NativeGpuTimerQuery s_gpuTimerQueries[NATIVE_GPU_TIMER_QUERY_COUNT];
global_variable u32 s_gpuTimerFrameIndex;
global_variable s32 s_gpuTimerNextQuery;
global_variable b32 s_gpuTimerSupported;
global_variable b32 s_gpuTimerActive;
#endif

global_variable BlendMode s_previousBlendMode = BM_NONE;
global_variable int s_previousDepthMode = 0;
global_variable int s_previousStencilMode = 0;
global_variable int s_previousScissorState = 0;
global_variable int s_previousOffscreenState = 0;
global_variable RECT16 s_previousOffscreen = {0, 0, 0, 0};

global_variable ShaderID s_previousShader = (ShaderID)-1;

global_variable TextureID s_rgLutTexture = (TextureID)-1;
#ifdef __vita__
global_variable TextureID s_presentLutTexture = (TextureID)-1;
#endif
// NOTE(penta3): Single persistent VRAM texture, matching real PS1's single
// 1MB VRAM. PS1 page-flips two windows *inside* one VRAM; it never keeps two
// full copies. The old double buffer existed only to orphan the texture on the
// per-frame full re-upload, which no longer happens (we upload dirty rects).
#define NATIVE_VRAM_DIRTY_RECT_CAP 128
#define NATIVE_VRAM_TILE_SIZE      8
#define NATIVE_VRAM_TILE_COLS      (VRAM_WIDTH / NATIVE_VRAM_TILE_SIZE)
#define NATIVE_VRAM_TILE_ROWS      (VRAM_HEIGHT / NATIVE_VRAM_TILE_SIZE)
#define NATIVE_VRAM_TILE_COUNT     (NATIVE_VRAM_TILE_COLS * NATIVE_VRAM_TILE_ROWS)
#define NATIVE_VRAM_TILE_WORDS     (NATIVE_VRAM_TILE_COUNT / 32)

// NOTE(aalhendi): Native splits PS1 VRAM between a CPU mirror and one packed
// GPU texture. CPU writes are uploaded before GPU reads; GPU-newer tiles are
// resolved into the CPU mirror only when game code reads those VRAM regions.
struct NativeVramState
{
	TextureID texture;
	u16 cpuPixels[VRAM_WIDTH * VRAM_HEIGHT];
	RECT16 cpuDirtyRects[NATIVE_VRAM_DIRTY_RECT_CAP];
	u32 gpuNewerTiles[NATIVE_VRAM_TILE_WORDS];
	s32 cpuDirtyRectCount;
};

global_variable struct NativeVramState s_vram;

#ifdef __vita__
global_variable u8 s_vitaVramTransferPixels[VRAM_WIDTH * VRAM_HEIGHT * 4];
#endif

struct NativeRenderTarget
{
	TextureID texture;
	GLuint framebuffer;
	GLuint stencilBuffer;
	s32 width;
	s32 height;
	s32 logicalWidth;
	s32 logicalHeight;
};

global_variable struct NativeRenderTarget s_mainRenderTarget;
global_variable struct NativeRenderTarget s_offscreenRenderTarget;

global_variable TextureID s_whiteTexture = (TextureID)-1;
global_variable TextureID s_lastBoundTexture = (TextureID)-1;

TextureID NativeRenderer_GetVRAMTexture(void)
{
	return s_vram.texture;
}

TextureID NativeRenderer_GetWhiteTexture(void)
{
	return s_whiteTexture;
}

TextureID NativeRenderer_CreateStreamingTexture(int width, int height)
{
	TextureID texture = 0;

	if ((width <= 0) || (height <= 0))
	{
		return 0;
	}

	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	s_lastBoundTexture = (TextureID)-1;
	return texture;
}

void NativeRenderer_UpdateStreamingTexture(TextureID texture, int width, int height, const u8 *rgbaPixels)
{
	if ((texture == 0) || (width <= 0) || (height <= 0) || (rgbaPixels == NULL))
	{
		return;
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, rgbaPixels);
	s_lastBoundTexture = (TextureID)-1;
}

void NativeRenderer_DestroyStreamingTexture(TextureID texture)
{
	if (texture == 0)
	{
		return;
	}

	if (s_lastBoundTexture == texture)
	{
		s_lastBoundTexture = (TextureID)-1;
	}
	glDeleteTextures(1, &texture);
}

int g_windowWidth = 0;
int g_windowHeight = 0;

global_variable int s_presentAspectW = 4;
global_variable int s_presentAspectH = 3;
global_variable SDL_Rect s_presentViewport = {0, 0, 0, 0};

int g_dbg_wireframeMode = 0;
int g_dbg_texturelessMode = 0;

int g_cfg_bilinearFiltering = 0;

// NOTE(aalhendi): Pack native RGBA render targets into the persistent RG8 VRAM
// texture on the GPU instead of a GPU-to-CPU-to-GPU round trip.
global_variable GLuint s_packShader = 0;
global_variable GLint s_packFlipYLoc = -1;
global_variable GLuint s_presentVramShader = 0;
global_variable GLint s_presentVramSourceRectLoc = -1;
global_variable GLuint s_presentRgbaShader = 0;
global_variable GLint s_presentRgbaFlipYLoc = -1;
global_variable GLuint s_vramQuadVAO = 0;
global_variable GLuint s_vramQuadVBO = 0;

internal int NativeRenderer_InitialiseGLContext(char *windowName, int fullscreen);
internal int NativeRenderer_InitialiseGLExt(void);
internal void NativeRenderer_DestroyTexture(TextureID texture);
internal void NativeRenderer_SetScissorState(int enable);
internal void NativeRenderer_EnableDepth(int enable);
internal void NativeRenderer_SetViewPort(int x, int y, int width, int height);
internal void NativeRenderer_SetPresentationAspect(int width, int height);
internal void NativeRenderer_UpdatePresentationViewport(void);
internal void NativeRenderer_ClearPresentationBars(void);
internal void NativeRenderer_SetWireframe(int enable);
internal void NativeRenderer_InitRenderTarget(struct NativeRenderTarget *target);
internal void NativeRenderer_DestroyRenderTarget(struct NativeRenderTarget *target);
internal void NativeRenderer_EnsureRenderTarget(struct NativeRenderTarget *target, int width, int height);
internal void NativeRenderer_BindMainRenderTarget(void);
internal void NativeRenderer_DrawVRAMRegion(int x, int y, int width, int height);
internal void NativeRenderer_LoadRenderTargetFromVRAM(struct NativeRenderTarget *target, int x, int y, int logicalWidth, int logicalHeight);
internal void NativeRenderer_DestroyPSXShaders(void);
#if defined(CTR_INTERNAL)
internal void NativeRenderer_ResolveGpuMeasurements(b32 waitForResults);
#endif

global_variable GLuint s_glVertexArray[MAX_NUM_VERTEX_BUFFERS];
global_variable GLuint s_glVertexBuffer[MAX_NUM_VERTEX_BUFFERS];
global_variable int s_curVertexBuffer = 0;
global_variable int s_boundVertexBuffer = -1;

global_variable GLuint s_glVramFramebuffer;


internal int NativeRenderer_InitialiseGLContext(char *windowName, int fullscreen)
{
	SDL_WindowFlags windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

	if (fullscreen)
	{
		windowFlags |= SDL_WINDOW_FULLSCREEN;
	}

	g_window = SDL_CreateWindow(windowName, g_windowWidth, g_windowHeight, windowFlags);

	if (g_window == NULL)
	{
		NATIVE_RENDERER_ERROR("%s\n", "Failed to initialise SDL window!");
		return 0;
	}

	int major_version = 3;
	int minor_version = 3;
	int profile = SDL_GL_CONTEXT_PROFILE_CORE;

	// find best OpenGL version
	do
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major_version);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor_version);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, profile);

		if (SDL_GL_CreateContext(g_window))
		{
			break;
		}

		minor_version--;

	} while (minor_version >= 0);

	if (minor_version == -1)
	{
		NATIVE_RENDERER_ERROR("%s\n", "Failed to initialise - OpenGL 3.x is not supported. Please update video drivers.");
		return 0;
	}

	return 1;
}

internal int NativeRenderer_InitialiseGLExt(void)
{
#ifndef __vita__
	GLenum err = gladLoadGL();

	if (err == 0)
	{
		return 0;
	}
#endif

	const char *rend = (const char *)glGetString(GL_RENDERER);
	const char *vendor = (const char *)glGetString(GL_VENDOR);
	NATIVE_RENDERER_LOG("*Video adapter: %s by %s\n", rend, vendor);

	const char *versionStr = (const char *)glGetString(GL_VERSION);
	NATIVE_RENDERER_LOG("*OpenGL version: %s\n", versionStr);

	const char *glslVersionStr = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
	NATIVE_RENDERER_LOG("*GLSL version: %s\n", glslVersionStr);

	return 1;
}

int NativeRenderer_InitialiseRender(char *windowName, int width, int height, int fullscreen)
{
	g_windowWidth = width;
	g_windowHeight = height;
	NativeRenderer_SetPresentationAspect(width, height);

	// Due to debugging in fullscreen
	SDL_SetHint(SDL_HINT_WINDOW_ALLOW_TOPMOST, "0");
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);

	if (!NativeRenderer_InitialiseGLContext(windowName, fullscreen))
	{
		NATIVE_RENDERER_ERROR("%s\n", "Failed to Initialise GL Context!");
		return 0;
	}

	if (!NativeRenderer_InitialiseGLExt())
	{
		NATIVE_RENDERER_ERROR("%s\n", "Failed to Intialise GL extensions");
		return 0;
	}

	return 1;
}

void NativeRenderer_Shutdown(void)
{
	glDeleteVertexArrays(MAX_NUM_VERTEX_BUFFERS, s_glVertexArray);
	glDeleteBuffers(MAX_NUM_VERTEX_BUFFERS, s_glVertexBuffer);

	NativeRenderer_DestroyRenderTarget(&s_mainRenderTarget);
	NativeRenderer_DestroyRenderTarget(&s_offscreenRenderTarget);
	glDeleteFramebuffers(1, &s_glVramFramebuffer);

	NativeRenderer_DestroyTexture(s_vram.texture);

	NativeRenderer_DestroyTexture(s_whiteTexture);
	NativeRenderer_DestroyTexture(s_rgLutTexture);
#ifdef __vita__
	NativeRenderer_DestroyTexture(s_presentLutTexture);
#endif
	NativeRenderer_DestroyPSXShaders();
	glDeleteProgram(s_packShader);
	glDeleteProgram(s_presentVramShader);
	glDeleteProgram(s_presentRgbaShader);
	glDeleteVertexArrays(1, &s_vramQuadVAO);
	glDeleteBuffers(1, &s_vramQuadVBO);
}

#if defined(CTR_INTERNAL)
internal void NativeRenderer_ResolveGpuMeasurements(b32 waitForResults)
{
	for (s32 i = 0; i < NATIVE_GPU_TIMER_QUERY_COUNT; i++)
	{
		struct NativeGpuTimerQuery *query = &s_gpuTimerQueries[i];
		if (!query->pending)
		{
			continue;
		}

		GLint available = 0;
		if (!waitForResults)
		{
			glGetQueryObjectiv(query->id, GL_QUERY_RESULT_AVAILABLE, &available);
			if (!available)
			{
				continue;
			}
		}

		GLuint elapsedNanoseconds;
		glGetQueryObjectuiv(query->id, GL_QUERY_RESULT, &elapsedNanoseconds);
		NativePerf_RecordGpuFrame(query->frameIndex, (f64)elapsedNanoseconds / 1000000.0);
		query->pending = false;
	}
}
#endif

void NativeRenderer_UpdateSwapIntervalState(int swapInterval)
{
	SDL_GL_SetSwapInterval(swapInterval);
}

void NativeRenderer_BeginScene(void)
{
#if defined(CTR_INTERNAL)
	NativeRenderer_ResolveGpuMeasurements(false);
	const u32 gpuFrameIndex = s_gpuTimerFrameIndex++;
	if (s_gpuTimerSupported && NativePerf_IsEnabled())
	{
		struct NativeGpuTimerQuery *query = &s_gpuTimerQueries[s_gpuTimerNextQuery];
		if (!query->pending)
		{
			query->frameIndex = gpuFrameIndex;
			query->pending = true;
			glBeginQuery(GL_TIME_ELAPSED, query->id);
			s_gpuTimerActive = true;
			s_gpuTimerNextQuery = (s_gpuTimerNextQuery + 1) % NATIVE_GPU_TIMER_QUERY_COUNT;
		}
	}
#endif

	NativePerf_BeginScope(NATIVE_PERF_BUCKET_RENDERER_BEGIN_SCENE);
	s_lastBoundTexture = 0;

	NativeRenderer_UpdatePresentationViewport();
	NativeRenderer_ClearPresentationBars();
	NativeRenderer_BindMainRenderTarget();

	NativeRenderer_UpdateVRAM();
	if (!activeDrawEnv.isbg)
	{
		NativeRenderer_LoadRenderTargetFromVRAM(&s_mainRenderTarget, activeDispEnv.disp.x, activeDispEnv.disp.y,
		                                        s_mainRenderTarget.logicalWidth, s_mainRenderTarget.logicalHeight);
	}
	else
	{
		glClear(GL_STENCIL_BUFFER_BIT);
	}
	NativeRenderer_SetViewPort(0, 0, s_mainRenderTarget.width, s_mainRenderTarget.height);

	if (g_dbg_wireframeMode)
	{
		NativeRenderer_SetWireframe(1);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	NativePerf_EndScope(NATIVE_PERF_BUCKET_RENDERER_BEGIN_SCENE);
}

void NativeRenderer_EndGpuFrame(void)
{
#if defined(CTR_INTERNAL)
	if (s_gpuTimerActive)
	{
		glEndQuery(GL_TIME_ELAPSED);
		s_gpuTimerActive = false;
	}
#endif
}

void NativeRenderer_FinishGpuMeasurements(void)
{
#if defined(CTR_INTERNAL)
	NativeRenderer_EndGpuFrame();
	if (!s_gpuTimerSupported)
	{
		return;
	}

	NativeRenderer_ResolveGpuMeasurements(true);
	for (s32 i = 0; i < NATIVE_GPU_TIMER_QUERY_COUNT; i++)
	{
		glDeleteQueries(1, &s_gpuTimerQueries[i].id);
	}
	SDL_memset(s_gpuTimerQueries, 0, sizeof(s_gpuTimerQueries));
	s_gpuTimerSupported = false;
#endif
}

void NativeRenderer_EndScene(void)
{
	if (s_previousOffscreenState)
	{
		NativeRenderer_SetOffscreenState(&s_previousOffscreen, 0);
	}

	if (g_dbg_wireframeMode)
	{
		NativeRenderer_SetWireframe(0);
	}

	glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------

global_variable u8 rgLUT[LUT_WIDTH * LUT_HEIGHT * sizeof(u32)];

internal int NativeRenderer_IntAbs(int value)
{
	return value < 0 ? -value : value;
}

internal int NativeRenderer_GCD(int a, int b)
{
	a = NativeRenderer_IntAbs(a);
	b = NativeRenderer_IntAbs(b);

	while (b != 0)
	{
		int t = a % b;
		a = b;
		b = t;
	}

	return a;
}

internal void NativeRenderer_SetPresentationAspect(int width, int height)
{
	const int divisor = NativeRenderer_GCD(width, height);

	if ((width <= 0) || (height <= 0) || (divisor <= 0))
	{
		return;
	}

	s_presentAspectW = width / divisor;
	s_presentAspectH = height / divisor;
}

internal void NativeRenderer_UpdatePresentationViewport(void)
{
	int viewportW;
	int viewportH;

	if ((g_windowWidth <= 0) || (g_windowHeight <= 0) || (s_presentAspectW <= 0) || (s_presentAspectH <= 0))
	{
		s_presentViewport.x = 0;
		s_presentViewport.y = 0;
		s_presentViewport.w = g_windowWidth;
		s_presentViewport.h = g_windowHeight;
		return;
	}

	viewportW = g_windowWidth;
	viewportH = (viewportW * s_presentAspectH) / s_presentAspectW;

	if (viewportH > g_windowHeight)
	{
		viewportH = g_windowHeight;
		viewportW = (viewportH * s_presentAspectW) / s_presentAspectH;
	}

	if (viewportW < 1)
	{
		viewportW = 1;
	}
	if (viewportH < 1)
	{
		viewportH = 1;
	}

	s_presentViewport.w = viewportW;
	s_presentViewport.h = viewportH;
	s_presentViewport.x = (g_windowWidth - viewportW) / 2;
	s_presentViewport.y = (g_windowHeight - viewportH) / 2;
}

internal void NativeRenderer_InitRenderTarget(struct NativeRenderTarget *target)
{
	target->texture = (TextureID)-1;
	glGenTextures(1, &target->texture);
	glBindTexture(GL_TEXTURE_2D, target->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, NATIVE_RENDER_TARGET_TYPE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffers(1, &target->stencilBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, target->stencilBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, 1, 1);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenFramebuffers(1, &target->framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, target->framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target->texture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, target->stencilBuffer);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		NATIVE_RENDERER_ERROR("%s\n", "failed to create RGBA/stencil render target");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	target->logicalWidth = 0;
	target->logicalHeight = 0;
}

internal void NativeRenderer_DestroyRenderTarget(struct NativeRenderTarget *target)
{
	glDeleteFramebuffers(1, &target->framebuffer);
	glDeleteRenderbuffers(1, &target->stencilBuffer);
	NativeRenderer_DestroyTexture(target->texture);
	SDL_memset(target, 0, sizeof(*target));
	target->texture = (TextureID)-1;
}

internal void NativeRenderer_EnsureRenderTarget(struct NativeRenderTarget *target, int width, int height)
{
	if (width < 1)
	{
		width = 1;
	}
	if (height < 1)
	{
		height = 1;
	}

	if ((target->width == width) && (target->height == height))
	{
		return;
	}

	glBindTexture(GL_TEXTURE_2D, target->texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, NATIVE_RENDER_TARGET_TYPE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, target->stencilBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	target->width = width;
	target->height = height;
	s_lastBoundTexture = (TextureID)-1;
}

internal void NativeRenderer_BindMainRenderTarget(void)
{
	int logicalWidth = activeDispEnv.disp.w;
	int logicalHeight = activeDispEnv.disp.h;
	if ((logicalWidth <= 0) || (logicalHeight <= 0))
	{
		logicalWidth = activeDrawEnv.clip.w;
		logicalHeight = activeDrawEnv.clip.h;
	}

	int physicalWidth = logicalWidth;
	int physicalHeight = logicalHeight;
#ifdef __vita__
	if ((g_windowWidth > 0) && (g_windowHeight > 0))
	{
		physicalWidth = g_windowWidth;
		physicalHeight = g_windowHeight;
	}
#endif

	NativeRenderer_EnsureRenderTarget(&s_mainRenderTarget, physicalWidth, physicalHeight);
	s_mainRenderTarget.logicalWidth = logicalWidth;
	s_mainRenderTarget.logicalHeight = logicalHeight;
	glBindFramebuffer(GL_FRAMEBUFFER, s_mainRenderTarget.framebuffer);
}

internal void NativeRenderer_DrawVRAMRegion(int x, int y, int width, int height)
{
	glUseProgram(s_presentVramShader);
	glUniform4f(s_presentVramSourceRectLoc, (float)x, (float)y, (float)width, (float)height);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, s_vram.texture);
	glBindVertexArray(s_vramQuadVAO);
	NativeRenderer_DrawTriangles(0, 2);
}

internal void NativeRenderer_LoadRenderTargetFromVRAM(struct NativeRenderTarget *target, int x, int y, int logicalWidth, int logicalHeight)
{
	const ShaderID previousShader = s_previousShader;
	const TextureID previousTexture = s_lastBoundTexture;
	const BlendMode previousBlendMode = s_previousBlendMode;
	const int previousScissorState = s_previousScissorState;
	const GLboolean previousStencilEnabled = glIsEnabled(GL_STENCIL_TEST);

	NativeRenderer_UpdateVRAM();
	glBindFramebuffer(GL_FRAMEBUFFER, target->framebuffer);
	glDisable(GL_BLEND);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glViewport(0, 0, target->width, target->height);
	NativeRenderer_DrawVRAMRegion(x, y, logicalWidth, logicalHeight);
	glClear(GL_STENCIL_BUFFER_BIT);
	if (previousStencilEnabled)
	{
		glEnable(GL_STENCIL_TEST);
	}

	if (s_boundVertexBuffer >= 0)
	{
		glBindVertexArray(s_glVertexArray[s_boundVertexBuffer]);
	}
	else
	{
		glBindVertexArray(0);
	}
	glUseProgram(previousShader == (ShaderID)-1 ? 0 : previousShader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, previousTexture == (TextureID)-1 ? 0 : previousTexture);
	s_previousShader = previousShader;
	s_lastBoundTexture = previousTexture;
	s_previousBlendMode = BM_NONE;
	s_previousScissorState = 0;
	NativeRenderer_SetBlendMode(previousBlendMode);
	NativeRenderer_SetScissorState(previousScissorState);
}

internal void NativeRenderer_ClearHostRect(int x, int y, int width, int height)
{
	if ((width <= 0) || (height <= 0))
	{
		return;
	}

	glScissor(x, y, width, height);
	glClear(GL_COLOR_BUFFER_BIT);
}

internal void NativeRenderer_ClearPresentationBars(void)
{
	GLint previousScissorBox[4];
	GLfloat previousClearColor[4];
	const GLboolean previousScissorEnabled = glIsEnabled(GL_SCISSOR_TEST);
	const int viewportRight = s_presentViewport.x + s_presentViewport.w;
	const int viewportTop = s_presentViewport.y + s_presentViewport.h;

	if ((g_windowWidth <= 0) || (g_windowHeight <= 0))
	{
		return;
	}

	if ((s_presentViewport.x == 0) && (s_presentViewport.y == 0) && (s_presentViewport.w == g_windowWidth) && (s_presentViewport.h == g_windowHeight))
	{
		return;
	}

	glGetIntegerv(GL_SCISSOR_BOX, previousScissorBox);
	glGetFloatv(GL_COLOR_CLEAR_VALUE, previousClearColor);

	glEnable(GL_SCISSOR_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	NativeRenderer_ClearHostRect(0, 0, s_presentViewport.x, g_windowHeight);
	NativeRenderer_ClearHostRect(viewportRight, 0, g_windowWidth - viewportRight, g_windowHeight);
	NativeRenderer_ClearHostRect(s_presentViewport.x, 0, s_presentViewport.w, s_presentViewport.y);
	NativeRenderer_ClearHostRect(s_presentViewport.x, viewportTop, s_presentViewport.w, g_windowHeight - viewportTop);

	if (previousScissorEnabled)
	{
		glEnable(GL_SCISSOR_TEST);
		glScissor(previousScissorBox[0], previousScissorBox[1], previousScissorBox[2], previousScissorBox[3]);
	}
	else
	{
		glDisable(GL_SCISSOR_TEST);
	}

	glClearColor(previousClearColor[0], previousClearColor[1], previousClearColor[2], previousClearColor[3]);
	s_previousScissorState = previousScissorEnabled ? 1 : 0;
}

void NativeRenderer_ResetDevice(void)
{
	NativeRenderer_UpdatePresentationViewport();
	NativeRenderer_UpdateSwapIntervalState(0);
}

typedef struct
{
	// shader itself
	ShaderID shader;

	GLint projectionLoc;
	GLint bilinearFilterLoc;
	GLint texelSizeLoc;
	GLint texLoc;
	GLint lutLoc;
#ifndef __vita__
	GLint psxSemiTransPassLoc;
#endif
	GLint psxDrawMaskSetLoc;
	GLint psxTextureOutputStpLoc;
} GTEShader;

#ifdef __vita__
enum NativePsxShaderVariant
{
	NATIVE_PSX_SHADER_OPAQUE,
	NATIVE_PSX_SHADER_OPAQUE_AVERAGE,
	NATIVE_PSX_SHADER_OPAQUE_QUARTER,
	NATIVE_PSX_SHADER_NON_STP,
	NATIVE_PSX_SHADER_STP,
	NATIVE_PSX_SHADER_STP_AVERAGE,
	NATIVE_PSX_SHADER_STP_QUARTER,
	NATIVE_PSX_SHADER_VARIANT_COUNT
};
#endif

internal int NativeRenderer_Shader_CheckShaderStatus(GLuint shader);
internal int NativeRenderer_Shader_CheckProgramStatus(GLuint program);
internal ShaderID NativeRenderer_Shader_Compile(const char *source, bool isPsxShader, const char *fragmentDefines);
internal void NativeRenderer_GenerateCommonTextures(void);
internal void NativeRenderer_CompilePSXShader(GTEShader *sh, const char *source, const char *fragmentDefines);
internal void NativeRenderer_InitialisePSXShaders(void);
internal void NativeRenderer_InitRG8LUT(void);
internal void NativeRenderer_Ortho2D(float left, float right, float bottom, float top, float znear, float zfar);
internal void NativeRenderer_SetShader(const ShaderID shader);
internal void NativeRenderer_SyncGpuVRAMToCPU(int x, int y, int w, int h);
internal void NativeRenderer_ResolveVRAMRead(int x, int y, int w, int h);
internal void NativeRenderer_GpuPackTextureToVRAM(TextureID sourceTexture, int x, int y, int w, int h, b32 flipY);

#ifdef __vita__
global_variable GTEShader s_gteShaderVariants[4][NATIVE_PSX_SHADER_VARIANT_COUNT];
global_variable GTEShader s_gteFullyOpaqueShaderVariants[2][3];
global_variable GTEShader s_gteUntexturedShaderVariants[3];
#else
global_variable GTEShader s_gteShader4;
global_variable GTEShader s_gteShader8;
global_variable GTEShader s_gteShader16;
global_variable GTEShader s_gteShader32Rgba;
#endif

GLint u_projectionLoc;
GLint u_bilinearFilterLoc;
GLint u_texelSizeLoc;
#ifndef __vita__
GLint u_psxSemiTransPassLoc;
#endif
GLint u_psxDrawMaskSetLoc;
GLint u_psxTextureOutputStpLoc;

internal void NativeRenderer_DestroyPSXShaders(void)
{
#ifdef __vita__
	for (int format = TF_4_BIT; format <= TF_32_BIT_RGBA; format++)
	{
		for (int variant = 0; variant < NATIVE_PSX_SHADER_VARIANT_COUNT; variant++)
		{
			if (s_gteShaderVariants[format][variant].shader != 0)
			{
				glDeleteProgram(s_gteShaderVariants[format][variant].shader);
			}
		}
	}
	for (int format = TF_4_BIT; format <= TF_8_BIT; format++)
	{
		for (int variant = 0; variant < 3; variant++)
		{
			glDeleteProgram(s_gteFullyOpaqueShaderVariants[format][variant].shader);
		}
	}
	for (int variant = 0; variant < 3; variant++)
	{
		glDeleteProgram(s_gteUntexturedShaderVariants[variant].shader);
	}
#else
	glDeleteProgram(s_gteShader4.shader);
	glDeleteProgram(s_gteShader8.shader);
	glDeleteProgram(s_gteShader16.shader);
	glDeleteProgram(s_gteShader32Rgba.shader);
#endif
}

#ifdef __vita__
#define GPU_SAMPLE_TEXTURE_4BIT_FUNC                                                                                \
	"\tvec2 samplePSX(vec2 tc) {\n"                                                                               \
	"\t\tvec2 texel = floor(tc + vec2(0.5));\n"                                                             \
	"\t\tfloat texelX = texel.x;\n"                                                                            \
	"\t\tfloat lanePhase = fract(texelX * 0.25);\n"                                                        \
	"\t\tfloat highByte = step(0.5, lanePhase);\n"                                                        \
	"\t\tfloat highNibbleSel = step(0.5, fract(texelX * 0.5));\n"                                     \
	"\t\tvec2 packedPixel = v_page_clut.xy + vec2(floor(texelX * 0.25), texel.y);\n"                      \
	"\t\tvec2 packedRg = VRAM((packedPixel + vec2(0.5)) * c_VRAMTexel);\n"                              \
	"\t\tfloat packedByte = floor(mix(packedRg.x, packedRg.y, highByte) * 255.0 + 0.5);\n"                 \
	"\t\tfloat highNibble = floor(packedByte * (1.0 / 16.0));\n"                                     \
	"\t\tfloat paletteIndex = mix(packedByte - highNibble * 16.0, highNibble, highNibbleSel);\n"       \
	"\t\treturn VRAM((v_page_clut.zw + vec2(paletteIndex + 0.5, 0.5)) * c_VRAMTexel);\n"              \
	"\t}\n"
#else
#define GPU_SAMPLE_TEXTURE_4BIT_FUNC                                                              \
	"	// returns 16 bit colour\n"                                                                 \
	"	vec2 samplePSX(vec2 tc) {\n"                                                        \
	"		vec2 uv = (tc * vec2(0.25, 1.0) + v_page_clut.xy) * c_VRAMTexel;\n"                \
	"		vec2 comp = VRAM(uv);\n"                                                           \
	"		float lane = mod(floor(tc.x + 0.0001), 4.0);\n"                                 \
	"		float byteValue = floor(mix(comp.x, comp.y, step(1.5, lane)) * 255.0 + 0.5);\n" \
	"		float lowNibble = mod(byteValue, 16.0);\n"                                        \
	"		float highNibble = floor(byteValue * (1.0 / 16.0));\n"                         \
	"		float paletteIndex = mix(lowNibble, highNibble, mod(lane, 2.0));\n"              \
	"		vec2 clut_pos = v_page_clut.zw;\n"                                                 \
	"		clut_pos.x += paletteIndex * c_VRAMTexel.x;\n"                                   \
	"		return VRAM(clut_pos);\n"                                                          \
	"	}\n"
#endif

#ifdef __vita__
#define GPU_SAMPLE_TEXTURE_8BIT_FUNC                                                                                \
	"\tvec2 samplePSX(vec2 tc) {\n"                                                                               \
	"\t\tfloat highByte = step(0.5, fract(tc.x * 0.5 + 0.25));\n"                                      \
	"\t\tvec2 packedRg = VRAM((v_page_clut.xy + vec2(tc.x * 0.5 + 0.25, tc.y + 0.5)) * c_VRAMTexel);\n" \
	"\t\tfloat paletteIndex = floor(mix(packedRg.x, packedRg.y, highByte) * 255.0 + 0.5);\n"              \
	"\t\treturn VRAM((v_page_clut.zw + vec2(paletteIndex + 0.5, 0.5)) * c_VRAMTexel);\n"              \
	"\t}\n"
#else
#define GPU_SAMPLE_TEXTURE_8BIT_FUNC                                                              \
	"	// returns 16 bit colour\n"                                                                 \
	"	vec2 samplePSX(vec2 tc) {\n"                                                        \
	"		vec2 uv = (tc * vec2(0.5, 1.0) + v_page_clut.xy) * c_VRAMTexel;\n"                 \
	"		vec2 comp = VRAM(uv);\n"                                                           \
	"		float lane = mod(floor(tc.x + 0.0001), 2.0);\n"                                 \
	"		float paletteIndex = mix(comp.x, comp.y, lane) * 255.0;\n"                       \
	"		vec2 clut_pos = v_page_clut.zw;\n"                                                 \
	"		clut_pos.x += paletteIndex * c_VRAMTexel.x;\n"                                   \
	"		return VRAM(clut_pos);\n"                                                          \
	"	}\n"
#endif

#ifdef __vita__
#define GPU_SAMPLE_TEXTURE_16BIT_FUNC                                                       \
	"	vec2 samplePSX(vec2 tc) {\n"                                                          \
	"\t\treturn VRAM((v_page_clut.xy + tc + vec2(0.5)) * c_VRAMTexel);\n"             \
	"	}\n"
#else
#define GPU_SAMPLE_TEXTURE_16BIT_FUNC                    \
	"	vec2 samplePSX(vec2 tc) {\n"                       \
	"		vec2 uv = (tc + v_page_clut.xy) * c_VRAMTexel;\n" \
	"		return VRAM(uv);\n"                               \
	"	}\n"
#endif

#define GPU_FETCH_VRAM_FUNC                                        \
	"	const vec2 c_VRAMTexel = vec2(1.0 / 1024.0, 1.0 / 512.0);\n" \
	"	uniform sampler2D s_texture;\n"                              \
	"	vec2 VRAM(vec2 uv) { return texture2D(s_texture, uv).rg; }\n"

#ifdef __vita__
#define GPU_STP_PASS_FUNC                                                                                 \
	"	float texelVisible(vec2 rg) { return step(0.5 / 255.0, rg.x + rg.y); }\n"                       \
	"	float stpWeight(vec2 rg) { return step(0.5, rg.y); }\n"                                           \
	"	bool discardForSemiTransPass(float visible, float stpClass) {\n"                             \
	"		if (visible < 0.5) { return true; }\n"                                                        \
	"#ifdef PSX_PASS_NON_STP\n		if (stpClass >= 0.5) { return true; }\n#endif\n"                         \
	"#ifdef PSX_PASS_STP\n		if (stpClass < 0.5) { return true; }\n#endif\n"                             \
	"		return false;\n"                                                                                 \
	"	}\n"
#define GPU_SEMI_TRANS_UNIFORM
#else
#define GPU_STP_PASS_FUNC                                                                                 \
	"	float texelVisible(vec2 rg) { return float(rg.x + rg.y > 0.0); }\n"                               \
	"	float stpWeight(vec2 rg) { return step(0.5, rg.y); }\n"                                           \
	"	bool discardForSemiTransPass(float visible, float stpClass) {\n"                             \
	"		if (visible < 0.5) { return true; }\n"                                                       \
	"		if (psxSemiTransPass == 1 && stpClass >= 0.5) { return true; }\n"                         \
	"		if (psxSemiTransPass == 2 && stpClass < 0.5) { return true; }\n"                          \
	"		return false;\n"                                                                                 \
	"	}\n"
#define GPU_SEMI_TRANS_UNIFORM "\tuniform int psxSemiTransPass;\n"
#endif

#ifdef __vita__
#define GPU_DITHERING "\tvec4 dither(vec4 color) { return color; }\n"
#else
#define GPU_DITHERING                                             \
	"	const mat4 c_dither = mat4(\n"                              \
	"		-4.0,  +0.0,  -3.0,  +1.0,\n"                              \
	"		+2.0,  -2.0,  +3.0,  -1.0,\n"                              \
	"		-3.0,  +1.0,  -4.0,  +0.0,\n"                              \
	"		+3.0,  -1.0,  +2.0,  -2.0) / 255.0;\n"                     \
	"	vec4 dither(vec4 color) {\n"                                \
	"		ivec2 dc = ivec2(mod(floor(v_ditherCoord), 4.0));\n"       \
	"		color.xyz += vec3(c_dither[dc.x][dc.y] * v_texcoord.w);\n" \
	"		return color;\n"                                           \
	"	}\n"
#endif

#ifdef __vita__
#define GPU_PSX_COLOR_UNIFORM
#define GPU_PSX_COLOR_DECODE                                                                                   \
	"	vec4 decodePSX(vec2 rg) {\n"                                                                         \
	"		vec2 scaled = rg * vec2(255.0 / 32.0, 255.0 / 4.0);\n"                                  \
	"		vec2 whole = floor(scaled + vec2(0.001));\n"                                                \
	"		vec2 part = scaled - whole;\n"                                                               \
	"		float blue = whole.y - step(32.0, whole.y) * 32.0;\n"                                      \
	"		vec3 color5 = vec3(part.x * 32.0, whole.x + part.y * 32.0, blue);\n"                       \
	"		return vec4(color5 * (8.0 / 255.0), 1.0);\n"                                            \
	"	}\n"
#define GPU_PSX_FRAGMENT_OUTPUT                                                                                \
	"		gl_FragColor.rgb = color.rgb * v_color.rgb;\n"                                                  \
	"		gl_FragColor.a = max(psxDrawMaskSet, psxTextureOutputStp * sampledStp);\n"
#else
#define GPU_PSX_COLOR_UNIFORM "\tuniform sampler2D s_rgLut;\n"
#define GPU_PSX_COLOR_DECODE                                                                                   \
	"	const vec2 c_LUTTexel = vec2(1.0 / 256.0, 1.0 / 256.0);\n"                                      \
	"	vec4 decodePSX(vec2 rg) { return texture2D(s_rgLut, rg - c_LUTTexel * 0.0001); }\n"
#define GPU_PSX_FRAGMENT_OUTPUT                                                                                \
	"		gl_FragColor = dither(color * v_color);\n"                                                   \
	"		gl_FragColor.a = max(psxDrawMaskSet, psxTextureOutputStp * sampledStp);\n"
#endif

#ifdef __vita__
// This is used to simulate GL_CONSTANT_COLOR blending since sceGxm has no equivalents for them
#define GPU_PSX_BLEND_APPLY                                                                        \
	"#ifdef PSX_BLEND_AVERAGE\n		gl_FragColor.a = (127.0 + gl_FragColor.a) / 255.0;\n#endif\n" \
	"#ifdef PSX_BLEND_QUARTER\n		gl_FragColor.rgb *= 0.25;\n#endif\n"
#else
#define GPU_PSX_BLEND_APPLY
#endif

#ifdef __vita__
#define GPU_TEXTURE_SAMPLE_MAIN "		vec4 color = nearestTextureSample(v_texcoord.xy);\n"
#else
#define GPU_TEXTURE_SAMPLE_MAIN \
	"		vec4 color = (bilinearFilter > 0) ? bilinearTextureSample(v_texcoord.xy) : nearestTextureSample(v_texcoord.xy);\n"
#endif

#define GPU_FRAGMENT_SAMPLE_SHADER(bit)                                                                                                               \
	GPU_FETCH_VRAM_FUNC                                                                                                                               \
	GPU_SAMPLE_TEXTURE_##bit##BIT_FUNC                                                                                                                \
	    GPU_PSX_COLOR_UNIFORM                                                                                                                         \
	    "#ifndef VITA_NEAREST_ONLY\n\tuniform int bilinearFilter;\n#endif\n"                                                                                 \
	    GPU_SEMI_TRANS_UNIFORM                                                                                                                        \
	    "	uniform float psxDrawMaskSet;\n"                                                                                                            \
	    "	uniform float psxTextureOutputStp;\n"                                                                                                       \
	    "	float sampledStp = 0.0;\n"                                                                                                                  \
	    GPU_PSX_COLOR_DECODE GPU_STP_PASS_FUNC "#ifndef VITA_NEAREST_ONLY\n\tvec4 bilinearTextureSample(vec2 P) {\n"                          \
	    "		vec2 _frac = fract(P);\n"                                                                                                                   \
	    "		vec2 pixel = floor(P);\n"                                                                                                                  \
	    "		vec2 C11 = samplePSX(pixel);\n"                                                                                                            \
	    "		vec2 C21 = samplePSX(pixel + vec2(1.0, 0.0));\n"                                                                                           \
	    "		vec2 C12 = samplePSX(pixel + vec2(0.0, 1.0));\n"                                                                                           \
	    "		vec2 C22 = samplePSX(pixel + vec2(1.0, 1.0));\n"                                                                                           \
	    "		float v11 = texelVisible(C11);\n"                                                                                                          \
	    "		float v21 = texelVisible(C21);\n"                                                                                                          \
	    "		float v12 = texelVisible(C12);\n"                                                                                                          \
	    "		float v22 = texelVisible(C22);\n"                                                                                                          \
	    "		float s11 = v11 * stpWeight(C11);\n"                                                                                                       \
	    "		float s21 = v21 * stpWeight(C21);\n"                                                                                                       \
	    "		float s12 = v12 * stpWeight(C12);\n"                                                                                                       \
	    "		float s22 = v22 * stpWeight(C22);\n"                                                                                                       \
	    "		float n11 = v11 - s11;\n"                                                                                                                  \
	    "		float n21 = v21 - s21;\n"                                                                                                                  \
	    "		float n12 = v12 - s12;\n"                                                                                                                  \
	    "		float n22 = v22 - s22;\n"                                                                                                                  \
	    "		float ax1 = mix(v11, v21, _frac.x);\n"                                                                                                      \
	    "		float ax2 = mix(v12, v22, _frac.x);\n"                                                                                                      \
	    "		float axm = mix(ax1, ax2, _frac.y);\n"                                                                                                      \
	    "		float sx1 = mix(s11, s21, _frac.x);\n"                                                                                                      \
	    "		float sx2 = mix(s12, s22, _frac.x);\n"                                                                                                      \
	    "		float stp = mix(sx1, sx2, _frac.y);\n"                                                                                                      \
	    "		float nx1 = mix(n11, n21, _frac.x);\n"                                                                                                      \
	    "		float nx2 = mix(n12, n22, _frac.x);\n"                                                                                                      \
	    "		float nonStp = mix(nx1, nx2, _frac.y);\n"                                                                                                   \
	    "		vec2 rg = mix(mix(C11, C21, _frac.x), mix(C12, C22, _frac.x), _frac.y);\n"                                                                    \
	    "		float stpClass = step(nonStp, stp);\n"                                                                                                    \
	    "		sampledStp = stpClass;\n"                                                                                                                  \
	    "		if(discardForSemiTransPass(axm, stpClass)) { discard; }\n"                                                                                 \
	    "		vec4 x1 = mix(decodePSX(C11), decodePSX(C21), _frac.x);\n"                                                                              \
	    "		vec4 x2 = mix(decodePSX(C12), decodePSX(C22), _frac.x);\n"                                                                              \
	    "		vec4 t = mix(x1, x2, _frac.y);\n"                                                                                                           \
	    "		return t;\n"                                                                                                                               \
	    "	}\n#endif\n"                                                                                                                                \
	    "	vec4 nearestTextureSample(vec2 P) {\n"                                                                                                      \
	    "		vec2 rg = samplePSX(P);\n"                                                                                                                 \
	    "#ifdef PSX_FULLY_OPAQUE\n"                                                                                                                      \
	    "		sampledStp = stpWeight(rg);\n"                                                                                                            \
	    "#else\n"                                                                                                                                       \
	    "		float visible = texelVisible(rg);\n"                                                                                                       \
	    "		sampledStp = visible * stpWeight(rg);\n"                                                                                                   \
	    "		if(discardForSemiTransPass(visible, sampledStp)) { discard; }\n"                                                                         \
	    "#endif\n"                                                                                                                                      \
	    "		vec4 t = decodePSX(rg);\n"                                                                                                                 \
	    "		return t;\n"                                                                                                                               \
	    "	}\n"                                                                                                                                        \
	    "	void main() {\n"                                                                                                                            \
	    GPU_TEXTURE_SAMPLE_MAIN                                                                                                                         \
	    GPU_PSX_FRAGMENT_OUTPUT                                                                                                                         \
	    GPU_PSX_BLEND_APPLY                                                                                                                            \
	    "	}\n"

global_variable const char *gpu_shader_common = "	centroid varying vec4 v_texcoord;\n"
                                                "	varying vec4 v_color;\n"
                                                "	varying vec4 v_page_clut;\n"
                                                "	varying vec2 v_ditherCoord;\n"
                                                "	varying float v_z;\n";

const char *gte_shader_4 = GPU_FRAGMENT_SAMPLE_SHADER(4);
const char *gte_shader_8 = GPU_FRAGMENT_SAMPLE_SHADER(8);
const char *gte_shader_16 = GPU_FRAGMENT_SAMPLE_SHADER(16);
#ifdef __vita__
const char *gte_shader_untextured = "\tuniform float psxDrawMaskSet;\n"
                                    "\tvoid main() {\n"
                                    "\t\tgl_FragColor.rgb = v_color.rgb * (248.0 / 255.0);\n"
                                    "\t\tgl_FragColor.a = psxDrawMaskSet;\n"
                                    GPU_PSX_BLEND_APPLY
                                    "\t}\n";

#define GPU_RGBA_FRAGMENT_OUTPUT                                    \
	"		gl_FragColor.rgb = color.rgb * v_color.rgb;\n"             \
	"		gl_FragColor.a = psxDrawMaskSet;\n"
#else
#define GPU_RGBA_FRAGMENT_OUTPUT                                    \
	"		gl_FragColor = dither(color * v_color);\n"              \
	"		gl_FragColor.a = psxDrawMaskSet;\n"
#endif

const char *gte_shader_32_rgba = "	uniform sampler2D s_texture;\n"
                                 "	uniform float psxDrawMaskSet;\n"
                                 "	uniform vec2 texelSize;\n"
                                 "	void main() {\n"
                                 "		vec2 tc = v_texcoord.xy * texelSize + texelSize * 0.5;\n"
                                 "		vec4 color = texture2D(s_texture, tc);\n"
                                 GPU_RGBA_FRAGMENT_OUTPUT
                                 GPU_PSX_BLEND_APPLY
                                 "	}\n";

#define GTE_PERSPECTIVE_CORRECTION "	gl_Position = Projection * vec4(a_position.xy, 0.0, 1.0);\n"

#ifdef __vita__
#define GTE_PAGE_CLUT_SETUP                                                                                     \
	"\t\tv_page_clut.x = fract(a_position.z / 16.0) * 1024.0;\n"                                           \
	"\t\tv_page_clut.y = floor(a_position.z / 16.0) * 256.0;\n"                                            \
	"\t\tv_page_clut.z = fract(a_position.w / 64.0) * 1024.0;\n"                                           \
	"\t\tv_page_clut.w = floor(a_position.w / 64.0);\n"
#else
#define GTE_PAGE_CLUT_SETUP                                                                                     \
	"\t\tv_page_clut.x = fract(a_position.z / 16.0) * 1024.0;\n"                                           \
	"\t\tv_page_clut.y = floor(a_position.z / 16.0) * 256.0;\n"                                            \
	"\t\tv_page_clut.z = fract(a_position.w / 64.0);\n"                                                    \
	"\t\tv_page_clut.w = floor(a_position.w / 64.0) / 512.0;\n"                                            \
	"\t\tv_page_clut.xy += c_UVFudge;\n"                                                                   \
	"\t\tv_page_clut.zw += c_UVFudge;\n"
#endif

#define GTE_VERTEX_SHADER                                                                                          \
	"	attribute vec4 a_position;\n"                                                                                \
	"	attribute vec4 a_texcoord; // uv, color multiplier, dither\n"                                                \
	"	attribute vec4 a_color;\n"                                                                                   \
	"	attribute vec4 a_extra; // texcoord.xy ofs, unused.xy\n"                                                     \
	"	uniform mat4 Projection;\n"                                                                                  \
	"	const vec2 c_UVFudge = vec2(0.00025, 0.00025);\n"                                                            \
	"	void main() {\n"                                                                                             \
	"		v_ditherCoord = a_position.xy;\n"                                                                           \
	"		v_texcoord = a_texcoord;\n"                                                                                 \
	"		v_texcoord.xy += a_extra.xy * 0.5;\n"                                                                       \
	"		v_color = a_color;\n"                                                                                       \
	"		v_color.xyz *= a_texcoord.z;\n"                                                                             \
	GTE_PAGE_CLUT_SETUP GTE_PERSPECTIVE_CORRECTION "		v_z = (gl_Position.z - 40.0) * 0.005;\n" \
	"	}\n"

internal int NativeRenderer_Shader_CheckShaderStatus(GLuint shader)
{
	char info[1024];
	GLint result;

	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

	if (result == GL_TRUE)
	{
		return 1;
	}

	glGetShaderInfoLog(shader, sizeof(info), NULL, info);
	if (info[0] && strlen(info) > 8)
	{
		NATIVE_RENDERER_ERROR("%s\n", info);
		assert(0);
	}

	return 0;
}

internal int NativeRenderer_Shader_CheckProgramStatus(GLuint program)
{
	char info[1024];
	GLint result;

	glGetProgramiv(program, GL_LINK_STATUS, &result);

	if (result == GL_TRUE)
	{
		return 1;
	}

	glGetProgramInfoLog(program, sizeof(info), NULL, info);
	if (info[0] && strlen(info) > 8)
	{
		NATIVE_RENDERER_ERROR("%s\n", info);
		assert(0);
	}

	return 0;
}

internal ShaderID NativeRenderer_Shader_Compile(const char *source, bool isPsxShader, const char *fragmentDefines)
{
	const char *GLSL_HEADER_VERT = "	#version 140\n"
	                               "	precision lowp  int;\n"
	                               "	precision highp float;\n"
#ifndef __vita__								   
	                               "	#define varying   out\n"
	                               "	#define attribute in\n"
	                               "	#define texture2D texture\n"
#endif
								   ;

	const char *GLSL_HEADER_FRAG = "	#version 140\n"
	                               "	precision lowp  int;\n"
	                               "	precision highp float;\n"
#ifndef __vita__
	                               "	#define varying     in\n"
	                               "	#define texture2D   texture\n"
	                               "	out vec4 fragColor;\n"
#endif
								   ;

	char extra_vs_defines[1024];
	char extra_fs_defines[1024];
	extra_vs_defines[0] = 0;
	extra_fs_defines[0] = 0;

	strcat(extra_vs_defines, "#define VERTEX\n");
	strcat(extra_fs_defines, "#define FRAGMENT\n");
	if (fragmentDefines != NULL)
	{
		strcat(extra_fs_defines, fragmentDefines);
	}
#ifdef __vita__
	strcat(extra_fs_defines, "#define VITA_NEAREST_ONLY\n");
#endif
	if (g_cfg_bilinearFiltering)
	{
		strcat(extra_fs_defines, "#define BILINEAR_FILTER\n");
	}

	const char *vs_list_psx[] = {GLSL_HEADER_VERT, extra_vs_defines, gpu_shader_common, GTE_VERTEX_SHADER};
	const char *fs_list_psx[] = {GLSL_HEADER_FRAG, extra_fs_defines, gpu_shader_common, GPU_DITHERING, source};
	const char *vs_list_src[] = {
	    GLSL_HEADER_VERT,
	    extra_vs_defines,
	    source,
	};
	const char *fs_list_src[] = {GLSL_HEADER_FRAG, extra_fs_defines, source};

	const char **vs_list = isPsxShader ? vs_list_psx : vs_list_src;
	const char **fs_list = isPsxShader ? fs_list_psx : fs_list_src;
	const int vs_list_cnt = isPsxShader ? 4 : 3;
	const int fs_list_cnt = isPsxShader ? 5 : 3;

	GLuint program = glCreateProgram();

	{
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, vs_list_cnt, vs_list, NULL);
		glCompileShader(vertexShader);

		if (NativeRenderer_Shader_CheckShaderStatus(vertexShader) == 0)
		{
			NATIVE_RENDERER_ERROR("%s\n", "Failed to compile Vertex Shader!");
		}

		glAttachShader(program, vertexShader);
		glDeleteShader(vertexShader);
	}

	{
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, fs_list_cnt, fs_list, NULL);
		glCompileShader(fragmentShader);

		if (NativeRenderer_Shader_CheckShaderStatus(fragmentShader) == 0)
		{
			NATIVE_RENDERER_ERROR("%s\n", "Failed to compile Fragment Shader!");
		}

		glAttachShader(program, fragmentShader);
		glDeleteShader(fragmentShader);
	}

	glBindAttribLocation(program, a_position, "a_position");
	glBindAttribLocation(program, a_texcoord, "a_texcoord");
	glBindAttribLocation(program, a_color, "a_color");
	glBindAttribLocation(program, a_extra, "a_extra");

	glLinkProgram(program);
	if (NativeRenderer_Shader_CheckProgramStatus(program) == 0)
	{
		NATIVE_RENDERER_ERROR("%s\n", "Failed to link Shader!");
	}

	GLint textureSampler = 0;
	GLint presentLutSampler = 2;
	glUseProgram(program);
	glUniform1iv(glGetUniformLocation(program, "s_texture"), 1, &textureSampler);
#ifndef __vita__
	GLint lutSampler = 1;
	glUniform1iv(glGetUniformLocation(program, "s_rgLut"), 1, &lutSampler);
#endif
	glUniform1iv(glGetUniformLocation(program, "s_presentLut"), 1, &presentLutSampler);
	glUseProgram(0);

	return program;
}

//--------------------------------------------------------------------------------------------

internal void NativeRenderer_GenerateCommonTextures(void)
{
	u32 whitePixelData = 0xFFFFFFFF;

	glGenTextures(1, &s_whiteTexture);
	{
		glBindTexture(GL_TEXTURE_2D, s_whiteTexture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &whitePixelData);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

#ifndef __vita__
	glGenTextures(1, &s_rgLutTexture);
	{
		// Texture unit 1 is reserved for the immutable PSX color lookup table.
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, s_rgLutTexture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, LUT_WIDTH, LUT_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, &rgLUT);

		glActiveTexture(GL_TEXTURE0);
	}
#endif

#ifdef __vita__
	for (u16 y = 0; y < LUT_HEIGHT; y++)
	{
		u8 *row = rgLUT + y * (LUT_WIDTH * 4);
		for (u16 x = 0; x < LUT_WIDTH; x++)
		{
			const u16 c = (y << 8) | x;
			const u8 r5 = (u8)(c & 31);
			const u8 g5 = (u8)((c >> 5) & 31);
			const u8 b5 = (u8)((c >> 10) & 31);
			u8 *pixel = row + x * 4;
			pixel[0] = (u8)((r5 << 3) | (r5 >> 2));
			pixel[1] = (u8)((g5 << 3) | (g5 >> 2));
			pixel[2] = (u8)((b5 << 3) | (b5 >> 2));
			pixel[3] = (c & 0x8000) ? 255 : 0;
		}
	}

	glGenTextures(1, &s_presentLutTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, s_presentLutTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, LUT_WIDTH, LUT_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, &rgLUT);
	glActiveTexture(GL_TEXTURE0);
#endif
}

internal void NativeRenderer_CompilePSXShader(GTEShader *sh, const char *source, const char *fragmentDefines)
{
	sh->shader = NativeRenderer_Shader_Compile(source, true, fragmentDefines);

	sh->bilinearFilterLoc = glGetUniformLocation(sh->shader, "bilinearFilter");
	sh->projectionLoc = glGetUniformLocation(sh->shader, "Projection");
	sh->texelSizeLoc = glGetUniformLocation(sh->shader, "texelSize");
	sh->texLoc = glGetUniformLocation(sh->shader, "s_texture");
	sh->lutLoc = glGetUniformLocation(sh->shader, "s_rgLut");
#ifndef __vita__
	sh->psxSemiTransPassLoc = glGetUniformLocation(sh->shader, "psxSemiTransPass");
#endif
	sh->psxDrawMaskSetLoc = glGetUniformLocation(sh->shader, "psxDrawMaskSet");
	sh->psxTextureOutputStpLoc = glGetUniformLocation(sh->shader, "psxTextureOutputStp");
}

internal void NativeRenderer_InitialisePSXShaders(void)
{
#ifdef __vita__
	local_persist const char *variantDefines[NATIVE_PSX_SHADER_VARIANT_COUNT] = {
	    "",
	    "#define PSX_BLEND_AVERAGE\n",
	    "#define PSX_BLEND_QUARTER\n",
	    "#define PSX_PASS_NON_STP\n",
	    "#define PSX_PASS_STP\n",
	    "#define PSX_PASS_STP\n#define PSX_BLEND_AVERAGE\n",
	    "#define PSX_PASS_STP\n#define PSX_BLEND_QUARTER\n",
	};
	local_persist const char *fullyOpaqueVariantDefines[3] = {
	    "#define PSX_FULLY_OPAQUE\n",
	    "#define PSX_FULLY_OPAQUE\n#define PSX_BLEND_AVERAGE\n",
	    "#define PSX_FULLY_OPAQUE\n#define PSX_BLEND_QUARTER\n",
	};
	const char *sources[4] = {gte_shader_4, gte_shader_8, gte_shader_16, gte_shader_32_rgba};

	for (int format = TF_4_BIT; format <= TF_32_BIT_RGBA; format++)
	{
		const int variantCount = format == TF_32_BIT_RGBA ? 3 : NATIVE_PSX_SHADER_VARIANT_COUNT;
		for (int variant = 0; variant < variantCount; variant++)
		{
			NativeRenderer_CompilePSXShader(&s_gteShaderVariants[format][variant], sources[format], variantDefines[variant]);
		}
	}
	for (int format = TF_4_BIT; format <= TF_8_BIT; format++)
	{
		for (int variant = 0; variant < 3; variant++)
		{
			NativeRenderer_CompilePSXShader(&s_gteFullyOpaqueShaderVariants[format][variant], sources[format],
			                                fullyOpaqueVariantDefines[variant]);
		}
	}
	for (int variant = 0; variant < 3; variant++)
	{
		NativeRenderer_CompilePSXShader(&s_gteUntexturedShaderVariants[variant], gte_shader_untextured, variantDefines[variant]);
	}
#else
	NativeRenderer_CompilePSXShader(&s_gteShader4, gte_shader_4, NULL);
	NativeRenderer_CompilePSXShader(&s_gteShader8, gte_shader_8, NULL);
	NativeRenderer_CompilePSXShader(&s_gteShader16, gte_shader_16, NULL);
	NativeRenderer_CompilePSXShader(&s_gteShader32Rgba, gte_shader_32_rgba, NULL);
#endif
}

#ifdef __vita__
global_variable const char *ctr_pack_shader = "#ifdef VERTEX\n"
                                              "attribute vec2 a_position;\n"
                                              "varying vec2 v_uv;\n"
                                              "uniform int flipY;\n"
                                              "void main() {\n"
                                              "    v_uv = a_position * 0.5 + 0.5;\n"
                                              "    if (flipY != 0) { v_uv.y = 1.0 - v_uv.y; }\n"
                                              "    gl_Position = vec4(a_position, 0.0, 1.0);\n"
                                              "}\n"
                                              "#endif\n"
                                              "#ifdef FRAGMENT\n"
                                              "varying vec2 v_uv;\n"
                                              "uniform sampler2D s_src;\n"
                                              "void main() {\n"
                                              "    vec4 c = texture2D(s_src, v_uv);\n"
                                              "    vec3 color5 = floor(c.rgb * (255.0 / 8.0) + 0.001);\n"
                                              "    float lowByte = color5.r + mod(color5.g, 8.0) * 32.0;\n"
                                              "    float highByte = floor(color5.g / 8.0) + color5.b * 4.0 + step(0.5, c.a) * 128.0;\n"
                                              "    gl_FragColor = vec4(lowByte, highByte, 0.0, 0.0) / 255.0;\n"
                                              "}\n"
                                              "#endif\n";

global_variable const char *ctr_present_vram_shader = "#ifdef VERTEX\n"
                                                      "attribute vec2 a_position;\n"
                                                      "varying vec2 v_uv;\n"
                                                      "uniform vec4 sourceRect;\n"
                                                      "void main() {\n"
                                                      "    vec2 screenUV = a_position * 0.5 + 0.5;\n"
                                                      "    vec2 sourcePixel = sourceRect.xy + vec2(screenUV.x, 1.0 - screenUV.y) * sourceRect.zw;\n"
                                                      "    v_uv = sourcePixel / vec2(1024.0, 512.0);\n"
                                                      "    gl_Position = vec4(a_position, 0.0, 1.0);\n"
                                                      "}\n"
                                                      "#endif\n"
                                                      "#ifdef FRAGMENT\n"
                                                      "varying vec2 v_uv;\n"
                                                      "uniform sampler2D s_texture;\n"
                                                      "uniform sampler2D s_presentLut;\n"
                                                      "const vec2 c_LUTTexel = vec2(1.0 / 256.0, 1.0 / 256.0);\n"
                                                      "void main() {\n"
                                                      "    vec2 packedRg = texture2D(s_texture, v_uv).rg;\n"
                                                      "    gl_FragColor = texture2D(s_presentLut, packedRg - c_LUTTexel * 0.0001);\n"
                                                      "}\n"
                                                      "#endif\n";
#else
// NOTE(aalhendi): GPU VRAM pack. Samples an RGBA render texture and writes PS1
// 5:5:5:1 pixels into RG8 VRAM, low byte in R and high byte in G. NEAREST
// sampling and integer channel shifts preserve the packed PS1 pixel value.
global_variable const char *ctr_pack_shader = "#ifdef VERTEX\n"
                                              "attribute vec2 a_position;\n"
                                              "varying vec2 v_uv;\n"
                                              "uniform int flipY;\n"
                                              "void main() {\n"
                                              "	v_uv = a_position * 0.5 + 0.5;\n"
                                              "	if (flipY != 0) { v_uv.y = 1.0 - v_uv.y; }\n"
                                              "	gl_Position = vec4(a_position, 0.0, 1.0);\n"
                                              "}\n"
                                              "#endif\n"
                                              "#ifdef FRAGMENT\n"
                                              "varying vec2 v_uv;\n"
                                              "uniform sampler2D s_src;\n"
                                              "void main() {\n"
                                              "	ivec4 c = ivec4(texture2D(s_src, v_uv) * 255.0 + 0.5);\n"
                                              "	int px16 = (c.r >> 3) | ((c.g >> 3) << 5) | ((c.b >> 3) << 10) | ((c.a >> 7) << 15);\n"
                                              "	gl_FragColor = vec4(float(px16 & 0xFF) / 255.0, float((px16 >> 8) & 0xFF) / 255.0, 0.0, 0.0);\n"
                                              "}\n"
                                              "#endif\n";

// NOTE(aalhendi): Expand packed VRAM without losing bit 15. Internal render
// targets carry that PS1 STP/mask bit in alpha so packing them is lossless.
global_variable const char *ctr_present_vram_shader = "#ifdef VERTEX\n"
                                                      "attribute vec2 a_position;\n"
                                                      "varying vec2 v_uv;\n"
                                                      "uniform vec4 sourceRect;\n"
                                                      "void main() {\n"
                                                      "\tvec2 screenUV = a_position * 0.5 + 0.5;\n"
                                                      "\tvec2 sourcePixel = sourceRect.xy + vec2(screenUV.x, 1.0 - screenUV.y) * sourceRect.zw;\n"
                                                      "\tv_uv = sourcePixel / vec2(1024.0, 512.0);\n"
                                                      "\tgl_Position = vec4(a_position, 0.0, 1.0);\n"
                                                      "}\n"
                                                      "#endif\n"
                                                      "#ifdef FRAGMENT\n"
                                                      "varying vec2 v_uv;\n"
                                                      "uniform sampler2D s_texture;\n"
                                                      "void main() {\n"
                                                      "\tivec2 packedBytes = ivec2(texture2D(s_texture, v_uv).rg * 255.0 + 0.5);\n"
                                                      "\tint pixel = packedBytes.r | (packedBytes.g << 8);\n"
                                                      "\tivec3 color5 = ivec3(pixel & 31, (pixel >> 5) & 31, (pixel >> 10) & 31);\n"
                                                      "\tivec3 color8 = (color5 << 3) | (color5 >> 2);\n"
                                                      "\tfloat stp = float((pixel >> 15) & 1);\n"
                                                      "\tgl_FragColor = vec4(vec3(color8) / 255.0, stp);\n"
                                                      "}\n"
                                                      "#endif\n";
#endif

global_variable const char *ctr_present_rgba_shader = "#ifdef VERTEX\n"
                                                       "attribute vec2 a_position;\n"
                                                       "varying vec2 v_uv;\n"
                                                       "void main() {\n"
                                                       "    v_uv = a_position * 0.5 + 0.5;\n"
                                                       "    gl_Position = vec4(a_position, 0.0, 1.0);\n"
                                                       "}\n"
                                                       "#endif\n"
                                                       "#ifdef FRAGMENT\n"
                                                       "varying vec2 v_uv;\n"
                                                       "uniform sampler2D s_src;\n"
	                                                   "uniform float flipY;\n"
                                                       "void main() {\n"
	                                                   "    gl_FragColor = texture2D(s_src, vec2(v_uv.x, mix(v_uv.y, 1.0 - v_uv.y, flipY)));\n"
                                                       "}\n"
                                                       "#endif\n";

internal void NativeRenderer_InitVRAMPipelines(void)
{
	local_persist const float quad[12] = {-1.f, -1.f, -1.f, 1.f, 1.f, 1.f, -1.f, -1.f, 1.f, 1.f, 1.f, -1.f};

	s_packShader = NativeRenderer_Shader_Compile(ctr_pack_shader, false, NULL);
	glUseProgram(s_packShader);
	const GLint packSrcLoc = glGetUniformLocation(s_packShader, "s_src");
	s_packFlipYLoc = glGetUniformLocation(s_packShader, "flipY");
	glUniform1i(packSrcLoc, 0);
	glUseProgram(0);

	s_presentVramShader = NativeRenderer_Shader_Compile(ctr_present_vram_shader, false, NULL);
	s_presentVramSourceRectLoc = glGetUniformLocation(s_presentVramShader, "sourceRect");

	s_presentRgbaShader = NativeRenderer_Shader_Compile(ctr_present_rgba_shader, false, NULL);
	glUseProgram(s_presentRgbaShader);
	const GLint presentRgbaSrcLoc = glGetUniformLocation(s_presentRgbaShader, "s_src");
	s_presentRgbaFlipYLoc = glGetUniformLocation(s_presentRgbaShader, "flipY");
	glUniform1i(presentRgbaSrcLoc, 0);
	glUniform1f(s_presentRgbaFlipYLoc, 0.0f);
	glUseProgram(0);

	glGenVertexArrays(1, &s_vramQuadVAO);
	glGenBuffers(1, &s_vramQuadVBO);
	glBindVertexArray(s_vramQuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, s_vramQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
	glEnableVertexAttribArray(a_position);
	glVertexAttribPointer(a_position, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

internal void NativeRenderer_InitRG8LUT(void)
{
	for (u16 y = 0; y < LUT_HEIGHT; y++)
	{
		u8 *row = rgLUT + y * (LUT_WIDTH * 4);
		for (u16 x = 0; x < LUT_WIDTH; x++)
		{
			const u16 c = (y << 8) | x;
			u8 *pixel = row + x * 4;
			pixel[0] = (u8)((c & 31) << 3);
			pixel[1] = (u8)(((c >> 5) & 31) << 3);
			pixel[2] = (u8)(((c >> 10) & 31) << 3);
			pixel[3] = (u8)(((c >> 15) & 1) << 7);
		}
	}
}

int NativeRenderer_InitialisePSX(void)
{
	SDL_memset(s_vram.cpuPixels, 0, sizeof(s_vram.cpuPixels));
	s_vram.cpuDirtyRects[0].x = 0;
	s_vram.cpuDirtyRects[0].y = 0;
	s_vram.cpuDirtyRects[0].w = VRAM_WIDTH;
	s_vram.cpuDirtyRects[0].h = VRAM_HEIGHT;
	s_vram.cpuDirtyRectCount = 1;
	SDL_memset(s_vram.gpuNewerTiles, 0, sizeof(s_vram.gpuNewerTiles));
	NativeRenderer_InitRG8LUT();
	NativeRenderer_GenerateCommonTextures();
	NativeRenderer_InitialisePSXShaders();
	NativeRenderer_InitVRAMPipelines();

#if defined(CTR_INTERNAL)
	GLint glMajor = 0;
	GLint glMinor = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &glMajor);
	glGetIntegerv(GL_MINOR_VERSION, &glMinor);
	s_gpuTimerSupported = (glMajor > 3) || ((glMajor == 3) && (glMinor >= 3)) || SDL_GL_ExtensionSupported("GL_ARB_timer_query");
	if (s_gpuTimerSupported)
	{
		GLuint queryIds[NATIVE_GPU_TIMER_QUERY_COUNT];
		glGenQueries(NATIVE_GPU_TIMER_QUERY_COUNT, queryIds);
		for (s32 i = 0; i < NATIVE_GPU_TIMER_QUERY_COUNT; i++)
		{
			s_gpuTimerQueries[i].id = queryIds[i];
		}
	}
#endif

	glDepthFunc(GL_LEQUAL);
	glEnable(GL_STENCIL_TEST);
#ifndef __vita__
	glBlendColor(0.5f, 0.5f, 0.5f, 0.25f);
#endif

	// Main and offscreen draws share one explicit render-target contract. The
	// main target stays at CTR's logical display size; host scaling is deferred
	// to presentation.
	NativeRenderer_InitRenderTarget(&s_mainRenderTarget);
	NativeRenderer_InitRenderTarget(&s_offscreenRenderTarget);

	// gen VRAM texture (single, persistent - mirrors PS1's single 1MB VRAM)
	{
		glGenTextures(1, &s_vram.texture);

		glBindTexture(GL_TEXTURE_2D, s_vram.texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		// set storage size
		glTexImage2D(GL_TEXTURE_2D, 0, VRAM_INTERNAL_FORMAT, VRAM_WIDTH, VRAM_HEIGHT, 0, VRAM_FORMAT, GL_UNSIGNED_BYTE, NULL);

		glBindTexture(GL_TEXTURE_2D, 0);

		// VRAM framebuffer for offscreen blitting to VRAM
		glGenFramebuffers(1, &s_glVramFramebuffer);
		{
			glBindFramebuffer(GL_FRAMEBUFFER, s_glVramFramebuffer);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_vram.texture, 0);
#ifndef __vita__
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
#else
			GLuint depthstencil;
			glGenRenderbuffers(1, &depthstencil);
			glBindRenderbuffer(GL_RENDERBUFFER, depthstencil);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, VRAM_WIDTH, VRAM_HEIGHT);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthstencil);
#endif

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}

	// gen vertex buffer and index buffer
	{
		int i;

		glGenBuffers(MAX_NUM_VERTEX_BUFFERS, s_glVertexBuffer);
		glGenVertexArrays(MAX_NUM_VERTEX_BUFFERS, s_glVertexArray);
		for (i = 0; i < MAX_NUM_VERTEX_BUFFERS; i++)
		{
			glBindVertexArray(s_glVertexArray[i]);
			glBindBuffer(GL_ARRAY_BUFFER, s_glVertexBuffer[i]);
#ifdef __vita__
			// Initialise vitaGL's VBO metadata. Each submitted batch later
			// replaces this pointer with mapped scratch storage, without a copy.
			glBufferData(GL_ARRAY_BUFFER, sizeof(GrVertex), NULL, GL_STREAM_DRAW);
#else
			glBufferData(GL_ARRAY_BUFFER, sizeof(GrVertex) * MAX_VERTEX_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW);
#endif
			glEnableVertexAttribArray(a_position);
			glEnableVertexAttribArray(a_texcoord);
			glEnableVertexAttribArray(a_color);
			glEnableVertexAttribArray(a_extra);

			glVertexAttribPointer(a_position, 4, GL_SHORT, GL_FALSE, sizeof(GrVertex), &((GrVertex *)NULL)->x);
			glVertexAttribPointer(a_texcoord, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(GrVertex), &((GrVertex *)NULL)->u);
			glVertexAttribPointer(a_color, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(GrVertex), &((GrVertex *)NULL)->r);
			glVertexAttribPointer(a_extra, 4, GL_BYTE, GL_FALSE, sizeof(GrVertex), &((GrVertex *)NULL)->tcx);
		}
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	NativeRenderer_ResetDevice();

	return 1;
}

internal void NativeRenderer_Ortho2D(float left, float right, float bottom, float top, float znear, float zfar)
{
	float a = 2.0f / (right - left);
	float b = 2.0f / (top - bottom);
	float c = 2.0f / (znear - zfar);

	float x = (left + right) / (left - right);
	float y = (bottom + top) / (bottom - top);

	// -1..1
	float z = (znear + zfar) / (znear - zfar);

	float ortho[16] = {a, 0, 0, 0, 0, b, 0, 0, 0, 0, c, 0, x, y, z, 1};

	glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, ortho);
}

void NativeRenderer_SetupClipMode(const RECT16 *rect, const DISPENV *displayEnv, int enable)
{
	if ((displayEnv->disp.w <= 0) || (displayEnv->disp.h <= 0))
	{
		NativeRenderer_SetScissorState(0);
		return;
	}

	if ((rect->w <= 0) || (rect->h <= 0))
	{
		// NOTE(aalhendi): Retail draw-area commands define inclusive corners.
		// Collapsed areas clip all pixels; GL scissor rejects negative sizes.
		NativeRenderer_SetScissorState(enable != 0);
		if (enable)
		{
			glScissor(0, 0, 0, 0);
		}
		return;
	}

	// [A] isinterlaced dirty hack for widescreen
	const bool scissorOn = enable && (displayEnv->isinter || (rect->x - displayEnv->disp.x > 0 || rect->y - displayEnv->disp.y > 0 ||
	                                                          rect->w < displayEnv->disp.w || rect->h < displayEnv->disp.h));

	NativeRenderer_SetScissorState(scissorOn);

	if (!scissorOn)
	{
		return;
	}

	const float emuScreenAspect = 1.0f;

	const float psxScreenWInv = 1.0f / (float)displayEnv->disp.w;
	const float psxScreenHInv = 1.0f / (float)displayEnv->disp.h;

	// first map to 0..1
	float clipRectX = (float)(rect->x - displayEnv->disp.x) * psxScreenWInv;
	float clipRectY = (float)(rect->y - displayEnv->disp.y) * psxScreenHInv;
	float clipRectW = (float)(rect->w) * psxScreenWInv;
	float clipRectH = (float)(rect->h) * psxScreenHInv;

	// then map to screen
	{
		clipRectX -= 0.5f;

		clipRectX *= emuScreenAspect;
		clipRectW *= emuScreenAspect;

		clipRectX += 0.5f;
	}

	// The draw environment remains in PS1 coordinates. Scale its normalized clip
	// rectangle to the physical main render target used by this frame.
	const float viewportX = 0.0f;
	const float viewportY = 0.0f;
	const float viewportW = (float)(s_mainRenderTarget.width > 0 ? s_mainRenderTarget.width : displayEnv->disp.w);
	const float viewportH = (float)(s_mainRenderTarget.height > 0 ? s_mainRenderTarget.height : displayEnv->disp.h);
	const float flipOffset = viewportY + viewportH - clipRectH * viewportH;
	const float crx = viewportX + clipRectX * viewportW;
	const float cry = clipRectY * viewportH;
	const float crw = clipRectW * viewportW;
	const float crh = clipRectH * viewportH;

	glScissor(crx, flipOffset - cry, crw, crh);
}

internal void NativeRenderer_SetShader(const ShaderID shader)
{
	if (s_previousShader != shader)
	{
		glUseProgram(shader);

		s_previousShader = shader;
	}
}


void NativeRenderer_SetTexture(TextureID texture, TexFormat texFormat, int semiTransPass, BlendMode blendMode, int textured,
                               int textureFullyOpaque)
{
#ifdef __vita__
	int variant;
	if (semiTransPass == 1)
	{
		variant = NATIVE_PSX_SHADER_NON_STP;
	}
	else if (semiTransPass == 2)
	{
		variant = blendMode == BM_AVERAGE         ? NATIVE_PSX_SHADER_STP_AVERAGE
		          : blendMode == BM_ADD_QUATER_SOURCE ? NATIVE_PSX_SHADER_STP_QUARTER
		                                               : NATIVE_PSX_SHADER_STP;
	}
	else
	{
		variant = blendMode == BM_AVERAGE         ? NATIVE_PSX_SHADER_OPAQUE_AVERAGE
		          : blendMode == BM_ADD_QUATER_SOURCE ? NATIVE_PSX_SHADER_OPAQUE_QUARTER
		                                               : NATIVE_PSX_SHADER_OPAQUE;
	}
	if (texFormat == TF_32_BIT_RGBA && variant >= NATIVE_PSX_SHADER_NON_STP)
	{
		variant = blendMode == BM_AVERAGE         ? NATIVE_PSX_SHADER_OPAQUE_AVERAGE
		          : blendMode == BM_ADD_QUATER_SOURCE ? NATIVE_PSX_SHADER_OPAQUE_QUARTER
		                                               : NATIVE_PSX_SHADER_OPAQUE;
	}
	if (texFormat < TF_4_BIT || texFormat > TF_32_BIT_RGBA)
	{
		return;
	}
	GTEShader *shader;
	if (!textured)
	{
		shader = &s_gteUntexturedShaderVariants[variant < 3 ? variant : 0];
	}
	else if (textureFullyOpaque && texFormat <= TF_8_BIT)
	{
		const int opaqueVariant = blendMode == BM_AVERAGE              ? NATIVE_PSX_SHADER_OPAQUE_AVERAGE
		                          : blendMode == BM_ADD_QUATER_SOURCE ? NATIVE_PSX_SHADER_OPAQUE_QUARTER
		                                                               : NATIVE_PSX_SHADER_OPAQUE;
		shader = &s_gteFullyOpaqueShaderVariants[texFormat][opaqueVariant];
	}
	else
	{
		shader = &s_gteShaderVariants[texFormat][variant];
	}
#else
	(void)textured;
	(void)textureFullyOpaque;
	GTEShader *shader = NULL;
	switch (texFormat)
	{
	case TF_4_BIT:
		shader = &s_gteShader4;
		break;
	case TF_8_BIT:
		shader = &s_gteShader8;
		break;
	case TF_16_BIT:
		shader = &s_gteShader16;
		break;
	case TF_32_BIT_RGBA:
		shader = &s_gteShader32Rgba;
		break;
	}
	if (shader == NULL)
	{
		return;
	}
#endif

	NativeRenderer_SetShader(shader->shader);
	u_bilinearFilterLoc = shader->bilinearFilterLoc;
	u_projectionLoc = shader->projectionLoc;
	u_texelSizeLoc = texFormat == TF_32_BIT_RGBA ? shader->texelSizeLoc : -1;
#ifndef __vita__
	u_psxSemiTransPassLoc = shader->psxSemiTransPassLoc;
#endif
	u_psxDrawMaskSetLoc = shader->psxDrawMaskSetLoc;
	u_psxTextureOutputStpLoc = shader->psxTextureOutputStpLoc;

	if (g_dbg_texturelessMode)
	{
		texture = s_whiteTexture;
	}

	// NOTE(penta3): s_texture (unit 0) and s_rgLut (unit 1) sampler bindings are baked
	// into each program at compile time (NativeRenderer_Shader_Compile) and uniform
	// values persist per-program, so re-setting them on every split was redundant GL
	// churn. bilinearFilter stays here because it toggles at runtime (debug key).
	if (u_bilinearFilterLoc >= 0)
	{
		glUniform1i(u_bilinearFilterLoc, g_cfg_bilinearFiltering);
	}
#ifndef __vita__
	if (u_psxSemiTransPassLoc >= 0)
	{
		glUniform1i(u_psxSemiTransPassLoc, semiTransPass);
	}
#endif

	if (s_lastBoundTexture == texture)
	{
		return;
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	s_lastBoundTexture = texture;
}

void NativeRenderer_SetOverrideTextureSize(int width, int height)
{
	if (u_texelSizeLoc == -1)
	{
		return;
	}

	float vec[] = {1.0f / (float)width, 1.0f / (float)height};
	glUniform2fv(u_texelSizeLoc, 1, vec);
}

void NativeRenderer_SetPSXTextureOutputSTP(int enabled)
{
	if (u_psxTextureOutputStpLoc >= 0)
	{
		glUniform1f(u_psxTextureOutputStpLoc, enabled ? 1.0f : 0.0f);
	}
}

void NativeRenderer_SetPSXDrawMaskSet(int maskSet)
{
	if (u_psxDrawMaskSetLoc >= 0)
	{
		glUniform1f(u_psxDrawMaskSetLoc, maskSet ? 1.0f : 0.0f);
	}
}

internal void NativeRenderer_DestroyTexture(TextureID texture)
{
	if (texture == (TextureID)-1)
	{
		return;
	}

	glDeleteTextures(1, &texture);
}

internal u16 NativeRenderer_PackRGB24ToPSX15(u8 r, u8 g, u8 b)
{
	return (u16)(((r >> 3) & 0x1f) | (((g >> 3) & 0x1f) << 5) | (((b >> 3) & 0x1f) << 10));
}

internal float NativeRenderer_PSXColorComponentFloat(u8 value)
{
	const u8 psx8 = (u8)((value >> 3) << 3);
	return (float)psx8 / 255.0f;
}

internal int NativeRenderer_ClipVRAMRect(RECT16 *out, int x, int y, int w, int h)
{
	if ((w <= 0) || (h <= 0))
	{
		return 0;
	}

	if (x < 0)
	{
		w += x;
		x = 0;
	}
	if (y < 0)
	{
		h += y;
		y = 0;
	}
	if (x + w > VRAM_WIDTH)
	{
		w = VRAM_WIDTH - x;
	}
	if (y + h > VRAM_HEIGHT)
	{
		h = VRAM_HEIGHT - y;
	}
	if ((w <= 0) || (h <= 0))
	{
		return 0;
	}

	out->x = (s16)x;
	out->y = (s16)y;
	out->w = (s16)w;
	out->h = (s16)h;
	return 1;
}

#ifdef __vita__
internal int NativeRenderer_HasGpuNewerVRAMTiles(int x, int y, int w, int h)
{
	const int tileX0 = x / NATIVE_VRAM_TILE_SIZE;
	const int tileY0 = y / NATIVE_VRAM_TILE_SIZE;
	const int tileX1 = (x + w - 1) / NATIVE_VRAM_TILE_SIZE;
	const int tileY1 = (y + h - 1) / NATIVE_VRAM_TILE_SIZE;
	for (int tileY = tileY0; tileY <= tileY1; tileY++)
	{
		for (int tileX = tileX0; tileX <= tileX1; tileX++)
		{
			const int tileIndex = tileY * NATIVE_VRAM_TILE_COLS + tileX;
			if ((s_vram.gpuNewerTiles[tileIndex >> 5] & (1u << (tileIndex & 31))) != 0)
			{
				return 1;
			}
		}
	}
	return 0;
}

int NativeRenderer_GetPaletteProperties(TexFormat format, int clut)
{
	const int width = format == TF_4_BIT ? 16 : format == TF_8_BIT ? 256 : 0;
	const int x = (clut & 0x3f) << 4;
	const int y = clut >> 6;
	if (width == 0 || y < 0 || y >= VRAM_HEIGHT || x < 0 || x + width > VRAM_WIDTH)
	{
		return NATIVE_PALETTE_HAS_TRANSPARENT | NATIVE_PALETTE_HAS_OPAQUE | NATIVE_PALETTE_HAS_STP;
	}
	if (NativeRenderer_HasGpuNewerVRAMTiles(x, y, width, 1))
	{
		return NATIVE_PALETTE_HAS_TRANSPARENT | NATIVE_PALETTE_HAS_OPAQUE | NATIVE_PALETTE_HAS_STP;
	}

	int properties = 0;
	const u16 *palette = s_vram.cpuPixels + y * VRAM_WIDTH + x;
	for (int colorIndex = 0; colorIndex < width; colorIndex++)
	{
		const u16 color = palette[colorIndex];
		if (color == 0)
		{
			properties |= NATIVE_PALETTE_HAS_TRANSPARENT;
		}
		else if ((color & 0x8000) != 0)
		{
			properties |= NATIVE_PALETTE_HAS_STP;
		}
		else
		{
			properties |= NATIVE_PALETTE_HAS_OPAQUE;
		}
		if (properties == (NATIVE_PALETTE_HAS_TRANSPARENT | NATIVE_PALETTE_HAS_OPAQUE | NATIVE_PALETTE_HAS_STP))
		{
			break;
		}
	}
	return properties;
}

#endif

internal int NativeRenderer_DirtyRectContains(const RECT16 *outer, const RECT16 *inner)
{
	const int outerRight = outer->x + outer->w;
	const int outerBottom = outer->y + outer->h;
	const int innerRight = inner->x + inner->w;
	const int innerBottom = inner->y + inner->h;

	return (outer->x <= inner->x) && (outer->y <= inner->y) && (outerRight >= innerRight) && (outerBottom >= innerBottom);
}

internal int NativeRenderer_TryMergeDirtyRect(RECT16 *dst, const RECT16 *src)
{
	const int dstRight = dst->x + dst->w;
	const int dstBottom = dst->y + dst->h;
	const int srcRight = src->x + src->w;
	const int srcBottom = src->y + src->h;

	if ((dst->y == src->y) && (dst->h == src->h) && (src->x <= dstRight) && (dst->x <= srcRight))
	{
		const int x0 = (src->x < dst->x) ? src->x : dst->x;
		const int x1 = (srcRight > dstRight) ? srcRight : dstRight;
		dst->x = (s16)x0;
		dst->w = (s16)(x1 - x0);
		return 1;
	}

	if ((dst->x == src->x) && (dst->w == src->w) && (src->y <= dstBottom) && (dst->y <= srcBottom))
	{
		const int y0 = (src->y < dst->y) ? src->y : dst->y;
		const int y1 = (srcBottom > dstBottom) ? srcBottom : dstBottom;
		dst->y = (s16)y0;
		dst->h = (s16)(y1 - y0);
		return 1;
	}

	return 0;
}

internal void NativeRenderer_AppendCpuDirtyRect(const RECT16 *rect)
{
	for (s32 i = 0; i < s_vram.cpuDirtyRectCount; i++)
	{
		if (NativeRenderer_DirtyRectContains(&s_vram.cpuDirtyRects[i], rect))
		{
			return;
		}

		if (NativeRenderer_DirtyRectContains(rect, &s_vram.cpuDirtyRects[i]))
		{
			s_vram.cpuDirtyRects[i] = *rect;
			return;
		}

		if (NativeRenderer_TryMergeDirtyRect(&s_vram.cpuDirtyRects[i], rect))
		{
			return;
		}
	}

	if (s_vram.cpuDirtyRectCount >= NATIVE_VRAM_DIRTY_RECT_CAP)
	{
		NativeRenderer_UpdateVRAM();
	}

	if (s_vram.cpuDirtyRectCount < NATIVE_VRAM_DIRTY_RECT_CAP)
	{
		s_vram.cpuDirtyRects[s_vram.cpuDirtyRectCount] = *rect;
		s_vram.cpuDirtyRectCount++;
	}
}

internal void NativeRenderer_MarkVRAMDirty(int x, int y, int w, int h)
{
	RECT16 rect;

	if (!NativeRenderer_ClipVRAMRect(&rect, x, y, w, h))
	{
		return;
	}

	NativeRenderer_AppendCpuDirtyRect(&rect);
}

internal void NativeRenderer_MarkGpuVRAMNewer(int x, int y, int w, int h)
{
	RECT16 rect;

	if (!NativeRenderer_ClipVRAMRect(&rect, x, y, w, h))
	{
		return;
	}

	const int tileX0 = rect.x / NATIVE_VRAM_TILE_SIZE;
	const int tileY0 = rect.y / NATIVE_VRAM_TILE_SIZE;
	const int tileX1 = (rect.x + rect.w - 1) / NATIVE_VRAM_TILE_SIZE;
	const int tileY1 = (rect.y + rect.h - 1) / NATIVE_VRAM_TILE_SIZE;

	for (int tileY = tileY0; tileY <= tileY1; tileY++)
	{
		for (int tileX = tileX0; tileX <= tileX1; tileX++)
		{
			const int tileIndex = tileY * NATIVE_VRAM_TILE_COLS + tileX;
			s_vram.gpuNewerTiles[tileIndex >> 5] |= 1u << (tileIndex & 31);
		}
	}
}

void NativeRenderer_ClearVRAM(int x, int y, int w, int h, u8 r, u8 g, u8 b)
{
	u16 *dst = s_vram.cpuPixels + x + y * VRAM_WIDTH;
	const u16 color = NativeRenderer_PackRGB24ToPSX15(r, g, b);

	if (x + w > VRAM_WIDTH)
	{
		w = VRAM_WIDTH - x;
	}

	if (y + h > VRAM_HEIGHT)
	{
		h = VRAM_HEIGHT - y;
	}

	// clear VRAM region with given color
	for (int i = 0; i < h; i++)
	{
		u16 *tmp = dst;

		for (int j = 0; j < w; j++)
		{
			*tmp++ = color;
		}

		dst += VRAM_WIDTH;
	}

	NativeRenderer_MarkVRAMDirty(x, y, w, h);
}

void NativeRenderer_Clear(int x, int y, int w, int h, u8 r, u8 g, u8 b)
{
	if ((w <= 0) || (h <= 0))
	{
		return;
	}

	int displayX = activeDispEnv.disp.x;
	int displayY = activeDispEnv.disp.y;
	int displayW = activeDispEnv.disp.w;
	int displayH = activeDispEnv.disp.h;

	if ((displayW <= 0) || (displayH <= 0))
	{
		displayX = activeDrawEnv.clip.x;
		displayY = activeDrawEnv.clip.y;
		displayW = activeDrawEnv.clip.w;
		displayH = activeDrawEnv.clip.h;
	}

	if ((displayW <= 0) || (displayH <= 0))
	{
		return;
	}

	const int clearRight = x + w;
	const int clearBottom = y + h;
	const int displayRight = displayX + displayW;
	const int displayBottom = displayY + displayH;

	const int overlapX = x > displayX ? x : displayX;
	const int overlapY = y > displayY ? y : displayY;
	const int overlapRight = clearRight < displayRight ? clearRight : displayRight;
	const int overlapBottom = clearBottom < displayBottom ? clearBottom : displayBottom;

	if ((overlapRight <= overlapX) || (overlapBottom <= overlapY))
	{
		return;
	}

	const int physicalW = s_mainRenderTarget.width > 0 ? s_mainRenderTarget.width : displayW;
	const int physicalH = s_mainRenderTarget.height > 0 ? s_mainRenderTarget.height : displayH;
	const int relLeft = overlapX - displayX;
	const int relRight = overlapRight - displayX;
	const int relTop = overlapY - displayY;
	const int relBottom = overlapBottom - displayY;
	const int scissorX = relLeft * physicalW / displayW;
	const int scissorRight = (relRight * physicalW + displayW - 1) / displayW;
	const int scissorY = (displayH - relBottom) * physicalH / displayH;
	const int scissorTop = ((displayH - relTop) * physicalH + displayH - 1) / displayH;
	const int scissorW = scissorRight - scissorX;
	const int scissorH = scissorTop - scissorY;

	if ((scissorW <= 0) || (scissorH <= 0))
	{
		return;
	}

	GLint previousScissorBox[4];
	const GLboolean previousScissorEnabled = glIsEnabled(GL_SCISSOR_TEST);
	glGetIntegerv(GL_SCISSOR_BOX, previousScissorBox);

	glEnable(GL_SCISSOR_TEST);
	glScissor(scissorX, scissorY, scissorW, scissorH);
	glClearColor(NativeRenderer_PSXColorComponentFloat(r), NativeRenderer_PSXColorComponentFloat(g), NativeRenderer_PSXColorComponentFloat(b), 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	if (previousScissorEnabled)
	{
		glEnable(GL_SCISSOR_TEST);
		glScissor(previousScissorBox[0], previousScissorBox[1], previousScissorBox[2], previousScissorBox[3]);
	}
	else
	{
		glDisable(GL_SCISSOR_TEST);
	}

	s_previousScissorState = previousScissorEnabled ? 1 : 0;
}

void NativeRenderer_SaveVRAM(const char *outputFileName, int x, int y, int width, int height, int bReadFromFrameBuffer)
{
#define FLIP_Y (VRAM_HEIGHT - i - 1)

	(void)x;
	(void)y;
	(void)bReadFromFrameBuffer;

	NativeRenderer_SyncGpuVRAMToCPU(0, 0, VRAM_WIDTH, VRAM_HEIGHT);

	FILE *fp = fopen(outputFileName, "wb");
	if (fp == NULL)
	{
		return;
	}

	u8 TGAheader[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	u8 header[6];
	header[0] = (width % 256);
	header[1] = (width / 256);
	header[2] = (height % 256);
	header[3] = (height / 256);
	header[4] = 16;
	header[5] = 0;

	fwrite(TGAheader, sizeof(u8), 12, fp);
	fwrite(header, sizeof(u8), 6, fp);

	for (int i = 0; i < VRAM_HEIGHT; i++)
	{
		fwrite(s_vram.cpuPixels + VRAM_WIDTH * FLIP_Y, sizeof(u16), VRAM_WIDTH, fp);
	}

	fclose(fp);

#undef FLIP_Y
}

internal void NativeRenderer_SyncGpuVRAMToCPU(int x, int y, int w, int h)
{
	RECT16 readRect;

	if (!NativeRenderer_ClipVRAMRect(&readRect, x, y, w, h))
	{
		return;
	}

	const int tileX0 = readRect.x / NATIVE_VRAM_TILE_SIZE;
	const int tileY0 = readRect.y / NATIVE_VRAM_TILE_SIZE;
	const int tileX1 = (readRect.x + readRect.w - 1) / NATIVE_VRAM_TILE_SIZE;
	const int tileY1 = (readRect.y + readRect.h - 1) / NATIVE_VRAM_TILE_SIZE;
	b32 needsReadback = false;

	for (int tileY = tileY0; tileY <= tileY1 && !needsReadback; tileY++)
	{
		for (int tileX = tileX0; tileX <= tileX1; tileX++)
		{
			const int tileIndex = tileY * NATIVE_VRAM_TILE_COLS + tileX;
			if ((s_vram.gpuNewerTiles[tileIndex >> 5] & (1u << (tileIndex & 31))) != 0)
			{
				needsReadback = true;
				break;
			}
		}
	}

	if (!needsReadback)
	{
		return;
	}

	readRect.x = (s16)(tileX0 * NATIVE_VRAM_TILE_SIZE);
	readRect.y = (s16)(tileY0 * NATIVE_VRAM_TILE_SIZE);
	readRect.w = (s16)((tileX1 - tileX0 + 1) * NATIVE_VRAM_TILE_SIZE);
	readRect.h = (s16)((tileY1 - tileY0 + 1) * NATIVE_VRAM_TILE_SIZE);

	// CPU writes must reach the persistent texture before a GPU-authored region
	// is read back, preserving PS1 VRAM command order in the split host mirror.
	NativeRenderer_UpdateVRAM();

	NativePerf_BeginScope(NATIVE_PERF_BUCKET_FRAMEBUFFER_READBACK);
	GLint previousReadFramebuffer;
	GLint previousPackRowLength;
	GLint previousPackAlignment;
	glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &previousReadFramebuffer);
#ifndef __vita__
	glGetIntegerv(GL_PACK_ROW_LENGTH, &previousPackRowLength);
	glGetIntegerv(GL_PACK_ALIGNMENT, &previousPackAlignment);
#endif

	glBindFramebuffer(GL_READ_FRAMEBUFFER, s_glVramFramebuffer);
#ifndef __vita__
	glPixelStorei(GL_PACK_ROW_LENGTH, VRAM_WIDTH);
	glPixelStorei(GL_PACK_ALIGNMENT, sizeof(u16));
	glReadPixels(readRect.x, readRect.y, readRect.w, readRect.h, VRAM_FORMAT, GL_UNSIGNED_BYTE,
	             s_vram.cpuPixels + (size_t)readRect.y * VRAM_WIDTH + readRect.x);
#else
	// READBACKS_SPEEDHACK deliberately omits vitaGL's implicit scene finish.
	// CPU consumers such as the pause-screen compressor access the returned
	// bytes immediately, so make this explicit at the native synchronization
	// boundary instead of disabling the speedhack globally.
	glFinish();
	// vitaGL always packs rows tightly.  Passing a pointer into cpuPixels here
	// would advance each subsequent row by readRect.w instead of VRAM_WIDTH.
	glReadPixels(readRect.x, readRect.y, readRect.w, readRect.h, VRAM_FORMAT, GL_UNSIGNED_BYTE, s_vitaVramTransferPixels);
	for (int row = 0; row < readRect.h; row++)
	{
		const u8 *src = s_vitaVramTransferPixels + (size_t)row * readRect.w * 4;
		u16 *dst = s_vram.cpuPixels + (size_t)(readRect.y + row) * VRAM_WIDTH + readRect.x;
		for (int column = 0; column < readRect.w; column++)
		{
			dst[column] = (u16)(src[column * 4] | ((u16)src[column * 4 + 1] << 8));
		}
	}
#endif

	for (int tileY = tileY0; tileY <= tileY1; tileY++)
	{
		for (int tileX = tileX0; tileX <= tileX1; tileX++)
		{
			const int tileIndex = tileY * NATIVE_VRAM_TILE_COLS + tileX;
			s_vram.gpuNewerTiles[tileIndex >> 5] &= ~(1u << (tileIndex & 31));
		}
	}

#ifndef __vita__
	glPixelStorei(GL_PACK_ROW_LENGTH, previousPackRowLength);
	glPixelStorei(GL_PACK_ALIGNMENT, previousPackAlignment);
#endif
	glBindFramebuffer(GL_READ_FRAMEBUFFER, (GLuint)previousReadFramebuffer);
	NativePerf_EndScope(NATIVE_PERF_BUCKET_FRAMEBUFFER_READBACK);
}

internal void NativeRenderer_ResolveVRAMRead(int x, int y, int w, int h)
{
	NativeRenderer_SyncGpuVRAMToCPU(x, y, w, h);
}

void NativeRenderer_SyncVRAMToCPU(int x, int y, int w, int h)
{
	NativeRenderer_SyncGpuVRAMToCPU(x, y, w, h);
}

internal int NativeRenderer_RectEquals(const RECT16 *a, const RECT16 *b)
{
	return a->x == b->x && a->y == b->y && a->w == b->w && a->h == b->h;
}

internal void NativeRenderer_FlushOffscreenToVRAM(void)
{
	if (s_previousOffscreen.w <= 0 || s_previousOffscreen.h <= 0)
	{
		return;
	}

	// NOTE(aalhendi): Native offscreen draws produce RGBA pixels. Pack them into
	// the persistent 5:5:5:1 VRAM texture instead of reading them through the CPU.
	NativeRenderer_GpuPackTextureToVRAM(s_offscreenRenderTarget.texture, s_previousOffscreen.x, s_previousOffscreen.y, s_previousOffscreen.w,
	                                    s_previousOffscreen.h, true);
}

internal void NativeRenderer_SetScissorState(int enable)
{
	if (s_previousScissorState == enable)
	{
		return;
	}

	if (s_previousScissorState)
	{
		glDisable(GL_SCISSOR_TEST);
	}
	else
	{
		glEnable(GL_SCISSOR_TEST);
	}
	s_previousScissorState = enable;
}

void NativeRenderer_SetOffscreenState(const RECT16 *offscreenRect, int enable)
{
	const int sameOffscreenRect = NativeRenderer_RectEquals(&s_previousOffscreen, offscreenRect);
	if (!enable && !s_previousOffscreenState)
	{
		return;
	}

	if (enable && s_previousOffscreenState && sameOffscreenRect)
	{
		return;
	}

	if (enable)
	{
		if (s_previousOffscreenState)
		{
			NativeRenderer_FlushOffscreenToVRAM();
		}

		s_previousOffscreenState = 1;
		NativeRenderer_EnsureRenderTarget(&s_offscreenRenderTarget, offscreenRect->w, offscreenRect->h);
		s_offscreenRenderTarget.logicalWidth = offscreenRect->w;
		s_offscreenRenderTarget.logicalHeight = offscreenRect->h;
		s_previousOffscreen = *offscreenRect;
		NativeRenderer_LoadRenderTargetFromVRAM(&s_offscreenRenderTarget, offscreenRect->x, offscreenRect->y, offscreenRect->w, offscreenRect->h);
	}
	else
	{
		s_previousOffscreenState = 0;

		NativeRenderer_FlushOffscreenToVRAM();
		NativeRenderer_BindMainRenderTarget();
		NativeRenderer_SetViewPort(0, 0, s_mainRenderTarget.width, s_mainRenderTarget.height);
	}
}

void NativeRenderer_SetProjection(const RECT16 *drawRect, const DISPENV *displayEnv, int offscreen)
{
	if (offscreen)
	{
		NativeRenderer_Ortho2D(0, drawRect->w, drawRect->h, 0, -1.0f, 1.0f);
		return;
	}

	const int displayW = displayEnv->disp.w > 0 ? displayEnv->disp.w : 1;
	const int displayH = displayEnv->disp.h > 0 ? displayEnv->disp.h : 1;
	NativeRenderer_Ortho2D(0, displayW, displayH, 0, -1.0f, 1.0f);
}

// NOTE(aalhendi): Pack an RGBA render texture straight into the RG8 VRAM texture
// on the GPU, no CPU round trip. Restore or invalidate the render-state caches
// disturbed by this native bridge before the submit run continues.
internal void NativeRenderer_GpuPackTextureToVRAM(TextureID sourceTexture, int x, int y, int w, int h, b32 flipY)
{
	const ShaderID previousShader = s_previousShader;
	const TextureID previousTexture = s_lastBoundTexture;
	const BlendMode previousBlendMode = s_previousBlendMode;
	const int previousScissorState = s_previousScissorState;
	const GLboolean previousStencilEnabled = glIsEnabled(GL_STENCIL_TEST);

	NativeRenderer_UpdateVRAM();

	glBindFramebuffer(GL_FRAMEBUFFER, s_glVramFramebuffer);

	glDisable(GL_BLEND);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glViewport(x, y, w, h);

	glUseProgram(s_packShader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sourceTexture);
	glUniform1i(s_packFlipYLoc, flipY);

	glBindVertexArray(s_vramQuadVAO);
	NativeRenderer_DrawTriangles(0, 2);
	if (s_boundVertexBuffer >= 0)
	{
		glBindVertexArray(s_glVertexArray[s_boundVertexBuffer]);
	}
	else
	{
		glBindVertexArray(0);
	}

	if (s_previousOffscreenState)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, s_offscreenRenderTarget.framebuffer);
		glViewport(0, 0, s_offscreenRenderTarget.width, s_offscreenRenderTarget.height);
	}
	else
	{
		NativeRenderer_BindMainRenderTarget();
		glViewport(0, 0, s_mainRenderTarget.width, s_mainRenderTarget.height);
	}
	if (previousStencilEnabled)
	{
		glEnable(GL_STENCIL_TEST);
	}

	glUseProgram(previousShader == (ShaderID)-1 ? 0 : previousShader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, previousTexture == (TextureID)-1 ? 0 : previousTexture);
	s_previousShader = previousShader;
	s_lastBoundTexture = previousTexture;
	s_previousBlendMode = BM_NONE;
	s_previousScissorState = 0;
	NativeRenderer_SetBlendMode(previousBlendMode);
	NativeRenderer_SetScissorState(previousScissorState);
	NativeRenderer_MarkGpuVRAMNewer(x, y, w, h);
}

// NOTE(aalhendi): PS1 draws into VRAM and can texture from that same VRAM. Native
// mirrors that by flushing pending CPU VRAM writes, then packing the presented
// framebuffer into the persistent RG8 VRAM texture. CPU-side VRAM reads pull from
// that packed texture lazily, avoiding the old per-frame GPU->CPU->GPU round trip.
void NativeRenderer_StoreFrameBuffer(int x, int y, int w, int h)
{
	NativePerf_BeginScope(NATIVE_PERF_BUCKET_FRAMEBUFFER_STORE);
	NativeRenderer_GpuPackTextureToVRAM(s_mainRenderTarget.texture, x, y, w, h, true);

	NativePerf_EndScope(NATIVE_PERF_BUCKET_FRAMEBUFFER_STORE);
}

void NativeRenderer_CopyVRAM(u16 *src, int x, int y, int w, int h, int dst_x, int dst_y)
{
	int stride = w;

	if (!src)
	{
		// NOTE(aalhendi): MoveImage reads exactly its PS1 VRAM source rectangle. Resolve only that
		// GPU-authored region into the CPU mirror before copying it.
		NativeRenderer_ResolveVRAMRead(x, y, w, h);
		src = s_vram.cpuPixels;
		stride = VRAM_WIDTH;
	}

	src += x + y * stride;

	u16 *dst = s_vram.cpuPixels + dst_x + dst_y * VRAM_WIDTH;

	for (int i = 0; i < h; i++)
	{
		SDL_memcpy(dst, src, w * sizeof(u16));
		dst += VRAM_WIDTH;
		src += stride;
	}

	NativeRenderer_MarkVRAMDirty(dst_x, dst_y, w, h);
}

void NativeRenderer_ReadVRAM(u16 *dst, int x, int y, int dst_w, int dst_h)
{
	NativeRenderer_ResolveVRAMRead(x, y, dst_w, dst_h);

	u16 *src = s_vram.cpuPixels + x + VRAM_WIDTH * y;

	for (int i = 0; i < dst_h; i++)
	{
		SDL_memcpy(dst, src, dst_w * sizeof(u16));
		dst += dst_w;
		src += VRAM_WIDTH;
	}
}

int NativeRenderer_GetVRAMStateSize(void)
{
	return (int)sizeof(s_vram.cpuPixels);
}

int NativeRenderer_CaptureVRAMState(void *dst, int dstSize)
{
	if ((dst == NULL) || (dstSize < (int)sizeof(s_vram.cpuPixels)))
	{
		return 0;
	}

	// NOTE(aalhendi): Save-states own the CPU-side PSX VRAM mirror, not GL
	// textures. Pull pending GPU-authored VRAM into the mirror first.
	NativeRenderer_SyncGpuVRAMToCPU(0, 0, VRAM_WIDTH, VRAM_HEIGHT);
	SDL_memcpy(dst, s_vram.cpuPixels, sizeof(s_vram.cpuPixels));
	return 1;
}

int NativeRenderer_RestoreVRAMState(const void *src, int srcSize)
{
	local_persist const RECT16 zeroRect = {0, 0, 0, 0};

	if ((src == NULL) || (srcSize < (int)sizeof(s_vram.cpuPixels)))
	{
		return 0;
	}

	SDL_memcpy(s_vram.cpuPixels, src, sizeof(s_vram.cpuPixels));
	// NOTE(aalhendi): Restored VRAM is authoritative PSX state. Host GL caches
	// are rebuildable, so mark all of VRAM dirty and drop stale bindings.
	s_vram.cpuDirtyRectCount = 0;
	SDL_memset(s_vram.gpuNewerTiles, 0, sizeof(s_vram.gpuNewerTiles));
	NativeRenderer_MarkVRAMDirty(0, 0, VRAM_WIDTH, VRAM_HEIGHT);
	s_mainRenderTarget.width = 0;
	s_mainRenderTarget.height = 0;
	s_mainRenderTarget.logicalWidth = 0;
	s_mainRenderTarget.logicalHeight = 0;
	s_offscreenRenderTarget.width = 0;
	s_offscreenRenderTarget.height = 0;
	s_offscreenRenderTarget.logicalWidth = 0;
	s_offscreenRenderTarget.logicalHeight = 0;
	s_previousOffscreen = zeroRect;
	s_previousOffscreenState = 0;
	s_previousShader = (ShaderID)-1;
	s_lastBoundTexture = (TextureID)-1;
	return 1;
}

void NativeRenderer_UpdateVRAM(void)
{
	if (s_vram.cpuDirtyRectCount == 0)
	{
		return;
	}

	NativePerf_BeginScope(NATIVE_PERF_BUCKET_RENDERER_UPDATE_VRAM);

	const s32 rectCount = s_vram.cpuDirtyRectCount;
	s_vram.cpuDirtyRectCount = 0;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, s_vram.texture);
#ifdef __vita__
	// Expand each packed PSX word to the RGBA render-target representation.
	// The shaders consume only .r/.g, which retain the low/high PSX bytes.
	for (s32 i = 0; i < rectCount; i++)
	{
		const RECT16 r = s_vram.cpuDirtyRects[i];
		u8 *dst = s_vitaVramTransferPixels;
		for (int y = 0; y < r.h; y++)
		{
			const u16 *src = s_vram.cpuPixels + (size_t)(r.y + y) * VRAM_WIDTH + r.x;
			for (int x = 0; x < r.w; x++)
			{
				const u16 pixel = src[x];
				*dst++ = (u8)pixel;
				*dst++ = (u8)(pixel >> 8);
				*dst++ = 0;
				*dst++ = 255;
			}
		}
		glTexSubImage2D(GL_TEXTURE_2D, 0, r.x, r.y, r.w, r.h, VRAM_FORMAT, GL_UNSIGNED_BYTE, s_vitaVramTransferPixels);
	}
#else
	glPixelStorei(GL_UNPACK_ROW_LENGTH, VRAM_WIDTH);
	for (s32 i = 0; i < rectCount; i++)
	{
		const RECT16 r = s_vram.cpuDirtyRects[i];
		glTexSubImage2D(GL_TEXTURE_2D, 0, r.x, r.y, r.w, r.h, VRAM_FORMAT, GL_UNSIGNED_BYTE, s_vram.cpuPixels + (size_t)r.y * VRAM_WIDTH + r.x);
	}
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
	s_lastBoundTexture = (TextureID)-1;

	NativePerf_EndScope(NATIVE_PERF_BUCKET_RENDERER_UPDATE_VRAM);
}

void NativeRenderer_PresentVRAMRect(int displayX, int displayY, int displayW, int displayH)
{
	if (displayW <= 0 || displayH <= 0)
	{
		return;
	}

	NativeRenderer_UpdateVRAM();

	NativeRenderer_SetViewPort(s_presentViewport.x, s_presentViewport.y, s_presentViewport.w, s_presentViewport.h);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	NativeRenderer_SetScissorState(0);
	NativeRenderer_EnableDepth(0);
	NativeRenderer_SetBlendMode(BM_NONE);
	const GLboolean previousStencilEnabled = glIsEnabled(GL_STENCIL_TEST);
	glDisable(GL_STENCIL_TEST);

	NativeRenderer_DrawVRAMRegion(displayX, displayY, displayW, displayH);
	if (previousStencilEnabled)
	{
		glEnable(GL_STENCIL_TEST);
	}
	glBindVertexArray(0);

	s_previousShader = (ShaderID)-1;
	s_lastBoundTexture = (TextureID)-1;
}

void NativeRenderer_PresentMainRenderTarget(void)
{
	if ((s_mainRenderTarget.texture == 0) || (s_mainRenderTarget.width <= 0) || (s_mainRenderTarget.height <= 0))
	{
		return;
	}

	NativeRenderer_SetViewPort(s_presentViewport.x, s_presentViewport.y, s_presentViewport.w, s_presentViewport.h);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	NativeRenderer_SetScissorState(0);
	NativeRenderer_EnableDepth(0);
	NativeRenderer_SetBlendMode(BM_NONE);
	const GLboolean previousStencilEnabled = glIsEnabled(GL_STENCIL_TEST);
	glDisable(GL_STENCIL_TEST);

	glUseProgram(s_presentRgbaShader);
	glUniform1f(s_presentRgbaFlipYLoc, 0.0f);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, s_mainRenderTarget.texture);
	glBindVertexArray(s_vramQuadVAO);
	NativeRenderer_DrawTriangles(0, 2);

	if (previousStencilEnabled)
	{
		glEnable(GL_STENCIL_TEST);
	}
	glBindVertexArray(0);

	s_previousShader = (ShaderID)-1;
	s_lastBoundTexture = (TextureID)-1;
}

void NativeRenderer_PresentStreamingTexture(TextureID texture, int contentHeight, int displayHeight)
{
	if ((texture == 0) || (contentHeight <= 0) || (displayHeight < contentHeight))
	{
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	NativeRenderer_SetScissorState(0);
	NativeRenderer_EnableDepth(0);
	NativeRenderer_SetBlendMode(BM_NONE);
	const GLboolean previousStencilEnabled = glIsEnabled(GL_STENCIL_TEST);
	glDisable(GL_STENCIL_TEST);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	const int viewportH = (s_presentViewport.h * contentHeight + displayHeight / 2) / displayHeight;
	const int viewportY = s_presentViewport.y + (s_presentViewport.h - viewportH) / 2;
	NativeRenderer_SetViewPort(s_presentViewport.x, viewportY, s_presentViewport.w, viewportH);

	glUseProgram(s_presentRgbaShader);
	glUniform1f(s_presentRgbaFlipYLoc, 1.0f);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glBindVertexArray(s_vramQuadVAO);
	NativeRenderer_DrawTriangles(0, 2);

	if (previousStencilEnabled)
	{
		glEnable(GL_STENCIL_TEST);
	}
	glBindVertexArray(0);
	s_previousShader = (ShaderID)-1;
	s_lastBoundTexture = (TextureID)-1;
}

void NativeRenderer_PresentVRAMDisplay(void)
{
	// NOTE(aalhendi): ctr-native local divergence. Retail presents this boot
	// splash path by displaying VRAM directly after DR_MOVE packets; the native
	// OpenGL backend otherwise swaps the current framebuffer and never shows
	// those VRAM-only copies.
	NativeRenderer_PresentVRAMRect(activeDispEnv.disp.x, activeDispEnv.disp.y, activeDispEnv.disp.w, activeDispEnv.disp.h);
}

void NativeRenderer_SwapWindow(void)
{
	NativePerf_BeginScope(NATIVE_PERF_BUCKET_SWAP_WINDOW);
	SDL_GL_SwapWindow(g_window);
	NativePerf_EndScope(NATIVE_PERF_BUCKET_SWAP_WINDOW);
}

internal void NativeRenderer_EnableDepth(int enable)
{
	if (s_previousDepthMode == enable)
	{
		return;
	}

	s_previousDepthMode = enable;

	glDisable(GL_DEPTH_TEST);
}

void NativeRenderer_SetStencilMode(int drawPrim)
{
	if (s_previousStencilMode == drawPrim)
	{
		return;
	}

	s_previousStencilMode = drawPrim;

	if (drawPrim)
	{
		glStencilFunc(GL_ALWAYS, 1, 0x10);
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	}
	else
	{
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);
	}
}

void NativeRenderer_SetBlendMode(BlendMode blendMode)
{
	if (s_previousBlendMode == blendMode)
	{
		return;
	}

	if (blendMode != BM_NONE)
	{
		if (s_previousBlendMode == BM_NONE)
		{
#ifndef __vita__
			glBlendColor(0.25f, 0.25f, 0.25f, 0.5f);
#endif
			glEnable(GL_BLEND);
		}

		NativeRenderer_EnableDepth(0);
	}

	switch (blendMode)
	{
	case BM_NONE:
		if (s_previousBlendMode != BM_NONE)
		{
#ifndef __vita__
			glBlendColor(1.f, 1.f, 1.f, 1.f);
#endif
			glDisable(GL_BLEND);
		}

		NativeRenderer_EnableDepth(1);
		break;
	case BM_AVERAGE:
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
#ifdef __vita__
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
#else
		// NOTE(aalhendi): keep RGB blend weight constant so alpha can carry the PS1 mask bit.
		glBlendFuncSeparate(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA, GL_ONE, GL_ZERO);
#endif
		break;
	case BM_ADD:
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ZERO);
		break;
	case BM_SUBTRACT:
		glBlendEquationSeparate(GL_FUNC_REVERSE_SUBTRACT, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ZERO);
		break;
	case BM_ADD_QUATER_SOURCE:
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
#ifdef __vita__
		glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ZERO);
#else
		glBlendFuncSeparate(GL_CONSTANT_COLOR, GL_ONE, GL_ONE, GL_ZERO);
#endif
		break;
	}

	s_previousBlendMode = blendMode;
}

internal void NativeRenderer_SetViewPort(int x, int y, int width, int height)
{
	glViewport(x, y, width, height);
}

internal void NativeRenderer_SetWireframe(int enable)
{
	glPolygonMode(GL_FRONT_AND_BACK, enable ? GL_LINE : GL_FILL);
}

void NativeRenderer_UpdateVertexBuffer(const GrVertex *vertices, int num_vertices)
{
	NativePerf_BeginScope(NATIVE_PERF_BUCKET_RENDERER_VERTEX_UPLOAD);
	if (num_vertices <= 0)
	{
		NativePerf_EndScope(NATIVE_PERF_BUCKET_RENDERER_VERTEX_UPLOAD);
		return;
	}
	if ((u32)num_vertices >= MAX_VERTEX_BUFFER_SIZE)
	{
		NATIVE_RENDERER_ERROR("%s\n", "MAX_VERTEX_BUFFER_SIZE reached, expect rendering errors");
		num_vertices = MAX_VERTEX_BUFFER_SIZE;
	}

	const int bufferIndex = s_curVertexBuffer;
#ifndef __vita__
	s_curVertexBuffer = (s_curVertexBuffer + 1) % MAX_NUM_VERTEX_BUFFERS;
#endif
	s_boundVertexBuffer = bufferIndex;
	glBindVertexArray(s_glVertexArray[bufferIndex]);
	glBindBuffer(GL_ARRAY_BUFFER, s_glVertexBuffer[bufferIndex]);
#ifdef __vita__
	// vertices is already in vitaGL's GPU-mapped circular pool.
	vglBufferData(GL_ARRAY_BUFFER, vertices);
#else
	glBufferSubData(GL_ARRAY_BUFFER, 0, num_vertices * sizeof(GrVertex), vertices);
#endif

	NativePerf_EndScope(NATIVE_PERF_BUCKET_RENDERER_VERTEX_UPLOAD);
}

GrVertex *NativeRenderer_AllocateVertexBuffer(int count)
{
#ifdef __vita__
	if (count <= 0 || (u32)count > MAX_VERTEX_BUFFER_SIZE)
	{
		return NULL;
	}
	return (GrVertex *)vglAllocFromScratch((size_t)count * sizeof(GrVertex));
#else
	(void)count;
	return NULL;
#endif
}

void NativeRenderer_DrawTriangles(int start_vertex, int triangles)
{
	NativePerf_BeginScope(NATIVE_PERF_BUCKET_RENDERER_DRAW_TRIANGLES);
	glDrawArrays(GL_TRIANGLES, start_vertex, triangles * 3);
	NativePerf_EndScope(NATIVE_PERF_BUCKET_RENDERER_DRAW_TRIANGLES);
}

void NativeRenderer_PushDebugLabel(const char *label)
{
#ifndef __vita__
	if (!GLAD_GL_KHR_debug)
	{
		return;
	}
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0x8000, strlen(label), label);
#endif
}

void NativeRenderer_PopDebugLabel(void)
{
#ifndef __vita__
	if (!GLAD_GL_KHR_debug)
	{
		return;
	}
	glPopDebugGroup();
#endif
}
