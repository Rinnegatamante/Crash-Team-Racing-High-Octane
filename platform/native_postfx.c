#include <platform/native_postfx.h>
#include <platform/native_postfx_params.h>
#include <string.h>

typedef struct NativePostFXShader
{
	const char *name;
	const char *sampler;
	const char *label;
} NativePostFXShader;

static const NativePostFXShader s_effects[NATIVE_POSTFX_MODE_COUNT] =
{
	{"off", NULL, "OFF"},
	{"crt", "s0", "CRT"},
	{"crt_bleeding", "s0", "CRT BLEEDING"},
	{"daltonic", "colorMap", "DALTONIC"},
	{"fxaa", "iChannel0", "FXAA"},
	{"gamma", "uTexture", "GAMMA"},
	{"greyscale", "colorMap", "GREYSCALE"},
	{"hue", "uTexture", "HUE / SATURATION"},
	{"lcd3x", "s0", "LCD 3X"},
	{"negative", "colorMap", "NEGATIVE"},
	{"sepia", "colorMap", "SEPIA"},
	{"sharp_grid", "s0", "SHARP GRID"},
	{"sharp_scanlines", "s0", "SHARP SCANLINES"},
	{"vignette", "uTexture", "VIGNETTE"}
};

int gNativePostFXMode;
int gNativePostFXParameterValues[NATIVE_POSTFX_PARAMETER_COUNT] = {200, 100, 100, 100, 100, 100, 100, 100, 100};
unsigned gNativePostFXRevision = 1;

const char *NativePostFX_GetName(int mode)
{
	if (mode < 0 || mode >= NATIVE_POSTFX_MODE_COUNT)
		mode = NATIVE_POSTFX_OFF;
	return s_effects[mode].name;
}

const char *NativePostFX_GetLabel(int mode)
{
	if (mode < 0 || mode >= NATIVE_POSTFX_MODE_COUNT)
		mode = NATIVE_POSTFX_OFF;
	return s_effects[mode].label;
}

void NativePostFX_SetMode(int mode)
{
	if (mode < 0 || mode >= NATIVE_POSTFX_MODE_COUNT)
		mode = NATIVE_POSTFX_OFF;
	if (gNativePostFXMode != mode)
	{
		gNativePostFXMode = mode;
		gNativePostFXRevision++;
	}
}

void NativePostFX_SetParameter(int index, int centValue)
{
	if (index < 0 || index >= NATIVE_POSTFX_PARAMETER_COUNT)
		return;
	const NativePostFXParameter *param = &native_postfx_parameters[index];
	if (centValue < param->minimumCent)
		centValue = param->minimumCent;
	if (centValue > param->maximumCent)
		centValue = param->maximumCent;
	if (gNativePostFXParameterValues[index] != centValue)
	{
		gNativePostFXParameterValues[index] = centValue;
		gNativePostFXRevision++;
	}
}

int NativePostFX_CalculateCrop(int x, int y, int width, int height, int displayWidth, int displayHeight, float crop[4])
{
	if (crop == NULL || displayWidth <= 0 || displayHeight <= 0 || width <= 0 || height <= 0 ||
	    x < 0 || y < 0 || x > displayWidth - width || y > displayHeight - height)
		return 0;
	crop[0] = (float)x / (float)displayWidth;
	crop[1] = (float)y / (float)displayHeight;
	crop[2] = (float)width / (float)displayWidth;
	crop[3] = (float)height / (float)displayHeight;
	return 1;
}

#ifdef __vita__
#include <platform/native_postfx_binaries.h>
#include <vitaGL.h>
#include <psp2/kernel/processmgr.h>
#include <stdio.h>

typedef struct NativePostFXVertex {
    float x, y, z;
    float u, v;
} NativePostFXVertex;

static GLuint s_texture;
static GLuint s_framebuffer;
static GLuint s_vao;
static GLuint s_vbo;
static GLuint s_program;
static GLuint s_vertex_shader;
static GLuint s_fragment_shader;
static GLint s_time_location;
static GLint s_crop_location;
static GLint s_output_size_location;
static GLint s_daltonic_green_location;
static GLint s_daltonic_blue_location;
static GLint s_parameter_locations[NATIVE_POSTFX_PARAMETER_COUNT];
static unsigned s_applied_settings_revision;
static int s_frame_width;
static int s_frame_height;
static int s_loaded_effect;
static int s_failed_effect;
static int s_target_failed;
static int s_pending;
static int s_quad_rect[6] = {-1, -1, -1, -1, -1, -1};

static int NativePostFX_Selected(void)
{
	return (gNativePostFXMode > NATIVE_POSTFX_OFF && gNativePostFXMode < NATIVE_POSTFX_MODE_COUNT)
	       ? gNativePostFXMode : NATIVE_POSTFX_OFF;
}

