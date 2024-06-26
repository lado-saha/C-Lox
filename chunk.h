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
   *  <opcode> <constant_index> e.g 0009 23, produces the constant stored at
index 23 from the list of constants in the array
   */
  OP_CONSTANT,
  OP_NEGATE, // Unary negation
  OP_PRINT,
  OP_JUMP_IF_FALSE,
  OP_LOOP,
  OP_ADD,

  // To emit a result or value from the stack
  OP_POP,
  OP_GET_LOCAL,
  OP_SET_LOCAL,

  OP_NIL,
  OP_TRUE,
  OP_FALSE,

  OP_EQUAL,
  OP_GREATER,
  OP_LESS,

  OP_JUMP,

  OP_SUBTRACT,
  OP_DIVIDE,
  OP_NOT,
  OP_MULTIPLY,

  OP_DEFINE_GLOBAL,
  OP_GET_GLOBAL,
  OP_SET_GLOBAL,

} OpCode;

#define STACK_MAX 256
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
