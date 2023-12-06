/*
  Exploit the VM-subsystem to provide a pointer that acts like an
  array but feels like a ring buffer. Namely, operations like copying
  from [-5..2] should work seamlessly. e.g. buf[-1] should give the
  value in the last slot.

  This is particularly useful because the prototypical structure of
  page tables and the VM-subsystem essentially grants this capability
  for free. The OOP style of constructing a ring buffer as some
  wrapper type with modulo to access elements given some index has
  major optimization disadvantages. By synthesizing a contiguous
  virtual address range with the same physical pages backing the
  region immediately before and after, tremendous optimization
  opportunities are unlocked, e.g. auto-vectorization, pre-fetching,
  etc. that normally would not work as well, if at all, with the OOP-y
  wrapper.

  And, as it's just an array, the interface is, by construction, much
  cleaner. Want to copy the last 5 things? `memcpy(&dst, src[-5], 5)'!

  Compared to a "normal" array, there are a few minor performance
  caveats. As the duplicate ranges are composed of new virtual
  addresses, the same physical pages will be aliased by 3 separate
  sets of cache lines in VIPT lookup, hampering L1 readthrough. For
  PIPT lookups in L2 and lower caches, this is negligible.
*/

#ifndef RINGBUF_H
#define RINGBUF_H

#include <stddef.h>

void *ringbuf_create(ptrdiff_t len);
int ringbuf_destroy(void *buf, ptrdiff_t len);

#endif
