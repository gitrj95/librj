#include <stdalign.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../posix_arena.c"
#include "../test.h"

void createndelete(void) {
  test("initialize and delete arena");
  arena a;
  expect(!arena_create(&a, 123), "happy-path initialization");
  expect(a.hd < a.tl, "head address below its tail address");
  expect(!((uintptr_t)a.hd % sysconf(_SC_PAGE_SIZE)),
         "head address aligned to page");
  arena_delete(&a, 123, 0);
  expect(!a.hd && !a.tl, "head and tail members zeroed");
  die(arena_create(&a, -1), "negative arena length");
}

int freecnt;

int myfree(void *p, ptrdiff_t len) {
  (void)len;
  free(p);
  return ++freecnt;
}

void initndelete(void) {
  test("initialize and delete arena with user-defined buffer");
  void *buf = malloc(123123);
  arena a;
  expect(!arena_init(&a, buf, 123123), "happy-path initialization");
  expect(a.hd == buf, "head pointer is the base address of the buffer");
  expect((char *)a.hd + 123123 == a.tl,
         "tail pointer is exactly the buffer's length away");
  arena_delete(&a, 123123, myfree);
  expect(1 == freecnt, "free called exactly once");
  expect(arena_create(&a, PTRDIFF_MAX),
         "fails initialization with sufficiently large length");
  die(arena_init(&a, buf, 0), "zero buffer length");
}

void alloc(void) {
  test("linear allocations");
  arena a;
  arena_create(&a, 123);
  int *n = linalloc(&a, int);
  expect(n, "non-zero pointer allocation");
  expect(!((uintptr_t)n % alignof(int)), "ensure pointer's aligned");
  expect((int *)a.hd < n, "head pointer below allocated pointer");
  expect((int *)a.tl == n, "tail pointer moves with latest allocation");
  *n = 123;
  int j;
  memcpy(&j, n, sizeof(int));
  expect(j == *n, "copied value matches dereferenced pointer");
  int *k = linalloc_explicit(&a, sizeof(*k), 1 << 10);
  expect(k, "non-zero explicit linear allocation");
  expect(0 == (uintptr_t)k % (1 << 10), "allocation suitably aligned");
  k = linalloc(&a, int);
  expect(!k, "null when arena exhausted");
  arena_delete(&a, 123, 0);
  int pagesz = (int)sysconf(_SC_PAGE_SIZE);
  arena_create(&a, pagesz + 123);
  expect(!((uintptr_t)a.hd % pagesz), "head pointer aligned to page boundary");
  float *f = linalloc_explicit(&a, sizeof(*f), pagesz);
  expect(0 == (uintptr_t)f % pagesz,
         "non-zero explicit linear allocation to page boundary");
  f = linalloc_explicit(&a, sizeof(*f), (uint16_t)1 << 4);
  f = linalloc(&a, float);
  *f = 1000.f;
  die(linalloc_explicit(&a, 0, 0), "zeroed item size");
  die(linalloc_explicit(&a, 10, -1), "negative alignment");
  die(linalloc_explicit(&a, 10, 123), "alignment not a power-of-2");
  die(arena_delete(&a, -1, 0), "negative length");
  arena_delete(&a, pagesz + 123, 0);
}

void hijack_alloc(void) {
  test("hijack arena members");
  arena a;
  a.hd = (char *)0;
  a.tl = (char *)1;
  int *p = linalloc(&a, int);
  expect(!p, "null when wrapping above arena's range");
}

int main(void) {
  suite();
  createndelete();
  initndelete();
  alloc();
  hijack_alloc();
}
