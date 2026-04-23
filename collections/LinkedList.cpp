#include "LinkedList.h"

#include "../error.h"

#include <stdlib.h>
#include <string.h>

static inline void _init_node(LinkedListNode* node)
{
	node->data = nullptr;
	node->next = nullptr;
	node->prev = nullptr;
}

static inline int _alloc_node(LinkedList& list, LinkedListNode*& new_node)
{
	new_node = (LinkedListNode*)malloc(sizeof(LinkedListNode));
	_init_node(new_node);
	new_node->data = malloc(list.element_size);
	if (new_node->data == nullptr)
	{
		free(new_node);
		return E_ERRNO;
	}

	return E_OK;
}

static inline void _free_node(LinkedListNode* node)
{
	if(node->data != nullptr)
		free(node->data);

	free(node);
}

static LinkedListNode* _linkedlist_get(LinkedList& list, size_t index)
{
	LinkedListNode* current_node;
	size_t i;

	// pick shorter traversal
	size_t mid_i = list.size % 2 == 0 ? (list.size / 2 - 1) : (list.size / 2);
	if (index < mid_i)
	{
		current_node = list.head->next;
		i = 0;
		while (i < index)
		{
			current_node = current_node->next;
			i += 1;
		}
	}
	else
	{
		current_node = list.tail->prev;
		i = list.size - 1;
		while (i > index)
		{
			current_node = current_node->prev;
			i -= 1;
		}
	}

	return current_node;
}

int create_linkedlist(LinkedList& list, size_t element_size)
{
	list.size = 0;
	list.element_size = element_size;
	list.head = nullptr;
	list.tail = nullptr;

	LinkedListNode* head = (LinkedListNode*)malloc(sizeof(LinkedListNode));
	if (head == nullptr)
		return E_ERRNO;

	LinkedListNode* tail = (LinkedListNode*)malloc(sizeof(LinkedListNode));
	if (tail == nullptr)
	{
		free(head);
		return E_ERRNO;
	}

	_init_node(head);
	_init_node(tail);

	head->next = tail;
	tail->prev = head;

	list.head = head;
	list.tail = tail;

	return E_OK;
}

void destroy_linkedlist(LinkedList& list)
{
	LinkedListNode* node = list.head;
	LinkedListNode* prev_node = nullptr;
	LinkedListNode* end_node = list.tail;

	while (node != end_node)
	{
		prev_node = node;
		node = node->next;

		_free_node(prev_node);
	}

	//free(list.head);
	free(list.tail);

	list.head = nullptr;
	list.tail = nullptr;
}

int linkedlist_push_back(LinkedList& list, void* element)
{
	int error;
	void* ptr = linkedlist_emplace_back(list, error);
	if (ptr == nullptr)
		return error;

	memcpy(ptr, element, list.element_size);

	return E_OK;
}

int linkedlist_push_front(LinkedList& list, void* element)
{
	int error;
	void* ptr = linkedlist_emplace_front(list, error);
	if (ptr == nullptr)
		return error;

	memcpy(ptr, element, list.element_size);

	return E_OK;
}

int linkedlist_insert(LinkedList& list, void* element, size_t index)
{
	int error;
	void* ptr = linkedlist_emplace(list, index, error);
	if (ptr == nullptr)
		return error;

	memcpy(ptr, element, list.element_size);

	return E_OK;
}

void* linkedlist_emplace_back(LinkedList& list, int& error)
{
	LinkedListNode* new_node;
	error = _alloc_node(list, new_node);
	if (error != E_OK)
		return nullptr;

	LinkedListNode* prev = list.tail->prev;

	new_node->prev = prev;
	new_node->next = list.tail;

	prev->next = new_node;
	list.tail->prev = new_node;

	list.size += 1;

	return new_node->data;
}

void* linkedlist_emplace_front(LinkedList& list, int& error)
{
	LinkedListNode* new_node;
	error = _alloc_node(list, new_node);
	if (error != E_OK)
		return nullptr;

	LinkedListNode* next = list.head->next;
	
	list.head->next = new_node;
	next->prev = new_node;
	new_node->prev = list.head;
	new_node->next = next;

	list.size += 1;

	return new_node->data;
}

void* linkedlist_emplace(LinkedList& list, size_t index, int& error)
{
	if (index >= list.size)
	{
		error = E_OUT_OF_RANGE;
		return nullptr;
	}

	LinkedListNode* new_node;
	error = _alloc_node(list, new_node);
	if (error != E_OK)
		return nullptr;

	LinkedListNode* i_node = _linkedlist_get(list, index);

	// insert

	LinkedListNode* prev = i_node->prev;
	LinkedListNode* next = i_node;

	// insert at i moving element at i upward

	prev->next = new_node;
	next->prev = new_node;
	new_node->prev = prev;
	new_node->next = next;

	list.size += 1;

	return new_node->data;
}

void* linkedlist_get(LinkedList& list, size_t index)
{
	if (index >= list.size)
	{
		return nullptr;
	}

	return _linkedlist_get(list, index)->data;
}

int linkedlist_remove(LinkedList& list, size_t index)
{
	if (index >= list.size)
	{
		return E_OUT_OF_RANGE;
	}

	LinkedListNode* i_node = _linkedlist_get(list, index);

	// short pointers

	LinkedListNode* prev = i_node->prev;
	LinkedListNode* next = i_node->next;

	prev->next = next;
	next->prev = prev;

	_free_node(i_node);

	return E_OK;
}