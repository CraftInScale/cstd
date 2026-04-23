#include "HashMap.h"

#include "../error.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

#define REHASH_LOAD_FACTOR 0.8

enum HashMapControl : int8_t
{
	kEmpty = -128, // 0b1000_0000
	kDeleted = -2  // 0b1111_1110
	// kFull =        0b0xxx_xxxx
};

static inline uint8_t H2(size_t hash)
{
	return hash & 0x7F;
}

static inline size_t H1(size_t hash)
{
	return hash >> 7;
}

#if HGROUP_SIZE == 16

static int _match(uint8_t hash, uint8_t* meta)
{
	// lower bits are first entries, little-endian

	__m128i line = _mm_set1_epi8(hash);
	int bits = _mm_movemask_epi8(_mm_cmpeq_epi8(line, *(__m128i*)meta));
	return bits;
}

static int _match_empty(uint8_t* meta)
{
	__m128i line = _mm_set1_epi8(kEmpty);
	int bits = _mm_movemask_epi8(_mm_cmpeq_epi8(line, *(__m128i*)meta));
	return bits;
}

static int _match_deleted(uint8_t* meta)
{
	__m128i line = _mm_set1_epi8(kDeleted);
	uint16_t bits = _mm_movemask_epi8(_mm_cmpeq_epi8(line, *(__m128i*)meta));
	return bits;
}

#elif HGROUP_SIZE == 32

static int _match(uint8_t hash, uint8_t* meta)
{
	__m256i line = _mm256_set1_epi8(hash);
	int bits = _mm256_movemask_epi8(
		_mm256_cmpeq_epi8(line, *(__m256i*)meta)
	);
	return bits;
}

static int _match_empty(uint8_t* meta)
{
	__m256i line = _mm256_set1_epi8(kEmpty);
	int bits = _mm256_movemask_epi8(
		_mm256_cmpeq_epi8(line, *(__m256i*)meta)
	);
	return bits;
}

static int _match_deleted(uint8_t* meta)
{
	__m256i line = _mm256_set1_epi8(kDeleted);
	int bits = _mm256_movemask_epi8(
		_mm256_cmpeq_epi8(line, *(__m256i*)meta)
	);
	return bits;
}

#endif

static size_t _get_data_count(HashMap& map)
{
	size_t count = map.capacity / map.data_vec_size;
	if (count * map.data_vec_size < map.capacity)
		count += 1;

	return count;
}

static void _init_group(HashMap& map, size_t group_index)
{
	constexpr size_t group_size = sizeof(HashGroup);

	HashGroup* meta = map.meta + group_index;
	HashGroupIndirections* indirections = map.indirections + group_index;

	//bzero(meta, group_size);
	static uint8_t empty[32] = {
		(uint8_t)kEmpty,
		(uint8_t)kEmpty,
		(uint8_t)kEmpty,
		(uint8_t)kEmpty,

		(uint8_t)kEmpty,
		(uint8_t)kEmpty,
		(uint8_t)kEmpty,
		(uint8_t)kEmpty,

		(uint8_t)kEmpty,
		(uint8_t)kEmpty,
		(uint8_t)kEmpty,
		(uint8_t)kEmpty,

		(uint8_t)kEmpty,
		(uint8_t)kEmpty,
		(uint8_t)kEmpty,
		(uint8_t)kEmpty,

		(uint8_t)kEmpty,
		(uint8_t)kEmpty,
		(uint8_t)kEmpty,
		(uint8_t)kEmpty,

		(uint8_t)kEmpty,
		(uint8_t)kEmpty,
		(uint8_t)kEmpty,
		(uint8_t)kEmpty,

		(uint8_t)kEmpty,
		(uint8_t)kEmpty,
		(uint8_t)kEmpty,
		(uint8_t)kEmpty,

		(uint8_t)kEmpty,
		(uint8_t)kEmpty,
		(uint8_t)kEmpty,
		(uint8_t)kEmpty
	};
	memcpy(meta->state, empty, group_size);

	bzero(indirections, sizeof(HashGroupIndirections));

	/*Vec* vec = set.data + group_index;

	int error = create_vec(*vec, set.element_size, set.data_vec_size);
	if (error != E_OK)
		return error;*/

	//return E_OK;
}

