#include <macros.h>
#include "platform/native_pgxp.h"

#include <stdlib.h>
#include <string.h>

int gNativePgxpMode = NATIVE_PGXP_MODE_OFF;
int gNativePgxpRequestedMode = NATIVE_PGXP_MODE_OFF;

int gNativePgxpIntegerNclipEnabled = 1;

#define NATIVE_PGXP_RECENT_COUNT 64
#define NATIVE_PGXP_RECENT_BUCKET_COUNT 256
#define NATIVE_PGXP_CACHE_BITS   12
#define NATIVE_PGXP_CACHE_COUNT  (1u << NATIVE_PGXP_CACHE_BITS)

typedef struct
{
	NativePgxpVertex vertex;
	b32 valid;
} NativePgxpRegister;

typedef struct
{
	NativePgxpVertex vertex;
	u32 generation;
} NativePgxpPrimSlot;

typedef struct
{
	const void *key;
	NativePgxpVertex vertex;
	u32 generation;
} NativePgxpCacheSlot;

global_variable NativePgxpRegister s_pgxpFifo[3];
global_variable NativePgxpVertex s_pgxpRecent[NATIVE_PGXP_RECENT_COUNT];
global_variable u32 s_pgxpRecentGeneration[NATIVE_PGXP_RECENT_COUNT];
global_variable u32 s_pgxpRecentHead;
// Entries are ring index+1 (0 means empty). Eviction unlinks the old slot.
global_variable u8 s_pgxpRecentBucketHeads[NATIVE_PGXP_RECENT_BUCKET_COUNT];
global_variable u8 s_pgxpRecentNext[NATIVE_PGXP_RECENT_COUNT];
global_variable u8 s_pgxpRecentPrev[NATIVE_PGXP_RECENT_COUNT];

global_variable NativePgxpCacheSlot s_pgxpCache[NATIVE_PGXP_CACHE_COUNT];
global_variable NativePgxpPrimSlot *s_pgxpPrimSlots;
global_variable uintptr_t s_pgxpPrimStart;
global_variable uintptr_t s_pgxpPrimSize;
global_variable u32 s_pgxpGeneration = 1;

void NativePgxp_SetPrimRegion(const void *start, size_t size)
{
	const uintptr_t regionStart = (uintptr_t)start;
	const uintptr_t regionSize = (uintptr_t)size & ~(uintptr_t)3;

	if ((regionStart == s_pgxpPrimStart) && (regionSize == s_pgxpPrimSize) && (s_pgxpPrimSlots != NULL))
	{
		return;
	}

	free(s_pgxpPrimSlots);
	s_pgxpPrimSlots = (NativePgxpPrimSlot *)calloc(regionSize / sizeof(u32), sizeof(NativePgxpPrimSlot));
	s_pgxpPrimStart = regionStart;
	s_pgxpPrimSize = (s_pgxpPrimSlots != NULL) ? regionSize : 0;
}

internal NativePgxpPrimSlot *NativePgxp_PrimSlot(const void *addr)
{
	const uintptr_t offset = (uintptr_t)addr - s_pgxpPrimStart;

	if (offset >= s_pgxpPrimSize)
	{
		return NULL;
	}

	return &s_pgxpPrimSlots[offset / sizeof(u32)];
}

internal NativePgxpCacheSlot *NativePgxp_CacheSlot(const void *addr)
{
	const u32 hash = (u32)((uintptr_t)addr >> 1) * 2654435761u;

	return &s_pgxpCache[hash >> (32 - NATIVE_PGXP_CACHE_BITS)];
}

internal void NativePgxp_SetShadow(const void *dst, NativePgxpVertex vertex)
{
	NativePgxpPrimSlot *primSlot = NativePgxp_PrimSlot(dst);

	if (primSlot != NULL)
	{
		primSlot->vertex = vertex;
		primSlot->generation = s_pgxpGeneration;
		return;
	}

	NativePgxpCacheSlot *cacheSlot = NativePgxp_CacheSlot(dst);
	cacheSlot->key = dst;
	cacheSlot->vertex = vertex;
	cacheSlot->generation = s_pgxpGeneration;
}

