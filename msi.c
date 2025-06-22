#include "msi.h"

long msi(int lg, unsigned long hash, long index)
{
  long mask = (1ul << lg) - 1;
  long stride = (hash >> (64 - lg)) | 1;
  return (index + stride) & mask;
}
