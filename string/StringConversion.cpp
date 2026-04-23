#include "StringConversion.h"

#include "../error.h"
#include "../math.h"

#include <stdlib.h>

int number_to_string(int n, String& append_to)
{
	constexpr int size = 11;
	char arr[size] = { 0 };

	bool negative = n < 0;
	int i = size - 1;
	while (true)
	{
		int m = n % 10;
		n /= 10;

		arr[i] = '0' + (m >= 0 ? m : (-m));
		i -= 1;

		if (n == 0)
			break;
	}

	if (negative)
	{
		arr[i] = '-';
		i -= 1;
	}

	return string_concat(append_to, arr + (i+1), size - (i + 1), size - (i + 1));
}

int number_to_string(long n, String& append_to)
{
	constexpr int size = 21;
	char arr[size] = { 0 };

	bool negative = n < 0;
	int i = size - 1;
	while (true)
	{
		int m = n % 10;
		n /= 10;

		arr[i] = '0' + (m >= 0 ? m : (-m));
		i -= 1;

		if (n == 0)
			break;
	}

	if (negative)
	{
		arr[i] = '-';
		i -= 1;
	}

	return string_concat(append_to, arr + (i + 1), size - (i + 1), size - (i + 1));
}

int number_to_string(size_t n, String& append_to)
{
	constexpr int size = 20;
	char arr[size] = { 0 };

	int i = size - 1;
	while (true)
	{
		int m = n % 10;
		n /= 10;

		arr[i] = '0' + (m >= 0 ? m : (-m));
		i -= 1;

		if (n == 0)
			break;
	}

	return string_concat(append_to, arr + (i + 1), size - (i + 1), size - (i + 1));
}

int number_to_string(uint8_t n, String& append_to)
{
	constexpr int size = 3;
	char arr[size] = { 0 };

	int i = size - 1;
	while (true)
	{
		int m = n % 10;
		n /= 10;

		arr[i] = '0' + (m >= 0 ? m : (-m));
		i -= 1;

		if (n == 0)
			break;
	}

	return string_concat(append_to, arr + (i + 1), size - (i + 1), size - (i + 1));
}

int number_to_string(uint16_t n, String& append_to)
{
	constexpr int size = 5;
	char arr[size] = { 0 };

	int i = size - 1;
	while (true)
	{
		int m = n % 10;
		n /= 10;

		arr[i] = '0' + (m >= 0 ? m : (-m));
		i -= 1;

		if (n == 0)
			break;
	}

	return string_concat(append_to, arr + (i + 1), size - (i + 1), size - (i + 1));
}

int number_to_hex(uint64_t n, String& add_to)
{
	char str[17];
	str[16] = '\0';

	size_t i = 15;
	while (true)
	{
		uint64_t digit = n % 16;
		n /= 16;

		if (digit < 10)
		{
			str[i] = '0' + digit;
		}
		else
		{
			str[i] = 'A' + (digit - 10);
		}

		if (i == 0 || n == 0)
			break;

		i -= 1;
	}

	return string_concat(add_to, str + i, 16 - i);
}

int string_to_bool(const char* s, size_t s_len, bool& result)
{
	const char* _true = "true";
	const char* _false = "false";

	StringIter match1, match2;
	init_stringiter_forward(match1, _true, 4);
	init_stringiter_forward(match2, _false, 5);

	StringIter iter;
	init_stringiter_forward(iter, s, s_len);

	bool match = false;
	int char_count = 0;

	uint32_t ch, ch1;
	int error;
	while (true)
	{
		error = stringiter_next(iter, ch);
		if (error != E_OK)
		{
			if (error == E_NULL)
			{
				break;
			}

			return error;
		}

		error = stringiter_next(match1, ch1);
		if (error != E_OK)
		{
			if (error == E_NULL)
			{
				break;
			}
			else return error;
		}

		if (ch != ch1)
		{
			break;
		}
		else char_count += 1;

		if (char_count == 4)
		{
			result = true;
			return E_OK;
		}
	}

	stringiter_reset(iter);
	while (true)
	{
		error = stringiter_next(iter, ch);
		if (error != E_OK)
		{
			if (error == E_NULL)
			{
				break;
			}

			return error;
		}

		error = stringiter_next(match2, ch1);
		if (error != E_OK)
		{
			if (error == E_NULL)
			{
				match = true;
				break;
			}
			else return error;
		}

		if (ch != ch1)
		{
			break;
		}
		else char_count += 1;

		if (char_count == 5)
		{
			result = false;
			return E_OK;
		}
	}

	return E_INVALID;
}

