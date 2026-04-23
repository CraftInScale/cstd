#pragma once

#include <stdint.h>
#include <stddef.h>

enum VecChunkDirection : int
{
	DIR_BACKWARD = 0,
	DIR_FORWARD = 1
};

struct VecChunk
{
	uint8_t* data;
	size_t offset;
	VecChunkDirection direction;
};

/*
* Purpose of chunked Vec is to have fast push front
*/
struct ChunkVec
{
	VecChunk* chunks;
	
	size_t element_size;
	size_t chunk_size;
	
	size_t backward_capacity;
	size_t forward_capacity;
	
	size_t starting_chunk_index;// starting chunk is always backward
	size_t size;// element count

	bool access_only;
};

int create_chunkvec(ChunkVec& vec, size_t element_size, size_t chunk_size = 16, size_t backward_capacity = 0, size_t forward_capacity = 1);
void destroy_chunkvec(ChunkVec& vec);

int chunkvec_push_back(ChunkVec& vec, void* element);
int chunkvec_push_front(ChunkVec& vec, void* element);
int chunkvec_emplace_back(ChunkVec& vec, void*& element);
int chunkvec_emplace_front(ChunkVec& vec, void*& element);

int chunkvec_insert_inplace(ChunkVec& vec, size_t index, void*& element);
int chunkvec_insert(ChunkVec& vec, size_t index, void* element);

void* chunkvec_get(ChunkVec& vec, size_t index, int& error);
void* chunkvec_get(ChunkVec& vec, size_t chunk_index, size_t atchunk_index);
int chunkvec_remove(ChunkVec& vec, size_t index);
void chunkvec_remove_last(ChunkVec& vec);

void chunkvec_reset_size(ChunkVec& vec, size_t new_lower_size);