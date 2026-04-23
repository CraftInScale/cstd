#include "ChunkVec.h"

#include "../error.h"
#include "../mem.h"

#include <stdlib.h>
#include <string.h>

static int _init_chunk(ChunkVec& vec, size_t index)
{
	VecChunk* chunk = vec.chunks + index;

	if (index < vec.backward_capacity)
	{
		chunk->offset = vec.chunk_size;
		chunk->direction = DIR_BACKWARD;
	}
	else
	{
		chunk->offset = 0;
		chunk->direction = DIR_FORWARD;
	}

	chunk->data = (uint8_t*)malloc(vec.element_size * vec.chunk_size);
	if (chunk->data == nullptr)
		return E_ERRNO;

	return E_OK;
}

static void _destroy_chunk(ChunkVec& vec, size_t index)
{
	VecChunk* chunk = vec.chunks + index;
	if (chunk->data != nullptr)
	{
		free(chunk->data);
		chunk->data = nullptr;
	}
}

static void _translate_index(ChunkVec& vec, size_t index, size_t& chunk_index, size_t& atchunk_index)
{
	/*size_t zero = vec.starting_chunk_index;
	VecChunk* chunk = vec.chunks + zero;
	size_t alloc_size = (vec.chunk_size - 1) - chunk->offset;

	if (index <= alloc_size)
	{
		chunk_index = zero;
		atchunk_index = index + chunk->offset;
		return;
	}

	index -= alloc_size;
	size_t chindex = index / vec.chunk_size;
	chunk_index = vec.starting_chunk_index + 1 + chindex;
	atchunk_index = index - chindex * vec.chunk_size;*/

	size_t zero = vec.starting_chunk_index;
	VecChunk* chunk = vec.chunks + zero;

	/*if (glob_flux->started.load())
	{
		if (vec.chunks != nullptr)
		{
			logger_log("vec chunks not nullptr");
		}
		else logger_log("vec chunks nullptr");
	}*/

	switch (chunk->direction)
	{
	case DIR_FORWARD:
	{
		// no backward chunk
		chunk_index = zero + index / vec.chunk_size;
		atchunk_index = index - (chunk_index - zero) * vec.chunk_size;
	}
		break;
	case DIR_BACKWARD:
	{
		// extra unary offset due to backward chunk
		size_t unary_off = vec.chunk_size - chunk->offset;
		if (index < unary_off)
		{
			chunk_index = zero;
			atchunk_index = chunk->offset + index;
		}
		else
		{
			index -= unary_off;
			chunk_index = zero + 1 + index / vec.chunk_size;
			atchunk_index = index - (chunk_index - zero - 1) * vec.chunk_size;
		}
	}
		break;
	}
}

static int _grow_forward(ChunkVec& vec)
{
	size_t new_cap = vec.backward_capacity + vec.forward_capacity;
	if (new_cap == __SIZE_MAX__)
		return E_SIZE_OVERFLOW;

	new_cap += 1;
	size_t new_size = sizeof(VecChunk) * new_cap;
	if (new_size / sizeof(VecChunk) != new_cap)
		return E_SIZE_OVERFLOW;

	void* data = realloc(vec.chunks, new_size);
	if (data == nullptr)
	{
		vec.access_only = true;
		return E_ERRNO;
	}

	vec.chunks = (VecChunk*)data;

	_init_chunk(vec, vec.backward_capacity + vec.forward_capacity);
	vec.forward_capacity += 1;

	return E_OK;
}

