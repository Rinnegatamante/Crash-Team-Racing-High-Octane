#include "platform/native_checkpoint.h"

#include <common.h>
#include <macros.h>

#include <platform.h>
#include "ctr_scratchpad.h"
#include "platform/native_memory.h"
#include "platform/native_state.h"

#include <stdlib.h>
#include <string.h>

#if defined(__vita__)
extern char _start[];
extern char __exidx_end[];
extern char __data_start[];
extern char _end[];
#endif

#define NATIVE_CHECKPOINT_FOURCC(a, b, c, d) ((u32)(a) | ((u32)(b) << 8) | ((u32)(c) << 16) | ((u32)(d) << 24))

// NOTE(aalhendi): Whole-machine checkpoints are included after native memory
// and retail globals are defined, so they can snapshot the same process-local
// regions the game mutates.
#define NATIVE_CHECKPOINT_MAGIC              NATIVE_CHECKPOINT_FOURCC('C', 'T', 'R', 'C')
#define NATIVE_CHECKPOINT_VERSION            3u
#define NATIVE_CHECKPOINT_ADDRESS_RANGE_CAP  20u
#define NATIVE_CHECKPOINT_POINTER_SLOT_CAP   65536u
#define NATIVE_CHECKPOINT_POINTER_HASH_CAP   (NATIVE_CHECKPOINT_POINTER_SLOT_CAP * 2u)
#define NATIVE_CHECKPOINT_POINTER_HASH_MASK  (NATIVE_CHECKPOINT_POINTER_HASH_CAP - 1u)
#define NATIVE_CHECKPOINT_CREDITS_STRING_CAP 4096u
#define NATIVE_CHECKPOINT_LNG_STRING_CAP     4096u
#define NATIVE_CHECKPOINT_POOL_ITEM_CAP      128u

CTR_STATIC_ASSERT((NATIVE_CHECKPOINT_POINTER_HASH_CAP & NATIVE_CHECKPOINT_POINTER_HASH_MASK) == 0);

enum NativeCheckpointRegionKind
{
	NATIVE_CHECKPOINT_REGION_RDATA = NATIVE_CHECKPOINT_FOURCC('R', 'D', 'A', 'T'), // resident rdata globals
	NATIVE_CHECKPOINT_REGION_DATA = NATIVE_CHECKPOINT_FOURCC('D', 'A', 'T', 'A'),  // resident data globals
	NATIVE_CHECKPOINT_REGION_SDATA = NATIVE_CHECKPOINT_FOURCC('S', 'D', 'A', 'T'), // resident sdata globals
	NATIVE_CHECKPOINT_REGION_R230 = NATIVE_CHECKPOINT_FOURCC('R', '2', '3', '0'),  // main-menu overlay static data
	NATIVE_CHECKPOINT_REGION_D230 = NATIVE_CHECKPOINT_FOURCC('D', '2', '3', '0'),  // main-menu overlay data
	NATIVE_CHECKPOINT_REGION_V230 = NATIVE_CHECKPOINT_FOURCC('V', '2', '3', '0'),  // main-menu video BSS
	NATIVE_CHECKPOINT_REGION_R231 = NATIVE_CHECKPOINT_FOURCC('R', '2', '3', '1'),  // race/battle overlay static data
	NATIVE_CHECKPOINT_REGION_D231 = NATIVE_CHECKPOINT_FOURCC('D', '2', '3', '1'),  // race/battle overlay data
	NATIVE_CHECKPOINT_REGION_R232 = NATIVE_CHECKPOINT_FOURCC('R', '2', '3', '2'),  // adventure overlay static data
	NATIVE_CHECKPOINT_REGION_D232 = NATIVE_CHECKPOINT_FOURCC('D', '2', '3', '2'),  // adventure overlay data
	NATIVE_CHECKPOINT_REGION_R233 = NATIVE_CHECKPOINT_FOURCC('R', '2', '3', '3'),  // cutscene overlay static data
	NATIVE_CHECKPOINT_REGION_D233 = NATIVE_CHECKPOINT_FOURCC('D', '2', '3', '3'),  // cutscene overlay mutable data
	NATIVE_CHECKPOINT_REGION_GAR3 = NATIVE_CHECKPOINT_FOURCC('G', 'A', 'R', '3'),  // garage runtime state
	NATIVE_CHECKPOINT_REGION_CRD3 = NATIVE_CHECKPOINT_FOURCC('C', 'R', 'D', '3'),  // credits runtime state
	NATIVE_CHECKPOINT_REGION_OXSM = NATIVE_CHECKPOINT_FOURCC('O', 'X', 'S', 'M'),  // native Oxide small-object pool
	NATIVE_CHECKPOINT_REGION_OXLG = NATIVE_CHECKPOINT_FOURCC('O', 'X', 'L', 'G'),  // native Oxide driver-object pool
	NATIVE_CHECKPOINT_REGION_MPAK = NATIVE_CHECKPOINT_FOURCC('M', 'P', 'A', 'K'),  // mempack backing store
	NATIVE_CHECKPOINT_REGION_SCRP = NATIVE_CHECKPOINT_FOURCC('S', 'C', 'R', 'P'),  // PS1 scratchpad RAM
	NATIVE_CHECKPOINT_REGION_PMAP = NATIVE_CHECKPOINT_FOURCC('P', 'M', 'A', 'P'),  // native pointer-map relocation slots
	NATIVE_CHECKPOINT_REGION_NATS = NATIVE_CHECKPOINT_FOURCC('N', 'A', 'T', 'S'),  // native subsystem state bundle
	NATIVE_CHECKPOINT_RANGE_CODE = NATIVE_CHECKPOINT_FOURCC('M', 'C', 'O', 'D'),   // executable module segment
	NATIVE_CHECKPOINT_RANGE_DATA = NATIVE_CHECKPOINT_FOURCC('M', 'D', 'A', 'T'),   // writable module segment
};

struct NativeCheckpointRegion
{
	u32 kind;
	u32 offset;
	u32 size;
};

struct NativeCheckpointAddressRange
{
	u32 kind;
	u32 start;
	u32 size;
};

struct NativeCheckpointPointerSlotRecord
{
	u32 slotRegion;
	u32 slotOffset;
};

struct NativeCheckpointPointerSlotState
{
	u32 count;
	u32 reserved[3];
	struct NativeCheckpointPointerSlotRecord records[NATIVE_CHECKPOINT_POINTER_SLOT_CAP];
};

struct NativeCheckpointPoolSet
{
	u32 activeCount;
	u32 takenCount;
	u8 slotActive[NATIVE_CHECKPOINT_POOL_ITEM_CAP];
	void *activeItems[NATIVE_CHECKPOINT_POOL_ITEM_CAP];
};

struct NativeCheckpointThreadSet
{
	u32 count;
	struct Thread *threads[NATIVE_CHECKPOINT_POOL_ITEM_CAP];
};

enum NativeCheckpointFieldRelocationKind
{
	NATIVE_CHECKPOINT_FIELD_POINTER,
	NATIVE_CHECKPOINT_FIELD_IMAGE_POINTER,
	NATIVE_CHECKPOINT_FIELD_POINTER_OR_IMAGE,
};

struct NativeCheckpointFieldRelocation
{
	u32 offset;
	u32 kind;
};

#define NATIVE_CHECKPOINT_FIELD_PTR(type, field)          {OFFSETOF(type, field), NATIVE_CHECKPOINT_FIELD_POINTER}
#define NATIVE_CHECKPOINT_FIELD_IMAGE(type, field)        {OFFSETOF(type, field), NATIVE_CHECKPOINT_FIELD_IMAGE_POINTER}
#define NATIVE_CHECKPOINT_FIELD_PTR_OR_IMAGE(type, field) {OFFSETOF(type, field), NATIVE_CHECKPOINT_FIELD_POINTER_OR_IMAGE}

struct NativeCheckpointHeader
{
	u32 magic;
	u32 version;
	u32 size;
	u32 regionCount;
	u32 psxRandSeed;
	s32 activeMempackIndex;
	u32 addressRangeCount;
	u32 codeAnchor;
	struct NativeCheckpointAddressRange addressRanges[NATIVE_CHECKPOINT_ADDRESS_RANGE_CAP];
	struct NativeCheckpointRegion regions[16];
};

global_variable void *s_nativeCheckpointPointerSlots[NATIVE_CHECKPOINT_POINTER_SLOT_CAP];
global_variable void *s_nativeCheckpointPointerSlotHash[NATIVE_CHECKPOINT_POINTER_HASH_CAP];
global_variable u32 s_nativeCheckpointPointerSlotCount;
global_variable int s_nativeCheckpointPointerSlotOverflow;
global_variable int s_nativeCheckpointRelocationValid;
global_variable u32 s_nativeCheckpointRelocationSlotCount;
global_variable int s_nativeCheckpointRelocationTracking;

internal int NativeCheckpoint_InitHeader(struct NativeCheckpointHeader *header);

internal u32 NativeCheckpoint_Align4(u32 value)
{
	return (value + 3u) & ~3u;
}

internal b32 NativeCheckpoint_PtrToU32(const void *ptr, u32 *out)
{
	uintptr_t value = (uintptr_t)ptr;

	if ((ptr == NULL) || (out == NULL) || (value > 0xffffffffu))
	{
		return 0;
	}

	*out = (u32)value;
	return 1;
}

internal b32 NativeCheckpoint_ReadU32Slot(const void *slot, u32 *out)
{
	if ((slot == NULL) || (out == NULL))
	{
		return 0;
	}

	memcpy(out, slot, sizeof(*out));
	return 1;
}

internal void NativeCheckpoint_WriteU32Slot(void *slot, u32 value)
{
	if (slot != NULL)
	{
		memcpy(slot, &value, sizeof(value));
	}
}

internal void NativeCheckpoint_ClearPointerSlots(void)
{
	s_nativeCheckpointPointerSlotCount = 0;
	s_nativeCheckpointPointerSlotOverflow = 0;
	s_nativeCheckpointRelocationSlotCount = 0;
	s_nativeCheckpointRelocationTracking = 0;
	memset(s_nativeCheckpointPointerSlotHash, 0, sizeof(s_nativeCheckpointPointerSlotHash));
}

internal int NativeCheckpoint_HashContainsPointerSlot(const void *slot)
{
	u32 hashIndex;

	if (slot == NULL)
	{
		return 0;
	}

	hashIndex = ((((u32)(uintptr_t)slot) >> 2) * 2654435761u) & NATIVE_CHECKPOINT_POINTER_HASH_MASK;
	for (u32 probe = 0; probe < NATIVE_CHECKPOINT_POINTER_HASH_CAP; probe++)
	{
		void *entry = s_nativeCheckpointPointerSlotHash[hashIndex];

		if (entry == slot)
		{
			return 1;
		}
		if (entry == NULL)
		{
			return 0;
		}
		hashIndex = (hashIndex + 1u) & NATIVE_CHECKPOINT_POINTER_HASH_MASK;
	}

	return 0;
}

internal int NativeCheckpoint_MarkRelocationSlot(void *slot)
{
	u32 hashIndex;

	if (!s_nativeCheckpointRelocationTracking)
	{
		return 1;
	}
	if (slot == NULL)
	{
		s_nativeCheckpointRelocationValid = 0;
		return 0;
	}
	if (NativeCheckpoint_HashContainsPointerSlot(slot))
	{
		return 0;
	}
	if (s_nativeCheckpointRelocationSlotCount >= NATIVE_CHECKPOINT_POINTER_SLOT_CAP)
	{
		s_nativeCheckpointPointerSlotOverflow = 1;
		s_nativeCheckpointRelocationValid = 0;
		return 0;
	}

	hashIndex = ((((u32)(uintptr_t)slot) >> 2) * 2654435761u) & NATIVE_CHECKPOINT_POINTER_HASH_MASK;
	while (s_nativeCheckpointPointerSlotHash[hashIndex] != NULL)
	{
		hashIndex = (hashIndex + 1u) & NATIVE_CHECKPOINT_POINTER_HASH_MASK;
	}

	s_nativeCheckpointPointerSlotHash[hashIndex] = slot;
	s_nativeCheckpointRelocationSlotCount++;
	return 1;
}

internal void NativeCheckpoint_BeginRelocationTracking(void)
{
	NativeCheckpoint_ClearPointerSlots();
	s_nativeCheckpointRelocationTracking = 1;
	s_nativeCheckpointRelocationValid = 1;
}

internal void NativeCheckpoint_AbortRelocationTracking(void)
{
	NativeCheckpoint_ClearPointerSlots();
}

void NativeCheckpoint_OnMempackArenaReset(void)
{
	NativeCheckpoint_ClearPointerSlots();
}

void NativeCheckpoint_RegisterPointerSlot(void *slot)
{
	u32 hashIndex;

	if (slot == NULL)
	{
		return;
	}

	hashIndex = ((((u32)(uintptr_t)slot) >> 2) * 2654435761u) & NATIVE_CHECKPOINT_POINTER_HASH_MASK;
	while (s_nativeCheckpointPointerSlotHash[hashIndex] != NULL)
	{
		if (s_nativeCheckpointPointerSlotHash[hashIndex] == slot)
		{
			return;
		}
		hashIndex = (hashIndex + 1u) & NATIVE_CHECKPOINT_POINTER_HASH_MASK;
	}
	if (s_nativeCheckpointPointerSlotCount >= NATIVE_CHECKPOINT_POINTER_SLOT_CAP)
	{
		s_nativeCheckpointPointerSlotOverflow = 1;
		return;
	}

	s_nativeCheckpointPointerSlotHash[hashIndex] = slot;
	s_nativeCheckpointPointerSlots[s_nativeCheckpointPointerSlotCount++] = slot;
}

internal int NativeCheckpoint_GetActiveMempackIndex(void)
{
	int i;

	for (i = 0; i < 4; i++)
	{
		if (sdata_static.PtrMempack == &sdata_static.mempack[i])
		{
			return i;
		}
	}

	if ((sdata_static.gameTracker.activeMempackIndex >= 0) && (sdata_static.gameTracker.activeMempackIndex < 4))
	{
		return sdata_static.gameTracker.activeMempackIndex;
	}

	return 0;
}

internal int NativeCheckpoint_GetRegionSize(u32 kind)
{
	switch (kind)
	{
	case NATIVE_CHECKPOINT_REGION_RDATA:
		return (int)sizeof(rdata);
	case NATIVE_CHECKPOINT_REGION_DATA:
		return (int)sizeof(data);
	case NATIVE_CHECKPOINT_REGION_SDATA:
		return (int)sizeof(sdata_static);
	case NATIVE_CHECKPOINT_REGION_R230:
		return (int)sizeof(R230);
	case NATIVE_CHECKPOINT_REGION_D230:
		return (int)sizeof(D230);
	case NATIVE_CHECKPOINT_REGION_V230:
		return (int)sizeof(V230);
	case NATIVE_CHECKPOINT_REGION_R231:
		return (int)sizeof(R231);
	case NATIVE_CHECKPOINT_REGION_D231:
		return (int)sizeof(D231);
	case NATIVE_CHECKPOINT_REGION_R232:
		return (int)sizeof(R232);
	case NATIVE_CHECKPOINT_REGION_D232:
		return (int)sizeof(D232);
	case NATIVE_CHECKPOINT_REGION_R233:
		return (int)sizeof(R233);
	case NATIVE_CHECKPOINT_REGION_D233:
		return (int)sizeof(D233);
	case NATIVE_CHECKPOINT_REGION_GAR3:
		return (int)sizeof(gGarage);
	case NATIVE_CHECKPOINT_REGION_CRD3:
		return (int)sizeof(creditsBSS) - OFFSETOF(struct Ovr233_Credits_BSS, creditThread);
	case NATIVE_CHECKPOINT_REGION_OXSM:
		return (int)sizeof(s_oxideSmallStackPool);
	case NATIVE_CHECKPOINT_REGION_OXLG:
		return (int)sizeof(s_oxideLargeStackPool);
	case NATIVE_CHECKPOINT_REGION_MPAK:
		return Platform_GetMempackBackingSize();
	case NATIVE_CHECKPOINT_REGION_SCRP:
		return (int)CTR_SCRATCHPAD_SIZE;
	case NATIVE_CHECKPOINT_REGION_PMAP:
		return (int)sizeof(struct NativeCheckpointPointerSlotState);
	case NATIVE_CHECKPOINT_REGION_NATS:
		return NativeState_GetSize();
	}

	return 0;
}

