#pragma once

#include "Duration.h"

#include <stdint.h>
#include <time.h>

enum TimerMeasureType
{
	TMT_SYSTEM = CLOCK_MONOTONIC,
	TMT_PROCESS = CLOCK_PROCESS_CPUTIME_ID,
	TMT_THREAD = CLOCK_THREAD_CPUTIME_ID
};

struct Timer
{
	uint64_t seconds;
	uint64_t nanos;
	TimerMeasureType type;
};

void init_timer(Timer& timer, TimerMeasureType type = TMT_SYSTEM);
int timer_start_or_reset(Timer& timer);
int timer_elapsed(Timer& timer, Duration& result);