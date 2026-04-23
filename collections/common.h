#pragma once

#include "hash/hash.h"

#include <stddef.h>

size_t closest_higher_power_of_2(size_t n);
size_t pad_to_8(size_t n);
size_t pad_to_4(size_t n);
size_t pad_to_2(size_t n);
size_t pad(size_t n);