#include "mem.h"

#include <stdint.h>

void memcpybackward(void* dst, void* src, size_t size)
{
	const uint8_t* src1 = ((uint8_t*)src) + size;
	const uint8_t* dst1 = ((uint8_t*)dst) + size;

	size_t block_size = sizeof(uint64_t);
	size_t nblocks = size / block_size;
	size -= nblocks * block_size;

	while (nblocks > 0)
	{
		src1 -= block_size;
		dst1 -= block_size;

		*((uint64_t*)dst1) = *((uint64_t*)src1);

		nblocks -= 1;
	}

	block_size = sizeof(uint32_t);
	if (size >= block_size)
	{
		size -= block_size;

		src1 -= block_size;
		dst1 -= block_size;

		*((uint32_t*)dst1) = *((uint32_t*)src1);
	}

	block_size = sizeof(uint16_t);
	if (size >= block_size)
	{
		size -= block_size;

		src1 -= block_size;
		dst1 -= block_size;

		*((uint16_t*)dst1) = *((uint16_t*)src1);
	}

	block_size = sizeof(uint8_t);
	if (size >= block_size)
	{
		size -= block_size;

		src1 -= block_size;
		dst1 -= block_size;

		*((uint8_t*)dst1) = *((uint8_t*)src1);
	}
}
