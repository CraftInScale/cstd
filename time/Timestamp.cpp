#include "Timestamp.h"

#include "../error.h"

//#include <sys/time.h>
#include <time.h>

int init_timestamp(Timestamp& timestamp)
{
	/*timeval tv;
	int error = gettimeofday(&tv, nullptr);
	if (error == -1)
		return E_ERRNO;

	timestamp.seconds = tv.tv_sec;
	timestamp.micros = tv.tv_usec;*/

	timespec ts;
	int error = clock_gettime(CLOCK_REALTIME, &ts);
	if (error == -1)
		return E_ERRNO;

	timestamp.seconds = ts.tv_sec;
	timestamp.nanos = ts.tv_nsec;

	return E_OK;
}