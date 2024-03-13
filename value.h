#ifndef clox_value_h
#define clox_value_h

#include "common.h"

typedef struct Obj Obj;
typedef struct ObjString ObjString;

/*
 * Each Value has a type represented here
 */
typedef enum { VAL_BOOL, VAL_NIL, VAL_NUMBER, VAL_OBJ } ValueType;

/*
 * We only support double values now
 * A type can be a number(8byte double ) or boolean(1byte bool) or an Object
 * type (String, Instance, functions), but since Lox is dynamically typed, the
 * type can change at any time and so, our Value need to accomodate to that. We
 * use union because a variable can only be of one type at the same time. NB:
 * The size of our union is the size of the largest member i.e 8bytes
 */
typedef struct {
  ValueType type;
  union {
    bool boolean;
    double number;
    Obj *obj; // Heap address to the memory blob storing the object
  } as;
} Value;

/*
 * These macros verify if a Lox value is of a particular Lox type
 */
#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_OBJ(value) ((value).type == VAL_OBJ)
/*
 * The following macros are to convert a Lox Value into a usable C value
 * e.g Number value -> double
 */
#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_OBJ(value) ((value).as.obj)

/* The following macros are to produce appropriate Lox Value and types from a C
 * value E.g double -> Number,
 */
#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define NIL_VAL ((Value){VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})
#define OBJ_VAL(object) ((Value){VAL_OBJ, {.obj = (Obj *)object}})

/**
 * This is a dynamic array to store all the constant values produced from our
 * code
 */
typedef struct {
  int capacity;
  int count;
  Value *values;
} ValueArray;

/**
 * Compares values and tell us if they are equals. We define how to compare
 * equality for each value type
 */
bool valuesEqual(Value a, Value b);
void initValueArray(ValueArray *array);
void writeValueArray(ValueArray *array, Value value);
void freeValueArray(ValueArray *array);
void printValue(Value value);

#endif // !clox_value_h
