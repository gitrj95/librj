#include "../spscqueue.c"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include "../test.h"

#define BERTHA 1'000'000

static int big_bertha[BERTHA];

struct opt {
  spscqueue *q;
  int n;
  int sleep;
};

void produce(void *ctx) {
  struct opt *opt = ctx;
  for (int i = 1; i < opt->n; ++i) {
    struct timespec ts = {.tv_sec = 0, .tv_nsec = opt->sleep};
    nanosleep(&ts, 0);
    spscqueue_push(opt->q, &i);
  }
}

void *consume(void *ctx) {
  struct opt *opt = ctx;
  for (int i = 1; i < opt->n; ++i) {
    struct timespec ts = {.tv_sec = 0, .tv_nsec = opt->sleep};
    void const *p = spscqueue_pop(opt->q);
    assert(i == *(int const *)p);
    nanosleep(&ts, 0);
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

void pushnpop(void) {
  test();
  spscqueue q;
  spscqueue_init(&q, big_bertha, big_bertha + BERTHA, sizeof(big_bertha[0]));
  struct opt ctx = {.n = BERTHA, .q = &q, .sleep = 0};
  pthread_t consumer;
  pthread_create(&consumer, 0, consume, &ctx);
  produce(&ctx);
  pthread_join(consumer, 0);
  expect_true(1 && "Zero-latency SPSC");
  struct opt slow_ctx = (struct opt){.n = BERTHA, .q = &q, .sleep = 1};
  pthread_create(&consumer, 0, consume, &slow_ctx);
  produce(&ctx);
  pthread_join(consumer, 0);
  expect_true(1 && "Slow-consumer SPSC");
  pthread_create(&consumer, 0, consume, &slow_ctx);
  produce(&ctx);
  pthread_join(consumer, 0);
  pthread_create(&consumer, 0, consume, &ctx);
  produce(&slow_ctx);
  pthread_join(consumer, 0);
  expect_true(1 && "Slow-producer SPSC");
  int primes[] = {19, 37};
  ctx.sleep = primes[0];
  slow_ctx.sleep = primes[1];
  pthread_create(&consumer, 0, consume, &slow_ctx);
  produce(&ctx);
  pthread_join(consumer, 0);
  expect_true(1 && "Dual-latency SPSC");
}

int main(void) {
  suite();
  init();
  trypush();
  trypop();
  pushnpop();
}
