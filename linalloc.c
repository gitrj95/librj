#include "linalloc.h"

#include <sanitizer/asan_interface.h>
#include <stddef.h>

#define unlikely(e) __builtin_expect(!!(e), 0)

#if __has_feature(address_sanitizer)
#define __LIBRJ_LINALLOC_REDZONE_LEN (alignof(max_align_t))
#else
#define __LIBRJ_LINALLOC_REDZONE_LEN 0
#endif

#define IMPL(n)                                                      \
  void *linalloc##n(struct arena *a, long sz)                        \
  {                                                                  \
    long hd = (long)a->hd;                                           \
    long tl = (long)a->tl;                                           \
    long pad = -hd & (n) - 1;                                        \
    long avail = tl - hd - pad;                                      \
                                                                     \
    if (unlikely(avail < 0)) {                                       \
      return 0;                                                      \
    }                                                                \
    char *p = (char *)hd + pad;                                      \
    a->hd = p + sz + __LIBRJ_LINALLOC_REDZONE_LEN;                   \
    ASAN_POISON_MEMORY_REGION(p + sz, __LIBRJ_LINALLOC_REDZONE_LEN); \
    return __builtin_assume_aligned(p, (n));                         \
  }

IMPL(1)
IMPL(2)
IMPL(4)
IMPL(8)
IMPL(16)
IMPL(32)
IMPL(64)
IMPL(128)
IMPL(4096)
