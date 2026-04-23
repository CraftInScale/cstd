#pragma once

#include <stddef.h>
#include <stdint.h>

//#define __AVX2__
#ifdef __AVX2__
#define HGROUP_SIZE 32
struct HashGroup
{
	uint8_t state[32];
};
struct HashGroupIndirections
{
	size_t pointers[32];
	size_t indexes[32];
};
#elif defined(__SSE2__)
#define HGROUP_SIZE 16
struct HashGroup
{
	uint8_t state[16];
};
struct HashGroupIndirections
{
	size_t pointers[16];
	size_t indexes[16];
};
#endif

// default hash - Murmur3

size_t hfunc_md5(void* data, size_t data_size);
size_t hfunc_murmur3(void* data, size_t data_size);
size_t hfunc_scalar(void* data, size_t data_size);

size_t hfunc_murmur3_string(void* data, size_t data_size);

int cmpfunc_default(void* elm1, void* elm2, size_t size);
int cmpfunc_string(void* elm1, void* elm2, size_t size);