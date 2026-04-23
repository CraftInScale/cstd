#pragma once

#include "Vec.h"

#include <stddef.h>

// Vec-based LIFO queue
struct Queue
{
	void* data;
	size_t size;
	//size_t element_count;
	size_t head, tail;

	size_t element_size;
	size_t element_count;

	Vec state;// Vec<bool = uint8_t>
	Vec free_indexes;// Vec<size_t>; can do better with sorted array and binary search for insertion, or tree in array (S-tree)
};

// size must be at least 2, if size=1 then E_DS_FULL on push
int create_queue(Queue& queue, size_t element_size, size_t size = 1024);
void destroy_queue(Queue& queue);

int queue_push(Queue& queue, void* element, size_t* result_index = nullptr);
void* queue_peek(Queue& queue, size_t index, int& error);
void* queue_peek(Queue& queue, int& error);
void* queue_pop(Queue& queue, int& error);
int queue_remove_top(Queue& queue);
int queue_invalidate(Queue& queue, size_t index);
size_t queue_get_size(Queue& queue);
size_t queue_get_free_count(Queue& queue);