static int _init_vec(HashMap& map, size_t index)
{
	Vec* vec = map.data + index;

	int error = create_vec(*vec, map.key_size + map.value_size, map.data_vec_size);
	if (error != E_OK)
		return error;

	vec->size = map.data_vec_size;

	return E_OK;
}

static int _hashmap_grow(HashMap& map)
{
	size_t new_size = map.capacity * 2;
	if (new_size / 2 != map.capacity)
		return E_SIZE_OVERFLOW;

	/*size_t new_size_count = new_size / sizeof(HashGroup);
	size_t data_new_size = new_size_count * sizeof(Vec);
	if (data_new_size / sizeof(Vec) != new_size_count)
	{
		return E_SIZE_OVERFLOW;
	}*/

	size_t new_data_count = new_size / map.data_vec_size;
	if (new_data_count * map.data_vec_size < new_size)
	{
		if (new_data_count < __SIZE_MAX__)
			new_data_count += 1;
		else return E_SIZE_OVERFLOW;
	}

	size_t data_new_size = new_data_count * sizeof(Vec);
	if (data_new_size / sizeof(Vec) != new_data_count)
		return E_SIZE_OVERFLOW;

	size_t new_indir_count = new_size / sizeof(HashGroup);
	size_t new_indir_size = new_indir_count * sizeof(HashGroupIndirections);
	if (new_indir_size / sizeof(HashGroupIndirections) != new_indir_count)
		return E_SIZE_OVERFLOW;

	size_t prev_size = map.capacity;

	void* meta = nullptr;// realloc(set.meta, new_size);
	int error = posix_memalign(&meta, sizeof(HashGroup), new_size);
	if (meta == nullptr)
	{
		errno = error;
		return E_ERRNO;
	}
	free(map.meta);

	map.meta = (HashGroup*)meta;

	void* indirections = realloc(map.indirections, new_indir_size);
	if (indirections == nullptr)
	{
		return E_ERRNO;
	}

	map.indirections = (HashGroupIndirections*)indirections;

	void* data = realloc(map.data, data_new_size);
	if (data == nullptr)
	{
		return E_ERRNO;
	}

	map.data = (Vec*)data;

	// init all
	/*size_t i_min = set.capacity / sizeof(HashGroup);
	size_t i_max = new_size / sizeof(HashGroup);
	size_t i = i_min;
	while (i < i_max)
	{
		_init_group(set, i);
		i += 1;
	}*/
	
	size_t count = map.capacity / sizeof(HashGroup);
	size_t i_max = new_size / sizeof(HashGroup);
	size_t i = count;
	while (i < i_max)
	{
		_init_group(map, i);

		i += 1;
	}
	//bzero(set.meta + count, new_size - prev_size);
	bzero(map.indirections + count, count * sizeof(HashGroupIndirections));

	size_t i_min = _get_data_count(map);
	i_max = new_data_count;
	i = i_min;
	while (i < i_max)
	{
		error = _init_vec(map, i);
		if (error != E_OK)
		{
			while (i >= i_min)
			{
				i -= 1;
				destroy_vec(*(map.data + i));
			}
			return error;
		}

		i += 1;
	}

	map.capacity = new_size;

	return E_OK;
}

static int _hashmap_add_free_indexes(HashMap& map, size_t from_capacity)
{
	//vec_reserve(set.free_indexes, set.capacity - from_capacity);

	size_t prev_size = map.free_indexes.chvec.size;
	size_t i = from_capacity;
	int error;
	while (i < map.capacity)
	{
		error = bcvec_insert(map.free_indexes, &i);
		if (error != E_OK)
		{
			//set.free_indexes.chvec.size = prev_size;
			//chunkvec_reset_size(set.free_indexes.chvec, prev_size);
			// no rollback
			while (i > from_capacity)
			{
				i -= 1;
				bcvec_remove(map.free_indexes, &i);
			}
			return error;
		}

		i += 1;
	}

	return E_OK;
}

