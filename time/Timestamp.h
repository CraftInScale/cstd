#pragma once

#include <time.h>

struct Timestamp
{
	time_t seconds;
	long nanos;
};

int init_timestamp(Timestamp& timestamp);