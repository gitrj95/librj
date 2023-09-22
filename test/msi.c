#include "../msi.h"
#include <stdint.h>
#include "../ssize.h"
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
  int32_t i = msi_init(EXP, h);
  expect_true(0 < i);
  expect_true(i < 1 << EXP);
  i = msi_init(EXP, UINT64_MAX);
  expect_true(0 < i);
  expect_true(i < 1 << EXP);
  i = msi_init(EXP, 0);
  expect_true(0 < i);
  expect_true(i < 1 << EXP);
}

void next(void) {
  test();
  int seen[1 << EXP] = {0};
  uint64_t h = hash(12);
  int32_t i = msi_init(EXP, h);
  for (int j = 0; j < (1 << EXP); ++j, i = msi_next(EXP, h, i)) {
    assert(!seen[i]);
    seen[i] += 1;
  }
  expect_true(1 && "No cycles");
  for (int k = 0; k < (1 << EXP); ++k) assert(1 == seen[k]);
  expect_true(1 && "Domain exhausted");
  expect_abort(msi_next(-1, 0, 0));
  expect_abort(msi_next(1, 0, -1));
  expect_abort(msi_next(32, 0, -1));
}

int main(void) {
  suite();
  init();
  next();
}
