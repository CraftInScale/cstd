#pragma once

#include <stdint.h>
#include <stddef.h>

struct Md5State
{
	uint32_t A, B, C, D;
};

void md5(Md5State& result, void* data, size_t data_size);