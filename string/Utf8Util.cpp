#include "Utf8Util.h"

#include "../error.h"

#include <immintrin.h>

int u8strcount(const char* str, Utf8StrCount& result)
{
	Utf8StrCount limit { 0,0 };
	return u8strcount(str, result, limit);
}

int u8strcount(const char* str, Utf8StrCount& result, Utf8StrCount& limit)
{
	result.size = 0;
	result.length = 0;

	if (str == nullptr)
		return E_NULL;

	limit.size = (limit.size == 0) * __SIZE_MAX__ + (limit.size != 0) * limit.size;
	limit.length = (limit.length == 0) * __SIZE_MAX__ + (limit.length != 0) * limit.length;

	uint8_t ch;
	size_t next_bytes_left = 0;
	size_t ones_count;
	uint8_t ch_mask;
	while (*str != '\0')
	{
		ch = *str;

		if (next_bytes_left == 0)
		{
			if (ch < 128)
			{
				result.size += 1;
				result.length += 1;

				if (result.size == limit.size)
					return E_OK;

				if (result.length == limit.length)
					return E_OK;
			}
			else
			{
				// count 1s from left, indicates how many next bytes
				// opts: 2, 3, 4

				ones_count = 0;
				ch_mask = 0x80;

				while (ones_count < 6)
				{
					if ((ch & ch_mask) > 0)
					{
						ones_count += 1;
						ch_mask >>= 1;
					}
					else break;
				}

				if (ones_count < 2 || ones_count > 4)
				{
					return E_UTF8_INVALID;
				}

				next_bytes_left = ones_count - 1;
				
				result.size += 1;// count in this byte

				if (result.size == limit.size)
					return E_UTF8_INVTERM;
			}

			str += 1;
		}
		else
		{
			while (next_bytes_left > 0)
			{
				next_bytes_left -= 1;
				ch = *str;

				if ((ch & 0xC0) != 0x80)
					return E_UTF8_INVALID;

				result.size += 1;
				if (result.size == limit.size)
				{
					if (next_bytes_left == 0)
						return E_OK;

					return E_UTF8_INVTERM;
				}

				str += 1;
				if (*str == '\0')
				{
					if (next_bytes_left == 0)
						return E_OK;
					else return E_UTF8_INVTERM;
				}
			}

			result.length += 1;

			if (result.length == limit.length)
				return E_OK;
		}
	}

	return E_OK;
}

int u8validate(const char* str, bool& valid)
{
	valid = true;

	Utf8StrCount result;
	Utf8StrCount limit { 0,0 };
	int error = u8strcount(str, result, limit);

	switch (error)
	{
	case E_OK: break;
	case E_UTF8_INVALID:
	case E_UTF8_INVTERM:
		valid = false;
		break;
	case E_NULL:
		return E_NULL;
	}

	return E_OK;
}

bool u8_is_literal(uint32_t ch)
{
	return 
		( // c-printable
			ch == 0x09 ||
			ch == 0x0A ||
			ch == 0x0D ||
			(ch >= 0x20 && ch <= 0x7E) ||
			ch == 0x85 ||
			(ch >= 0xA0 && ch <= 0xD7FF) ||
			(ch >= 0xE000 && ch <= 0xFFFD) ||
			(ch >= 0x010000 && ch <= 0x10FFFF)
		) 
		&& (
			ch != 0x0A && ch != 0x0D && // b-char
			ch != 0xFEFF // b-byteorder-mark
			)
		;
}
