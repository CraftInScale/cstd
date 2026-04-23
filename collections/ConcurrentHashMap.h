#pragma once

#include "hash/hash.h"
#include "Vec.h"
#include "BinaryChunkVec.h"
#include "../thread/Mutex.h"

#include <stdint.h>
#include <stddef.h>

struct ConcurrentHashMapResult
{
	bool present;
	void* key;
	void* value;
	Mutex* mutex;   // per-entry mutex; lock to safely read/modify value after the call returns
};

struct ConcurrentHashMap
{
	HashGroup* meta;
	HashGroupIndirections* indirections;
	Vec* data;          // each Vec holds data_vec_size entries of entry_size bytes each
	size_t data_vec_size;
	size_t key_size;    // in bytes
	size_t value_size;  // in bytes
	size_t entry_size;  // sizeof(Mutex) + key_size + value_size; stride of each slot in data Vec
	size_t size;
	size_t deleted_size;
	size_t capacity;

	BinaryChunkVec free_indexes;
	Vec occupied_indexes;   // cannot be reset due to same-address keeping guarantee

	size_t(*hfunc)(void*, size_t);
	int(*cmpfunc)(void*, void*, size_t);

	Mutex global_mutex; // acquired by write operations (insert, remove) for write-write serialisation
	Mutex read_mutex;   // acquired by reads (get, iter) and also by writes to block concurrent reads
};

int create_concurrent_hashmap(ConcurrentHashMap& map, size_t key_size, size_t value_size, size_t(*hfunc)(void*, size_t) = hfunc_murmur3, int(*cmpfunc)(void*, void*, size_t) = cmpfunc_default);

/*
* @param data_vec_size set to 0 for auto value
*/
int create_concurrent_hashmap(ConcurrentHashMap& map, size_t key_size, size_t value_size, size_t data_vec_size, size_t capacity = 0, size_t(*hfunc)(void*, size_t) = hfunc_murmur3, int(*cmpfunc)(void*, void*, size_t) = cmpfunc_default);
void destroy_concurrent_hashmap(ConcurrentHashMap& map);

int concurrent_hashmap_insert(ConcurrentHashMap& map, void* key, void* value);
int concurrent_hashmap_insert(ConcurrentHashMap& map, void* key, void* value, ConcurrentHashMapResult& result);
void concurrent_hashmap_get(ConcurrentHashMap& map, void* key, ConcurrentHashMapResult& result);
int concurrent_hashmap_remove(ConcurrentHashMap& map, void* key);

struct ConcurrentHashMapIter
{
	ConcurrentHashMap* map;
	size_t occupy_index;
};

// Acquires map.global_mutex. Must be paired with finish_concurrent_hashmap_iter.
void begin_concurrent_hashmap_iter(ConcurrentHashMapIter& iter, ConcurrentHashMap* map);
// check result.present == true for loop continuation, returned error shouldn't be indicator of continuation
int concurrent_hashmap_iter_next(ConcurrentHashMapIter& iter, ConcurrentHashMapResult& result);
// Releases map.global_mutex. Call when iteration is complete or on early exit.
void finish_concurrent_hashmap_iter(ConcurrentHashMapIter& iter);
