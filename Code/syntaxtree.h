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

struct Node *addToTree(char *type,int num,...);
struct Node *add(char *type,int line);
void printTree(struct Node *p,int depth);

#endif
