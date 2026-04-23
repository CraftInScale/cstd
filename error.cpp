#include "error.h"

#include "string/StringConversion.h"

#include <string.h>
#include <errno.h>
#include <stdio.h>

Vec cstd_error_providers;

void sprint_error(String& append_to, const char* message, int error, LogLevel level)
{
	if (error == E_OK)
		return;

	String& s = append_to;
	string_concat(s, message);
	string_concat(s, ": ", 2, 2);

	sprint_error(append_to, error, level);
}

void sprint_error(String& append_to, String& message, int error, LogLevel level)
{
	if (error == E_OK)
		return;

	String& s = append_to;
	string_concat(s, string_c_str(message), message.size-1, message.length);
	string_concat(s, ": ", 2, 2);

	sprint_error(append_to, error, level);
}

void sprint_error(String& append_to, int error, LogLevel level)
{
	String& s = append_to;

	switch (error)
	{
	case E_ERRNO: 
		string_concat(s, "Linux error: ");
		string_concat(s, strerror(errno)); 
		break;
	case E_NULL: string_concat(s, "Data is null pointer"); break;
	case E_UTF8_INVALID: string_concat(s, "String has non-UTF8 bytes"); break;
	case E_UTF8_INVTERM: string_concat(s, "String terminated with not finished UTF8 last char"); break;
	case E_SIZE_OVERFLOW: string_concat(s, "Data structure heap size variable overflow"); break;
	case E_OUT_OF_RANGE: string_concat(s, "Index out of range"); break;
	case E_ACCESS_ONLY: string_concat(s, "Data structure is available for reading only"); break;
	case E_DS_FULL: string_concat(s, "Data structure is full"); break;
	case E_DS_EMPTY: string_concat(s, "Data structure is empty"); break;
	case E_UNSUPPORTED: string_concat(s, "Operation not supported"); break;
	case E_PARSE_FAIL: string_concat(s, "Failed to parse data"); break;
	case E_FILE_DESCRIPTOR_NOT_OPEN: string_concat(s, "File descriptor is not open"); break;
	case E_EOF: string_concat(s, "End of file"); break;
	case E_FILE_NOT_FOUND: string_concat(s, "File not found"); break;
	case E_INVALID: string_concat(s, "Invalid data or operation"); break;
	case E_EXPECT_FAILED: string_concat(s, "Expect failed"); break;
	case E_CONFIG_ERROR: string_concat(s, "Config error"); break;
	case E_NO_THREADS: string_concat(s, "No threads available"); break;
	case E_THREADS_FULL: string_concat(s, "Threads are full"); break;
	case E_NO_DATA: string_concat(s, "Buffer has no more data"); break;
	case E_OUT_OF_MEMORY: string_concat(s, "Out of memory"); break;
	case E_UNDERFLOW: string_concat(s, "Underflow"); break;
	case E_NOT_FOUND: string_concat(s, "Not found"); break;
	default:
		bool handled = false;

		size_t i = 0;
		int err;
		ErrorProviderFn* fn_ptr;
		while (i < cstd_error_providers.size)
		{
			fn_ptr = (ErrorProviderFn*) vec_get(cstd_error_providers, i, err);
			if ((*fn_ptr)(s, error))
			{
				handled = true;
				break;
			}

			i += 1;
		}

		if (!handled)
		{
			string_concat(s, "Unknown: ");
			number_to_string(error, s);
		}
		break;
	}
}

void print_error(const char* message, int error, LogLevel level)
{
	if (error == E_OK)
		return;

	String s;
	create_string(s, message, 0, 0, 1024);
	string_concat(s, ": ", 2, 2);

	sprint_error(s, error, level);

	//printf("%s: %s\n", message, string_c_str(s));
	logger_log(s, level);

	destroy_string(s);
}

void print_error(String& message, int error, LogLevel level)
{
	/*if (error != E_OK)
		print_error((char*)message.data, error, level);*/

	if (error == E_OK)
		return;

	String s;
	create_string(s, string_c_str(message), message.size-1, message.length, 1024);
	string_concat(s, ": ", 2, 2);

	sprint_error(s, error, level);

	//printf("%s: %s\n", message, string_c_str(s));
	logger_log(s, level);

	destroy_string(s);
}

int add_custom_error_provider(ErrorProviderFn provider)
{
	vec_push_back(cstd_error_providers, &provider);

	return E_OK;
}
