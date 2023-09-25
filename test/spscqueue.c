#include "../spscqueue.c"
#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include "../test.h"

#define BERTHA 10000

static int big_bertha[100];
atomic_uint nops;

struct opt {
  spscqueue *q;
  int n;
  int spin;
};

void *produce(void *ctx) {
  struct opt *opt = ctx;
  for (int i = 1; i < 1 + opt->n; ++i) {
    for (int j = 0; j < opt->spin; ++j) {}
    spscqueue_push(opt->q, &i);
    atomic_fetch_add_explicit(&nops, 1, memory_order_relaxed);
  }
  return 0;
}

void *consume(void *ctx) {
  struct opt *opt = ctx;
  for (int i = 1; i < 1 + opt->n; ++i) {
    void const *p = spscqueue_pop(opt->q);
    atomic_fetch_add_explicit(&nops, 1, memory_order_relaxed);
    assert(i == *(int const *)p);
    for (int j = 0; j < opt->spin; ++j) {}
  }
  return 0;
}

void init(void) {
  test();
  spscqueue q0, q1;
  int a[1000] = {0};
  spscqueue_init_from_static(&q0, a);
  expect_true(sizeof(a) == (char *)q0.tl - (char *)q0.hd);
  float *f = malloc(1000 * sizeof(*f));
  spscqueue_init(&q1, f, f + 1000, sizeof(*f));
  expect_true(sizeof(a) == (char *)q0.tl - (char *)q0.hd);
  free(f);
  expect_abort(spscqueue_init(0, a, a + 20, 10));
  expect_abort(spscqueue_init(&q0, 0, a + 20, 10));
  expect_abort(spscqueue_init(&q0, a, 0, 10));
  expect_abort(spscqueue_init(&q0, a, a + 20, 0));
  expect_abort(spscqueue_init(&q0, a + 19, a + 3, 0));
  expect_abort(spscqueue_init(&q0, a, a + 1000, 99));
}

void trypush(void) {
  test();
  spscqueue q;
  int a[2] = {0};
  spscqueue_init_from_static(&q, a);
  int ins = 11;
  expect_true(spscqueue_trypush(&q, &ins));
  expect_true(!spscqueue_trypush(&q, &ins));
  expect_abort(spscqueue_trypush(0, &ins));
  expect_abort(spscqueue_trypush(&q, 0));
}

void trypop(void) {
  test();
  spscqueue q;
  int a[10000] = {0};
  spscqueue_init_from_static(&q, a);
  int ins = 11;
  spscqueue_trypush(&q, &ins);
  void const *top;
  expect_true((top = spscqueue_trypop(&q)));
  expect_true(ins == *(int const *)top);
  expect_true(!spscqueue_trypop(&q));
  expect_abort(spscqueue_trypop(0));
}

void mintnrun_thrds(int pspin, int cspin) {
  spscqueue q;
  spscqueue_init(&q, big_bertha,
                 big_bertha + sizeof(big_bertha) / sizeof(big_bertha[0]),
                 sizeof(big_bertha[0]));
  struct opt pctx = {.n = BERTHA, .q = &q, .spin = pspin};
  struct opt cctx = {.n = BERTHA, .q = &q, .spin = cspin};
  pthread_t producer, consumer;
  pthread_create(&producer, 0, produce, &pctx);
  pthread_create(&consumer, 0, consume, &cctx);
  while (atomic_load_explicit(&q.r, memory_order_consume) !=
         atomic_load_explicit(&q.w, memory_order_consume)) {}
  pthread_join(producer, 0);
  pthread_join(consumer, 0);
  assert(2 * BERTHA == nops);
  nops = 0;
}

void pushnpop(void) {
  test();
  mintnrun_thrds(0, 0);
  expect_true(1 && "Zero-latency SPSC");
  mintnrun_thrds(0, 10000);
  expect_true(1 && "Slow-consumer SPSC");
  mintnrun_thrds(10000, 0);
  expect_true(1 && "Slow-producer SPSC");
  int primes[] = {19, 37};
  mintnrun_thrds(primes[0], primes[1]);
  expect_true(1 && "Dual-latency SPSC");
}

int main(void) {
  suite();
  init();
  trypush();
  trypop();
  pushnpop();
}
