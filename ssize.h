/*
  I have recently been convinced that sizes and subscripts should be
  signed. Bjarne's justification:
  `https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1428r0.pdf`.

  Succinctly, choosing an unsigned type because it's non-negative is
  not valid. Unsigned types in C/C++ have modular arithmetic, so
  wraparound is defined and desirable behavior for those types. E.g.,
  there's no way for a callee to disambiguate between a wrapped
  negative and a genuine value within its non-negative domain besides
  excluding in-band values. Yet, wraparound is almost certainly not
  desirable behavior for sizes and subscripts. Misuses with integer
  literals are trivial to spot, but under layers of abstraction,
  arithmetic, etc., they're not the easiest to see.

  Alternatively, one can use signed types for sizes/subscripts, check
  for overflow candidates by checking if they're less than 0, and
  discard the extra interval of values the sign bit would have
  given. Unlike unsigned overflow, signed overflow is UB, so static
  and dynamic analyzers are very good at picking it up.

  Under this convention, valid uses of unsigned include either those
  cases when properties of modular arithmetic are desirable (e.g. when
  doing bitwise ops) or those cases when bitsize and domain are fixed
  (e.g. some protocol that exhausts the full set of values).
*/

#ifndef SSIZE_H
#define SSIZE_H

#include <assert.h>
#include <stddef.h>

typedef long long ssize;
static_assert(sizeof(ssize) == sizeof(size_t));

#endif
