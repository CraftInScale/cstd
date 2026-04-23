#pragma once

#include <stdint.h>
#include <stddef.h>

// todo optimize repetetive routines, e.g. multiplication performed each time instead of using Iterator and addition (advancing pointer)

struct Vec
{
	uint8_t* data;
	size_t element_size;// in bytes
	size_t size;// element_count
	size_t capacity; // max_element_count
	bool access_only;// true if realloc failed
};

int create_vec(Vec& vec, size_t element_size, size_t capacity = 0);
void destroy_vec(Vec& vec);

// @param extra_size extra element count over vec.size
int vec_reserve(Vec& vec, size_t extra_size);

int vec_push_back(Vec& vec, void* element);
int vec_push_front(Vec& vec, void* element);
int vec_emplace_back(Vec& vec, void*& element);
int vec_emplace_front(Vec& vec, void*& element);

int vec_insert_inplace(Vec& vec, size_t index, void*& element);
int vec_insert(Vec& vec, size_t index, void* element);

void* vec_get(Vec& vec, size_t index, int& error);
void* vec_get_last(Vec& vec);
int vec_remove(Vec& vec, size_t index);
void vec_remove_last(Vec& vec);
// shrink to lowest power of 2 higher than or equal to size
int vec_shrink(Vec& vec);

void vec_reset(Vec& vec);
