#pragma once

#include "Queue.h"

#include <stddef.h>

struct QueueIter
{
	Queue* queue;
	size_t index;     // offset from head, 0-based
	size_t last_slot; // absolute slot index (0..size-1) of last returned element
};

void init_queue_iter(QueueIter& iter, Queue* queue);
// returns nullptr on end
void* queue_iter_next(QueueIter& iter);