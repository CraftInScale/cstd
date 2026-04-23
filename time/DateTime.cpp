#include "DateTime.h"

#include "../error.h"
#include "../string/StringConversion.h"

void init_datetime(DateTime& date, Timestamp& timestamp)
{
	constexpr time_t year_seconds = 365 * 24 * 3600;
	constexpr time_t leapyear_seconds = 366 * 24 * 3600;

	date.timezone_sign = 1;
	date.timezone_hour = 0;
	date.timezone_minute = 0;

	int year = 1970;
	time_t seconds = timestamp.seconds;
	while (true)
	{
		bool leap_year = is_leap_year(year);
		time_t subtract_seconds = !leap_year ? year_seconds : leapyear_seconds;

		if (seconds >= subtract_seconds)
		{
			year += 1;
			seconds -= subtract_seconds;
		}
		else
		{
			break;
		}
	}

	date.year = year;

	const int month_days[2][12] = {
		{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
		{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
	};

	bool leap_year = is_leap_year(year);

	int month = 1;
	time_t month_seconds = month_days[leap_year][0] * 24 * 3600;
	time_t prev_month_seconds = 0;
	while (true)
	{
		if (seconds < month_seconds)
		{
			break;
		}
		else
		{
			prev_month_seconds = month_seconds;
			month_seconds += month_days[leap_year][month] * 24 * 3600;
			month += 1;
		}
	}

	date.month = month;

	/*int day = 1;
	time_t day_seconds = prev_month_seconds + 24 * 3600;
	while (seconds > day_seconds)
	{
		day_seconds += 24 * 3600;
		day += 1;
	}*/
	seconds -= prev_month_seconds;
	int day = 1 + seconds / (24 * 3600);

	date.day = day;

	seconds -= (day-1) * 24 * 3600;
	date.hour = seconds / 3600;

	seconds -= date.hour * 3600;
	date.minute = seconds / 60;

	seconds -= date.minute * 60;
	date.second = seconds;

	date.nanos = timestamp.nanos;
}

int datetime_format(DateTime& date, String& format_and_result)
{
	String s;
	create_string(s, 4);

#define REPLACE(value, match)\
	string_reset(s);\
	if(value < 10)\
	{\
		E_RET_IF_ERROR_USE2(string_concat(s, "0", 1, 1), destroy_string(s));\
	}\
	\
	E_RET_IF_ERROR_USE2(number_to_string(value, s), destroy_string(s));\
	E_RET_IF_ERROR_USE2(string_replace_all(format_and_result, match, 1, string_c_str(s), s.length), destroy_string(s));

	int error;

	REPLACE(date.year, "Y");
	REPLACE(date.month, "M");
	REPLACE(date.day, "D");
	
	REPLACE(date.hour, "H");
	REPLACE(date.minute, "m");
	REPLACE(date.second, "s");

	string_reset(s);
	uint64_t u = date.nanos / 1'000;
	if (u < 10)
	{
		E_RET_IF_ERROR_USE2(string_concat(s, "00000"), destroy_string(s));
	}
	else if (u < 100)
	{
		E_RET_IF_ERROR_USE2(string_concat(s, "0000"), destroy_string(s));
	}
	else if (u < 1000)
	{
		E_RET_IF_ERROR_USE2(string_concat(s, "000"), destroy_string(s));
	}
	else if (u < 10000)
	{
		E_RET_IF_ERROR_USE2(string_concat(s, "00"), destroy_string(s));
	}
	else if (u < 100000)
	{
		E_RET_IF_ERROR_USE2(string_concat(s, "0"), destroy_string(s));
	}
	E_RET_IF_ERROR_USE2(number_to_string(u, s), destroy_string(s));
	E_RET_IF_ERROR_USE2(string_replace_all(format_and_result, "u", 1, string_c_str(s), s.length), destroy_string(s));

	string_reset(s);
	uint64_t nanos = date.nanos;
	if (nanos < 10llu)
	{
		E_RET_IF_ERROR_USE2(string_concat(s, "00000000"), destroy_string(s));
	}
	else if (nanos < 100llu)
	{
		E_RET_IF_ERROR_USE2(string_concat(s, "0000000"), destroy_string(s));
	}
	else if (nanos < 1000llu)
	{
		E_RET_IF_ERROR_USE2(string_concat(s, "000000"), destroy_string(s));
	}
	else if (nanos < 10000llu)
	{
		E_RET_IF_ERROR_USE2(string_concat(s, "00000"), destroy_string(s));
	}
	else if (nanos < 100000llu)
	{
		E_RET_IF_ERROR_USE2(string_concat(s, "0000"), destroy_string(s));
	}
	else if (nanos < 1000000llu)
	{
		E_RET_IF_ERROR_USE2(string_concat(s, "000"), destroy_string(s));
	}
	else if (nanos < 10000000llu)
	{
		E_RET_IF_ERROR_USE2(string_concat(s, "00"), destroy_string(s));
	}
	else if (nanos < 100000000llu)
	{
		E_RET_IF_ERROR_USE2(string_concat(s, "0"), destroy_string(s));
	}
	E_RET_IF_ERROR_USE2(number_to_string(u, s), destroy_string(s));
	E_RET_IF_ERROR_USE2(string_replace_all(format_and_result, "S", 1, string_c_str(s), s.length), destroy_string(s));

	string_reset(s);
	const char* ch;
	switch (date.timezone_sign)
	{
	case 1:
		ch = "+";
		break;
	case -1:
		ch = "-";
		break;
	default:
		ch = "?";
		break;
	}

	E_RET_IF_ERROR_USE2(string_concat(s, ch, 1, 1), destroy_string(s));
	
	if (date.timezone_hour < 10)
		E_RET_IF_ERROR_USE2(string_concat(s, "0", 1, 1), destroy_string(s));
	E_RET_IF_ERROR_USE2(number_to_string(date.timezone_hour, s), destroy_string(s));

	E_RET_IF_ERROR_USE2(string_concat(s, ":", 1, 1), destroy_string(s));

	if (date.timezone_minute < 10)
		E_RET_IF_ERROR_USE2(string_concat(s, "0", 1, 1), destroy_string(s));
	E_RET_IF_ERROR_USE2(number_to_string(date.timezone_minute, s), destroy_string(s));

	E_RET_IF_ERROR_USE2(string_replace_all(format_and_result, "P", 1, string_c_str(s), s.length), destroy_string(s));

	destroy_string(s);

	return E_OK;
}
