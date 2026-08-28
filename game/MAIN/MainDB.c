#include <common.h>

#if defined(CTR_NATIVE)
enum
{
	MAINDB_NATIVE_PRIMMEM_CAPACITY = 0x40000,
	MAINDB_NATIVE_PRIMMEM_CANARY_WORDS = 16,
};

struct MainDBNativePrimMemArena
{
	u32 words[MAINDB_NATIVE_PRIMMEM_CAPACITY / sizeof(u32)];
	u32 canary[MAINDB_NATIVE_PRIMMEM_CANARY_WORDS];
};

static struct MainDBNativePrimMemArena s_mainDbNativePrimMem[2] __attribute__((aligned(64)));
static u32 s_mainDbNativePrimMemRetailBudget[2];
static u32 s_mainDbNativePrimMemHighWater[2];
static u32 s_mainDbNativePrimMemLastLogged[2];

static int MainDB_NativePrimMemIndex(const struct PrimMem *primMem)
{
	struct GameTracker *gGT = sdata->gGT;
	if (gGT == NULL)
	{
		return -1;
	}

	for (int i = 0; i < 2; i++)
	{
		if (primMem == &gGT->db[i].primMem)
		{
			return i;
		}
	}
	return -1;
}

static void MainDB_NativePrimMemArmCanary(int index)
{
	for (int i = 0; i < MAINDB_NATIVE_PRIMMEM_CANARY_WORDS; i++)
	{
		s_mainDbNativePrimMem[index].canary[i] = 0xc0defaceu ^ (u32)(index << 16) ^ (u32)i;
	}
}

static void MainDB_NativePrimMemBind(struct PrimMem *primMem, int index)
{
	void *start = s_mainDbNativePrimMem[index].words;
	primMem->capacityBytes = MAINDB_NATIVE_PRIMMEM_CAPACITY;
	primMem->allocationStart = start;
	primMem->start = start;
	primMem->cursor = start;
	primMem->end = (void *)((char *)start + MAINDB_NATIVE_PRIMMEM_CAPACITY);
	primMem->guardEnd = (void *)((char *)primMem->end - 0x100);
	primMem->primitiveCount = 0;
	MainDB_NativePrimMemArmCanary(index);
}

void MainDB_RebindNativePrimMem(struct GameTracker *gGT)
{
	if (gGT == NULL)
	{
		return;
	}

	for (int i = 0; i < 2; i++)
	{
		MainDB_NativePrimMemBind(&gGT->db[i].primMem, i);
	}
}

void MainDB_NativePrimMemFrameEnd(struct PrimMem *primMem)
{
	int index = MainDB_NativePrimMemIndex(primMem);
	if (index < 0)
	{
		return;
	}

	uintptr_t start = (uintptr_t)primMem->start;
	uintptr_t cursor = (uintptr_t)primMem->cursor;
	u32 used = cursor >= start ? (u32)(cursor - start) : 0xffffffffu;

	if (used > s_mainDbNativePrimMemHighWater[index])
	{
		s_mainDbNativePrimMemHighWater[index] = used;
	}

	u32 retailBudget = s_mainDbNativePrimMemRetailBudget[index];
	if ((retailBudget != 0) && (used > retailBudget - 0x100) &&
	    ((s_mainDbNativePrimMemLastLogged[index] == 0) || (used >= s_mainDbNativePrimMemLastLogged[index] + 0x1000)))
	{
		s_mainDbNativePrimMemLastLogged[index] = used;
		Platform_Log("[CTR Native] PrimMem exceeded retail budget db=%d used=%u retail=%u capacity=%u\n",
		             index, used, retailBudget, (u32)MAINDB_NATIVE_PRIMMEM_CAPACITY);
	}

	for (int i = 0; i < MAINDB_NATIVE_PRIMMEM_CANARY_WORDS; i++)
	{
		u32 expected = 0xc0defaceu ^ (u32)(index << 16) ^ (u32)i;
		if (s_mainDbNativePrimMem[index].canary[i] != expected)
		{
			Platform_Log("[CTR Native] PrimMem CANARY CORRUPTION db=%d used=%u capacity=%u word=%d got=0x%08x\n",
			             index, used, (u32)MAINDB_NATIVE_PRIMMEM_CAPACITY, i, s_mainDbNativePrimMem[index].canary[i]);
			MainDB_NativePrimMemArmCanary(index);
			break;
		}
	}
}
#endif

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80034960-0x800349c4.
int MainDB_GetClipSize(u32 levelID, int numPlyrCurrGame)
{
	switch (levelID)
	{
	case ADVENTURE_GARAGE:
		return 24000;

	case MAIN_MENU_LEVEL:
		return 16;

	case SEWER_SPEEDWAY:
		return 6000;

	case MYSTERY_CAVES:
		return 2500;

	case PAPU_PYRAMID:
	case POLAR_PASS:
		if (numPlyrCurrGame < 3)
		{
			return 3000;
		}

		return 2500;

	default:
		return 3000;
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800349c4-0x80034a28.
void MainDB_PrimMem(struct PrimMem *primMem, u32 size)
{
	u32 alignedSize;
	void *pvVar1;

	// Preserve the retail mempack layout even on native. The allocation is kept
	// as a placeholder, while GPU primitive packets use a larger native-only
	// arena so legacy unchecked packet writers cannot corrupt following mempack data.
	pvVar1 = MEMPACK_AllocMem(size);
#if defined(CTR_NATIVE)
	int nativeIndex = MainDB_NativePrimMemIndex(primMem);
	if (nativeIndex >= 0)
	{
		s_mainDbNativePrimMemRetailBudget[nativeIndex] = size;
		s_mainDbNativePrimMemHighWater[nativeIndex] = 0;
		s_mainDbNativePrimMemLastLogged[nativeIndex] = 0;
		MainDB_NativePrimMemBind(primMem, nativeIndex);
		return;
	}
#endif

	primMem->capacityBytes = size;
	primMem->allocationStart = pvVar1;
	primMem->cursor = pvVar1;
	primMem->start = pvVar1;

	alignedSize = (size >> 2) << 2;
	pvVar1 = (void *)((int)pvVar1 + alignedSize);
	primMem->end = pvVar1;
	primMem->guardEnd = (void *)((int)pvVar1 - 0x100);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80034a28-0x80034a80.
void MainDB_OTMem(struct OTMem *otMem, u32 size)
{
	u32 alignedSize;
	void *pvVar1;

	pvVar1 = MEMPACK_AllocMem(size);
	otMem->capacityBytes = size;
	otMem->cursor = pvVar1;
	otMem->start = pvVar1;

	alignedSize = (size >> 2) << 2;
	otMem->end = (void *)((int)pvVar1 + alignedSize);
}
