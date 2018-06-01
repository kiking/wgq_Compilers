#include "semantic.h"
#include "hashtable.h"
#include "syntaxtree.h"
#include <string.h>
#define IR_DEBUG 1

/* High-level Definitions */
void Program(Node* root)
{
#ifdef IR_DEBUG
  printf("%s\n", root->type);
#endif

  initTable();
  ExtDefList(root->firstChild);
}

void ExtDefList(Node* n)
{
  if (n==NULL) return ;

#ifdef IR_DEBUG
  printf("%s\n", n->type);
#endif

  //ExtDefList->NULL
  if (n->firstChild==NULL) return ;

  //ExtDefList->ExtDef ExtDefList
  ExtDef(n->firstChild);
  ExtDefList(n->firstChild->nextSibling);
}

void ExtDef(Node *n)
{
  if (n==NULL) return ;

#ifdef IR_DEBUG
  printf("%s\n", n->type);
#endif

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

#ifdef IR_DEBUG
  printf("%s\n", n->type);
#endif

  Node *firstChild=n->firstChild;

  FieldList fl=VarDec(firstChild,type,FROM_VARIABLE);

  if (fl!=NULL)
  {
    if(fl->type->kind == ARRAY)			//array
		{
      Operand *op=newTempvar();
			InterCode *deccode=malloc(sizeof(InterCode));
			deccode->kind=IR_DEC;
			deccode->u.dec.op = op;
			deccode->u.dec.size=typeSize(fl->type);
      InterCodes *pdec=malloc(sizeof(InterCodes));
      pdec->code=deccode;
			insertCode(pdec);
      Operand *v=newVariable(fl->name);
			InterCode *addrcode = malloc(sizeof(InterCode));
			addrcode->kind = IR_ADDRESS;
			addrcode->u.assign.left = v;
			addrcode->u.assign.right = op;
      InterCodes *paddr=malloc(sizeof(InterCodes));
      paddr->code=addrcode;
			insertCode(paddr);
		}
  }

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

#ifdef IR_DEBUG
  printf("%s\n", n->type);
#endif

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

#ifdef IR_DEBUG
  printf("%s\n", n->type);
#endif

  Node *firstChild=n->firstChild->nextSibling;

  Type type=NULL;

  //StructSpecifier->STRUCT OptTag LC DefList RC
  if (strcmp(firstChild->type,"OptTag")==0)
  {
    Type type = (Type)malloc(sizeof(struct Type_));
    type->kind = STRUCTURE;

    type->u.structure.name = OptTag(firstChild);

    firstChild=firstChild->nextSibling->nextSibling;

    type->u.structure.domain = DefList(firstChild,FROM_FIELD);

    if (type->u.structure.name!=NULL)
    {
      Entry *tmp = getTable(type->u.structure.name);
      if (tmp!=NULL)
      {
        ErrorHandle(16, n->firstChild->line, type->u.structure.name);
        return NULL;
      }
      else
      {
        structInsertTable(type->u.structure.name,type->u.structure.domain);
      }
    }
  }
  //StructSpecifier->STRUCT Tag
  else if (strcmp(firstChild->type,"Tag")==0)
  {
    char *name = Tag(firstChild);
    Entry *tmp = getTable(name);
    if (tmp==NULL || tmp->ekind!=STRUCT)
    {
      ErrorHandle(17, n->firstChild->line, name);
      type=NULL;
    }
    else
    {
      type=tmp->e.type;
    }
  }
  return type;
}

char *OptTag(Node *n)
{
  if (n==NULL) return NULL;

  Node *firstChild=n->firstChild;

  //OptTag->ID
  if (strcmp(firstChild->type,"ID")==0)
  {
    return firstChild->text;
  }
  //OptTag->NULL
  else
  {
    return NULL;
  }
}

char *Tag(Node *n)
{
  if (n==NULL) return NULL;

  //Tag->ID
  return n->firstChild->text;
}

