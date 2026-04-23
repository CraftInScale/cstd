#include "LinkedListIter.h"

void init_linkedlist_iter(LinkedListIter& iter, LinkedList* list)
{
	iter.list = list;
	iter.node = list->head->next;
}

void* linkedlist_iter_next(LinkedListIter& iter)
{
	LinkedListNode* node = iter.node;

	if (node == iter.list->tail)
		return nullptr;

	void* ret = node->data;

	iter.node = node->next;

	return ret;
}
