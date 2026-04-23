#include "Logger.h"

#include "../time/Timestamp.h"
#include "../time/DateTime.h"
#include "../string/Utf8Util.h"
#include "../string/StringConversion.h"
#include "../error.h"
#include "../collections/HashMap.h"
#include "../thread/Mutex.h"

#include <locale.h>
#include <stdio.h>
#include <unistd.h>

struct AppThreads
{
	Mutex mutex;
	HashMap names;// HashMap<pthread_t, String>
};

static __attribute__((aligned(64))) Logger global_logger;
static AppThreads threads;

bool create_logger(Logger& logger, const char* name, const char* format, LogLevel min_level)
{
	int error;
	error = create_string(logger.name, name);
	if (error != E_OK)
	{
		printf("creating logger name failed, probably out of memory, error code: %i", error);
		return false;
	}

	error = create_string(logger.log_format, format);
	if (error != E_OK)
	{
		printf("creating logger log_format failed, probably out of memory, error code: %i", error);
		destroy_string(logger.name);
		return false;
	}

	logger.passthrough_level = min_level;

	return true;
}

int init_global_logger(Logger& logger)
{
	setlocale(LC_ALL, "");

	global_logger = logger;
	
	int error;

	if (global_logger.log_format.capacity < 1024)
		E_RET_IF_ERROR_USE(string_reserve(global_logger.log_format, 1024 - global_logger.log_format.capacity));

	threads = AppThreads{};
	create_mutex(threads.mutex);
	error = create_hashmap(threads.names, sizeof(pthread_t), sizeof(String), hfunc_scalar);
	if (error != E_OK)
	{
		printf("failed to create global logger threads mapping, error code: %i", error);
		return error;
	}

	return E_OK;
}

void destroy_global_logger()
{
	destroy_string(global_logger.name);
	destroy_string(global_logger.log_format);

	mutex_lock(threads.mutex);
	destroy_hashmap(threads.names);
	mutex_unlock(threads.mutex);
	destroy_mutex(threads.mutex);
}

void set_thread_name(pthread_t id, String& name)
{
	mutex_lock(threads.mutex);
	hashmap_insert(threads.names, &id, &name);
	mutex_unlock(threads.mutex);
}

void unset_thread(pthread_t id)
{
	mutex_lock(threads.mutex);

	HashMapResult result;
	hashmap_get(threads.names, &id, result);
	if (result.present)
	{
		destroy_string(*(String*)result.value);
	}

	hashmap_remove(threads.names, &id);
	mutex_unlock(threads.mutex);
}

int logger_log(String& message, LogLevel level)
{
	return logger_log(string_c_str(message), level, message.size, message.length);
}

int logger_log(const char* message, LogLevel level, size_t message_size, size_t message_len)
{
	if (message_size == 0 || message_len == 0)
	{
		Utf8StrCount count;
		Utf8StrCount limit{ message_size, message_len };
		int error = u8strcount(message, count);
		if (error != E_OK)
		{
			printf("logger message size and length count error: %s\n", message);
			return error;
		}

		message_size = count.size;
		message_len = count.length;
	}

	Logger* logger = &global_logger;

	int error;

	String format;
	error = string_clone(logger->log_format, format, true);
	if (error != E_OK)
		return error;

	String dt_format;
	error = create_string(dt_format, "Y-M-D H:m:s", 11, 11, 19);
	if (error != E_OK)
	{
		destroy_string(format);
		return error;
	}
	
	Timestamp ts;
	DateTime dt;
	init_timestamp(ts);
	init_datetime(dt, ts);
	error = datetime_format(dt, dt_format);
	if (error != E_OK)
	{
		destroy_string(format);
		destroy_string(dt_format);
		return error;
	}

	error = string_replace_all(format, "%datetime%", 10, string_c_str(dt_format), dt_format.length);
	if (error != E_OK)
	{
		destroy_string(format);
		destroy_string(dt_format);
		return error;
	}

	const char* level_str;
	size_t level_size;
	switch (level)
	{
	case LL_DEBUG: level_str = "DEBUG"; level_size = 5; break;
	case LL_INFO: level_str = "INFO"; level_size = 4; break;
	case LL_WARN: level_str = "WARN"; level_size = 4; break;
	case LL_ERROR: level_str = "ERROR"; level_size = 5; break;
	case LL_FATAL: level_str = "FATAL"; level_size = 5; break;
	}

	error = string_replace_all(format, "%lvl%", 5, level_str, level_size);
	if (error != E_OK)
	{
		destroy_string(format);
		destroy_string(dt_format);
		return error;
	}

	error = string_replace_all(format, "%name%", 6, string_c_str(logger->name), logger->name.length);
	if (error != E_OK)
	{
		destroy_string(format);
		destroy_string(dt_format);
		return error;
	}

	pthread_t tid = gettid();
	HashMapResult result;
	mutex_lock(threads.mutex);
	hashmap_get(threads.names, &tid, result);
	mutex_unlock(threads.mutex);
	if (result.present)
	{
		String* thread_name = (String*)result.value;
		error = string_replace_all(format, "%tname%", 7, string_c_str(*thread_name), thread_name->length);
		if (error != E_OK)
		{
			destroy_string(format);
			destroy_string(dt_format);
			return error;
		}
	}
	else
	{
		error = string_replace_all(format, "%tname%", 7, "?", 1);
		if (error != E_OK)
		{
			destroy_string(format);
			destroy_string(dt_format);
			return error;
		}
	}

	String s_tid;
	error = create_string(s_tid);
	if (error != E_OK)
	{
		destroy_string(format);
		destroy_string(dt_format);
		return error;
	}

	error = number_to_string(tid, s_tid);
	if (error != E_OK)
	{
		destroy_string(format);
		destroy_string(dt_format);
		destroy_string(s_tid);
		return error;
	}

	error = string_replace_all(format, "%tid%", 5, string_c_str(s_tid), s_tid.length);
	destroy_string(s_tid);
	if (error != E_OK)
	{
		destroy_string(format);
		destroy_string(dt_format);
		return error;
	}

	error = string_concat(format, message, message_size, message_len);
	if (error != E_OK)
	{
		destroy_string(format);
		destroy_string(dt_format);
		return error;
	}

	printf("%s\n", string_c_str(format));

	destroy_string(format);
	destroy_string(dt_format);

	return E_OK;
}
