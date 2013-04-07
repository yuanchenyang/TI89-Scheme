#define SAVE_SCREEN         // this directive forces saving/restoring the screen
#define USE_TI89
#define OPTIMIZE_ROM_CALLS

#define MAX_TOKENS 200
#define MAX_INPUT 1000

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <kbd.h>
#include <timath.h>

char SYMBOL_CONTAINS[] = "0123456789abcdefghijklmnopqrstuvwxyz"
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ!$%&*/:<=>?@^_~+-.";
char WHITESPACE[] = " \n\t";
char SINGLE_CHAR_TOKENS[] = "()' \n\t";

enum {T_SYMBOL, T_NUMBER, T_TRUE, T_FALSE, T_PAIR, T_END};

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

Token tokenBuffer[MAX_TOKENS];

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
  if (p == NULL) {
    printf ("()");
  } else {
    printf("(");
    printPair(p);
    printf(")");
  }
}

Token *tokenalloc(void) {
  return (Token *) malloc(sizeof(Token));
}

Token tFromStr(char *s) {
  double number;
  Token *t;
  t = tokenalloc();
  if (is_nan(number = atof(s))) {
    if (strcmp(s, "#t") == 0) {
      t->type = T_TRUE;
    } else if (strcmp(s, "#f") == 0) {
      t->type = T_FALSE;
    } else if (strcmp(s, "nil") == 0) {
      t->type = T_PAIR;
      t->pair = NULL;
    } else {
      t->type = T_SYMBOL;
      t->symbol = s;
    }
  } else {
    t->type = T_NUMBER;
    t->number = number;
  }
  return *t;
}

Token tFromPair(Pair *p) {
  Token *t;
  t = tokenalloc();
  t->type = T_PAIR;
  t->pair = p;
  return *t;
}

Token *tokenize(char *input) {
  Token *tp = &tokenBuffer[0];
  char *currtok;
  char inputcp[MAX_INPUT];
  strcpy(inputcp, input);
  currtok = strtok(inputcp, SINGLE_CHAR_TOKENS);
  do {
    *(tp++) = tFromStr(currtok);
  } while ((currtok = strtok(NULL, SINGLE_CHAR_TOKENS)) != NULL);
  Token end;
  end.type = T_END;
  *tp = end;
  return tokenBuffer;
}

Token car(Token t) {
  if (t.type == T_PAIR)
    return t.pair->item;
}

Token cdr(Token t) {
  if (t.type == T_PAIR)
    return tFromPair(t.pair->next);
}

Token cons(Token t1, Token t2) {
  if (t2.type == T_PAIR)
    return tFromPair(makePair(t1, t2.pair));  
}

_main() {
  clrscr();
  Token null = tFromStr("nil");
  Token t1 = tFromStr("vas");
  Token t2 = tFromStr("102.32");
  Token t3 = cons(t1, cons(t2, null));
  Token t0 = cons(t3, cons(null, null));
  printToken(t0);
  printf("\n");
  printToken(car(t0));
  printf("\n");
  printToken(cdr(cdr(car(t0))));
  printf("\n");
  Token *tp = tokenize("(define (enumerate-tree tree)(cond ((null? tree) nil)((not (pair? tree)) (list tree))(else (append (enumerate-tree (car tree))(enumerate-tree (cdr tree))))))(enumerate-tree (list 1 (list 2 (list 3 4)) 5))");
  while ((*tp).type != T_END) {
    printToken(*tp);
    printf(" ");
    tp++;
  }  
  ngetchx();
}
