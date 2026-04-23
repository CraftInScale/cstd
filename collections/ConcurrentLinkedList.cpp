#include "ConcurrentLinkedList.h"

#include "../error.h"

#include <stdlib.h>
#include <string.h>

static inline void _init_node(ConcurrentLinkedListNode* node)
{
	create_mutex(node->mutex);
	node->data = nullptr;
	node->next = nullptr;
	node->prev = nullptr;
}

static inline int _alloc_node(ConcurrentLinkedList& list, ConcurrentLinkedListNode*& new_node)
{
	new_node = (ConcurrentLinkedListNode*)malloc(sizeof(ConcurrentLinkedListNode));
	if (new_node == nullptr)
		return E_ERRNO;

	_init_node(new_node);
	new_node->data = malloc(list.element_size);
	if (new_node->data == nullptr)
	{
		destroy_mutex(new_node->mutex);
		free(new_node);
		return E_ERRNO;
	}

	return E_OK;
}

static inline void _free_node(ConcurrentLinkedListNode* node)
{
	mutex_lock(node->mutex);

	if (node->data != nullptr)
		free(node->data);

	mutex_unlock(node->mutex);
	destroy_mutex(node->mutex);
	free(node);
}

// 2-lock hand-over-hand traversal to the data node at position 'index' (0-based).
// Returns with *out_prev and *out_curr both locked in forward order.
// Caller must unlock both when done.
static void _traverse_to(ConcurrentLinkedList& list, size_t index,
	ConcurrentLinkedListNode*& out_prev, ConcurrentLinkedListNode*& out_curr)
{
	ConcurrentLinkedListNode* prev = list.head;
	mutex_lock(prev->mutex);
	ConcurrentLinkedListNode* curr = list.head->next;
	mutex_lock(curr->mutex);

	for (size_t i = 0; i < index; i++)
	{
		ConcurrentLinkedListNode* next = curr->next;
		mutex_lock(next->mutex);
		mutex_unlock(prev->mutex);
		prev = curr;
		curr = next;
	}

	out_prev = prev;
	out_curr = curr;
}

int create_concurrent_linkedlist(ConcurrentLinkedList& list, size_t element_size)
{
	list.size.store(0);
	list.element_size = element_size;
	list.head = nullptr;
	list.tail = nullptr;

	ConcurrentLinkedListNode* head = (ConcurrentLinkedListNode*)malloc(sizeof(ConcurrentLinkedListNode));
	if (head == nullptr)
		return E_ERRNO;

	ConcurrentLinkedListNode* tail = (ConcurrentLinkedListNode*)malloc(sizeof(ConcurrentLinkedListNode));
	if (tail == nullptr)
	{
		free(head);
		return E_ERRNO;
	}

	_init_node(head);
	_init_node(tail);

	head->next = tail;
	head->prev = nullptr;
	tail->prev = head;
	tail->next = nullptr;

	list.head = head;
	list.tail = tail;

	return E_OK;
}

void destroy_concurrent_linkedlist(ConcurrentLinkedList& list)
{
	ConcurrentLinkedListNode* node = list.head;
	ConcurrentLinkedListNode* end_node = list.tail;

	while (node != end_node)
	{
		ConcurrentLinkedListNode* next = node->next;
		_free_node(node);
		node = next;
	}

	_free_node(list.tail);

	list.head = nullptr;
	list.tail = nullptr;
}

int concurrent_linkedlist_push_back(ConcurrentLinkedList& list, void* element)
{
	int error;
	void* ptr = concurrent_linkedlist_emplace_back(list, error);
	if (ptr == nullptr)
		return error;

	memcpy(ptr, element, list.element_size);
	return E_OK;
}

int concurrent_linkedlist_push_front(ConcurrentLinkedList& list, void* element)
{
	int error;
	void* ptr = concurrent_linkedlist_emplace_front(list, error);
	if (ptr == nullptr)
		return error;

	memcpy(ptr, element, list.element_size);
	return E_OK;
}

int concurrent_linkedlist_insert(ConcurrentLinkedList& list, void* element, size_t index)
{
	int error;
	void* ptr = concurrent_linkedlist_emplace(list, index, error);
	if (ptr == nullptr)
		return error;

	memcpy(ptr, element, list.element_size);
	return E_OK;
}