static int _grow_backward(ChunkVec& vec)
{
	size_t cap = vec.backward_capacity + vec.forward_capacity;
	if (cap == __SIZE_MAX__)
		return E_SIZE_OVERFLOW;

	cap += 1;
	size_t size = sizeof(VecChunk) * cap;
	if (size / sizeof(VecChunk) != cap)
		return E_SIZE_OVERFLOW;

	void* chunk_data = malloc(vec.element_size * vec.chunk_size);
	if (chunk_data == nullptr)
		return E_ERRNO;

	void* data = realloc(vec.chunks, size);
	if (data == nullptr)
	{
		free(chunk_data);
		vec.access_only = true;
		return E_ERRNO;
	}

	vec.chunks = (VecChunk*)data;

	// shift right by 1
	// copy last backward (first from left) and all used forwards
	//memcpybackward(vec.chunks + 1 + vec.backward_capacity, vec.chunks + vec.backward_capacity, size - sizeof(VecChunk));
	/*size_t n = vec.forward_capacity * vec.chunk_size;
	VecChunk* chunk = vec.chunks + vec.starting_chunk_index;
	switch (chunk->direction)
	{
	case DIR_BACKWARD:
	{
		size_t backward_elm_count = vec.chunk_size-1 - chunk->offset;
		if (n <= backward_elm_count)
		{
			n = 1;
		}
		else
		{
			size_t forward_elm_count = n - backward_elm_count;
			n = forward_elm_count / vec.chunk_size;
			if (n * vec.chunk_size != forward_elm_count)
			{
				n += 1;
			}
		}
	}
		break;
	case DIR_FORWARD:
		n = n / vec.chunk_size;
		if (n * vec.chunk_size != vec.size)
		{
			n += 1;
		}
		break;
	}*/
	size_t n = vec.forward_capacity;
	vec.backward_capacity += 1;
	memcpybackward(vec.chunks + vec.backward_capacity, vec.chunks + vec.backward_capacity - 1, n * sizeof(VecChunk));

	VecChunk* chunk = vec.chunks + vec.backward_capacity - 1;
	chunk->data = (uint8_t*)chunk_data;
	chunk->offset = vec.chunk_size;
	chunk->direction = DIR_BACKWARD;

	//vec.backward_capacity += 1;
	vec.starting_chunk_index += 1;

	return E_OK;
}

int create_chunkvec(ChunkVec& vec, size_t element_size, size_t chunk_size, size_t backward_capacity, size_t forward_capacity)
{
	vec.chunks = nullptr;
	vec.element_size = element_size;

	size_t chunk_data_size = element_size * chunk_size;
	if (chunk_data_size / element_size != chunk_size)
		return E_SIZE_OVERFLOW;

	if (backward_capacity == 0)
		backward_capacity = 1;

	if (forward_capacity == 0)
		forward_capacity = 1;

	size_t cap = backward_capacity + forward_capacity;
	if (backward_capacity > __SIZE_MAX__ - forward_capacity)
		return E_SIZE_OVERFLOW;

	size_t size = sizeof(VecChunk) * cap;
	if (size / sizeof(VecChunk) != cap)
		return E_SIZE_OVERFLOW;

	void* data = malloc(size);
	if (data == nullptr)
	{
		return E_ERRNO;
	}

	vec.chunks = (VecChunk*)data;
	vec.chunk_size = chunk_size;
	vec.size = 0;
	vec.forward_capacity = forward_capacity;
	vec.backward_capacity = backward_capacity;
	vec.starting_chunk_index = backward_capacity;// - 1;

	size_t i = 0;
	int error;
	while (i < cap)
	{
		error = _init_chunk(vec, i);
		if (error != E_OK)
		{
			while (i > 0)
			{
				_destroy_chunk(vec, i);
				i -= 1;
			}

			free(data);
			vec.chunks = nullptr;
			return error;
		}
		i += 1;
	}

	return E_OK;
}

void destroy_chunkvec(ChunkVec& vec)
{
	if (vec.chunks == nullptr)
		return;

	size_t i = 0;
	while (i < vec.backward_capacity + vec.forward_capacity)
	{
		_destroy_chunk(vec, i);
		i += 1;
	}

	free(vec.chunks);
	vec.chunks = nullptr;
}

int chunkvec_push_back(ChunkVec& vec, void* element)
{
	void* elm = nullptr;
	int error = chunkvec_emplace_back(vec, elm);
	if (error != E_OK)
	{
		return error;
	}

	memcpy(elm, element, vec.element_size);

	return E_OK;
}

int chunkvec_push_front(ChunkVec& vec, void* element)
{
	void* elm = nullptr;
	int error = chunkvec_emplace_front(vec, elm);
	if (error != E_OK)
	{
		return error;
	}

	memcpy(elm, element, vec.element_size);

	return E_OK;
}