internal void *NativeCheckpoint_GetRegionPtr(u32 kind)
{
	switch (kind)
	{
	case NATIVE_CHECKPOINT_REGION_RDATA:
		return &rdata;
	case NATIVE_CHECKPOINT_REGION_DATA:
		return &data;
	case NATIVE_CHECKPOINT_REGION_SDATA:
		return &sdata_static;
	case NATIVE_CHECKPOINT_REGION_R230:
		return &R230;
	case NATIVE_CHECKPOINT_REGION_D230:
		return &D230;
	case NATIVE_CHECKPOINT_REGION_V230:
		return &V230;
	case NATIVE_CHECKPOINT_REGION_R231:
		return &R231;
	case NATIVE_CHECKPOINT_REGION_D231:
		return &D231;
	case NATIVE_CHECKPOINT_REGION_R232:
		return &R232;
	case NATIVE_CHECKPOINT_REGION_D232:
		return &D232;
	case NATIVE_CHECKPOINT_REGION_R233:
		return (void *)&R233;
	case NATIVE_CHECKPOINT_REGION_D233:
		return &D233;
	case NATIVE_CHECKPOINT_REGION_GAR3:
		return &gGarage;
	case NATIVE_CHECKPOINT_REGION_CRD3:
		return &creditsBSS.creditThread;
	case NATIVE_CHECKPOINT_REGION_OXSM:
		return s_oxideSmallStackPool;
	case NATIVE_CHECKPOINT_REGION_OXLG:
		return s_oxideLargeStackPool;
	case NATIVE_CHECKPOINT_REGION_MPAK:
		return Platform_GetMempackBacking();
	case NATIVE_CHECKPOINT_REGION_SCRP:
		return CTR_SCRATCHPAD_BASE;
	}

	return NULL;
}

internal int NativeCheckpoint_CaptureD233(void *dst, int dstSize)
{
	struct OverlayDATA_233 *state = (struct OverlayDATA_233 *)dst;

	if ((dst == NULL) || (dstSize != (int)sizeof(*state)))
	{
		return 0;
	}

	*state = D233;
	memset(state->cs_initMatrixTable, 0, sizeof(state->cs_initMatrixTable));

	return 1;
}

internal int NativeCheckpoint_RestoreD233(const void *src, int srcSize)
{
	const struct OverlayDATA_233 *state = (const struct OverlayDATA_233 *)src;

	if ((src == NULL) || (srcSize != (int)sizeof(*state)))
	{
		return 0;
	}

	D233 = *state;
	OVR233_RebuildInitMatrixTable();

	return 1;
}

internal int NativeCheckpoint_AddRawAddressRange(struct NativeCheckpointHeader *header, u32 kind, u32 start, u32 size)
{
	struct NativeCheckpointAddressRange *range;

	if ((header == NULL) || (start == 0) || (size == 0) || (size > 0xffffffffu - start) ||
	    (header->addressRangeCount >= NATIVE_CHECKPOINT_ADDRESS_RANGE_CAP))
	{
		return 0;
	}

	range = &header->addressRanges[header->addressRangeCount++];
	range->kind = kind;
	range->start = start;
	range->size = size;
	return 1;
}

internal int NativeCheckpoint_AddAddressRange(struct NativeCheckpointHeader *header, u32 kind)
{
	void *ptr = NativeCheckpoint_GetRegionPtr(kind);
	int size = NativeCheckpoint_GetRegionSize(kind);
	u32 start;

	if ((header == NULL) || (ptr == NULL) || (size <= 0))
	{
		return 0;
	}
	if (!NativeCheckpoint_PtrToU32(ptr, &start))
	{
		return 0;
	}

	return NativeCheckpoint_AddRawAddressRange(header, kind, start, (u32)size);
}

internal int NativeCheckpoint_FillAddressRanges(struct NativeCheckpointHeader *header)
{
	local_persist const u32 rangeKinds[] = {
	    NATIVE_CHECKPOINT_REGION_RDATA, NATIVE_CHECKPOINT_REGION_DATA, NATIVE_CHECKPOINT_REGION_SDATA, NATIVE_CHECKPOINT_REGION_R230,
	    NATIVE_CHECKPOINT_REGION_D230,  NATIVE_CHECKPOINT_REGION_V230, NATIVE_CHECKPOINT_REGION_R231,  NATIVE_CHECKPOINT_REGION_D231,
	    NATIVE_CHECKPOINT_REGION_R232,  NATIVE_CHECKPOINT_REGION_D232, NATIVE_CHECKPOINT_REGION_R233,  NATIVE_CHECKPOINT_REGION_D233,
	    NATIVE_CHECKPOINT_REGION_GAR3,  NATIVE_CHECKPOINT_REGION_CRD3, NATIVE_CHECKPOINT_REGION_OXSM,  NATIVE_CHECKPOINT_REGION_OXLG,
	    NATIVE_CHECKPOINT_REGION_MPAK,  NATIVE_CHECKPOINT_REGION_SCRP,
	};

	if (header == NULL)
	{
		return 0;
	}

	header->addressRangeCount = 0;
	for (u32 i = 0; i < len(rangeKinds); i++)
	{
		if (!NativeCheckpoint_AddAddressRange(header, rangeKinds[i]))
		{
			return 0;
		}
	}

#if defined(__vita__)
	{
		const uintptr_t codeStart = (uintptr_t)_start & ~(uintptr_t)0xfff;
		const uintptr_t codeEnd = (uintptr_t)__exidx_end;
		const uintptr_t dataStart = (uintptr_t)__data_start & ~(uintptr_t)0xfff;
		const uintptr_t dataEnd = (uintptr_t)_end;

		if ((codeStart == 0) || (codeEnd <= codeStart) || (dataStart == 0) || (dataEnd <= dataStart) ||
		    (codeEnd > 0xffffffffu) || (dataEnd > 0xffffffffu) ||
		    !NativeCheckpoint_AddRawAddressRange(header, NATIVE_CHECKPOINT_RANGE_CODE, (u32)codeStart, (u32)(codeEnd - codeStart)) ||
		    !NativeCheckpoint_AddRawAddressRange(header, NATIVE_CHECKPOINT_RANGE_DATA, (u32)dataStart, (u32)(dataEnd - dataStart)))
		{
			return 0;
		}
	}
#endif

	return 1;
}

internal const struct NativeCheckpointAddressRange *NativeCheckpoint_FindAddressRange(const struct NativeCheckpointHeader *header, u32 kind)
{
	if (header == NULL)
	{
		return NULL;
	}

	for (u32 i = 0; i < header->addressRangeCount; i++)
	{
		const struct NativeCheckpointAddressRange *range = &header->addressRanges[i];

		if (range->kind == kind)
		{
			return range;
		}
	}

	return NULL;
}

internal b32 NativeCheckpoint_IsAddressRangeValid(const struct NativeCheckpointAddressRange *range)
{
	if ((range == NULL) || (range->size == 0))
	{
		return false;
	}

	return range->start + range->size >= range->start;
}

internal const struct NativeCheckpointAddressRange *NativeCheckpoint_FindAddressOwner(const struct NativeCheckpointHeader *header, u32 address, u32 *offsetOut)
{
	if (header == NULL)
	{
		return NULL;
	}

	for (u32 i = 0; i < header->addressRangeCount; i++)
	{
		const struct NativeCheckpointAddressRange *range = &header->addressRanges[i];
		const u32 end = range->start + range->size;

		if (NativeCheckpoint_IsAddressRangeValid(range) && (address >= range->start) && (address < end))
		{
			if (offsetOut != NULL)
			{
				*offsetOut = address - range->start;
			}
			return range;
		}
	}

	return NULL;
}

internal void *NativeCheckpoint_GetAddressFromRangeOffset(const struct NativeCheckpointHeader *header, u32 kind, u32 offset)
{
	const struct NativeCheckpointAddressRange *range = NativeCheckpoint_FindAddressRange(header, kind);

	if (!NativeCheckpoint_IsAddressRangeValid(range) || (offset >= range->size))
	{
		return NULL;
	}

	return (void *)(uintptr_t)(range->start + offset);
}

internal int NativeCheckpoint_RelocateAddress(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader, u32 oldAddress,
                                              u32 *newAddressOut)
{
	u32 offset;
	const struct NativeCheckpointAddressRange *oldRange;
	const struct NativeCheckpointAddressRange *liveRange;

	if ((oldAddress == 0) || (newAddressOut == NULL))
	{
		return 0;
	}

	oldRange = NativeCheckpoint_FindAddressOwner(oldHeader, oldAddress, &offset);
	if (oldRange == NULL)
	{
		return 0;
	}

	liveRange = NativeCheckpoint_FindAddressRange(liveHeader, oldRange->kind);
	if ((liveRange == NULL) || (offset >= liveRange->size))
	{
		return 0;
	}

	*newAddressOut = liveRange->start + offset;
	return 1;
}

internal int NativeCheckpoint_RelocateAddressInRange(const struct NativeCheckpointHeader *oldHeader,
                                                     const struct NativeCheckpointHeader *liveHeader,
                                                     u32 kind, u32 oldAddress, u32 *newAddressOut)
{
	const struct NativeCheckpointAddressRange *oldRange = NativeCheckpoint_FindAddressRange(oldHeader, kind);
	const struct NativeCheckpointAddressRange *liveRange = NativeCheckpoint_FindAddressRange(liveHeader, kind);
	u32 offset;

	if ((newAddressOut == NULL) || !NativeCheckpoint_IsAddressRangeValid(oldRange) || !NativeCheckpoint_IsAddressRangeValid(liveRange) ||
	    (oldAddress < oldRange->start) || (oldAddress >= oldRange->start + oldRange->size))
	{
		return 0;
	}

	offset = oldAddress - oldRange->start;
	if (offset >= liveRange->size)
	{
		return 0;
	}

	*newAddressOut = liveRange->start + offset;
	return 1;
}

internal b32 NativeCheckpoint_IsPointerSentinel(u32 address)
{
	return (address == 0) || (address == 0xffffffffu) || (address == 0xfffffffeu);
}

internal int NativeCheckpoint_IsLivePointer(const struct NativeCheckpointHeader *liveHeader, const void *ptr)
{
	u32 address;

	if (!NativeCheckpoint_PtrToU32(ptr, &address))
	{
		return 0;
	}

	return NativeCheckpoint_FindAddressOwner(liveHeader, address, NULL) != NULL;
}

internal void NativeCheckpoint_RelocatePointerSlot(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader, void *slot)
{
	u32 oldAddress;
	u32 newAddress;

	if (s_nativeCheckpointRelocationTracking && NativeCheckpoint_HashContainsPointerSlot(slot))
	{
		return;
	}

	if (!NativeCheckpoint_ReadU32Slot(slot, &oldAddress))
	{
		return;
	}
	if (NativeCheckpoint_IsPointerSentinel(oldAddress))
	{
		return;
	}

	if (NativeCheckpoint_RelocateAddress(oldHeader, liveHeader, oldAddress, &newAddress))
	{
		if (NativeCheckpoint_MarkRelocationSlot(slot))
		{
			NativeCheckpoint_WriteU32Slot(slot, newAddress);
		}
	}
}

internal void NativeCheckpoint_RelocateImagePointerSlot(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                        void *slot)
{
	u32 oldAddress;
	u32 newAddress;
	u32 thumbBit;

	if (s_nativeCheckpointRelocationTracking && NativeCheckpoint_HashContainsPointerSlot(slot))
	{
		return;
	}

	if ((oldHeader == NULL) || (liveHeader == NULL) || (oldHeader->codeAnchor == 0) || (liveHeader->codeAnchor == 0))
	{
		return;
	}
	if (!NativeCheckpoint_ReadU32Slot(slot, &oldAddress) || NativeCheckpoint_IsPointerSentinel(oldAddress))
	{
		return;
	}

	thumbBit = oldAddress & 1u;
#if defined(__vita__)
	if (!NativeCheckpoint_RelocateAddressInRange(oldHeader, liveHeader, NATIVE_CHECKPOINT_RANGE_CODE, oldAddress & ~1u, &newAddress))
	{
		s_nativeCheckpointRelocationValid = 0;
		return;
	}
	newAddress |= thumbBit;
	#else
	newAddress = (oldAddress & ~1u) + ((liveHeader->codeAnchor & ~1u) - (oldHeader->codeAnchor & ~1u));
	newAddress |= thumbBit;
	#endif
	if (NativeCheckpoint_MarkRelocationSlot(slot))
	{
		NativeCheckpoint_WriteU32Slot(slot, newAddress);
	}
}

internal void NativeCheckpoint_RelocatePointerOrImageSlot(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                          void *slot)
{
	u32 oldAddress;
	u32 newAddress;

	if (s_nativeCheckpointRelocationTracking && NativeCheckpoint_HashContainsPointerSlot(slot))
	{
		return;
	}

	if (!NativeCheckpoint_ReadU32Slot(slot, &oldAddress))
	{
		return;
	}
	if (NativeCheckpoint_IsPointerSentinel(oldAddress))
	{
		return;
	}

	if (NativeCheckpoint_RelocateAddress(oldHeader, liveHeader, oldAddress, &newAddress))
	{
		if (NativeCheckpoint_MarkRelocationSlot(slot))
		{
			NativeCheckpoint_WriteU32Slot(slot, newAddress);
		}
	}
	else
	{
	#if defined(__vita__)
		s_nativeCheckpointRelocationValid = 0;
	#else
		NativeCheckpoint_RelocateImagePointerSlot(oldHeader, liveHeader, slot);
	#endif
	}
}

internal void NativeCheckpoint_RelocateFields(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader, void *base,
                                              const struct NativeCheckpointFieldRelocation *fields, u32 fieldCount)
{
	u8 *bytes = (u8 *)base;

	if ((base == NULL) || (fields == NULL))
	{
		return;
	}

	for (u32 i = 0; i < fieldCount; i++)
	{
		void *slot = &bytes[fields[i].offset];

		switch (fields[i].kind)
		{
		case NATIVE_CHECKPOINT_FIELD_POINTER:
			NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, slot);
			break;
		case NATIVE_CHECKPOINT_FIELD_IMAGE_POINTER:
			NativeCheckpoint_RelocateImagePointerSlot(oldHeader, liveHeader, slot);
			break;
		case NATIVE_CHECKPOINT_FIELD_POINTER_OR_IMAGE:
			NativeCheckpoint_RelocatePointerOrImageSlot(oldHeader, liveHeader, slot);
			break;
		}
	}
}

