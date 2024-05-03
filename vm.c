#include "vm.h"
#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

VM vm;

static void resetStack() { vm.stackTop = vm.stack; }

/*
 * Notice the ... to take variable arguments. Then the `va_list args` to collect
 * them, the vfprintf a version of printf that takes many arguments. found in
 * the stdarg.h
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

void initVM() {
  resetStack();
  vm.objects = NULL;
  initTable(&vm.globals);
  initTable(&vm.strings);
}

void freeVM() {
  freeObjects();
  freeTable(&vm.globals);
  freeTable(&vm.strings);
}

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

/** Takes 2 lox strings, convert them to cstrings, concat them and push them to
 * the stack */
static void concatenate() {
  ObjString *b = AS_STRING(pop());
  ObjString *a = AS_STRING(pop());

  int length = a->length + b->length;
  char *chars = ALLOCATE(char, length + 1);
  memcpy(chars, a->chars, a->length);
  memcpy(chars + a->length, b->chars, b->length);
  chars[length] = '\0';

  ObjString *result = takeString(chars, length);
  push(OBJ_VAL(result));
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->consants.values[READ_BYTE()])
#define READ_STRING() AS_STRING(READ_CONSTANT())
#define READ_SHORT() (vm.ip += 2, (uint16_t)((vm.ip[-2] << 8) | vm.ip[-1]));
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
      // OP_LOOP is from the vm perspective similar to the OP_JUMP
    case OP_LOOP: {
      uint16_t offset = READ_SHORT();
      vm.ip -= offset;
      break;
    }
    case OP_ADD: {
      if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
        concatenate();
      } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
        double b = AS_NUMBER(pop());
        double a = AS_NUMBER(pop());
        push(NUMBER_VAL(a + b));
      } else {
        runtimeError("Operand must be two numbers or two string");
        return INTERPRET_RUNTIME_ERROR;
      }
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
      BINARY_OP(BOOL_VAL, <);
      break;

    // Comparison
    case OP_EQUAL: {
      Value a = pop();
      Value b = pop();
      push(BOOL_VAL(valuesEqual(a, b)));
      break;
    }
    case OP_PRINT: {
      printValue(pop());
      printf("\n");
      break;
    }
    case OP_POP:
      pop();
      break;

      // Adds global var to the table
    case OP_DEFINE_GLOBAL: {
      ObjString *name = READ_STRING();
      tableSet(&vm.globals, name, peek(0));
      pop();
      break;
    }

    case OP_GET_LOCAL: {
      uint8_t slot = READ_BYTE();
      push(vm.stack[slot]);
      break;
    }

    case OP_SET_LOCAL: {
      uint8_t slot = READ_BYTE();
      vm.stack[slot] = peek(0);
      break;
    }
      // Tries to resolve and return the variable name
    case OP_GET_GLOBAL: {
      ObjString *name = READ_STRING();
      Value value;
      if (!tableGet(&vm.globals, name, &value)) {
        runtimeError("Undefined variable '%s'.", name->chars);
        return INTERPRET_RUNTIME_ERROR;
      }
      push(value);
      break;
    }

      /* In this case, we are trying to change the value of an alrady declared
      variable */
    case OP_SET_GLOBAL: {
      ObjString *name = READ_STRING();
      if (tableSet(&vm.globals, name, peek(0))) {
        tableDelete(&vm.globals, name);
        runtimeError("Undefined variable '%s'.", name->chars);
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    }

      // To jump, we change the value of the istruction pointer(ip)
    case OP_JUMP_IF_FALSE: {
      uint16_t offset = READ_SHORT();
      if (isFalsey(peek(0)))
        vm.ip += offset;
      break;
    }

    // This is the core of control flow
    case OP_JUMP: {
      uint16_t offset = READ_SHORT();
      vm.ip += offset;
      break;
    }

    case OP_RETURN: {
      // Temporary exit point
      return INTERPRET_OK;
    }
    }
#undef READ_SHORT
#undef READ_BYTE
#undef READ_STRING
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

  /* Try to compile the source code into bytecode chunk. If any error, stop
   * and return compilation error */
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