static void NativePostFX_UnloadShader(void)
{
    glUseProgram(0);
    if (s_program)
        glDeleteProgram(s_program);
    if (s_vertex_shader)
        glDeleteShader(s_vertex_shader);
    if (s_fragment_shader)
        glDeleteShader(s_fragment_shader);
    s_program = 0;
    s_vertex_shader = 0;
    s_fragment_shader = 0;
    s_loaded_effect = 0;
    s_time_location = -1;
    s_crop_location = -1;
    s_output_size_location = -1;
    s_daltonic_green_location = -1;
    s_daltonic_blue_location = -1;
    s_applied_settings_revision = 0;
}

static GLuint NativePostFX_LoadShader(const void *binary, size_t length, GLenum type)
{
    if (!binary || length < 16)
        return 0;
    GLuint shader = glCreateShader(type);
    if (shader)
        glShaderBinary(1, &shader, 0, binary, (GLsizei)length);
    return shader;
}

static const NativePostFXBinary *NativePostFX_FindBinary(const char *effect)
{
    for (unsigned i = 0; i < native_postfx_binary_count; ++i)
        if (strcmp(effect, native_postfx_binaries[i].name) == 0)
            return &native_postfx_binaries[i];
    return NULL;
}

static int NativePostFX_LoadProgram(int effect)
{
    if (effect == s_loaded_effect && s_program)
        return 1;
    if (effect == s_failed_effect)
        return 0;

    NativePostFX_UnloadShader();
    const NativePostFXShader *desc = &s_effects[effect];
    const NativePostFXBinary *binary = NativePostFX_FindBinary(desc->name);
    if (binary) {
        s_vertex_shader = NativePostFX_LoadShader(binary->vertex, binary->vertex_size,
                                               GL_CG_VERTEX_SHADER_EXT);
        s_fragment_shader = NativePostFX_LoadShader(binary->fragment, binary->fragment_size,
                                                 GL_CG_FRAGMENT_SHADER_EXT);
    }
    if (s_vertex_shader && s_fragment_shader) {
        s_program = glCreateProgram();
        if (s_program) {
            glAttachShader(s_program, s_vertex_shader);
            glAttachShader(s_program, s_fragment_shader);
            vglBindAttribLocation(s_program, 0, "position", 3, GL_FLOAT);
            vglBindAttribLocation(s_program, 1, "texcoord", 2, GL_FLOAT);
            glLinkProgram(s_program);
            GLint linked = GL_FALSE;
            glGetProgramiv(s_program, GL_LINK_STATUS, &linked);
            if (linked) {
                glUseProgram(s_program);
                GLint sampler = glGetUniformLocation(s_program, desc->sampler);
                if (sampler >= 0)
                    glUniform1i(sampler, 0);
                for (size_t i = 0; i < NATIVE_POSTFX_PARAMETER_COUNT; ++i) {
                    s_parameter_locations[i] = strcmp(native_postfx_parameters[i].effect,
                                                      desc->name) == 0 ?
                        glGetUniformLocation(s_program, native_postfx_parameters[i].uniform) : -1;
                }
                s_time_location = glGetUniformLocation(s_program, "iTime");
                s_crop_location = glGetUniformLocation(s_program, "uCrop");
                s_output_size_location = glGetUniformLocation(s_program, "uOutputSize");
                if (strcmp(desc->name, "daltonic") == 0) {
                    s_daltonic_green_location = glGetUniformLocation(s_program, "uGreenRow");
                    s_daltonic_blue_location = glGetUniformLocation(s_program, "uBlueRow");
                }
                glUseProgram(0);
                s_loaded_effect = effect;
                return 1;
            }
        }
    }

    if (s_program) {
        char log[512];
        GLsizei count = 0;
        glGetProgramInfoLog(s_program, sizeof(log), &count, log);
        fprintf(stderr, "CTR Native: post-processing shader %s link failed: %s\n",
                desc->name, count > 0 ? log : "(no log)");
    } else {
        fprintf(stderr, "CTR Native: failed to load post-processing shader %s\n", desc->name);
    }
    NativePostFX_UnloadShader();
    s_failed_effect = effect;
    return 0;
}

static int NativePostFX_CreateTarget(int width, int height)
{
    if (s_framebuffer && s_frame_width == width && s_frame_height == height)
        return 1;
    if (s_framebuffer)
        glDeleteFramebuffers(1, &s_framebuffer);
    if (s_texture)
        glDeleteTextures(1, &s_texture);
    s_framebuffer = 0;
    s_texture = 0;
    s_frame_width = s_frame_height = 0;

    glGenTextures(1, &s_texture);
    if (!s_texture)
        return 0;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glGenFramebuffers(1, &s_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, s_framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           s_texture, 0);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "CTR Native: post-processing framebuffer unavailable: 0x%x\n", (unsigned)status);
        glDeleteFramebuffers(1, &s_framebuffer);
        glDeleteTextures(1, &s_texture);
        s_framebuffer = s_texture = 0;
        return 0;
    }
    s_frame_width = width;
    s_frame_height = height;
    return 1;
}

static int NativePostFX_CreateQuad(void)
{
    if (s_vao && s_vbo)
        return 1;
    glGenVertexArrays(1, &s_vao);
    glGenBuffers(1, &s_vbo);
    if (!s_vao || !s_vbo)
        return 0;
    glBindVertexArray(s_vao);
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(NativePostFXVertex) * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(NativePostFXVertex), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(NativePostFXVertex),
                          (void *)(3 * sizeof(float)));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    s_quad_rect[0] = -1;
    return 1;
}