internal void NativeCheckpoint_RelocateRectMenu(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                struct RectMenu *menu)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR_OR_IMAGE(struct RectMenu, rows),
	    NATIVE_CHECKPOINT_FIELD_IMAGE(struct RectMenu, funcPtr),
	    NATIVE_CHECKPOINT_FIELD_PTR_OR_IMAGE(struct RectMenu, ptrNextBox_InHierarchy),
	    NATIVE_CHECKPOINT_FIELD_PTR_OR_IMAGE(struct RectMenu, ptrPrevBox_InHierarchy),
	};

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, menu, fields, len(fields));
}

internal void NativeCheckpoint_RelocateTitle(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                             struct Title *title)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Title, t),
	};

	if (title == NULL)
	{
		return;
	}

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, title, fields, len(fields));
	for (u32 i = 0; i < len(title->i); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &title->i[i]);
	}
}

internal void NativeCheckpoint_RelocateAdventurePauseObject(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                            struct PauseObject *pauseObject)
{
	if (pauseObject == NULL)
	{
		return;
	}

	for (u32 i = 0; i < len(pauseObject->members); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &pauseObject->members[i].inst);
	}
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &pauseObject->t);
}

internal void NativeCheckpoint_RelocateLoadQueueSlot(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                     struct LoadQueueSlot *slot)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct LoadQueueSlot, ptrBigfileCdPos_UNUSED),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct LoadQueueSlot, ptrDestination),
	    NATIVE_CHECKPOINT_FIELD_IMAGE(struct LoadQueueSlot, callbackFuncPtr),
	};

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, slot, fields, len(fields));
}

internal void NativeCheckpoint_RelocateLinkedList(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                  struct LinkedList *list)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct LinkedList, first),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct LinkedList, last),
	};

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, list, fields, len(fields));
}

internal void NativeCheckpoint_RelocateItemLinks(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                 struct Item *item)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Item, next),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Item, prev),
	};

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, item, fields, len(fields));
}

internal u32 NativeCheckpoint_GetPoolItemStride(u32 itemSize)
{
	return JITPOOL_ALIGN_ITEM_STRIDE(itemSize);
}

internal int NativeCheckpoint_GetPoolItemIndex(const struct JitPool *pool, const void *item, u32 *indexOut)
{
	u32 poolAddress;
	u32 itemAddress;
	u32 stride;
	u32 offset;
	u64 poolEnd;

	if ((pool == NULL) || (item == NULL) || (indexOut == NULL) || (pool->maxItems <= 0) ||
	    ((u32)pool->maxItems > NATIVE_CHECKPOINT_POOL_ITEM_CAP) || (pool->itemSize == 0) ||
	    !NativeCheckpoint_PtrToU32(pool->ptrPoolData, &poolAddress) || !NativeCheckpoint_PtrToU32(item, &itemAddress))
	{
		return 0;
	}

	stride = NativeCheckpoint_GetPoolItemStride(pool->itemSize);
	poolEnd = (u64)poolAddress + ((u64)stride * (u32)pool->maxItems);
	if ((stride == 0) || (poolEnd > 0x100000000ULL) || (itemAddress < poolAddress) || ((u64)itemAddress >= poolEnd))
	{
		return 0;
	}

	offset = itemAddress - poolAddress;
	if ((offset % stride) != 0)
	{
		return 0;
	}

	*indexOut = offset / stride;
	return *indexOut < (u32)pool->maxItems;
}

internal void *NativeCheckpoint_GetPoolItem(const struct JitPool *pool, u32 index)
{
	u32 poolAddress;
	u32 stride;
	u64 address;

	if ((pool == NULL) || (index >= (u32)pool->maxItems) || !NativeCheckpoint_PtrToU32(pool->ptrPoolData, &poolAddress))
	{
		return NULL;
	}

	stride = NativeCheckpoint_GetPoolItemStride(pool->itemSize);
	address = (u64)poolAddress + ((u64)stride * index);
	if ((stride == 0) || (address > 0xffffffffULL))
	{
		return NULL;
	}

	return (void *)(uintptr_t)(u32)address;
}

internal int NativeCheckpoint_ValidatePoolMetadata(const struct NativeCheckpointHeader *liveHeader, const struct JitPool *pool, u32 expectedItemSize)
{
	const struct NativeCheckpointAddressRange *range;
	u32 poolAddress;
	u32 poolOffset;
	u32 stride;
	u64 expectedPoolSize;

	if ((liveHeader == NULL) || (pool == NULL) || (pool->maxItems <= 0) ||
	    ((u32)pool->maxItems > NATIVE_CHECKPOINT_POOL_ITEM_CAP) || (pool->itemSize == 0) ||
	    ((expectedItemSize != 0) && (pool->itemSize != expectedItemSize)) ||
	    (pool->free.count < 0) || (pool->free.count > pool->maxItems) ||
	    (pool->taken.count < 0) || (pool->taken.count > pool->maxItems) ||
	    !NativeCheckpoint_PtrToU32(pool->ptrPoolData, &poolAddress))
	{
		return 0;
	}

	stride = NativeCheckpoint_GetPoolItemStride(pool->itemSize);
	expectedPoolSize = (u64)stride * (u32)pool->maxItems;
	if ((stride != pool->itemSize) || (expectedPoolSize == 0) || (expectedPoolSize > 0x7fffffffULL) ||
	    (pool->poolSize != (s32)expectedPoolSize))
	{
		return 0;
	}

	range = NativeCheckpoint_FindAddressOwner(liveHeader, poolAddress, &poolOffset);
	return (range != NULL) && (((u64)poolOffset + expectedPoolSize) <= range->size);
}

internal int NativeCheckpoint_ValidatePoolList(const struct JitPool *pool, const struct LinkedList *list, u8 *seen)
{
	struct Item *item;
	struct Item *previous = NULL;

	if ((pool == NULL) || (list == NULL) || (seen == NULL) || (list->count < 0) || (list->count > pool->maxItems) ||
	    ((list->count == 0) != ((list->first == NULL) && (list->last == NULL))))
	{
		return 0;
	}

	item = list->first;
	for (s32 i = 0; i < list->count; i++)
	{
		u32 index;

		if (!NativeCheckpoint_GetPoolItemIndex(pool, item, &index) || seen[index] || (item->prev != previous))
		{
			return 0;
		}

		seen[index] = 1;
		previous = item;
		item = item->next;
	}

	return (item == NULL) && (previous == list->last);
}

internal int NativeCheckpoint_PreparePoolSet(const struct NativeCheckpointHeader *liveHeader, const struct JitPool *pool, u32 expectedItemSize,
                                            struct NativeCheckpointPoolSet *poolSet)
{
	u8 freeItems[NATIVE_CHECKPOINT_POOL_ITEM_CAP];
	u8 takenItems[NATIVE_CHECKPOINT_POOL_ITEM_CAP];

	if ((poolSet == NULL) || !NativeCheckpoint_ValidatePoolMetadata(liveHeader, pool, expectedItemSize))
	{
		return 0;
	}

	memset(poolSet, 0, sizeof(*poolSet));
	memset(freeItems, 0, sizeof(freeItems));
	memset(takenItems, 0, sizeof(takenItems));
	if (!NativeCheckpoint_ValidatePoolList(pool, &pool->free, freeItems) ||
	    !NativeCheckpoint_ValidatePoolList(pool, &pool->taken, takenItems))
	{
		return 0;
	}

	for (u32 i = 0; i < (u32)pool->maxItems; i++)
	{
		if (freeItems[i] && takenItems[i])
		{
			return 0;
		}
		if (!freeItems[i])
		{
			void *item = NativeCheckpoint_GetPoolItem(pool, i);

			if (item == NULL)
			{
				return 0;
			}
			poolSet->slotActive[i] = 1;
			poolSet->activeItems[poolSet->activeCount++] = item;
		}
	}

	poolSet->takenCount = (u32)pool->taken.count;
	return poolSet->activeCount == (u32)(pool->maxItems - pool->free.count);
}

internal int NativeCheckpoint_IsActivePoolItem(const struct JitPool *pool, const struct NativeCheckpointPoolSet *poolSet, const void *item, u32 *indexOut)
{
	u32 index;

	if ((poolSet == NULL) || !NativeCheckpoint_GetPoolItemIndex(pool, item, &index) || !poolSet->slotActive[index])
	{
		return 0;
	}

	if (indexOut != NULL)
	{
		*indexOut = index;
	}
	return 1;
}

internal int NativeCheckpoint_RepairFreeStackObjects(const struct JitPool *pool, const struct NativeCheckpointPoolSet *poolSet)
{
	if ((pool == NULL) || (poolSet == NULL) || (pool->itemSize < sizeof(struct Item) + sizeof(void *)))
	{
		return 0;
	}

	for (u32 i = 0; i < (u32)pool->maxItems; i++)
	{
		if (!poolSet->slotActive[i])
		{
			u8 *slot = (u8 *)NativeCheckpoint_GetPoolItem(pool, i);
			void *object;

			if (slot == NULL)
			{
				return 0;
			}
			object = slot + sizeof(struct Item);
			memcpy(object, &object, sizeof(object));
		}
	}

	return 1;
}

internal int NativeCheckpoint_RelocateJitPool(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                              struct JitPool *pool, u32 expectedItemSize)
{
	uintptr_t currSlot;
	u32 itemStep;

	if (pool == NULL)
	{
		return 0;
	}

	NativeCheckpoint_RelocateLinkedList(oldHeader, liveHeader, &pool->free);
	NativeCheckpoint_RelocateLinkedList(oldHeader, liveHeader, &pool->taken);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &pool->ptrPoolData);

	if (!NativeCheckpoint_ValidatePoolMetadata(liveHeader, pool, expectedItemSize))
	{
		return 0;
	}

	itemStep = NativeCheckpoint_GetPoolItemStride(pool->itemSize);

	currSlot = (uintptr_t)pool->ptrPoolData;
	for (int itemIndex = 0; itemIndex < pool->maxItems; itemIndex++)
	{
		NativeCheckpoint_RelocateItemLinks(oldHeader, liveHeader, (struct Item *)currSlot);
		currSlot += itemStep;
	}

	return 1;
}

internal void NativeCheckpoint_RelocatePrimMem(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                               struct PrimMem *primMem)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct PrimMem, start),           NATIVE_CHECKPOINT_FIELD_PTR(struct PrimMem, end),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct PrimMem, cursor),          NATIVE_CHECKPOINT_FIELD_PTR(struct PrimMem, guardEnd),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct PrimMem, allocationStart),
	};

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, primMem, fields, len(fields));
}

internal void NativeCheckpoint_RelocateOTMem(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                             struct OTMem *otMem)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct OTMem, start),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct OTMem, end),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct OTMem, cursor),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct OTMem, uiOT),
	};

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, otMem, fields, len(fields));
}

internal void NativeCheckpoint_RelocateDB(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader, struct DB *db)
{
	if (db == NULL)
	{
		return;
	}

	NativeCheckpoint_RelocatePrimMem(oldHeader, liveHeader, &db->primMem);
	NativeCheckpoint_RelocateOTMem(oldHeader, liveHeader, &db->otMem);
}

internal void NativeCheckpoint_RelocatePushBuffer(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                  struct PushBuffer *pushBuffer)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct PushBuffer, ptrOT),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct PushBuffer, renderBucketOTRangeEnd),
	};

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, pushBuffer, fields, len(fields));
}

internal void NativeCheckpoint_RelocateCameraDC(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                struct CameraDC *camera)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct CameraDC, ptrQuadBlock), NATIVE_CHECKPOINT_FIELD_PTR(struct CameraDC, visLeafSrc),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct CameraDC, visFaceSrc),   NATIVE_CHECKPOINT_FIELD_PTR(struct CameraDC, visInstSrc),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct CameraDC, visOVertSrc),  NATIVE_CHECKPOINT_FIELD_PTR(struct CameraDC, visSCVertSrc),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct CameraDC, driverToFollow), NATIVE_CHECKPOINT_FIELD_PTR(struct CameraDC, pushBuffer),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct CameraDC, trackPathNode), NATIVE_CHECKPOINT_FIELD_PTR(struct CameraDC, currEOR),
	};

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, camera, fields, len(fields));
}

internal void NativeCheckpoint_RelocateInstDrawPerPlayer(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                         struct InstDrawPerPlayer *idpp)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct InstDrawPerPlayer, pushBuffer),    NATIVE_CHECKPOINT_FIELD_PTR(struct InstDrawPerPlayer, ptrCurrFrame),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct InstDrawPerPlayer, ptrNextFrame),  NATIVE_CHECKPOINT_FIELD_PTR(struct InstDrawPerPlayer, ptrCommandList),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct InstDrawPerPlayer, ptrTexLayout),  NATIVE_CHECKPOINT_FIELD_PTR(struct InstDrawPerPlayer, ptrColorLayout),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct InstDrawPerPlayer, ptrDeltaArray), NATIVE_CHECKPOINT_FIELD_PTR(struct InstDrawPerPlayer, mh),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct InstDrawPerPlayer, otRangeNormal), NATIVE_CHECKPOINT_FIELD_PTR(struct InstDrawPerPlayer, otRangeSecondary),
	};

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, idpp, fields, len(fields));
}

internal void NativeCheckpoint_RelocateInstance(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                struct Instance *inst, s32 numPlayers)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Instance, model),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Instance, instDef),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Instance, thread),
	};

	if (inst == NULL)
	{
		return;
	}

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, inst, fields, len(fields));

	if ((numPlayers < 1) || (numPlayers > 4))
	{
		numPlayers = 4;
	}

	struct InstDrawPerPlayer *idpp = INST_GETIDPP(inst);
	for (s32 playerIndex = 0; playerIndex < numPlayers; playerIndex++)
	{
		NativeCheckpoint_RelocateInstDrawPerPlayer(oldHeader, liveHeader, &idpp[playerIndex]);
	}
}

internal void NativeCheckpoint_RelocateThread(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                              struct Thread *thread)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Thread, name),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Thread, parentThread),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Thread, siblingThread),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Thread, childThread),
	    NATIVE_CHECKPOINT_FIELD_IMAGE(struct Thread, funcThDestroy),
	    NATIVE_CHECKPOINT_FIELD_IMAGE(struct Thread, funcThCollide),
	    NATIVE_CHECKPOINT_FIELD_IMAGE(struct Thread, funcThTick),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Thread, object),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Thread, inst),
	};

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, thread, fields, len(fields));
}

