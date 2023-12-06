#include "../spinlock.h"
#include <pthread.h>
#include "../test.h"

#define LAST 123456

int non_atomic;

void *add(void *ctx) {
  spinlock *mtx = ctx;
  for (int i = 0; i < LAST; ++i) {
    spin_lock(mtx);
    ++non_atomic;
    spin_unlock(mtx);
  }
  return 0;
}

int im_lazy(int nthreads) {
  int i = 0;
  for (; i < LAST; ++i) {};
  return nthreads * i;
}

void sum(void) {
  test("linearized access");
  pthread_t t1, t2;
  spinlock mtx = SPINLOCK_INIT;
  pthread_create(&t1, 0, add, &mtx);
  pthread_create(&t2, 0, add, &mtx);
  add(&mtx);
  pthread_join(t1, 0);
  pthread_join(t2, 0);
  expect(im_lazy(3) == non_atomic, "non-atomic counter incremented race-free");
}

int main(void) {
  suite();
  sum();
}