static int _hashmap_rehash(HashMap& map)
{
	size_t prev_cap = map.capacity;

	// first grow times 2 in size
	int error = _hashmap_grow(map);
	if (error != E_OK)
		return error;

	// reset current state to 0

	error = _hashmap_add_free_indexes(map, prev_cap);
	if (error != E_OK)
		return error;

	//bzero(set.meta, set.capacity / 2);

	size_t i = 0;
	size_t i_max = map.capacity / sizeof(HashGroup);
	while (i < i_max)
	{
		_init_group(map, i);
		i += 1;
	}

	bzero(map.indirections, ((map.capacity / 2) / sizeof(HashGroup)) * sizeof(HashGroupIndirections));

	// use set.data to set indirections pointers and meta

	/*size_t i = 0;
	size_t count = _get_data_count(set);
	while (i < count)
	{
		Vec* vec = set.data + i;
		size_t j = 0;
		while (j < vec->size)
		{
			void* element = vec_get(*vec, j, error);// assuming no error

			size_t hash = set.hfunc(element, set.element_size);

			size_t h1 = H1(hash);
			size_t group_count = set.capacity / sizeof(HashGroup);
			size_t index = h1 % group_count;
			size_t end_index = index;

			HashGroup* group = set.meta + index;
			int match;
			do
			{
				match = 0;
				match = _match_empty(group->state);
				if (match != 0)
				{
					int k = __bsfd(match);
					group->state[k] = H2(hash);
					
					HashGroupIndirections* indir = set.indirections + index;
					indir->pointers[k] = (size_t)element;

					break;
				}

				index = (index + 1) % group_count;
			} 
			while (index != end_index);

			j += 1;
		}

		i += 1;
	}*/

	i = 0;
	size_t j;
	size_t index, end_index;
	while (i < map.occupied_indexes.size)
	{
		j = *(size_t*)vec_get(map.occupied_indexes, i, error);// assuming no error

		// convert index to data index
		size_t data_index = j / map.data_vec_size;
		size_t elm_index = j - data_index * map.data_vec_size;

		Vec* vec = map.data + data_index;
		void* element = vec->data + elm_index * (map.key_size + map.value_size);

		size_t hash = map.hfunc(element, map.key_size);

		size_t h1 = H1(hash);
		size_t group_count = map.capacity / sizeof(HashGroup);
		index = h1 % group_count;
		end_index = index;

		HashGroup* group;
		int match;
		do
		{
			group = map.meta + index;

			match = 0;
			match = _match_empty(group->state);
			if (match != 0)
			{
				int k = __bsfd(match);
				group->state[k] = H2(hash);

				HashGroupIndirections* indir = map.indirections + index;
				indir->pointers[k] = (size_t)element;
				indir->indexes[k] = j;

				break;
			}

			index = (index + 1) % group_count;
		}
		while (index != end_index);

		i += 1;
	}

	return E_OK;
}

struct FindResult
{
	bool found;
	size_t group_index;
	size_t atgroup_index;
};

