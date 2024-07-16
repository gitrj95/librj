#ifndef MSI_H
#define MSI_H

#include "base.h"

static inline uint msi( short lg, ulong h, uint i ) {
  uint w    = ( 1ul << lg ) - 1;
  uint step = ( ( h >> 32 ) >> ( 32 - lg ) ) | 1;
  return i + step & w;
}

#endif
