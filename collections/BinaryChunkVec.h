#pragma once

#include "ChunkVec.h"
#include "hash/hash.h"

struct BinaryChunkVec
{
	ChunkVec chvec;
	int(*cmpfunc)(void*, void*, size_t);
};

int create_bcvec(BinaryChunkVec& vec, size_t element_size, size_t chunk_size = 16, size_t backward_capacity = 0, size_t forward_capacity = 0, int(*cmpfunc)(void*, void*, size_t) = cmpfunc_default);
void destroy_bcvec(BinaryChunkVec& vec);

int bcvec_insert(BinaryChunkVec& vec, void* element);

void* bcvec_get(BinaryChunkVec& vec, size_t index, int& error);
void* bcvec_find(BinaryChunkVec& vec, void* element, int& error);
int bcvec_remove(BinaryChunkVec& vec, size_t index);
int bcvec_remove(BinaryChunkVec& vec, void* element);