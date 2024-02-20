#include "chunk.h"
#include "common.h"
#include <stdlib.h>

int main(int argc, char *argv[]) {
  Chunk chunk;
  initChunk(&chunk);
  writeChunk(&chunk, OP_RETURN);
  freeChunk(&chunk);
  return EXIT_SUCCESS;
}
