#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "semantic.h"
#include "syntaxtree.h"
#include <string.h>

#define HASH_SIZE 2048

typedef struct Entry_
{
  char *name;
  enum { VARIABLE, STRUCT, FUNCTION } ekind;
  union
  {
    Type type;
    Function function;
  }e;
  struct Entry_ *next;
} Entry;

extern Entry *hashTable[HASH_SIZE];

void initTable();
Entry* getTable(char *name);

void varInsertTable(FieldList value);

void funcInsertTable(Function func);

void structInsertTable(char *name, FieldList domain);

#endif
