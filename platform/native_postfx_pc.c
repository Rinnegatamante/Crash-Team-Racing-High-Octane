#if !defined(__vita__) && !defined(__EMSCRIPTEN__)

#include <platform/native_postfx.h>
#include <platform/native_postfx_params.h>
#include <platform/native_glad.h>
#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>
#include "native_postfx_pc_shaders.h"

typedef struct NativePostFXPcProgram
{
    GLuint id;
    GLint crop;
    GLint outputSize;
    GLint framebufferSize;
    GLint time;
    GLint parameter[NATIVE_POSTFX_PARAMETER_COUNT];
    GLint greenRow;
    GLint blueRow;
    unsigned settingsRevision;
} NativePostFXPcProgram;

static NativePostFXPcProgram s_postfxPcPrograms[NATIVE_POSTFX_MODE_COUNT];
static unsigned s_postfxPcFailedPrograms;
static GLuint s_postfxPcTexture;
static GLuint s_postfxPcFramebuffer;
static GLuint s_postfxPcVAO;
static int s_postfxPcWidth;
static int s_postfxPcHeight;
static int s_postfxPcFailedWidth;
static int s_postfxPcFailedHeight;
static int s_postfxPcPending;

static void NativePostFXPc_ReleaseTarget(void)
{
    if (s_postfxPcFramebuffer)
        glDeleteFramebuffers(1, &s_postfxPcFramebuffer);
    if (s_postfxPcTexture)
        glDeleteTextures(1, &s_postfxPcTexture);
    s_postfxPcFramebuffer = 0;
    s_postfxPcTexture = 0;
    s_postfxPcWidth = s_postfxPcHeight = 0;
}

static GLuint NativePostFXPc_Compile(GLenum stage, const char *defines, const char *body)
{
    const GLchar *sources[3];
    GLsizei sourceCount = 0;
    GLuint shader = glCreateShader(stage);
    if (!shader)
        return 0;
    if (stage == GL_FRAGMENT_SHADER)
    {
        sources[sourceCount++] = "#version 140\n";
        sources[sourceCount++] = defines;
    }
    sources[sourceCount++] = body;
    glShaderSource(shader, sourceCount, sources, NULL);
    glCompileShader(shader);
    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE)
    {
        char log[2048];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        fprintf(stderr, "High Octane Post FX: %s shader compile failed: %s\n",
                stage == GL_VERTEX_SHADER ? "vertex" : "fragment", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static NativePostFXPcProgram *NativePostFXPc_LoadProgram(int effect)
{
    NativePostFXPcProgram *program = &s_postfxPcPrograms[effect];
    if (program->id)
        return program;
    if (s_postfxPcFailedPrograms & (1u << effect))
        return NULL;

    char defines[64];
    snprintf(defines, sizeof(defines), "#define POSTFX_EFFECT %d\n", effect);
    GLuint vertex = NativePostFXPc_Compile(GL_VERTEX_SHADER, NULL, native_postfx_pc_vert);
    GLuint fragment = NativePostFXPc_Compile(GL_FRAGMENT_SHADER, defines, native_postfx_pc_frag);
    GLuint linkedProgram = 0;
    if (vertex && fragment)
    {
        linkedProgram = glCreateProgram();
        if (linkedProgram)
        {
            glAttachShader(linkedProgram, vertex);
            glAttachShader(linkedProgram, fragment);
            glLinkProgram(linkedProgram);
            GLint linked = GL_FALSE;
            glGetProgramiv(linkedProgram, GL_LINK_STATUS, &linked);
            if (linked != GL_TRUE)
            {
                char log[2048];
                glGetProgramInfoLog(linkedProgram, sizeof(log), NULL, log);
                fprintf(stderr, "High Octane Post FX: %s program link failed: %s\n",
                        NativePostFX_GetName(effect), log);
                glDeleteProgram(linkedProgram);
                linkedProgram = 0;
            }
        }
    }
    if (vertex)
        glDeleteShader(vertex);
    if (fragment)
        glDeleteShader(fragment);
    if (!linkedProgram)
    {
        s_postfxPcFailedPrograms |= (1u << effect);
        return NULL;
    }

    program->id = linkedProgram;
    program->crop = glGetUniformLocation(linkedProgram, "uCrop");
    program->outputSize = glGetUniformLocation(linkedProgram, "uOutputSize");
    program->framebufferSize = glGetUniformLocation(linkedProgram, "uFramebufferSize");
    program->time = glGetUniformLocation(linkedProgram, "iTime");
    program->greenRow = glGetUniformLocation(linkedProgram, "uGreenRow");
    program->blueRow = glGetUniformLocation(linkedProgram, "uBlueRow");
    const char *name = NativePostFX_GetName(effect);
    for (int i = 0; i < NATIVE_POSTFX_PARAMETER_COUNT; ++i)
    {
        const NativePostFXParameter *parameter = &native_postfx_parameters[i];
        program->parameter[i] = strcmp(parameter->effect, name) == 0 ?
                                glGetUniformLocation(linkedProgram, parameter->uniform) : -1;
    }
    glUseProgram(linkedProgram);
    GLint sampler = glGetUniformLocation(linkedProgram, "s0");
    if (sampler >= 0)
        glUniform1i(sampler, 0);
    glUseProgram(0);
    return program;
}

static int NativePostFXPc_CreateTarget(int width, int height)
{
    if (s_postfxPcFramebuffer && s_postfxPcWidth == width && s_postfxPcHeight == height)
        return 1;
    NativePostFXPc_ReleaseTarget();

    glGenTextures(1, &s_postfxPcTexture);
    if (!s_postfxPcTexture)
        return 0;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_postfxPcTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glGenFramebuffers(1, &s_postfxPcFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, s_postfxPcFramebuffer);
    if (s_postfxPcFramebuffer)
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               s_postfxPcTexture, 0);
    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        fprintf(stderr, "High Octane Post FX: framebuffer %dx%d unavailable (0x%x)\n",
                width, height, (unsigned)status);
        if (s_postfxPcFramebuffer)
            glDeleteFramebuffers(1, &s_postfxPcFramebuffer);
        glDeleteTextures(1, &s_postfxPcTexture);
        s_postfxPcFramebuffer = s_postfxPcTexture = 0;
        s_postfxPcFailedWidth = width;
        s_postfxPcFailedHeight = height;
        return 0;
    }
    if (!s_postfxPcVAO)
        glGenVertexArrays(1, &s_postfxPcVAO);
    if (!s_postfxPcVAO)
    {
        glDeleteFramebuffers(1, &s_postfxPcFramebuffer);
        glDeleteTextures(1, &s_postfxPcTexture);
        s_postfxPcFramebuffer = s_postfxPcTexture = 0;
        s_postfxPcFailedWidth = width;
        s_postfxPcFailedHeight = height;
        return 0;
    }
    s_postfxPcWidth = width;
    s_postfxPcHeight = height;
    return 1;
}

