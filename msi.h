#ifndef MSI_H
#define MSI_H

#include "base.h"

[[gnu::const]]
static inline uint msi( short lg, ulong h, uint i ) {
  uint w    = ( 1 << lg ) - 1;
  uint step = ( ( h >> 32 ) >> ( 32 - lg ) ) | 1;
  return i + step & w;
}

#endif
