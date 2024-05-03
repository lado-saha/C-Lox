#ifndef clox_common_h
#define clox_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Comment this line to avoid dumping the whole content of the chunk bytecode
#define DEBUG_PRINT_CODE

/* When this flag is defined, the VM disassembles and prints each instruction
right before executing it. Uncomment this to toggle off debug mode
comment it to avoid this behaviour*/
#define DEBUG_TRACE_EXECUTION

#define UINT8_COUNT (UINT8_MAX + 1)

#endif // !clox_common_h