internal void NativeCheckpoint_RelocateDriver(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                              struct Driver *driver)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Driver, wheelSprites),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Driver, instBombThrow),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Driver, instBubbleHold),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Driver, instTntRecv),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Driver, instSelf),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Driver, instTntSend),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Driver, currBlockTouching),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Driver, underDriver),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Driver, lastValid),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Driver, terrainMeta1),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Driver, terrainMeta2),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Driver, instBigNum),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Driver, instFruitDisp),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Driver, thCloud),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Driver, thTrackingMe),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Driver, plantEatingMe),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Driver, wakeInst),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Driver, pendingDamageAttacker),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Driver, botData.botNavFrame),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Driver, botData.maskObj),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Driver, EndOfRaceComment_ptrQuip),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Driver, ghostTape),
	};

	if (driver == NULL)
	{
		return;
	}

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, driver, fields, len(fields));

	for (u32 i = 0; i < len(driver->funcPtrs); i++)
	{
		NativeCheckpoint_RelocateImagePointerSlot(oldHeader, liveHeader, &driver->funcPtrs[i]);
	}

	NativeCheckpoint_RelocateItemLinks(oldHeader, liveHeader, &driver->botData.item);

	if (driver->kartState == KS_MASK_GRABBED)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &driver->KartStates.MaskGrab.maskObj);
	}
	else if (driver->kartState == KS_ENGINE_REVVING)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &driver->KartStates.RevEngine.maskObj);
	}
}

internal void NativeCheckpoint_RelocateGhostTape(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                struct GhostTape *tape)
{
	if (tape == NULL)
	{
		return;
	}

	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &tape->gh);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &tape->ptrStart);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &tape->ptrEnd);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &tape->ptrCurr);
	for (u32 i = 0; i < len(tape->packets); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &tape->packets[i].bufferPacket);
	}
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &tape->gh_again);
}

internal void NativeCheckpoint_RelocateMaskHeadWeapon(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                      struct MaskHeadWeapon *weapon)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct MaskHeadWeapon, maskBeamInst),
	};

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, weapon, fields, len(fields));
}

internal void NativeCheckpoint_RelocateTrackerWeapon(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                     struct TrackerWeapon *weapon)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct TrackerWeapon, driverTarget), NATIVE_CHECKPOINT_FIELD_PTR(struct TrackerWeapon, driverParent),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct TrackerWeapon, instParent),   NATIVE_CHECKPOINT_FIELD_PTR(struct TrackerWeapon, ptrParticle),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct TrackerWeapon, ptrNodeCurr),  NATIVE_CHECKPOINT_FIELD_PTR(struct TrackerWeapon, ptrNodeNext),
	};

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, weapon, fields, len(fields));
}

internal void NativeCheckpoint_RelocateRainLocal(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                 struct RainLocal *rain)
{
	if (rain == NULL)
	{
		return;
	}

	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &rain->cloudInst);
}

internal void NativeCheckpoint_RelocateRainCloud(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                 struct RainCloud *cloud)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct RainCloud, rainLocal),
	};

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, cloud, fields, len(fields));
}

internal void NativeCheckpoint_RelocateMaskHint(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                struct MaskHint *maskHint)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct MaskHint, self),
	};

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, maskHint, fields, len(fields));
}

internal void NativeCheckpoint_RelocateShield(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                              struct Shield *shield)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Shield, instColor),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Shield, instHighlight),
	};

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, shield, fields, len(fields));
}

internal void NativeCheckpoint_RelocateMineWeapon(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                  struct MineWeapon *weapon)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct MineWeapon, driverTarget),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct MineWeapon, instParent),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct MineWeapon, crateInst),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct MineWeapon, weaponSlot231),
	};

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, weapon, fields, len(fields));
}

internal void NativeCheckpoint_RelocateBaron(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                             struct Baron *baron)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Baron, otherInst),
	};

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, baron, fields, len(fields));
}

internal void NativeCheckpoint_RelocateFollower(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                struct Follower *follower)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Follower, driver),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Follower, mineTh),
	};

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, follower, fields, len(fields));
}

internal void NativeCheckpoint_RelocateFruit(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                             struct Fruit *fruit)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Fruit, driver),
	};

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, fruit, fields, len(fields));
}

internal void NativeCheckpoint_RelocateSpider(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                              struct Spider *spider)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Spider, shadowInst),
	};

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, spider, fields, len(fields));
}

internal void NativeCheckpoint_RelocateTurbo(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                             struct Turbo *turbo)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Turbo, inst),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Turbo, driver),
	};

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, turbo, fields, len(fields));
}

internal void NativeCheckpoint_RelocateBlowupSlots(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader, s32 *slots)
{
	if (slots == NULL)
	{
		return;
	}

	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &slots[0]);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &slots[1]);
}

internal void NativeCheckpoint_RelocateBurstSlots(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader, s32 *slots)
{
	if (slots == NULL)
	{
		return;
	}

	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &slots[0]);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &slots[1]);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &slots[2]);
}

internal void NativeCheckpoint_RelocateBossGarageDoor(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                      struct BossGarageDoor *door)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct BossGarageDoor, garageTopInst),
	};

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, door, fields, len(fields));
}

internal void NativeCheckpoint_RelocateWoodDoor(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                struct WoodDoor *door)
{
	if (door == NULL)
	{
		return;
	}

	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &door->otherDoor);
	for (u32 i = 0; i < len(door->keyInst); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &door->keyInst[i]);
	}
}

internal void NativeCheckpoint_RelocateWarpPad(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                               struct WarpPad *warpPad)
{
	if (warpPad == NULL)
	{
		return;
	}

	for (u32 i = 0; i < len(warpPad->inst); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &warpPad->inst[i]);
	}
}

internal void NativeCheckpoint_RelocateSaveObj(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                               struct SaveObj *saveObj)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct SaveObj, inst),
	};

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, saveObj, fields, len(fields));
}

internal void NativeCheckpoint_RelocateCutsceneObj(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                   struct CutsceneObj *cutscene)
{
	if (cutscene == NULL)
	{
		return;
	}

	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &cutscene->ptrIcons);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &cutscene->metadata);
	for (u32 i = 0; i < len(cutscene->currOpcode); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &cutscene->currOpcode[i]);
	}
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &cutscene->prevOpcode);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &cutscene->frameOverrideRoot);
}

internal void NativeCheckpoint_RelocateSelectProfileLoadSaveObj(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                                struct SelectProfileLoadSaveObj *obj)
{
	if (obj == NULL)
	{
		return;
	}

	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &obj->thread);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &obj->icons);

	if (NativeCheckpoint_IsLivePointer(liveHeader, obj->icons))
	{
		for (u32 i = 0; i < 12; i++)
		{
			NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &obj->icons[i].inst);
		}
	}
}

internal void NativeCheckpoint_RelocateThreadObject(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                    struct Thread *thread)
{
	if ((thread == NULL) || (thread->object == NULL))
	{
		return;
	}

	if ((thread->modelIndex == DYNAMIC_PLAYER) || (thread->modelIndex == DYNAMIC_ROBOT_CAR) || (thread->modelIndex == DYNAMIC_GHOST))
	{
		struct Driver *driver = (struct Driver *)thread->object;

		NativeCheckpoint_RelocateDriver(oldHeader, liveHeader, driver);
		if (NativeCheckpoint_IsLivePointer(liveHeader, driver->ghostTape))
		{
			NativeCheckpoint_RelocateGhostTape(oldHeader, liveHeader, driver->ghostTape);
		}
		return;
	}

	if (thread->funcThTick == VehTalkMask_ThTick)
	{
		NativeCheckpoint_RelocateMaskHint(oldHeader, liveHeader, (struct MaskHint *)thread->object);
	}
	else if ((thread->funcThTick == RB_MaskWeapon_ThTick) || (thread->funcThTick == RB_MaskWeapon_FadeAway))
	{
		NativeCheckpoint_RelocateMaskHeadWeapon(oldHeader, liveHeader, (struct MaskHeadWeapon *)thread->object);
	}
	else if ((thread->funcThTick == RB_MovingExplosive_ThTick) || (thread->funcThTick == RB_Warpball_ThTick) || (thread->funcThTick == RB_Warpball_FadeAway) ||
	         (thread->funcThTick == RB_Warpball_TurnAround))
	{
		NativeCheckpoint_RelocateTrackerWeapon(oldHeader, liveHeader, (struct TrackerWeapon *)thread->object);
	}
	else if ((thread->funcThTick == RB_GenericMine_ThTick) || (thread->funcThTick == RB_TNT_ThTick_ThrowOffHead) ||
	         (thread->funcThTick == RB_TNT_ThTick_SitOnHead) || (thread->funcThTick == RB_TNT_ThTick_ThrowOnHead) ||
	         (thread->funcThTick == RB_Potion_ThTick_InAir))
	{
		NativeCheckpoint_RelocateMineWeapon(oldHeader, liveHeader, (struct MineWeapon *)thread->object);
	}
	else if ((thread->funcThTick == RB_RainCloud_ThTick) || (thread->funcThTick == RB_RainCloud_FadeAway))
	{
		NativeCheckpoint_RelocateRainCloud(oldHeader, liveHeader, (struct RainCloud *)thread->object);
	}
	else if ((thread->funcThTick == RB_ShieldDark_ThTick_Grow) || (thread->funcThTick == RB_ShieldDark_ThTick_Pop))
	{
		NativeCheckpoint_RelocateShield(oldHeader, liveHeader, (struct Shield *)thread->object);
	}
	else if (thread->funcThTick == RB_Baron_ThTick)
	{
		NativeCheckpoint_RelocateBaron(oldHeader, liveHeader, (struct Baron *)thread->object);
	}
	else if (thread->funcThTick == RB_Follower_ThTick)
	{
		NativeCheckpoint_RelocateFollower(oldHeader, liveHeader, (struct Follower *)thread->object);
	}
	else if (thread->funcThTick == RB_Fruit_ThTick)
	{
		NativeCheckpoint_RelocateFruit(oldHeader, liveHeader, (struct Fruit *)thread->object);
	}
	else if (thread->funcThTick == RB_Spider_ThTick)
	{
		NativeCheckpoint_RelocateSpider(oldHeader, liveHeader, (struct Spider *)thread->object);
	}
	else if (thread->funcThTick == VehTurbo_ThTick)
	{
		NativeCheckpoint_RelocateTurbo(oldHeader, liveHeader, (struct Turbo *)thread->object);
	}
	else if (thread->funcThTick == RB_Blowup_ThTick)
	{
		NativeCheckpoint_RelocateBlowupSlots(oldHeader, liveHeader, (s32 *)thread->object);
	}
	else if (thread->funcThTick == RB_Burst_ThTick)
	{
		NativeCheckpoint_RelocateBurstSlots(oldHeader, liveHeader, (s32 *)thread->object);
	}
	else if (thread->funcThTick == AH_Garage_ThTick)
	{
		NativeCheckpoint_RelocateBossGarageDoor(oldHeader, liveHeader, (struct BossGarageDoor *)thread->object);
	}
	else if (thread->funcThTick == AH_Door_ThTick)
	{
		NativeCheckpoint_RelocateWoodDoor(oldHeader, liveHeader, (struct WoodDoor *)thread->object);
	}
	else if (thread->funcThTick == AH_WarpPad_ThTick)
	{
		NativeCheckpoint_RelocateWarpPad(oldHeader, liveHeader, (struct WarpPad *)thread->object);
	}
	else if (thread->funcThTick == AH_SaveObj_ThTick)
	{
		NativeCheckpoint_RelocateSaveObj(oldHeader, liveHeader, (struct SaveObj *)thread->object);
	}
	else if (thread->funcThTick == CS_Thread_ThTick)
	{
		NativeCheckpoint_RelocateCutsceneObj(oldHeader, liveHeader, (struct CutsceneObj *)thread->object);
	}
	else if (thread->funcThTick == SelectProfile_ThTick)
	{
		NativeCheckpoint_RelocateSelectProfileLoadSaveObj(oldHeader, liveHeader, (struct SelectProfileLoadSaveObj *)thread->object);
	}
}

internal int NativeCheckpoint_RelocateActiveThreads(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                    struct GameTracker *gGT, const struct NativeCheckpointPoolSet *poolSet,
                                                    struct NativeCheckpointThreadSet *threadSet)
{
	struct JitPool *pool;
	struct Thread *pending[NATIVE_CHECKPOINT_POOL_ITEM_CAP];
	u8 seen[NATIVE_CHECKPOINT_POOL_ITEM_CAP];
	u32 pendingCount = 0;

	if ((gGT == NULL) || (poolSet == NULL) || (threadSet == NULL) || (poolSet->takenCount != 0))
	{
		return 0;
	}
	pool = &gGT->JitPools.thread;

	memset(threadSet, 0, sizeof(*threadSet));
	memset(seen, 0, sizeof(seen));
	for (u32 i = 0; i < poolSet->activeCount; i++)
	{
		NativeCheckpoint_RelocateThread(oldHeader, liveHeader, (struct Thread *)poolSet->activeItems[i]);
	}
	if (!s_nativeCheckpointRelocationValid)
	{
		return 0;
	}

	for (u32 bucket = 0; bucket < len(gGT->threadBuckets); bucket++)
	{
		struct Thread *root = gGT->threadBuckets[bucket].thread;

		if (root == NULL)
		{
			continue;
		}
		if ((pendingCount >= len(pending)) || !NativeCheckpoint_IsActivePoolItem(pool, poolSet, root, NULL) ||
		    (root->parentThread != NULL) || ((root->flags & 0xffu) != bucket))
		{
			return 0;
		}
		pending[pendingCount++] = root;
	}

	while (pendingCount > 0)
	{
		struct Thread *thread = pending[--pendingCount];
		u32 poolIndex;

		if (!NativeCheckpoint_IsActivePoolItem(pool, poolSet, thread, &poolIndex) || seen[poolIndex] ||
		    (thread->next != NULL) || (thread->prev != NULL) || ((thread->flags & 0xffu) >= NUM_BUCKETS) ||
		    ((thread->parentThread != NULL) && !NativeCheckpoint_IsActivePoolItem(pool, poolSet, thread->parentThread, NULL)) ||
		    ((thread->siblingThread != NULL) && !NativeCheckpoint_IsActivePoolItem(pool, poolSet, thread->siblingThread, NULL)) ||
		    ((thread->childThread != NULL) && !NativeCheckpoint_IsActivePoolItem(pool, poolSet, thread->childThread, NULL)) ||
		    (threadSet->count >= len(threadSet->threads)))
		{
			return 0;
		}

		seen[poolIndex] = 1;
		threadSet->threads[threadSet->count++] = thread;
		if (thread->siblingThread != NULL)
		{
			if (pendingCount >= len(pending))
			{
				return 0;
			}
			pending[pendingCount++] = thread->siblingThread;
		}
		if (thread->childThread != NULL)
		{
			if (pendingCount >= len(pending))
			{
				return 0;
			}
			pending[pendingCount++] = thread->childThread;
		}
	}

	if (threadSet->count != poolSet->activeCount)
	{
		return 0;
	}

	for (u32 i = 0; i < (u32)pool->maxItems; i++)
	{
		if ((poolSet->slotActive[i] != 0) != (seen[i] != 0))
		{
			return 0;
		}
	}

	for (u32 i = 0; i < threadSet->count; i++)
	{
		const struct Thread *thread = threadSet->threads[i];
		const struct Thread *ancestor = thread->parentThread;
		u32 depth = 0;

		if ((thread->siblingThread != NULL) && (thread->siblingThread->parentThread != thread->parentThread))
		{
			return 0;
		}
		if (thread->childThread != NULL)
		{
			const struct Thread *expectedChildParent =
			    (thread->flags & CHILD_BETWEEN) != 0 ? thread->parentThread : thread;

			if (thread->childThread->parentThread != expectedChildParent)
			{
				return 0;
			}
		}
		while (ancestor != NULL)
		{
			if ((ancestor == thread) || (++depth > threadSet->count))
			{
				return 0;
			}
			ancestor = ancestor->parentThread;
		}
	}

