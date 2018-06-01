#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "syntaxtree.h"
#include "stdbool.h"
#include "intercode.h"

#define FROM_VARIABLE 1
#define FROM_FIELD 2

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


bool typeEqual(Type lhs,Type rhs);
bool structEqual(FieldList lhs, FieldList rhs);

void Program(Node* root);
void ExtDefList(Node* n);
void ExtDef(Node *n);
void ExtDecList(Node *n, Type type);

Type Specifier(Node *n);
Type StructSpecifier(Node *n);
char *OptTag(Node *n);
char *Tag(Node *n);

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

Type Exp(Node *n, Operand *place);
Type Exp_Cond(Node *n, Operand *label_true, Operand *label_false);
bool Args(Node *n, FieldList param, Operand *arg_list);

int typeSize(Type type);

void ErrorHandle(int type, int line, char *info);

#endif
