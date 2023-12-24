#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../posix_ringbuf.c"
#include "../test.h"

void bad_args(void) {
  test("bad arguments for creation/destruction");
  die(ringbuf_create(0, 1, 1), "null ring buffer");
  die(ringbuf_create((struct ringbuf *){0}, -1, 1), "negative item size");
  die(ringbuf_create((struct ringbuf *){0}, 1, -2), "negative minimum items");
  die(ringbuf_destroy((struct ringbuf){0, 1}, 1), "null pointer member");
  die(ringbuf_destroy((struct ringbuf){(void *)0xdeadbeef, -1}, 1),
      "negative number of items member");
}

void blockargs(void) {
  test("bad args for block allocation");
  die(allocblock(0, 1, 1, 1), "null base pointer");
  die(allocblock((void *)1, 0, 1, 1), "0 length");
  die(allocblock((void *)1, 1, 1, -1), "negative fd");
}

void name_gen(void) {
  test("name generation invariants");
  FILE *f = fopen("name_gen.etc", "wb+");
  assert(f);
  char buf[100];
  fwrite(buf, sizeof(char), arraysize(buf), f);
  rewind(f);
  char alphabet[] = "wijlk6)";
  char name[arraysize(buf)];
  fill_name(name, arraysize(buf), alphabet, arraysize(alphabet) - 1, f);
  expect('/' == name[0], "first char is `/'");
  expect(arraysize(buf) - 1 == strlen(name), "maximal length given arguments");
  expect(arraysize(buf) - 2 == strspn(name + 1, alphabet),
         "fully from alphabet");
  die(fill_name(buf, arraysize(buf), "/1", 1, f), "no `/' in alphabet");
}

void shmem(void) {
  test("shared memory");
  die(open_shmem(0, 2), "null name");
  die(open_shmem("/asd", -1), "negative len");
  die(open_shmem("asd", 2), "name[0] is not `/'");
  die(open_shmem("//", 2), "duplicate `/'");
}

void allocworound(void) {
  test("ring buffer allocations with no rounding");
  die(alloc3(-1, 1), "negative block length");
  die(alloc3(0, -1), "negative fd");
  ptrdiff_t pagesz = sysconf(_SC_PAGE_SIZE);
  struct ringbuf rb;
  ptrdiff_t nitems = pagesz / sizeof(int);
  expect(ringbuf_create(&rb, sizeof(int), nitems), "allocate");
  expect(rb.nitems == nitems, "no rounding of `nitems'");
  int *buf = rb.p;
  for (int i = 0; i < nitems; ++i) buf[i] = i;
  for (int i = -1; i < 2; ++i)
    for (int j = 0; j < nitems; ++j) assert(j == buf[i * nitems + j]);
  pass("negative and positive offsets back same buffer");
}

void allocwround(void) {
  test("ring buffer allocations with rounding");
  ptrdiff_t pagesz = sysconf(_SC_PAGE_SIZE);
  assert(111 < pagesz);
  struct ringbuf rb;
  ptrdiff_t nitems = 111 + pagesz / sizeof(int);
  expect(ringbuf_create(&rb, sizeof(int), nitems), "allocate");
  expect(rb.nitems == (ptrdiff_t)(2 * pagesz / sizeof(int)),
         "rounds to next highest page");
  int *buf = rb.p;
  for (int i = 0; i < nitems; ++i) buf[i] = i;
  for (int i = -1; i < 2; ++i)
    for (int j = 0; j < nitems; ++j) assert(j == buf[i * rb.nitems + j]);
  pass("negative and positive offsets back same buffer");
}

void allocwslop(void) {
  test("ring buffer allocations with slop");
  struct ringbuf rb;
  ptrdiff_t pagesz = sysconf(_SC_PAGE_SIZE);
  struct with_slop {
    int i1, i2, i3;
  };
  ptrdiff_t nitems = pagesz / sizeof(struct with_slop);
  die(ringbuf_create(&rb, sizeof(struct with_slop), nitems),
      "incompatible page slop");
}

void allocaggr(void) {
  test("ring buffer allocations with aggregates");
  struct aggregate {
    int i1, i2;
  };
  struct ringbuf rb;
  ptrdiff_t pagesz = sysconf(_SC_PAGE_SIZE);
  assert(0 == pagesz % sizeof(struct aggregate));
  ptrdiff_t nitems = pagesz / sizeof(struct aggregate);
  expect(ringbuf_create(&rb, sizeof(struct aggregate), nitems),
         "aggregate allocation");
  struct aggregate *buf = rb.p;
  for (int i = 0; i < nitems; ++i) buf[i] = (struct aggregate){i, i};
  for (int i = -1; i < 2; ++i)
    for (int j = 0; j < nitems; ++j) {
      struct aggregate v = buf[i * nitems + j];
      assert(j == v.i1);
      assert(j == v.i2);
    }
  pass("negative and positive offsets back same buffer");
}

void copy(void) {
  test("copy");
  struct ringbuf rb;
  ringbuf_create(&rb, sizeof(int), 11);
  int *buf = rb.p;
  for (int i = 0; i < rb.nitems; ++i) buf[i] = i;
  int a[5] = {0};
  memcpy(a, buf - 5, sizeof(a));
  for (int i = 0; i < 5; ++i) assert(a[i] == rb.nitems - arraysize(a) + i);
  pass("copied buffer matches");
}

void destroy(void) {
  test("destroy ring buffer");
  struct ringbuf rb;
  ringbuf_create(&rb, sizeof(float), 11111);
  expect(ringbuf_destroy(rb, sizeof(float)), "successful destruction");
}

int main(void) {
  suite();
  bad_args();
  blockargs();
  name_gen();
  shmem();
  allocworound();
  allocwround();
  allocwslop();
  allocaggr();
  copy();
  destroy();
  return 0;
}