static void _hashmap_find(HashMap& map, size_t hash, void* key, FindResult& result)
{
	result.found = false;

	size_t group_count = map.capacity / sizeof(HashGroup);
	size_t group_index = H1(hash) % group_count;
	size_t end_index = group_index;
	
	uint8_t h2 = H2(hash);

	int match;
	do
	{
		// find element with matching hash
		// stop on empty entry

		uint8_t* state = (map.meta + group_index)->state;
		match = 0;
		match = _match(h2, state);
		if (match != 0)
		{
			//logger_log("match > 0");

			// found H2 part

			int i_low = __bsfd(match);
			int i_high = __bsrd(match);

			while (i_low <= i_high)
			{
				if ((match & (1 << i_low)) > 0)
				{
					HashGroupIndirections* indirections = map.indirections + group_index;
					void* elm = *(void**)(indirections->pointers + i_low);

					if (map.cmpfunc(elm, key, map.key_size) == 0)
					{
						result.found = true;
						result.group_index = group_index;
						result.atgroup_index = i_low;
						return;
					}
				}

				i_low += 1;
			}
		}

		// from group_index down find first empty entry
		match = _match_empty(state);
		if (match != 0)
			break;

		group_index = (group_index + 1) % group_count;
	}
	while (group_index != end_index);
}

int create_hashmap(HashMap& map, size_t key_size, size_t value_size, size_t(*hfunc)(void*, size_t), int(*cmpfunc)(void*, void*, size_t))
{
	return create_hashmap(map, key_size, value_size, 0, 0, hfunc, cmpfunc);
}

int create_hashmap(HashMap& map, size_t key_size, size_t value_size, size_t data_vec_size, size_t capacity, size_t(*hfunc)(void*, size_t), int(*cmpfunc)(void*, void*, size_t))
{
	map.meta = nullptr;
	map.indirections = nullptr;
	map.data = nullptr;
	map.data_vec_size = 0;
	map.key_size = 0;
	map.value_size = 0;
	map.size = 0;
	map.deleted_size = 0;
	map.capacity = 0;
	map.hfunc = nullptr;
	map.cmpfunc = nullptr;

	if (capacity == 0)
		capacity = 1;

	// round up capacity to store at least capacity elements
	constexpr size_t group_size = sizeof(HashGroup);

	size_t group_count = capacity / group_size;
	if (group_count * group_size < capacity)
	{
		group_count += 1;
		capacity = group_count * group_size;// overwrite, required for (capacity / sizeof(HashGroup)) calculations
	}

	size_t meta_size = group_count * sizeof(HashGroup);
	if (meta_size / sizeof(HashGroup) != group_count)
		return E_SIZE_OVERFLOW;

	size_t indirections_size = group_count * sizeof(HashGroupIndirections);
	if (indirections_size / sizeof(HashGroupIndirections) != group_count)
		return E_SIZE_OVERFLOW;

	/*size_t data_size = group_count * sizeof(Vec);
	if (data_size / sizeof(Vec) != group_count)
		return E_SIZE_OVERFLOW;*/

	if (data_vec_size == 0)
		data_vec_size = sizeof(HashGroup);

	size_t data_count = capacity / data_vec_size;
	if (data_count * data_vec_size < capacity)
		data_count += 1;

	size_t data_size = data_count * sizeof(Vec);
	if (data_size / sizeof(Vec) != data_count)
		return E_SIZE_OVERFLOW;

	void* meta = nullptr;// malloc(meta_size);
	int error = posix_memalign(&meta, sizeof(HashGroup), meta_size);
	if (meta == nullptr)
	{
		errno = error;
		return E_ERRNO;
	}

	void* indirections = malloc(indirections_size);
	if (indirections == nullptr)
	{
		free(meta);
		return E_ERRNO;
	}

	void* data = malloc(data_size);
	if (data == nullptr)
	{
		free(meta);
		free(indirections);
		return E_ERRNO;
	}

	int err;
	err = create_bcvec(map.free_indexes, sizeof(size_t), sizeof(HashGroup), 1, capacity/sizeof(HashGroup));
	if (err != E_OK)
	{
		free(meta);
		free(indirections);
		free(data);
		return err;
	}

	err = create_vec(map.occupied_indexes, sizeof(size_t), capacity * REHASH_LOAD_FACTOR);
	if (err != E_OK)
	{
		free(meta);
		free(indirections);
		free(data);
		destroy_bcvec(map.free_indexes);
		return err;
	}

	map.meta = (HashGroup*)meta;
	map.indirections = (HashGroupIndirections*)indirections;
	map.data = (Vec*)data;
	map.data_vec_size = data_vec_size;
	map.key_size = key_size;
	map.value_size = value_size;
	map.capacity = meta_size;

	_hashmap_add_free_indexes(map, 0);

	size_t i = 0;
	while (i < data_count)
	{
		error = _init_vec(map, i);
		if (error != E_OK)
		{
			// rollback
			while (i > 0)
			{
				i -= 1;
				destroy_vec(*(map.data + i));
			}

			free(meta);
			free(indirections);
			free(data);
			destroy_bcvec(map.free_indexes);
			destroy_vec(map.occupied_indexes);

			map.meta = nullptr;
			map.indirections = nullptr;
			map.data = nullptr;
			map.capacity = 0;

			return error;
		}

		i += 1;
	}

	i = 0;
	while (i < group_count)
	{
		_init_group(map, i);

		i += 1;
	}

	map.hfunc = hfunc;
	map.cmpfunc = cmpfunc;

	return E_OK;
}