int chunkvec_emplace_back(ChunkVec& vec, void*& element)
{
	size_t chunk_index;
	size_t atchunk_index;
	_translate_index(vec, vec.size, chunk_index, atchunk_index);

	if (chunk_index >= vec.starting_chunk_index + vec.forward_capacity)
	{
		_grow_forward(vec);
	}

	VecChunk* chunk = vec.chunks + chunk_index;
	element = chunk->data + atchunk_index * vec.element_size;
	chunk->offset += 1;
	vec.size += 1;

	return E_OK;
}

int chunkvec_emplace_front(ChunkVec& vec, void*& element)
{
	// push to backward
	VecChunk* chunk = vec.chunks + vec.starting_chunk_index;

	if (chunk->direction == DIR_FORWARD)
	{
		if (vec.backward_capacity == 0)
		{
			int error = _grow_backward(vec);
			if (error != E_OK)
				return error;
		}

		vec.starting_chunk_index -= 1;
		chunk = vec.chunks + vec.starting_chunk_index;
	}

	// chunk->direction = DIR_BACKWARD

	if (chunk->offset == 0)
	{
		chunk->direction = DIR_FORWARD; 
		chunk->offset = vec.chunk_size;
		vec.backward_capacity -= 1;
		vec.forward_capacity += 1;

		if (vec.backward_capacity == 0)
		{
			int error = _grow_backward(vec);
			if (error != E_OK)
				return error;
		}

		vec.starting_chunk_index -= 1;
		chunk = vec.chunks + vec.starting_chunk_index;
	}

	chunk->offset -= 1;
	element = chunk->data + chunk->offset * vec.element_size;

	vec.size += 1;

	return E_OK;
}

int chunkvec_insert_inplace(ChunkVec& vec, size_t index, void*& element)
{
	if (index > vec.size)
		return E_OUT_OF_RANGE;

	size_t chunk_index;
	size_t atchunk_index;
	_translate_index(vec, index, chunk_index, atchunk_index);

	// chunk either full or not

	VecChunk* chunk = vec.chunks + chunk_index;

	// go up to first non-full chunk
	VecChunk* end = vec.chunks + vec.backward_capacity + vec.forward_capacity;
	size_t chunk_iter_index = chunk_index;
	while (chunk < end)
	{
		if (chunk->offset == vec.chunk_size)
		{
			chunk += 1;
			chunk_iter_index += 1;
		}
		else break;
	}

	// last chunk full, need one more slot -> need one more chunk
	if (chunk == end)
	{
		int error = _grow_forward(vec);
		if (error != E_OK)
			return error;

		// copy last element to next chunk
		/*chunk -= 1;
		memcpy(
			end->data,
			chunk->data + (vec.chunk_size - 1) * vec.element_size,
			vec.element_size
		);*/
		chunk = vec.chunks + chunk_iter_index;
		chunk->offset += 1;
	}

	// move data up by 1 element
	// in reverse order

	VecChunk* first = vec.chunks + chunk_index;
	VecChunk* prev = chunk - 1;
	VecChunk* next = chunk;
	while (prev > first)
	{
		// copy last element of prev to first element of next
		memcpy(
			next->data,
			prev->data + (vec.chunk_size - 1) * vec.element_size,
			vec.element_size
		);

		// move up by 1 in prev
		memcpybackward(
			prev->data + vec.element_size,
			prev->data,
			(vec.chunk_size - 1) * vec.element_size
		);

		next -= 1;
		prev -= 1;
	}

	// prev is +1 from chunk, free first element

	// copy last element of chunk to first element of next
	// after successful loop execution
	if (prev == first)
	{
		// next = first + 1

		memcpy(
			next->data,
			prev->data + (vec.chunk_size - 1) * vec.element_size,
			vec.element_size
		);
	}

	chunk = first;

	// move locally up by 1 from atchunk_index
	memcpy(
		chunk->data + (atchunk_index + 1) * vec.element_size,
		chunk->data + atchunk_index * vec.element_size,
		(vec.chunk_size - 1 - atchunk_index) * vec.element_size
	);

	element = chunk->data + atchunk_index * vec.element_size;
	vec.size += 1;

	return E_OK;
}

