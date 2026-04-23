#pragma once

#include <stddef.h>

struct LinkedListNode
{
	void* data;
	LinkedListNode *next, *prev;
};

// Doubly-Linked List
struct LinkedList
{
	LinkedListNode *head, *tail;
	size_t size;
	size_t element_size;
};

int create_linkedlist(LinkedList& list, size_t element_size);
void destroy_linkedlist(LinkedList& list);

int linkedlist_push_back(LinkedList& list, void* element);
int linkedlist_push_front(LinkedList& list, void* element);
int linkedlist_insert(LinkedList& list, void* element, size_t index);

void* linkedlist_emplace_back(LinkedList& list, int& error);
void* linkedlist_emplace_front(LinkedList& list, int& error);
void* linkedlist_emplace(LinkedList& list, size_t index, int& error);

void* linkedlist_get(LinkedList& list, size_t index);
int linkedlist_remove(LinkedList& list, size_t index);
