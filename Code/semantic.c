#include "semantic.h"
#include "hashtable.h"
#include "syntaxtree.h"
#include <string.h>

/* High-level Definitions */
void Program(Node* root)
{
  if (root==NULL) return ;

  initTable();
  ExtDefList(root->firstChild);
}

void ExtDefList(Node* n)
{
  if (n==NULL) return ;

  //ExtDefList->NULL
  if (n->firstChild==NULL) return ;

  //ExtDefList->ExtDef ExtDefList
  ExtDef(n->firstChild);
  ExtDefList(n->firstChild->nextSibling);
}

void ExtDef(Node *n)
{
  if (n==NULL) return ;

  Node* firstChild=n->firstChild;
  Type type = Specifier(firstChild);

  firstChild=firstChild->nextSibling;

  //ExtDef->Specifier ExtDecList SEMI
  if (strcmp(firstChild->type,"ExtDecList")==0)
  {
    ExtDecList(firstChild,type);
  }

  //ExtDef->Specifier SEMI
  else if (strcmp(firstChild->type,"SEMI")==0) return ;

  //ExtDef->Specifier FunDec CompSt
  else if (strcmp(firstChild->type,"FunDec")==0)
  {
    FunDec(firstChild,type);
    CompSt(firstChild->nextSibling,type);
  }
}

void ExtDecList(Node *n,Type type)
{
  if (n==NULL) return;

  Node *firstChild=n->firstChild;

  VarDec(firstChild,type,FROM_VARIABLE);

  firstChild=firstChild->nextSibling;

  //ExtDecList->VarDec
  //ExtDecList->VarDec COMMA ExtDecList
  if (firstChild!=NULL)
  {
    ExtDecList(firstChild->nextSibling,type);
  }
}

/* Specifiers */
Type Specifier(Node *n)
{
  if (n==NULL) return NULL;

  Node *firstChild=n->firstChild;

  Type type;

  //Specifier->TYPE
  if (strcmp(firstChild->type,"TYPE")==0)
  {
    type=(Type)malloc(sizeof(struct Type_));
    type->kind = BASIC;
    if (strcmp(firstChild->text,"int")==0)
    {
      type->u.basic=TYPE_INT;
    }
    else if (strcmp(firstChild->text,"float")==0)
    {
      type->u.basic=TYPE_FLOAT;
    }
  }
  //Specifier->StructSpecifier
  else if (strcmp(firstChild->type,"StructSpecifier")==0)
  {
    type=StructSpecifier(firstChild);
  }

  return type;
}

Type StructSpecifier(Node *n)
{
  if (n==NULL) return NULL;

  Node *firstChild=n->firstChild->nextSibling;

  Type type;

  //StructSpecifier->STRUCT OptTag LC DefList RC
  if (strcmp(firstChild->type,"OptTag")==0)
  {
    type = OptTag(firstChild);

    firstChild=firstChild->nextSibling->nextSibling;

    type->u.structure->domain = DefList(firstChild,FROM_FIELD);

    if (type->u.structure->name!=NULL)
    {
      if (structExit(type->u.structure)==true)
      {
        ErrorHandle(16, n->firstChild->line, type->u.structure->name);
        return NULL;
      }
      else
      {
        structInsertTable(type->u.structure);
      }
    }
  }
  //StructSpecifier->STRUCT Tag
  else if (strcmp(firstChild->type,"Tag")==0)
  {
    type = Tag(firstChild);
  }
  return type;
}

Type OptTag(Node *n)
{
  if (n==NULL) return NULL;

  Type type = (Type)malloc(sizeof(struct Type_));
  type->kind = STRUCTURE;
  Structure structure = (Structure)malloc(sizeof(struct Structure_));
  type->u.structure = structure;

  Node *firstChild=n->firstChild;

  //OptTag->ID
  if (strcmp(firstChild->type,"ID")==0)
  {
    structure->name = firstChild->text;
  }
  //OptTag->NULL
  else
  {
    structure->name = NULL;
  }
  return type;
}

Type Tag(Node *n)
{
  if (n==NULL) return NULL;
  Node *firstChild=n->firstChild;

  //Tag->ID
  Type type = getTable(firstChild->text);
  if (type==NULL || type->kind!=STRUCTURE )
  {
    ErrorHandle(17, firstChild->line, firstChild->text);
  }
  return type;
}

