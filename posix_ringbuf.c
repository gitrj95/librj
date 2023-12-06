#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "arraysize.h"
#include "ringbuf.h"

#define SHMEM_NAME_MAX 10
static_assert(SHMEM_NAME_MAX < INT_MAX);

#define LENCHK(len) assert((len) > 0 && 0 == (len) % sysconf(_SC_PAGESIZE));

#define PROT (PROT_READ | PROT_WRITE)
#define FLAG (MAP_SHARED | MAP_FIXED)

#define LOW(p, len) p
#define MID(p, len) ((char *)(p) + (len))
#define HIGH(p, len) ((char *)(p) + 2 * (len))

static void fill_name(char *buf, int buflen) {
  FILE *f = fopen("/dev/urandom", "r");
  assert(f);
  ptrdiff_t read = fread(buf, sizeof(char), -1 + buflen, f);
  buf[read] = 0;
  fclose(f);
}

static int open_shmem(char *name, ptrdiff_t len) {
  int fd = shm_open(name, O_CREAT | O_RDWR | O_EXCL, 0600);
  if (-1 == fd) return -1;
  if (-1 == ftruncate(fd, len)) return -1;
  return fd;
}

static void alloc3(ptrdiff_t len, int shmem_fd, void **p) {
  void *base = mmap(0, 3 * len, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  void *low = mmap(LOW(base, len), len, PROT, FLAG, shmem_fd, 0);
  assert(low == base);
  void *mid = mmap(MID(base, len), len, PROT, FLAG, shmem_fd, 0);
  void *high = mmap(HIGH(base, len), len, PROT, FLAG, shmem_fd, 0);
#define _(p) (MAP_FAILED == (p))
  if (0 == _(low) + _(mid) + _(high))
#undef _
    *p = mid;
  else
    *p = 0;
}

void *ringbuf_create(ptrdiff_t len) {
  LENCHK(len);
  char name[SHMEM_NAME_MAX];
  fill_name(name, arraysize(name));
  int fd = open_shmem(name, len);
  if (0 > fd) return 0;
  void *p;
  alloc3(len, fd, &p);
  close(fd);
  return p;
}

int ringbuf_destroy(void *buf, ptrdiff_t len) {
  assert(buf);
  LENCHK(len);
  return munmap((char *)buf - len, 3 * len);
}
