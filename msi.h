#ifndef MSI_H
#define MSI_H

#include <assert.h>
#include <stdint.h>

static inline int64_t msi_next(int exp, uint64_t hash, int64_t i);

static inline int64_t msi_init(int exp, uint64_t hash) {
  return msi_next(exp, hash, hash & (((uint64_t)1 << 63) - 1));
}

static inline int64_t msi_next(int exp, uint64_t hash, int64_t i) {
  assert(exp < 63 && "Length is bounded above by -1 + 2^63 and is required to "
                     "be a power of 2.");
  uint64_t m = ((uint64_t)1 << exp) - 1;
  uint64_t step = (hash >> (64 - exp)) | 1;
  return (i + step) & m;
}

#endif
