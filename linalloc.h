#ifndef LIBRJ_LINALLOC_H
#define LIBRJ_LINALLOC_H

#define LINALLOC(n) [[gnu::malloc]] void *linalloc##n(struct arena *a, long sz);

struct arena {
  void *hd, *tl;
};

LINALLOC(1)
LINALLOC(2)
LINALLOC(4)
LINALLOC(8)
LINALLOC(16)
LINALLOC(32)
LINALLOC(64)
LINALLOC(128)
LINALLOC(4096)

#endif
