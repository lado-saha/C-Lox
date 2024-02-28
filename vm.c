#include "vm.h"
#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "value.h"
#include <stdint.h>
#include <stdio.h>

VM vm;

static void resetStack() { vm.stackTop = vm.stack; }

void initVM() { resetStack(); }

void freeVM() {}

void push(Value value) {
  /* We set the value pointed by the address stackTop as value. Which is just
   * equivalent as storing in the stack at the index immidiately after the last
   * used one */
  *vm.stackTop = value;
  vm.stackTop++;
}

Value pop() {
  vm.stackTop--;
  return *vm.stackTop;
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->consants.values[READ_BYTE()])
  for (;;) {
    /* The pointer arithmetic is to find the index of the current instruction,
     * by subtracting the ip the address of first byte in code from the ip
     */
#ifndef DEBUG_TRACE_EXECUTION
    printf("         ");
    for (Value *slot = vm.stack; slot < vm.stackTop; slot++) {
      printf("[ ");
      printValue(*slot);
      printf(" ]");
    }
    printf("\n");
    disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));

#endif /* ifdef DEBUG_TRACE_EXECUTION */

    uint8_t instruction;

    switch (instruction = READ_BYTE()) {
    case OP_RETURN: {
      // Temporary exit point
      printValue(pop());
      printf("\n");
      return INTERPRET_OK;
    }
    case OP_CONSTANT: {
      Value constant = READ_CONSTANT();
      push(constant);
      break;
    }
    }
  }

#undef READ_BYTE
#undef READ_CONSTANT
}

InterpretResult interpret(Chunk *chunk) {
  vm.chunk = chunk;
  // We begin with the first byte of the chunk
  vm.ip = vm.chunk->code;
  return run();
}
