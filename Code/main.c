#include <stdio.h>
#include "syntaxtree.h"
#include "hashtable.h"
#include "semantic.h"
#include "intercode.h"

extern struct Node* root;
extern int yylineno;
extern int BError;
extern int AError;
int main(int argc, char** argv)
{
  if (argc < 1)
	{
		printf("Usage:./parser + filename + filename ...!!\n");
		return 1;
	}
	FILE *f=fopen(argv[1],"r");
	if(!f){
		perror(argv[1]);
		return 1;
	}
	root = NULL;
	BError = 0;
  AError = 0;
	yylineno = 1;
	yyrestart(f);
	yyparse();

	zeroStr = malloc(sizeof(char[2]));
	strcpy(zeroStr, "0");
	oneStr = malloc(sizeof(char[2]));
	strcpy(oneStr, "1");
	neStr = malloc(sizeof(char[3]));
	strcpy(neStr, "!=");

	if(root != NULL && AError == 0 && BError == 0)
  {
    printTree(root,0);
    Program(root);
		printCode(argv[2]);
  }
	
  return 0;
}
