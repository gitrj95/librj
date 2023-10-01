/*
  A KISS unit testing helper header. Its semantics can be trivially
  gleaned from examples, but n.b. One, the commands write to
  stderr. Two, the commands write via `fprintf`; namely, they are not
  re-entrant. When re-entrancy is needed, check runtime conditions
  with `assert` and call the helper commands once appropriate. Similar
  logic follows for parallel contexts (although POSIX guarantees its
  thread-safety). Three, `expect_abort` works by unwinding the stack
  when a SIGABRT is hit--e.g., via `assert`.

  Notably, there's no mocking support here, because mocking in C is
  complicated to generally implement, and any such solution I know of
  imposes restrictions on the mockable functions or exploits
  non-trivial details of the toolchain/runtime. Examples: 1) poisoning
  global offset table 2) textual replacement via unity build and some
  statically known, global notion of mockable functions 3) weak
  symbols via the linker, etc.

  Therefore, if mocking is required, I think a non-generic solution
  for the specific context is more useful.
*/

#ifndef TEST_H
#define TEST_H

#include <assert.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#if NCOLOR
#define KNRM ""
#define KRED ""
#define KGRN ""
#define KCYN ""
#define KYEL ""
#else
#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KCYN "\x1B[36m"
#define KYEL "\x1B[33m"
#endif

#define suite() fprintf(stderr, "%sSuite: %s%s\n", KCYN, __FILE__, KNRM)
#define test() fprintf(stderr, "%sTest: %s%s\n", KYEL, __func__, KNRM)
#define pass(str) fprintf(stderr, "  %sPass: %s%s\n", KGRN, (str), KNRM)
#define fail(str) \
  (fprintf(stderr, "  %sFail: %s\n%s", KRED, (str), KNRM), exit(EXIT_FAILURE))

#define expect_true(expr) \
  do {                    \
    bool cond__ = (expr); \
    if (cond__)           \
      pass(#expr);        \
    else                  \
      fail(#expr);        \
  } while (0)

#define expect_abort(expr)                         \
  do {                                             \
    void (*old__)(int) = signal(SIGABRT, unwind_); \
    if (!setjmp(frame_)) (void)(expr);             \
    if (fail_)                                     \
      pass(#expr);                                 \
    else                                           \
      fail(#expr);                                 \
    fail_ = 0;                                     \
    signal(SIGABRT, old__);                        \
  } while (0)

sig_atomic_t volatile fail_;
jmp_buf frame_;

void unwind_(int sig) {
  assert(SIGABRT == sig);
  fail_ = 1;
  longjmp(frame_, 1);
}

#endif
