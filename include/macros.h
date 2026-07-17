#ifndef MACROS_H
#define MACROS_H

#include <ctr_compiler.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CTR_STATIC_ASSERT(expr) _Static_assert((expr), #expr)

typedef uint64_t u64;
typedef int64_t s64;
typedef uint32_t u32;
typedef int32_t s32;
typedef s32 b32;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint8_t u8;
typedef int8_t s8;
typedef float f32;
typedef double f64;

#define AugReview 805
// TODO: Aug5 and Aug14
#define SepReview 903
#define UsaRetail 926
#define JpnTrial  1006
#define EurRetail 1020
#define JpnRetail 1111

#if BUILD == EurRetail
#define SCREEN_HEIGHT 236
#define FPS           25
#define ELAPSED_MS    40
#else
#define SCREEN_HEIGHT 216
#define FPS           30
#define ELAPSED_MS    32
#endif
#define SCREEN_WIDTH               512
#define SECOND                     (FPS * ELAPSED_MS)
#define MINUTE                     (SECOND * 60)
#define HOUR                       (MINUTE * 60)

#define CTR_FRAMES_PER_SECOND      30
#define CTR_SECONDS_TO_FRAMES(sec) ((s32)((sec) * CTR_FRAMES_PER_SECOND))

#define SECONDS(x)                 ((s32)(((f32)(x)) * SECOND))
#define MINUTES(x)                 ((s32)(((f32)(x)) * MINUTE))
#define HOURS(x)                   ((s32)(((f32)(x)) * HOUR))

#define nullptr                    ((void *)0)

#define force_inline CTR_FORCE_INLINE

#define internal                static
#define local_persist           static
#define global_variable         static

#define len(arr)                (sizeof(arr) / sizeof(arr[0]))
#define OFFSETOF(TYPE, ELEMENT) ((u32)offsetof(TYPE, ELEMENT))
#define CTR_OFFSET_OF_ARRAY(TYPE, MEMBER, INDEX) \
	(offsetof(TYPE, MEMBER) + (size_t)(INDEX) * sizeof(((TYPE *)0)->MEMBER[0]))
#define CTR_OFFSET_OF_2D_ARRAY(TYPE, MEMBER, ROW, COLUMN)                                                        \
	(offsetof(TYPE, MEMBER) + (size_t)(ROW) * sizeof(((TYPE *)0)->MEMBER[0]) + (size_t)(COLUMN) * sizeof(((TYPE *)0)->MEMBER[0][0]))

force_inline u16 CTR_ReadU16LE(const void *src)
{
	const u8 *bytes = (const u8 *)src;

	return (u16)((u16)bytes[0] | ((u16)bytes[1] << 8));
}

force_inline u32 CTR_ReadU32LE(const void *src)
{
	const u8 *bytes = (const u8 *)src;

	return (u32)bytes[0] | ((u32)bytes[1] << 8) | ((u32)bytes[2] << 16) | ((u32)bytes[3] << 24);
}

force_inline void CTR_WriteU16LE(void *dst, u16 value)
{
	u8 *bytes = (u8 *)dst;

	bytes[0] = (u8)value;
	bytes[1] = (u8)(value >> 8);
}

force_inline void CTR_WriteU32LE(void *dst, u32 value)
{
	u8 *bytes = (u8 *)dst;

	bytes[0] = (u8)value;
	bytes[1] = (u8)(value >> 8);
	bytes[2] = (u8)(value >> 16);
	bytes[3] = (u8)(value >> 24);
}

// Raw [3] vector array helpers. Arguments must be side-effect-free lvalues.
#define CTR_COPY_VEC3(DST, SRC) \
	do                          \
	{                           \
		(DST)[0] = (SRC)[0];    \
		(DST)[1] = (SRC)[1];    \
		(DST)[2] = (SRC)[2];    \
	} while (0)

#define CTR_SET_VEC3(DST, X, Y, Z) \
	do                             \
	{                              \
		(DST)[0] = (X);            \
		(DST)[1] = (Y);            \
		(DST)[2] = (Z);            \
	} while (0)

// Retail format strings use PsyQ `%ld` for 32-bit values. Keep call sites on
// project-width types while satisfying host printf varargs for the literal.
#define CTR_PRINTF_PSX_LONG(value) ((long)(s32)(value))

#define RGBtoBGR(color)            ((color & 0xFF0000) >> 16) | (color & 0xFF00) | ((color & 0xFF) << 16)

#define GetRed(color)              (color & 0xFF)

#define GetGreen(color)            (color & 0xFF00) >> 8

#define GetBlue(color)             (color & 0xFF0000) >> 16

#define aspectratioupsample(int)   (int * 7) / 4

#define aspectratiodownsample(int) (int * 4) / 7

#endif