int string_to_number(const char* s, size_t s_len, int8_t& result)
{
	constexpr size_t max_digits_count = 3;

	if (s_len == 0)
		return E_INVALID;

	uint8_t digits[max_digits_count] = { 0 };
	size_t digit_count = 0;

	bool negative = false;

	StringIter iter;
	init_stringiter_forward(iter, s, s_len);

	uint32_t ch;
	int error;
	size_t i = 0;
	while (true)
	{
		error = stringiter_next(iter, ch);
		if (error != E_OK)
		{
			if (error == E_NULL)
				break;
			
			return error;
		}

		if (ch == '-')
		{
			if (i == 0)
			{
				negative = true;
			}
			else return E_INVALID;
		}
		else if (ch >= '0' && ch <= '9')
		{
			if (digit_count == max_digits_count)
				return E_OUT_OF_RANGE;

			digits[digit_count] = ch;
			digit_count += 1;
		}
		else return E_INVALID;

		i += 1;
	}

	i = 0;
	int16_t number = 0;
	unsigned int exponent = digit_count - 1;
	while (i < digit_count)
	{
		number += pow(10, exponent) * (digits[i] - '0');
		exponent -= 1;
		i += 1;
	}

	if (negative)
	{
		number *= -1;
	}

	if (number > 127 || number < -128)
		return E_OUT_OF_RANGE;

	result = number;

	return E_OK;
}

int string_to_number(const char* s, size_t s_len, int16_t& result)
{
	constexpr size_t max_digits_count = 5;

	if (s_len == 0)
		return E_INVALID;

	uint8_t digits[max_digits_count] = { 0 };
	size_t digit_count = 0;

	bool negative = false;

	StringIter iter;
	init_stringiter_forward(iter, s, s_len);

	uint32_t ch;
	int error;
	size_t i = 0;
	while (true)
	{
		error = stringiter_next(iter, ch);
		if (error != E_OK)
		{
			if (error == E_NULL)
				break;

			return error;
		}

		if (ch == '-')
		{
			if (i == 0)
			{
				negative = true;
			}
			else return E_INVALID;
		}
		else if (ch >= '0' && ch <= '9')
		{
			if (digit_count == max_digits_count)
				return E_OUT_OF_RANGE;

			digits[digit_count] = ch;
			digit_count += 1;
		}
		else return E_INVALID;

		i += 1;
	}

	i = 0;
	int32_t number = 0;
	unsigned int exponent = digit_count - 1;
	while (i < digit_count)
	{
		number += pow(10, exponent) * (digits[i] - '0');
		exponent -= 1;
		i += 1;
	}

	if (negative)
	{
		number *= -1;
	}

	if (number > 32767 || number < -32768)
		return E_OUT_OF_RANGE;

	result = number;

	return E_OK;
}

int string_to_number(const char* s, size_t s_len, int32_t& result)
{
	constexpr size_t max_digits_count = 10;

	if (s_len == 0)
		return E_INVALID;

	uint8_t digits[max_digits_count] = { 0 };
	size_t digit_count = 0;

	bool negative = false;

	StringIter iter;
	init_stringiter_forward(iter, s, s_len);

	uint32_t ch;
	int error;
	size_t i = 0;
	while (true)
	{
		error = stringiter_next(iter, ch);
		if (error != E_OK)
		{
			if (error == E_NULL)
				break;

			return error;
		}

		if (ch == '-')
		{
			if (i == 0)
			{
				negative = true;
			}
			else return E_INVALID;
		}
		else if (ch >= '0' && ch <= '9')
		{
			if (digit_count == max_digits_count)
				return E_OUT_OF_RANGE;

			digits[digit_count] = ch;
			digit_count += 1;
		}
		else return E_INVALID;

		i += 1;
	}

	i = 0;
	int64_t number = 0;
	unsigned int exponent = digit_count - 1;
	while (i < digit_count)
	{
		number += pow(10, exponent) * (digits[i] - '0');
		exponent -= 1;
		i += 1;
	}

	if (negative)
	{
		number *= -1;
	}

	if (number > 2147483647 || number < -2147483648)
		return E_OUT_OF_RANGE;

	result = number;

	return E_OK;
}

int string_to_number(const char* s, size_t s_len, int64_t& result)
{
	constexpr size_t max_digits_count = 19;

	if (s_len == 0)
		return E_INVALID;

	uint8_t digits[max_digits_count] = { 0 };
	size_t digit_count = 0;

	bool negative = false;

	StringIter iter;
	init_stringiter_forward(iter, s, s_len);

	uint32_t ch;
	int error;
	size_t i = 0;
	while (true)
	{
		error = stringiter_next(iter, ch);
		if (error != E_OK)
		{
			if (error == E_NULL)
				break;

			return error;
		}

		if (ch == '-')
		{
			if (i == 0)
			{
				negative = true;
			}
			else return E_INVALID;
		}
		else if (ch >= '0' && ch <= '9')
		{
			if (digit_count == max_digits_count)
				return E_OUT_OF_RANGE;

			digits[digit_count] = ch;
			digit_count += 1;
		}
		else return E_INVALID;

		i += 1;
	}

	i = 0;
	uint64_t number = 0;
	uint64_t exponent = digit_count - 1;
	while (i < digit_count)
	{
		number += pow((uint64_t)10, exponent) * (digits[i] - '0');
		exponent -= 1;
		i += 1;
	}

	if (negative)
	{
		if (number * -1 < -9223372036854775808llu)
			return E_OUT_OF_RANGE;

		result = int64_t(number) * (-1);
	}
	else
	{
		if (number > 9223372036854775807)
			return E_OUT_OF_RANGE;

		result = number;
	}

	return E_OK;
}