void destroy_hashmap(HashMap& map)
{
	if (map.meta != nullptr)
	{
		free(map.meta);
		map.meta = nullptr;
	}

	if (map.indirections != nullptr)
	{
		free(map.indirections);
		map.indirections = nullptr;
	}

	if (map.data != nullptr)
	{
		size_t i = _get_data_count(map);
		while (i > 0)
		{
			i -= 1;
			destroy_vec(*(map.data + i));
		}

		free(map.data);
		map.data = nullptr;
	}

	destroy_bcvec(map.free_indexes);
	destroy_vec(map.occupied_indexes);

	map.capacity = 0;
	map.size = 0;
	map.deleted_size = 0;
}

int hashmap_insert(HashMap& map, void* key, void* value)
{
	HashMapResult result;

	return hashmap_insert(map, key, value, result);
}

int hashmap_insert(HashMap& map, void* key, void* value, HashMapResult& map_result)
{
	map_result.present = false;
	int error;

	size_t hash = map.hfunc(key, map.key_size);

	FindResult result;
	_hashmap_find(map, hash, key, result);

	if (result.found)
	{
		map_result.present = true;

		HashGroupIndirections* indir = map.indirections + result.group_index;
		void* ptr = (void*)indir->pointers[result.atgroup_index];

		map_result.key = ptr;
		map_result.value = ((uint8_t*)ptr) + map.key_size;
		return E_OK;
	}

	size_t lf_cap = map.capacity * REHASH_LOAD_FACTOR;
	if (map.size + map.deleted_size >= lf_cap)
	{
		int err = _hashmap_rehash(map);
		if (err != E_OK)
		{
			error = err;
			return error;
		}
	}

	// insert

	size_t group_count = map.capacity / sizeof(HashGroup);
	size_t index = H1(hash) % group_count;
	size_t end_index = index;

	int match;
	uint8_t* state;
	do
	{
		state = (map.meta + index)->state;
		match = 0;
		match = _match_empty(state);
		bool deleted_match = false;
		if (match == 0)
		{
			match = _match_deleted(state);
			deleted_match = true;
		}

		if (match != 0)
		{
			int i = __bsfd(match);
			state[i] = H2(hash);

			// either free indexes and occupied indexes
			// can do a map of bit set or not and linear scan instead of 2 extra Vec
			// or keep size of set.data Vec to HashGroup size

			// what with removing elements - how to retrieve index of element from indirections

			// transform free index to occupied index
			//size_t k = set.free_indexes.size - 1;// todo pop front
			size_t k = 0;
			void* ptr = bcvec_get(map.free_indexes, k, error);
			if (error != E_OK)
			{
				return error;
			}

			error = vec_push_back(map.occupied_indexes, ptr);
			if (error != E_OK)
			{
				return error;
			}

			int j = *(int*)ptr;
			size_t data_index = j / map.data_vec_size;
			size_t atdata_index = j - data_index * map.data_vec_size;
			
			Vec* vec = (map.data + data_index);
			void* elm = vec->data + atdata_index * (map.key_size + map.value_size);

			memcpy(elm, key, map.key_size);
			memcpy(((uint8_t*)elm) + map.key_size, value, map.value_size);

			error = bcvec_remove(map.free_indexes, k);
			if (error != E_OK)
			{
				vec_remove(map.occupied_indexes, map.occupied_indexes.size - 1);
				return error;
			}

			HashGroupIndirections* indir = map.indirections + index;
			indir->pointers[i] = (size_t)elm;
			indir->indexes[i] = j;
			
			map.size += 1;

			map_result.present = true;
			map_result.key = elm;
			map_result.value = ((uint8_t*)elm) + map.key_size;

			if (deleted_match)
				map.deleted_size -= 1;

			return E_OK;
		}

		index = (index + 1) % group_count;
	}
	while (index != end_index);

	return E_OK;
}