internal void NativePgxp_ClearShadow(const void *dst)
{
	NativePgxpPrimSlot *primSlot = NativePgxp_PrimSlot(dst);

	if (primSlot != NULL)
	{
		primSlot->generation = 0;
		return;
	}

	NativePgxpCacheSlot *cacheSlot = NativePgxp_CacheSlot(dst);
	if (cacheSlot->key == dst)
	{
		cacheSlot->generation = 0;
	}
}

internal const NativePgxpVertex *NativePgxp_FindShadow(const void *addr, u32 value)
{
	const NativePgxpPrimSlot *primSlot = NativePgxp_PrimSlot(addr);

	if (primSlot != NULL)
	{
		if ((primSlot->generation == s_pgxpGeneration) && (primSlot->vertex.value == value))
		{
			return &primSlot->vertex;
		}
		return NULL;
	}

	const NativePgxpCacheSlot *cacheSlot = NativePgxp_CacheSlot(addr);
	if ((cacheSlot->key == addr) && (cacheSlot->generation == s_pgxpGeneration) && (cacheSlot->vertex.value == value))
	{
		return &cacheSlot->vertex;
	}

	return NULL;
}

internal u32 NativePgxp_RecentBucket(u32 value)
{
	const u32 mixed = (value ^ (value >> 16)) * 2654435761u;
	return mixed >> 24;
}

internal const NativePgxpVertex *NativePgxp_FindRecent(u32 value)
{
	const NativePgxpVertex *match = NULL;
	const u32 bucket = NativePgxp_RecentBucket(value);
	for (u8 entry = s_pgxpRecentBucketHeads[bucket]; entry != 0;)
	{
		const u32 index = (u32)entry - 1;
		entry = s_pgxpRecentNext[index];

		if ((s_pgxpRecentGeneration[index] == s_pgxpGeneration) && (s_pgxpRecent[index].value == value))
		{
			const NativePgxpVertex *candidate = &s_pgxpRecent[index];
			if (match != NULL &&
			    (match->x != candidate->x || match->y != candidate->y || match->w != candidate->w))
				return NULL;
			match = candidate;
		}
	}

	return match;
}

internal void NativePgxp_RememberRecent(const NativePgxpVertex *vertex)
{
	const u32 last = (s_pgxpRecentHead - 1) & (NATIVE_PGXP_RECENT_COUNT - 1);

	if ((s_pgxpRecentGeneration[last] == s_pgxpGeneration) && (s_pgxpRecent[last].value == vertex->value) && (s_pgxpRecent[last].x == vertex->x) &&
	    (s_pgxpRecent[last].y == vertex->y) && (s_pgxpRecent[last].w == vertex->w))
	{
		return;
	}

	const u32 index = s_pgxpRecentHead++ & (NATIVE_PGXP_RECENT_COUNT - 1);
	if (s_pgxpRecentGeneration[index] == s_pgxpGeneration)
	{
		// Remove the oldest ring entry from the hash chain before reusing it.
		const u32 oldBucket = NativePgxp_RecentBucket(s_pgxpRecent[index].value);
		const u8 prev = s_pgxpRecentPrev[index];
		const u8 next = s_pgxpRecentNext[index];
		if (prev)
			s_pgxpRecentNext[prev - 1] = next;
		else
			s_pgxpRecentBucketHeads[oldBucket] = next;
		if (next)
			s_pgxpRecentPrev[next - 1] = prev;
	}
	s_pgxpRecent[index] = *vertex;
	s_pgxpRecentGeneration[index] = s_pgxpGeneration;
	const u32 bucket = NativePgxp_RecentBucket(vertex->value);
	const u8 entry = (u8)(index + 1);
	const u8 previousHead = s_pgxpRecentBucketHeads[bucket];
	s_pgxpRecentPrev[index] = 0;
	s_pgxpRecentNext[index] = previousHead;
	if (previousHead)
		s_pgxpRecentPrev[previousHead - 1] = entry;
	s_pgxpRecentBucketHeads[bucket] = entry;
}

