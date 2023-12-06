#include "spscqueue.h"
#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

void spscqueue_init(spscqueue *restrict q, void *buf, ptrdiff_t buflen,
                    ptrdiff_t itemsz) {
  assert(atomic_is_lock_free(&(spscqueue){0}.w));
  assert(q);
  assert(buf);
  assert(1 < buflen);
  assert(0 < itemsz);
  assert(!(buflen % itemsz));
  assert(1 < buflen / itemsz);
  char *tl = (char *)buf + buflen;
  *q = (spscqueue){.itemsz = itemsz,
                   .hd = buf,
                   .tl = tl,
                   .w = buf,
                   .r = buf,
                   .readerw = buf,
                   .writerr = buf};
}

bool spscqueue_trypush(spscqueue *restrict q, void const *restrict p) {
  assert(q);
  assert(p);
  char *wp = atomic_load_explicit(&q->w, memory_order_relaxed);
  char *next_wp = wp + q->itemsz;
  if (next_wp == q->tl) next_wp = q->hd;
  if (next_wp == q->writerr) {
    q->writerr = atomic_load_explicit(&q->r, memory_order_consume);
    if (next_wp == q->writerr) return 0;
  }
  memcpy(wp, p, q->itemsz);
  atomic_store_explicit(&q->w, next_wp, memory_order_release);
  return 1;
}

void const *spscqueue_trypop(spscqueue *q) {
  assert(q);
  char *rp = atomic_load_explicit(&q->r, memory_order_relaxed);
  if (rp == q->readerw) {
    q->readerw = atomic_load_explicit(&q->w, memory_order_acquire);
    if (rp == q->readerw) return 0;
  }
  char *next_rp = rp + q->itemsz;
  if (next_rp == q->tl) next_rp = q->hd;
  atomic_store_explicit(&q->r, next_rp, memory_order_release);
  return rp;
}
