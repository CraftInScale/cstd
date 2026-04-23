#pragma once

#include <pthread.h>

struct Mutex
{
	pthread_mutex_t inner;
};

void create_mutex(Mutex& mutex);
void destroy_mutex(Mutex& mutex);

/* @returns pthread_mutex_lock error code or 0 on success */
int mutex_lock(Mutex& mutex);
/* @returns pthread_mutex_unlock error code or 0 on success */
int mutex_unlock(Mutex& mutex);
/* if mutex_lock returned EOWNERDEAD */
void mutex_reown(Mutex& mutex);