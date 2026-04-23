#pragma once

#include "hash/hash.h"
#include "Vec.h"
#include "BinaryChunkVec.h"

#include <stdint.h>
#include <stddef.h>

struct HashMapResult
{
	bool present;
	void* key;
	void* value;
};

struct HashMap
{
	HashGroup* meta;
	HashGroupIndirections* indirections;
	Vec* data;// each Vec holds either 16 or 32 elements/entries, or data_vec_size
	size_t data_vec_size;// size in elements of single Vec in data
	size_t key_size;// in bytes
	size_t value_size;// in bytes
	size_t size;// element count
	size_t deleted_size;// to count for load factor
	size_t capacity;// entries count

	BinaryChunkVec free_indexes;// todo LIFO Queue (for cache-locality friendly use LIFO so latest element accessed would be in cache)
	                 // Vec with push_back and "pop_back" is already LIFO Queue
	Vec occupied_indexes;// cannot be reset due to same-address keeping guarantee

	size_t(*hfunc)(void*, size_t);
	int(*cmpfunc)(void*, void*, size_t);
};

int create_hashmap(HashMap& map, size_t key_size, size_t value_size, size_t(*hfunc)(void*, size_t) = hfunc_murmur3, int(*cmpfunc)(void*, void*, size_t) = cmpfunc_default);

/*
* @param data_vec_size set to 0 for auto value
*/
int create_hashmap(HashMap& map, size_t key_size, size_t value_size, size_t data_vec_size, size_t capacity = 0, size_t(*hfunc)(void*, size_t) = hfunc_murmur3, int(*cmpfunc)(void*, void*, size_t) = cmpfunc_default);
void destroy_hashmap(HashMap& map);

int hashmap_insert(HashMap& map, void* key, void* value);
int hashmap_insert(HashMap& map, void* key, void* value, HashMapResult& result);
void hashmap_get(HashMap& map, void* key, HashMapResult& result);
int hashmap_remove(HashMap& map, void* key);

struct HashMapIter
{
	HashMap* map;
	size_t occupy_index;
};

void init_hashmap_iter(HashMapIter& iter, HashMap* map);
// check result.present == true for loop continuation, returned error shouldn't be indicator of continuation
int hashmap_iter_next(HashMapIter& iter, HashMapResult& result);
