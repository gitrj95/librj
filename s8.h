#ifndef LIBRJ_S8_H
#define LIBRJ_S8_H

#define S8( s ) { s, sizeof s - 1 }

struct s8 {
  char const *s;
  long        len;
};

#endif
