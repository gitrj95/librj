#ifndef LIBRJ_MSI_H
#define LIBRJ_MSI_H

#include "base.h"

static inline int msi( int lg, ulong h, int i ) {
  uint m = ( 1u << lg ) - 1;
  uint s = ( h >> ( 64 - lg ) ) | 1;
  return i + s & m;
}

#endif
