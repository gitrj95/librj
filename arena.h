/*
  Change the nature of the allocation problem. malloc/free are
  extremely general. Arenas are simple to compose with more complex
  allocators and data structures, provide nice spatial locality via
  linear allocation, and help untangle the lifetime mess. Memory
  management in C is much more enjoyable as a result:
  `https://www.rfleury.com/p/untangling-lifetimes-the-arena-allocator'.
*/

#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>

#define linalloc(a, T) (T *)linalloc_explicit((a), sizeof(T), alignof(T))

typedef struct {
  void *hd, *tl;
} arena;

int arena_create(arena *a, ptrdiff_t len);
int arena_init(arena *a, void *buf, ptrdiff_t buflen);
void *linalloc_explicit(arena *a, ptrdiff_t itemsz, int align);
int arena_delete(arena *a, ptrdiff_t len, int (*deleter)(void *, ptrdiff_t));

#endif
