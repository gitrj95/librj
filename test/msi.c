#include "../msi.h"
#include <stdint.h>
#include "../test.h"

#define EXP 12

uint64_t hash(uint64_t x) {
  x ^= x << 13;
  x *= 111111111111111;
  x ^= x >> 31;
  return x;
}

void init(void) {
  test();
  uint64_t h = hash(12);
  uint32_t i = msi(EXP, h, (uint32_t)h);
  expect_true(0 < i);
  expect_true(i < 1 << EXP);
  i = msi(EXP, UINT64_MAX, UINT32_MAX);
  expect_true(0 < i);
  expect_true(i < 1 << EXP);
  i = msi(EXP, 0, 0);
  expect_true(0 < i);
  expect_true(i < 1 << EXP);
  i = msi(0, h, (uint32_t)h);
  expect_true(!i);
  expect_abort(msi(33, h, (uint32_t)h));
  (void)msi(32, h, (uint32_t)h);
  pass("Maximum exponent");
  i = msi(0, h, (uint32_t)h);
  expect_true(!msi(0, h, i));
  expect_abort(msi(-1, 0, 0));
}

void walk(void) {
  test();
  int seen[1 << EXP] = {0};
  uint64_t h = hash(12);
  uint32_t j, i;
  int count = 0;
  j = i = msi(EXP, h, (uint32_t)h);
  do {
    assert(!seen[i]);
    seen[i] += 1;
    ++count;
  } while (j != (i = msi(EXP, h, i)));
  expect_true(count == 1 << EXP);
  for (int k = 0; k < (1 << EXP); ++k) assert(1 == seen[k]);
  pass("Domain exhausted");
}

int main(void) {
  suite();
  init();
  walk();
}
