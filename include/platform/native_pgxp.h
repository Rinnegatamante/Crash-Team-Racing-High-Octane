#ifndef NATIVE_PGXP_H
#define NATIVE_PGXP_H

#include <macros.h>
#include <stddef.h>

enum NativePgxpMode
{
	NATIVE_PGXP_MODE_OFF,
	// Sub-pixel vertex positions, retail affine texture mapping.
	NATIVE_PGXP_MODE_GEOMETRY,
	// Sub-pixel vertex positions and perspective-correct texture mapping.
	NATIVE_PGXP_MODE_FULL,
	NATIVE_PGXP_MODE_TEXTURES,
	NATIVE_PGXP_MODE_COUNT,
};

static inline int NativePgxp_NormalizeMode(int mode)
{
	if (mode < NATIVE_PGXP_MODE_OFF || mode >= NATIVE_PGXP_MODE_COUNT)
		return NATIVE_PGXP_MODE_OFF;
#ifdef __vita__
	return mode == NATIVE_PGXP_MODE_TEXTURES ? mode : NATIVE_PGXP_MODE_OFF;
#else
	return mode;
#endif
}

static inline int NativePgxp_NextMode(int mode)
{
	mode = NativePgxp_NormalizeMode(mode);
#ifdef __vita__
	return mode == NATIVE_PGXP_MODE_OFF ? NATIVE_PGXP_MODE_TEXTURES : NATIVE_PGXP_MODE_OFF;
#else
	// Menu order differs from the numeric values stored in config.ini.
	switch (mode)
	{
		case NATIVE_PGXP_MODE_OFF: return NATIVE_PGXP_MODE_TEXTURES;
		case NATIVE_PGXP_MODE_TEXTURES: return NATIVE_PGXP_MODE_GEOMETRY;
		case NATIVE_PGXP_MODE_GEOMETRY: return NATIVE_PGXP_MODE_FULL;
		default: return NATIVE_PGXP_MODE_OFF;
	}
#endif
}

extern int gNativePgxpMode;
extern int gNativePgxpRequestedMode;
extern int gNativePgxpIntegerNclipEnabled;

typedef struct
{
	float x;
	float y;
	// View-space depth; <= 0 when the projection has no usable depth.
	float w;
	// Retail packed SXY (x low, y high) this vertex was truncated to.
	u32 value;
} NativePgxpVertex;

#define NATIVE_PGXP_ACTIVE() (gNativePgxpMode != NATIVE_PGXP_MODE_OFF)
#define NATIVE_PGXP_GEOMETRY_ACTIVE() (gNativePgxpMode == NATIVE_PGXP_MODE_GEOMETRY || gNativePgxpMode == NATIVE_PGXP_MODE_FULL)
#define NATIVE_PGXP_TEXTURES_ACTIVE() (gNativePgxpMode == NATIVE_PGXP_MODE_TEXTURES || gNativePgxpMode == NATIVE_PGXP_MODE_FULL)
#define NATIVE_VERTEX_TRACKING_ACTIVE() NATIVE_PGXP_GEOMETRY_ACTIVE()

void NativePgxp_SetPrimRegion(const void *start, size_t size);

// GTE side: mirror of the SXY0..SXY2 FIFO.
void NativePgxp_GteProject(float x, float y, float w, u32 value);
// Fast GTE TEXTURES path: projected SXY is retail; only SZ3 is required.
void NativePgxp_GteProjectTexture(u32 value, float w);
void NativePgxp_GteWriteSXY(int reg, u32 value);
void NativePgxp_GteReadSXY(int reg, u32 value);

void NativePgxp_StoreGteSXY(const void *dst, int reg, u32 value);
void NativePgxp_CopyXY(const void *dst, const void *src, u32 value);

void NativePgxp_SetPosition(const void *key, const s16 *vector, const float *precise);
void NativePgxp_ClearPosition(const void *key);
// Returns 1 with the exact position, or 0 with `vector` converted to float.
int NativePgxp_GetPosition(const void *key, const s16 *vector, float *out);
void NativePgxp_CopyPosition(const void *dstKey, const void *srcKey, const s16 *vector);

void NativePgxp_GteLoadSXY(int reg, const void *src, u32 value);
// NCLIP from the precise SXY0..SXY2; returns 0 when any is unknown.
int NativePgxp_GteNclip(u32 sxy0, u32 sxy1, u32 sxy2, s64 *out);

void NativePgxp_GteInvalidateInput(int reg);
void NativePgxp_GteSetInput(int slot, const s16 *vector, const float *precise);
int NativePgxp_GteGetInput(int slot, s16 vx, s16 vy, s16 vz, double *out);
// Exact MAC1..MAC3 of the last MVMVA, matched against the IR1..IR3 it produced.
void NativePgxp_GteSetMvmvaResult(const double *precise, s16 ir1, s16 ir2, s16 ir3);
int NativePgxp_GteGetMvmvaResult(const s16 *ir, float *out);

void NativePgxp_Transform(const double *rotation, const double *translation, const double *input, double *result);

// GPU side.
int NativePgxp_Lookup(const void *addr, u32 value, NativePgxpVertex *out);
void NativePgxp_EndFrame(void);

#endif
