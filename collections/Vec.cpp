#include "Vec.h"

#include "../error.h"
#include "../mem.h"
#include "common.h"

#include <stdlib.h>
#include <string.h>

static int _vec_grow(Vec& vec)
{
	if (vec.size < vec.capacity)
		return E_OK;

	size_t current_size = vec.capacity * vec.element_size;
	size_t new_size = current_size * 2;
	if (new_size / 2 != current_size)
	{
		return E_SIZE_OVERFLOW;
	}

	void* data = realloc(vec.data, new_size);
	if (data == nullptr)
	{
		vec.access_only = true;
		return E_ERRNO;
	}

	vec.data = (uint8_t*)data;
	vec.capacity *= 2;

	return E_OK;
}

int create_vec(Vec& vec, size_t element_size, size_t capacity)
{
	vec.data = nullptr;
	vec.element_size = 0;
	vec.size = 0;
	vec.capacity = 0;
	vec.access_only = false;

	if (element_size == 0)
		return E_NULL;

	element_size = pad(element_size);

	size_t cap = capacity == 0 ? 1 : capacity;
	size_t sz = cap * element_size;

	if (sz / element_size != cap)
	{
		return E_SIZE_OVERFLOW;
	}

	void* data = malloc(sz);
	if (data == nullptr)
	{
		return E_ERRNO;
	}

	vec.data = (uint8_t*)data;
	vec.element_size = element_size;
	vec.capacity = cap;

	return E_OK;
}

void destroy_vec(Vec& vec)
{
	if (vec.data != nullptr)
	{
		free(vec.data);
		vec.data = nullptr;
		vec.size = 0;
		vec.capacity = 0;
	}
}

int vec_reserve(Vec& vec, size_t extra_size)
{
	if (vec.size + extra_size <= vec.capacity)
		return E_OK;

	size_t new_count = vec.size + extra_size;
	size_t total_size = new_count * vec.element_size;
	if (total_size / vec.element_size != new_count)
		return E_SIZE_OVERFLOW;

	void* data = realloc(vec.data, total_size);
	if (data == nullptr)
	{
		vec.access_only = true;
		return E_ERRNO;
	}

	vec.data = (uint8_t*)data;
	vec.capacity = new_count;

	return E_OK;
}

int vec_push_back(Vec& vec, void* element)
{
	void* elm = nullptr;
	int error = vec_emplace_back(vec, elm);
	if (error != E_OK)
		return error;

	memcpy(elm, element, vec.element_size);

	return E_OK;
}

int vec_push_front(Vec& vec, void* element)
{
	void* elm = nullptr;
	int error = vec_emplace_front(vec, elm);
	if (error != E_OK)
		return error;

	memcpy(elm, element, vec.element_size);

	return E_OK;
}

int vec_emplace_back(Vec& vec, void*& element)
{
	if (vec.access_only)
		return E_ACCESS_ONLY;

	vec.size += 1;

	int error = _vec_grow(vec);
	if (error != E_OK)
		return error;

	element = vec.data + vec.element_size * (vec.size - 1);

	return E_OK;
}

int vec_emplace_front(Vec& vec, void*& element)
{
	if (vec.access_only)
		return E_ACCESS_ONLY;

	// need to shift entire data by one element

	vec.size += 1;
	int error = _vec_grow(vec);
	if (error != E_OK)
		return error;

	memcpybackward(vec.data + vec.element_size, vec.data, vec.size * vec.element_size);
	element = vec.data;

	return E_OK;
}

int vec_insert_inplace(Vec& vec, size_t index, void*& element)
{
	if (index > vec.size)
		return E_OUT_OF_RANGE;

	int error = _vec_grow(vec);
	if (error != E_OK)
		return error;

	// shift right by 1 the part on right
	memcpybackward(
		vec.data + (index + 1) * vec.element_size,
		vec.data + index * vec.element_size,
		(vec.size - index) * vec.element_size
	);

	element = vec.data + index * vec.element_size;
	vec.size += 1;

	return E_OK;
}

int vec_insert(Vec& vec, size_t index, void* element)
{
	void* ptr;
	int error = vec_insert_inplace(vec, index, ptr);
	if (error != E_OK)
		return error;

	memcpy(ptr, element, vec.element_size);

	return E_OK;
}

void* vec_get(Vec& vec, size_t index, int& error)
{
	error = E_OK;

	if (index >= vec.size)
	{
		error = E_OUT_OF_RANGE;
		return nullptr;
	}

	return vec.data + vec.element_size * index;
}

void* vec_get_last(Vec& vec)
{
	if (vec.size == 0)
		return nullptr;

	return vec.data + (vec.size-1) * vec.element_size;
}

int vec_remove(Vec& vec, size_t index)
{
	if (index >= vec.size)
		return E_OUT_OF_RANGE;

	size_t off = vec.element_size * index;
	memcpy(vec.data + off, vec.data + off + vec.element_size, vec.element_size * vec.size - off - vec.element_size);

	vec.size -= 1;

	return E_OK;
}

void vec_remove_last(Vec& vec)
{
	if (vec.size == 0)
		return;

	vec.size -= 1;
}

int vec_shrink(Vec& vec)
{
	if (vec.size == vec.capacity)
		return E_OK;

	// export highest power of 2 of size

	size_t size = vec.size * vec.element_size;
	size_t bit_index = sizeof(size_t) * 8 - 1;
	size_t max_bit_index = bit_index;
	size_t mask = 1 << bit_index;

	while (true)
	{
		if ((mask & size) != 0)
		{
			break;
		}

		if (mask == 1)
		{
			bit_index = -1;
			break;
		}

		mask >>= 1;
		bit_index -= 1;
	}

	if (bit_index == max_bit_index)
	{
		// size < next power of 2
		return E_OK;
	}

	size_t new_size = 1 << (bit_index + 1);
	if (vec.capacity <= new_size)
	{
		return E_OK;
	}

	// shrink, capacity > next power of 2

	// make correction of size to align to element_size
	new_size = (new_size / vec.element_size) * vec.element_size;

	void* data = realloc(vec.data, new_size);
	if (data == nullptr)
	{
		vec.access_only = true;
		return E_ERRNO;
	}

	vec.data = (uint8_t*)data;
	vec.capacity = new_size / vec.element_size;

	return E_OK;
}

void vec_reset(Vec& vec)
{
	vec.size = 0;
}
