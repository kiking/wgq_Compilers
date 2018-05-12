%locations
%{
	#include<stdio.h>
	#include<stdlib.h>
	#include<string.h>
	#include<stdarg.h>
	#include "lex.yy.c"

	struct Node *root;

	int BError=0;

%}

/*declared types*/
%union{struct Node *node;}

/*declared tokens*/
%token 			<node>	INT FLOAT TYPE ID SEMI COMMA
%right 			<node>	ASSIGNOP NOT
%left  			<node>	PLUS MINUS STAR DIV RELOP AND OR
%left  			<node>	DOT LP RP LB RB LC RC
%nonassoc 	<node>	STRUCT RETURN IF ELSE WHILE

/*declared non-terminals*/
%type 		<node>	Program ExtDefList ExtDef ExtDecList
%type			<node>	Specifier StructSpecifier OptTag Tag
%type 		<node>	VarDec FunDec VarList ParamDec
%type 		<node>	CompSt StmtList Stmt
%type 		<node>	DefList Def DecList Dec
%type 		<node>	Exp Args


%%

/* High-level Definitions */
Program					:	ExtDefList										{$$=addToTree("Program",1,$1);root=$$;}
								;

ExtDefList			:	ExtDef ExtDefList							{$$=addToTree("ExtDefList",2,$1,$2);}
								|	/*empty*/											{$$=add("ExtDefList",@$.first_line);}
								;

ExtDef					:	Specifier ExtDecList SEMI			{$$=addToTree("ExtDef",3,$1,$2,$3);}			//全局变量申明 int x,y,z;
								|	Specifier SEMI								{$$=addToTree("ExtDef",2,$1,$2);}					//类型申明 struct xxx;
								|	Specifier FunDec CompSt				{$$=addToTree("ExtDef",3,$1,$2,$3);}			//函数 int f(...){...}
								|	error SEMI										{BError = 1;}
								;

ExtDecList			:	VarDec												{$$=addToTree("ExtDecList",1,$1);}
								|	VarDec COMMA ExtDecList				{$$=addToTree("ExtDecList",3,$1,$2,$3);}	//变量序列 x,y,z,...
								;


/* Specifiers */
Specifier				:	TYPE													{$$=addToTree("Specifier",1,$1);}					//基本类型
								|	StructSpecifier								{$$=addToTree("Specifier",1,$1);}					//结构体
								;

StructSpecifier	:	STRUCT OptTag LC DefList RC		{$$=addToTree("StructSpecifier",5,$1,$2,$3,$4,$5);}		//结构体定义
								|	STRUCT Tag										{$$=addToTree("StructSpecifier",2,$1,$2);}						//结构体
								;

OptTag					:	ID														{$$=addToTree("OptTag",1,$1);}								//optional tag 可选的结构体名
								|	/*empty*/											{$$=add("OptTag",@$.first_line);}
								;

Tag							:	ID														{$$=addToTree("Tag",1,$1);}
								;


/* Declarations */
VarDec					:	ID														{$$=addToTree("VarDec",1,$1);}								//变量描述 id
								|	VarDec LB INT RB							{$$=addToTree("VarDec",4,$1,$2,$3,$4);}				//数组变量表示 id[i][j]
								;

FunDec					:	ID LP VarList RP							{$$=addToTree("FunDec",4,$1,$2,$3,$4);}				//函数描述符 f(参数)
								|	ID LP RP											{$$=addToTree("FunDec",3,$1,$2,$3);}					//函数描述符 f()
								|	error RP											{BError = 1;}
								;

VarList					:	ParamDec COMMA VarList				{$$=addToTree("VarList",3,$1,$2,$3);}					//int x,int y,...
								|	ParamDec											{$$=addToTree("VarList",1,$1);}								//int x
								;

ParamDec				:	Specifier VarDec							{$$=addToTree("ParamDec",2,$1,$2);}						//int x
								;


/* Statements */
CompSt					:	LC DefList StmtList RC				{$$=addToTree("CompSt",4,$1,$2,$3,$4);}				//{定义、初始化，stmtlist}
								|	error RC											{BError = 1;}
								;

StmtList				:	Stmt StmtList									{$$=addToTree("StmtList",2,$1,$2);}
								|	/*empty*/											{$$=add("StmtList",@$.first_line);}
								;