void* concurrent_linkedlist_emplace_back(ConcurrentLinkedList& list, int& error)
{
	ConcurrentLinkedListNode* new_node;
	error = _alloc_node(list, new_node);
	if (error != E_OK)
		return nullptr;

	// Must acquire last_node and tail in forward order (last_node before tail) to
	// match the head-to-tail ordering used by _traverse_to and avoid deadlock.
	// Read tail->prev as a hint, then lock both in forward order and verify.
	ConcurrentLinkedListNode* last_node;
	while (true)
	{
		mutex_lock(list.tail->mutex);
		last_node = list.tail->prev;
		mutex_unlock(list.tail->mutex);

		mutex_lock(last_node->mutex);  // forward order: last_node before tail
		mutex_lock(list.tail->mutex);

		if (list.tail->prev == last_node)
			break;

		// Another thread changed the tail end; retry with the new last node.
		mutex_unlock(list.tail->mutex);
		mutex_unlock(last_node->mutex);
	}

	new_node->prev = last_node;
	new_node->next = list.tail;
	last_node->next = new_node;
	list.tail->prev = new_node;

	list.size.fetch_add(1);

	mutex_unlock(last_node->mutex);
	mutex_unlock(list.tail->mutex);

	return new_node->data;
}

void* concurrent_linkedlist_emplace_front(ConcurrentLinkedList& list, int& error)
{
	ConcurrentLinkedListNode* new_node;
	error = _alloc_node(list, new_node);
	if (error != E_OK)
		return nullptr;

	// Lock head then head->next (naturally forward order, no deadlock risk).
	mutex_lock(list.head->mutex);
	ConcurrentLinkedListNode* first = list.head->next;
	mutex_lock(first->mutex);

	new_node->prev = list.head;
	new_node->next = first;
	list.head->next = new_node;
	first->prev = new_node;

	list.size.fetch_add(1);

	mutex_unlock(first->mutex);
	mutex_unlock(list.head->mutex);

	return new_node->data;
}

void* concurrent_linkedlist_emplace(ConcurrentLinkedList& list, size_t index, int& error)
{
	if (index >= list.size.load())
	{
		error = E_OUT_OF_RANGE;
		return nullptr;
	}

	ConcurrentLinkedListNode* new_node;
	error = _alloc_node(list, new_node);
	if (error != E_OK)
		return nullptr;

	ConcurrentLinkedListNode* prev;
	ConcurrentLinkedListNode* curr;
	_traverse_to(list, index, prev, curr);

	// Insert new_node before curr (at position 'index'), shifting curr to index+1.
	new_node->prev = prev;
	new_node->next = curr;
	prev->next = new_node;
	curr->prev = new_node;

	list.size.fetch_add(1);

	mutex_unlock(prev->mutex);
	mutex_unlock(curr->mutex);

	return new_node->data;
}

void* concurrent_linkedlist_get(ConcurrentLinkedList& list, size_t index)
{
	if (index >= list.size.load())
		return nullptr;

	ConcurrentLinkedListNode* prev;
	ConcurrentLinkedListNode* curr;
	_traverse_to(list, index, prev, curr);

	void* data = curr->data;

	mutex_unlock(prev->mutex);
	mutex_unlock(curr->mutex);

	return data;
}

int concurrent_linkedlist_remove(ConcurrentLinkedList& list, size_t index)
{
	if (index >= list.size.load())
		return E_OUT_OF_RANGE;

	ConcurrentLinkedListNode* prev;
	ConcurrentLinkedListNode* curr;
	_traverse_to(list, index, prev, curr);

	// Lock curr->next to update its prev pointer (still forward order).
	ConcurrentLinkedListNode* next = curr->next;
	mutex_lock(next->mutex);

	prev->next = next;
	next->prev = prev;

	list.size.fetch_sub(1);

	mutex_unlock(prev->mutex);
	mutex_unlock(next->mutex);

	// curr is now disconnected; no other thread can reach it, safe to free.
	mutex_unlock(curr->mutex);
	_free_node(curr);

	return E_OK;
}
