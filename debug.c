#include "debug.h"
#include "chunk.h"
#include "value.h"
#include <stdint.h>
#include <stdio.h>

void disassembleChunk(Chunk *chunk, const char *name) {
  printf("== %s ==\n", name);
  /*
   * Notice that we are the one to handle the incrementation in the for loop
   * But nevertheless, we stop when we have exhausted all the instructions
   */
  for (int offset = 0; offset < chunk->count;) {
    // We disassemble each instruction of the bytecode
    offset = disassembleInstruction(chunk, offset);
  }
}

static int constantInstruction(const char *name, Chunk *chunk, int offset) {
  uint8_t constant = chunk->code[offset + 1];
  // We print OP_CODE CONST_INDEX CONST_VALUE
  printf("%-16s %4d '", name, constant);
  printValue(chunk->consants.values[constant]);
  printf("'\n");
  return offset + 2;
}

static int simpleInstruction(const char *name, int offset) {
  printf("%s\n", name);
  return offset + 1;
}

int disassembleInstruction(Chunk *chunk, int offset) {
  printf("%04d ", offset);
  // print the line number
  if (offset && chunk->lines[offset] == chunk->lines[offset - 1]) {
    /* We show '|' for any instruction that appears on a linw number already
     * printed */
    printf("   | ");
  } else {
    printf("%4d ", chunk->lines[offset]);
  }

  uint8_t instruction = chunk->code[offset];
  switch (instruction) {
  case OP_RETURN:
    return simpleInstruction("OP_RETURN", offset);

  case OP_CONSTANT:
    return constantInstruction("OP_CONSTANT", chunk, offset);

  default:
    printf("Unknown opcode %d\n", instruction);
    return offset + 1;
  }
}
