#include "debug.h"
#include "chunk.h"
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

static int simpleInstruction(const char *name, int offset) {
  printf("%s\n", name);
  return offset + 1;
}

int disassembleInstruction(Chunk *chunk, int offset) {
  printf("%04d ", offset);

  uint8_t instruction = chunk->code[offset];
  switch (instruction) {
  case OP_RETURN:
    return simpleInstruction("OP_RETURN", offset);
  default:
    printf("Unknown opcode %d\n", instruction);
    return offset + 1;
  }
}