/* Declarators */
FieldList VarDec(Node *n,Type type,int from)
{
  if (n==NULL) return NULL;

  Node *firstChild=n->firstChild;

  //VarDec->ID
   if(strcmp(firstChild->type,"ID")==0)
   {
     FieldList varDec = (FieldList)malloc(sizeof(struct FieldList_));
     varDec->name = firstChild->text;
     varDec->type = type;
 		 varDec->tail = NULL;
     if (from==FROM_PARAM)
     {
       return varDec;
     }
     if (varExit(varDec)==true)
     {
       if (from==FROM_VARIABLE)
       {
         ErrorHandle(3, firstChild->line, varDec->name);
       }
       else if (from==FROM_FIELD)
       {
         ErrorHandle(15, firstChild->line, varDec->name);
       }
       return NULL;
     }
     else
     {
       varInsertTable(varDec);
     }
     return varDec;
   }
   //VarDec->VarDec LB INT RB
   else if (strcmp(firstChild->type,"VarDec")==0)
   {
     Type varDec = (Type)malloc(sizeof(struct Type_));
     varDec->kind = ARRAY;
  	 varDec->u.array.size = (int)strtol(firstChild->nextSibling->nextSibling->text,NULL,10);
 		 varDec->u.array.elem = type;

 		 return VarDec(firstChild, varDec, from);
   }
}

void FunDec(Node *n,Type type)
{
  if (n==NULL) return ;

  Node *firstChild = n->firstChild;
  Function func = (Function)malloc(sizeof(struct Function_));
  func->name = firstChild->text;
  func->param = NULL;
  func->retype = type;

  firstChild = firstChild->nextSibling->nextSibling;
  // ID LP VarList RP
  if (strcmp(firstChild->type,"VarList")==0)
  {
    func->param=VarList(firstChild);
  }

  if (funcExist(func)==true)
  {
    ErrorHandle(4,n->firstChild->line,n->firstChild->text);
  }
  else funcInsertTable(func);
}

FieldList VarList(Node *n)
{
  if (n==NULL) return NULL;

  Node *firstChild=n->firstChild;

  //VarList->ParamDec
  FieldList varList = ParamDec(firstChild);
  firstChild = firstChild->nextSibling;

  //VarList->ParamDec COMMA VarList
  if (firstChild!=NULL)
  {
    varList->tail = VarList(firstChild->nextSibling);
  }

  return varList;
}

FieldList ParamDec(Node *n)
{
  if (n==NULL) return NULL;

  Node *firstChild = n->firstChild;

  // Specifier VarDec
  Type type = Specifier(firstChild);
  firstChild = firstChild->nextSibling;
  FieldList ParamDec = VarDec(firstChild,type,FROM_PARAM);
  return ParamDec;
}

/* Statements */
void CompSt(Node *n,Type retype)
{
  if (n==NULL) return ;

  Node *firstChild = n->firstChild;
  //CompSt->LC DefList StmtList RC

  firstChild = firstChild->nextSibling;

  DefList(firstChild,FROM_VARIABLE);

  firstChild = firstChild->nextSibling;

  StmtList(firstChild,retype);
}

void StmtList(Node *n,Type retype)
{
  if (n==NULL) return ;

  Node *firstChild = n->firstChild;

  //DefList->NULL
  if (firstChild==NULL) return ;

  //StmtList->Stmt StmtList
  Stmt(firstChild,retype);
  firstChild = firstChild->nextSibling;
  StmtList(firstChild,retype);
}

void Stmt(Node *n,Type retype)
{
  if (n==NULL) return ;

  Node *firstChild = n->firstChild;

  //Stmt->Exp SEMI
  if (strcmp(firstChild->type,"Exp")==0)
  {
    Exp(firstChild);
  }
  //Stmt->CompSt
  else if (strcmp(firstChild->type,"CompSt")==0)
  {
    CompSt(firstChild,retype);
  }
  //Stmt->RETURN Exp SEMI
  else if (strcmp(firstChild->type,"RETURN")==0)
  {
    Type expType = Exp(firstChild->nextSibling);

    if (typeEqual(retype,expType)==false)
    {
      ErrorHandle(8, firstChild->line, NULL);
    }
  }
  //Stmt->IF LP Exp RP Stmt (ELSE Stmt)
  else if (strcmp(firstChild->type,"IF")==0)
  {
    firstChild=firstChild->nextSibling->nextSibling;

    Exp(firstChild);
    firstChild=firstChild->nextSibling->nextSibling;
    Stmt(firstChild,retype);
    firstChild=firstChild->nextSibling;
    if (firstChild!=NULL)
    {
      Stmt(firstChild->nextSibling,retype);
    }
  }
  //Stmt->WHILE LP Exp RP Stmt
  else if (strcmp(firstChild->type,"WHILE")==0)
  {
    firstChild = firstChild->nextSibling->nextSibling;
    Exp(firstChild);
    firstChild = firstChild->nextSibling->nextSibling;
    Stmt(firstChild,retype);
  }
}

