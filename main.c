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

  int constant = addConstant(&chunk, 1.2);
  writeChunk(&chunk, OP_CONSTANT, 123);
  writeChunk(&chunk, constant, 123);
  constant = addConstant(&chunk, 3.4);
  writeChunk(&chunk, OP_CONSTANT, 123);
  writeChunk(&chunk, constant, 123);
  writeChunk(&chunk, OP_ADD, 123);
  constant = addConstant(&chunk, 5.6);
  writeChunk(&chunk, OP_CONSTANT, 123);
  writeChunk(&chunk, constant, 123);
  writeChunk(&chunk, OP_DIVIDE, 123);
  writeChunk(&chunk, OP_NEGATE, 123);
  writeChunk(&chunk, OP_RETURN, 123);
  printf("\n");
  disassembleChunk(&chunk, "test_chunk");
  interpret(&chunk);
  freeVM();
  freeChunk(&chunk);
  return EXIT_SUCCESS;
}
