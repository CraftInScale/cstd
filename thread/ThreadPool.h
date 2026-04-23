#pragma once

#include "../collections/LinkedList.h"
#include "../collections/HashMap.h"
#include "Thread.h"

#include <atomic>

typedef size_t thread_id;

struct ThreadDataPool
{
	Thread thread;
	void* userdata;
	thread_id id;
};

struct ThreadPool
{
	LinkedList threads;// LinkedList<ThreadDataPool>
	std::atomic<thread_id> thread_id_gen;
	Mutex threads_lock;
	
	String basename;
	int stack_size_per_thread;
	void* (*thread_routine)(void*);
	int (*userdata_creator)(int, Thread*, void*, void**);
	void (*userdata_destroyer)(void**);
	void* userdata_creation_data;
};

/*
* @param userdata_creator Thread, userdata_creation_data, thread_data_pointer
*/
int create_threadpool(ThreadPool& threadpool, String basename, void* (*thread_routine)(void*), int (*userdata_creator)(int, Thread*, void*, void**), void* userdata_creation_data, void (*userdata_destroyer)(void**), size_t initial_thread_count = 1, size_t stack_size_per_thread = 2 * 1024 * 1024);
// stops all threads, waits for all finish and returns; if failed to wait-for-stop for some then thread is not destroyed
void destroy_threadpool(ThreadPool& threadpool);

/*
	@param errornous_threads creates HashMap<size_t = thread number, int = error> (can be empty)
	@returns error if failed to create errornous_threads
*/
int threadpool_start(ThreadPool& threadpool, HashMap& errornous_threads);

// spawn a thread (create and start) and include into threadpool
int threadpool_spawn_thread(ThreadPool& threadpool);

void threadpool_stop(ThreadPool& threadpool);