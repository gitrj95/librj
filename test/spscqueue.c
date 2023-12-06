#include "../spscqueue.c"
#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include "../test.h"

#define BERTHA 10000

int big_bertha[100];
atomic_int nops;

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
    int const *p = spscqueue_pop(opt->q);
    atomic_fetch_add_explicit(&nops, 1, memory_order_relaxed);
    assert(i == *p);
    for (int j = 0; j < opt->spin; ++j) {}
  }
  return 0;
}

void init(void) {
  test("initialization");
  spscqueue q0, q1;
  int a[1000] = {0};
  spscqueue_init2(&q0, a);
  expect(sizeof(a) == (char *)q0.tl - (char *)q0.hd,
         "head and tail pointers spaced by the length of the static buffer");
  float *f = malloc(1000 * sizeof(*f));
  spscqueue_init(&q1, f, 1000, sizeof(*f));
  expect(sizeof(a) == (char *)q0.tl - (char *)q0.hd,
         "head and tail pointers spaced by the length of the dynamic buffer");
  free(f);
  die(spscqueue_init(0, a, 20, 10), "null `q'");
  die(spscqueue_init(&q0, 0, 20, 10), "null input buffer");
  die(spscqueue_init(&q0, a, 0, 10), "zero buffer length");
  die(spscqueue_init(&q0, a, 20, 0), "zero item size");
  die(spscqueue_init(&q0, a, 1000, 99),
      "buffer length not a multiple of item size");
  die(spscqueue_init(&q0, a, sizeof(*f), sizeof(*f)),
      "buffer holds exactly one item");
}

void trypush(void) {
  test("writer");
  spscqueue q;
  int a[2] = {0};
  spscqueue_init2(&q, a);
  int ins = 11;
  expect(spscqueue_trypush(&q, &ins), "happy-path push");
  expect(!spscqueue_trypush(&q, &ins), "push does not clobber reader's slot");
  die(spscqueue_trypush(0, &ins), "null `q'");
  die(spscqueue_trypush(&q, 0), "null value to write");
}

void trypop(void) {
  test("reader");
  spscqueue q;
  int a[10000] = {0};
  spscqueue_init2(&q, a);
  int ins = 11;
  spscqueue_trypush(&q, &ins);
  void const *top;
  expect((top = spscqueue_trypop(&q)), "happy-path pop");
  expect(ins == *(int const *)top, "value popped matches written value");
  expect(!spscqueue_trypop(&q), "pop does not clobber writer's slot");
  die(spscqueue_trypop(0), "null `q'");
}

void mintnrun_thrds(int pspin, int cspin) {
  spscqueue q;
  spscqueue_init2(&q, big_bertha);
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
  test("race-free with differing push/pop rates");
  mintnrun_thrds(0, 0);
  pass("Zero-latency SPSC");
  mintnrun_thrds(0, 10000);
  pass("Slow-consumer SPSC");
  mintnrun_thrds(10000, 0);
  pass("Slow-producer SPSC");
  int primes[] = {19, 37};
  mintnrun_thrds(primes[0], primes[1]);
  pass("Dual-latency SPSC");
}

int main(void) {
  suite();
  init();
  trypush();
  trypop();
  pushnpop();
}
