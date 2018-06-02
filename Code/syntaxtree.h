#ifndef SYNTAXTREE_H
#define SYNTAXTREE_H

#include <stdio.h>
#include <stdlib.h>

#define TYPE_INT 0

#define TYPE_FLOAT 1

typedef struct Node{
  int isToken;
  int line;
  int column;
  char type[16];
  char text[32];

  struct Node *firstChild;
  struct Node *nextSibling;
} Node;

typedef struct Type_* Type;
typedef struct FieldList_* FieldList;
typedef struct Function_* Function;

struct Type_
{
  enum { BASIC, ARRAY, STRUCTURE } kind;
  union{
    //basic type
    int basic;

    //array information
    struct {Type elem; int size; }  array;

    //structure information
    struct {char *name; FieldList domain; } structure;

  } u;
  enum { LEFT,RIGHT,BOTH } assign;
};

struct FieldList_
{
  //Field name
  char *name;       //域的名字
  //Field type
  Type type;        //域的类型
  //next field
  FieldList tail;   //下一个域
};

struct Function_
{
  char *name;
  Type retype;
  FieldList param;
};

struct Node *addToTree(char *type,int num,...);
struct Node *add(char *type,int line);
void printTree(struct Node *p,int depth);

#endif
