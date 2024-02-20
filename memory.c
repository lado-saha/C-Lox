#include "memory.h"
#include <stdlib.h>

void *reallocate(void *pointer, size_t oldSize, size_t newSize) {
  if (newSize == 0) { // By our convention, we free when the newSize is 0
    free(pointer);
    return NULL;
  }

  void *result = realloc(pointer, newSize);
  return result;
}
