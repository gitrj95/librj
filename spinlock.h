#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdatomic.h>
#include <time.h>

#define SPINLOCK_NSPINS_BEFORE_SLEEP 8

typedef atomic_int spinlock_t;

enum { spinlock_success_lf, spinlock_success_no_lf, spinlock_error };

int init(spinlock_t *lock);

static inline void lock(spinlock_t lock[static 1]) {
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

static inline void unlock(spinlock_t lock[static 1]) {
  atomic_store_explicit(lock, 0, memory_order_release);
}

#endif
