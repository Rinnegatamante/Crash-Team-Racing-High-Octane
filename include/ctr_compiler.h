#ifndef CTR_COMPILER_H
#define CTR_COMPILER_H

#include <stdlib.h>

// NOTE(aalhendi): The MSVC C runtime exposes non-standard min/max macros in C mode. They collide with the project's typed helpers even when NOMINMAX is set.
#if defined(_MSC_VER)
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#endif

#if defined(_MSC_VER)
#define CTR_PRAGMA(value) __pragma(value)
#else
#define CTR_PRAGMA_IMPL(value) _Pragma(#value)
#define CTR_PRAGMA(value)      CTR_PRAGMA_IMPL(value)
#endif

#define CTR_PACKED_BEGIN CTR_PRAGMA(pack(push, 1))
#define CTR_PACKED_END   CTR_PRAGMA(pack(pop))

#if defined(__GNUC__) || defined(__clang__)
#define CTR_MAY_ALIAS __attribute__((may_alias))
#define CTR_PRINTF_FORMAT(fmtArg, firstVararg) __attribute__((format(printf, fmtArg, firstVararg)))
#define CTR_TRAP() __builtin_trap()
#else
#define CTR_MAY_ALIAS
#define CTR_PRINTF_FORMAT(fmtArg, firstVararg)
#define CTR_TRAP() abort()
#endif

#if defined(__GNUC__) && !defined(__clang__)
#define CTR_GCC_OPTIMIZE_O0 __attribute__((optimize("O0")))
#else
#define CTR_GCC_OPTIMIZE_O0
#endif

#if defined(CTR_NATIVE)
#define CTR_FORCE_INLINE static inline
#elif defined(_MSC_VER)
#define CTR_FORCE_INLINE static __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define CTR_FORCE_INLINE static inline __attribute__((always_inline))
#else
#define CTR_FORCE_INLINE static inline
#endif

#endif
