#include "BinaryChunkVec.h"

#include "../error.h"

int create_bcvec(BinaryChunkVec& vec, size_t element_size, size_t chunk_size, size_t backward_capacity, size_t forward_capacity, int(*cmpfunc)(void*, void*, size_t))
{
	int error = create_chunkvec(vec.chvec, element_size, chunk_size, backward_capacity, forward_capacity);
	if (error != E_OK)
		return error;

	vec.cmpfunc = cmpfunc;

	return E_OK;
}

void destroy_bcvec(BinaryChunkVec& vec)
{
	destroy_chunkvec(vec.chvec);
}

int bcvec_insert(BinaryChunkVec& vec, void* element)
{
	if (vec.chvec.size == 0)
	{
		return chunkvec_push_back(vec.chvec, element);
	}

	// binary search

	size_t L = 0;
	size_t R = vec.chvec.size - 1;
	size_t M;
	int error;
	while (L <= R)
	{
		M = L + (R - L) / 2;
		/*
		
		1 -> 0
		2 -> 1
		4 -> 2

		always lower
		
		*/

		void* M_elm = chunkvec_get(vec.chvec, M, error);
		if (error != E_OK)
			return error;

		int cmp = vec.cmpfunc(M_elm, element, vec.chvec.element_size);
		if (cmp <= -1)// M < elm
		{
			L = M + 1;
		}
		else if (cmp >= 1)// M > elm
		{
			R = M - 1;
			if (M == 0)
				break;
		}
		else
		{
			//size_t i = M;
			//while (i+1 < vec.chvec.size)
			//{
			//	i += 1;
			//	M_elm = chunkvec_get(vec.chvec, M, error);
			//	if (error != E_OK)
			//		return error;

			//	cmp = vec.cmpfunc(M_elm, element, vec.chvec.size);
			//	if (cmp != 0)
			//	{
			//		i -= 1;

			//	}
			//}

			//return chunkvec_insert(vec.chvec, M, element);
			L = M;
			break;
		}
	}

	return chunkvec_insert(vec.chvec, L, element);
}

void* bcvec_get(BinaryChunkVec& vec, size_t index, int& error)
{
	return chunkvec_get(vec.chvec, index, error);
}

void* bcvec_find(BinaryChunkVec& vec, void* element, int& error)
{
	error = E_OK;

	if (vec.chvec.size == 0)
	{
		return nullptr;
	}

	// binary search

	size_t L = 0;
	size_t R = vec.chvec.size - 1;
	size_t M;
	while (L <= R)
	{
		M = L + (R - L) / 2;

		void* M_elm = chunkvec_get(vec.chvec, M, error);
		if (error != E_OK)
			return nullptr;

		int cmp = vec.cmpfunc(M_elm, element, vec.chvec.element_size);
		if (cmp <= -1)// M < elm
		{
			L = M + 1;
		}
		else if (cmp >= 1)// M > elm
		{
			R = M - 1;
			if (M == 0)
				break;
		}
		else
		{
			return bcvec_get(vec, M, error);
		}
	}

	return E_OK;
}

int bcvec_remove(BinaryChunkVec& vec, size_t index)
{
	return chunkvec_remove(vec.chvec, index);
}

int bcvec_remove(BinaryChunkVec& vec, void* element)
{
	if (vec.chvec.size == 0)
	{
		return E_OK;
	}

	// binary search

	size_t L = 0;
	size_t R = vec.chvec.size - 1;
	size_t M;
	int error;
	while (L <= R)
	{
		M = L + (R - L) / 2;

		void* M_elm = chunkvec_get(vec.chvec, M, error);
		if (error != E_OK)
			return error;

		int cmp = vec.cmpfunc(M_elm, element, vec.chvec.element_size);
		if (cmp <= -1)// M < elm
		{
			L = M + 1;
		}
		else if (cmp >= 1)// M > elm
		{
			R = M - 1;
			if (M == 0)
				break;
		}
		else
		{
			chunkvec_remove(vec.chvec, M);
			break;
		}
	}

	return E_OK;
}

void bcvec_reset_size(BinaryChunkVec& vec, size_t new_lower_size)
{
	chunkvec_reset_size(vec.chvec, new_lower_size);
}
