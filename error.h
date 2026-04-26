#pragma once

#include "log/Logger.h"
#include "string/String.h"
#include "collections/Vec.h"

#include <errno.h>

extern Vec cstd_error_providers;

#define E_OK 0
#define E_ERRNO 1
#define E_NULL 2
#define E_UTF8_INVALID 3
#define E_UTF8_INVTERM 4 // not terminated properly
#define E_SIZE_OVERFLOW 5
#define E_OUT_OF_RANGE 6
#define E_ACCESS_ONLY 8
#define E_DS_FULL 9
#define E_DS_EMPTY 10
#define E_UNSUPPORTED 11
#define E_PARSE_FAIL 12
#define E_FILE_DESCRIPTOR_NOT_OPEN 14
#define E_EOF 15
#define E_FILE_NOT_FOUND 16
#define E_INVALID 18
#define E_EXPECT_FAILED 20
#define E_CONFIG_ERROR 21
#define E_NO_THREADS 22
#define E_THREADS_FULL 23
#define E_NO_DATA 24
#define E_OUT_OF_MEMORY 25
#define E_UNDERFLOW 26
#define E_NOT_FOUND 27

// USE - uses previously declared "int error;"
#define E_RET_IF_ERROR_USE(expression)\
  error = expression;\
  if(error != E_OK)\
      return error;

#define E_RET_IF_ERROR_USE2(expression, on_error)\
  error = expression;\
  if(error != E_OK)\
  {\
      on_error;\
      return error;\
  }

// should return true if error matches one of handled errors
typedef bool(*ErrorProviderFn)(String&, int);

void sprint_error(String& append_to, const char* message, int error, LogLevel level = LL_WARN);
void sprint_error(String& append_to, String& message, int error, LogLevel level = LL_WARN);
void sprint_error(String& append_to, int error, LogLevel level = LL_WARN);
void print_error(const char* message, int error, LogLevel level = LL_WARN);
void print_error(String& message, int error, LogLevel level = LL_WARN);
int add_custom_error_provider(ErrorProviderFn provider);