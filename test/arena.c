#include <stdalign.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "../posix_arena.c"
#include "../test.h"

void createndelete(void) {
  test();
  arena *a = arena_create(123);
  expect_true(a);
  expect_true(a->hd < a->tl);
  expect_true(a->p == a->tl);
  expect_true(!((uintptr_t)a->hd % sysconf(_SC_PAGESIZE)));
  arena_delete(&a);
  expect_true(!a);
  expect_abort(arena_create(-1));
  expect_abort(arena_delete(0));
  expect_abort(arena_delete(&a));
}

void myfree(void *p, ptrdiff_t len) {
  (void)len;
  free(p);
}

void create3ndelete(void) {
  test();
  void *buf = malloc(123123);
  arena *a = arena_create3(buf, 123123, myfree);
  expect_true(a->hd == buf);
  expect_true(a->hd + 123123 == a->tl);
  int *n = linalloc(a, int);
  expect_true(!((uintptr_t)n % alignof(int)));
  expect_true(a->hd < (char *)n);
  expect_true(a->tl > (char *)n);
  *n = 123;
  arena_delete(&a);
  a = arena_create(PTRDIFF_MAX);
  expect_true(!a);
  expect_abort(arena_create3(0, 1, myfree));
  expect_abort(arena_create3(buf, 0, myfree));
}

void alloc(void) {
  test();
  arena *a = arena_create(123);
  int *i = linalloc(a, int);
  expect_true(i);
  expect_true(!((uintptr_t)a->hd % alignof(int)));
  *i = 12;
  int j;
  memcpy(&j, i, sizeof(int));
  expect_true(j == *i);
  int *k = linalloc_explicit(a, sizeof(*k), 1 << 10);
  expect_true(k);
  expect_true(a->hd == a->p);
  expect_true(!((uintptr_t)a->p % (1 << 10)));
  k = linalloc(a, int);
  expect_true(!k);
  arena_delete(&a);
  int64_t pagesz = sysconf(_SC_PAGESIZE);
  a = arena_create(pagesz + 123);
  expect_true(a);
  expect_true(!((uintptr_t)a->hd % pagesz));
  float *f = linalloc_explicit(a, sizeof(*f), (uint16_t)pagesz);
  expect_true(f);
  expect_true(!((uintptr_t)f % pagesz));
  f = linalloc_explicit(a, sizeof(*f), (uint16_t)1 << 4);
  expect_true(f);
  expect_true((uintptr_t)f % pagesz);
  f = linalloc(a, float);
  *f = 1000.f;
  expect_true(!((uintptr_t)f % alignof(float)));
  expect_abort(linalloc_explicit(0, 0, 0));
  expect_abort(linalloc_explicit(a, 0, 0));
  expect_abort(linalloc_explicit(a, 10, -1));
  expect_abort(linalloc_explicit(a, 10, 123));
  arena_delete(&a);
}

void reset(void) {
  test();
  arena *a = arena_create(123);
  struct fat {
    int a[1000];
  };
  struct fat *f = linalloc(a, struct fat);
  expect_true(!f);
  arena_reset(a);
  expect_true(a->p == a->tl);
  struct thin {
    int a[2];
  };
  struct thin *t = linalloc(a, struct thin);
  expect_true(t);
  *t = (struct thin){{3, 4}};
  expect_true(t->a[0] == 3);
  expect_true(t->a[1] == 4);
  expect_abort(arena_reset(0));
  arena_delete(&a);
}

int main(void) {
  suite();
  createndelete();
  create3ndelete();
  alloc();
  reset();
}
