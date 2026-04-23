#include "hash.h"

#include "md5.h"
#include "murmur3.h"
#include "../../minmax.h"
#include "../../string/String.h"

#include <string.h>

#define COMMON_HFUNC_SEED 0xE157'F48A'341E'CA82ull
#define COMMON_HFUNC_SEED32 0x341E'CA82

size_t hfunc_md5(void* data, size_t data_size)
{
	Md5State result;

	md5(result, data, data_size);

	return (size_t)(
		(((size_t)result.A) | (((size_t)result.B) << 32)) ^
		(((size_t)result.C) | (((size_t)result.D) << 32))
		);
}

size_t hfunc_murmur3(void* data, size_t data_size)
{
	uint64_t result[2];

	MurmurHash3_x64_128(data, data_size, COMMON_HFUNC_SEED32, result);

	return result[0] ^ result[1];
}

size_t hfunc_scalar(void* data, size_t data_size)
{
	size_t key = 0;
	memcpy(&key, data, min(data_size, 8));
	return key;
}

size_t hfunc_murmur3_string(void* data, size_t data_size)
{
	uint64_t result[2];

	String* s = (String*)data;

	MurmurHash3_x64_128(s->data, s->size - 1, COMMON_HFUNC_SEED32, result);

	return result[0] ^ result[1];
}

int cmpfunc_default(void* elm1, void* elm2, size_t size)
{
	return strncmp((const char*)elm1, (const char*)elm2, size);
}

int cmpfunc_string(void* elm1, void* elm2, size_t size)
{
	String* s1 = (String*)elm1;
	String* s2 = (String*)elm2;

	if (s1->size > s2->size)
		return 1;
	else if (s1->size < s2->size)
		return -1;
	
	return strncmp((char*)s1->data, (char*)s2->data, s2->size-1);
}
