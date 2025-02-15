#ifndef LIBRJ_LINALLOC_H
#define LIBRJ_LINALLOC_H

#include "base.h"

#define LINALLOC( n )                                                          \
  [[gnu::malloc]]                                                              \
  static inline void *linalloc##n( struct arena *a, long sz ) {                \
    ulong offs = (ulong)a->tl - sz;                                            \
    offs &= ~( n - 1 );                                                        \
    if( offs - (ulong)a->hd > (ulong)a->tl - (ulong)a->hd ) return 0;          \
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

#endif
