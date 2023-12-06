#include "../arraysize.h"
#include "../test.h"

struct a {
  int b;
  float c;
  char d;
};
static_assert(12 == sizeof(struct a));

struct a buf[11];

void buffers(void) {
  test("general buffers");
  expect(11 == arraysize(buf), "static array of structs");
  char buf1[991];
  expect(991 == arraysize(buf1), "automatic array of primitives");
}

void sliterals(void) {
  test("string literals");
  expect(6 == arraysize("apple"), "string literal size, including nul");
}

int main(void) {
  suite();
  buffers();
  sliterals();
}
