#ifndef CTR_NATIVE_POSTFX_BINARIES_H
#define CTR_NATIVE_POSTFX_BINARIES_H

#include <stddef.h>

typedef struct NativePostFXBinary
{
	const char *name;
	const void *vertex;
	size_t vertex_size;
	const void *fragment;
	size_t fragment_size;
} NativePostFXBinary;

extern const NativePostFXBinary native_postfx_binaries[];
extern const unsigned native_postfx_binary_count;

#endif
