#ifndef clox_compiler_h
#define clox_compiler_h

#include "chunk.h"
/*
 * This takes in the source code, scan it and then transform it into a chunk of
 * bytecode. Returns false in case there is any error
 */
bool compile(const char *source, Chunk *chunk);

#endif // !clox_compiler_h
