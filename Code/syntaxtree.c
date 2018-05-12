#include "syntaxtree.h"
#include <string.h>

void printTree(struct Node *p,int depth){
	if(p == NULL) return;
	int i;
	for(i = 0 ; i < depth ; i++)
		printf("  ");
	if(!p->isToken){
		printf("%s (%d)\n", p->type, p->line);
		printTree(p->firstChild , depth+1);
	}
	else{
		if(strcmp(p->type,"INT") == 0)
			printf("%s: %d\n", p->type, atoi(p->text));
		else if(strcmp(p->type,"FLOAT") == 0)
			printf("%s: %f\n", p->type, atof(p->text));
		else if(strcmp(p->type,"TYPE") == 0 || strcmp(p->type,"ID") == 0)
			printf("%s: %s\n", p->type, p->text);
		else
			printf("%s\n", p->type);
	}
	printTree(p->nextSibling , depth);
}
