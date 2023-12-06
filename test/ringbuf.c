#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "../posix_ringbuf.c"
#include "../test.h"

void bad_args(void) {
  test("strange arguments");
  die(ringbuf_create(-1), "negative length");
  die(ringbuf_create(123), "length not a multiple of page size");
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

int main(void) {
  suite();
  bad_args();
  alloc();
  copy();
  return 0;
}