//int _chunkvec_insert_inplace(ChunkVec& vec, size_t index, void*& element)
//{
//	if (index > vec.size)
//		return E_OUT_OF_RANGE;
//
//	size_t chunk_index;
//	size_t atchunk_index;
//	_translate_index(vec, index, chunk_index, atchunk_index);
//
//	VecChunk* chunk = vec.chunks + chunk_index;
//
//	// copy data on right inclusive up by 1 position
//
//	// from end
//
//	size_t chunk_index2;
//	size_t atchunk_index2;
//	_translate_index(vec, vec.size - 1, chunk_index2, atchunk_index2);
//
//	if (atchunk_index2 == vec.chunk_size - 1)
//	{
//		/*bool more = false;
//		VecChunk* zero_chunk = vec.chunks + vec.starting_chunk_index;
//		if (zero_chunk->direction == DIR_BACKWARD)
//		{
//			int backward_off = (vec.chunk_size - zero_chunk->offset);
//			int forward_off = vec.size - backward_off;
//			int chunk_count = forward_off / vec.chunk_size;
//			if (chunk_count * vec.chunk_size < forward_off)
//			{
//				chunk_count += 1;
//			}
//			chunk_count += vec.backward_capacity;
//
//			more = chunk_count >= vec.forward_capacity;
//		}
//		else more = */
//
//		if (chunk_index2 >= vec.backward_capacity + vec.forward_capacity - 1)
//		{
//			int error = _grow_forward(vec);
//			if (error != E_OK)
//				return error;
//		}
//
//		VecChunk* chunk_end = vec.chunks + chunk_index2 + 1;
//		VecChunk* chunk_prev = vec.chunks + chunk_index2;
//
//		// copy last element from chunk to chunk_end first element
//		memcpy(
//			chunk_end->data,
//			chunk_prev->data + atchunk_index2 * vec.element_size,
//			vec.element_size
//		);
//
//		chunk_end->offset += 1;
//	}
//
//	if (chunk_index2 > chunk_index)
//	{
//		// copy from chunk_index2 downward to chunk
//		VecChunk* ch = vec.chunks + chunk_index2;
//		VecChunk* ch_prev = vec.chunks + chunk_index2 - 1;
//		while (ch > chunk)
//		{
//			// move locally in next chunk the data up by 1 element
//			memcpybackward(
//				ch->data + vec.element_size,
//				ch->data,
//				(vec.chunk_size - 1) * vec.element_size
//			);
//
//			// copy last element of prev chunk to first element of next chunk
//			memcpy(
//				ch->data,
//				ch_prev->data + (vec.chunk_size - 1) * vec.element_size,
//				vec.element_size
//			);
//
//			ch -= 1;
//		}
//	}
//
//	// move locally in index chunk the data up by 1 element
//	memcpybackward(
//		chunk->data + (atchunk_index + 1) * vec.element_size,
//		chunk->data + atchunk_index * vec.element_size,
//		(vec.chunk_size - atchunk_index - 1) * vec.element_size
//	);
//
//	element = chunk->data + atchunk_index * vec.element_size;
//	vec.size += 1;
//
//	return E_OK;
//}

int chunkvec_insert(ChunkVec& vec, size_t index, void* element)
{
	void* ptr = nullptr;
	int error = chunkvec_insert_inplace(vec, index, ptr);
	if (error != E_OK)
		return error;

	memcpy(ptr, element, vec.element_size);

	return E_OK;
}

