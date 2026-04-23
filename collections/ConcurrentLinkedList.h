#pragma once

#include "../thread/Mutex.h"

#include <stddef.h>
#include <atomic>

struct ConcurrentLinkedListNode
{
	Mutex mutex;    // protects: data, next, prev
	void* data;
	ConcurrentLinkedListNode *next, *prev;
};

// Doubly-linked list with per-node mutex.
// Per-node mutexes protect node fields (data, next, prev) and enable safe
// concurrent iteration via hand-over-hand locking (see ConcurrentLinkedListIter).
// Structural modifications (insert/remove) acquire adjacent node mutexes in
// forward order; if concurrent insert+remove are needed, the caller must also
// hold an external list-level lock.
struct ConcurrentLinkedList
{
	ConcurrentLinkedListNode *head, *tail;
	std::atomic<size_t> size;
	size_t element_size;
};

int create_concurrent_linkedlist(ConcurrentLinkedList& list, size_t element_size);
void destroy_concurrent_linkedlist(ConcurrentLinkedList& list);

int concurrent_linkedlist_push_back(ConcurrentLinkedList& list, void* element);
int concurrent_linkedlist_push_front(ConcurrentLinkedList& list, void* element);
int concurrent_linkedlist_insert(ConcurrentLinkedList& list, void* element, size_t index);

void* concurrent_linkedlist_emplace_back(ConcurrentLinkedList& list, int& error);
void* concurrent_linkedlist_emplace_front(ConcurrentLinkedList& list, int& error);
void* concurrent_linkedlist_emplace(ConcurrentLinkedList& list, size_t index, int& error);

void* concurrent_linkedlist_get(ConcurrentLinkedList& list, size_t index);
int concurrent_linkedlist_remove(ConcurrentLinkedList& list, size_t index);