internal NativePgxpRegister *NativePgxp_FifoRegister(int reg)
{
	// SXYP (15) reads back as SXY2.
	return &s_pgxpFifo[(reg >= 14) ? 2 : (reg - 12)];
}

internal const NativePgxpVertex *NativePgxp_FifoMatch(int reg, u32 value)
{
	const NativePgxpRegister *fifoReg = NativePgxp_FifoRegister(reg);

	return (fifoReg->valid && (fifoReg->vertex.value == value)) ? &fifoReg->vertex : NULL;
}

void NativePgxp_GteProject(float x, float y, float w, u32 value)
{
	s_pgxpFifo[0] = s_pgxpFifo[1];
	s_pgxpFifo[1] = s_pgxpFifo[2];
	s_pgxpFifo[2].vertex.x = x;
	s_pgxpFifo[2].vertex.y = y;
	s_pgxpFifo[2].vertex.w = w;
	s_pgxpFifo[2].vertex.value = value;
	s_pgxpFifo[2].valid = 1;
}

void NativePgxp_GteProjectTexture(u32 value, float w)
{
	s_pgxpFifo[0] = s_pgxpFifo[1];
	s_pgxpFifo[1] = s_pgxpFifo[2];
	NativePgxpVertex *vertex = &s_pgxpFifo[2].vertex;
	vertex->value = value;
	vertex->x = (float)(s16)value;
	vertex->y = (float)(s16)(value >> 16);
	vertex->w = w;
	s_pgxpFifo[2].valid = 1;
}

void NativePgxp_GteWriteSXY(int reg, u32 value)
{
	NativePgxpRegister written = {0};
	const NativePgxpVertex *source = NULL;
	int ambiguous = 0;

	for (int i = 2; i >= 0; i--)
	{
		if (s_pgxpFifo[i].valid && (s_pgxpFifo[i].vertex.value == value))
		{
			const NativePgxpVertex *candidate = &s_pgxpFifo[i].vertex;
			if (source != NULL &&
			    (source->x != candidate->x || source->y != candidate->y || source->w != candidate->w))
			{
				ambiguous = 1;
				break;
			}
			source = candidate;
		}
	}

	if (source == NULL && !ambiguous)
	{
		source = NativePgxp_FindRecent(value);
	}

	if (source != NULL && !ambiguous)
	{
		written.vertex = *source;
		written.valid = 1;
	}

	if (reg == 15)
	{
		s_pgxpFifo[0] = s_pgxpFifo[1];
		s_pgxpFifo[1] = s_pgxpFifo[2];
		s_pgxpFifo[2] = written;
	}
	else
	{
		s_pgxpFifo[reg - 12] = written;
	}
}

void NativePgxp_GteLoadSXY(int reg, const void *src, u32 value)
{
	const NativePgxpVertex *vertex = NativePgxp_FindShadow(src, value);
	NativePgxpRegister *fifoReg = NativePgxp_FifoRegister(reg);

	if (vertex != NULL)
	{
		fifoReg->vertex = *vertex;
		fifoReg->valid = 1;
	}
	else
	{
		fifoReg->valid = 0;
	}
}

