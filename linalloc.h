#ifndef LINALLOC_H
#define LINALLOC_H

#include "base.h"

struct arena {
  void *hd, *tl;
};

#define LINALLOC( n )                                                \
  [[gnu::malloc]]                                                    \
  static inline void *linalloc##n( struct arena *a, long sz ) {      \
    ulong end  = (ulong)a->tl;                                       \
    ulong offs = end - sz;                                           \
    offs &= ~( n - 1 );                                              \
    if( offs - (ulong)a->hd > end ) return 0;                        \
    a->tl = __builtin_assume_aligned( (void *)offs, n );             \
    return a->tl;                                                    \
  }

LINALLOC( 1 )
LINALLOC( 2 )
LINALLOC( 4 )
LINALLOC( 8 )
LINALLOC( 16 )
LINALLOC( 32 )
LINALLOC( 64 )
LINALLOC( 128 )

#endif
