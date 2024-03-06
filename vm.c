#include "vm.h"
#include "chunk.h"
#include "common.h"
#include "compiler.h"
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

  /*The do { ... } while (false)  is used so that all the code will be pasted in
   * braces and in the same context, when the macro will be ran *
   * Notice that the 1st pop goes into `b` and not `a`
   * Because the right operand is pushed last and so is on top the stack*/
#define BINARY_OP(op)                                                          \
  do {                                                                         \
    double b = pop();                                                          \
    /* printf("b= %lf", b); */                                                 \
    double a = pop();                                                          \
    /*printf(" | a= %lf", a);  */                                              \
    push(a op b);                                                              \
  } while (false)

  for (;;) {

    /* The pointer arithmetic is to find the index of the current instruction,
     * by subtracting the ip the address of first byte in code from the ip
     */
#ifdef DEBUG_TRACE_EXECUTION
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
    case OP_CONSTANT: {
      Value constant = READ_CONSTANT();
      push(constant);
      break;
    }
    case OP_NEGATE: {
      // Just notice that we just negate the loaded constant
      push(-pop());
      break;
    }
    case OP_ADD: {
      BINARY_OP(+);
      break;
    }
    case OP_SUBTRACT: {
      BINARY_OP(-);
      break;
    }
    case OP_MULTIPLY: {
      BINARY_OP(*);
      break;
    }
    case OP_DIVIDE: {
      BINARY_OP(/);
      break;
    }

    case OP_RETURN: {
      // Temporary exit point
      printValue(pop());
      printf("\n");
      return INTERPRET_OK;
    }
    }
#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
  }
}

/*
 * This is the entry point of the whole compilation process
 * Takes source code compile it into bytecode chunk and
 * then execute it in the vm*/
InterpretResult interpret(const char *source) {
  Chunk chunk;
  initChunk(&chunk);

  /* Try to compile the source code into bytecode chunk. If any error, stop and
   * return compilation error */
  if (!compile(source, &chunk)) {
    freeChunk(&chunk);
    return INTERPRET_COMPILE_ERROR;
  }

  vm.chunk = &chunk;
  vm.ip = vm.chunk->code;

  InterpretResult result = run();

  freeChunk(&chunk);
  return result;
}
std::out_of_range
