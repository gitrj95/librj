#include "spscqueue.h"
#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "ssize.h"

void spscqueue_init(spscqueue *restrict q, void *hd, void *tl, ssize itemsz) {
  assert(q);
  assert(hd);
  assert(tl);
  assert((char *)hd < (char *)tl);
  assert(itemsz > 0);
  assert(!(((char *)tl - (char *)hd) % itemsz));
  assert(1 < ((char *)tl - (char *)hd) / itemsz);
#define HD ((intptr_t)(hd))
  *q = (spscqueue){.itemsz = itemsz,
                   .hd = hd,
                   .tl = tl,
                   .w = HD,
                   .r = HD,
                   .readerw = HD,
                   .writerr = HD};
#undef HD
}

bool spscqueue_trypush(spscqueue *restrict q, void const *restrict p) {
  assert(q);
  assert(p);
#define C(x) ((char *)(x))
  char *wp = C(atomic_load_explicit(&q->w, memory_order_relaxed));
  char *next_wp = wp + q->itemsz;
  if (next_wp == q->tl) next_wp = q->hd;
  if (next_wp == C(q->writerr)) {
    q->writerr = atomic_load_explicit(&q->r, memory_order_relaxed);
    if (next_wp == C(q->writerr)) return 0;
#undef C
  }
  memcpy(wp, p, q->itemsz);
  atomic_store_explicit(&q->w, (intptr_t)next_wp, memory_order_release);
  return 1;
}

void const *spscqueue_trypop(spscqueue *q) {
  assert(q);
#define C(x) ((char *)(x))
  char *rp = C(atomic_load_explicit(&q->r, memory_order_relaxed));
  if (rp == C(q->readerw)) {
    q->readerw = atomic_load_explicit(&q->w, memory_order_acquire);
    if (rp == C(q->readerw)) return 0;
#undef C
  }
  char *next_rp = rp + q->itemsz;
  if (next_rp == q->tl) next_rp = q->hd;
  atomic_store_explicit(&q->r, (intptr_t)next_rp, memory_order_relaxed);
  return rp;
}