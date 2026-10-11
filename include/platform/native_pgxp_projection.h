#ifndef NATIVE_PGXP_PROJECTION_H
#define NATIVE_PGXP_PROJECTION_H

#include <stdint.h>
#include <math.h>

static inline float NativePgxp_TextureDepthFromGte(unsigned int sz3, int reciprocal)
{
    return (sz3 >= 64 && sz3 <= 65535 && reciprocal < 0x1ffff)
               ? (float)sz3 : 0.0f;
}

static inline int NativePgxp_ProjectViewFloat(float viewX, float viewY, float viewZ,
                                               int shift, int lm, int32_t ofx, int32_t ofy,
                                               unsigned int h, float *screenX,
                                               float *screenY, float *viewDepth)
{
    const float depth = viewZ * (1.0f / 4096.0f);
    if (!(depth > 0.0f))
        return 0;

    const float scale = shift ? (1.0f / 4096.0f) : 1.0f;
    const float minIr = lm ? 0.0f : -32768.0f;
    const float irX = fmaxf(minIr, fminf(32767.0f, viewX * scale));
    const float irY = fmaxf(minIr, fminf(32767.0f, viewY * scale));
    *screenX = (float)ofx * (1.0f / 65536.0f) + irX * (float)h / depth;
    *screenY = (float)ofy * (1.0f / 65536.0f) + irY * (float)h / depth;
    *viewDepth = depth;
    return isfinite(*screenX) && isfinite(*screenY);
}

static inline int NativePgxp_VitaProjectionConsistent(float preciseX, float preciseY,
                                                       int retailX, int retailY)
{
    return fabsf(preciseX - (float)retailX) <= 8.0f &&
           fabsf(preciseY - (float)retailY) <= 8.0f;
}

#endif
