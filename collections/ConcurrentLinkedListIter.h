#pragma once

#include "ConcurrentLinkedList.h"

// Hand-over-hand iterator: always holds exactly one node's mutex between calls.
// This ensures safe traversal even if other threads concurrently insert or remove
// nodes — the held mutex prevents the current node's next/prev from being modified.
//
// Usage:
//   ConcurrentLinkedListIter iter;
//   init_concurrent_linkedlist_iter(iter, &list);
//   void* data;
//   while ((data = concurrent_linkedlist_iter_next(iter)) != nullptr) { ... }
//   // No finish call needed when iterating to completion.
//   // If breaking early, call finish_concurrent_linkedlist_iter to release the held mutex.
struct ConcurrentLinkedListIter
{
	ConcurrentLinkedList* list;
	ConcurrentLinkedListNode* node;  // currently locked node; nullptr when finished
};

// Acquires the first node's mutex. Must pair with finish_concurrent_linkedlist_iter
// if iteration does not run to completion.
void init_concurrent_linkedlist_iter(ConcurrentLinkedListIter& iter, ConcurrentLinkedList* list);

// Locks the next node, returns current node's data, unlocks current node.
// Returns nullptr at list end and releases the last held mutex automatically.
void* concurrent_linkedlist_iter_next(ConcurrentLinkedListIter& iter);

// Releases the currently held node mutex. Call when breaking out of iteration early.
void finish_concurrent_linkedlist_iter(ConcurrentLinkedListIter& iter);