int NativePgxp_GteNclip(u32 sxy0, u32 sxy1, u32 sxy2, s64 *out)
{
	if (!NATIVE_PGXP_GEOMETRY_ACTIVE())
		return 0;
	if (gNativePgxpIntegerNclipEnabled)
		return 0;

	const NativePgxpVertex *v0 = NativePgxp_FifoMatch(12, sxy0);
	const NativePgxpVertex *v1 = NativePgxp_FifoMatch(13, sxy1);
	const NativePgxpVertex *v2 = NativePgxp_FifoMatch(14, sxy2);

	if ((v0 == NULL) || (v1 == NULL) || (v2 == NULL))
	{
		return 0;
	}
#ifdef __vita__
	if (!(v0->w > 0.0f && v1->w > 0.0f && v2->w > 0.0f))
		return 0;
#endif

	double nclip = ((double)v0->x * v1->y) + ((double)v1->x * v2->y) + ((double)v2->x * v0->y) - ((double)v0->x * v2->y) - ((double)v1->x * v0->y) -
	               ((double)v2->x * v1->y);

	const double nclipAbs = (nclip < 0.0) ? -nclip : nclip;
	if ((nclipAbs > 0.1) && (nclipAbs < 1.0))
	{
		nclip += (nclip < 0.0) ? -1.0 : 1.0;
	}

	*out = (s64)nclip;
	return 1;
}

void NativePgxp_GteReadSXY(int reg, u32 value)
{
	const NativePgxpVertex *vertex = NativePgxp_FifoMatch(reg, value);

	if (vertex != NULL)
	{
		NativePgxp_RememberRecent(vertex);
	}
}

void NativePgxp_StoreGteSXY(const void *dst, int reg, u32 value)
{
	const NativePgxpVertex *vertex = NativePgxp_FifoMatch(reg, value);

	if (vertex != NULL)
	{
		NativePgxp_SetShadow(dst, *vertex);
	}
	else
	{
		NativePgxp_ClearShadow(dst);
	}
}

void NativePgxp_CopyXY(const void *dst, const void *src, u32 value)
{
	const NativePgxpVertex *vertex = NativePgxp_FindShadow(src, value);

	if (vertex != NULL)
	{
		NativePgxp_SetShadow(dst, *vertex);
	}
	else
	{
		NativePgxp_ClearShadow(dst);
	}
}

#define NATIVE_PGXP_POSITION_BITS  13
#define NATIVE_PGXP_POSITION_COUNT (1u << NATIVE_PGXP_POSITION_BITS)

typedef struct
{
	const void *key;
	s16 vector[3];
	float precise[3];
	u32 generation;
} NativePgxpPositionSlot;

typedef struct
{
	s16 vector[3];
	b32 valid;
	double precise[3];
} NativePgxpInput;

global_variable NativePgxpPositionSlot s_pgxpPositions[NATIVE_PGXP_POSITION_COUNT];
global_variable NativePgxpInput s_pgxpInputs[3];
global_variable NativePgxpInput s_pgxpMvmvaResult;

internal NativePgxpPositionSlot *NativePgxp_PositionSlot(const void *key)
{
	const u32 hash = (u32)((uintptr_t)key >> 1) * 2654435761u;

	return &s_pgxpPositions[hash >> (32 - NATIVE_PGXP_POSITION_BITS)];
}

void NativePgxp_SetPosition(const void *key, const s16 *vector, const float *precise)
{
	NativePgxpPositionSlot *slot = NativePgxp_PositionSlot(key);

	slot->key = key;
	slot->vector[0] = vector[0];
	slot->vector[1] = vector[1];
	slot->vector[2] = vector[2];
	slot->precise[0] = precise[0];
	slot->precise[1] = precise[1];
	slot->precise[2] = precise[2];
	slot->generation = s_pgxpGeneration;
}

void NativePgxp_ClearPosition(const void *key)
{
	NativePgxpPositionSlot *slot = NativePgxp_PositionSlot(key);

	if (slot->key == key)
	{
		slot->generation = 0;
	}
}

int NativePgxp_GetPosition(const void *key, const s16 *vector, float *out)
{
	const NativePgxpPositionSlot *slot = NativePgxp_PositionSlot(key);

	if ((slot->key == key) && (slot->generation == s_pgxpGeneration) && (slot->vector[0] == vector[0]) && (slot->vector[1] == vector[1]) &&
	    (slot->vector[2] == vector[2]))
	{
		out[0] = slot->precise[0];
		out[1] = slot->precise[1];
		out[2] = slot->precise[2];
		return 1;
	}

	out[0] = (float)vector[0];
	out[1] = (float)vector[1];
	out[2] = (float)vector[2];
	return 0;
}

