#include "spinlock.h"
#include <stdatomic.h>

int init(spinlock_t *lock) {
  if (!lock) return spinlock_error;
  *lock = 0;
  return ATOMIC_INT_LOCK_FREE ? spinlock_success_lf : spinlock_success_no_lf;
}
