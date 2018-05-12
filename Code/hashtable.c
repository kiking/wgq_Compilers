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
}

void varInsertTable(FieldList value)
{
  unsigned int hashValue = 0;
  hashValue = hashPJW(value->name)%HASH_SIZE;

  Entry *curEntry = (Entry *)malloc(sizeof(Entry));
  curEntry->next = hashTable[hashValue];
  hashTable[hashValue] = curEntry;

  curEntry->type = value->type;
  curEntry->name = value->name;

}

void funcInsertTable(Function func)
{
  unsigned int hashValue = 0;
  hashValue = hashPJW(func->name)%HASH_SIZE;

  Entry *curEntry = (Entry *)malloc(sizeof(Entry));
  curEntry->next = hashTable[hashValue];
  hashTable[hashValue] = curEntry;

  curEntry->type = (Type)malloc(sizeof(struct Function_));
  curEntry->type->kind = FUNCTION;
  curEntry->type->u.function = func;
  curEntry->name = func->name;

  FieldList param = func->param;
  int flag;

 	while(param != NULL)
  {
    varInsertTable(param);
 		param = param->tail;
 	}
}

void structInsertTable(Structure structure)
{
  unsigned int hashValue = 0;
  hashValue = hashPJW(structure->name)%HASH_SIZE;

  Entry *curEntry = (Entry *)malloc(sizeof(Entry));
  curEntry->next = hashTable[hashValue];
  hashTable[hashValue] = curEntry;

  curEntry->type = (Type)malloc(sizeof(struct Structure_));
  curEntry->type->kind = STRUCTURE;
  curEntry->type->u.structure = structure;
  curEntry->name = structure->name;

}

bool varExit(FieldList var)
{
  if (hashTable==NULL || var==NULL)
  {
    return false;
  }

  unsigned int hashValue = hashPJW(var->name)%HASH_SIZE;
  Entry *tmp = hashTable[hashValue];
  for (;tmp!=NULL;tmp=tmp->next)
  {
    if (tmp->type->kind!=FUNCTION && strcmp(tmp->name,var->name)==0)
    {
      return true;
    }
  }
  return false;
}

bool funcExist(Function func)
{
  if (hashTable==NULL||func==NULL)
  {
    return false;
  }

  unsigned int hashValue = hashPJW(func->name)%HASH_SIZE;
  Entry *tmp=hashTable[hashValue];
  for (;tmp!=NULL;tmp=tmp->next)
  {
    if (tmp->type->kind != FUNCTION)
    {
      continue;
    }
    Function tmpFunc = tmp->type->u.function;
    if (strcmp(tmpFunc->name,func->name)!=0)
    {
      continue;
    }

    return true;
  }
  return false;
}

bool structExit(Structure structure)
{
  if (hashTable==NULL || structure==NULL)
  {
    return false;
  }

  unsigned int hashValue = hashPJW(structure->name)%HASH_SIZE;
  Entry *tmp = hashTable[hashValue];
  for (;tmp!=NULL;tmp=tmp->next)
  {
    if(tmp->type->kind!= FUNCTION && strcmp(tmp->name,structure->name)==0)
    {
      return true;
    }
  }
  return false;
}

Type getTable(char *name)
{
  if (hashTable==NULL||name==NULL)
  {
    return NULL;
  }

  unsigned int hashValue = hashPJW(name)%HASH_SIZE;

  Entry *tmp = hashTable[hashValue];
  for(;tmp!=NULL;tmp=tmp->next)
  {
    if (strcmp(tmp->name,name)==0)
    {
      return tmp->type;
    }
  }
  return NULL;
}
