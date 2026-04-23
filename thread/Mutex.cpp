#include "Mutex.h"

#include "../error.h"

void create_mutex(Mutex& mutex)
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
	pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);

	pthread_mutex_init(&mutex.inner, &attr);
}

void destroy_mutex(Mutex& mutex)
{
	pthread_mutex_destroy(&mutex.inner);
}

int mutex_lock(Mutex& mutex)
{
	return pthread_mutex_lock(&mutex.inner);
}

int mutex_unlock(Mutex& mutex)
{
	return pthread_mutex_unlock(&mutex.inner);
}

void mutex_reown(Mutex& mutex)
{
	pthread_mutex_consistent(&mutex.inner);
}
