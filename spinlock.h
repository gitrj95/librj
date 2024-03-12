/*
  This is an implementation of an optimized spinlock that has
  reasonable performance on a variety of hardware. To avoid paying the
  penalty for a RMW on each spin, we do a load first and only attempt
  a write if the load works i.e. the lock appears unlocked. There is
  no need to do any atomic comparisons, here, given we have two states
  (0 or 1 WLOG). The nanosleep is a heuristic hack sourced from Fedor
  Pikus. Without the sleep, the OS would think that the waiter of a
  lock is doing lots of work, so it would give it more time slices
  erroneously. Namely, a hardware `PAUSE' wouldn't necessarily inform
  the kernel.

  Spinlocks also compose nicely with other concurrency structures.
*/

#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdatomic.h>
#include <time.h>

#define SPINLOCK_INIT 0

typedef atomic_int spinlock;

static inline void spin_lock(spinlock *lock) {
  for (int i = 0; atomic_load_explicit(lock, memory_order_relaxed) ||
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
  atomic_store_explicit(lock, 0, memory_order_release);
}

#endif
