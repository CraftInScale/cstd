#include "Thread.h"

#include "../error.h"
#include "../log/Logger.h"
#include "../string/StringConversion.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <atomic>
#include <semaphore.h>

void* thread_routine(void* data)
{
	ThreadData* thdata = (ThreadData*)data;

	int error = thread_init_routine(*thdata->thread);
	if (error != E_OK)
	{
		print_error("Error initializing thread", error);
		return nullptr;
	}

	void* ret = thdata->user_func(thdata->user_data);

	Thread* thread = thdata->thread;

	logger_log("Exited thread");
	unset_thread(thread->id);

	free(data);

	// need to signal thread exit
	// duration-less waiting with semaphore (spin wait?) (as opposite to conditional variable with lock)
	sem_post(&thread->exit_sem);

	return ret;
}

int create_thread(Thread& thread, const char* name, size_t stack_size)
{
	String name_s;
	create_string(name_s, name);

	return create_thread(thread, name_s, stack_size);
}

int create_thread(Thread& thread, String name, size_t stack_size)
{
	thread.name = name;
	thread.data = nullptr;
	thread.stack_size = stack_size;
	thread.stack = malloc(stack_size);
	if (thread.stack == nullptr)
		return E_ERRNO;

	int error = sem_init(&thread.exit_sem, 0, 0);
	if (error == -1)
	{
		free(thread.stack);
		return E_ERRNO;
	}

	pthread_attr_t* attr_ptr = &thread.attr;

	pthread_attr_init(attr_ptr);
	pthread_attr_setstack(attr_ptr, thread.stack, thread.stack_size);

	// won't turn off exit signaling?
	sigset_t sigmask;
	bzero(&sigmask.__val[0], 16 * sizeof(unsigned long));
	pthread_attr_setsigmask_np(attr_ptr, &sigmask);

	thread.running.store(false);

	return E_OK;
}

int thread_start(Thread& thread, void* (*thread_func)(void*), void* func_arg)
{
	//int ec = pthread_create(&thread.uuid, &thread.attr, thread_func, func_arg);
	ThreadData* thdata = (ThreadData*)malloc(sizeof(ThreadData));
	thdata->thread = &thread;
	thdata->user_func = thread_func;
	thdata->user_data = func_arg;
	thread.data = thdata;

	atomic_exchange(&thread.running, true);
	int ec = pthread_create(&thread.id, &thread.attr, thread_routine, thdata);
	if (ec != 0)
	{
		atomic_exchange(&thread.running, false);
		thread.data = nullptr;
		free(thdata);
		errno = ec;
		return E_ERRNO;
	}

	return E_OK;
}

int thread_init_routine(Thread& thread)
{
	String name_cloned;
	int error = string_clone(thread.name, name_cloned);
	if (error != E_OK)
		return error;

	set_thread_name(gettid(), name_cloned);

	return E_OK;
}

bool thread_check_if_running(Thread& thread)
{
	bool status = thread.running.load();

	if (!status)
	{
		logger_log("Stopping thread");
	}

	return status;
}

void thread_schedule_stop(Thread& thread)
{
	atomic_exchange(&thread.running, false);
}

int thread_wait_for_stopped(Thread& thread)
{
	int error = sem_wait(&thread.exit_sem);
	if (error == -1)
		return E_ERRNO;

	return E_OK;
}

int thread_join(Thread& thread, void** return_value)
{
	int error = pthread_join(thread.id, return_value);

	if (error != 0)
	{
		errno = error;
		return E_ERRNO;
	}

	return E_OK;
}

void destroy_thread(Thread& thread)
{
	if (thread.stack == nullptr)
		return;

	free(thread.stack);
	thread.stack = nullptr;

	sem_destroy(&thread.exit_sem);
	destroy_string(thread.name);
}
