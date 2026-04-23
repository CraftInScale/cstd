#include "rangecheck.h"

#include <limits.h>

bool overflows_on_sum(size_t a, size_t b)
{
	return b > 0 && a > __SIZE_MAX__ - b;
}

bool overflows_on_sum(int a, int b)
{
	return b > 0 && a > INT_MAX - b;
}

bool overflows_on_multiply(size_t a, size_t b)
{
	return b != 0 && a > __SIZE_MAX__ / b;
}
