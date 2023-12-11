#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../arraysize.h"
#include "../posix_ringbuf.c"
#include "../test.h"

void bad_args(void) {
  test("strange arguments");
  die(ringbuf_create(-1), "negative length");
  die(ringbuf_create(123), "length not a multiple of page size");
}

void name_gen(void) {
  test("name generation invariants");
  FILE *f = fopen("name_gen.etc", "wb+");
  assert(f);
  char buf[100];
  fwrite(buf, sizeof(char), arraysize(buf), f);
  rewind(f);
  char alphabet[] = "wijlk6)"; /* NOTE: lazy removal of nul */
  char name[arraysize(buf)];
  fill_name(name, arraysize(buf), alphabet, arraysize(alphabet) - 1, f);
  expect('/' == name[0], "first char is `/'");
  expect(arraysize(buf) - 1 == strlen(name), "maximal length given arguments");
  expect(arraysize(buf) - 2 == strspn(name + 1, alphabet),
         "fully from alphabet");
  die(fill_name(buf, arraysize(buf), ")", 1, f), "illegal `/' in alphabet");
}

void shmem(void) {
  test("shared memory");
  die(open_shmem(0, 2), "null name");
  die(open_shmem("/asd", -1), "negative len");
  die(open_shmem("//", 2), "duplicate `/'");
}

void alloc(void) {
  test("ring buffer allocations");
  ptrdiff_t pagesz = sysconf(_SC_PAGESIZE);
  int *buf = ringbuf_create(pagesz);
  expect(buf, "allocate a ring buffer");
  ptrdiff_t cnt = pagesz / sizeof(int);
  for (int i = 0; i < cnt; ++i) buf[i] = i;
  for (int i = -1; i < 2; ++i)
    for (int j = 0; j < cnt; ++j) assert(j == buf[i * cnt + j]);
  pass("negative and positive offsets back same buffer");
}

void copy(void) {
  test("memcpy");
  ptrdiff_t pagesz = sysconf(_SC_PAGESIZE);
  int *buf = ringbuf_create(pagesz);
  ptrdiff_t cnt = pagesz / sizeof(int);
  for (int i = 0; i < cnt; ++i) buf[i] = i;
  int a[5] = {0};
  memcpy(a, buf - 5, sizeof(a));
  for (int i = 0; i < 5; ++i) assert(a[i] == cnt - (ptrdiff_t)arraysize(a) + i);
  pass("copied buffer matches");
}

void destroy(void) {
  test("destroy ring buffer");
  ptrdiff_t pagesz = sysconf(_SC_PAGESIZE);
  int *buf = ringbuf_create(pagesz);
  die(ringbuf_destroy(0, pagesz), "null buffer");
  die(ringbuf_destroy(buf, -pagesz), "negative length");
  die(ringbuf_destroy(buf, 123), "length not a multiple of page size");
  expect(!ringbuf_destroy(buf, pagesz), "successful destruction");
}

int main(void) {
  suite();
  bad_args();
  name_gen();
  shmem();
  alloc();
  copy();
  destroy();
  return 0;
}
