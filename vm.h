#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"

/*
 * @ip is the Instruction Pointer or a Program counter. It will always point to
 * the next Instruction or byte to be executed in a given chunk. We begin from
 * the first byte of the chunk
 */
typedef struct {
  Chunk *chunk;
  uint8_t *ip;
  Value stack[STACK_MAX];
  Value *stackTop;

} VM;

typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} InterpretResult;

void initVM();
void freeVM();
/*
 * This is to produce a constant, done by loading it on the stack
 */
void push(Value value);
/*
 * This is used to load a constant, by poping from the stack
 */
Value pop();
/*
 * Interpretes a chunk of code and is the main entry point into the into the VM
 */
InterpretResult interpret(Chunk *chunk);

#endif // !DEBUG
