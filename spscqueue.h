/*
  This is an optimized SPSC queue. It accepts a head and tail pointer
  to some region of memory such that the head pointer is the first
  valid pointer into the buffer and the tail pointer is the
  prototypical too-far pointer--i.e. one past the end. The existence
  and validity of such a pointer is guaranteed by the C standard; that
  is, we can read (not dereference!) such a pointer and can reach it
  via the appropriate offset from the head.

  The API is trivial. We initialize a queue, we try to push to it, try
  to pop from it, or we force a push or force a pop by redoing tries
  until they succeed. Finally, because it is useful and frequent to
  initialize this queue given some static memory, a helper macro
  exists as sugar. The only nontrivial implementation details regard
  the trypush and trypop definitions.

  We walk through the buffer as a ring, so naturally, we will need
  atomic pointers to denote the writer's and reader's positions. If
  the reader comes up to the writer, namely, the consumer would
  approach the slot where the producer is writing, we fail. A similar
  argument follows for the writer coming up to the reader given a
  critical nuance. Specifically, the producer stops in the slot
  immediately before the consumer's, as we need to disambiguate the
  empty/full states. The alternative is to use modulus, a slow
  operation unless the length of the given buffer is a power of 2--an
  undesirable restriction. Given that these pointers only logically
  increment by 1 and wrap to 0, codegen can trivially produce a test
  and cmov, so there's no branch to worry about on reasonable
  compilers.

  Nevertheless, a typical implementation would have the producer
  atomically load the reader pointer and the consumer atomically load
  the writer pointer and compare them, but this will incur substantial
  coherency traffic. WLOG, we can talk about the consumer example, as
  the producer is symmetric WRT this condition. The consumer loads the
  writer pointer, but the producer writes to the writer pointer,
  broadcasting invalidates on the bus, etc.

  The above may be rectified by introducing a cached copy of these
  atomic pointers that belong to the threads that read them. Namely,
  the producer has a cached copy of the reader pointer and the
  consumer has a cached copy of the writer pointer. Because both
  always move forwards, and this invariant is preserved by modular
  arithmetic, we have that the cached copies, exclusively owned by
  their pursuant jobs, will be behind the true pointers. We then
  greatly reduce the coherency traffic by significantly reducing the
  need to atomically load the others' pointers.

  What remains is how we handle synchronization. It is clear that we
  must pair a release with an acquire when we copy a new value into
  the buffer and publish it for the consumer to see. The remaining
  point of synchronization concerns the loads and stores on the reader
  pointer. The immediate conclusion is to pair a release with an
  acquire (or a consume) on the reader pointer to induce a
  happens-before edge from the reader to the writer.
*/

#ifndef SPSCQUEUE_H
#define SPSCQUEUE_H

#include <stdalign.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include "ssize.h"

#ifndef CACHE_BLOCK_BYTES
#define CACHE_BLOCK_BYTES 64
#endif

#define spscqueue_init2(qp, a) \
  spscqueue_init((qp), (a), sizeof(a), sizeof((a)[0]))

typedef struct {
  ssize itemsz;
  void *hd, *tl;
  alignas(CACHE_BLOCK_BYTES) atomic_intptr_t w, r;
  alignas(CACHE_BLOCK_BYTES) intptr_t readerw, writerr;
} spscqueue;

void spscqueue_init(spscqueue *restrict q, void *buf, ssize buflen,
                    ssize itemsz);
bool spscqueue_trypush(spscqueue *restrict q, void const *restrict p);
void const *spscqueue_trypop(spscqueue *q);

static inline void spscqueue_push(spscqueue *restrict q,
                                  void const *restrict p) {
  while (!spscqueue_trypush(q, p)) {}
}

static inline void const *spscqueue_pop(spscqueue *q) {
  void const *p;
  while (!(p = spscqueue_trypop(q))) {}
  return p;
}

#endif
