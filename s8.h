#pragma once

#define S8( s ) { s, sizeof s - 1 }

typedef struct {
  char const *s;
  long        len;
} s8;
