#pragma once

#include <stdint.h>
#include <stddef.h>

struct String;

struct StringIter
{
	uint8_t* data;
	size_t size_off;
	size_t len_off;
	
	bool forward;
	size_t init_size_off;
	size_t init_len_off;
	size_t max_len_off;
	bool reached_end;
};

void init_stringiter_forward(StringIter& iter, String* string);
int init_stringiter_forward(StringIter& iter, const char* str, size_t str_len = 0);
void init_stringiter_backward(StringIter& iter, String* string);
void init_stringiter_backward(StringIter& iter, const char* str, size_t str_size, size_t str_len);
int stringiter_next(StringIter& iter, uint32_t& result_char);
int stringiter_prev(StringIter& iter, uint32_t& result_char);

void stringiter_reset(StringIter& iter);