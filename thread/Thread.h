#pragma once

#include "../string/String.h"
#include "Mutex.h"

#include <pthread.h>
#include <atomic>
#include <semaphore.h>

struct Thread;

struct ThreadData
{
	Thread* thread;
	void* (*user_func)(void*);
	void* user_data;
};

struct Thread
{
	String name;

	pthread_attr_t attr;
	void* stack;
	size_t stack_size;

	pthread_t id;

	std::atomic<bool> running;
	sem_t exit_sem;

	ThreadData* data;
};

//int create_thread_and_start(Thread& thread, int (*routine(void)));
// Thread.h provides routine for producer consumer pattern and so requires routine owning (argument passing - data structure)

// ! it is required that Thread is allocated on heap or guaranteed same address until thread stop
int create_thread(Thread& thread, const char* name, size_t stack_size = 2 * 1024 * 1024);
// ! it is required that Thread is allocated on heap or guaranteed same address until thread stop
int create_thread(Thread& thread, String name, size_t stack_size = 2 * 1024 * 1024);
int thread_start(Thread& thread, void* (*thread_func)(void*), void* func_arg);
int thread_init_routine(Thread& thread);
bool thread_check_if_running(Thread& thread);
void thread_schedule_stop(Thread& thread);
int thread_wait_for_stopped(Thread& thread);

int thread_join(Thread& thread, void** return_value);

// cleanup after handling exit signal
void destroy_thread(Thread& thread);
