#ifndef clox_debug_h
#define clox_debug_h

#include "chunk.h"

/*
 * Converts the chunk bytecode to human readable representation of it
 */
void disassembleChunk(Chunk *chunk, const char *name);

/*
 * Note that the offset is the index of this instruction in the chunk
 */
int disassembleInstruction(Chunk *chunk, int offset);

#endif // !clox_debug_h
