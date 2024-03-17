#ifndef clox_table_h
#define clox_table_h
#include "common.h"
#include "object.h"
#include "value.h"

/*
 * This is the percentage of fullness after which we need to grow the table
 */
#define TABLE_MAX_LOAD 0.75

/** An Key value pair to be stored in the hashtable */
typedef struct {
  ObjString *key;
  Value value;
} Entry;

/* A hastable is a wrapper around a dynamic array of entry
 * @count is the number of entries plus number tombstones.*/
typedef struct {
  int count;
  int capacity;
  Entry *entries;
} Table;

void initTable(Table *table);
void freeTable(Table *table);
/* Inserting a new element into the table */
bool tableSet(Table *table, ObjString *key, Value value);
/* Using the key gets from the table and stores it inside the value. Returns
 * true in case of succes*/
bool tableGet(Table *table, ObjString *key, Value *value);
/* We donot actually delete or emoty the bucket since it could completely break
 * probing. Instead, we place a tombstone entry and nullify the key*/
bool tableDelete(Table *table, ObjString *key);

/*
 * Copy all the entries from one table to another table
 */
void tableAddAll(Table *from, Table *to);

/*
 * This is to create a way to efficiently find a string from the table without
 * comparing character after character */
ObjString *tableFindString(Table *table, const char *chars, int length,
                           uint32_t hash);

#endif
