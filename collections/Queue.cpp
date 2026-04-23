#include "Queue.h"

#include "../error.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int create_queue(Queue& queue, size_t element_size, size_t size)
{
	queue.data = nullptr;
	queue.element_size = element_size;
	queue.element_count = 0;
	queue.size = 0;
	queue.head = 0;
	queue.tail = 0;

	size_t cap = size != 0 ? size : 1;
	size_t alloc_size = cap * element_size;

	void* data = malloc(alloc_size);
	if (data == nullptr)
	{
		return E_ERRNO;
	}

	int error = create_vec(queue.state, sizeof(uint8_t), cap);
	if (error != E_OK)
	{
		free(data);
		return error;
	}

	queue.state.size = cap;

	error = create_vec(queue.free_indexes, sizeof(size_t));
	if (error != E_OK)
	{
		destroy_vec(queue.state);
		free(data);
		return error;
	}

	queue.data = data;
	queue.size = cap;

	return E_OK;
}

void destroy_queue(Queue& queue)
{
	if (queue.data != nullptr)
	{
		free(queue.data);
		queue.head = 0;
		queue.tail = 0;
	}
}

static int _find_min_free_index(Queue& queue, bool& found, size_t& index, uint8_t*& state)
{
	found = false;
	size_t elm_count = queue.free_indexes.size;

	if (elm_count > 0)
	{
		size_t min_free_index = -1;
		size_t min_index = 0;
		size_t* free_index;

		int error;
		size_t i = 0;
		while (i < elm_count)
		{
			free_index = (size_t*)vec_get(queue.free_indexes, i, error);
			if (error != E_OK)
				return error;

			if (*free_index <= min_free_index)
			{
				min_free_index = *free_index;
				min_index = i;
			}

			i += 1;
		}

		error = vec_remove(queue.free_indexes, min_index);
		if (error != E_OK)
			return error;

		found = true;
		index = min_free_index;
		state = (uint8_t*)vec_get(queue.state, index, error);
		return error;
	}
	else
	{
		found = false;
		return E_OK;
	}
}

// for pop operation, FIFO order => first from head
static int _find_first_occupied_index(Queue& queue, bool& found, size_t& index, uint8_t*& state)
{
	found = false;

	// begin at head, move towards tail

	size_t i = queue.head;
	size_t i_max = queue.tail >= queue.head ? queue.tail : queue.size;
	int error;

	while (i < i_max)
	{
		state = (uint8_t*)vec_get(queue.state, i, error);
		if (error != E_OK)
			return error;

		if (*state == 1)
		{
			found = true;
			index = i;
			return E_OK;
		}

		i += 1;
	}

	if(queue.tail < queue.head)
	{
		// second loop 0 -> tail
		i = 0;
		while (i < queue.tail)
		{
			state = (uint8_t*)vec_get(queue.state, i, error);
			if (error != E_OK)
				return error;

			if (*state == 1)
			{
				found = true;
				index = i;
				return E_OK;
			}

			i += 1;
		}
	}

	return E_OK;
}

int queue_push(Queue& queue, void* element, size_t* result_index)
{
	// check if any free index
	bool found;
	size_t index;
	uint8_t* state;
	int error = _find_min_free_index(queue, found, index, state);
	if (error != E_OK)
		return error;

	if (found)
	{
		*state = 1;
		queue.element_count += 1;

		if (result_index != nullptr)
		{
			*result_index = index;
		}

		memcpy(((uint8_t*)queue.data) + index * queue.element_size, element, queue.element_size);

		return E_OK;
	}
	else
	{
		if ((queue.tail + 1) % queue.size == queue.head)
		{
			return E_DS_FULL;
		}

		int error;
		uint8_t* state = (uint8_t*)vec_get(queue.state, queue.tail, error);
		if (error != E_OK)
			return error;

		*state = 1;
		queue.element_count += 1;

		if (result_index != nullptr)
		{
			*result_index = queue.tail;
		}

		memcpy(((uint8_t*)queue.data) + queue.tail * queue.element_size, element, queue.element_size);
		queue.tail = (queue.tail + 1) % queue.size;

		return E_OK;
	}
}

void* queue_peek(Queue& queue, size_t index, int& error)
{
	if (index >= queue.size)
	{
		error = E_OUT_OF_RANGE;
		return nullptr;
	}

	if (queue.tail == queue.head)
	{
		error = E_DS_EMPTY;
		return nullptr;
	}
	
	if (queue.tail > queue.head)
	{
		if (index < queue.head)
		{
			error = E_OUT_OF_RANGE;
			return nullptr;
		}
	}
	else
	{
		if (index >= queue.tail)
		{
			if (index < queue.head)
			{
				error = E_OUT_OF_RANGE;
				return nullptr;
			}
		}
	}

	error = E_OK;
	return ((uint8_t*)queue.data) + index * queue.element_size;
}

void* queue_peek(Queue& queue, int& error)
{
	if (queue.element_count == 0)
	{
		error = E_DS_EMPTY;
		return nullptr;
	}

	bool found;
	size_t index;
	uint8_t* state;
	error = _find_first_occupied_index(queue, found, index, state);
	if (error != E_OK)
		return nullptr;

	if (!found)
	{
		error = E_DS_EMPTY;
		return nullptr;
	}

	return ((uint8_t*)queue.data) + index * queue.element_size;
}

void* queue_pop(Queue& queue, int& error)
{
	if (queue.element_count == 0)
	{
		error = E_DS_EMPTY;
		return nullptr;
	}

	bool found;
	size_t index;
	uint8_t* state;
	error = _find_first_occupied_index(queue, found, index, state);
	if (error != E_OK)
		return nullptr;

	if (!found)
	{
		error = E_DS_EMPTY;
		return nullptr;
	}

	*state = 0;
	queue.element_count -= 1;

	queue.head = (index + 1) % queue.size;

	void* tmp = malloc(queue.element_size);
	if (tmp == nullptr)
	{
		error = E_ERRNO;
		return nullptr;
	}

	memcpy(tmp, ((uint8_t*)queue.data) + index * queue.element_size, queue.element_size);

	return tmp;
}

int queue_remove_top(Queue& queue)
{
	if (queue.element_count == 0)
	{
		return E_DS_EMPTY;
	}

	bool found;
	size_t index;
	uint8_t* state;
	int error = _find_first_occupied_index(queue, found, index, state);
	if (error != E_OK)
		return error;

	if (!found)
	{
		return E_DS_EMPTY;
	}

	*state = 0;
	queue.element_count -= 1;

	queue.head = (index + 1) % queue.size;

	return E_OK;
}

int queue_invalidate(Queue& queue, size_t index)
{
	if (index >= queue.size)
	{
		return E_OUT_OF_RANGE;
	}

	int error;
	uint8_t* state = (uint8_t*)vec_get(queue.state, index, error);
	if (error != E_OK)
		return error;

	error = vec_push_back(queue.free_indexes, &index);
	if (error != E_OK)
		return error;

	*state = 0;

	return E_OK;
}

size_t queue_get_size(Queue& queue)
{
	return queue.element_count;
}

size_t queue_get_free_count(Queue& queue)
{
	size_t head, tail, qsize, count;

	head = queue.head;
	tail = queue.tail;
	qsize = queue.size;
	count = queue.element_count;

	size_t size = head <= tail ? (tail - head) : ((qsize - head) + tail);
	size_t count2 = qsize - size;
	size_t count1 = size - count;

	return count1 + count2 + queue.free_indexes.size;
}
