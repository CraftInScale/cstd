#pragma once

#include "HashMap.h"

#include <stdint.h>
#include <stddef.h>

struct HashSet
{
	HashMap map;
};

int create_hashset(HashSet& set, size_t element_size, size_t(*hfunc)(void*, size_t) = hfunc_murmur3, int(*cmpfunc)(void*, void*, size_t) = cmpfunc_default);

/*
* @param data_vec_size set to 0 for auto value
*/
int create_hashset(HashSet& set, size_t element_size, size_t data_vec_size, size_t capacity = 0, size_t(*hfunc)(void*, size_t) = hfunc_murmur3, int(*cmpfunc)(void*, void*, size_t) = cmpfunc_default);
void destroy_hashset(HashSet& set);

void* hashset_insert(HashSet& set, void* element, int& error);
void* hashset_get(HashSet& set, void* element);
int hashset_remove(HashSet& set, void* element);

// shrinks to 1/2 of capacity if load factor would be meet
// shrinks meta and indirections only
//int hashset_shrink(HashSet& set);
// not practical as capacity is used for set.data size calculation
// solution: second capacity variable that is higher or equal to meta capacity
//           shrinking means also rehash
// further opportunities: make free_indexes sorted and popping lowest index first so it can allow no occupied indexes above capacity/2
