#ifndef ARENA_H
#define ARENA_H

#include <stdalign.h>
#include <stdint.h>
#include "ssize.h"

#define LINALLOC(ap, T) (T *)(linalloc((ap), sizeof(T), alignof(T)))

typedef struct arena arena;

arena *arena_create(ssize len);
int arena_destroy(arena **arena);
void *linalloc(arena *arena, ssize itemsz, int32_t align);

#endif
