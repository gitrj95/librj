#ifndef ARRAYSIZE_H
#define ARRAYSIZE_H

#include <stddef.h>

#define arraysize(a) ((ptrdiff_t)(sizeof(a) / sizeof((a)[0])))

#endif