	return 1;
}

internal int NativeCheckpoint_ValidateActiveThreadObjects(const struct NativeCheckpointThreadSet *threadSet,
                                                          const struct JitPool *smallPool, const struct NativeCheckpointPoolSet *smallSet,
                                                          const struct JitPool *mediumPool, const struct NativeCheckpointPoolSet *mediumSet,
                                                          const struct JitPool *largePool, const struct NativeCheckpointPoolSet *largeSet)
{
	u8 smallOwners[NATIVE_CHECKPOINT_POOL_ITEM_CAP];
	u8 mediumOwners[NATIVE_CHECKPOINT_POOL_ITEM_CAP];
	u8 largeOwners[NATIVE_CHECKPOINT_POOL_ITEM_CAP];
	u32 ownedCount = 0;

	if ((threadSet == NULL) || (smallPool == NULL) || (smallSet == NULL) || (mediumPool == NULL) || (mediumSet == NULL) ||
	    (largePool == NULL) || (largeSet == NULL) || (smallSet->takenCount != 0) || (mediumSet->takenCount != 0) ||
	    (largeSet->takenCount != 0))
	{
		return 0;
	}

	memset(smallOwners, 0, sizeof(smallOwners));
	memset(mediumOwners, 0, sizeof(mediumOwners));
	memset(largeOwners, 0, sizeof(largeOwners));
	for (u32 i = 0; i < threadSet->count; i++)
	{
		const struct Thread *thread = threadSet->threads[i];
		const struct JitPool *pool;
		const struct NativeCheckpointPoolSet *poolSet;
		u8 *owners;
		u32 objectAddress;
		u32 slotIndex;
		void *slot;

		switch (thread->flags & 0x300u)
		{
		case LARGE:
			pool = largePool;
			poolSet = largeSet;
			owners = largeOwners;
			break;
		case MEDIUM:
			pool = mediumPool;
			poolSet = mediumSet;
			owners = mediumOwners;
			break;
		default:
			pool = smallPool;
			poolSet = smallSet;
			owners = smallOwners;
			break;
		}

		if (!NativeCheckpoint_PtrToU32(thread->object, &objectAddress) || (objectAddress < sizeof(struct Item)))
		{
			return 0;
		}
		slot = (void *)(uintptr_t)(objectAddress - sizeof(struct Item));
		if (!NativeCheckpoint_IsActivePoolItem(pool, poolSet, slot, &slotIndex) || owners[slotIndex] ||
		    ((thread->flags >> 16) >= (pool->itemSize - sizeof(struct Item))) ||
		    (thread->object != (u8 *)slot + sizeof(struct Item)))
		{
			return 0;
		}

		owners[slotIndex] = 1;
		ownedCount++;
	}

	if ((ownedCount != threadSet->count) ||
	    (ownedCount != (smallSet->activeCount + mediumSet->activeCount + largeSet->activeCount)))
	{
		return 0;
	}

	for (u32 i = 0; i < (u32)smallPool->maxItems; i++)
	{
		if ((smallSet->slotActive[i] != 0) != (smallOwners[i] != 0))
		{
			return 0;
		}
	}
	for (u32 i = 0; i < (u32)mediumPool->maxItems; i++)
	{
		if ((mediumSet->slotActive[i] != 0) != (mediumOwners[i] != 0))
		{
			return 0;
		}
	}
	for (u32 i = 0; i < (u32)largePool->maxItems; i++)
	{
		if ((largeSet->slotActive[i] != 0) != (largeOwners[i] != 0))
		{
			return 0;
		}
	}

	return 1;
}

internal void NativeCheckpoint_RelocateActiveThreadObjects(const struct NativeCheckpointHeader *oldHeader,
                                                           const struct NativeCheckpointHeader *liveHeader,
                                                           const struct NativeCheckpointThreadSet *threadSet)
{
	if (threadSet == NULL)
	{
		return;
	}

	for (u32 i = 0; i < threadSet->count; i++)
	{
		NativeCheckpoint_RelocateThreadObject(oldHeader, liveHeader, threadSet->threads[i]);
	}
}

internal struct Thread *NativeCheckpoint_FindThreadByObject(const struct NativeCheckpointThreadSet *threadSet, const void *object)
{
	if ((threadSet == NULL) || (object == NULL))
	{
		return NULL;
	}

	for (u32 i = 0; i < threadSet->count; i++)
	{
		if (threadSet->threads[i]->object == object)
		{
			return threadSet->threads[i];
		}
	}

	return NULL;
}

internal int NativeCheckpoint_IsDriverThread(const struct Thread *thread)
{
	return (thread != NULL) &&
	       ((thread->modelIndex == DYNAMIC_PLAYER) || (thread->modelIndex == DYNAMIC_ROBOT_CAR) || (thread->modelIndex == DYNAMIC_GHOST));
}

internal int NativeCheckpoint_ValidateDrivers(const struct GameTracker *gGT, const struct NativeCheckpointThreadSet *threadSet,
                                              const struct JitPool *instancePool, const struct NativeCheckpointPoolSet *instanceSet)
{
	u8 raceOrderSeen[len(gGT->drivers)];

	if ((gGT == NULL) || (threadSet == NULL) || (instancePool == NULL) || (instanceSet == NULL))
	{
		return 0;
	}

	for (u32 i = 0; i < threadSet->count; i++)
	{
		struct Thread *thread = threadSet->threads[i];
		struct Driver *driver;

		if (!NativeCheckpoint_IsDriverThread(thread))
		{
			continue;
		}

		driver = (struct Driver *)thread->object;
		if ((driver == NULL) || ((u32)driver->driverID >= len(gGT->drivers)) ||
		    !NativeCheckpoint_IsActivePoolItem(instancePool, instanceSet, driver->instSelf, NULL) ||
		    (driver->instSelf->thread != thread))
		{
			return 0;
		}
	}

	for (u32 i = 0; i < len(gGT->drivers); i++)
	{
		struct Driver *driver = gGT->drivers[i];

		if ((driver != NULL) && !NativeCheckpoint_IsDriverThread(NativeCheckpoint_FindThreadByObject(threadSet, driver)))
		{
			return 0;
		}
	}

	memset(raceOrderSeen, 0, sizeof(raceOrderSeen));
	for (u32 i = 0; i < len(gGT->driversInRaceOrder); i++)
	{
		struct Driver *driver = gGT->driversInRaceOrder[i];
		u32 driverIndex;

		if (driver == NULL)
		{
			continue;
		}
		for (driverIndex = 0; driverIndex < len(gGT->drivers); driverIndex++)
		{
			if (gGT->drivers[driverIndex] == driver)
			{
				break;
			}
		}
		if ((driverIndex == len(gGT->drivers)) || raceOrderSeen[driverIndex])
		{
			return 0;
		}
		raceOrderSeen[driverIndex] = 1;
	}

	return 1;
}

internal int NativeCheckpoint_RelocateActiveInstances(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                      const struct NativeCheckpointPoolSet *poolSet, s32 numPlayers)
{
	if ((poolSet == NULL) || (numPlayers < 1) || (numPlayers > 4))
	{
		return 0;
	}

	for (u32 i = 0; i < poolSet->activeCount; i++)
	{
		NativeCheckpoint_RelocateInstance(oldHeader, liveHeader, (struct Instance *)poolSet->activeItems[i], numPlayers);
	}

	return s_nativeCheckpointRelocationValid;
}

internal int NativeCheckpoint_ValidateActiveInstanceThreads(const struct NativeCheckpointPoolSet *instanceSet,
                                                            const struct JitPool *threadPool,
                                                            const struct NativeCheckpointPoolSet *threadSet)
{
	if ((instanceSet == NULL) || (threadPool == NULL) || (threadSet == NULL))
	{
		return 0;
	}

	for (u32 i = 0; i < instanceSet->activeCount; i++)
	{
		const struct Instance *instance = (const struct Instance *)instanceSet->activeItems[i];

		if ((instance->thread != NULL) && !NativeCheckpoint_IsActivePoolItem(threadPool, threadSet, instance->thread, NULL))
		{
			return 0;
		}
	}

	return 1;
}

internal int NativeCheckpoint_RelocateActiveRain(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                 const struct NativeCheckpointPoolSet *poolSet)
{
	if ((poolSet == NULL) || (poolSet->takenCount != poolSet->activeCount))
	{
		return 0;
	}

	for (u32 i = 0; i < poolSet->activeCount; i++)
	{
		NativeCheckpoint_RelocateRainLocal(oldHeader, liveHeader, (struct RainLocal *)poolSet->activeItems[i]);
	}

	return s_nativeCheckpointRelocationValid;
}

internal void NativeCheckpoint_RelocateParticle(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                struct Particle *particle)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Particle, ptrIconArray),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct Particle, ptrIconGroup),
	    NATIVE_CHECKPOINT_FIELD_IMAGE(struct Particle, funcPtr),
	};

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, particle, fields, len(fields));
	if (particle->funcPtr != Particle_FuncPtr_PotionShatter)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &particle->driverInst);
	}
}

internal int NativeCheckpoint_MarkParticleList(const struct JitPool *particlePool, const struct NativeCheckpointPoolSet *particleSet,
                                               struct Particle *particle, u8 *seen, u32 *seenCount)
{
	while (particle != NULL)
	{
		u32 index;

		if ((seen == NULL) || (seenCount == NULL) || (*seenCount >= particleSet->activeCount) ||
		    !NativeCheckpoint_IsActivePoolItem(particlePool, particleSet, particle, &index) || seen[index])
		{
			return 0;
		}

		seen[index] = 1;
		(*seenCount)++;
		particle = particle->next;
	}

	return 1;
}

internal int NativeCheckpoint_RelocateAndValidateParticles(const struct NativeCheckpointHeader *oldHeader,
                                                           const struct NativeCheckpointHeader *liveHeader,
                                                           struct GameTracker *gGT,
                                                           const struct NativeCheckpointPoolSet *particleSet,
                                                           const struct NativeCheckpointPoolSet *oscillatorSet)
{
	const struct JitPool *particlePool;
	const struct JitPool *oscillatorPool;
	u8 particleSeen[NATIVE_CHECKPOINT_POOL_ITEM_CAP];
	u8 oscillatorSeen[NATIVE_CHECKPOINT_POOL_ITEM_CAP];
	u32 particleCount = 0;
	u32 oscillatorCount = 0;

	if ((gGT == NULL) || (particleSet == NULL) || (oscillatorSet == NULL) ||
	    (particleSet->takenCount != 0) || (oscillatorSet->takenCount != 0) || (gGT->numParticles < 0))
	{
		return 0;
	}
	particlePool = &gGT->JitPools.particle;
	oscillatorPool = &gGT->JitPools.oscillator;

	memset(particleSeen, 0, sizeof(particleSeen));
	memset(oscillatorSeen, 0, sizeof(oscillatorSeen));
	for (u32 i = 0; i < particleSet->activeCount; i++)
	{
		NativeCheckpoint_RelocateParticle(oldHeader, liveHeader, (struct Particle *)particleSet->activeItems[i]);
	}
	if (!s_nativeCheckpointRelocationValid ||
	    !NativeCheckpoint_MarkParticleList(particlePool, particleSet, gGT->particleList_ordinary, particleSeen, &particleCount) ||
	    !NativeCheckpoint_MarkParticleList(particlePool, particleSet, gGT->particleList_heatWarp, particleSeen, &particleCount) ||
	    (particleCount != particleSet->activeCount) || ((u32)gGT->numParticles != particleCount))
	{
		return 0;
	}

	for (u32 i = 0; i < particleSet->activeCount; i++)
	{
		struct ParticleOscillator *oscillator = ((struct Particle *)particleSet->activeItems[i])->oscillator;

		while (oscillator != NULL)
		{
			u32 index;

			if ((oscillatorCount >= oscillatorSet->activeCount) ||
			    !NativeCheckpoint_IsActivePoolItem(oscillatorPool, oscillatorSet, oscillator, &index) || oscillatorSeen[index])
			{
				return 0;
			}

			oscillatorSeen[index] = 1;
			oscillatorCount++;
			oscillator = oscillator->next;
		}
	}

	return oscillatorCount == oscillatorSet->activeCount;
}

internal void NativeCheckpoint_RelocateRDataPointers(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader)
{
	for (u32 i = 0; i < len(rdata.jumpPointers1); i++)
	{
		NativeCheckpoint_RelocateImagePointerSlot(oldHeader, liveHeader, &rdata.jumpPointers1[i]);
	}
	for (u32 i = 0; i < len(rdata.jumpPointers2); i++)
	{
		NativeCheckpoint_RelocateImagePointerSlot(oldHeader, liveHeader, &rdata.jumpPointers2[i]);
	}
	for (u32 i = 0; i < len(rdata.jumpPointers3); i++)
	{
		NativeCheckpoint_RelocateImagePointerSlot(oldHeader, liveHeader, &rdata.jumpPointers3[i]);
	}
	for (u32 i = 0; i < len(rdata.LOAD_TenStages_jumpPointers4); i++)
	{
		NativeCheckpoint_RelocateImagePointerSlot(oldHeader, liveHeader, &rdata.LOAD_TenStages_jumpPointers4[i]);
	}
	for (u32 i = 0; i < len(rdata.jumpPointers5); i++)
	{
		NativeCheckpoint_RelocateImagePointerSlot(oldHeader, liveHeader, &rdata.jumpPointers5[i]);
	}
	for (u32 i = 0; i < len(rdata.jumpPointers6); i++)
	{
		NativeCheckpoint_RelocateImagePointerSlot(oldHeader, liveHeader, &rdata.jumpPointers6[i]);
	}
}

internal void NativeCheckpoint_RelocateMetaDataModel(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                     struct MetaDataMODEL *meta)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct MetaDataMODEL, name),
	    NATIVE_CHECKPOINT_FIELD_IMAGE(struct MetaDataMODEL, LInB),
	    NATIVE_CHECKPOINT_FIELD_IMAGE(struct MetaDataMODEL, LInC),
	};

	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, meta, fields, len(fields));
}

