#pragma once

#include <stdint.h>
#include <stddef.h>

#include "StringIter.h"

struct StringCharResult
{
	uint32_t character;
	size_t size;
	StringIter iter;
};

struct StringIndexofResult
{
	bool found;
	StringIter iter;
};

struct String
{
	uint8_t* data;
	size_t capacity;
	size_t size;
	size_t length;
};

int create_string(String& string, size_t capacity = 0);
int create_string(String& string, const char* data, size_t data_size = 0, size_t data_len = 0, size_t capacity = 0);

/*

function: replace_all
          \-> find i-th occurrence

functional functions:

char_at(s, i, StringCharResult& result)
starts_with(s, str, str_len, bool& matches)
starts_with(s, s1, bool& matches)
ends_with(s, str, str_len, bool& matches)
ends_with(s, s1, bool& matches)
index_of(s, str, str_len, StringIndexOfResult& result)
replace_all(s, match, replacement)

*/

void string_reset(String& string);
int string_concat(String& string, const char* str, size_t str_size = 0, size_t str_len = 0);
int string_push(String& string, char ch);
int string_clone(String& string, String& cloned, bool keep_capacity = false);
inline const char* string_c_str(String& string)
{
	return (const char*)string.data;
}

int string_char_at(String& string, size_t index, StringCharResult& result);
int string_starts_with(String& string, const char* str, size_t str_len, bool& matches);
//int string_ends_with(String& string, const char* str, size_t str_len, bool& matches);
/*
* @param start_from next char is first char to compare with
*/
int string_indexof(String& string, const char* match, size_t match_len, StringIndexofResult& result, StringIter* start_from = nullptr, size_t nth = 1);
/*
* @param match_len 0 = auto count
* @param replacement_len 0 = auto count
*/
int string_replace_all(String& string, const char* match, size_t match_len, const char* replacement, size_t replacement_len);

bool string_equals(String& string, const char* other);
// other_size cannot be 0
bool string_equals(String& string, const char* other, size_t other_size);

int string_reserve(String& string, size_t extra_capacity);
// "match" - allocate more capacity so it matches or is higher than size + extra_size
int string_reserve_match(String& string, size_t extra_size);

void destroy_string(String& string);