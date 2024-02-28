#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  initVM();
  Chunk chunk;
  initChunk(&chunk);

  int constant = addConstant(&chunk, 1.0003);
  int const1 = addConstant(&chunk, 3.5);
  int line = 123;
  // For a constant, we write byte by byte. The first byte is the opcode
  // followed by the index of the constant
  writeChunk(&chunk, OP_CONSTANT, line);
  writeChunk(&chunk, constant, line);

  writeChunk(&chunk, OP_CONSTANT, line + 2);
  writeChunk(&chunk, const1, line + 2);

  writeChunk(&chunk, OP_RETURN, line);
  printf("\n");
  // disassembleChunk(&chunk, "test_chunk");
  interpret(&chunk);
  freeVM();
  freeChunk(&chunk);
  return EXIT_SUCCESS;
}
