#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

static Obj *allocateObject(size_t size, ObjType type) {
  Obj *object = (Obj *)reallocate(NULL, 0, size);
  object->type = type;

  // We keep reference to all allocatd objects by keeping a linked list
  object->next = vm.objects;
  vm.objects = object;

  return object;
}

#define ALLOCATE_OBJ(type, objectType)                                         \
  (type *)allocateObject(sizeof(type), objectType)

static ObjString *allocateString(char *chars, int length, uint32_t hash) {

  ObjString *string = ALLOCATE_OBJ(ObjString, OBJ_STRING);

  string->length = length;
  string->chars = chars;
  string->hash = hash;

  // Note that we intern all strings so that similar strings can point to the
  // same memory location and there by fasten comparisons

  tableSet(&vm.strings, string, NIL_VAL);

  return string;
}

/*
 * Hashing the string at allocation
 * FNV-1a algorithm
 * So this is hashong hein???
 */
static uint32_t hashString(const char *key, int length) {
  uint32_t hash = 216613626u;

  for (int i = 0; i < length; i++) {
    hash ^= key[i];
    hash *= 16777619;
  }

  return hash;
}

ObjString *copyString(const char *chars, int length) {
  uint32_t hash = hashString(chars, length);

  ObjString *interned = tableFindString(&vm.strings, chars, length, hash);

  // If this is a new string, we intern
  if (interned != NULL)
    return interned;

  // Instead of copying a string, we return the reference to the interned string
  char *heapChars = ALLOCATE(char, length + 1);
  memcpy(heapChars, chars, length);
  heapChars[length] = '\0';

  return allocateString(heapChars, length, hash);
}

void printObject(Value value) {
  switch (OBJ_TYPE(value)) {
  case OBJ_STRING:
    printf("%s", AS_CSTRING(value));
    break;
  }
}

ObjString *takeString(char *chars, int length) {
  uint32_t hash = hashString(chars, length);
  ObjString *interned = tableFindString(&vm.strings, chars, length, hash);

  // If this is a new string, we intern
  if (interned != NULL) {
    FREE_ARRAY(char, chars, length + 1);
    return interned;
  }

  return allocateString(chars, length, hash);
}
