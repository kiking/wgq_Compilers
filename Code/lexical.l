%option yylineno

%{
	#include <stdio.h>
	#include <string.h>
	#include "syntax.tab.h"
	#include "syntaxtree.h"

	int yycolumn=1;

	#define YY_USER_ACTION \
		yylloc.first_line = yylloc.last_line = yylineno; \
		yylloc.first_column = yycolumn; \
		yylloc.last_column = yycolumn + yyleng - 1; \
		yycolumn += yyleng;


	void setNode(char *type,char *text);
	int AError=0;
%}

/*regular definitions */
space				[ \t\r]
ws					{space}+
line    		[\n]
letter			[A-Za-z]
digit				[0-9]
type				int|float
id					({letter}|_)({letter}|{digit}|_)*
int					[0]|[1-9][0-9]*
float				{digit}+\.{digit}+
relop				>|<|>=|<=|==|!=
bcomment		"/*"([^\*]|(\*)*[^\*/])*(\*)*"*/"

%%
{line}						{yycolumn=1;}
{ws}							{/*no action and no return*/}
struct						{setNode("STRUCT",yytext);return(STRUCT);}
return						{setNode("RETURN",yytext);return(RETURN);}
if								{setNode("IF",yytext);return(IF);}
else							{setNode("ELSE",yytext);return(ELSE);}
while							{setNode("WHILE",yytext);return(WHILE);}
{type}						{setNode("TYPE",yytext);return TYPE;}
{int}							{setNode("INT",yytext);return INT;}
{float}						{setNode("FLOAT",yytext);return FLOAT;}
{id}							{setNode("ID",yytext);return ID;}
{relop}						{setNode("RELOP",yytext);return RELOP;}
";"								{setNode("SEMI",yytext);return SEMI;}
","								{setNode("COMMA",yytext);return COMMA;}
"="								{setNode("ASSIGNOP",yytext);return ASSIGNOP;}
"+"								{setNode("PLUS",yytext);return PLUS;}
"-"								{setNode("MINUS",yytext);return MINUS;}
"*"								{setNode("STAR",yytext);return STAR;}
"/"								{setNode("DIV",yytext);return DIV;}
"&&"							{setNode("AND",yytext);return AND;}
"||"							{setNode("OR",yytext);return OR;}
"."								{setNode("DOT",yytext);return DOT;}
"!"								{setNode("NOT",yytext);return NOT;}
"("								{setNode("LP",yytext);return LP;}
")"								{setNode("RP",yytext);return RP;}
"["								{setNode("LB",yytext);return LB;}
"]"								{setNode("RB",yytext);return RB;}
"{"								{setNode("LC",yytext);return LC;}
"}"								{setNode("RC",yytext);return RC;}
"//"							{char c=input();while (c!='\n') c=input();}
{bcomment}				{/*block comment*/}
.									{AError=1;printf("Error type A at line %d,Column %d: Mysterious character \'%s\'\n", yylloc.first_line,yylloc.first_column, yytext);}

%%

void setNode(char *type,char *text){
	yylval.node = (struct Node *)malloc(sizeof(struct Node));
	yylval.node->isToken = 1;
	yylval.node->line = yylloc.first_line;
	yylval.node->column = yylloc.first_column;
	strcpy(yylval.node->type, type);
	strcpy(yylval.node->text, text);
	yylval.node->firstChild = NULL;
	yylval.node->nextSibling = NULL;
}
