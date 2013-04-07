#define SAVE_SCREEN         // this directive forces saving/restoring the screen
#define USE_TI89
#define OPTIMIZE_ROM_CALLS

#define MAX_TOKENS 200
#define MAX_INPUT 1000
#define ESC 8

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <kbd.h>
#include <timath.h>

char SYMBOL_CONTAINS[] = "0123456789abcdefghijklmnopqrstuvwxyz"
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ!$%&*/:<=>?@^_~+-.";
char WHITESPACE[] = " \n\t";
char SINGLE_CHAR_TOKENS[] = "()' \n\t";

enum {T_SYMBOL, T_NUMBER, T_TRUE, T_FALSE, T_PAIR, T_procedure};

enum {P_PRIMITIVE, P_LAMBDA};

struct token;
struct pair;
struct procedure;

struct token {
  int type;
  char *symbol;
  double number;
  struct pair *pair;
  struct procedure *procedure;
};
typedef struct token Token;

struct pair {
  Token item;
  struct pair *next;
};
typedef struct pair Pair;

struct procedure {
  int type;
  Token (*func)(Token, Token);
  Pair *proc_list;
  Token *rest;
};
typedef struct procedure Procedure;

struct bNode {
  char *key;
  Token item;
  struct bNode *next;
};
typedef struct bNode BNode;

struct binding {
  BNode *head;
  struct binding *parent;
};
typedef struct binding Binding;

void printList();
void printPair();
void printToken();
Token scheme_read();
Pair *read_tail();
Token cons();

Token tokenBuffer[MAX_TOKENS];
Token *tp;
char *ip;


BNode *bnodealloc() {
  return (BNode *) malloc(sizeof(BNode));
}

Binding *bindingalloc() {
  return (Binding *) malloc(sizeof(Binding));
}

Binding *makeNewFrame(Binding *parent) {
  Binding *b = bindingalloc();
  b->head = NULL;
  b->parent = parent;
  return b;
}

BNode *findBinding(Binding *b, char *key) {
  BNode *currNode = b->head;
  while (currNode != NULL) {
    if (strcmp(currNode->key, key) == 0) {
      return currNode;
    }
    currNode = currNode->next;
  }
  if (b->parent != NULL) {
    return findBinding(b->parent, key);
  }
  return NULL;
}

void addBinding(Binding *b, char *key, Token item) {
  BNode *temp = b->head;
  BNode *new = bnodealloc();  
  new->item = item;
  new->key = key;
  new->next = temp;
  b->head = new;  
}

void printBinding(Binding *b) {
  BNode *curr = b->head;
  printf("{");
  while (curr != NULL) {
    printf("%s: ", curr->key);
    printToken(curr->item);
    printf(", ");
    curr = curr->next;
  }
  printf("}");
}

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

Token tokenize(char *input) {  
  char *currtok;
  char inputcp[MAX_INPUT];
  tp = &tokenBuffer[0];  
  strcpy(inputcp, input);
  currtok = strtok(inputcp, SINGLE_CHAR_TOKENS);
  do {
    *(tp++) = tFromStr(currtok);    
  } while ((currtok = strtok(NULL, SINGLE_CHAR_TOKENS)) != NULL);
  ip = input;
  tp = &tokenBuffer[0];
  return scheme_read();
}

void run_down_symbol() {
  while (strchr(SINGLE_CHAR_TOKENS, *(++ip)) == NULL)
    ;
}

Token scheme_read() {
  Token temp;
  if (*ip == ' ') {
    while (*(++ip) == ' ')
      ;
  }
  switch (*ip) {
  case '\'':
    ip++;
    temp = scheme_read();    
    return cons(tFromStr("quote"), cons(temp, tFromStr("nil")));
  case '(':
    ip++;
    return tFromPair(read_tail());  
  default:
    run_down_symbol();
    return *(tp++);
  }
}

Pair *read_tail() {
  Token first;
  Pair *rest;
  if (*ip == ' ') {
    while (*(++ip) == ' ')
      ;
  }
  switch (*ip) {
  case ')':
    ip++;
    return NULL;  
  default:
    first = scheme_read();
    rest = read_tail();
    return makePair(first, rest);
  }
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

short scheme_true(Token t) {
  return (t.type != T_FALSE);
}

short nullp(Token t) {
  return (t.type == T_PAIR && t.pair == NULL);
}

Token eval(Token t) {
  switch (t.type) {
  case T_SYMBOL:
    break;
  case T_NUMBER: case T_TRUE: case T_FALSE:
    return t;
  case T_PAIR:
    break;
  }
}

void test() {
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
  printToken(tokenize("(define fact (lambda (n) (if (<= n 1) 1 (* n (fact (- n 1))))))"));  
  ngetchx();
  clrscr();
  char *ca = "a";
  char *cb = "b";
  char *cc = "c";  
  Binding *b = makeNewFrame(NULL);
  Binding *c = makeNewFrame(b);
  addBinding(b, ca, t1);
  addBinding(b, cc, null);
  addBinding(c, cb, t2);
  addBinding(c, cc, t1);
  printToken(findBinding(b, ca)->item);
  printf("\n");
  printToken(findBinding(c, cb)->item);
  printf("\n");
  printToken(findBinding(c, cc)->item);
  printf("\n");
  printToken(findBinding(c, ca)->item);
  printf("\n");
  ngetchx();
}

_main() {
  if (0) {
    char buf[MAX_INPUT];
    int cp = 0;
    char *bp;
    clrscr();
    while (1) {
      getsn(buf, MAX_INPUT);
      if (buf[0] == ESC)
        break;
      bp = buf;
      while (*bp != '\0') {
        if (*bp == '(')
          cp++;
        else if (*bp == ')')
          cp--;
        bp++;
      }
      while (cp != 0) {
        *(bp++) = ')';      
        cp--;
      }
      *bp = '\0';
      printf("\n");
      printToken(tokenize(buf));
      printf("\n");    
    }
  } else {
    test();
  }
}
