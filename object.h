#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "value.h"

/* returns the actual type of an Obj derived type */
#define OBJ_TYPE(value) (AS_OBJ(value)->type)
#define IS_STRING(value) (isObjType(value, OBJ_STRING))

/* Convert to string */
#define AS_STRING(value) ((ObjString *)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString *)AS_OBJ(value))->chars)

typedef enum {
  OBJ_STRING,
} ObjType;
/*
 * A base class for storing String, Instance and Function types
 * All heap allocated types
 *
 * @next is the pointer to the next ever allocated object in the linked list of
 * allocated objects, this is done to avoid memory leaks
 */
struct Obj {
  ObjType type;

  struct Obj *next;
};

/*
 * A C-kinda-sub-class of Obj
 * Since the obj field is perfectly aligned at the beginning of this struct, we
 * can cast from an ObjString object to a Obj by just accessing its starting
 * address which will point to the first element, namely: Obj
 *
 * This can also go the other way round
 *
 * @obj To signify this is a sort of child to the Obj class
 * @length is used to know the size of the string so that we donot need a null
 * terminator
 * @chars points to the actual array of characters
 */
struct ObjString {
  Obj obj;
  int length;
  char *chars;
};

/*
 * We take ownership of the character array chars */
ObjString *takeString(char *chars, int length);

/**
 * Creates a lox string from a C-string. Lox strings donot have terminators and
 * so need to store their length */
ObjString *copyString(const char *chars, int length);

/* Helper to print an objext*/
void printObject(Value value);

static inline bool isObjType(Value value, ObjType type) {
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif // !clox_object_h
