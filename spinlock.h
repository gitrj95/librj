#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <assert.h>
#include <stdatomic.h>

typedef atomic_int spinlock_t;
static_assert(ATOMIC_INT_LOCK_FREE, "atomic_int not lf");

void init(spinlock_t lock[static 1]);
void lock(spinlock_t lock[static 1]);
void unlock(spinlock_t lock[static 1]);

#endif
