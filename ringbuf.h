/*
  Exploit the VM-subsystem to provide a pointer that acts like an
  array but feels like a ring buffer. Namely, operations like copying
  from [-5..2] should work seamlessly. e.g. buf[-1] should give the
  value in the last slot.

  This is particularly useful because the prototypical structure of
  page tables and the VM-subsystem essentially give you this
  capability for free. Constructing a ring-buffer as some wrapper type
  with modulo to access elements given some index has major
  optimization disadvantages. By synthesizing a contiguous virtual
  address range with duplicates immediately before and after, we
  unlock tremendous optimization opportunities,
  e.g. auto-vectorization, pre-fetching, etc. that normally would not
  work with the OOP-y wrapper type.

  And, as it's just an array, the interface is, by construction, much
  cleaner. Want to copy the last 5 things? `memcpy(&dst, src[-5], 5)'!
*/

#ifndef RINGBUF_H
#define RINGBUF_H

#include <stddef.h>

void *ringbuf_create(ptrdiff_t len);

#endif
