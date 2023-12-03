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

#define linalloc(ap, T) linalloc_explicit((ap), sizeof(T), alignof(T))

typedef int (*arena_deleter)(void *, ptrdiff_t);

typedef struct {
  char *hd, *tl;
  arena_deleter deleter;
} arena;

int arena_init(arena *a, ptrdiff_t len);
int arena_init4(arena *a, void *buf, ptrdiff_t buflen, arena_deleter deleter);
[[gnu::malloc]] void *linalloc_explicit(arena *a, ptrdiff_t itemsz, int align);
int arena_delete(arena *a);

#endif