internal void NativeCheckpoint_RelocateDataPointers(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader)
{
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &data.menuRacingWheelConfig);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &data.menuQuit);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &data.menuAdvHub);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &data.menuAdvRace);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &data.menuAdvCup);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &data.menuBattle);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &data.menuArcadeCup);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &data.menuArcadeRace);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &data.menuSaveGame);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &data.menuQueueLoadTrack);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &data.menuGreenLoadSave);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &data.menuFourAdvProfiles);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &data.menuGhostSelection);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &data.menuWarning2);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &data.menuSubmitName);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &data.menuQueueLoadHub);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &data.menuOverwriteAdv);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &data.menuOverwriteGhost);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &data.menuRetryExit);

	for (u32 i = 0; i < len(data.xaLanguagePtrs); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &data.xaLanguagePtrs[i]);
	}
	for (u32 i = 0; i < len(data.audioMeta); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &data.audioMeta[i].name);
	}
	for (u32 i = 0; i < len(data.MetaDataModels); i++)
	{
		NativeCheckpoint_RelocateMetaDataModel(oldHeader, liveHeader, &data.MetaDataModels[i]);
	}
	for (u32 i = 0; i < len(data.ptrRenderedQuadblockDestination_forEachPlayer); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &data.ptrRenderedQuadblockDestination_forEachPlayer[i]);
	}
	for (u32 i = 0; i < len(data.ptrRenderedQuadblockDestination_again); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &data.ptrRenderedQuadblockDestination_again[i]);
	}
	for (u32 i = 0; i < len(data.ptrColor); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &data.ptrColor[i]);
	}
	for (u32 i = 0; i < len(data.opcodeFunc); i++)
	{
		NativeCheckpoint_RelocateImagePointerSlot(oldHeader, liveHeader, &data.opcodeFunc[i]);
	}
	for (u32 i = 0; i < len(data.voiceData); i++)
	{
		for (u32 j = 0; j < len(data.voiceData[i].voiceSet); j++)
		{
			NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &data.voiceData[i].voiceSet[j].ptr);
		}
	}
	for (u32 i = 0; i < len(data.voiceSetPtr); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &data.voiceSetPtr[i]);
	}
	for (u32 i = 0; i < len(data.driverModelExtras); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &data.driverModelExtras[i].fileBase);
	}

	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &data.podiumModel_firstPlace);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &data.podiumModel_secondPlace);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &data.podiumModel_thirdPlace);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &data.podiumModel_tawna);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &data.podiumModel_unk1);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &data.podiumModel_dingoFire);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &data.podiumModel_unk2);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &data.podiumModel_podiumStands);
	NativeCheckpoint_RelocateLoadQueueSlot(oldHeader, liveHeader, &data.currSlot);

	for (u32 i = 0; i < len(data.overlayCallbackFuncs); i++)
	{
		NativeCheckpoint_RelocateImagePointerSlot(oldHeader, liveHeader, &data.overlayCallbackFuncs[i]);
	}
	for (u32 i = 0; i < len(data.metaDataLEV); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &data.metaDataLEV[i].name_Debug);
	}
	for (u32 i = 0; i < len(data.PtrClipBuffer); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &data.PtrClipBuffer[i]);
	}
	for (u32 i = 0; i < len(data.bossWeaponMetaPtr); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &data.bossWeaponMetaPtr[i]);
	}
	for (u32 i = 0; i < len(data.hudStructPtr); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &data.hudStructPtr[i]);
	}
	for (u32 i = 0; i < len(data.MetaDataCharacters); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &data.MetaDataCharacters[i].name_Debug);
	}
	for (u32 i = 0; i < len(data.bakedGteMath); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &data.bakedGteMath[i].physEntry);
	}
	for (u32 i = 0; i < len(data.MetaDataScrub); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &data.MetaDataScrub[i].name);
	}
	for (u32 i = 0; i < len(data.MetaDataTerrain); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &data.MetaDataTerrain[i].name);
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &data.MetaDataTerrain[i].em_OddFrame);
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &data.MetaDataTerrain[i].em_EvenFrame);
	}
}

internal void NativeCheckpoint_RelocateLanguagePointers(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader)
{
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.lngFile);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.lngStrings);

	if ((sdata_static.numLngStrings > 0) && ((u32)sdata_static.numLngStrings <= NATIVE_CHECKPOINT_LNG_STRING_CAP) &&
	    NativeCheckpoint_IsLivePointer(liveHeader, sdata_static.lngStrings))
	{
		for (s32 i = 0; i < sdata_static.numLngStrings; i++)
		{
			NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.lngStrings[i]);
		}
	}
}

internal void NativeCheckpoint_RelocateGhostRecording(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader)
{
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.GhostRecording.ptrGhost);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.GhostRecording.ptrStartOffset);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.GhostRecording.ptrEndOffset);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.GhostRecording.ptrCurrOffset);
}

internal void NativeCheckpoint_RelocateHowlLists(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader)
{
	NativeCheckpoint_RelocateLinkedList(oldHeader, liveHeader, &sdata_static.channelTaken);
	NativeCheckpoint_RelocateLinkedList(oldHeader, liveHeader, &sdata_static.channelFree);
	for (u32 i = 0; i < len(sdata_static.channelStatsPrev); i++)
	{
		NativeCheckpoint_RelocateItemLinks(oldHeader, liveHeader, &sdata_static.channelStatsPrev[i].item);
	}

	NativeCheckpoint_RelocateLinkedList(oldHeader, liveHeader, &sdata_static.Voiceline1);
	NativeCheckpoint_RelocateLinkedList(oldHeader, liveHeader, &sdata_static.Voiceline2);
	for (u32 i = 0; i < len(sdata_static.voicelinePool); i++)
	{
		NativeCheckpoint_RelocateItemLinks(oldHeader, liveHeader, &sdata_static.voicelinePool[i].item);
	}
}

internal void NativeCheckpoint_RelocateSDataPointers(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader)
{
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.arcade_difficultyParams);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.cup_difficultyParams);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.advHubSongSet.ptrSongSetBits);
	for (u32 i = 0; i < len(sdata_static.PausePtrsVRAM); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.PausePtrsVRAM[i]);
	}

	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrMPK);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrLevelFile);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.PatchMem_Ptr);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrBigfileCdPos_2);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.modelMaskHints3D);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.gGT);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.gGamepads);
	NativeCheckpoint_RelocateImagePointerSlot(oldHeader, liveHeader, &sdata_static.MainDrawCb_DrawSyncPtr);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrVlcTable);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.memcard_ptrStart);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.PtrMempack);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.bestHumanRank);
	for (u32 i = 0; i < len(sdata_static.difficultyParams); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.difficultyParams[i]);
	}
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.nav_ptrFirstPoint);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.nav_ptrLastPoint);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.bestRobotRank);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrArray_XaSize);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrArray_NumXAs);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrArray_XaCdPos);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrArray_firstXaIndex);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrArray_numSongs);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrArray_firstSongIndex);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrActiveHighScoreEntry);
	for (u32 i = 0; i < len(sdata_static.ptrGhostTape); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrGhostTape[i]);
		if (NativeCheckpoint_IsLivePointer(liveHeader, sdata_static.ptrGhostTape[i]))
		{
			NativeCheckpoint_RelocateGhostTape(oldHeader, liveHeader, sdata_static.ptrGhostTape[i]);
		}
	}
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrGhostTapePlaying);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrLastBank);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrSampleBlock1);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrSampleBlock2);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrCseqHeader);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrCseqSongStartOffset);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrHowlHeader);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrCseqShortSamples);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrCseqSongData);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.howl_metaEngineFX);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.howl_metaOtherFX);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.howl_spuAddrs);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.howl_songOffsets);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.howl_bankOffsets);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrCseqLongSamples);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.howlChainParams[0]);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.howlChainParams[1]);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrHubAlloc);
	NativeCheckpoint_RelocateLanguagePointers(oldHeader, liveHeader);
	NativeCheckpoint_RelocateGhostRecording(oldHeader, liveHeader);
	NativeCheckpoint_RelocateHowlLists(oldHeader, liveHeader);
	for (u32 i = 0; i < len(sdata_static.songSeq); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.songSeq[i].firstNote);
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.songSeq[i].currNote);
	}
	for (u32 i = 0; i < len(sdata_static.songPool); i++)
	{
		for (u32 j = 0; j < len(sdata_static.songPool[i].CseqSequences); j++)
		{
			NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.songPool[i].CseqSequences[j]);
		}
	}
	NativeCheckpoint_RelocateImagePointerSlot(oldHeader, liveHeader, &sdata_static.callbackCdReadSuccess);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.instMaskHints3D);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrBigfile1);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.PLYROBJECTLIST);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.bossWeaponMeta);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrLoadSaveObj);
	NativeCheckpoint_RelocatePointerOrImageSlot(oldHeader, liveHeader, &sdata_static.ptrActiveMenu);
	NativeCheckpoint_RelocatePointerOrImageSlot(oldHeader, liveHeader, &sdata_static.ptrDesiredMenu);
	NativeCheckpoint_RelocatePointerOrImageSlot(oldHeader, liveHeader, &sdata_static.activeSubMenu);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrPushBufferUI);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrFruitDisp);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrRelic);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrHudCrystal);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrMenuCrystal);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrHudT);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrHudR);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrHudC);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrToken);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ptrTimebox1);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.blank_NavHeader.last);

	for (u32 i = 0; i < len(sdata_static.NavPath_ptrNavFrameArray); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.NavPath_ptrNavFrameArray[i]);
	}
	for (u32 i = 0; i < len(sdata_static.NavPath_ptrHeader); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.NavPath_ptrHeader[i]);
	}
	for (u32 i = 0; i < len(sdata_static.navBotList); i++)
	{
		NativeCheckpoint_RelocateLinkedList(oldHeader, liveHeader, &sdata_static.navBotList[i]);
	}
	for (u32 i = 0; i < len(sdata_static.queueSlots); i++)
	{
		NativeCheckpoint_RelocateLoadQueueSlot(oldHeader, liveHeader, &sdata_static.queueSlots[i]);
	}
	for (u32 i = 0; i < len(sdata_static.quadBlocksRendered); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.quadBlocksRendered[i]);
	}
	for (u32 i = 0; i < len(sdata_static.gamepadSystem.gamepad); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.gamepadSystem.gamepad[i].ptrControllerPacket);
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.gamepadSystem.gamepad[i].rwd);
	}
	for (u32 i = 0; i < len(sdata_static.LoadSaveData); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.LoadSaveData[i].inst);
	}

	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ghostProfile_ptrGhostHeader);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ghostProfile_fileName);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &sdata_static.ghostProfile_fileIconHeader);
	NativeCheckpoint_RelocatePushBuffer(oldHeader, liveHeader, &sdata_static.pushBuffer_DecalMP);
}

internal void NativeCheckpoint_RelocateD230Pointers(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader)
{
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &D230.menuMainMenu);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &D230.menuPlayers1P2P);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &D230.menuPlayers2P3P4P);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &D230.menuDifficulty);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &D230.menuRaceType);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &D230.menuAdventure);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &D230.menuCharacterSelect);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &D230.menuTrackSelect);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &D230.menuCupSelect);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &D230.menuBattleWeapons);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &D230.menuHighScores);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &D230.menuScrapbook);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &D230.menuLapSel);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &D230.menuBattleType);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &D230.menuBattleLengthLifeTime);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &D230.menuBattleLengthTimeTime);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &D230.menuBattleLengthPoints);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &D230.menuBattleLengthLifeLife);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &D230.menuBattleStartGame);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &D230.menuHighScore);

	for (u32 i = 0; i < len(D230.arrayMenuPtrs); i++)
	{
		NativeCheckpoint_RelocatePointerOrImageSlot(oldHeader, liveHeader, &D230.arrayMenuPtrs[i]);
	}
	for (u32 i = 0; i < len(D230.battleMenuArray); i++)
	{
		NativeCheckpoint_RelocatePointerOrImageSlot(oldHeader, liveHeader, &D230.battleMenuArray[i]);
	}
	for (u32 i = 0; i < len(D230.cheats); i++)
	{
		NativeCheckpoint_RelocateImagePointerSlot(oldHeader, liveHeader, &D230.cheats[i].handler);
	}
	for (u32 i = 0; i < len(D230.characterSelectWindowPosByLayout); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &D230.characterSelectWindowPosByLayout[i]);
	}
	for (u32 i = 0; i < len(D230.characterSelectMetaByLayout); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &D230.characterSelectMetaByLayout[i]);
	}
	for (u32 i = 0; i < len(D230.characterSelectTransitionByPlayerCount); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &D230.characterSelectTransitionByPlayerCount[i]);
	}
	for (u32 i = 0; i < len(D230.playerNumberStrings); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &D230.playerNumberStrings[i]);
	}

	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &D230.titleObj);
	if (NativeCheckpoint_IsLivePointer(liveHeader, D230.titleObj))
	{
		NativeCheckpoint_RelocateTitle(oldHeader, liveHeader, D230.titleObj);
	}

	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &D230.activeCharacterSelectWindowPos);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &D230.activeCharacterSelectMeta);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &D230.titleIntroCameraPath);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &D230.characterSelectTransitionMeta);
}

internal void NativeCheckpoint_RelocateV230Pointers(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader)
{
	for (u32 i = 0; i < len(V230.in_Buf); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &V230.in_Buf[i]);
	}
	for (u32 i = 0; i < len(V230.out_Buf); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &V230.out_Buf[i]);
	}
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &V230.ptrCdLoc);
}

internal void NativeCheckpoint_RelocateD231Pointers(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader)
{
	for (u32 i = 0; i < len(D231.minePoolItem); i++)
	{
		NativeCheckpoint_RelocateItemLinks(oldHeader, liveHeader, &D231.minePoolItem[i].item);
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &D231.minePoolItem[i].mineWeapon);
	}

	NativeCheckpoint_RelocateLinkedList(oldHeader, liveHeader, &D231.minePoolTaken);
	NativeCheckpoint_RelocateLinkedList(oldHeader, liveHeader, &D231.minePoolFree);
}

internal void NativeCheckpoint_RelocateD232Pointers(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader)
{
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &D232.menuTokenRelic);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &D232.menuHintMenu);

	for (u32 i = 0; i < len(D232.hubItemsXY_ptrArray); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &D232.hubItemsXY_ptrArray[i]);
	}

	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &D232.ptrPauseObject);
	NativeCheckpoint_RelocateAdventurePauseObject(oldHeader, liveHeader, &D232.pauseObject);
	if ((D232.ptrPauseObject != &D232.pauseObject) && NativeCheckpoint_IsLivePointer(liveHeader, D232.ptrPauseObject))
	{
		NativeCheckpoint_RelocateAdventurePauseObject(oldHeader, liveHeader, D232.ptrPauseObject);
	}
}

internal void NativeCheckpoint_RelocateD233Pointers(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader)
{
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &D233.ptrModelBossHead);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &D233.ptrModelBossBody);
}

internal void NativeCheckpoint_RelocateCreditsObjPointers(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                          struct CreditsObj *creditsObj)
{
	local_persist const struct NativeCheckpointFieldRelocation fields[] = {
	    NATIVE_CHECKPOINT_FIELD_PTR(struct CreditsObj, creditDanceInst),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct CreditsObj, creditsTopString),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct CreditsObj, epilogueTopString),
	    NATIVE_CHECKPOINT_FIELD_PTR(struct CreditsObj, epilogueNextString),
	};

	if (creditsObj == NULL)
	{
		return;
	}

	for (u32 i = 0; i < len(creditsObj->creditGhostModel); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &creditsObj->creditGhostModel[i]);
	}
	for (u32 i = 0; i < len(creditsObj->creditGhostInst); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &creditsObj->creditGhostInst[i]);
	}
	NativeCheckpoint_RelocateFields(oldHeader, liveHeader, creditsObj, fields, len(fields));
}

