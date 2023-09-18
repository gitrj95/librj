#include <stdalign.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "../posix_arena.c"
#include "../test.h"

void createndestroy(void) {
  test();
  arena *ap = arena_create(123);
  expect_true(ap);
  expect_true(ap->hd < ap->tl);
  expect_true(ap->p == ap->tl);
  expect_true(!((uintptr_t)ap->hd % sysconf(_SC_PAGESIZE)));
  arena_destroy(&ap);
  expect_true(!ap);
  expect_abort(arena_create(-1));
  expect_abort(arena_destroy(0));
  expect_abort(arena_destroy(&ap));
}

void alloc(void) {
  test();
  arena *ap = arena_create(123);
  int *i = LINALLOC(ap, int);
  expect_true(i);
  expect_true(!((uintptr_t)ap->hd % alignof(int)));
  *i = 12;
  int j;
  memcpy(&j, i, sizeof(int));
  expect_true(j == *i);
  int *k = linalloc(ap, sizeof(*k), 1 << 10);
  expect_true(k);
  expect_true(ap->hd == ap->p);
  expect_true(!((uintptr_t)ap->p % (1 << 10)));
  k = LINALLOC(ap, int);
  expect_true(!k);
  arena_destroy(&ap);
  int64_t pagesz = sysconf(_SC_PAGESIZE);
  ap = arena_create(pagesz + 123);
  expect_true(ap);
  expect_true(!((uintptr_t)ap->hd % pagesz));
  float *f = linalloc(ap, sizeof(*f), (uint16_t)pagesz);
  expect_true(f);
  expect_true(!((uintptr_t)f % pagesz));
  f = linalloc(ap, sizeof(*f), (uint16_t)1 << 4);
  expect_true(f);
  expect_true((uintptr_t)f % pagesz);
  f = LINALLOC(ap, float);
  *f = 1000.f;
  expect_true(!((uintptr_t)f % alignof(float)));
  expect_abort(linalloc(0, 0, 0));
  expect_abort(linalloc(ap, 0, 0));
  expect_abort(linalloc(ap, 10, -1));
  expect_abort(linalloc(ap, 10, 123));
}

int main(void) {
  suite();
  createndestroy();
  alloc();
}
