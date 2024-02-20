#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include <stddef.h>
#include <stdint.h>

/*
 * We define all operation for our bytecode format
 */
typedef enum {
  OP_RETURN // Means return from current function,
} OpCode;

/*
 * Bytecode is a series of instructions or bytes and a group of them is stored
 * in a chunk This is just a Dynamic arraylist structure to store all
 * instructions or bytes.
 *
 * @count is the actual number of elements allocated
 * @capacity is the number of elements which can be allocated
 * @code is a list of bytes or instructions
 */
typedef struct {
  int count;
  int capacity;
  uint8_t *code;
} Chunk;

void initChunk(Chunk *chunk);
/*
 * Adds a byte to the chunk
 */
void writeChunk(Chunk *chunk, uint8_t byte);
/*
 * Deletes all bytes from the chunk
 */
void freeChunk(Chunk *chunk);

#endif // !clox_chunk_h
