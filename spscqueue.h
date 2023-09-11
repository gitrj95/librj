#ifndef SPSCQUEUE_H
#define SPSCQUEUE_H

#include <stdalign.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef CACHE_BLOCK_BYTES
#define CACHE_BLOCK_BYTES 64
#endif

#define spscqueue_init_from_static(qp, a)                   \
  do {                                                      \
    size_t len__ = sizeof(a) / sizeof((a)[0]);              \
    spscqueue_init((qp), (a), &(a)[len__], sizeof((a)[0])); \
  } while (0)

typedef struct {
  long item_len;
  void *hd, *tl;
  alignas(CACHE_BLOCK_BYTES) atomic_intptr_t w, r;
  alignas(CACHE_BLOCK_BYTES) intptr_t readerw, writerr;
} spscqueue;

void spscqueue_init(spscqueue *restrict q, void *hd, void *tl, long item_len);
bool spscqueue_push(spscqueue *restrict q, void *restrict p);
void const *spscqueue_pop(spscqueue *q);

#endif