void NativePgxp_CopyPosition(const void *dstKey, const void *srcKey, const s16 *vector)
{
	float precise[3];

	if (dstKey == srcKey)
	{
		return;
	}

	if (NativePgxp_GetPosition(srcKey, vector, precise))
	{
		NativePgxp_SetPosition(dstKey, vector, precise);
	}
	else
	{
		NativePgxp_ClearPosition(dstKey);
	}
}

void NativePgxp_GteInvalidateInput(int reg)
{
	if ((reg >= 0) && (reg <= 5))
	{
		s_pgxpInputs[reg >> 1].valid = 0;
	}
}

void NativePgxp_GteSetInput(int slot, const s16 *vector, const float *precise)
{
	NativePgxpInput *input = &s_pgxpInputs[slot];

	input->vector[0] = vector[0];
	input->vector[1] = vector[1];
	input->vector[2] = vector[2];
	input->precise[0] = precise[0];
	input->precise[1] = precise[1];
	input->precise[2] = precise[2];
	input->valid = 1;
}

int NativePgxp_GteGetInput(int slot, s16 vx, s16 vy, s16 vz, double *out)
{
	if ((slot < 0) || (slot >= 3))
		return 0;
	const NativePgxpInput *input = &s_pgxpInputs[slot];

	if (!input->valid || (input->vector[0] != vx) || (input->vector[1] != vy) || (input->vector[2] != vz))
	{
		return 0;
	}

	out[0] = input->precise[0];
	out[1] = input->precise[1];
	out[2] = input->precise[2];
	return 1;
}

void NativePgxp_GteSetMvmvaResult(const double *precise, s16 ir1, s16 ir2, s16 ir3)
{
	s_pgxpMvmvaResult.vector[0] = ir1;
	s_pgxpMvmvaResult.vector[1] = ir2;
	s_pgxpMvmvaResult.vector[2] = ir3;
	s_pgxpMvmvaResult.precise[0] = precise[0];
	s_pgxpMvmvaResult.precise[1] = precise[1];
	s_pgxpMvmvaResult.precise[2] = precise[2];
	s_pgxpMvmvaResult.valid = 1;
}

int NativePgxp_GteGetMvmvaResult(const s16 *ir, float *out)
{
	if (!s_pgxpMvmvaResult.valid || (s_pgxpMvmvaResult.vector[0] != ir[0]) || (s_pgxpMvmvaResult.vector[1] != ir[1]) || (s_pgxpMvmvaResult.vector[2] != ir[2]))
	{
		return 0;
	}

	out[0] = (float)s_pgxpMvmvaResult.precise[0];
	out[1] = (float)s_pgxpMvmvaResult.precise[1];
	out[2] = (float)s_pgxpMvmvaResult.precise[2];
	return 1;
}

void NativePgxp_Transform(const double *rotation, const double *translation, const double *input, double *result)
{
	for (int row = 0; row < 3; row++)
	{
		const double *r = &rotation[row * 3];
		result[row] = r[0] * input[0] + r[1] * input[1] + r[2] * input[2] + translation[row] * 4096.0;
	}
}

int NativePgxp_Lookup(const void *addr, u32 value, NativePgxpVertex *out)
{
	const NativePgxpVertex *vertex = NativePgxp_FindShadow(addr, value);

	if (vertex == NULL)
	{
		return 0;
	}

	*out = *vertex;
	return 1;
}

void NativePgxp_EndFrame(void)
{
	s_pgxpGeneration++;
	if (s_pgxpGeneration == 0)
	{
		s_pgxpGeneration = 1;
	}
	memset(s_pgxpRecentBucketHeads, 0, sizeof(s_pgxpRecentBucketHeads));
	s_pgxpRecentHead = 0;
	memset(s_pgxpFifo, 0, sizeof(s_pgxpFifo));
	memset(s_pgxpInputs, 0, sizeof(s_pgxpInputs));
	s_pgxpMvmvaResult.valid = 0;
}
