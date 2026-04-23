# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

# Project Overview

cstd is replacement library for C/C++ standard library.

# Files Overview

- ./collections/: data structures written from scratch
- ./string/: UTF-8 String implementation
- ./thread/: Thread and ThreadPool implementation with signaling shutdown of particular thread or entire threadpool (co-works with requirement of executing while(thread_check_if_running(thread)) in user thread's routine)
- ./time/: currently initial logic of time with Timestamp and DateTime (need to add parsing of date for YamlConfig purpose - part of Flux)
- ./error.h/cpp: errors used in function returning or function param reference if it's preferred to return some other value like pointer for iteration in loop

./collections/:
- Vec - works like std::vector, pads element size to closest upper power of 2
- ChunkVec - written for fast push_front, allows for keeping same elements address
- BinaryChunkVec - ChunkVec wrap, with ordering elements with comparison function
- HashMap - uses linear probing and two-level indexing (put is named as insert), keeps entries (key+value) addresses, requires avalanche-effect hashing function (e.g. Murmur3)
- ConcurrentHashMap - thread-safe version of HashMap with concurrent reads (get operation) and locked reads on write operations (insert and remove); on read in result (ConcurrentHashMapResult) returns mutex protecting key-value pair to be used by the user
- ConcurrentHashMapIter - called with convention begin, while(iter == E_OK && present) and finish
- HashSet - subset of HashMap (key is HashSet value, HashMap value is empty)
- HashMapIter - next function returns E_OK on no error and for loop continuation should be checked result.present
- LinkedList - Java doubly-linked list implementation
- ConcurrentLinkedList - thread-safe version of LinkedList, locks 1-3 nodes at the time of iter or other operation
- Queue - FIFO queue, has invalidate function and state + free_indexes Vecs for invalidating functionality (created for Flux's network pipeline inner usage)
- QueueConcurrentIter - create for Flux inner use purpose, thread-safe iterator over Queue
- QueueIter - thread-unsafe iterator over Queue
- src/std/collections/hash provides Murmur3 and Md5 hash functions, provides scalar hash func for e.g. integers, doubles, or any other data with size lower or equal to 8 bytes (sidenote: HashMap uses two-level indexing which requires avalanche-effect hash which scalar hash does not provide; use hfunc_murmur3_string for String key in HashMap)

./string/:
- String.h: UTF-8 string implementation with size (includes null termination char) and length params (length includes chars before null termination char)
- Utf8Util.h: UTF-8 string size checking
- StringConversion.h: appending numbers to String (e.g. size_t or int), also variant for appending hex formatted value for uint64_t
- StringIter.h: UTF-8 char-by-char iteration (uint32_t char result)

./thread/:
- Thread.h: stop signaling with thread_schedule_stop(thread) and join logic
- ThreadPool.h: no join of threads, need to apply alternative logic, e.g. assuming when threadpool will finish; growing threadpool logic with function threadpool_spawn_thread(threadpool)

# Coding Conventions

**Memory management**: No RAII. Resources use explicit `create`/`destroy` (or `init`/`deinit`) function pairs. Always call destroy when done.

**Error handling**: Functions return `Error` codes (defined in `./error.h`). Output is returned via reference parameter or int return value (latter more common). Use the `E_RET_IF_ERROR_USE` / `E_RET_IF_ERROR_USE2` macros for propagation. `E_OK` means success.

**String type**: Use the custom `String` type (not `std::string`). It tracks byte size, capacity, and UTF-8 character length separately.

**Threading**: Shared state is protected with `Mutex`. Semaphores coordinate thread lifecycle. Each major subsystem owns its thread(s) and data.

**Hashing**: `HashMap` and `HashSet` use murmur3 hashing (in `./collections/hash/`).

**Compiler flags**: `-mavx2` is enabled; SIMD-friendly data layouts are expected in hot paths.

**Destroying**: if some resource is protected with mutex then need to first acquire mutex lock then destroy that resource, unlock the mutex and then destroy the mutex

**Function params**: on API functions of struct try to fit to max 3 params not counting in the first param if it's struct (from OOP it is in place of usage of "this"); try exporting single params to grouped params, i.e. structs