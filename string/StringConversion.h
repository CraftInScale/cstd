#pragma once

#include "String.h"

int number_to_string(int n, String& append_to);
int number_to_string(long n, String& append_to);
int number_to_string(size_t n, String& append_to);
int number_to_string(uint8_t n, String& append_to);
int number_to_string(uint16_t n, String& append_to);
int number_to_hex(uint64_t n, String& append_to);

int string_to_bool(const char* s, size_t s_len, bool& result);

int string_to_number(const char* s, size_t s_len, int8_t& result);
int string_to_number(const char* s, size_t s_len, int16_t& result);
int string_to_number(const char* s, size_t s_len, int32_t& result);
int string_to_number(const char* s, size_t s_len, int64_t& result);

int string_to_number(const char* s, size_t s_len, uint8_t& result);
int string_to_number(const char* s, size_t s_len, uint16_t& result);
int string_to_number(const char* s, size_t s_len, uint32_t& result);
int string_to_number(const char* s, size_t s_len, uint64_t& result);

// todo test it
int string_to_number(const char* s, size_t s_len, double& result);