static int chunkvec_emplace_front_old(ChunkVec& vec, void*& element)
{
	size_t chunk_index = vec.starting_chunk_index;
	VecChunk* chunk = vec.chunks + chunk_index;

	vec.size += 1;

	// set backward chunk at starting index if chunk full
	switch (chunk->direction)
	{
	case DIR_BACKWARD:
	{
		if (chunk->offset == 0)
		{
			chunk->direction = DIR_FORWARD;

			if (vec.backward_capacity <= 1)
			{
				int error = _grow_backward(vec);
				if (error != E_OK)
					return error;
			}

			vec.backward_capacity -= 1;
			vec.forward_capacity += 1;
			vec.starting_chunk_index -= 1;

			chunk = vec.chunks + vec.starting_chunk_index;
			chunk->offset -= 1;
			element = chunk->data + chunk->offset * vec.element_size;

			/*if (vec.backward_capacity <= 1)
			{
				int error = _grow_backward(vec);
				if (error != E_OK)
					return error;

				chunk = vec.chunks + vec.starting_chunk_index;
				vec.starting_chunk_index -= 1;

				// transform full chunk into forward chunk
				vec.backward_capacity -= 1;
				vec.forward_capacity += 1;
				(vec.chunks + vec.starting_chunk_index + 1)->direction = DIR_FORWARD;
			}
			else
			{
				// transform full chunk into forward chunk
				vec.backward_capacity -= 1;
				vec.forward_capacity += 1;
				chunk->direction = DIR_FORWARD;
			}

			element = chunk->data + chunk->offset * vec.element_size;*/
		}
		else 
		{
			chunk->offset -= 1;
			element = chunk->data + chunk->offset * vec.element_size;
		}
	}
		break;
	case DIR_FORWARD:
	{
		if (vec.backward_capacity == 0)
		{
			int error = _grow_backward(vec);
			if (error != E_OK)
				return error;
		}

		vec.starting_chunk_index -= 1;

		chunk = vec.chunks + vec.starting_chunk_index;
		element = chunk->data + chunk->offset * vec.element_size;
	}
		break;
	}

	return E_OK;
}

void* chunkvec_get(ChunkVec& vec, size_t index, int& error)
{
	if (index >= vec.size)
	{
		error = E_OUT_OF_RANGE;
		return nullptr;
	}

	size_t chunk_index;
	size_t atchunk_index;
	_translate_index(vec, index, chunk_index, atchunk_index);

	error = E_OK;
	return (vec.chunks + chunk_index)->data + atchunk_index * vec.element_size;
}

void* chunkvec_get(ChunkVec& vec, size_t chunk_index, size_t atchunk_index)
{
	return (vec.chunks + chunk_index)->data + atchunk_index * vec.element_size;
}

