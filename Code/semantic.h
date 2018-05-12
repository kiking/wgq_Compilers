#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "syntaxtree.h"
#include "stdbool.h"

#define FROM_VARIABLE 3
#define FROM_FIELD 2
#define FROM_PARAM 1

typedef struct Type_* Type;
typedef struct Structure_* Structure;
typedef struct FieldList_* FieldList;
typedef struct Function_* Function;

struct Type_
{
  enum { BASIC, ARRAY, STRUCTURE, FUNCTION } kind;
  union{
    //basic type
    int basic;

    //array information
    struct {Type elem; int size; }  array;

    //structure information
    Structure structure;

    //function information
    Function function;
  } u;
  enum { LEFT,RIGHT,BOTH } assign;
};

struct Structure_
{
  char *name;
  FieldList domain;
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

bool typeEqual(Type lhs,Type rhs);
bool structEqual(Structure lhs, Structure rhs);

void Program(Node* root);
void ExtDefList(Node* n);
void ExtDef(Node *n);
void ExtDecList(Node *n, Type type);

Type Specifier(Node *n);
Type StructSpecifier(Node *n);
Type OptTag(Node *n);
Type Tag(Node *n);

FieldList VarDec(Node *n,Type type,int from);
void FunDec(Node *n,Type type);
FieldList VarList(Node *n);
FieldList ParamDec(Node *n);

void CompSt(Node *n,Type retype);
void StmtList(Node *n, Type retype);
void Stmt(Node *n, Type retype);

FieldList DefList(Node *n, int from);
FieldList Def(Node *n, int from);
FieldList DecList(Node *n, Type type,int from);
FieldList Dec(Node *n, Type type,int from);

Type Exp(Node *n);
bool Args(Node *n, FieldList param);

void ErrorHandle(int type, int line, char *info);

#endif
