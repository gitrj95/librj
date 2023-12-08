#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "arraysize.h"
#include "ringbuf.h"

#define SHMEM_NAME_MAX 10
static_assert(SHMEM_NAME_MAX < INT_MAX);

#define PROT (PROT_READ | PROT_WRITE)
#define FLAG (MAP_SHARED | MAP_FIXED)

static int fill_name(char *buf, int buflen) {
  FILE *f = fopen("/dev/urandom", "r");
  assert(f);
  ptrdiff_t read = fread(buf, sizeof(char), --buflen, f);
  if (feof(f) || ferror(f)) return -1;
  assert(read == buflen);
  buf[read] = 0;
  fclose(f);
  return 0;
}

static int open_shmem(char *name, ptrdiff_t len) {
  int fd = shm_open(name, O_CREAT | O_RDWR | O_EXCL, 0600);
  if (-1 == fd) return -1;
  if (-1 == ftruncate(fd, len)) return -1;
  return fd;
}

static void *offsetn(void *p, ptrdiff_t len, int n) {
  return (char *)p + n * len;
}

static void alloc3(ptrdiff_t len, int shmem_fd, void **p) {
  void *base = mmap(0, 3 * len, PROT, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#define F(p) (MAP_FAILED == (p))
  if (F(base)) goto FAIL;
  void *low = mmap(base, len, PROT, FLAG, shmem_fd, 0);
  assert(low == base); /* no need for `F(low)' here */
  void *mid = mmap(offsetn(low, len, 1), len, PROT, FLAG, shmem_fd, 0);
  if (F(mid)) goto FAIL_LOW;
  void *high = mmap(offsetn(base, len, 2), len, PROT, FLAG, shmem_fd, 0);
  if (F(high)) goto FAIL_MID;
#undef F
  *p = mid;
  return;
FAIL_MID:
  munmap(mid, len);
FAIL_LOW:
  munmap(low, len);
FAIL:
  *p = 0;
}

static bool lenchk(ptrdiff_t len) {
  return len > 0 && 0 == len % sysconf(_SC_PAGESIZE);
}

void *ringbuf_create(ptrdiff_t len) {
  assert(lenchk(len));
  char name[SHMEM_NAME_MAX];
  if (0 > fill_name(name, arraysize(name))) return 0;
  int fd = open_shmem(name, len);
  if (0 > fd) return 0;
  void *p;
  alloc3(len, fd, &p);
  close(fd);
  return p;
}

int ringbuf_destroy(void *buf, ptrdiff_t len) {
  assert(buf);
  assert(lenchk(len));
  return munmap(offsetn(buf, len, -1), 3 * len);
}