Stmt						:	Exp SEMI											{$$=addToTree("Stmt",2,$1,$2);}										//表达式
								|	CompSt												{$$=addToTree("Stmt",1,$1);} 											//{定义、初始化，stmtlist}
								|	RETURN Exp SEMI								{$$=addToTree("Stmt",3,$1,$2,$3);}								//return 表达式
								| IF LP Exp RP Stmt							{$$=addToTree("Stmt",5,$1,$2,$3,$4,$5);}					//if(表达式) stmt
								|	IF LP Exp RP Stmt ELSE Stmt		{$$=addToTree("Stmt",7,$1,$2,$3,$4,$5,$6,$7);}		//if(表达式) stmt else stmt
								|	WHILE LP Exp RP Stmt					{$$=addToTree("Stmt",5,$1,$2,$3,$4,$5);}					//while(表达式) stmt
								|	error SEMI										{BError = 1;}
								| error													{BError = 1;}
								;


/* Local Definitions */
DefList					:	Def DefList										{$$=addToTree("DefList",2,$1,$2);}					//int x,y=1;float m=1.0;定义、初始化列表
								|	/*empty*/											{$$=add("DefList",@$.first_line);}																	//
								;

Def							:	Specifier DecList SEMI				{$$=addToTree("Def",3,$1,$2,$3);}						//int x,y=1;
								|	error SEMI										{BError = 1;}															//
								;

DecList					:	Dec														{$$=addToTree("DecList",1,$1);}							//
								|	Dec COMMA DecList							{$$=addToTree("DecList",3,$1,$2,$3);}				//id1,id2=表达式，id3...
								;

Dec							:	VarDec												{$$=addToTree("Dec",1,$1);}									//id
								|	VarDec ASSIGNOP	Exp						{$$=addToTree("Dec",3,$1,$2,$3);}						//id=表达式
								;


/* Expressions */
Exp							:	Exp ASSIGNOP Exp							{$$=addToTree("Exp",3,$1,$2,$3);}
								|	Exp AND Exp										{$$=addToTree("Exp",3,$1,$2,$3);}
								|	Exp OR Exp										{$$=addToTree("Exp",3,$1,$2,$3);}
								|	Exp RELOP Exp									{$$=addToTree("Exp",3,$1,$2,$3);}
								|	Exp PLUS Exp									{$$=addToTree("Exp",3,$1,$2,$3);}
								|	Exp MINUS Exp									{$$=addToTree("Exp",3,$1,$2,$3);}
								|	Exp STAR Exp									{$$=addToTree("Exp",3,$1,$2,$3);}
								|	Exp DIV Exp										{$$=addToTree("Exp",3,$1,$2,$3);}
								|	LP Exp RP											{$$=addToTree("Exp",3,$1,$2,$3);}
								|	MINUS Exp											{$$=addToTree("Exp",2,$1,$2);}
								|	NOT Exp												{$$=addToTree("Exp",2,$1,$2);}
								|	ID LP Args RP									{$$=addToTree("Exp",4,$1,$2,$3,$4);}
								|	ID LP RP											{$$=addToTree("Exp",3,$1,$2,$3);}
								|	Exp LB Exp RB									{$$=addToTree("Exp",4,$1,$2,$3,$4);}
								|	Exp DOT ID										{$$=addToTree("Exp",3,$1,$2,$3);}
								|	ID														{$$=addToTree("Exp",1,$1);}
								|	INT														{$$=addToTree("Exp",1,$1);}
								|	FLOAT													{$$=addToTree("Exp",1,$1);}
								| error													{BError = 1;}
								;

Args						:	Exp COMMA Args								{$$=addToTree("Args",3,$1,$2,$3);}
								|	Exp														{$$=addToTree("Args",1,$1);}
								;

%%

yyerror(char* msg){
	fprintf(stderr,"Error type B at line %d,column %d:%s\n",yylineno,yycolumn,msg);
}

struct Node *addToTree(char *type,int num,...){
	struct Node *current = (struct Node *)malloc(sizeof(struct Node));
	struct Node *temp = (struct Node *)malloc(sizeof(struct Node));
	current->isToken = 0;
	va_list nodeList;
	va_start(nodeList,num);
	temp = va_arg(nodeList,struct Node*);
	current->line = temp->line;
	strcpy(current->type,type);
	current->firstChild = temp;
	int i;
	for(i = 1 ; i < num ; i++){
		temp->nextSibling = va_arg(nodeList,struct Node*);
		if(temp->nextSibling != NULL)
			temp = temp->nextSibling;
	}
	temp->nextSibling = NULL;
	va_end(nodeList);
	return current;
}

struct Node *add(char *type,int line)
{
	struct Node *current = (struct Node *)malloc(sizeof(struct Node));
	current->isToken = 0;
	current->line=line;
	strcpy(current->type,type);
	current->firstChild = NULL;
	return current;
}
