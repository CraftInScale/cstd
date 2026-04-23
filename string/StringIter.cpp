#include "StringIter.h"

#include "String.h"
#include "../error.h"
#include "Utf8Util.h"

void init_stringiter_forward(StringIter& iter, String* string)
{
	iter.data = string->data;
	iter.size_off = 0;
	iter.len_off = 0;
	iter.max_len_off = string->length;
	iter.forward = true;
	iter.reached_end = true;
}

int init_stringiter_forward(StringIter& iter, const char* str, size_t str_len)
{
	if (str_len == 0)
	{
		Utf8StrCount count;
		int error = u8strcount(str, count);
		if (error != E_OK)
			return error;

		str_len = count.length;
	}

	iter.data = (uint8_t*)str;
	iter.size_off = 0;
	iter.len_off = 0;
	iter.max_len_off = str_len;
	iter.forward = true;
	iter.reached_end = true;

	return E_OK;
}

void init_stringiter_backward(StringIter& iter, String* string)
{
	iter.data = (uint8_t*)string->data;
	iter.size_off = string->size - 1;
	iter.len_off = string->length;
	
	iter.forward = false;
	iter.init_size_off = iter.size_off;
	iter.init_len_off = iter.len_off;
	iter.max_len_off = string->length;

	iter.reached_end = false;
}

void init_stringiter_backward(StringIter& iter, const char* str, size_t str_size, size_t str_len)
{
	iter.data = (uint8_t*)str;
	iter.size_off = str_size;
	iter.len_off = str_len;

	iter.forward = false;
	iter.init_size_off = iter.size_off;
	iter.init_len_off = iter.len_off;
	iter.max_len_off = str_len;

	iter.reached_end = false;
}

int stringiter_next(StringIter& iter, uint32_t& result_char)
{
	if (iter.len_off >= iter.max_len_off)
	{
		result_char = 0;
		return E_NULL;
	}

	result_char = 0;

	uint8_t* ch = iter.data + iter.size_off;
	
	uint8_t ones_count = 0;
	uint8_t ch_mask = 0x80;

	while (ones_count < 6)
	{
		if ((*ch & ch_mask) > 0)
		{
			ones_count += 1;
			ch_mask >>= 1;
		}
		else break;
	}

	if (ones_count > 0)
	{
		if (ones_count < 2 || ones_count > 4)
		{
			return E_UTF8_INVALID;
		}

		uint8_t next_bytes_left = ones_count - 1;

		// gen mask based on number of 1s in first byte
		// 4 -> shift by 3
		// 2 -> shift by 5
		ch_mask = (1 << (7 - ones_count)) - 1;
		result_char = (uint32_t(*ch) & ch_mask) << (next_bytes_left * 6);

		ch_mask = 0xC0;
		while (next_bytes_left > 0)
		{
			ch += 1;
			if ((*ch & ch_mask) != 0x80)
				return E_UTF8_INVALID;

			next_bytes_left -= 1;
			result_char = (uint32_t(*ch) & 0x7F) << (next_bytes_left * 6);
		}

		iter.len_off += 1;
		iter.size_off += ones_count;
	}
	else
	{
		result_char = *ch;

		if (result_char == '\0')
			return E_NULL;

		iter.len_off += 1;
		iter.size_off += 1;
	}

	iter.reached_end = false;

	return E_OK;
}

int stringiter_prev(StringIter& iter, uint32_t& result_char)
{
	result_char = 0;

	//if (iter.size_off == 0)
	//	return E_NULL;
	if (iter.reached_end)
		return E_NULL;

	uint8_t* ch = iter.data + iter.size_off - 1;

	if ((*ch & 0x80) == 0)
	{
		iter.size_off -= 1;
		iter.len_off -= 1;

		iter.reached_end = iter.len_off == 0;

		result_char = *ch;

		return E_OK;
	}
	else
	{
		// count number of subsequent bytes in UTF-8
		size_t subseq_bytes = 0;
		while ((*ch & 0xC0) == 0x80)
		{
			subseq_bytes += 1;
			ch -= 1;

			if (ch < iter.data)
				return E_UTF8_INVALID;
		}

		if (subseq_bytes > 3)
			return E_UTF8_INVALID;

		uint8_t chbits_mask = (1 << (6 - subseq_bytes)) - 1;
		uint8_t utfbits_mask = ~chbits_mask;

		if ((*ch & utfbits_mask) != (utfbits_mask << 1))
		{
			return E_UTF8_INVALID;
		}

		iter.size_off -= subseq_bytes + 1;
		iter.len_off -= 1;

		iter.reached_end = iter.len_off == 0;

		result_char = uint32_t(*ch & chbits_mask) << (subseq_bytes * 6);
		while (subseq_bytes > 0)
		{
			ch += 1;
			subseq_bytes -= 1;

			result_char |= uint32_t(*ch & 0b0011'1111) << (subseq_bytes * 6);
		}

		return E_OK;
	}
}

void stringiter_reset(StringIter& iter)
{
	if (iter.forward)
	{
		iter.size_off = 0;
		iter.len_off = 0;
		iter.reached_end = true;
	}
	else
	{
		iter.size_off = iter.init_size_off;
		iter.len_off = iter.init_len_off;
		iter.reached_end = false;
	}
}