int chunkvec_remove(ChunkVec& vec, size_t index)
{
	if (index >= vec.size)
	{
		return E_OUT_OF_RANGE;
	}

	size_t chunk_index;
	size_t atchunk_index;
	_translate_index(vec, index, chunk_index, atchunk_index);

	VecChunk* chunk = vec.chunks + chunk_index;

	switch (chunk->direction)
	{
	case DIR_BACKWARD:
	{
		// shift right by 1 the part on left
		size_t right_index = atchunk_index;
		size_t left_index = chunk->offset;

		/*memcpybackward(
			chunk->data + (left_index + 1) * vec.element_size,
			chunk->data + left_index * vec.element_size,
			(right_index - left_index) * vec.element_size
		);*/
		memcpybackward(
			chunk->data + vec.element_size,
			chunk->data,
			right_index * vec.element_size
		);

		if (left_index > 0)
		{
			chunk->offset += 1;
			return E_OK;
		}
		
		size_t i = chunk_index;
		while (i > vec.starting_chunk_index)
		{
			i -= 1;

			VecChunk* prev = vec.chunks + i;
			VecChunk* next = vec.chunks + i + 1;

			// copy from previous chunk to next chunk
			
			// copy last element from prev chunk to first slot in next chunk
			memcpybackward(
				next->data,
				prev->data + (vec.chunk_size - 1) * vec.element_size,
				vec.element_size
			);

			// move data right by 1 in prev chunk
			memcpybackward(
				prev->data + vec.element_size,
				prev->data,
				(vec.chunk_size - 1) * vec.element_size
			);
		}

		(vec.chunks + vec.starting_chunk_index)->offset += 1;
	}
		break;
	case DIR_FORWARD:
	{
		// shift left by 1 the part on right
		VecChunk* chunk = vec.chunks + chunk_index;
		
		//if (atchunk_index < vec.chunk_size - 1)
		memcpy(
			chunk->data + atchunk_index * vec.element_size,
			chunk->data + (atchunk_index + 1) * vec.element_size,
			(vec.chunk_size - atchunk_index - 1) * vec.element_size
		);

		if (vec.size > index + 1)
		{
			size_t i = chunk_index + 1;
			while (i < vec.backward_capacity + vec.forward_capacity)
			{
				VecChunk* prev = vec.chunks + i - 1;
				VecChunk* next = vec.chunks + i;

				// copy first element of next to prev last element
				memcpy(
					prev->data + (vec.chunk_size - 1) * vec.element_size,
					next->data,
					vec.element_size
				);

				// move to left by 1 in next chunk
				memcpy(
					next->data,
					next->data + vec.element_size,
					(vec.chunk_size - 1) * vec.element_size
				);

				i += 1;
			}

			_translate_index(vec, vec.size - 1, chunk_index, atchunk_index);
			chunk = vec.chunks + chunk_index;
			chunk->offset -= 1;
		}
		else chunk->offset -= 1;
	}
		break;
	}

	vec.size -= 1;

	return E_OK;

	/*if (chunk_index == vec.starting_chunk_index)
	{
		// shift right by 1 element
		VecChunk* chunk = vec.chunks + chunk_index;

		uint8_t* src = chunk->data + chunk->offset * vec.element_size;
		uint8_t* dst = src + vec.element_size;
		size_t size = (atchunk_index - chunk->offset) * vec.element_size;
		memcpybackward(dst, src, size);
	}
	else
	{
		// what to copy?
		// chunk at index -> move to left after index
		// next chunks -> move first element to last element in previous chunk
		//                and memcpy after first element to shift left by 1

		VecChunk* chunk = vec.chunks + chunk_index;
		memcpy(
			chunk->data + atchunk_index * vec.element_size,
			chunk->data + (atchunk_index + 1) * vec.element_size,
			(chunk->offset - atchunk_index) * vec.element_size
		);

		size_t items_left = vec.size - index;
		VecChunk* prev = chunk;
		VecChunk* end = vec.chunks + vec.backward_capacity + vec.forward_capacity;
		chunk += 1;
		while (chunk < end && items_left > 0)
		{
			memcpy(
				prev->data + (vec.chunk_size - 1) * vec.element_size,
				chunk->data,
				vec.element_size
			);

			memcpy(
				chunk->data,
				chunk->data + vec.element_size,
				vec.element_size * (vec.chunk_size - 1)
			);

			items_left -= vec.chunk_size;
			chunk += 1;
		}
	}

	return E_OK;*/
}

void chunkvec_remove_last(ChunkVec& vec)
{
	if (vec.size == 0)
		return;

	chunkvec_remove(vec, vec.size - 1);
}

void chunkvec_reset_size(ChunkVec& vec, size_t new_lower_size)
{
	if (new_lower_size > vec.size)
		return;

	// reset all chunks above new_lower_size
	size_t chunk_index;
	size_t atchunk_index;
	_translate_index(vec, new_lower_size, chunk_index, atchunk_index);

	VecChunk* chunk = vec.chunks + chunk_index;
	
	// structure can have 1 backward chunk at the time
	chunk->offset = atchunk_index;

	// assuming next chunks are forward chunks
	VecChunk* end = vec.chunks + vec.backward_capacity + vec.forward_capacity;
	chunk += 1;
	while (chunk < end)
	{
		chunk->offset = 0;
		chunk->direction = DIR_FORWARD;

		chunk += 1;
	}
}

/*int chunkvec_emplace_front(ChunkVec& vec, void*& element)
{
	size_t zero = vec.starting_chunk_index;
	VecChunk* zero_chunk = vec.chunks + zero;
	
	if (zero_chunk->offset > 0)
	{
		zero_chunk->offset -= 1;
		element = zero_chunk->data + zero_chunk->offset * vec.element_size;
	}
	else
	{
		// require one chunk on left
		if (vec.backward_capacity == 0)
		{
			int error = _grow_backward(vec);
			if (error != E_OK)
				return error;
		}

		// take one chunk on left
		vec.backward_capacity -= 1;
		vec.forward_capacity += 1;
		vec.starting_chunk_index -= 1;

		VecChunk* chunk = vec.chunks + vec.starting_chunk_index;
		chunk->offset -= 1;
		element = chunk->data + chunk->offset * vec.element_size;
	}

	vec.size += 1;

	return E_OK;
}*/