/* Declarators */
FieldList VarDec(Node *n,Type type,int from)
{
  if (n==NULL) return NULL;

#ifdef IR_DEBUG
  printf("%s\n", n->type);
#endif

  Node *firstChild=n->firstChild;

  //VarDec->ID
  if(strcmp(firstChild->type,"ID")==0)
  {
    FieldList varDec = (FieldList)malloc(sizeof(struct FieldList_));
    varDec->name = firstChild->text;
    varDec->type = type;
 	  varDec->tail = NULL;
    Entry *tmp = getTable(varDec->name);
    if (tmp!=NULL)
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
       if(type->kind==STRUCTURE && from==FROM_FIELD)
       {
         Operand *op=newTempvar();
				InterCode *deccode=malloc(sizeof(InterCode));
				deccode->kind = IR_DEC;
				deccode->u.dec.op = op;
				deccode->u.dec.size = typeSize(type);
        InterCodes *pdec=malloc(sizeof(InterCodes));
        pdec->code=deccode;
				insertCode(pdec);
        Operand *v=newVariable(firstChild->text);
				InterCode *addrcode=malloc(sizeof(InterCode));
				addrcode->kind = IR_ADDRESS;
				addrcode->u.assign.left = v;
				addrcode->u.assign.right = op;
        InterCodes *pa=malloc(sizeof(InterCodes));
        pa->code=addrcode;
				insertCode(pa);
			}
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

#ifdef IR_DEBUG
  printf("%s\n", n->type);
#endif

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

  Entry *tmp = getTable(func->name);
  if (tmp!=NULL && tmp->ekind==FUNCTION)
  {
    ErrorHandle(4,n->firstChild->line,n->firstChild->text);
  }
  else 
  {
    funcInsertTable(func);
    Operand *funcop=newFunction(func->name);
    InterCode *code=malloc(sizeof(InterCode));
		code->kind = IR_FUNCTION;
		code->u.sinop.op = funcop;
    InterCodes *p=malloc(sizeof(InterCodes));
    p->code=code;
		insertCode(p);		//funtion  :
    FieldList param=func->param;
    while(param!=NULL)
		{
      Operand *pop=newVariable(param->name);
			InterCode *pcode = malloc(sizeof(InterCode));
			pcode->kind = IR_PARAM;
			pcode->u.sinop.op=pop;
      InterCodes *pp=malloc(sizeof(InterCodes));
      pp->code=pcode;
			insertCode(pp);
			param=param->tail;
		}
  }

}

FieldList VarList(Node *n)
{
  if (n==NULL) return NULL;

#ifdef IR_DEBUG
  printf("%s\n", n->type);
#endif

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

#ifdef IR_DEBUG
  printf("%s\n", n->type);
#endif

  Node *firstChild = n->firstChild;

  // Specifier VarDec
  Type type = Specifier(firstChild);
  return VarDec(firstChild->nextSibling,type,FROM_VARIABLE);
}

/* Statements */
void CompSt(Node *n,Type retype)
{
  if (n==NULL) return ;

#ifdef IR_DEBUG
  printf("%s\n", n->type);
#endif

  //CompSt->LC DefList StmtList RC

  DefList(n->firstChild->nextSibling,FROM_VARIABLE);

  StmtList(n->firstChild->nextSibling->nextSibling,retype);
}

void StmtList(Node *n,Type retype)
{
  if (n==NULL) return ;

#ifdef IR_DEBUG
  printf("%s\n", n->type);
#endif

  //StmtList->NULL
  //StmtList->Stmt StmtList
  if (n->firstChild!=NULL)
  {
    Stmt(n->firstChild,retype);
    StmtList(n->firstChild->nextSibling,retype);
  }

}

void Stmt(Node *n,Type retype)
{
  if (n==NULL) return ;

#ifdef IR_DEBUG
  printf("%s\n", n->type);
#endif

  Node *firstChild = n->firstChild;

  //Stmt->Exp SEMI
  if (strcmp(firstChild->type,"Exp")==0)
  {
    Exp(firstChild,NULL);
  }
  //Stmt->CompSt
  else if (strcmp(firstChild->type,"CompSt")==0)
  {
    CompSt(firstChild,retype);
  }
  //Stmt->RETURN Exp SEMI
  else if (strcmp(firstChild->type,"RETURN")==0)
  {
    Operand *op=newTempvar();
    if (typeEqual(retype,Exp(firstChild->nextSibling,op))==false)
    {
      ErrorHandle(8, firstChild->line, NULL);
    }
    InterCode *code = malloc(sizeof(InterCode));
		code->kind = IR_RETURN;
		code->u.sinop.op = op;
    InterCodes *pc=malloc(sizeof(InterCodes));
    pc->code=code;
		insertCode(pc);
  }
  //Stmt->IF LP Exp RP Stmt (ELSE Stmt)
  //Stmt->WHILE LP Exp RP Stmt
  else if (strcmp(firstChild->type,"IF")==0)
  {
    firstChild=firstChild->nextSibling->nextSibling;
    Operand *lb1=newLabel();
    Operand *lb2 = newLabel();
		Type expType = Exp_Cond(firstChild,lb1,lb2);
		InterCode *code1=malloc(sizeof(InterCode));
		code1->kind=IR_LABEL;
		code1->u.sinop.op=lb1;
    InterCodes *p1=malloc(sizeof(InterCodes));
    p1->code=code1;
		insertCode(p1);
		firstChild = firstChild->nextSibling->nextSibling;
		Stmt(firstChild, retype);
		InterCode *lb2code = malloc(sizeof(InterCode));
		lb2code->kind = IR_LABEL;
		lb2code->u.sinop.op = lb2;
		firstChild = firstChild->nextSibling;
    if (firstChild==NULL)
    {
      InterCodes *p2=malloc(sizeof(InterCodes));
      p2->code=lb2code;
      insertCode(p2);
      return ;
    }
    Operand *lb3 = newLabel();
		InterCode *code2=malloc(sizeof(InterCode));
		code2->kind = IR_GOTO;
		code2->u.sinop.op = lb3;
    InterCodes *pcode2=malloc(sizeof(InterCodes));
    pcode2->code=code2;
		insertCode(pcode2);			//goto label3
    InterCodes *plb2code=malloc(sizeof(InterCodes));
    plb2code->code=lb2code;
		insertCode(plb2code);		//label2
    firstChild=firstChild->nextSibling;
		Stmt(firstChild, retype);
		InterCode *lb3code=malloc(sizeof(InterCode));
		lb3code->kind = IR_LABEL;
		lb3code->u.sinop.op = lb3;
    InterCodes *plb3code=malloc(sizeof(InterCodes));
    plb3code->code=lb3code;
		insertCode(plb3code);		//label3
    
  }
  else if (strcmp(firstChild->type,"WHILE")==0)
  {
    Operand *lb1=newLabel();
    Operand *lb2=newLabel();
    Operand *lb3=newLabel();
		firstChild=firstChild->nextSibling->nextSibling;
		InterCode *lb1code = malloc(sizeof(InterCode));
		lb1code->kind = IR_LABEL;
		lb1code->u.sinop.op = lb1;
    InterCodes *p1=malloc(sizeof(InterCodes));
    p1->code=lb1code;
		insertCode(p1);		//label 1
		Exp_Cond(firstChild,lb2,lb3);	//code1
		InterCode *lb2code=malloc(sizeof(InterCode));
		lb2code->kind = IR_LABEL;
		lb2code->u.sinop.op = lb2;
    InterCodes *p2=malloc(sizeof(InterCodes));
    p2->code=lb2code;
		insertCode(p2);		//label 2
		firstChild = firstChild->nextSibling->nextSibling;
		Stmt(firstChild, retype);
		InterCode *gotolb1 = malloc(sizeof(InterCode));
		gotolb1->kind = IR_GOTO;
		gotolb1->u.sinop.op = lb1;
    InterCodes *pg=malloc(sizeof(InterCodes));
    pg->code=gotolb1;
		insertCode(pg);		//goto label1
		InterCode *lb3code = malloc(sizeof(InterCode));
		lb3code->kind = IR_LABEL;
		lb3code->u.sinop.op = lb3;
    InterCodes *p3=malloc(sizeof(InterCodes));
    p3->code=lb3code;
		insertCode(p3);		//label3
  }
}

/* Local Definitions */
FieldList DefList(Node *n,int from)
{
  if (n==NULL) return NULL;

#ifdef IR_DEBUG
  printf("%s\n", n->type);
#endif

  Node *firstChild = n->firstChild;

  //DefList->NULL
  if (firstChild==NULL) return NULL;

  //DefList->Def DefList
  FieldList defList=Def(firstChild,from);

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

#ifdef IR_DEBUG
  printf("%s\n", n->type);
#endif

  Node *firstChild = n->firstChild;

  //Def->Specifier DecList SEMI
  Type type = Specifier(n->firstChild);
  return DecList(n->firstChild->nextSibling,type,from);
}

FieldList DecList(Node *n,Type type,int from)
{
  if (n==NULL) return NULL;

#ifdef IR_DEBUG
  printf("%s\n", n->type);
#endif

  //DecList->Dec
  Node *firstChild = n->firstChild;
  FieldList decList = Dec(firstChild,type,from);

  if (firstChild->nextSibling==NULL)
  {
    return decList;
  }
  //DecList->Dec COMMA DecList
  firstChild = firstChild->nextSibling->nextSibling;
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

#ifdef IR_DEBUG
  printf("%s\n", n->type);
#endif

  //Dec->VarDec
  Node *firstChild = n->firstChild;
  FieldList dec = VarDec(firstChild,type,from);

  if (dec->type->kind==ARRAY&&from==FROM_VARIABLE)
  {
    //array space
    Operand *op=newTempvar();

		InterCode *deccode=malloc(sizeof(InterCode));
		deccode->kind=IR_DEC;
		deccode->u.dec.op=op;
		deccode->u.dec.size=typeSize(dec->type);
    InterCodes *pd=malloc(sizeof(InterCodes));
    pd->code=deccode;
		insertCode(pd);

    Operand *v=newVariable(dec->name);

		InterCode *addrcode=malloc(sizeof(InterCode));
		addrcode->kind = IR_ADDRESS;
		addrcode->u.assign.left = v;
		addrcode->u.assign.right = op;
    InterCodes *pa=malloc(sizeof(InterCodes));
    pa->code=addrcode;
		insertCode(pa);
  }

  if (firstChild->nextSibling == NULL)
  {
    return dec;
  }
  //Dec->VarDec ASSIGNOP Exp
  if (from==FROM_FIELD)
  {
    ErrorHandle(15, firstChild->line, dec->name);//在定义时对域进行初始化
    return NULL;
  }
  Operand *place=newVariable(dec->name);

  firstChild = firstChild->nextSibling->nextSibling;
  if (typeEqual(type,Exp(firstChild,place))==false)
  {
    ErrorHandle(5, firstChild->line, NULL);
    return NULL;
  }
  if(place->kind!=VARIABLE||place->u.value!=dec->name)
	{
    Operand *left=newLabel(dec->name);
		InterCode *asscode=malloc(sizeof(InterCode));
		asscode->kind=IR_ASSIGN;
		asscode->u.assign.left=left;
		asscode->u.assign.right=place;
    InterCodes *pass=malloc(sizeof(InterCodes));
    pass->code=asscode;
		insertCode(pass);
	}
  return dec;
}

/* Expressions */
Type Exp(Node *n, Operand *place)
{
  if (n==NULL) return NULL;

#ifdef IR_DEBUG
  printf("%s\n", n->type);
#endif

  Node *firstChild = n->firstChild;
 
  if (strcmp(firstChild->type,"Exp")==0)
  {
    //Exp->Exp ...
    if (strcmp(firstChild->nextSibling->type,"ASSIGNOP")==0)
    {
      //Exp->Exp ASSIGNOP Exp

      Operand *leftOp=newTempvar();
      Operand *rightOp=newTempvar();
      int rightOpNO=rightOp->u.var_no;

      //判断表达式左边是否为左值
      Type lhs = Exp(firstChild,leftOp);
      Type rhs = Exp(firstChild->nextSibling->nextSibling,rightOp);
      if (lhs==NULL||rhs==NULL) return NULL;
      if(lhs->assign==RIGHT)
      {
        ErrorHandle(6, firstChild->line, NULL);
        return NULL;
      } 
      //判断赋值号两边表达式是否类型匹配
      if(typeEqual(lhs, rhs)==false)
      {
        ErrorHandle(5, firstChild->line, NULL);
        return NULL;
      }
      else
      {
        if( !(rightOp->kind==OP_TEMPVAR && rightOp->u.var_no==rightOpNO && (leftOp->kind==OP_TEMPVAR || leftOp->kind==OP_VARIABLE)) )
        {
          InterCodes *p = malloc(sizeof(InterCodes));
          InterCode *assignCode1 = malloc(sizeof(InterCode));
          assignCode1->kind = IR_ASSIGN;
					assignCode1->u.assign.left = leftOp;
					assignCode1->u.assign.right = rightOp;
          p->code=assignCode1;
					insertCode(p);
        }
        else{
					memcpy(rightOp, leftOp, sizeof(Operand));
				}
				if(place!=NULL){
          InterCodes *q = malloc(sizeof(InterCodes));
					InterCode *assignCode2 = malloc(sizeof(InterCode));
          assignCode2->kind = IR_ASSIGN;
					assignCode2->u.assign.left = place;
					assignCode2->u.assign.right = rightOp;
          q->code=assignCode2;
					insertCode(q);
				}
        return lhs;
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
      //Exp->Exp |PLUS|MINUS|STAR|DIV Exp

      Operand *leftOp=newTempvar();
      Operand *rightOp=newTempvar();

      Type lhs = Exp(firstChild,leftOp);
      Type rhs = Exp(firstChild->nextSibling->nextSibling,rightOp);

      if (lhs==NULL||rhs==NULL) return NULL;
      if(lhs->kind==BASIC && rhs->kind==BASIC && lhs->u.basic==rhs->u.basic)
      {
        if (place!=NULL)
        {
          InterCodes *p=malloc(sizeof(InterCodes));
          InterCode *calc=malloc(sizeof(InterCode));
          if (strcmp(firstChild->nextSibling->type,"PLUS")==0)
          {
            calc->kind=IR_ADD;
          }
          else if (strcmp(firstChild->nextSibling->type,"MINUS")==0)
          {
            calc->kind=IR_SUB;
          }
          else if (strcmp(firstChild->nextSibling->type,"STAR")==0)
          {
            calc->kind=IR_MUL;
          }
          else if (strcmp(firstChild->nextSibling->type,"DIV")==0)
          {
            calc->kind=IR_DIV;
          }
          else exit(-1);
          calc->u.binop.op1=leftOp;
          calc->u.binop.op2=rightOp;
          calc->u.binop.result=place;
          p->code=calc;
          insertCode(p);
        }
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

      Operand *baseOp=newTempvar();

      Type array = Exp(firstChild,baseOp);
      if (array==NULL) return NULL;
      if (array->kind!=ARRAY)
      {
        ErrorHandle(10, firstChild->line, firstChild->firstChild->text);
        return NULL;
      }

      int subscipt = 1;
			if(strcmp(firstChild->nextSibling->nextSibling->firstChild->type, "INT")==0)
				subscipt = atoi(firstChild->nextSibling->nextSibling->firstChild->text);
			Operand *subscriptOp=NULL;
			if(subscipt!=0)
      {
				subscriptOp = malloc(sizeof(Operand));
        subscriptOp->kind = OP_TEMPVAR;
				subscriptOp->u.var_no = tempvarNO++;
			}


      Type arrayNumber = Exp(firstChild->nextSibling->nextSibling,subscriptOp);
      if (arrayNumber==NULL) return NULL;
      if (arrayNumber->kind!=BASIC || arrayNumber->u.basic!=TYPE_INT)
      {
        ErrorHandle(12, firstChild->line, NULL);
        return NULL;
      }

			Operand *offsetOp=newTempvar();


			if(subscipt!=0){
        Operand *wideOp=newConstant();
				char* wideStr = malloc(30*sizeof(char));
				int wide = array->u.array.size;
				sprintf(wideStr, "%d", wide);
				wideOp->u.value = wideStr;

				InterCode *offsetCode = malloc(sizeof(InterCode));
        offsetCode->kind = IR_MUL;
				offsetCode->u.binop.op1 = subscriptOp;
				offsetCode->u.binop.op2 = wideOp;
				offsetCode->u.binop.result = offsetOp;
				InterCodes *p=malloc(sizeof(InterCodes));
        p->code=offsetCode;
				insertCode(p);

        InterCodes *q=malloc(sizeof(InterCodes));
				InterCode *addrCode = malloc(sizeof(InterCode));
        q->code=addrCode;
        addrCode->kind = IR_ADD;
				addrCode->u.binop.op1 = baseOp;
				addrCode->u.binop.op2 = offsetOp;
				if(array->u.array.elem->kind==BASIC)
        {
					Operand *temAddrOp=newTempvar();

					addrCode->u.binop.result = temAddrOp;
          place->kind = OP_TADDRESS;
					place->u.addr = temAddrOp;
				}
				else{
					addrCode->u.binop.result = place;
				}
				insertCode(q);
			}
			else{
        InterCodes *q=malloc(sizeof(InterCodes));
				InterCode *addrCode = malloc(sizeof(InterCode));
        q->code=addrCode;
        addrCode->kind = IR_ASSIGN;
				addrCode->u.assign.right = baseOp;
				if(array->u.array.elem->kind==BASIC){
          Operand *temAddrOp=newTempvar();
					addrCode->u.assign.left = temAddrOp;
          place->kind = OP_TADDRESS;
					place->u.addr = temAddrOp;
				}
				else{
					addrCode->u.assign.left = place;
				}
				insertCode(q);
			}

      //return array->u.array.elem;
      Type rtn = (Type)malloc(sizeof(struct Type_));
      memcpy(rtn, array->u.array.elem, sizeof(struct Type_));
      rtn->assign = BOTH;
      return rtn;
    }
    else if (strcmp(firstChild->nextSibling->type,"DOT")==0)
    {
      Operand *structVarOp=newTempvar();

      //Exp->Exp DOT ID
      Type structure = Exp(firstChild,structVarOp);
      if (structure==NULL) return NULL;
      if (structure->kind!=STRUCTURE)
      {
        ErrorHandle(13, firstChild->line, NULL);
        return NULL;
      }

      FieldList structDomain = structure->u.structure.domain;
      firstChild = firstChild->nextSibling->nextSibling;

      int offset=0;

      while(structDomain!=NULL)
      {
        if(strcmp(structDomain->name, firstChild->text) ==0)
        {

          if(offset==0){
						if(place!=NULL){
							if(structDomain->type->kind==BASIC)
              {
                place->kind = OP_VADDRESS;
								place->u.addr = structVarOp;
								
							}
							else{
								memcpy(place, structVarOp, sizeof(Operand));
							}
						}
					}
					else{
            Operand *offsetOp=newConstant();
						char* offsetStr = malloc(30*sizeof(char));
						sprintf(offsetStr, "%d", offset);
						offsetOp->u.value = offsetStr;
						InterCode *addrCode = malloc(sizeof(InterCode));
            addrCode->kind = IR_ADD;
						addrCode->u.binop.op1 = structVarOp;
						addrCode->u.binop.op2 = offsetOp;
						if(structure->kind==BASIC)
            {
              Operand *temAddrOp=newTempvar();

							addrCode->u.binop.result = temAddrOp;
              place->kind = OP_TADDRESS;
							place->u.addr = temAddrOp;
						}
						else{
							addrCode->u.binop.result = place;
						}
            InterCodes *p=malloc(sizeof(InterCodes));
            p->code=addrCode;
						insertCode(p);
					}
          Type rtn = (Type)malloc(sizeof(struct Type_));
          memcpy(rtn, structDomain->type, sizeof(struct Type_));
          rtn->assign = BOTH;
          return rtn;
         }
        structDomain = structDomain->tail;

        offset+=typeSize(structure);
      }
      ErrorHandle(14, firstChild->line, firstChild->text);
      return NULL;
    }
    return NULL;
  }
  else if (strcmp(firstChild->type,"LP")==0)
  {
    //Exp->LP Exp RP
    return Exp(firstChild->nextSibling,place);
  }
  else if (strcmp(firstChild->type,"MINUS")==0)
  {
    //Exp->MINUS Exp

    Operand *rightOp=newTempvar();
    int rightOpNO=rightOp->u.var_no;

    firstChild = firstChild->nextSibling;
    Type type = Exp(firstChild,rightOp);
    if (type==NULL) return NULL;
    if (type->kind!=BASIC)
    {
      ErrorHandle(7, firstChild->line, NULL);
      return NULL;
    }
    Operand *zeroOp=newConstant();
    zeroOp->u.value=zeroStr;
    if (place!=NULL)
    {
      InterCodes *p=malloc(sizeof(InterCodes));
      InterCode *minus=malloc(sizeof(InterCode));
      minus->kind=IR_SUB;
      minus->u.binop.op1=zeroOp;
      minus->u.binop.op2=rightOp;
      minus->u.binop.result=place;
      p->code=minus;
      insertCode(p);
    }
    Type rtn = (Type)malloc(sizeof(struct Type_));
    memcpy(rtn, type, sizeof(struct Type_));
    rtn->assign = RIGHT;
    return rtn;
  }
  else if (strcmp(firstChild->type,"NOT")==0)
  {
    //Exp->NOT Exp

    Operand *label1=newLabel();
    Operand *label2=newLabel();

    InterCodes *p0=malloc(sizeof(InterCodes));
    InterCode *code0=malloc(sizeof(InterCode));
    code0->kind=IR_ASSIGN;
    code0->u.assign.left=place;
    Operand *zero=newConstant();
    zero->u.value=zeroStr;
    code0->u.assign.right=zero;
    p0->code=code0;
    if (place!=NULL) insertCode(p0);
    Type t=Exp_Cond(n,label1,label2);

    InterCodes *p1=malloc(sizeof(InterCodes));
    InterCode *code1=malloc(sizeof(InterCode));
    code1->kind=IR_LABEL;
    code1->u.sinop.op=label1;
    p1->code=code1;
    insertCode(p1);

    Operand *cc=newConstant();
    cc->u.value=oneStr;
    InterCodes *p2=malloc(sizeof(InterCodes));
    InterCode *code2=malloc(sizeof(InterCode));
    code2->kind=IR_ASSIGN;
    code2->u.assign.left=place;
    code2->u.assign.right=cc;
    p2->code=code2;
    if (place!=NULL) insertCode(p2);

    InterCodes *p3=malloc(sizeof(InterCodes));
    InterCode *code3=malloc(sizeof(InterCode));
    code3->kind=IR_LABEL;
    code3->u.sinop.op=label2;
    p3->code=code3;
    insertCode(p3);

    t->assign = RIGHT;
    return t;
  }
  else if (strcmp(firstChild->type,"ID")==0)
  {
    if (firstChild->nextSibling==NULL)
    {
      //Exp -> ID
      Entry *value = getTable(firstChild->text);  //判断是否定义过
      if (value == NULL || value->ekind == FUNCTION)
      {
        ErrorHandle(1, firstChild->line, firstChild->text);
        return NULL;
      }

      if (place!=NULL)
      {
        place->kind=OP_VARIABLE;
        place->u.value=firstChild->text;
      }

      Type rtn = (Type)malloc(sizeof(struct Type_));
      memcpy(rtn, value->e.type, sizeof(struct Type_));
      rtn->assign = BOTH;
      return rtn;
    }
    else
    {
      //Exp->ID LP RP | ID LP Args RP
      Entry *func = getTable(firstChild->text);  //判断是否定义过
      if (func==NULL)
      {
        ErrorHandle(2, firstChild->line, firstChild->text);
        return NULL;
      }
      if (func->ekind!=FUNCTION)
      {
        ErrorHandle(11, firstChild->line, firstChild->text);
        return NULL;
      }
      FieldList param = func->e.function->param;
      firstChild = firstChild->nextSibling->nextSibling;
      if (strcmp(firstChild->type,"RP")==0)
      {
        if (param != NULL)
        {
          ErrorHandle(9, firstChild->line, func->name);
          return NULL;
        }
        else
        {
					if(strcmp(func->e.function->name,"read")==0){
						if(place!=NULL){
              InterCodes *p=malloc(sizeof(InterCodes));
							InterCode *funcCode = malloc(sizeof(InterCode));
              funcCode->kind = IR_READ;
							funcCode->u.sinop.op = place;
              p->code=funcCode;
							insertCode(p);
						}
					}
					else
          {
            Operand *funcOp=newFunction(func->e.function->name);
						if(place!=NULL){
              InterCodes *p=malloc(sizeof(InterCodes));
							InterCode *funcCode = malloc(sizeof(InterCode));
              funcCode->kind = IR_CALL;
							funcCode->u.assign.left = place;
							funcCode->u.assign.right = funcOp;
							p->code=funcCode;
							insertCode(p);
						}
						else
            {
							Operand *uselessOp = newTempvar();

              InterCodes *p=malloc(sizeof(InterCodes));
							InterCode *funcCode = malloc(sizeof(InterCode));
              funcCode->kind = IR_CALL;
							funcCode->u.assign.left = uselessOp;
							funcCode->u.assign.right = funcOp;
							p->code=funcCode;
							insertCode(p);
						}
					}
				}
      }
      else
      {
        Operand *argsListHead=malloc(sizeof(Operand));
        argsListHead->next=NULL;

        if (Args(firstChild,param,argsListHead)==false)   //比较两个类型是否匹配
        {
          ErrorHandle(9, firstChild->line, func->name);
          return NULL;
        }
        else
        {
					if(strcmp(func->e.function->name,"write")==0)
          {
            InterCodes *p=malloc(sizeof(InterCodes));
						InterCode *funcCode = malloc(sizeof(InterCode));
            funcCode->kind = IR_WRITE;
						funcCode->u.sinop.op = argsListHead->next;
            p->code=funcCode;
						insertCode(p);
					}
					else
          {
						Operand *argsP = argsListHead->next;
						while(argsP!=NULL)
            {
              InterCodes *p=malloc(sizeof(InterCodes));
							InterCode *argCode = malloc(sizeof(InterCode));
              argCode->kind = IR_ARG;
							argCode->u.sinop.op = argsP;
              p->code=argCode;
							insertCode(p);
							argsP = argsP->next;
						}
            Operand *funcOp=newFunction(func->e.function->name);
						if(place!=NULL)
            {
              InterCodes *p=malloc(sizeof(InterCodes));
							InterCode *funcCode = malloc(sizeof(InterCode));
              funcCode->kind = IR_CALL;
							funcCode->u.assign.left = place;
							funcCode->u.assign.right = funcOp;
              p->code=funcCode;
							insertCode(p);
						}
						else
            {
              
							Operand *uselessOp = newTempvar();

              InterCodes *p=malloc(sizeof(InterCodes));
							InterCode *funcCode = malloc(sizeof(InterCode));
              funcCode->kind = IR_CALL;
							funcCode->u.assign.left = uselessOp;
							funcCode->u.assign.right = funcOp;
							p->code=funcCode;
							insertCode(p);
						}
					}
				}
      }
      //return func->retype;
      Type rtn = (Type)malloc(sizeof(struct Type_));
 			memcpy(rtn, func->e.function->retype, sizeof(struct Type_));
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

    if (place!=NULL)
    {
      place->kind=OP_CONSTANT;
      place->u.value=firstChild->text;
    }

    return type;
  }
  else if (strcmp(firstChild->type,"FLOAT")==0)
  {
    //Exp -> FLOAT
    Type type = (Type)malloc(sizeof(struct Type_));
    type->kind = BASIC;
    type->u.basic = TYPE_FLOAT;
    type->assign = RIGHT;

    if (place!=NULL)
    {
      place->kind=OP_CONSTANT;
      place->u.value=firstChild->text;
    }

    return type;
  }
  return NULL;
}


/* exp condition */
Type Exp_Cond(Node *n,Operand *label_true,Operand *label_false)
{//printName(n->name);
	Node *firstChild = n->firstChild;
	Type type;
	if(strcmp(firstChild->type,"Exp")==0)
	{
		if(strcmp(firstChild->nextSibling->type,"RELOP")==0)//< >
		{
			//new temp
      Operand *t1=newTempvar();
      Operand *t2=newTempvar();

			Type tp=Exp(firstChild,t1);	//code1
			Type tp2=Exp(firstChild->nextSibling->nextSibling,t2);	//code2
			if(tp==NULL||tp2==NULL)return NULL;
			else if((tp->kind==BASIC||tp->kind==FUNCTION)&&(tp2->kind==BASIC||tp2->kind==STRUCTURE)&&tp->u.basic==tp2->u.basic)
			{
				InterCode *code3=malloc(sizeof(InterCode));
				code3->kind=IR_IFGOTO;
				code3->u.triop.op1=t1;
				code3->u.triop.relop=firstChild->nextSibling->text;
				code3->u.triop.op2=t2;
				code3->u.triop.label=label_true;
        InterCodes *p3=malloc(sizeof(InterCodes));
        p3->code=code3;
				insertCode(p3);		//code3

				InterCode *gotolbf=malloc(sizeof(InterCode));
				gotolbf->kind=IR_GOTO;
				gotolbf->u.sinop.op=label_false;
        InterCodes *pg=malloc(sizeof(InterCodes));
        pg->code=gotolbf;
				insertCode(pg);		//goto label false
				return tp;
			}
			else
			{
        ErrorHandle(7,firstChild->line,NULL);
				return NULL;
			}
		}
		else if(strcmp(firstChild->nextSibling->type,"AND")==0)
		{
			//new temp
			Operand *lb1=newLabel();

			Type t=Exp_Cond(firstChild,lb1,label_false);	//code1

			InterCode *lb1code=malloc(sizeof(InterCode));
			lb1code->kind=IR_LABEL;
			lb1code->u.sinop.op=lb1;
      InterCodes *pl=malloc(sizeof(InterCodes));
      pl->code=lb1code;
			insertCode(pl);		//label 1

			Type t2=Exp_Cond(firstChild->nextSibling->nextSibling,label_true,label_false);	//code2
			if(t==NULL||t2==NULL)return NULL;
			else if((t->kind==BASIC||t->kind==FUNCTION)&&(t2->kind==BASIC||t2->kind==FUNCTION)&&t->u.basic==t2->u.basic)
								return t;
			else
			{
        ErrorHandle(7,firstChild->line,NULL);
				return NULL;
			}

		}
		else if(strcmp(firstChild->nextSibling->type,"OR")==0)
		{
			//new temp
			Operand *lb1=newLabel();

			Type t=Exp_Cond(firstChild,label_true,lb1);	//code1

			InterCode *lb1code=malloc(sizeof(InterCode));
			lb1code->kind=IR_LABEL;
			lb1code->u.sinop.op=lb1;
      InterCodes *pl=malloc(sizeof(InterCodes));
      pl->code=lb1code;
			insertCode(pl);		//label 1

			Type t2=Exp_Cond(firstChild->nextSibling->nextSibling,label_true,label_false);	//code2
			if(t==NULL||t2==NULL)return NULL;
			else if((t->kind==BASIC||t->kind==FUNCTION)&&(t2->kind==BASIC||t2->kind==FUNCTION)&&t->u.basic==t2->u.basic)
				return t;
			else
			{
        ErrorHandle(7,firstChild->line,NULL);
				return NULL;
			}

		}

	}
	if(strcmp(firstChild->type,"NOT")==0)	//not
	{
		Type t=Exp_Cond(firstChild->nextSibling,label_false,label_true);
		if(t==NULL)return NULL;
		if(t->kind==BASIC&&t->u.basic==TYPE_INT)return t;
    ErrorHandle(7,firstChild->line,NULL);
		return NULL;
	}
	Operand *t1=newTempvar();
	type=Exp(n,t1);		//code1
	InterCode *code2=malloc(sizeof(InterCode));
	code2->kind=IR_IFGOTO;
	code2->u.triop.op1=t1;
	code2->u.triop.relop= neStr;
	Operand *t2=newConstant();
	t2->u.value=zeroStr;
	code2->u.triop.op2=t2;
	code2->u.triop.label=label_true;
  InterCodes *p2=malloc(sizeof(InterCodes));
  p2->code=code2;
	insertCode(p2);		//code2

	InterCode *gotolbf=malloc(sizeof(InterCode));
	gotolbf->kind=IR_GOTO;
	gotolbf->u.sinop.op=label_false;
  InterCodes *pf=malloc(sizeof(InterCodes));
  pf->code=gotolbf;
	insertCode(pf);		//goto label false
	return type;
}



bool Args(Node *n, FieldList param, Operand *arg)
{
  if (n==NULL||param==NULL) return false;

#ifdef IR_DEBUG
  printf("%s\n", n->type);
#endif

  Operand *t=newTempvar();

  Node *firstChild = n->firstChild;

 	Type tmpParam = Exp(firstChild,t);
   t->next=arg->next;
   arg->next=t;
 	if(typeEqual(param->type, tmpParam)==true)
  {
    //Args->Exp
 		if(firstChild->nextSibling==NULL)
 				return true;
 		else
 				return Args(firstChild->nextSibling->nextSibling, param->tail,arg);
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
        printf("Error type 12 at Line %d: Not an integer\n",line);
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
    return structEqual(lhs->u.structure.domain, rhs->u.structure.domain);
  }
  else
  {
    exit(-1);
  }

  return true;
}

bool structEqual(FieldList lhs, FieldList rhs)
{
  FieldList lDomain = lhs;
  FieldList rDomain = rhs;
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

 int typeSize(Type type)
 {
	if(type->kind==BASIC||type->kind==FUNCTION)
	{
		if(type->u.basic==TYPE_INT)
			return 4;
		else return 8;
	}
	else if(type->kind==STRUCTURE)	//struct
	{
		int size=0;
		FieldList f=type->u.structure.domain;
		while(f!=NULL)
		{
			size+=typeSize(f->type);
			f=f->tail;
		}
		return size;
	}
	else if(type->kind==ARRAY)		//array
	{
		//高维数组
		if(type->u.array.elem->kind==ARRAY)
		{
			printf("Cannot translate: Code contains variables of multi-dimensional array type or parameters of array type!\n");
			exit(-1);
		}
		return	type->u.array.size*typeSize(type->u.array.elem);
	}
	printf("type size error!\n");
	return 0;
}
