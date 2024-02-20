#include "chunk.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  Chunk chunk;
  initChunk(&chunk);
  writeChunk(&chunk, OP_RETURN);

  printf("\n");
  disassembleChunk(&chunk, "test_chunk");
  freeChunk(&chunk);
  return EXIT_SUCCESS;
}
