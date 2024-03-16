/*
  A KISS testing helper header for POSIX systems. It is reentrant,
  supports terminal colors, and traps on failure. Executables using
  this header can be easily run under gdb in both the death and
  expected-value cases. lldb does not work reliably, however
  (following forks in child processes does not work reliably in lldb).

  The API is trivial and may be gleaned from examples, and the string
  "keys" make it harder to semantically duplicate test cases.
*/

#ifndef TEST_H
#define TEST_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

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

#define INVALID_POSIX_RC UINT8_MAX

#define pr(fd, fmt, clr)                                                   \
  do {                                                                     \
    char buf__[BUFSIZ];                                                    \
    ptrdiff_t len__ = snprintf(buf__, BUFSIZ, "%s" fmt "%s\n", clr, KNRM); \
    assert(len__ == write((fd), buf__, len__));                            \
  } while (0)

#define suite() pr(STDOUT_FILENO, "Suite: " __FILE__, KCYN)
#define test(slit) pr(STDOUT_FILENO, " Test: " slit, KYEL)
#define pass(slit) pr(STDOUT_FILENO, "  Pass: " slit, KGRN)
#define fail(slit) pr(STDERR_FILENO, "  Fail: " slit, KRED)
#define trap() __builtin_trap()

#define expect(expr, slit) \
  do {                     \
    _Bool cond__ = (expr); \
    if (cond__)            \
      pass(slit);          \
    else {                 \
      fail(slit);          \
      trap();              \
    }                      \
  } while (0)

#define die(expr, slit)                        \
  do {                                         \
    pid_t chld__;                              \
    int rc__ = -1;                             \
    if (-1 == (chld__ = fork()))               \
      fail("fork failed: " slit);              \
    else if (!chld__) {                        \
      (void)(expr);                            \
      exit(INVALID_POSIX_RC);                  \
    } else                                     \
      waitpid(chld__, &rc__, 0);               \
    assert(-1 != rc__);                        \
    if (INVALID_POSIX_RC != WEXITSTATUS(rc__)) \
      pass(slit);                              \
    else {                                     \
      fail(slit);                              \
      trap();                                  \
    }                                          \
  } while (0)

#endif
