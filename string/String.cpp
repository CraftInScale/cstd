#include "String.h"

#include "../error.h"
#include "Utf8Util.h"
#include "../collections/Vec.h"
#include "../mem.h"

#include <stdlib.h>
#include <string.h>

static int _string_grow(String& string, size_t by_capacity)
{
	size_t new_size = string.size + by_capacity;

	if (new_size < string.size)
		return E_SIZE_OVERFLOW;

	if (new_size > string.capacity)
	{
		void* new_block = realloc(string.data, new_size);
		if (new_block == nullptr)
		{
			return E_ERRNO;
		}

		string.data = (uint8_t*)new_block;
		string.capacity = new_size;
	}

	return E_OK;
}

int create_string(String& string, size_t capacity)
{
	string.data = nullptr;
	string.length = 0;
	string.size = 0;
	string.capacity = 0;

	size_t cap = capacity != 0 ? capacity : 1;

	void* data = malloc(cap);
	if (data == nullptr)
	{
		return E_ERRNO;
	}

	string.data = (uint8_t*)data;
	string.size = 1;
	string.capacity = cap;

	*string.data = 0;// null termination

	return E_OK;
}

int create_string(String& string, const char* str, size_t str_size, size_t str_len, size_t capacity)
{
	string.data = nullptr;
	string.size = 0;
	string.length = 0;
	string.capacity = 0;

	if (str_size == 0)
	{
		Utf8StrCount result;
		Utf8StrCount limit{ 0,str_len };
		int error = u8strcount(str, result, limit);
		if (error != E_OK)
			return error;

		str_size = result.size;
		str_len = result.length;
	}
	else if (str_len == 0)
	{
		Utf8StrCount result;
		Utf8StrCount limit{ str_size,0 };
		int error = u8strcount(str, result, limit);
		if (error != E_OK)
			return error;

		str_len = result.length;
	}

	if (capacity < str_size)
	{
		capacity = str_size + 1;
	}
	else if (capacity == str_size)
	{
		capacity += 1;// null-termination
	}

	void* data = malloc(capacity);
	if (data == nullptr)
	{
		return E_ERRNO;
	}

	string.data = (uint8_t*)data;
	string.capacity = capacity;
	string.size = str_size + 1;
	string.length = str_len;

	memcpy(string.data, str, str_size);

	string.data[str_size] = 0;// null-termination

	return E_OK;
}

int string_concat(String& string, const char* str, size_t str_size, size_t str_len)
{
	if (str_size == 0)
	{
		Utf8StrCount count;
		int error = u8strcount(str, count);
		if (error != E_OK)
			return error;

		str_size = count.size;
		str_len = count.length;
	}
	else if (str_len == 0)
	{
		Utf8StrCount count;
		Utf8StrCount limit{ str_size,0 };
		int error = u8strcount(str, count, limit);
		if (error != E_OK)
			return error;

		str_len = count.length;
	}

	int error = _string_grow(string, str_size);
	if (error != E_OK)
		return error;

	memcpy(string.data + string.size - 1, str, str_size);

	string.size += str_size;
	string.length += str_len;
	*(string.data + (string.size - 1)) = uint8_t(0);

	return E_OK;
}

int string_push(String& string, char ch)
{
	if (uint8_t(ch) > 127)
		return E_UTF8_INVALID;

	int error = _string_grow(string, 1);
	if (error != E_OK)
		return error;

	string.size += 1;
	string.length += 1;
	*(string.data + (string.size - 1)) = uint8_t(0);
	*(string.data + (string.size - 2)) = ch;

	return E_OK;
}

void string_reset(String& string)
{
	string.size = 1;
	string.length = 0;
	string.data[0] = 0;
}

int string_clone(String& string, String& cloned, bool keep_capacity)
{
	cloned.data = nullptr;
	cloned.capacity = 0;
	cloned.size = 0;
	cloned.length = 0;

	size_t cap = keep_capacity ? string.capacity : string.size;

	void* data = malloc(cap);
	if (data == nullptr)
		return E_ERRNO;

	cloned.data = (uint8_t*)data;
	cloned.capacity = cap;
	cloned.size = string.size;
	cloned.length = string.length;

	memcpy(data, string.data, string.size);

	return E_OK;
}

int string_char_at(String& string, size_t index, StringCharResult& result)
{
	result.character = 0;
	result.size = 0;

	if (index >= string.size)
		return E_OUT_OF_RANGE;

	StringIter& iter = result.iter;
	init_stringiter_forward(iter, &string);

	size_t i = 0;
	int error;
	size_t prev_size_off = 0;
	while (i < index)
	{
		prev_size_off = iter.size_off;
		error = stringiter_next(iter, result.character);
		if (error != E_OK)
			return error;

		i += 1;
	}

	result.size = iter.size_off - prev_size_off;

	return E_OK;
}