unsigned NativePostFX_TargetFramebuffer(int width, int height)
{
    int effect = NativePostFX_Selected();
    if (!effect) {
        if (s_framebuffer || s_program)
            NativePostFX_Shutdown();
        return 0;
    }
    if (width <= 0 || height <= 0 || width > 2048 || height > 2048 ||
        s_target_failed)
        return 0;
    if (!NativePostFX_LoadProgram(effect))
        return 0;
    if (!NativePostFX_CreateTarget(width, height) || !NativePostFX_CreateQuad()) {
        s_target_failed = 1;
        return 0;
    }
    s_pending = 1;
    return s_framebuffer;
}

int NativePostFX_Finish(int x, int y, int width, int height, int display_width,
                      int display_height)
{
    if (!s_pending)
        return 0;
    s_pending = 0;
	float crop[4];
	if (!s_program || !s_texture || !NativePostFX_CalculateCrop(x, y, width, height,
	                                                             display_width, display_height, crop))
		return 0;
	const float u0 = crop[0];
	const float v0 = crop[1];
	const float u1 = crop[0] + crop[2];
	const float v1 = crop[1] + crop[3];
    const NativePostFXVertex quad[4] = {
        {-1, -1, 0, u0, v0},
        { 1, -1, 0, u1, v0},
        {-1,  1, 0, u0, v1},
        { 1,  1, 0, u1, v1}
    };

    const GLboolean previous_stencil = glIsEnabled(GL_STENCIL_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, display_width, display_height);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(x, y, width, height);
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, y, width, height);
    glUseProgram(s_program);
    const unsigned revision = gNativePostFXRevision;
    if (s_applied_settings_revision != revision) {
        for (size_t i = 0; i < NATIVE_POSTFX_PARAMETER_COUNT; ++i) {
            const GLint location = s_parameter_locations[i];
            if (location < 0)
                continue;
            glUniform1f(location, (float)gNativePostFXParameterValues[i] * 0.01f);
        }
        if (strcmp(s_effects[s_loaded_effect].name, "daltonic") == 0) {
            static const float rows[4][6] = {
                {0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f},
                {0.50894941f, 0.49105389f, 0.00000069f, 0.61732665f, -0.61732265f, 1.00000083f},
                {0.20232475f, 0.79767449f, -0.00000015f, 0.51741093f, -0.51741286f, 0.99999962f},
                {-0.13853643f, 1.13853827f, 0.00000021f, 3.36558457f, -3.36562933f, 0.99999501f}
            };
            int mode = 0;
            if (gNativePostFXParameterValues[0] > 100)
                mode = 1;
            if (gNativePostFXParameterValues[1] > 100)
                mode = 2;
            if (gNativePostFXParameterValues[2] > 100)
                mode = 3;
            if (s_daltonic_green_location >= 0)
                glUniform3f(s_daltonic_green_location, rows[mode][0], rows[mode][1], rows[mode][2]);
            if (s_daltonic_blue_location >= 0)
                glUniform3f(s_daltonic_blue_location, rows[mode][3], rows[mode][4], rows[mode][5]);
        }
        s_applied_settings_revision = revision;
    }
    if (s_time_location >= 0)
        glUniform1f(s_time_location, (float)sceKernelGetProcessTimeLow() / 1000000.0f);
    if (s_crop_location >= 0)
        glUniform4f(s_crop_location, crop[0], crop[1], crop[2], crop[3]);
    if (s_output_size_location >= 0)
        glUniform2f(s_output_size_location, (float)width, (float)height);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_texture);
    glBindVertexArray(s_vao);
    if (s_quad_rect[0] != x || s_quad_rect[1] != y ||
        s_quad_rect[2] != width || s_quad_rect[3] != height ||
        s_quad_rect[4] != display_width || s_quad_rect[5] != display_height) {
        glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad), quad);
        s_quad_rect[0] = x;
        s_quad_rect[1] = y;
        s_quad_rect[2] = width;
        s_quad_rect[3] = height;
        s_quad_rect[4] = display_width;
        s_quad_rect[5] = display_height;
    }
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glUseProgram(0);
    glDisable(GL_SCISSOR_TEST);
    if (previous_stencil)
        glEnable(GL_STENCIL_TEST);
    glViewport(0, 0, display_width, display_height);
    return 1;
}

void NativePostFX_Shutdown(void)
{
    s_pending = 0;
    NativePostFX_UnloadShader();
    if (s_vao)
        glDeleteVertexArrays(1, &s_vao);
    if (s_vbo)
        glDeleteBuffers(1, &s_vbo);
    if (s_framebuffer)
        glDeleteFramebuffers(1, &s_framebuffer);
    if (s_texture)
        glDeleteTextures(1, &s_texture);
    s_vao = s_vbo = s_framebuffer = s_texture = 0;
    s_frame_width = s_frame_height = 0;
    s_failed_effect = 0;
    s_target_failed = 0;
    s_quad_rect[0] = -1;
}

#endif
