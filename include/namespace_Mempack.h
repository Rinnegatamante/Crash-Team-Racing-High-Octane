#ifndef CTR_NATIVE_NAMESPACE_MEMPACK_H
#define CTR_NATIVE_NAMESPACE_MEMPACK_H

enum MempackConstants
{
	MEMPACK_BOOKMARK_COUNT = 0x10,

	MEMPACK_ALIGNMENT = 4,
	MEMPACK_ALIGNMENT_MASK = MEMPACK_ALIGNMENT - 1,
	MEMPACK_ALIGNMENT_CLEAR_MASK = -MEMPACK_ALIGNMENT,

	MEMPACK_PS1_RAM_ADDRESS_MASK = 0xffffff,
	MEMPACK_PS1_OVERLAY_ALIGNMENT = 0x800,
	MEMPACK_PS1_OVERLAY_ALIGNMENT_MASK = MEMPACK_PS1_OVERLAY_ALIGNMENT - 1,
	MEMPACK_PS1_END_GUARD_SIZE = 0x800,
	MEMPACK_PS1_END_OF_MEMORY = 0x80200000,
};

#define MEMPACK_ALIGN_SIZE(size) (((size) + MEMPACK_ALIGNMENT_MASK) & MEMPACK_ALIGNMENT_CLEAR_MASK)

struct Mempack
{
	// 0x0
	s32 packSize; // end - start, 0x144E10

	// 0x4
	void *start; // 0x800ba9f0

	// 0x8
	void *lastFreeByte; // where you can put new data on High-End,

	// 0xC
	void *endOfAllocator; // always 0x801FF800

	// 0x10
	void *endOfMemory; // 0x80200000 (2mb) (never used)

	// 0x14
	void *firstFreeByte; // where you can put new data on Low-End

	// 0x18
	s32 sizeOfPrevAllocation; // self-explanatory

	// 0x1C
	s32 numBookmarks; // amount of bookmarks used

	// 0x20
	void *bookmarks[MEMPACK_BOOKMARK_COUNT]; // address of each bookmark

	// 0x60 -- size of struct
};

CTR_STATIC_ASSERT(OFFSETOF(struct Mempack, packSize) == 0x0);
CTR_STATIC_ASSERT(OFFSETOF(struct Mempack, start) == 0x4);
CTR_STATIC_ASSERT(OFFSETOF(struct Mempack, lastFreeByte) == 0x8);
CTR_STATIC_ASSERT(OFFSETOF(struct Mempack, endOfAllocator) == 0xc);
CTR_STATIC_ASSERT(OFFSETOF(struct Mempack, endOfMemory) == 0x10);
CTR_STATIC_ASSERT(OFFSETOF(struct Mempack, firstFreeByte) == 0x14);
CTR_STATIC_ASSERT(OFFSETOF(struct Mempack, sizeOfPrevAllocation) == 0x18);
CTR_STATIC_ASSERT(OFFSETOF(struct Mempack, numBookmarks) == 0x1c);
CTR_STATIC_ASSERT(OFFSETOF(struct Mempack, bookmarks) == 0x20);
CTR_STATIC_ASSERT(sizeof(struct Mempack) == 0x60);

#endif