/* Local Definitions */
FieldList DefList(Node *n,int from)
{
  if (n==NULL) return NULL;

  Node *firstChild = n->firstChild;
  FieldList defList = NULL;

  //DefList->NULL
  if (firstChild==NULL)
  {
    return defList;
  }

  //DefList->Def DefList
  defList=Def(firstChild,from);

  if(defList == NULL)
  {
 		defList = DefList(firstChild->nextSibling, from);
 	}
  else {
    FieldList tmp = defList;
    while (tmp->tail != NULL)
    {
      tmp = tmp->tail;
    }
    tmp->tail = DefList(firstChild->nextSibling,from);
  }

  return defList;
}

FieldList Def(Node *n,int from)
{
  if (n==NULL) return NULL;

  Node *firstChild = n->firstChild;

  //Def->Specifier DecList SEMI
  Type type = Specifier(firstChild);
  firstChild = firstChild->nextSibling;
  FieldList def = DecList(firstChild,type,from);
  return def;
}

FieldList DecList(Node *n,Type type,int from)
{
  if (n==NULL) return NULL;

  Node *firstChild = n->firstChild;
  FieldList decList = Dec(firstChild,type,from);
  firstChild = firstChild->nextSibling;

  //DecList->Dec
  if (firstChild==NULL)
  {
    return decList;
  }
  //DecList->Dec COMMA DecList
  firstChild = firstChild->nextSibling;
  if(decList==NULL)
  {
 		decList = DecList(firstChild, type, from);
 	}
 	else{
 		FieldList tmp = decList;
 		while(tmp->tail != NULL)
    {
      tmp = tmp->tail;
    }
 		tmp->tail = DecList(firstChild, type, from);
 	}
  return decList;
}

FieldList Dec(Node *n,Type type,int from)
{
  if (n==NULL) return NULL;

  Node *firstChild = n->firstChild;
  FieldList dec = VarDec(firstChild,type,from);

  firstChild = firstChild->nextSibling;
  //Dec->VarDec
  if (firstChild == NULL)
  {
    return dec;
  }
  //Dec->VarDec ASSIGNOP Exp
  if (from==FROM_FIELD)
  {
    ErrorHandle(15, firstChild->line, dec->name);
    return NULL;
  }
  firstChild = firstChild->nextSibling;
  Type expType = Exp(firstChild);
  if (typeEqual(type,expType)==false)
  {
    ErrorHandle(5, firstChild->line, NULL);
    return NULL;
  }
  return dec;
}

