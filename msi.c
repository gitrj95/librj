#include "msi.h"

int msi(int lg, unsigned long hash, int index)
{
  unsigned mask = (1u << lg) - 1;
  unsigned stride = (hash >> (64 - lg)) | 1;
  return (index + stride) & mask;
}
