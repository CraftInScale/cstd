#pragma once

#include "Timestamp.h"
#include "../string/String.h"

#include <stdint.h>

struct DateTime
{
	int year;
	int month;
	int day;

	int hour;
	int minute;
	int second;
	long nanos;

	int timezone_sign;
	int timezone_hour;
	int timezone_minute;
};

void init_datetime(DateTime& date, Timestamp& timestamp);
/*
placeholders: Y-M-D H:m:s.u/S P
P - timezone
u - micros
S - nanos
*/
int datetime_format(DateTime& date, String& format_and_result);

static inline bool is_leap_year(int year)
{
	// year & 3 <=> year % 4
	// (year & 3) == 0 && (year % 100 != 0 || year % 400 == 0);

	// optimized version

	// The Textbook Formula [2-6x speed]
	// year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)

	// Neri-Schneider (Version 1)
	// bool is_cen = (year % 100 == 0)
	// bool leap = is_cen ? (year % 16 == 0) : (year % 4 == 0)

	// Drepper-Neri-Schneider (version 2) [1x speed]
	// bool is_cen = (year % 100 == 0)
	// bool leap = (year & (is_cen ? 15 : 3) == 0)

	// Drepper-Neri-Schneider (version 3)
	// bool is_cen = (year % 25 == 0)
	// bool leap = (year & (is_cen ? 15 : 3) == 0)

	// You can see that the century line is checking for divisibility by 25 rather than by 100. 
	// The reason for this is that modern compilers are able to compile % 25 more efficiently than % 100. 
	// This will yield false-positives for the century check, 
	// but they do not matter, as they are cancelled out in the next step.

	// Drepper-Neri-Schneider (version 3 - manually unrolled)
	// const int32 A = -1030792151
	// const uint32 B = 85899345
	// const uint32 C = 171798691
	//
	// bool is_cen = uint32(year * A + B) < C
	// bool is_leap = (year & (is_cen ? 15 : 3)) == 0

	// On 15 May 2025, Falk Hüffner published an incredibly brief 3-instruction function to compute the leap year check over a restricted range of years

	// 0 ➜ 102499
	// Falk Hüffner's Algorithm (Unsigned 32-bit) [0.85x speed]
	// bool is_leap = (uint32(year * 1073750999) & 3221352463) <= 126976

	// 0 ➜ 5965232499
	// Falk Hüffner's Algorithm (Unsigned 64-bit)
	// const uint64 f = 4611686019114582671
	// const uint64 m = 13835058121854156815
	// const uint64 t = 66571993088
	// bool is_leap = (uint64(year * f) & m) <= t

	// https://hueffner.de/falk/blog/a-leap-year-check-in-three-instructions.html

	return (uint32_t(year * 1073750999) & 3221352463u) <= 126976u;
}