void hashmap_get(HashMap& map, void* key, HashMapResult& map_result)
{
	map_result.present = false;

	size_t hash = map.hfunc(key, map.key_size);

	FindResult result;
	_hashmap_find(map, hash, key, result);

	if (result.found)
	{
		HashGroupIndirections* indir = map.indirections + result.group_index;
		void* ptr = (void*)indir->pointers[result.atgroup_index];

		map_result.present = true;
		map_result.key = ptr;
		map_result.value = ((uint8_t*)ptr) + map.key_size;
	}
}

int hashmap_remove(HashMap& map, void* key)
{
	size_t hash = map.hfunc(key, map.key_size);

	FindResult result;
	_hashmap_find(map, hash, key, result);

	if (result.found)
	{
		HashGroupIndirections* indir = map.indirections + result.group_index;
		size_t index = indir->indexes[result.atgroup_index];

		// find position in occupied_indexes of index
		// todo binary search and sorted Vec
		Vec* occ_indexes = &map.occupied_indexes;
		size_t i = 0;
		int error;
		bool found = false;
		while (i < occ_indexes->size)
		{
			size_t* ptr = (size_t*)vec_get(*occ_indexes, i, error);// assuming no error
			if (error != E_OK)
				return error;

			if (*ptr == index)
			{
				found = true;
				break;
			}

			i += 1;
		}

		HashGroup* group = map.meta + result.group_index;

		if (!found)
		{
			group->state[result.atgroup_index] = kDeleted;
			return E_OK;
		}

		error = vec_remove(*occ_indexes, i);
		if (error != E_OK)
			return error;

		error = bcvec_insert(map.free_indexes, &index);
		if (error != E_OK)
			return error;

		group->state[result.atgroup_index] = kDeleted;
		map.size -= 1;
		map.deleted_size += 1;
	}

	return E_OK;
}

void init_hashmap_iter(HashMapIter& iter, HashMap* map)
{
	iter.map = map;
	iter.occupy_index = 0;
}

int hashmap_iter_next(HashMapIter& iter, HashMapResult& result)
{
	result.present = false;

	size_t occupy_index = iter.occupy_index;
	if (occupy_index >= iter.map->occupied_indexes.size)
		return E_OK;

	int error;
	void* ptr = vec_get(iter.map->occupied_indexes, occupy_index, error);
	if (error != E_OK)
		return error;

	size_t index = *(size_t*)ptr;

	size_t data_index = index / iter.map->data_vec_size;
	size_t atdata_index = index - data_index * iter.map->data_vec_size;
	Vec* vec = iter.map->data + data_index;
	ptr = vec->data + atdata_index * (iter.map->key_size + iter.map->value_size);

	result.present = true;
	result.key = ptr;
	result.value = ((uint8_t*)ptr) + iter.map->key_size;

	iter.occupy_index += 1;

	return E_OK;
}
