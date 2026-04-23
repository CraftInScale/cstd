#pragma once

#include "Queue.h"
#include "../thread/Mutex.h"

// Lock-based iter, need to call both begin() and finish()
struct ConcurrentQueueIter
{
	Queue* queue;
	Mutex* mutex;
	size_t index;
};

void queue_iter_begin(ConcurrentQueueIter& iter, Queue* queue, Mutex* mutex);
void* queue_iter_next(ConcurrentQueueIter& iter);
void queue_iter_finish(ConcurrentQueueIter& iter);