#ifndef MSI_H
#define MSI_H

#include <assert.h>
#include <stdint.h>

static inline int32_t msi_next(int exp, uint64_t hash, int32_t i);

static inline int32_t msi_init(int exp, uint64_t hash) {
  return msi_next(exp, hash, hash & 0xcfffffff);
}

static inline int32_t msi_next(int exp, uint64_t hash, int32_t i) {
  assert(exp > 0);
  assert(exp < 31);
  assert(i > -1);
  uint32_t m = (1 << exp) - 1;
  uint32_t s = ((hash >> 34) >> (30 - exp)) | 1;
  assert(s % 2);
  assert(s < (1 << exp));
  return (i + s) & m;
}

#endif