internal void NativeCheckpoint_RelocateCreditsPointers(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader)
{
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &creditsBSS.creditThread);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &creditsBSS.dancerThread);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &creditsBSS.dancerInst_invisible);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &creditsBSS.ptrStrings);

	if ((creditsBSS.numStrings > 0) && ((u32)creditsBSS.numStrings <= NATIVE_CHECKPOINT_CREDITS_STRING_CAP) &&
	    NativeCheckpoint_IsLivePointer(liveHeader, creditsBSS.ptrStrings))
	{
		for (s32 i = 0; i < creditsBSS.numStrings; i++)
		{
			NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &creditsBSS.ptrStrings[i]);
		}
	}

	NativeCheckpoint_RelocateCreditsObjPointers(oldHeader, liveHeader, &creditsBSS.creditsObj);
}

internal int NativeCheckpoint_RelocateGameTrackerPointers(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader)
{
	struct GameTracker *gGT = &sdata_static.gameTracker;
	struct NativeCheckpointThreadSet activeThreads;
	struct NativeCheckpointPoolSet threadPoolSet;
	struct NativeCheckpointPoolSet instancePoolSet;
	struct NativeCheckpointPoolSet smallStackPoolSet;
	struct NativeCheckpointPoolSet mediumStackPoolSet;
	struct NativeCheckpointPoolSet largeStackPoolSet;
	struct NativeCheckpointPoolSet particlePoolSet;
	struct NativeCheckpointPoolSet oscillatorPoolSet;
	struct NativeCheckpointPoolSet rainPoolSet;
	u32 instanceItemSize;

	if ((gGT->numPlyrCurrGame < 1) || (gGT->numPlyrCurrGame > 4))
	{
		return 0;
	}
	instanceItemSize = sizeof(struct Instance) + (sizeof(struct InstDrawPerPlayer) * gGT->numPlyrCurrGame);

	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->backBuffer);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->frontBuffer);
	for (u32 i = 0; i < len(gGT->db); i++)
	{
		NativeCheckpoint_RelocateDB(oldHeader, liveHeader, &gGT->db[i]);
	}
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->level1);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->level2);

	for (u32 i = 0; i < len(gGT->pushBuffer); i++)
	{
		NativeCheckpoint_RelocatePushBuffer(oldHeader, liveHeader, &gGT->pushBuffer[i]);
	}
	for (u32 i = 0; i < len(gGT->DecalMP); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->DecalMP[i].inst);
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->DecalMP[i].ptrOT1);
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->DecalMP[i].ptrOT2);
	}
	NativeCheckpoint_RelocatePushBuffer(oldHeader, liveHeader, &gGT->pushBuffer_UI);
	for (u32 i = 0; i < len(gGT->cameraDC); i++)
	{
		NativeCheckpoint_RelocateCameraDC(oldHeader, liveHeader, &gGT->cameraDC[i]);
	}

	for (u32 player = 0; player < len(gGT->LevRenderLists); player++)
	{
		for (u32 listIndex = 0; listIndex < len(gGT->LevRenderLists[player].list); listIndex++)
		{
			NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->LevRenderLists[player].list[listIndex].ptrQuadBlocksRendered);
			NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->LevRenderLists[player].list[listIndex].bspListStart);
		}
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->LevRenderLists[player].bspListStart_FullDynamic);
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->LevRenderLists[player].ptrQuadBlocksRendered_FullDynamic);
	}

	for (u32 i = 0; i < len(gGT->otSwapchainDB); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->otSwapchainDB[i]);
	}

	if (!NativeCheckpoint_RelocateJitPool(oldHeader, liveHeader, &gGT->JitPools.thread, sizeof(struct Thread)) ||
	    !NativeCheckpoint_RelocateJitPool(oldHeader, liveHeader, &gGT->JitPools.instance, instanceItemSize) ||
	    !NativeCheckpoint_RelocateJitPool(oldHeader, liveHeader, &gGT->JitPools.smallStack, 0x48) ||
	    !NativeCheckpoint_RelocateJitPool(oldHeader, liveHeader, &gGT->JitPools.mediumStack, 0x88) ||
	    !NativeCheckpoint_RelocateJitPool(oldHeader, liveHeader, &gGT->JitPools.largeStack, 0x670) ||
	    !NativeCheckpoint_RelocateJitPool(oldHeader, liveHeader, &gGT->JitPools.particle, sizeof(struct Particle)) ||
	    !NativeCheckpoint_RelocateJitPool(oldHeader, liveHeader, &gGT->JitPools.oscillator, sizeof(struct ParticleOscillator)) ||
	    !NativeCheckpoint_RelocateJitPool(oldHeader, liveHeader, &gGT->JitPools.rain, sizeof(struct RainLocal)) ||
	    !NativeCheckpoint_PreparePoolSet(liveHeader, &gGT->JitPools.thread, sizeof(struct Thread), &threadPoolSet) ||
	    !NativeCheckpoint_PreparePoolSet(liveHeader, &gGT->JitPools.instance, instanceItemSize, &instancePoolSet) ||
	    !NativeCheckpoint_PreparePoolSet(liveHeader, &gGT->JitPools.smallStack, 0x48, &smallStackPoolSet) ||
	    !NativeCheckpoint_PreparePoolSet(liveHeader, &gGT->JitPools.mediumStack, 0x88, &mediumStackPoolSet) ||
	    !NativeCheckpoint_PreparePoolSet(liveHeader, &gGT->JitPools.largeStack, 0x670, &largeStackPoolSet) ||
	    !NativeCheckpoint_PreparePoolSet(liveHeader, &gGT->JitPools.particle, sizeof(struct Particle), &particlePoolSet) ||
	    !NativeCheckpoint_PreparePoolSet(liveHeader, &gGT->JitPools.oscillator, sizeof(struct ParticleOscillator), &oscillatorPoolSet) ||
	    !NativeCheckpoint_PreparePoolSet(liveHeader, &gGT->JitPools.rain, sizeof(struct RainLocal), &rainPoolSet) ||
	    !NativeCheckpoint_RepairFreeStackObjects(&gGT->JitPools.smallStack, &smallStackPoolSet) ||
	    !NativeCheckpoint_RepairFreeStackObjects(&gGT->JitPools.mediumStack, &mediumStackPoolSet) ||
	    !NativeCheckpoint_RepairFreeStackObjects(&gGT->JitPools.largeStack, &largeStackPoolSet))
	{
		return 0;
	}

	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->visMem1);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->visMem2);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->ptrCircle);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->ptrClod);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->ptrDustpuff);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->ptrSmoking);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->ptrSparkle);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->mpkIcons);
	for (u32 i = 0; i < len(gGT->iconGroup); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->iconGroup[i]);
	}

	for (u32 i = 0; i < len(gGT->threadBuckets); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->threadBuckets[i].thread);
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->threadBuckets[i].s_longName);
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->threadBuckets[i].s_shortName);
	}

	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->ptrRenderBucketInstance);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->particleList_ordinary);
	NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->particleList_heatWarp);
	for (u32 i = 0; i < len(gGT->trafficLightIcon); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->trafficLightIcon[i]);
	}
	for (u32 i = 0; i < len(gGT->ptrIcons); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->ptrIcons[i]);
	}
	for (u32 i = 0; i < len(gGT->modelPtr); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->modelPtr[i]);
	}
	for (u32 i = 0; i < len(gGT->drivers); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->drivers[i]);
	}
	for (u32 i = 0; i < len(gGT->driversInRaceOrder); i++)
	{
		NativeCheckpoint_RelocatePointerSlot(oldHeader, liveHeader, &gGT->driversInRaceOrder[i]);
	}

	if (!NativeCheckpoint_RelocateActiveThreads(oldHeader, liveHeader, gGT, &threadPoolSet, &activeThreads) ||
	    !NativeCheckpoint_ValidateActiveThreadObjects(&activeThreads,
	                                                 &gGT->JitPools.smallStack, &smallStackPoolSet,
	                                                 &gGT->JitPools.mediumStack, &mediumStackPoolSet,
	                                                 &gGT->JitPools.largeStack, &largeStackPoolSet) ||
	    !NativeCheckpoint_RelocateActiveInstances(oldHeader, liveHeader, &instancePoolSet, gGT->numPlyrCurrGame) ||
	    !NativeCheckpoint_ValidateActiveInstanceThreads(&instancePoolSet, &gGT->JitPools.thread, &threadPoolSet) ||
	    !NativeCheckpoint_RelocateActiveRain(oldHeader, liveHeader, &rainPoolSet) ||
	    !NativeCheckpoint_RelocateAndValidateParticles(oldHeader, liveHeader, gGT, &particlePoolSet, &oscillatorPoolSet))
	{
		return 0;
	}

	NativeCheckpoint_RelocateActiveThreadObjects(oldHeader, liveHeader, &activeThreads);
	if (!s_nativeCheckpointRelocationValid ||
	    !NativeCheckpoint_ValidateDrivers(gGT, &activeThreads, &gGT->JitPools.instance, &instancePoolSet))
	{
		return 0;
	}

	return 1;
}

internal int NativeCheckpoint_RelocateRuntimePointers(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader)
{
	s_nativeCheckpointRelocationValid = 1;
	NativeCheckpoint_RelocateRDataPointers(oldHeader, liveHeader);
	NativeCheckpoint_RelocateDataPointers(oldHeader, liveHeader);
	NativeCheckpoint_RelocateSDataPointers(oldHeader, liveHeader);
	NativeCheckpoint_RelocateD230Pointers(oldHeader, liveHeader);
	NativeCheckpoint_RelocateV230Pointers(oldHeader, liveHeader);
	NativeCheckpoint_RelocateD231Pointers(oldHeader, liveHeader);
	NativeCheckpoint_RelocateD232Pointers(oldHeader, liveHeader);
	NativeCheckpoint_RelocateD233Pointers(oldHeader, liveHeader);
	NativeCheckpoint_RelocateRectMenu(oldHeader, liveHeader, &gGarage.menuGarage);
	NativeCheckpoint_RelocateCreditsPointers(oldHeader, liveHeader);
	return s_nativeCheckpointRelocationValid && NativeCheckpoint_RelocateGameTrackerPointers(oldHeader, liveHeader);
}

internal int NativeCheckpoint_CapturePointerSlotState(void *dst, int dstSize)
{
	struct NativeCheckpointHeader liveHeader;
	struct NativeCheckpointPointerSlotState *state = (struct NativeCheckpointPointerSlotState *)dst;
	u32 count = 0;

	if ((dst == NULL) || (dstSize != (int)sizeof(*state)))
	{
		return 0;
	}
	if (!NativeCheckpoint_InitHeader(&liveHeader))
	{
		return 0;
	}
	if (s_nativeCheckpointPointerSlotOverflow)
	{
		return 0;
	}

	memset(state, 0, sizeof(*state));

	for (u32 i = 0; i < s_nativeCheckpointPointerSlotCount; i++)
	{
		u32 slotAddress;
		u32 slotOffset;
		const struct NativeCheckpointAddressRange *slotRange;

		if (count >= NATIVE_CHECKPOINT_POINTER_SLOT_CAP)
		{
			return 0;
		}
		if (!NativeCheckpoint_PtrToU32(s_nativeCheckpointPointerSlots[i], &slotAddress))
		{
			continue;
		}

		slotRange = NativeCheckpoint_FindAddressOwner(&liveHeader, slotAddress, &slotOffset);
		if ((slotRange == NULL) || (slotRange->kind == NATIVE_CHECKPOINT_RANGE_CODE) ||
		    (slotRange->kind == NATIVE_CHECKPOINT_RANGE_DATA))
		{
			continue;
		}
		if ((slotRange->size < sizeof(u32)) || ((slotOffset & (sizeof(u32) - 1u)) != 0) ||
		    (slotOffset > slotRange->size - sizeof(u32)))
		{
			return 0;
		}

		state->records[count].slotRegion = slotRange->kind;
		state->records[count].slotOffset = slotOffset;
		count++;
	}

	state->count = count;
	return 1;
}

internal int NativeCheckpoint_RelocateRegisteredPointerSlot(const struct NativeCheckpointHeader *oldHeader,
                                                            const struct NativeCheckpointHeader *liveHeader, void *slot)
{
	u32 oldAddress;
	u32 newAddress;

	if (!NativeCheckpoint_ReadU32Slot(slot, &oldAddress))
	{
		return 0;
	}
	if (NativeCheckpoint_IsPointerSentinel(oldAddress))
	{
		return 1;
	}
	if (!NativeCheckpoint_RelocateAddress(oldHeader, liveHeader, oldAddress, &newAddress))
	{
		return 0;
	}

	NativeCheckpoint_WriteU32Slot(slot, newAddress);
	return 1;
}

internal int NativeCheckpoint_ComparePointerSlots(const void *lhs, const void *rhs)
{
	const uintptr_t left = (uintptr_t)*(void *const *)lhs;
	const uintptr_t right = (uintptr_t)*(void *const *)rhs;

	return (left > right) - (left < right);
}

internal int NativeCheckpoint_ApplyPointerSlotState(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader,
                                                    const void *src, int srcSize)
{
	const struct NativeCheckpointPointerSlotState *state = (const struct NativeCheckpointPointerSlotState *)src;

	if ((oldHeader == NULL) || (liveHeader == NULL) || (src == NULL) || (srcSize != (int)sizeof(*state)))
	{
		NativeCheckpoint_AbortRelocationTracking();
		return 0;
	}
	if (state->count > NATIVE_CHECKPOINT_POINTER_SLOT_CAP)
	{
		NativeCheckpoint_AbortRelocationTracking();
		return 0;
	}
	for (u32 i = 0; i < len(state->reserved); i++)
	{
		if (state->reserved[i] != 0)
		{
			NativeCheckpoint_AbortRelocationTracking();
			return 0;
		}
	}

	for (u32 i = 0; i < state->count; i++)
	{
		const struct NativeCheckpointPointerSlotRecord *record = &state->records[i];
		const struct NativeCheckpointAddressRange *range = NativeCheckpoint_FindAddressRange(liveHeader, record->slotRegion);
		void *slot = NativeCheckpoint_GetAddressFromRangeOffset(liveHeader, record->slotRegion, record->slotOffset);

		if ((range == NULL) || (record->slotRegion == NATIVE_CHECKPOINT_RANGE_CODE) ||
		    (record->slotRegion == NATIVE_CHECKPOINT_RANGE_DATA) || (range->size < sizeof(u32)) ||
		    ((record->slotOffset & (sizeof(u32) - 1u)) != 0) ||
		    (record->slotOffset > range->size - sizeof(u32)) || (slot == NULL))
		{
			NativeCheckpoint_AbortRelocationTracking();
			return 0;
		}

		if (!NativeCheckpoint_HashContainsPointerSlot(slot) &&
		    !NativeCheckpoint_RelocateRegisteredPointerSlot(oldHeader, liveHeader, slot))
		{
			NativeCheckpoint_AbortRelocationTracking();
			return 0;
		}
		s_nativeCheckpointPointerSlots[i] = slot;
	}

	qsort(s_nativeCheckpointPointerSlots, state->count, sizeof(s_nativeCheckpointPointerSlots[0]), NativeCheckpoint_ComparePointerSlots);
	for (u32 i = 1; i < state->count; i++)
	{
		if (s_nativeCheckpointPointerSlots[i - 1] == s_nativeCheckpointPointerSlots[i])
		{
			NativeCheckpoint_AbortRelocationTracking();
			return 0;
		}
	}

	NativeCheckpoint_ClearPointerSlots();
	for (u32 i = 0; i < state->count; i++)
	{
		void *slot = s_nativeCheckpointPointerSlots[i];

		NativeCheckpoint_RegisterPointerSlot(slot);
		if (s_nativeCheckpointPointerSlotOverflow)
		{
			NativeCheckpoint_ClearPointerSlots();
			return 0;
		}
	}

	return 1;
}

