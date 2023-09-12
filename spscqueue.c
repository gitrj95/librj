#include "spscqueue.h"
#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

void spscqueue_init(spscqueue *restrict q, void *hd, void *tl, long item_len) {
  assert(q);
  assert(hd);
  assert(tl);
  assert((char *)hd < (char *)tl);
  assert(item_len > 0);
  assert(!(((char *)tl - (char *)hd) % item_len));
#define HD (intptr_t)(hd)
  *q = (spscqueue){.item_len = item_len,
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
  char *wp = (char *)atomic_load_explicit(&q->w, memory_order_relaxed);
  char *next_wp = wp + q->item_len;
  if (next_wp == q->tl) next_wp = q->hd;
  if (next_wp == (char *)q->writerr) {
    q->writerr = atomic_load_explicit(&q->r, memory_order_relaxed);
    if (next_wp == (char *)q->writerr) return 0;
  }
  memcpy(wp, p, q->item_len);
  atomic_store_explicit(&q->w, (intptr_t)next_wp, memory_order_release);
  return 1;
}

void const *spscqueue_trypop(spscqueue *q) {
  assert(q);
  char *rp = (char *)atomic_load_explicit(&q->r, memory_order_relaxed);
  if (rp == (char *)q->readerw) {
    q->readerw = atomic_load_explicit(&q->w, memory_order_acquire);
    if (rp == (char *)q->readerw) return 0;
  }
  char *next_rp = rp + q->item_len;
  if (next_rp == q->tl) next_rp = q->hd;
  atomic_store_explicit(&q->r, (intptr_t)next_rp, memory_order_relaxed);
  return rp;
}
