/*
  DANGER: As far as the C standard is concerned, this implementation
  is UB. Practically, if the system is POSIX and the uintptr_t
  representation of an object pointer is precisely its address, then
  it is "safe".
*/

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "arena.h"
#include "ssize.h"

#define unlikely(expr) __builtin_expect(!!(expr), 0)

struct arena {
  char *hd, *tl, *p;
  void (*deleter)(void *);
};

struct arena *arena_create(ssize len) {
  assert(0 < len);
  void *buf =
      mmap(0, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (MAP_FAILED == buf) return 0;
  return arena_create3(buf, len, 0);
}

struct arena *arena_create3(void *buf, ssize buflen, void (*deleter)(void *)) {
  assert(buf);
  assert(0 < buflen);
  if (buflen >= PTRDIFF_MAX) return 0;
  struct arena *a = malloc(sizeof(*a));
  if (!a) return 0;
  *a =
      (struct arena){.hd = buf, .tl = (char *)buf + buflen, .deleter = deleter};
  a->p = a->tl;
  return a;
}

int arena_delete(struct arena **a) {
  assert(a);
  assert(*a);
  ptrdiff_t len = a[0]->tl - a[0]->hd;
  if (a[0]->deleter)
    a[0]->deleter(a[0]->hd);
  else if (munmap(a[0]->hd, len))
    return -1;
  free(*a);
  *a = 0;
  return 0;
}

void *linalloc_explicit(struct arena *a, ssize itemsz, int32_t align) {
  assert(a);
  assert(0 < itemsz);
  assert(-1 < align);
  assert(!(align & (align - 1)));
  static_assert(sizeof(uintptr_t) >= sizeof(uint32_t));
  uintptr_t addr = (uintptr_t)a->p;
  addr -= itemsz;
  addr = addr & ~(align - 1); /* need sign extension */
  if (unlikely(addr < (uintptr_t)a->hd))
    return 0; /* prefer speculative execution to cmov */
  a->p = (char *)addr;
  return a->p;
}

void arena_reset(arena *a) {
  assert(a);
  a->p = a->tl;
}
