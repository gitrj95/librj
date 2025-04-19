#ifndef LIBRJ_LINALLOC_H
#define LIBRJ_LINALLOC_H

#include "base.h"

#ifndef __SANITIZE_ADDRESS__

#define LINALLOC( n )                                                                              \
  [[gnu::malloc]]                                                                                  \
  static inline void *linalloc##n( struct arena *a, long sz )                                      \
  {                                                                                                \
    ulong offs = (ulong)a->tl - sz;                                                                \
    offs &= ~( n - 1 );                                                                            \
    if( offs - (ulong)a->hd > (ulong)a->tl - (ulong)a->hd ) {                                      \
      return 0;                                                                                    \
    };                                                                                             \
    a->tl = __builtin_assume_aligned( (void *)offs, n );                                           \
    return a->tl;                                                                                  \
  }

#else

#include <sanitizer/asan_interface.h>

#define LINALLOC( n )                                                                              \
  [[gnu::malloc]]                                                                                  \
  static inline void *linalloc##n( struct arena *a, long sz )                                      \
  {                                                                                                \
    sz += 16;                                                                                      \
    ulong offs = (ulong)a->tl - sz;                                                                \
    offs &= ~( n - 1 );                                                                            \
    if( offs - (ulong)a->hd > (ulong)a->tl - (ulong)a->hd ) {                                      \
      return 0;                                                                                    \
    };                                                                                             \
    ASAN_POISON_MEMORY_REGION( (schar *)offs + sz, (schar *)a->tl - (schar *)offs + sz );          \
    a->tl = __builtin_assume_aligned( (void *)offs, n );                                           \
    return a->tl;                                                                                  \
  }

#endif

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
LINALLOC( 4096 )
LINALLOC( 2097152 )
LINALLOC( 1073741824 )

#endif
