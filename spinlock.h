#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdatomic.h>
#include <time.h>

#define SPINLOCK_NSPINS_BEFORE_SLEEP 8

typedef atomic_int spinlock_t;

#define SPIN_INIT 0

static inline void spin_lock(spinlock_t *lock) {
  for (int i = 0; atomic_load_explicit(lock, memory_order_acquire) ||
                  atomic_exchange_explicit(lock, 1, memory_order_acquire);
       i++) {
    if (SPINLOCK_NSPINS_BEFORE_SLEEP == i) {
      static struct timespec ts = {.tv_sec = 0, .tv_nsec = 1};
      nanosleep(&ts, 0);
      i = 0;
    }
  }
}

static inline void spin_unlock(spinlock_t *lock) {
  atomic_store_explicit(lock, 0, memory_order_release);
}

#endif
