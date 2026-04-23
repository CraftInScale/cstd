#include "ThreadPool.h"

#include "../error.h"
#include "../string/StringConversion.h"
#include "../collections/LinkedListIter.h"

static int _initalize_thdata(ThreadPool& pool, ThreadDataPool* thdata, size_t id)
{
	String thread_name;
	int error = string_clone(pool.basename, thread_name);
	if (error == E_OK)
	{
		error = string_concat(thread_name, " #");
		if (error == E_OK)
		{
			error = number_to_string(id, thread_name);
			if (error == E_OK)
			{
				error = create_thread(thdata->thread, thread_name, pool.stack_size_per_thread);
				if (error == E_OK)
				{
					error = pool.userdata_creator(id, &thdata->thread, pool.userdata_creation_data, &thdata->userdata);
					if (error != E_OK)
					{
						destroy_thread(thdata->thread);
					}
				}
			}
		}

		if (error != E_OK)
			destroy_string(thread_name);
	}

	thdata->id = id;

	return error;
}

int create_threadpool(ThreadPool& threadpool, String name, void* (*thread_routine)(void*), int (*userdata_creator)(int, Thread*, void*, void**), void* userdata_creation_data, void (*userdata_destroyer)(void**), size_t initial_thread_count, size_t stack_size)
{
	threadpool.thread_routine = thread_routine;
	threadpool.userdata_creator = userdata_creator;
	threadpool.userdata_creation_data = userdata_creation_data;
	threadpool.userdata_destroyer = userdata_destroyer;
	threadpool.basename = name;
	threadpool.stack_size_per_thread = stack_size;
	int error;
	E_RET_IF_ERROR_USE(create_linkedlist(threadpool.threads, sizeof(ThreadDataPool)));
	create_mutex(threadpool.threads_lock);

	mutex_lock(threadpool.threads_lock);

	size_t i = 0;
	ThreadDataPool* thdata;
	threadpool.thread_id_gen = initial_thread_count;
	while (i < initial_thread_count)
	{
		thdata = (ThreadDataPool*)linkedlist_emplace_back(threadpool.threads, error);
		if (thdata == nullptr || error != E_OK)
		{
			while (i > 0)
			{
				i -= 1;
				thdata = (ThreadDataPool*)linkedlist_get(threadpool.threads, i);
				destroy_thread(thdata->thread);
				userdata_destroyer(&thdata->userdata);
			}
			mutex_unlock(threadpool.threads_lock);
			return error;
		}

		error = _initalize_thdata(threadpool, thdata, i);

		if (error != E_OK)
		{
			while (i > 0)
			{
				thdata = (ThreadDataPool*)linkedlist_get(threadpool.threads, i);
				destroy_thread(thdata->thread);
				userdata_destroyer(&thdata->userdata);

				i -= 1;
			}

			// i = 0
			thdata = (ThreadDataPool*)linkedlist_get(threadpool.threads, 0);
			destroy_thread(thdata->thread);
			userdata_destroyer(&thdata->userdata);

			mutex_unlock(threadpool.threads_lock);
			return error;
		}

		i += 1;
	}

	mutex_unlock(threadpool.threads_lock);

	return E_OK;
}

void destroy_threadpool(ThreadPool& threadpool)
{
	mutex_lock(threadpool.threads_lock);
	LinkedListIter iter;
	size_t i;

	init_linkedlist_iter(iter, &threadpool.threads);
	i = 0;
	int error;
	while (i < threadpool.threads.size)
	{
		ThreadDataPool* thdata = (ThreadDataPool*)linkedlist_iter_next(iter);
		
		threadpool.userdata_destroyer(&thdata->userdata);
		destroy_thread(thdata->thread);

		i += 1;
	}

	destroy_linkedlist(threadpool.threads);
	mutex_unlock(threadpool.threads_lock);
	destroy_string(threadpool.basename);
}

int threadpool_start(ThreadPool& threadpool, HashMap& errornous_threads)
{
	int error;
	E_RET_IF_ERROR_USE(create_hashmap(errornous_threads, sizeof(size_t), sizeof(int), hfunc_scalar));

	mutex_lock(threadpool.threads_lock);
	size_t i = 0;
	LinkedListIter iter;
	init_linkedlist_iter(iter, &threadpool.threads);

	ThreadDataPool* thdata;
	while ((thdata = (ThreadDataPool*)linkedlist_iter_next(iter)) != nullptr)
	{
		error = thread_start(thdata->thread, threadpool.thread_routine, thdata->userdata);
		if (error != E_OK)
		{
			hashmap_insert(errornous_threads, &i, &error);
		}
		i += 1;
	}

	mutex_unlock(threadpool.threads_lock);

	return E_OK;
}

int threadpool_spawn_thread(ThreadPool& threadpool)
{
	int error;
	mutex_lock(threadpool.threads_lock);
	ThreadDataPool* thdata = (ThreadDataPool*)linkedlist_emplace_back(threadpool.threads, error);
	mutex_unlock(threadpool.threads_lock);

	if (error != E_OK)
		return error;

	size_t id = threadpool.thread_id_gen.fetch_add(1);
	error = _initalize_thdata(threadpool, thdata, id);

	if (error != E_OK)
		return error;

	return thread_start(thdata->thread, threadpool.thread_routine, thdata->userdata);
}

void threadpool_stop(ThreadPool& threadpool)
{
	mutex_lock(threadpool.threads_lock);
	LinkedListIter iter;
	size_t i;

	init_linkedlist_iter(iter, &threadpool.threads);
	i = 0;
	while (i < threadpool.threads.size)
	{
		ThreadDataPool* thdata = (ThreadDataPool*)linkedlist_iter_next(iter);
		thread_schedule_stop(thdata->thread);
		i += 1;
	}

	init_linkedlist_iter(iter, &threadpool.threads);
	i = 0;
	int error;
	while (i < threadpool.threads.size)
	{
		ThreadDataPool* thdata = (ThreadDataPool*)linkedlist_iter_next(iter);

		error = thread_wait_for_stopped(thdata->thread);
		if (error == E_OK)
		{
			thread_join(thdata->thread, nullptr);
		}
		else
		{
			String s;
			if (create_string(s, "Failed to wait-for-stop in thread pool for thread ") == E_OK)
			{
				if (string_concat(s, string_c_str(thdata->thread.name), thdata->thread.name.size - 1) == E_OK)
				{
					print_error(string_c_str(s), error);
				}

				destroy_string(s);
			}
		}

		i += 1;
	}

	mutex_unlock(threadpool.threads_lock);
}
