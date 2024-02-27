#ifndef clox_value_h
#define clox_value_h

#include "common.h"

/*
 * We only support double values now
 */
typedef double Value;

/**
 * This is a dynamic array to store all the constant values produced from our
 * code
 */
typedef struct {
  int capacity;
  int count;
  Value *values;
} ValueArray;

void initValueArray(ValueArray *array);
void writeValueArray(ValueArray *array, Value value);
void freeValueArray(ValueArray *array);
void printValue(Value value);

#endif // !clox_value_h
