/*
  NOTE: this implementation is not secure. Namely, the shared-memory
  handle used to synthesize the ring buffer is subject to a race with
  a potential attacker.
*/

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ringbuf.h"

#define SHMEM_SEED_FILENAME "/dev/urandom"
#define SHMEM_NAME_LEN 10
#define SHMEM_RETRY_MAX 10

#define arraysize(a) ((ptrdiff_t)(sizeof(a) / sizeof((a)[0])))

static ptrdiff_t get_padding(ptrdiff_t itemsz, ptrdiff_t nitems) {
  assert(0 < itemsz);
  assert(0 < nitems);
  ptrdiff_t pagesz = sysconf(_SC_PAGE_SIZE);
  assert(0 == pagesz % itemsz);
  assert(0 == (pagesz & (pagesz - 1)));
  size_t usize = itemsz * nitems;
  return -usize & (pagesz - 1); /* NOTE: relies on two's complement */
}

static int fill_name(char *name, int namelen, char *alphabet,
                     ptrdiff_t alphabetlen, FILE *f) {
  assert(name);
  assert(1 < namelen);
  assert(f);
  for (int i = 0; i < alphabetlen; ++i) assert('/' != alphabet[i]);
  *name++ = '/';
  --namelen;
  name[fread(name, sizeof(char), --namelen, f)] = 0;
  if (ferror(f)) return -1;
  for (int i = 0; i < namelen; ++i)
    name[i] = alphabet[(unsigned char)name[i] % alphabetlen];
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
  assert(p);
  assert(0 < len);
  return (char *)p + n * len;
}

static void *allocblock(void *base, ptrdiff_t len, int off, int shmem_fd) {
  assert(base);
  assert(0 < len);
  assert(-1 < shmem_fd);
  char *p = offsetn(base, len, off);
  void *allocp =
      mmap(p, len, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, shmem_fd, 0);
  if (MAP_FAILED == allocp) return 0;
  if (base) assert(p == allocp);
  return allocp;
}

static void *alloc3(ptrdiff_t blocklen, int shmem_fd) {
  assert(0 < blocklen);
  assert(-1 < shmem_fd);
  void *base =
      mmap(0, 3 * blocklen, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (MAP_FAILED == base) return 0;
  void *low = allocblock(base, blocklen, 0, shmem_fd);
  if (!low) goto fail;
  void *mid = allocblock(base, blocklen, 1, shmem_fd);
  if (!mid) goto fail_low;
  void *high = allocblock(base, blocklen, 2, shmem_fd);
  if (!high) goto fail_mid;
  return mid;
fail_mid:
  munmap(mid, blocklen);
fail_low:
  munmap(low, blocklen);
fail:
  return 0;
}

bool ringbuf_create(struct ringbuf *rb, ptrdiff_t itemsz, ptrdiff_t minitems) {
  assert(rb);
  assert(0 < itemsz);
  assert(0 < minitems);
  struct ringbuf ret = {0};
  char name[SHMEM_NAME_LEN];
  FILE *f = fopen(SHMEM_SEED_FILENAME, "rb");
  assert(f);
  static char *alphabet = "abcde";
  ptrdiff_t alphabetlen = strlen(alphabet);
  ptrdiff_t padding = get_padding(itemsz, minitems);
  ptrdiff_t alloclen = padding + itemsz * minitems;
  ret.nitems = alloclen / itemsz;
  int fd = -1;
  for (int i = 0; i < SHMEM_RETRY_MAX; ++i) {
    if (0 > fill_name(name, arraysize(name), alphabet, alphabetlen, f))
      goto fail_f;
    if (-1 < (fd = open_shmem(name, alloclen))) break;
  }
  if (0 > fd) goto fail_f;
  ret.p = alloc3(alloclen, fd);
  close(fd);
fail_f:
  fclose(f);
  *rb = ret;
  return ret.p;
}

bool ringbuf_destroy(struct ringbuf rb, ptrdiff_t itemsz) {
  assert(rb.p);
  assert(0 < rb.nitems);
  assert(0 < itemsz);
  ptrdiff_t plen = itemsz * rb.nitems;
  return !munmap(offsetn(rb.p, plen, -1), 3 * plen);
}