int string_starts_with(String& string, const char* str, size_t str_len, bool& matches)
{
	matches = false;

	if (str_len > string.length)
		return E_OK;

	StringIter iter;
	init_stringiter_forward(iter, &string);

	StringIter iter_match;
	init_stringiter_forward(iter_match, str);

	uint32_t ch1, ch2;
	int err1, err2;
	size_t i = 0;
	while (i < str_len)
	{
		err1 = stringiter_next(iter, ch1);
		err2 = stringiter_next(iter_match, ch2);

		if (err1 == E_NULL || err2 == E_NULL)
		{
			matches = err2 == E_NULL;
			return E_OK;
		}
		else if (err2 != E_OK)
		{
			return err2;
		}
		else if (err1 != E_OK)
		{
			return err1;
		}

		if (ch2 != ch1)
		{
			matches = false;
			return E_OK;
		}

		i += 1;
	}

	matches = true;

	return E_OK;
}

int string_indexof(String& string, const char* str, size_t str_len, StringIndexofResult& result, StringIter* start_from, size_t nth)
{
	result.found = false;

	StringIter primary_iter;
	if (start_from == nullptr)
	{
		init_stringiter_forward(primary_iter, &string);
	}
	else 
	{
		primary_iter = *start_from;

		/*if (primary_iter.len_off > 0)
		{
			uint32_t ch;
			int error = stringiter_prev(primary_iter, ch);
			if (error != E_OK)
				return error;
		}*/
	}

	StringIter match_iter;
	init_stringiter_forward(match_iter, str, str_len);

	uint32_t ch1, match_ch;
	int error;
	size_t reset_size_off = 0;
	size_t reset_len_off = 0;
	while (true)
	{
		error = stringiter_next(primary_iter, ch1);
		if (error != E_OK)
		{
			if (error == E_NULL)
				break;

			return error;
		}

		error = stringiter_next(match_iter, match_ch);
		if (error != E_OK)
		{
			if (error == E_NULL)
			{
				nth -= 1;
				if (nth > 0)
				{
					stringiter_reset(match_iter);
					error = stringiter_next(match_iter, match_ch);
					if (error != E_OK)
						return error;

					error = stringiter_next(primary_iter, ch1);
					if (error != E_OK)
					{
						if (error == E_NULL)
							break;

						return error;
					}
				}
				else
				{
					result.found = true;

					StringIter& iter = result.iter;

					iter.data = primary_iter.data;
					iter.forward = true;

					iter.size_off = reset_size_off;
					iter.len_off = reset_len_off;
					iter.max_len_off = primary_iter.max_len_off;

					iter.init_size_off = string.size - 1;
					iter.init_len_off = string.length;
					return E_OK;
				}
			}
			else return error;
		}

		if (ch1 != match_ch)
		{
			stringiter_reset(match_iter);
			reset_size_off = primary_iter.size_off;
			reset_len_off = primary_iter.len_off;
		}
	}

	error = stringiter_next(match_iter, match_ch);
	if (error != E_OK)
	{
		if (error == E_NULL)
		{
			nth -= 1;
			if (nth == 0)
			{
				result.found = true;

				StringIter& iter = result.iter;

				iter.data = primary_iter.data;
				iter.forward = true;

				iter.size_off = reset_size_off;
				iter.len_off = reset_len_off;
				iter.max_len_off = primary_iter.max_len_off;

				iter.init_size_off = string.size - 1;
				iter.init_len_off = string.length;
				return E_OK;
			}
		}
		else return error;
	}

	return E_OK;
}

