#pragma once

#include <stddef.h>

bool overflows_on_sum(size_t a, size_t b);
bool overflows_on_sum(int a, int b);

bool overflows_on_multiply(size_t a, size_t b);