internal int NativeCheckpoint_RelocateMempackPointer(const struct NativeCheckpointHeader *oldHeader,
                                                     const struct NativeCheckpointHeader *liveHeader, void **slot)
{
	const struct NativeCheckpointAddressRange *oldRange = NativeCheckpoint_FindAddressRange(oldHeader, NATIVE_CHECKPOINT_REGION_MPAK);
	const struct NativeCheckpointAddressRange *liveRange = NativeCheckpoint_FindAddressRange(liveHeader, NATIVE_CHECKPOINT_REGION_MPAK);
	u32 oldAddress;
	u32 offset;
	u32 newAddress;

	if ((slot == NULL) || !NativeCheckpoint_ReadU32Slot(slot, &oldAddress) ||
	    !NativeCheckpoint_IsAddressRangeValid(oldRange) || !NativeCheckpoint_IsAddressRangeValid(liveRange))
	{
		return 0;
	}
	if (oldAddress == 0)
	{
		return 1;
	}
	if (s_nativeCheckpointRelocationTracking && NativeCheckpoint_HashContainsPointerSlot(slot))
	{
		return 1;
	}
	if ((oldAddress < oldRange->start) || ((u64)oldAddress > (u64)oldRange->start + oldRange->size))
	{
		return 0;
	}

	offset = oldAddress - oldRange->start;
	if (offset > liveRange->size)
	{
		return 0;
	}
	newAddress = liveRange->start + offset;
	if (!NativeCheckpoint_MarkRelocationSlot(slot))
	{
		return s_nativeCheckpointRelocationValid;
	}

	NativeCheckpoint_WriteU32Slot(slot, newAddress);
	return 1;
}

internal int NativeCheckpoint_RelocateMempackPointers(const struct NativeCheckpointHeader *oldHeader, const struct NativeCheckpointHeader *liveHeader)
{
	for (u32 i = 0; i < len(sdata_static.mempack); i++)
	{
		struct Mempack *mempack = &sdata_static.mempack[i];
		u32 start;
		u32 firstFree;
		u32 lastFree;
		u32 endOfAllocator;
		u32 endOfMemory;

		if ((mempack->numBookmarks < 0) || (mempack->numBookmarks > (s32)len(mempack->bookmarks)) ||
		    !NativeCheckpoint_RelocateMempackPointer(oldHeader, liveHeader, &mempack->start) ||
		    !NativeCheckpoint_RelocateMempackPointer(oldHeader, liveHeader, &mempack->lastFreeByte) ||
		    !NativeCheckpoint_RelocateMempackPointer(oldHeader, liveHeader, &mempack->endOfAllocator) ||
		    !NativeCheckpoint_RelocateMempackPointer(oldHeader, liveHeader, &mempack->endOfMemory) ||
		    !NativeCheckpoint_RelocateMempackPointer(oldHeader, liveHeader, &mempack->firstFreeByte))
		{
			return 0;
		}

		for (u32 bookmarkIndex = 0; bookmarkIndex < len(mempack->bookmarks); bookmarkIndex++)
		{
			if (bookmarkIndex < (u32)mempack->numBookmarks)
			{
				if (!NativeCheckpoint_RelocateMempackPointer(oldHeader, liveHeader, &mempack->bookmarks[bookmarkIndex]))
				{
					return 0;
				}
			}
			else
			{
				mempack->bookmarks[bookmarkIndex] = NULL;
			}
		}

		start = (u32)(uintptr_t)mempack->start;
		firstFree = (u32)(uintptr_t)mempack->firstFreeByte;
		lastFree = (u32)(uintptr_t)mempack->lastFreeByte;
		endOfAllocator = (u32)(uintptr_t)mempack->endOfAllocator;
		endOfMemory = (u32)(uintptr_t)mempack->endOfMemory;
		if (start == 0)
		{
			if ((firstFree != 0) || (lastFree != 0) || (endOfAllocator != 0) || (endOfMemory != 0) ||
			    (mempack->packSize != 0) || (mempack->numBookmarks != 0))
			{
				return 0;
			}
			continue;
		}
		if ((mempack->packSize <= 0) || (firstFree < start) || (lastFree < firstFree) || (endOfMemory < lastFree) ||
		    ((endOfAllocator != 0) && ((endOfAllocator < lastFree) || (endOfAllocator > endOfMemory))))
		{
			return 0;
		}
		for (u32 bookmarkIndex = 0; bookmarkIndex < (u32)mempack->numBookmarks; bookmarkIndex++)
		{
			const u32 bookmark = (u32)(uintptr_t)mempack->bookmarks[bookmarkIndex];

			if ((bookmark < start) || (bookmark > firstFree))
			{
				return 0;
			}
		}
	}

	return 1;
}

internal int NativeCheckpoint_CaptureRegion(u32 kind, void *dst, int dstSize)
{
	void *src;

	if (kind == NATIVE_CHECKPOINT_REGION_D233)
	{
		return NativeCheckpoint_CaptureD233(dst, dstSize);
	}
	if (kind == NATIVE_CHECKPOINT_REGION_PMAP)
	{
		return NativeCheckpoint_CapturePointerSlotState(dst, dstSize);
	}
	if (kind == NATIVE_CHECKPOINT_REGION_NATS)
	{
		return NativeState_Capture(dst, dstSize);
	}

	src = NativeCheckpoint_GetRegionPtr(kind);
	if (src == NULL)
	{
		return 0;
	}

	memcpy(dst, src, (size_t)dstSize);
	return 1;
}

internal int NativeCheckpoint_RestoreRegion(u32 kind, const void *src, int srcSize)
{
	void *dst;

	if (kind == NATIVE_CHECKPOINT_REGION_D233)
	{
		return NativeCheckpoint_RestoreD233(src, srcSize);
	}

	dst = NativeCheckpoint_GetRegionPtr(kind);
	if (dst == NULL)
	{
		return 0;
	}

	memcpy(dst, src, (size_t)srcSize);
	return 1;
}

internal int NativeCheckpoint_InitHeader(struct NativeCheckpointHeader *header)
{
	u32 offset = NativeCheckpoint_Align4((u32)sizeof(*header));
	u32 i;
	local_persist const u32 regionKinds[] = {
	    NATIVE_CHECKPOINT_REGION_RDATA, NATIVE_CHECKPOINT_REGION_DATA, NATIVE_CHECKPOINT_REGION_SDATA, NATIVE_CHECKPOINT_REGION_D230,
	    NATIVE_CHECKPOINT_REGION_V230,  NATIVE_CHECKPOINT_REGION_D231, NATIVE_CHECKPOINT_REGION_D232,  NATIVE_CHECKPOINT_REGION_D233,
	    NATIVE_CHECKPOINT_REGION_GAR3,  NATIVE_CHECKPOINT_REGION_CRD3, NATIVE_CHECKPOINT_REGION_OXSM,  NATIVE_CHECKPOINT_REGION_OXLG,
	    NATIVE_CHECKPOINT_REGION_MPAK,  NATIVE_CHECKPOINT_REGION_SCRP, NATIVE_CHECKPOINT_REGION_PMAP,  NATIVE_CHECKPOINT_REGION_NATS,
	};

	memset(header, 0, sizeof(*header));
	header->magic = NATIVE_CHECKPOINT_MAGIC;
	header->version = NATIVE_CHECKPOINT_VERSION;
	header->regionCount = (u32)len(regionKinds);
	if (!NativeCheckpoint_PtrToU32((const void *)(uintptr_t)&NativeCheckpoint_GetSize, &header->codeAnchor))
	{
		return 0;
	}
	if (!NativeCheckpoint_FillAddressRanges(header))
	{
		return 0;
	}

	if (header->regionCount > len(header->regions))
	{
		return 0;
	}

	for (i = 0; i < header->regionCount; i++)
	{
		const int regionSize = NativeCheckpoint_GetRegionSize(regionKinds[i]);

		if (regionSize <= 0)
		{
			return 0;
		}

		header->regions[i].kind = regionKinds[i];
		header->regions[i].offset = offset;
		header->regions[i].size = (u32)regionSize;
		offset = NativeCheckpoint_Align4(offset + (u32)regionSize);
	}

	header->size = offset;
	return 1;
}

internal int NativeCheckpoint_ValidateHeader(const struct NativeCheckpointHeader *header, int srcSize)
{
	struct NativeCheckpointHeader liveHeader;
	u32 i;

	if ((header == NULL) || (srcSize < (int)sizeof(*header)))
	{
		return 0;
	}
	if ((header->magic != NATIVE_CHECKPOINT_MAGIC) || (header->version != NATIVE_CHECKPOINT_VERSION))
	{
		return 0;
	}
	if ((header->size < sizeof(*header)) || (header->size != (u32)srcSize))
	{
		return 0;
	}
	if ((header->activeMempackIndex < 0) || (header->activeMempackIndex >= (s32)len(sdata_static.mempack)))
	{
		return 0;
	}
	if (!NativeCheckpoint_InitHeader(&liveHeader))
	{
		return 0;
	}
	if ((header->size != liveHeader.size) || (header->regionCount != liveHeader.regionCount))
	{
		return 0;
	}
	if (header->addressRangeCount != liveHeader.addressRangeCount)
	{
		return 0;
	}

	for (i = 0; i < header->addressRangeCount; i++)
	{
		const struct NativeCheckpointAddressRange *range = &header->addressRanges[i];
		const struct NativeCheckpointAddressRange *liveRange = &liveHeader.addressRanges[i];

		if ((range->kind != liveRange->kind) || (range->size != liveRange->size))
		{
			return 0;
		}
		if (!NativeCheckpoint_IsAddressRangeValid(range) || !NativeCheckpoint_IsAddressRangeValid(liveRange))
		{
			return 0;
		}
		for (u32 j = 0; j < i; j++)
		{
			if (header->addressRanges[j].kind == range->kind)
			{
				return 0;
			}
		}
	}

#if defined(__vita__)
	{
		u32 translatedAnchor;

		if (!NativeCheckpoint_RelocateAddressInRange(header, &liveHeader, NATIVE_CHECKPOINT_RANGE_CODE,
		                                             header->codeAnchor & ~1u, &translatedAnchor) ||
		    ((translatedAnchor & ~1u) != (liveHeader.codeAnchor & ~1u)))
		{
			return 0;
		}
	}
#endif

	for (i = 0; i < header->regionCount; i++)
	{
		const struct NativeCheckpointRegion *region = &header->regions[i];
		const struct NativeCheckpointRegion *liveRegion = &liveHeader.regions[i];
		const u32 end = region->offset + region->size;

		if ((region->kind != liveRegion->kind) || (region->offset != liveRegion->offset) || (region->size != liveRegion->size))
		{
			return 0;
		}
		if ((region->size == 0) || (region->offset < sizeof(*header)) || (end < region->offset) || (end > header->size))
		{
			return 0;
		}
	}

	return 1;
}

int NativeCheckpoint_GetSize(void)
{
	struct NativeCheckpointHeader header;

	if (!NativeCheckpoint_InitHeader(&header))
	{
		return 0;
	}

	return (int)header.size;
}

unsigned int NativeCheckpoint_GetFormatVersion(void)
{
	return NATIVE_CHECKPOINT_VERSION;
}

int NativeCheckpoint_Capture(void *dst, int dstSize)
{
	struct NativeCheckpointHeader header;
	u8 *bytes = (u8 *)dst;
	u32 i;

	if (!NativeCheckpoint_InitHeader(&header))
	{
		return 0;
	}
	if ((dst == NULL) || (dstSize < (int)header.size))
	{
		return 0;
	}

	header.psxRandSeed = PSX_BIOS_GetRandSeed();
	header.activeMempackIndex = NativeCheckpoint_GetActiveMempackIndex();
	if ((header.activeMempackIndex < 0) ||
	    (sdata_static.gameTracker.activeMempackIndex != header.activeMempackIndex) ||
	    !NativeCheckpoint_RelocateMempackPointers(&header, &header) ||
	    !NativeCheckpoint_RelocateRuntimePointers(&header, &header))
	{
		return 0;
	}

	memset(dst, 0, header.size);
	memcpy(dst, &header, sizeof(header));

	for (i = 0; i < header.regionCount; i++)
	{
		struct NativeCheckpointRegion *region = &header.regions[i];

		if (!NativeCheckpoint_CaptureRegion(region->kind, &bytes[region->offset], (int)region->size))
		{
			return 0;
		}
	}

	return 1;
}

int NativeCheckpoint_Restore(const void *src, int srcSize)
{
	const struct NativeCheckpointHeader *header = (const struct NativeCheckpointHeader *)src;
	const u8 *bytes = (const u8 *)src;
	const struct NativeCheckpointRegion *nativeStateRegion = NULL;
	const struct NativeCheckpointRegion *pointerMapRegion = NULL;
	struct NativeCheckpointHeader liveHeader;
	u32 i;

	if (!NativeCheckpoint_ValidateHeader(header, srcSize))
	{
		return 0;
	}
	if (!NativeCheckpoint_InitHeader(&liveHeader))
	{
		return 0;
	}

	// NOTE(aalhendi): 233 checkpoints store only mutable overlay state. Restore
	// the source-owned static image first, then overlay the captured runtime
	// fields below.
	OVR233_ResetRuntimeState();

	for (i = 0; i < header->regionCount; i++)
	{
		const struct NativeCheckpointRegion *region = &header->regions[i];

		if (region->kind == NATIVE_CHECKPOINT_REGION_NATS)
		{
			nativeStateRegion = region;
		}
		else if (region->kind == NATIVE_CHECKPOINT_REGION_PMAP)
		{
			pointerMapRegion = region;
		}
		else
		{
			if (!NativeCheckpoint_RestoreRegion(region->kind, &bytes[region->offset], (int)region->size))
			{
				return 0;
			}
		}
	}

	PSX_BIOS_SetRandSeed(header->psxRandSeed);
	Platform_ConfigureMempackArena();
	NativeCheckpoint_BeginRelocationTracking();
	if ((sdata_static.gameTracker.activeMempackIndex != header->activeMempackIndex) ||
	    !NativeCheckpoint_RelocateMempackPointers(header, &liveHeader) ||
	    !NativeCheckpoint_RelocateRuntimePointers(header, &liveHeader))
	{
		NativeCheckpoint_AbortRelocationTracking();
		return 0;
	}
	if (pointerMapRegion == NULL)
	{
		NativeCheckpoint_AbortRelocationTracking();
		return 0;
	}
	if (!NativeCheckpoint_ApplyPointerSlotState(header, &liveHeader, &bytes[pointerMapRegion->offset], (int)pointerMapRegion->size))
	{
		NativeCheckpoint_AbortRelocationTracking();
		return 0;
	}
	Platform_RepairResidentPointers(header->activeMempackIndex);

	if (nativeStateRegion == NULL)
	{
		NativeCheckpoint_AbortRelocationTracking();
		return 0;
	}
	if (!NativeState_Restore(&bytes[nativeStateRegion->offset], (int)nativeStateRegion->size))
	{
		NativeCheckpoint_AbortRelocationTracking();
		return 0;
	}

	return 1;
}
