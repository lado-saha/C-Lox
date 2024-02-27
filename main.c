#include "chunk.h"
#include "common.h"
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  Chunk chunk;
  initChunk(&chunk);

  int constant = addConstant(&chunk, 1.0003);
  int line = 123;
  // For a constant, we write byte by byte. The first byte is the opcode
  // followed by the index of the constant
  writeChunk(&chunk, OP_CONSTANT, line);
  writeChunk(&chunk, constant, line);

  writeChunk(&chunk, OP_RETURN, line);
  printf("\n");
  disassembleChunk(&chunk, "test_chunk");
  freeChunk(&chunk);
  return EXIT_SUCCESS;
}
