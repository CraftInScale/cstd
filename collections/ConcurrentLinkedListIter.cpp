#include "ConcurrentLinkedListIter.h"

void init_concurrent_linkedlist_iter(ConcurrentLinkedListIter& iter, ConcurrentLinkedList* list)
{
	iter.list = list;

	// Hand-over-hand from head: lock head, lock head->next, release head.
	// This safely reads head->next under its mutex before exposing the first node.
	mutex_lock(list->head->mutex);
	ConcurrentLinkedListNode* first = list->head->next;
	mutex_lock(first->mutex);
	mutex_unlock(list->head->mutex);

	iter.node = first;
}

void* concurrent_linkedlist_iter_next(ConcurrentLinkedListIter& iter)
{
	ConcurrentLinkedListNode* node = iter.node;

	if (node == nullptr)
		return nullptr;

	if (node == iter.list->tail)
	{
		// Reached the end sentinel; release and signal completion.
		mutex_unlock(node->mutex);
		iter.node = nullptr;
		return nullptr;
	}

	void* data = node->data;

	// Hand-over-hand: lock next while still holding current, then release current.
	// Reading node->next is safe here since we hold node->mutex.
	ConcurrentLinkedListNode* next = node->next;
	mutex_lock(next->mutex);
	mutex_unlock(node->mutex);

	iter.node = next;

	return data;
}

void finish_concurrent_linkedlist_iter(ConcurrentLinkedListIter& iter)
{
	if (iter.node != nullptr)
	{
		mutex_unlock(iter.node->mutex);
		iter.node = nullptr;
	}
}
