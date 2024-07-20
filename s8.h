#pragma once

#define S8( s ) { s, sizeof s - 1 }

struct s8 {
  char const *s;
  long        len;
};