/* Expressions */
Type Exp(Node *n)
{
  if (n==NULL) return NULL;

  Node *firstChild = n->firstChild;

  if (strcmp(firstChild->type,"Exp")==0)
  {
    //Exp->Exp ...
    if (strcmp(firstChild->nextSibling->type,"ASSIGNOP")==0)
    {
      //Exp->Exp ASSIGNOP Exp
      //判断表达式左边是否为左值
      Type lhs = Exp(firstChild);
      Type rhs = Exp(firstChild->nextSibling->nextSibling);
      if(lhs==NULL || rhs==NULL)
      {
        return NULL;
      }
      if(lhs->assign==RIGHT)
      {
        ErrorHandle(6, firstChild->line, NULL);
        return NULL;
      }
      //判断赋值号两边表达式是否类型匹配
      if(typeEqual(lhs, rhs)==true)
      {
        return lhs;
      }
      else
      {
        ErrorHandle(5, firstChild->line, NULL);
        return NULL;
      }
    }
    else if (strcmp(firstChild->nextSibling->type,"AND")==0 ||
              strcmp(firstChild->nextSibling->type,"OR")==0 ||
              strcmp(firstChild->nextSibling->type,"RELOP")==0 ||
              strcmp(firstChild->nextSibling->type,"PLUS")==0 ||
              strcmp(firstChild->nextSibling->type,"MINUS")==0 ||
              strcmp(firstChild->nextSibling->type, "STAR")==0 ||
              strcmp(firstChild->nextSibling->type,"DIV")==0)
    {
      //Exp->Exp AND|OR|RELOP|PLUS|MINUS|STAR|DIV Exp
      Type lhs = Exp(firstChild);
      Type rhs = Exp(firstChild->nextSibling->nextSibling);

      if(lhs==NULL||rhs==NULL)
      {
        return NULL;
      }
      if(lhs->kind==BASIC && rhs->kind==BASIC && lhs->u.basic==rhs->u.basic)
      {
        Type rtn = (Type)malloc(sizeof(struct Type_));
        memcpy(rtn, lhs, sizeof(struct Type_));
        rtn->assign = RIGHT;
        return rtn;
      }
      else
      {
        ErrorHandle(7, firstChild->line, NULL);
        return NULL;
      }
    }
    else if (strcmp(firstChild->nextSibling->type,"LB")==0)
    {
      //Exp->Exp LB Exp RB
      Type array = Exp(firstChild);

      if (array==NULL) return NULL;
      if (array->kind!=ARRAY)
      {
        ErrorHandle(10, firstChild->line, firstChild->firstChild->text);
        return NULL;
      }
      firstChild = firstChild->nextSibling->nextSibling;
      Type arrayNumber = Exp(firstChild);
      if (arrayNumber==NULL) return NULL;
      if (arrayNumber->kind!=BASIC || arrayNumber->u.basic!=TYPE_INT)
      {
        ErrorHandle(12, firstChild->line, firstChild->firstChild->text);
        return NULL;
      }
      //return array->u.array.elem;
      Type rtn = (Type)malloc(sizeof(struct Type_));
      memcpy(rtn, array->u.array.elem, sizeof(struct Type_));
      rtn->assign = BOTH;
      return rtn;
    }
    else if (strcmp(firstChild->nextSibling->type,"DOT")==0)
    {
      //Exp->Exp DOT ID
      Type structure = Exp(firstChild);
      if (structure==NULL) return NULL;
      if (structure->kind!=STRUCTURE)
      {
        ErrorHandle(13, firstChild->line, NULL);
        return NULL;
      }

      FieldList structDomain = structure->u.structure->domain;
      firstChild = firstChild->nextSibling->nextSibling;
      while(structDomain!=NULL)
      {
        if(strcmp(structDomain->name, firstChild->text) ==0)
        {
          Type rtn = (Type)malloc(sizeof(struct Type_));
          memcpy(rtn, structDomain->type, sizeof(struct Type_));
          rtn->assign = BOTH;
          return rtn;
         }
        structDomain = structDomain->tail;  //可能有错，找下一个
      }
      ErrorHandle(14, firstChild->line, firstChild->text);
      return NULL;
    }
    return NULL;
  }
  else if (strcmp(firstChild->type,"LP")==0)
  {
    //Exp->LP Exp RP
    firstChild = firstChild->nextSibling;
    return Exp(firstChild);
  }
  else if (strcmp(firstChild->type,"MINUS")==0)
  {
    //Exp->MINUS Exp
    firstChild = firstChild->nextSibling;
    Type type = Exp(firstChild);
    if (type==NULL) return NULL;
    if (type->kind!=BASIC)
    {
      ErrorHandle(7, firstChild->line, NULL);
      return NULL;
    }
    Type rtn = (Type)malloc(sizeof(struct Type_));
    memcpy(rtn, type, sizeof(struct Type_));
    rtn->assign = RIGHT;
    return rtn;
  }
  else if (strcmp(firstChild->type,"NOT")==0)
  {
    //Exp->NOT Exp
    firstChild = firstChild->nextSibling;
    Type type = Exp(firstChild);
    if (type==NULL) return NULL;
    if (type->kind!=BASIC || type->u.basic!=TYPE_INT)
    {
      ErrorHandle(7, firstChild->line, NULL);
      return NULL;
    }
    Type rtn = (Type)malloc(sizeof(struct Type_));
    memcpy(rtn, type, sizeof(struct Type_));
    rtn->assign = RIGHT;
    return rtn;
  }
  else if (strcmp(firstChild->type,"ID")==0)
  {
    if (firstChild->nextSibling==NULL)
    {
      //Exp -> ID
      Type value = getTable(firstChild->text);  //判断是否定义过
      if (value == NULL || value->kind == FUNCTION)
      {
        ErrorHandle(1, firstChild->line, firstChild->text);
        return NULL;
      }
      Type rtn = (Type)malloc(sizeof(struct Type_));
      memcpy(rtn, value, sizeof(struct Type_));
      rtn->assign = BOTH;
      return rtn;
    }
    else
    {
      //Exp->ID LP RP | ID LP Args RP
      Type func = getTable(firstChild->text);  //判断是否定义过
      if (func==NULL)
      {
        ErrorHandle(2, firstChild->line, firstChild->text);
        return NULL;
      }
      if (func->kind!=FUNCTION)
      {
        ErrorHandle(2, firstChild->line, firstChild->text);
        return NULL;
      }
      FieldList param = func->u.function->param;
      firstChild = firstChild->nextSibling->nextSibling;
      if (strcmp(firstChild->type,"RP")==0)
      {
        if (param != NULL)
        {
          ErrorHandle(9, firstChild->line, func->u.function->name);
          return NULL;
        }
      }
      else
      {
        if (Args(firstChild,param)==false)   //比较两个类型是否匹配
        {
          ErrorHandle(9, firstChild->line, func->u.function->name);
          return NULL;
        }
      }
      //return func->retype;
      Type rtn = (Type)malloc(sizeof(struct Type_));
 			memcpy(rtn, func->u.function->retype, sizeof(struct Type_));
 			rtn->assign = RIGHT;
 			return rtn;
    }
  }
  else if (strcmp(firstChild->type,"INT")==0)
  {
    //Exp -> INT
    Type type = (Type)malloc(sizeof(struct Type_));
    type->kind = BASIC;
    type->u.basic = TYPE_INT;
    type->assign = RIGHT;
    return type;
  }
  else if (strcmp(firstChild->type,"FLOAT")==0)
  {
    //Exp -> FLOAT
    Type type = (Type)malloc(sizeof(struct Type_));
    type->kind = BASIC;
    type->u.basic = TYPE_FLOAT;
    type->assign = RIGHT;
    return type;
  }
  return NULL;
}

