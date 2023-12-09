/*
  DANGER: As far as the C standard is concerned, this implementation
  is UB. If the system is POSIX and the uintptr_t representation of an
  object pointer is precisely its address, then it is both correct and
  non-portable.

  N.b.: tooling normally traces memory manager subroutines, but not
  `mmap'. Pass a malloc'd buffer into `arena_init3' to make use of it.
*/

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>
#include "arena.h"

#define unlikely(expr) \
  __builtin_expect(!!(expr), 0) /* NOTE: prefer spec exec to cmov */

int arena_init(arena *a, ptrdiff_t len) {
  assert(a);
  assert(0 < len);
  void *buf =
      mmap(0, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (MAP_FAILED == buf) return -1;
  return arena_init3(a, buf, len);
}

int arena_init3(arena *a, void *buf, ptrdiff_t buflen) {
  assert(a);
  assert(buf);
  assert(0 < buflen);
  *a = (arena){.hd = buf, .tl = (char *)buf + buflen};
  return 0;
}

int arena_delete(arena *a, ptrdiff_t len,
                 int (*deleter)(void *, ptrdiff_t len)) {
  assert(a);
  assert(a->hd && a->tl);
  assert(0 < len);
  if (deleter)
    return deleter(a->hd, len);
  else if (munmap(a->hd, len))
    return -1;
  *a = (arena){0};
  return 0;
}

void *linalloc_explicit(arena *a, ptrdiff_t itemsz, int align) {
  assert(a);
  assert(0 < itemsz);
  assert(-1 < align);
  assert(!(align & (align - 1)));
  static_assert(sizeof(uintptr_t) >= sizeof(int));
  uintptr_t addr = (uintptr_t)a->tl;
  addr -= itemsz;
  addr = addr & ~(align - 1);             /* NOTE: need sign extension */
  if (unlikely(addr >= (uintptr_t)a->tl)) /* NOTE: guard wrap */
    return 0;
  if (unlikely(addr < (uintptr_t)a->hd)) return 0;
  a->tl = (char *)addr;
  return a->tl;
}
