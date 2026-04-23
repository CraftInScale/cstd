#include "rand.h"

#include "error.h"

#include <stdlib.h>

int gen_rand_block(int seed, size_t size, uint8_t*& block)
{
	block = (uint8_t*)malloc(32);
	if (block == nullptr)
	{
		return E_NULL;
	}

	// generate random data
	srand(seed);

	size_t i = 0;
	while (i < size)
	{
		*(block + i) = rand() % 256;
		i += 1;
	}

	return E_OK;
}