bool Args(Node *n, FieldList param)
{
  if (n==NULL||param==NULL) return false;

  Node *firstChild = n->firstChild;

 	Type tmpParam = Exp(firstChild);
 	if(typeEqual(param->type, tmpParam)==true)
  {
    //Args->Exp
 		if(firstChild->nextSibling==NULL)
 				return true;
 		else
 				return Args(firstChild->nextSibling->nextSibling, param->tail);
 	}
 	else
 		return false;
 }

void ErrorHandle(int type, int line, char *info)
{
    switch (type) {
    case 1:
        printf("Error type 1 at Line %d: Undefined variable \"%s\".\n",line,info);
        break;
    case 2:
        printf("Error type 2 at Line %d: Undefined function \"%s\".\n",line,info);
        break;
    case 3:
        printf("Error type 3 at Line %d: Redefined variable \"%s\".\n",line,info);
        break;
    case 4:
        printf("Error type 4 at Line %d: Redefined function \"%s\".\n",line,info);
        break;
    case 5:
        printf("Error type 5 at Line %d: Type mismatched for assignment.\n",line);
        break;
    case 6:
        printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n",line);
        break;
    case 7:
        printf("Error type 7 at Line %d: Type mismatched for operands.\n",line);
        break;
    case 8:
        printf("Error type 8 at Line %d: Type mismatched for return.\n",line);
        break;
    case 9:
        printf("Error type 9 at Line %d: Function \"%s\" is not applicable for arguments.\n",line,info);
        break;
    case 10:
        printf("Error type 10 at Line %d: \"%s\" is not an array.\n",line,info);
        break;
    case 11:
        printf("Error type 11 at Line %d: \"%s\" is not a function.\n",line,info);
        break;
    case 12:
        printf("Error type 12 at Line %d: \"%s\" is not an integer\n",line,info);
        break;
    case 13:
        printf("Error type 13 at Line %d: Illegal use of \".\".\n",line);
        break;
    case 14:
        printf("Error type 14 at Line %d: Non-existent field \"%s\".\n",line,info);
        break;
    case 15:
        printf("Error type 15 at Line %d: Redefined or initial field \"%s\".\n",line,info);
        break;
    case 16:
        printf("Error type 16 at Line %d: Duplicated name \"%s\".\n",line,info);
        break;
    case 17:
        printf("Error type 17 at Line %d: Undefined structure \"%s\".\n",line,info);
        break;
    default:
        exit(1);
    }
}


bool typeEqual(Type lhs,Type rhs)
{
  if (lhs->kind != rhs->kind)
  {
    return false;
  }

  if (lhs->kind==BASIC)
  {
    if (lhs->u.basic!=rhs->u.basic) return false;
  }
  else if (lhs->kind==ARRAY)
  {
    return typeEqual(lhs->u.array.elem,rhs->u.array.elem);
  }
  else if (lhs->kind==STRUCTURE)
  {
    return structEqual(lhs->u.structure, rhs->u.structure);
  }
  else exit(-1);

  return true;
}

bool structEqual(Structure lhs, Structure rhs){
  FieldList lDomain = lhs->domain;
  FieldList rDomain = rhs->domain;
  while(lDomain!=NULL && rDomain!=NULL)
  {
 		if(typeEqual(lDomain->type, rDomain->type)==false)
        return false;
 		lDomain = lDomain->tail;
 		rDomain = rDomain->tail;
 	}
 	if(lDomain==NULL && rDomain==NULL)
 			return true;
 	else
 			return false;
 }
