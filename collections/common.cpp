#include "common.h"

#include "../minmax.h"

inline size_t gen_1(int off)
{
	return (1ull << 63) >> off;
}

int find_leftmost_bit(size_t n)
{
	int off = 0;
	int max_off = sizeof(n) * 8;

	while (off < max_off)
	{
		size_t m = n & gen_1(off);
		if (m > 0)
			return off;

		off += 1;
	}

	return off;
}

size_t closest_higher_power_of_2(size_t n)
{
	int off = find_leftmost_bit(n);
	if (off >= 64)
		return 8;

	if (off == 0)
		return 0;// unreachable?

	return gen_1(off - 1);
}

size_t pad_to_8(size_t n)
{
	size_t low_part = n & 0b0111;
	size_t missing_to_add = 8 - low_part;

	return low_part > 0 ? (n + missing_to_add) : n;
}

size_t pad_to_4(size_t n)
{
	size_t low_part = n & 0b011;
	size_t missing_to_add = 4 - low_part;

	return low_part > 0 ? (n + missing_to_add) : n;
}

size_t pad_to_2(size_t n)
{
	size_t low_part = n & 0b01;

	return low_part > 0 ? (n + 1) : n;
}

size_t pad(size_t n)
{
	if (n < 8)
	{
		if (n < 4)
		{
			if (n < 2)
			{
				return 1;
			}
			else return pad_to_2(n);
		}
		else return pad_to_4(n);
	}
	else return pad_to_8(n);
}