int string_to_number(const char* s, size_t s_len, uint8_t& result)
{
	constexpr size_t max_digits_count = 3;

	if (s_len == 0)
		return E_INVALID;

	uint8_t digits[max_digits_count] = { 0 };
	size_t digit_count = 0;

	StringIter iter;
	init_stringiter_forward(iter, s, s_len);

	uint32_t ch;
	int error;
	size_t i = 0;
	while (true)
	{
		error = stringiter_next(iter, ch);
		if (error != E_OK)
		{
			if (error == E_NULL)
				break;

			return error;
		}

		if (ch >= '0' && ch <= '9')
		{
			if (digit_count == max_digits_count)
				return E_OUT_OF_RANGE;

			digits[digit_count] = ch;
			digit_count += 1;
		}
		else return E_INVALID;

		i += 1;
	}

	i = 0;
	uint16_t number = 0;
	unsigned int exponent = digit_count - 1;
	while (i < digit_count)
	{
		number += pow(10, exponent) * (digits[i] - '0');
		exponent -= 1;
		i += 1;
	}

	if (number > 255)
		return E_OUT_OF_RANGE;

	result = number;

	return E_OK;
}

int string_to_number(const char* s, size_t s_len, uint16_t& result)
{
	constexpr size_t max_digits_count = 5;

	if (s_len == 0)
		return E_INVALID;

	uint8_t digits[max_digits_count] = { 0 };
	size_t digit_count = 0;

	StringIter iter;
	init_stringiter_forward(iter, s, s_len);

	uint32_t ch;
	int error;
	size_t i = 0;
	while (true)
	{
		error = stringiter_next(iter, ch);
		if (error != E_OK)
		{
			if (error == E_NULL)
				break;

			return error;
		}

		if (ch >= '0' && ch <= '9')
		{
			if (digit_count == max_digits_count)
				return E_OUT_OF_RANGE;

			digits[digit_count] = ch;
			digit_count += 1;
		}
		else return E_INVALID;

		i += 1;
	}

	i = 0;
	uint32_t number = 0;
	unsigned int exponent = digit_count - 1;
	while (i < digit_count)
	{
		number += pow(10, exponent) * (digits[i] - '0');
		exponent -= 1;
		i += 1;
	}

	if (number > 65535)
		return E_OUT_OF_RANGE;

	result = number;

	return E_OK;
}

int string_to_number(const char* s, size_t s_len, uint32_t& result)
{
	constexpr size_t max_digits_count = 10;

	if (s_len == 0)
		return E_INVALID;

	uint8_t digits[max_digits_count] = { 0 };
	size_t digit_count = 0;

	StringIter iter;
	init_stringiter_forward(iter, s, s_len);

	uint32_t ch;
	int error;
	size_t i = 0;
	while (true)
	{
		error = stringiter_next(iter, ch);
		if (error != E_OK)
		{
			if (error == E_NULL)
				break;

			return error;
		}

		if (ch >= '0' && ch <= '9')
		{
			if (digit_count == max_digits_count)
				return E_OUT_OF_RANGE;

			digits[digit_count] = ch;
			digit_count += 1;
		}
		else return E_INVALID;

		i += 1;
	}

	i = 0;
	uint64_t number = 0;
	unsigned int exponent = digit_count - 1;
	while (i < digit_count)
	{
		number += pow(10, exponent) * (digits[i] - '0');
		exponent -= 1;
		i += 1;
	}

	if (number > uint64_t(0xFFFF'FFFF))
		return E_OUT_OF_RANGE;

	result = number;

	return E_OK;
}

int string_to_number(const char* s, size_t s_len, uint64_t& result)
{
	constexpr size_t max_digits_count = 20;

	if (s_len == 0)
		return E_INVALID;

	uint8_t digits[max_digits_count] = { 0 };
	size_t digit_count = 0;

	StringIter iter;
	init_stringiter_forward(iter, s, s_len);

	uint32_t ch;
	int error;
	size_t i = 0;
	while (true)
	{
		error = stringiter_next(iter, ch);
		if (error != E_OK)
		{
			if (error == E_NULL)
				break;

			return error;
		}

		if (ch >= '0' && ch <= '9')
		{
			if (digit_count == max_digits_count)
				return E_OUT_OF_RANGE;

			digits[digit_count] = ch;
			digit_count += 1;
		}
		else return E_INVALID;

		i += 1;
	}

	i = 0;
	uint64_t number = 0;
	uint64_t next_number = 0;
	unsigned int exponent = digit_count - 1;
	while (i < digit_count)
	{
		next_number = number + pow(10, exponent) * (digits[i] - '0');
		if (next_number >= number)
			number = next_number;
		else return E_OUT_OF_RANGE;

		exponent -= 1;
		i += 1;
	}

	result = number;

	return E_OK;
}

int string_to_number(const char* s, size_t s_len, double& result)
{
	char* endptr;
	double value = strtod(s, &endptr);

	if (endptr != s + s_len)
		return E_INVALID;

	result = value;

	return E_OK;
}
