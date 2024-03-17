#include "table.h"
#include "memory.h"
#include "object.h"
#include "value.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void initTable(Table *table) {
  table->count = 0;
  table->capacity = 0;
  table->entries = NULL;
}

void freeTable(Table *table) {
  FREE_ARRAY(Entry, table->entries, table->capacity);
  initTable(table);
}

/*
 * Finds and returns an entry from the the table. It also does the linear
 * probing in case the entry was not stored in its prefered bucket due to
 * collision
 *
 * If it return NULL, key does not exists in the table (or the bucket is
 * tombstone) and this can be used for insertion
 *
 * Exit the the function when we find an empty bucket or the right required
 * bucket
 */
static Entry *findEntry(Entry *entries, int capacity, ObjString *key) {
  uint32_t index = key->hash % capacity;
  Entry *tombstone = NULL;
  for (;;) {
    Entry *entry = &entries[index];

    /* While finding the matching key, we store the first ever tombstone we
     * encounter. Then when we find and empty bucket furhter, we return  the
     * tombstone instead. */
    if (entry->key == NULL) {
      if (IS_NIL(entry->value)) {
        // Empty entry.
        return tombstone != NULL ? tombstone : entry;
      } else {
        // We found a tombstone and thus set the variable
        if (tombstone == NULL)
          tombstone = entry;
      }
    } else if (entry->key == key) {
      // We found the key.
      return entry;
    }
    index = (index + 1) % capacity;
  }
}

/* Creates and allocate and array of buckets and initializes all of them to NULL
 * When we need to grow the map, we instead recreate it and recalculate all
 * array. Why? result of collision and from Probing
 *
 * We donot copy tombstones over to the new table
 */
static void adjustCapacity(Table *table, int capacity) {

  Entry *entries = ALLOCATE(Entry, capacity);
  for (int i = 0; i < capacity; i++) {
    entries[i].key = NULL;
    entries[i].value = NIL_VAL;
  }

  table->count = 0;
  for (int i = 0; i < table->capacity; i++) {
    Entry *entry = &table->entries[i];
    if (entry->key == NULL)
      continue;

    Entry *dest = findEntry(entries, capacity, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
    table->count++;
  }

  // We free the old entries
  FREE_ARRAY(Entry, table->entries, table->capacity);
  // Set the new entries to the table
  table->entries = entries;
  table->capacity = capacity;
}

bool tableSet(Table *table, ObjString *key, Value value) {
  // Grow the table it is more than 75 percent full or empty
  if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
    int capacity = GROW_CAPACITY(table->capacity);

    adjustCapacity(table, capacity);
  }

  //
  Entry *entry = findEntry(table->entries, table->capacity, key);

  bool isNewKey = entry->key == NULL;
  // We increement count only if the bucket was completely empty and was not a
  // tombstone
  if (isNewKey && IS_NIL(entry->value))
    table->count++;

  table->count++;

  entry->key = key;
  entry->value = value;
  return isNewKey;
}

void tableAddAll(Table *from, Table *to) {
  for (int i = 0; i < from->capacity; i++) {
    Entry *entry = &from->entries[i];

    if (entry->key != NULL) {
      tableSet(to, entry->key, entry->value);
    }
  }
}

ObjString *tableFindString(Table *table, const char *chars, int length,
                           uint32_t hash) {

  if (table->count == 0)
    return NULL;

  uint32_t index = hash % table->capacity;

  for (;;) {
    Entry *entry = &table->entries[index];

    if (entry->key == NULL) {
      // Check if this not a tombstone
      if (IS_NIL(entry->value))
        return NULL;
    } else if (entry->key->length == length && entry->key->hash == hash &&
               memcmp(entry->key->chars, chars, length) == 0) {
      // We found the string
      return entry->key;
    }
    index = (index + 1) % table->capacity;
  }

  printf("Called \n");
}

bool tableGet(Table *table, ObjString *key, Value *value) {
  if (table->count == 0)
    return false;

  Entry *entry = findEntry(table->entries, table->capacity, key);
  if (entry->key == NULL)
    return false;

  *value = entry->value;
  return true;
}

bool tableDelete(Table *table, ObjString *key) {
  if (table->count == 0)
    return false;
  Entry *entry = findEntry(table->entries, table->capacity, key);
  if (entry->key == NULL)
    return false;

  // A tombstone value: {key= NULL, value = true}
  entry->key = NULL;
  entry->value = BOOL_VAL(true);

  return true;
}
