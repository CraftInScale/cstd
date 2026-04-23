#include "Timer.h"

#include "../error.h"

void init_timer(Timer& timer, TimerMeasureType type)
{
	timer.type = type;
}

int timer_start_or_reset(Timer& timer)
{
	timespec ts;
	int error = clock_gettime(timer.type, &ts);
	if (error == -1)
		return E_ERRNO;

	timer.seconds = ts.tv_sec;
	timer.nanos = ts.tv_nsec;

	return E_OK;
}

int timer_elapsed(Timer& timer, Duration& result)
{
	timespec ts;
	int error = clock_gettime(timer.type, &ts);
	if (error == -1)
		return E_ERRNO;

	result.seconds = uint64_t(ts.tv_sec) - timer.seconds;
	result.nanos = 1'000'000'000llu - timer.nanos + uint64_t(ts.tv_nsec);

	if (result.nanos > 1'000'000'000llu)
	{
		result.seconds += 1;
		result.nanos -= 1'000'000'000llu;
	}

	return E_OK;
}
