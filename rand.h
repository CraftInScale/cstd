#pragma once

#include <stddef.h>
#include <stdint.h>

int gen_rand_block(int seed, size_t size, uint8_t*& block);
