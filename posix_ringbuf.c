/*
  NOTE: this implementation is not secure. Namely, the shared-memory
  handle used to synthesize the ring buffer is subject to a race with
  a potential attacker.
*/

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "arraysize.h"
#include "ringbuf.h"

#define RAND_SEED_FILENAME "/dev/urandom"
#define SHMEM_NAMESZ 10
#define RETRY_MAX 3

#define PROT (PROT_READ | PROT_WRITE)
#define FLAG (MAP_SHARED | MAP_FIXED)

static int fill_name(char *buf, int bufcnt, char *alphabet, int alphabetcnt,
                     FILE *f) {
  assert(buf);
  assert(1 < bufcnt);
  assert(alphabet);
  assert(1 < alphabetcnt);
  for (int i = 0; i < alphabetcnt; ++i) assert('/' != alphabet[i]);
  assert(f);
  *buf++ = '/';
  --bufcnt;
  buf[fread(buf, sizeof(char), --bufcnt, f)] = 0;
  if (ferror(f)) return -1;
  for (int i = 0; i < bufcnt; ++i)
    buf[i] = alphabet[(unsigned char)buf[i] % alphabetcnt];
  return 0;
}

static int open_shmem(char *name, ptrdiff_t len) {
  assert(name);
  assert('/' == name[0]);
  for (ptrdiff_t i = 1; i < (ptrdiff_t)strlen(name); ++i)
    assert('/' != name[i]);
  assert(0 < len);
  int fd = shm_open(name, O_CREAT | O_RDWR | O_EXCL, 0600);
  if (-1 == fd) return -1;
  if (-1 == ftruncate(fd, len)) return -1;
  shm_unlink(name);
  return fd;
}

static void *offsetn(void *p, ptrdiff_t len, int n) {
  return (char *)p + n * len;
}

static void *alloc3(ptrdiff_t len, int shmem_fd) {
  void *base = mmap(0, 3 * len, PROT, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#define F(p) (MAP_FAILED == (p))
  if (F(base)) return 0;
  void *low = mmap(base, len, PROT, FLAG, shmem_fd, 0);
  assert(low == base); /* NOTE: no need for `F(low)' here */
  void *mid = mmap(offsetn(low, len, 1), len, PROT, FLAG, shmem_fd, 0);
  if (F(mid)) goto CLEANUP_low;
  void *high = mmap(offsetn(base, len, 2), len, PROT, FLAG, shmem_fd, 0);
  if (F(high)) goto CLEANUP_mid;
#undef F
  return mid;
CLEANUP_mid:
  munmap(mid, len);
CLEANUP_low:
  munmap(low, len);
  return 0;
}

static bool lenchk(ptrdiff_t len) {
  return len > 0 && 0 == len % sysconf(_SC_PAGESIZE);
}

void *ringbuf_create(ptrdiff_t len) {
  assert(lenchk(len));
  void *p = 0;
  char name[SHMEM_NAMESZ];
  FILE *f = fopen(RAND_SEED_FILENAME, "rb");
  assert(f);
  int fd = -1;
  for (int i = 0; i < RETRY_MAX; ++i) {
    static char al[] = {'a', 'b', 'c', 'd', 'e', '1', '2', '3', '4', '5'};
    if (0 > fill_name(name, arraysize(name), al, arraysize(al), f))
      goto CLEANUP_f;
    if (-1 < (fd = open_shmem(name, len))) break;
  }
  if (0 > fd) goto CLEANUP_f;
  p = alloc3(len, fd);
  close(fd);
CLEANUP_f:
  fclose(f);
  return p;
}

int ringbuf_destroy(void *buf, ptrdiff_t len) {
  assert(buf);
  assert(lenchk(len));
  return munmap(offsetn(buf, len, -1), 3 * len);
}
