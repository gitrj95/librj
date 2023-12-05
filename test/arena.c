#include <stdalign.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../posix_arena.c"
#include "../test.h"

void initndelete(void) {
  test();
  arena a;
  expect_true(!arena_init(&a, 123));
  expect_true(a.hd < a.tl);
  expect_true(!((uintptr_t)a.hd % sysconf(_SC_PAGESIZE)));
  arena_delete(&a, 123, 0);
  expect_true(!a.hd && !a.tl);
  expect_abort(arena_init(&a, -1));
  expect_abort(arena_delete(0, 1, 0));
  expect_abort(arena_init(0, 1));
}

int freecnt;

int myfree(void *p, ptrdiff_t len) {
  (void)len;
  free(p);
  return ++freecnt;
}

void init3ndelete(void) {
  test();
  void *buf = malloc(123123);
  arena a;
  expect_true(!arena_init3(&a, buf, 123123));
  expect_true(a.hd == buf);
  expect_true(a.hd + 123123 == a.tl);
  int *n = linalloc(&a, int);
  expect_true(!((uintptr_t)n % alignof(int)));
  expect_true(a.hd < (char *)n);
  expect_true(a.tl == (char *)n);
  *n = 123;
  arena_delete(&a, 123123, myfree);
  expect_true(1 == freecnt);
  expect_true(arena_init(&a, PTRDIFF_MAX));
  expect_abort(arena_init3(&a, 0, 1));
  expect_abort(arena_init3(&a, buf, 0));
  expect_abort(arena_init3(0, buf, 1));
}

void alloc(void) {
  test();
  arena a;
  expect_true(!arena_init(&a, 123));
  int *i = linalloc(&a, int);
  expect_true(i);
  expect_true(!((uintptr_t)a.hd % alignof(int)));
  *i = 12;
  int j;
  memcpy(&j, i, sizeof(int));
  expect_true(j == *i);
  int *k = linalloc_explicit(&a, sizeof(*k), 1 << 10);
  expect_true(k);
  expect_true(!((uintptr_t)a.tl % (1 << 10)));
  k = linalloc(&a, int);
  expect_true(!k);
  arena_delete(&a, 123, 0);
  int64_t pagesz = sysconf(_SC_PAGESIZE);
  expect_true(!arena_init(&a, pagesz + 123));
  expect_true(!((uintptr_t)a.hd % pagesz));
  float *f = linalloc_explicit(&a, sizeof(*f), (uint16_t)pagesz);
  expect_true(f);
  expect_true(!((uintptr_t)f % pagesz));
  f = linalloc_explicit(&a, sizeof(*f), (uint16_t)1 << 4);
  expect_true(f);
  expect_true((uintptr_t)f % pagesz);
  f = linalloc(&a, float);
  *f = 1000.f;
  expect_true(!((uintptr_t)f % alignof(float)));
  expect_abort(linalloc_explicit(0, 0, 0));
  expect_abort(linalloc_explicit(&a, 0, 0));
  expect_abort(linalloc_explicit(&a, 10, -1));
  expect_abort(linalloc_explicit(&a, 10, 123));
  expect_abort(arena_delete(&a, -1, 0));
  arena_delete(&a, pagesz + 123, 0);
  expect_abort(arena_delete(&a, 10, 0));
}

void hijack_alloc(void) {
  test();
  arena a;
  a.hd = (char *)0;
  a.tl = (char *)1;
  int *p = linalloc(&a, int);
  expect_true(!p);
}

int main(void) {
  suite();
  initndelete();
  init3ndelete();
  alloc();
  hijack_alloc();
}
