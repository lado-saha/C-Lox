#include "vm.h"
#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "value.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

VM vm;

static void resetStack() { vm.stackTop = vm.stack; }

/*
 * Notice the ... to take variable arguments. Then the `va_list args` to collect
 * them, the vfprintf a version of printf that takes many arguments. found in
 * the stdarg.h
 *
 *
 *
 */
static void runtimeError(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  size_t instruction = vm.ip - vm.chunk->code - 1;
  int line = vm.chunk->lines[instruction];
  fprintf(stderr, "[line %d] in script\n", line);

  resetStack();
}

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

/* returns the value from the stack but doesnot pop it . @distance is how far
 * down in the stack we need to look*/
static Value peek(int distance) { return vm.stackTop[-1 - distance]; }

/* Assigns a truth value to any value: nil and false are False and the rest true
 */
static bool isFalsey(Value value) {
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->consants.values[READ_BYTE()])

  /*The do { ... } while (false)  is used so that all the code will be pasted in
   * braces and in the same context, when the macro will be ran *
   * Notice that the 1st pop goes into `b` and not `a`
   * Because the right operand is pushed last and so is on top the stack*/
#define BINARY_OP(valueType, op)                                               \
  do {                                                                         \
    if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {                          \
      runtimeError("Operands must be numbers.");                               \
      return INTERPRET_RUNTIME_ERROR;                                          \
    }                                                                          \
                                                                               \
    double b = AS_NUMBER(pop());                                               \
    /* printf("b= %lf", b); */                                                 \
    double a = AS_NUMBER(pop());                                               \
    /*printf(" | a= %lf", a);  */                                              \
    push(valueType(a op b));                                                   \
  } while (false)

  for (;;) {

    /* The pointer arithmetic is to find the index of the current instruction,
     * by subtracting the ip the address of first byte in code from the ip
     */
#ifdef DEBUG_TRACE_EXECUTION
    printf("        ");
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
    case OP_NIL:
      push(NIL_VAL);
      break;
    case OP_TRUE:
      push(BOOL_VAL(true));
      break;
    case OP_FALSE:
      push(BOOL_VAL(false));
      break;
    case OP_NEGATE: {
      // Just notice that we just negate the loaded constant
      if (!IS_NUMBER(peek(0))) {
        runtimeError("Operand must be a number.");
        return INTERPRET_RUNTIME_ERROR;
      }

      push(NUMBER_VAL(-AS_NUMBER(pop())));
      break;
    }
    case OP_NOT: {
      push(BOOL_VAL(isFalsey(pop())));
      break;
    }
    case OP_ADD: {
      BINARY_OP(NUMBER_VAL, +);
      break;
    }
    case OP_SUBTRACT: {
      BINARY_OP(NUMBER_VAL, -);
      break;
    }
    case OP_MULTIPLY: {
      BINARY_OP(NUMBER_VAL, *);
      break;
    }
    case OP_DIVIDE: {
      BINARY_OP(NUMBER_VAL, /);
      break;
    }

    case OP_GREATER:
      BINARY_OP(BOOL_VAL, >);
      break;
    case OP_LESS:
      BINARY_OP(BOOL_VAL, >);
      break;
    // Comparison
    case OP_EQUAL: {
      Value a = pop();
      Value b = pop();
      push(BOOL_VAL(valuesEqual(a, b)));
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