unsigned NativePostFX_TargetFramebuffer(int width, int height)
{
    s_postfxPcPending = 0;
    if (gNativePostFXMode <= NATIVE_POSTFX_OFF)
    {
        if (s_postfxPcFramebuffer)
            NativePostFXPc_ReleaseTarget();
        s_postfxPcFailedWidth = s_postfxPcFailedHeight = 0;
        return 0;
    }
    if (gNativePostFXMode >= NATIVE_POSTFX_MODE_COUNT ||
        width <= 0 || height <= 0 || width > 16384 || height > 16384 ||
        (width == s_postfxPcFailedWidth && height == s_postfxPcFailedHeight))
        return 0;
    if (!NativePostFXPc_LoadProgram(gNativePostFXMode) || !NativePostFXPc_CreateTarget(width, height))
        return 0;
    s_postfxPcPending = 1;
    return s_postfxPcFramebuffer;
}

static void NativePostFXPc_ApplyParameters(NativePostFXPcProgram *program)
{
    if (program->settingsRevision == gNativePostFXRevision)
        return;
    for (int i = 0; i < NATIVE_POSTFX_PARAMETER_COUNT; ++i)
    {
        if (program->parameter[i] >= 0)
            glUniform1f(program->parameter[i], 0.01f * (float)gNativePostFXParameterValues[i]);
    }
    if (program->greenRow >= 0 || program->blueRow >= 0)
    {
        static const float rows[4][6] =
        {
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
        if (program->greenRow >= 0)
            glUniform3fv(program->greenRow, 1, &rows[mode][0]);
        if (program->blueRow >= 0)
            glUniform3fv(program->blueRow, 1, &rows[mode][3]);
    }
    program->settingsRevision = gNativePostFXRevision;
}

int NativePostFX_Finish(int x, int y, int width, int height, int displayWidth, int displayHeight)
{
    if (!s_postfxPcPending)
        return 0;
    s_postfxPcPending = 0;
    float crop[4];
    if (!NativePostFX_CalculateCrop(x, y, width, height, displayWidth, displayHeight, crop))
        return 0;
    NativePostFXPcProgram *program = &s_postfxPcPrograms[gNativePostFXMode];
    if (!program->id || !s_postfxPcTexture)
        return 0;

    GLfloat previousClearColor[4];
    glGetFloatv(GL_COLOR_CLEAR_VALUE, previousClearColor);
    const GLboolean previousStencil = glIsEnabled(GL_STENCIL_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glViewport(0, 0, displayWidth, displayHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(previousClearColor[0], previousClearColor[1], previousClearColor[2], previousClearColor[3]);

    glViewport(x, y, width, height);
    glUseProgram(program->id);
    if (program->crop >= 0)
        glUniform4fv(program->crop, 1, crop);
    if (program->outputSize >= 0)
        glUniform2f(program->outputSize, (float)width, (float)height);
    if (program->framebufferSize >= 0)
        glUniform2f(program->framebufferSize, (float)displayWidth, (float)displayHeight);
    if (program->time >= 0)
        glUniform1f(program->time, (float)((double)SDL_GetTicks() * 0.001));
    NativePostFXPc_ApplyParameters(program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_postfxPcTexture);
    glBindVertexArray(s_postfxPcVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glUseProgram(0);
    if (previousStencil)
        glEnable(GL_STENCIL_TEST);
    glViewport(0, 0, displayWidth, displayHeight);
    return 1;
}

void NativePostFX_Shutdown(void)
{
    s_postfxPcPending = 0;
    for (int i = 1; i < NATIVE_POSTFX_MODE_COUNT; ++i)
    {
        if (s_postfxPcPrograms[i].id)
            glDeleteProgram(s_postfxPcPrograms[i].id);
    }
    memset(s_postfxPcPrograms, 0, sizeof(s_postfxPcPrograms));
    NativePostFXPc_ReleaseTarget();
    if (s_postfxPcVAO)
        glDeleteVertexArrays(1, &s_postfxPcVAO);
    s_postfxPcVAO = 0;
    s_postfxPcFailedWidth = s_postfxPcFailedHeight = 0;
    s_postfxPcFailedPrograms = 0;
}

#endif
