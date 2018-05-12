#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "semantic.h"
#include "syntaxtree.h"
#include <string.h>

#define HASH_SIZE 1024

typedef struct Entry_
{
  char *name;
  Type type;
  struct Entry_ *next;
} Entry;

extern Entry *hashTable[HASH_SIZE];

void initTable();
Type getTable(char *name);

void varInsertTable(FieldList value);
bool varExit(FieldList var);

void funcInsertTable(Function func);
bool funcExist(Function func);

void structInsertTable(Structure structure);
bool structExit(Structure structure);

#endif
