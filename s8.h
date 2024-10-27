#ifndef LIBRJ_S8_H
#define LIBRJ_S8_H

#define S8( s ) { s, sizeof s - 1 }

typedef struct {
  char const *s;
  long        len;
} s8_t;

#endif
