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
  test("set of first calls to msi");
  uint64_t h = hash(12);
  uint32_t j = (uint32_t)h, i = msi(EXP, h, j);
  expect(i != j, "index minted by msi different from seed");
  expect(i < 1 << EXP, "index bounded above by 1 << `EXP'");
  i = msi(EXP, UINT64_MAX, UINT32_MAX);
  expect(i < 1 << EXP, "index from maximal hash and index valid");
  i = msi(EXP, 0, 0);
  expect(i < 1 << EXP, "index from minimal hash and index valid");
  i = msi(0, h, j);
  expect(!i, "zero index on singleton array");
  die(msi(33, h, j), "`exp' above valid range");
  (void)msi(32, h, j);
  pass("maximum exponent");
  i = msi(0, h, (uint32_t)h);
  die(msi(-1, 0, 0), "negative `exp'");
}

void walk(void) {
  test("walk set of minted msi sequence");
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
  expect(count == 1 << EXP, "msi sequence of maximal size");
  for (int k = 0; k < (1 << EXP); ++k) assert(1 == seen[k]);
  pass("domain exhausted");
}

int main(void) {
  suite();
  init();
  walk();
  return 0;
}
