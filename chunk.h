#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"
#include <stddef.h>
#include <stdint.h>

/*
 * We define all operation for our bytecode format
 */
typedef enum {
  OP_RETURN, // Means return from current function,
  /*
   * Aimed at producing a constants
   * It has an operand. An the operand is the index of the constant to produce
   * It is a 2byte instruction of the format
   *  <opcode> <constant_index> e.g 0010 23, produces the constant stored at
index 23 from the list of constants in the array
   */
  OP_CONSTANT
} OpCode;

/*
 * Bytecode is a series of instructions or bytes and a group of them is stored
 * in a chunk This is just a Dynamic arraylist structure to store all
 * instructions or bytes.
 *
 * @count is the actual number of elements allocated
 * @capacity is the number of elements which can be allocated
 * @code is a list of bytes or instructions
 * @consants Are the values produced at runtime like numbers, strings etc, they
 * are stored in a dynamic array
 */
typedef struct {
  int count;
  int capacity;
  uint8_t *code;
  ValueArray consants;
  int *lines;

} Chunk;
void initChunk(Chunk *chunk); /*
                               * Adds a byte to the chunk
                               */
void writeChunk(Chunk *chunk, uint8_t byte, int line);
/*
 * Appends the constant to the list of constants
 */
int addConstant(Chunk *chunk, Value value);
/*
 * Deletes all bytes from the chunk
 */
void freeChunk(Chunk *chunk);

#endif // !clox_chunk_h
