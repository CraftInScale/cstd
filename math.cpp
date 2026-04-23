#include "math.h"

int pow10(int n)
{
	int m = 1;

	while (n > 0)
	{
		m *= 10;
		n -= 1;
	}

	return m;
}

int pow16(int n)
{
	int m = 1;

	while (n > 0)
	{
		m *= 16;
		n -= 1;
	}

	return m;
}

size_t pow(size_t base, size_t exponent)
{
	size_t n = 1;

	while (exponent > 0)
	{
		n *= base;
		exponent -= 1;
	}

	return n;
}

int pow(int base, unsigned int exponent)
{
	int n = 1;

	while (exponent > 0)
	{
		n *= base;
		exponent -= 1;
	}

	return n;
}

uint8_t pow(uint8_t base, unsigned int exponent)
{
	uint8_t n = 1;

	while (exponent > 0)
	{
		n *= base;
		exponent -= 1;
	}

	return n;
}