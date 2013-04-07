#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_SYMBOL_LENGTH 20

char SYMBOL_CONTAINS[] = "0123456789abcdefghijklmnopqrstuvwxyz"
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ!$%&*/:<=>?@^_~+-.";
char WHITESPACE[] = " \n\t";
char SINGLE_CHAR_TOKENS[] = "()'";
char TOKEN_END[] = " \n\t()'";
char DELIMITERS[] = "()'";

enum {T_SYMBOL, T_NUMBER, T_TRUE, T_FALSE, T_NIL, T_PAIR};

struct token;
struct pair;

struct token {
  int type;
  char *symbol;
  double number;
  struct pair *pair;
};
typedef struct token Token;

struct pair {
  Token item;
  struct pair *next;
};
typedef struct pair Pair;

void printList();
void printPair();

Pair *pairalloc(void) {
  return (Pair *) malloc(sizeof(Pair));
}

Pair *makePair(Token item, Pair *next) {
  Pair *p;
  p = pairalloc();
  p->item = item;
  p->next = next;
  return p;
}

void printToken(Token t) {
  switch (t.type) {
  case T_SYMBOL:
    printf("%s", t.symbol);
    break;
  case T_NUMBER:
    printf("%.12g", t.number);
    break;
  case T_TRUE:
    printf("#t");
    break;
  case T_FALSE:
    printf("#f");
    break;
  case T_PAIR:
    printList(t.pair);
  }
}

void printPair(Pair *p) {
  printToken(p->item);
  if (p->next != NULL) {
    printf(" ");
    printPair(p->next);
  }
}

void printList(Pair *p) {
  printf("(");
  printPair(p);
  printf(")");
}

Token *tokenize(char *input) {
  
}

main() {
  Token t1, t2, t3, t4;
  t1.type = T_NUMBER;
  t1.number = 1;
  t2.type = T_NUMBER;
  t2.number = 2;
  t3.type = T_PAIR;
  t3.pair = makePair(t1, makePair(t2, NULL));
  printList(makePair(t3, makePair(t2, NULL)));
  printf("\n");
}
