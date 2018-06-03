#include "hashtable.h"

Entry *hashTable[HASH_SIZE];

unsigned int hashPJW(char *name)
{
  unsigned int val=0,i;
  for (;*name;++name)
  {
    val=(val<<2)+*name;
    if (i=val & ~0x3fff) val = (val^(i>>12))&0x3fff;
  }
  return val;
}

void initTable()
{
  int i=0;
  for(;i<HASH_SIZE;i++)
  {
    hashTable[i]=NULL;
  }
  Function read=malloc(sizeof(struct Function_));
  read->name=malloc(20);
	strcpy(read->name,"read");
  read->retype=malloc(sizeof(struct Type_));
	read->retype->kind=BASIC;
	read->retype->u.basic=TYPE_INT;
	read->param=NULL;
	funcInsertTable(read);
  
	Function write=malloc(sizeof(struct Function_));
  write->name=malloc(20);
	strcpy(write->name,"write");
  write->retype=malloc(sizeof(struct Type_));
	write->retype->kind=BASIC;
	write->retype->u.basic=TYPE_INT;
  write->param=malloc(sizeof(struct FieldList_));
	write->param->name = malloc(20);
	strcpy(write->param->name,"write_param");
  write->param->type=malloc(sizeof(struct Type_));
	write->param->type->kind = BASIC;
	write->param->type->u.basic = TYPE_INT;
	write->param->tail=NULL;
	funcInsertTable(write);
}

void varInsertTable(FieldList value)
{
  unsigned int hashValue = 0;
  hashValue = hashPJW(value->name)%HASH_SIZE;

  Entry *curEntry = (Entry *)malloc(sizeof(Entry));
  curEntry->next = hashTable[hashValue];
  hashTable[hashValue] = curEntry;

  curEntry->name = value->name;
  curEntry->ekind = VARIABLE;
  curEntry->e.type = value->type;
}

void funcInsertTable(Function func)
{
  unsigned int hashValue = 0;
  hashValue = hashPJW(func->name)%HASH_SIZE;

  Entry *curEntry = (Entry *)malloc(sizeof(Entry));
  curEntry->next = hashTable[hashValue];
  hashTable[hashValue] = curEntry;

  curEntry->name = func->name;
  curEntry->ekind = FUNCTION;
  curEntry->e.function = func;

}

void structInsertTable(char *name, FieldList domain)
{
  unsigned int hashValue = 0;
  hashValue = hashPJW(name)%HASH_SIZE;

  Entry *curEntry = (Entry *)malloc(sizeof(Entry));
  curEntry->next = hashTable[hashValue];
  hashTable[hashValue] = curEntry;

  curEntry->name = name;
  curEntry->ekind = STRUCT;
  curEntry->e.type = (Type)malloc(sizeof(struct Type_));
  curEntry->e.type->kind = STRUCTURE;
  curEntry->e.type->u.structure.name = name;
  curEntry->e.type->u.structure.domain = domain;

}

Entry* getTable(char *name)
{
  if (hashTable==NULL||name==NULL)
  {
    return NULL;
  }

  unsigned int hashValue = hashPJW(name)%HASH_SIZE;

  Entry *tmp = hashTable[hashValue];
  while (tmp!=NULL && strcmp(tmp->name,name)!=0)
  {
    tmp = tmp->next;
  }

  return tmp;
}
