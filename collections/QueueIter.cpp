#include "QueueIter.h"

void init_queue_iter(QueueIter& iter, Queue* queue)
{
	iter.queue = queue;
	iter.index = queue->head;
}

void* queue_iter_next(QueueIter& iter)
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
		{
			iter.last_slot = slot;
			return (uint8_t*)queue.data + slot * queue.element_size;
		}
	}

	return nullptr;
}
