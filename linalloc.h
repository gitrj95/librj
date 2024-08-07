#pragma once

#include "base.h"
#include "branch.h"

#define LINALLOC( n )                                                          \
  [[gnu::malloc]]                                                              \
  static inline void *linalloc##n( struct arena *a, long sz ) {                \
    ulong end  = (ulong)a->tl;                                                 \
    ulong offs = end - sz;                                                     \
    offs &= ~( n - 1 );                                                        \
    if( UNLIKELY( offs - (ulong)a->hd > end ) ) return 0;                      \
    a->tl = __builtin_assume_aligned( (void *)offs, n );                       \
    return a->tl;                                                              \
  }

struct arena {
  void *hd, *tl;
};

LINALLOC( 1 )
LINALLOC( 2 )
LINALLOC( 4 )
LINALLOC( 8 )
LINALLOC( 16 )
LINALLOC( 32 )
LINALLOC( 64 )
LINALLOC( 128 )
