#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <assert.h>
#include <stdatomic.h>
#include <time.h>

#define SPINLOCK_INIT 0

typedef atomic_int spinlock;

static inline void spin_lock(spinlock *lock) {
  assert(lock && "Spinlock assumed non-null.");
  for (int i = 0; atomic_load_explicit(lock, memory_order_acquire) ||
                  atomic_exchange_explicit(lock, 1, memory_order_acquire);
       ++i) {
    if (8 == i) {
      static struct timespec ts = {.tv_sec = 0, .tv_nsec = 1};
      nanosleep(&ts, 0);
      i = 0;
    }
  }
}

static inline void spin_unlock(spinlock *lock) {
  assert(lock && "Spinlock assumed non-null.");
  atomic_store_explicit(lock, 0, memory_order_release);
}

#endif
