#ifndef clox_value_h
#define clox_value_h

#include "common.h"

/*
 * Each Value has a type represented here
 */
typedef enum {
  VAL_BOOL,
  VAL_NIL,
  VAL_NUMBER,
} ValueType;

/*
 * We only support double values now
 * A type can be a number(8byte double ) or boolean(1byte bool), but since Lox
 * is dynamically typed, the type can change at any time and so, our Value need
 * to accomodate to that. We use union because a variable can only be of one
 * type at the same time.
 * NB: The size of our union is the size of the largest member i.e 8bytes
 */
typedef struct {
  ValueType type;
  union {
    bool boolean;
    double number;
  } as;
} Value;

/*
 * These macros verify if a Lox value is of a particular Lox type
 */
#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)

/*
 * The following macros are to convert a Lox Value into a usable C value
 * e.g Number value -> double
 */
#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)

/* The following macros are to produce appropriate Lox Value and types from a C
 * value E.g double -> Number,
 */
#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define NIL_VAL ((Value){VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})

/**
 * This is a dynamic array to store all the constant values produced from our
 * code
 */
typedef struct {
  int capacity;
  int count;
  Value *values;
} ValueArray;

bool valuesEqual(Value a, Value b);
void initValueArray(ValueArray *array);
void writeValueArray(ValueArray *array, Value value);
void freeValueArray(ValueArray *array);
void printValue(Value value);

#endif // !clox_value_h
