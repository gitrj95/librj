#include "spinlock.h"
#include <stdatomic.h>
#include <time.h>

#define SPIN_BEFORE_SLEEP_MAX 8

void init(spinlock_t lock[static 1]) {
  *lock = 0;
}

void lock(spinlock_t lock[static 1]) {
  for (register int i = 0;
       atomic_load_explicit(lock, memory_order_acquire) ||
       atomic_exchange_explicit(lock, 1, memory_order_acquire);
       i++) {
    if (SPIN_BEFORE_SLEEP_MAX == i) {
      static struct timespec ts = {.tv_sec = 0, .tv_nsec = 1};
      nanosleep(&ts, 0);
      i = 0;
    }
  }
}

void unlock(spinlock_t lock[static 1]) {
  atomic_store_explicit(lock, 0, memory_order_release);
}