int string_replace_all(String& string, const char* match, size_t match_len, const char* replacement, size_t replacement_len)
{
	if (string.length == 0)
		return E_OK;

	int error;
	size_t match_size = 0;

	if (match_len == 0)
	{
		Utf8StrCount count;
		error = u8strcount(match, count);
		if (error != E_OK)
			return error;

		match_len = count.length;
		match_size = count.size;
	}

	if (match_len == 0)
		return E_OK;

	if (match_len > string.length)
		return E_OK;

	if (match_size == 0)
	{
		Utf8StrCount count;
		Utf8StrCount limit{ 0, match_len };
		error = u8strcount(match, count, limit);
		if (error != E_OK)
			return error;

		match_size = count.size;
	}

	size_t replacement_size;

	Utf8StrCount count;
	Utf8StrCount limit{ 0, replacement_len };
	error = u8strcount(replacement, count, limit);
	if (error != E_OK)
		return error;

	replacement_size = count.size;

	if (replacement_len == 0)
	{
		replacement_len = count.length;
	}

	if (replacement_len == 0)
		return E_OK;

	// replacing from end

	StringIter s_iter;
	init_stringiter_forward(s_iter, &string);

	StringIter match_iter;
	init_stringiter_forward(match_iter, match, match_len);// match_size, match_len

	struct Range
	{
		size_t from;
		size_t to;// inclusive
	};

	Vec replacements;
	error = create_vec(replacements, sizeof(Range));
	if (error != E_OK)
		return error;

	int err;
	uint32_t s_ch = 1;
	uint32_t match_ch;
	
	/*err = stringiter_next(match_iter, match_ch);
	if (err != E_OK && err != E_NULL)
		return err;*/

	Range range;
	range.from = 0;
	range.to = 0;

	/*
	
	for ch in s:
	  ch2 = next from match
	  if ch2 == NULL
	  {
	    // match -> replace


		ch2 = first from match
	  }

	  if(ch2 != ch)
	  {
	    match iter reset
	  }
	
	*/

	size_t prev_off;
	while (s_ch != 0)
	{
		prev_off = s_iter.size_off;
		err = stringiter_next(s_iter, s_ch);
		if (err != E_OK)
		{
			if (err == E_NULL)
			{
				break;
			}

			destroy_vec(replacements);
			return err;
		}

		err = stringiter_next(match_iter, match_ch);
		if (err != E_OK)
		{
			if (err == E_NULL)
			{
				// match
				range.to = prev_off;
				vec_push_back(replacements, &range);

				// and reset
				stringiter_reset(match_iter);

				range.from = s_iter.size_off;
				err = stringiter_next(match_iter, match_ch);
				if (err != E_OK)
				{
					destroy_vec(replacements);
					return err;
				}
			}
			else
			{
				destroy_vec(replacements);
				return err;
			}
		}

		if (s_ch != match_ch)
		{
			stringiter_reset(match_iter);
			range.from = s_iter.size_off;
		}
	}

	err = stringiter_next(match_iter, match_ch);
	if (err == E_NULL)
	{
		// match
		range.to = s_iter.size_off;
		vec_push_back(replacements, &range);
	}

	/*while (s_ch != 0)
	{
		err = stringiter_next(s_iter, s_ch);
		if (err != E_OK && err != E_NULL)
			return err;

		if (s_ch != match_ch)
		{
			range.from = s_iter.size_off;
			stringiter_reset(match_iter);
		}

		range.to = s_iter.size_off;
		err = stringiter_next(match_iter, match_ch);
		if (err != E_OK && err != E_NULL)
			return err;

		if (err == E_NULL)
		{
			// match
			vec_push_back(replacements, &range);

			// reset to first char
			range.from = s_iter.size_off;
			stringiter_reset(match_iter);

			err = stringiter_next(match_iter, match_ch);
			if (err != E_OK && err != E_NULL)
				return err;
		}
	}*/

	size_t i = replacements.size;
	if (replacement_size > match_size)
	{
		_string_grow(string, (replacement_size - match_size)* replacements.size);

		size_t off = replacement_size - match_size;
		while (i > 0)
		{
			i -= 1;

			Range* range = (Range*)vec_get(replacements, i, err);

			memcpybackward(string.data + range->to + off, string.data + range->to, string.size - range->to);
			memcpy(string.data + range->from, replacement, replacement_size);
			string.size += off;
			string.length += replacement_len - match_len;
		}
	}
	else
	{
		// match_size >= replacement_size
		/*
		
		range represents match

		// first shift upper part
		memcpy(string.data + range.from + replacement_size, string.data + range.to, string.size - range.to)

		// then copy
		memcpy(string.data + range.from, replacement, replacement_size)
		
		*/

		size_t off = match_size - replacement_size;
		while (i > 0)
		{
			i -= 1;

			Range* range = (Range*)vec_get(replacements, i, err);

			// range can be smaller than match, need to shift left the part of string on right
			//memcpy(string.data + range->to, string.data + range->to + off, string.size - range->to - off);
			memcpy(string.data + range->from + replacement_size, string.data + range->to, string.size - range->to);
			memcpy(string.data + range->from, replacement, replacement_size);
			string.size -= off;
			string.length -= match_len - replacement_len;
		}
	}

	destroy_vec(replacements);

	return E_OK;
}

bool string_equals(String& string, const char* other)
{
	return 0 == strcmp(string_c_str(string), other);
}

bool string_equals(String& string, const char* other, size_t other_size)
{
	if (string.size - 1 != other_size)
		return false;

	return 0 == strncmp((char*)string.data, other, other_size);
}

int string_reserve(String& string, size_t extra_capacity)
{
	size_t new_size = string.capacity + extra_capacity;

	if (new_size < string.capacity)
		return E_SIZE_OVERFLOW;

	void* new_block = realloc(string.data, new_size);
	if (new_block == nullptr)
	{
		return E_ERRNO;
	}

	string.data = (uint8_t*)new_block;
	string.capacity = new_size;

	return E_OK;
}

int string_reserve_match(String& string, size_t extra_size)
{
	size_t new_size = string.size + extra_size;

	if (new_size < string.size)
		return E_SIZE_OVERFLOW;

	if (new_size >= string.capacity)
		return E_OK;

	void* new_block = realloc(string.data, new_size);
	if (new_block == nullptr)
	{
		return E_ERRNO;
	}

	string.data = (uint8_t*)new_block;
	string.capacity = new_size;

	return E_OK;
}

void destroy_string(String& string)
{
	if (string.data != nullptr)
	{
		free(string.data);
		string.data = nullptr;
	}
}
