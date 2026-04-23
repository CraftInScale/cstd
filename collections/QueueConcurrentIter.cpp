#include "QueueConcurrentIter.h"

void queue_iter_begin(ConcurrentQueueIter& iter, Queue* queue, Mutex* mutex)
{
	iter.queue = queue;
	iter.mutex = mutex;
	iter.index = queue->head;

	mutex_lock(*mutex);
}

void* queue_iter_next(ConcurrentQueueIter& iter)
{
	Queue& queue = *iter.queue;

	size_t range = queue.head <= queue.tail
		? queue.tail - queue.head
		: (queue.size - queue.head) + queue.tail;

	while (iter.index - queue.head < range)
	{
		size_t slot = iter.index % queue.size;
		iter.index++;

		uint8_t* state = (uint8_t*)queue.state.data + slot;
		if (*state == 1)
			return (uint8_t*)queue.data + slot * queue.element_size;
	}

	return nullptr;
}

void queue_iter_finish(ConcurrentQueueIter& iter)
{
	mutex_unlock(*iter.mutex);
}
