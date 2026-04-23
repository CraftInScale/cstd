#pragma once

#include "LinkedList.h"

struct LinkedListIter
{
	LinkedList* list;
	LinkedListNode* node;
};

// list should remain same memory location until finished using this iter
void init_linkedlist_iter(LinkedListIter& iter, LinkedList* list);
void* linkedlist_iter_next(LinkedListIter& iter);