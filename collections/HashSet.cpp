#include "HashSet.h"

#include "../error.h"

int create_hashset(HashSet& set, size_t element_size, size_t(*hfunc)(void*, size_t), int(*cmpfunc)(void*, void*, size_t))
{
	return create_hashset(set, element_size, 0, 0, hfunc, cmpfunc);
}

int create_hashset(HashSet& set, size_t element_size, size_t data_vec_size, size_t capacity, size_t(*hfunc)(void*, size_t), int(*cmpfunc)(void*, void*, size_t))
{
	return create_hashmap(set.map, element_size, (size_t)0, data_vec_size, capacity, hfunc, cmpfunc);
}

void destroy_hashset(HashSet& set)
{
	destroy_hashmap(set.map);
}

void* hashset_insert(HashSet& set, void* element, int& error)
{
	HashMapResult result;
	error = hashmap_insert(set.map, element, nullptr, result);

	if (result.present)
		return result.key;

	return nullptr;
}

void* hashset_get(HashSet& set, void* element)
{
	HashMapResult result;
	hashmap_get(set.map, element, result);

	return result.present ? result.key : nullptr;
}

int hashset_remove(HashSet& set, void* element)
{
	return hashmap_remove(set.map, element);
}
