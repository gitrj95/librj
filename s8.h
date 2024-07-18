#pragma once

#define S8( s ) ( ( struct s8 ){ s, sizeof s - 1 } )

struct s8 {
  char const *s;
  long        len;
};
