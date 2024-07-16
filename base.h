#ifndef BASE_H
#define BASE_H

#include <assert.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>

typedef unsigned long  ulong;
typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef signed char    schar;

static_assert( sizeof( long ) == 8 );
static_assert( sizeof( int ) == 4 );
static_assert( sizeof( short ) == 2 );

static_assert( sizeof( long ) == sizeof( size_t ) );
static_assert( sizeof( long ) == sizeof( intptr_t ) );
static_assert( sizeof( long ) == sizeof( ptrdiff_t ) );

extern long  *d8__;
extern int   *d4__;
extern short *d2__;
extern schar *d1__;

static_assert( atomic_is_lock_free( d8__ ) );
static_assert( atomic_is_lock_free( d4__ ) );
static_assert( atomic_is_lock_free( d2__ ) );
static_assert( atomic_is_lock_free( d1__ ) );

#endif
