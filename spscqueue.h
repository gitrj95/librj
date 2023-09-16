/*
This is an optimized SPSC queue. It is wait-free with optimal
fencing. It accepts a head and tail pointer to some region of memory
such that the head pointer is the first valid pointer into the buffer
and the tail pointer is the prototypical too-far pointer--i.e. one
past the end. The existence and validity of such a pointer is
guaranteed by the C standard; that is, we can read (not dereference!)
such a pointer and can reach it via the appropriate offset from the
head.

The API is trivial. We initialize a queue, we try to push to it, try
to pop from it, or we force a push or force a pop by redoing tries
until they succeed. Finally, because it is useful and frequent to
initialize this queue given some static memory, a helper macro exists
as sugar. The only nontrivial implementation details regard the
trypush and trypop definitions.

We walk through the buffer as a ring buffer, so naturally we will need
atomic pointers to denote the writer's and reader's positions. If the
reader comes up to the writer, namely, the consumer would approach the
slot where the producer is writing, we fail. Similar arguments follow
for the writer coming up to the reader. In reality, the producer stops
in the slot before the consumer's, as this allows the queue to not
have to be a power of 2 large without performance penalties--a trick
gleaned from Rigtorp. We can do this because the pointers always
increase by 1, and modular arithmetic takes care of the rest.

The typical implementation would have the producer atomically load the
reader pointer and the consumer atomically load the writer pointer and
compare them in their respective jobs, but this will incur substantial
coherency traffic. WLOG, we can talk about the consumer example, as
the producer is symmetric WRT this condition. The consumer loads the
writer pointer, but the producer writes to the writer pointer,
broadcasting invalidates on the bus, etc.

The above may be rectified by introducing a cached copy of these
atomic pointers that belong to the jobs that read them. Namely, the
producer has a cached copy of the reader pointer and the consumer has
a cached copy of the writer pointer. Because both always move
forwards, and this invariant is preserved by modular arithmetic, we
have that the cached copies, exclusively owned by their pursuant jobs,
will be behind the true pointers. We then greatly reduce the coherency
traffic by significantly reducing the need to atomically load the
others' pointers.

What remains is how we make writes visible. It is clear that we must
pair a release with an acquire when we copy a new value into the
buffer and publish it for the consumer to see. We can use the above
trick to avoid needing to use an acquire barrier in the consumer in
most cases. Namely, as the consumer's cached copy is behind the true
writer pointer, the memory up to the cached copy must be visible to
the consumer. If we need to update the cached copy with the atomic
writer pointer, at that point, we must load with acquire semantics to
guarantee initialized memory of the cells between our cached copy and
the update with the true value. The consumer will never advance ahead
of the producer by the same argument.

Do we need any other barriers despite synchronization with multiple
atomics? Actually, no. Intuitively, the consumer is not trying to make
anything visible to the producer, as it's only updating something that
the writer never stores-to, only loads-from. Formally, we only have
two cases. The reader pointer is atomic, so we have, that, at minimum,
that the load from the producer is indeterminately sequenced with the
store from the consumer. If the load from the producer happens before
the store from the consumer, the writer's cached reader pointer is
behind, and we potentially may fail a write: harmless. If the store
from the consumer happens before the load from the producer, the
writer's cached reader pointer will be updated to a less stale value:
the "correct" behavior. However the sequencing, we have valid
behavior.

In sum, we can reduce the miss rate and the barrier placement by
virtue of symmetric, cached values that are guaranteed to be "behind",
and as afforded by modular arithmetic, they won't "step on each
other".
*/

#ifndef SPSCQUEUE_H
#define SPSCQUEUE_H

#include <stdalign.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef CACHE_BLOCK_BYTES
#define CACHE_BLOCK_BYTES 64
#endif

#define spscqueue_init_from_static(qp, a)                           \
  do {                                                              \
    size_t sz__ = sizeof(a) / sizeof((a)[0]);                      \
    typeof(&a[0]) a_dcy__ = (a);                                    \
    spscqueue_init((qp), a_dcy__, a_dcy__ + sz__, sizeof((a)[0])); \
  } while (0)

typedef struct {
  long itemsz;
  void *hd, *tl;
  alignas(CACHE_BLOCK_BYTES) atomic_intptr_t w, r;
  alignas(CACHE_BLOCK_BYTES) intptr_t readerw, writerr;
} spscqueue;

void spscqueue_init(spscqueue *restrict q, void *hd, void *tl, long item_len);
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
