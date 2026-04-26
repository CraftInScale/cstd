#pragma once

#include "../string/String.h"

#include <pthread.h>

#define LOG_DEFAULT_LOG_FORMAT "%date% :: %lvl% : %name% / %tname% :: "
#define LOG_DEFAULT_DATE_FORMAT "Y-M-D h:m:s"
#define LOG_DEFAULT_PASS_LEVEL LL_DEBUG

enum LogLevel
{
	LL_DEBUG,
	LL_INFO,
	LL_WARN,
	LL_ERROR,
	LL_FATAL
};

struct Logger
{
	String name;
	String log_format;// %datetime%, %lvl%, %tname%, %tid%
	LogLevel passthrough_level;
};

bool create_logger(Logger& logger, const char* name, const char* format, LogLevel min_level);

int init_global_logger(Logger& global_logger);
void destroy_global_logger();
// @param name is moved
void set_thread_name(pthread_t id, String& name);
void unset_thread(pthread_t id);

// uses global logger
int logger_log(String& message, LogLevel level = LL_INFO);
// uses global logger
int logger_log(const char* message, LogLevel level = LL_INFO, size_t message_size = 0, size_t message_len = 0);