#pragma once

#include <stddef.h>
#include <stdint.h>

struct Utf8StrCount
{
	size_t size;
	size_t length;
};

int u8strcount(const char* str, Utf8StrCount& result);
int u8strcount(const char* str, Utf8StrCount& result, Utf8StrCount& limit);
int u8validate(const char* str, bool& valid);
bool u8_is_literal(uint32_t ch);