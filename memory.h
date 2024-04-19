#ifndef clox_memory_h
#define clox_memory_h

#include "common.h"

/* To allocate a particular type of object */
#define ALLOCATE(type, count)                                                  \
  (type *)reallocate(NULL, 0, sizeof(type) * (count))

/* simple free the memory blob pointer */
#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)

/*
 * The capacity is always a multiple of 8.
 * If the capacity is less than 8 (i.e 0), we set it to 8 else we
 */
#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity) * 2)

// Reallocates inorder to increase the size of the array to the new size
#define GROW_ARRAY(type, pointer, oldCount, newCount)                          \
  (type *)reallocate(pointer, sizeof(type) * oldCount, sizeof(type) * newCount)

#define FREE_ARRAY(type, pointer, oldCount)                                    \
  reallocate(pointer, sizeof(type), 0);

/*
 * Main dynamic memory manager for clox
 *
 * 1) Allocate new block: oldSize = 0 & newSize != 0
 * 2) Free block: oldSize != 0 & newSize = 0
 * 3) Shrink existing block: oldSize != 0 & newSize < oldSize
 * 4) Grow existing block: oldSize != 0 & newSize > oldSize
 *
 */

void *reallocate(void *pointer, size_t oldSize, size_t newSize);
/*
 * This ensures that all allocated objects are not dereferenced without freeing
 */
void freeObjects();

#endif // !clox_memory_h
