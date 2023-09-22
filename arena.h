/*
  Change the nature of the allocation problem. malloc/free are
  extremely general. Arenas are simple to compose with more complex
  allocators and data structures, provide nice spatial locality via
  linear allocation, and help untangle the lifetime mess. Memory
  management in C is much more enjoyable as a result:
  `https://www.rfleury.com/p/untangling-lifetimes-the-arena-allocator`.
*/

#ifndef ARENA_H
#define ARENA_H

#include <stdalign.h>
#include <stdint.h>
#include "ssize.h"

#define linalloc(ap, T) (T *)(linalloc_explicit((ap), sizeof(T), alignof(T)))

typedef struct arena arena;

arena *arena_create(ssize len);
int arena_destroy(arena **arena);
void *linalloc_explicit(arena *arena, ssize itemsz, int32_t align);
void arena_reset(arena *arena);

#endif
