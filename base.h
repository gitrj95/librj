#ifndef LIBRJ_BASE_H
#define LIBRJ_BASE_H

#include <assert.h>
#include <limits.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>

static_assert(CHAR_BIT == 8);

static_assert(sizeof(long) == 8);
static_assert(sizeof(int) == 4);
static_assert(sizeof(short) == 2);

static_assert(sizeof(long) == sizeof(size_t));
static_assert(sizeof(long) == sizeof(intptr_t));
static_assert(sizeof(long) == sizeof(ptrdiff_t));

static_assert(ATOMIC_POINTER_LOCK_FREE == 2);
static_assert(ATOMIC_LONG_LOCK_FREE == 2);
static_assert(ATOMIC_INT_LOCK_FREE == 2);
static_assert(ATOMIC_SHORT_LOCK_FREE == 2);
static_assert(ATOMIC_CHAR_LOCK_FREE == 2);

#endif
