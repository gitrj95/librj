/*
  This is a derivative of Skeeto's with some small changes to avoid UB
  but maintain performance given a sufficiently deep pipeline. MSI
  provides a sequence of indices into [0, n) given some hash and n as
  some power of 2. The generated sequence has some useful
  properties. First, as the step is odd and the input is assumed a
  power of 2, they are co-prime. Namely, all elements of [0, n) will
  be touched exactly once in a sequence of indices of size n, after
  which, by the pigeonhole principle, we will have our first
  duplicate. Second, as the step size of the operation is both
  deterministic and derived from the hash, the generation of the
  (i+1)th element of the sequence is inlineable and predictable. The
  latter is of particular consequence, as a use case for this pattern
  is to generate collision indices for an open-addressing hash
  table. Normally, such double-hashing methods would provide no better
  cache complexity than a chaining implementation in the worst case,
  but as the step size is predictable, a decent hardware prefetcher
  should hide that cost (at least until parallel lookup is
  desired). This allows an array to trivially be turned into a fast
  hash table given some sentinel value that can disambiguate empty
  cells.
*/

#ifndef MSI_H
#define MSI_H

#include <assert.h>
#include <stdint.h>

static inline uint32_t msi(int exp, uint64_t hash, uint32_t i) {
  assert(-1 < exp);
  assert(31 > exp);
  uint32_t w = (1 << exp) - 1;
  uint32_t step = (uint32_t)((hash >> 34) >> (30 - exp)) | 1;
  return (i + step) & w;
}

#endif
