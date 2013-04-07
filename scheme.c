#define SAVE_SCREEN         // this directive forces saving/restoring the screen
#define USE_TI89
#define OPTIMIZE_ROM_CALLS

#define MAX_TOKENS 200
#define MAX_INPUT 1000
#define MAXSTR 15
#define ESC 8

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <kbd.h>
#include <timath.h>
#include <mem.h>

char SYMBOL_CONTAINS[] = "0123456789abcdefghijklmnopqrstuvwxyz"
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ!$%&*/:<=>?@^_~+-.";
char WHITESPACE[] = " \n\t";
char SINGLE_CHAR_TOKENS[] = "()' \n\t";

enum {T_SYMBOL, T_NUMBER, T_TRUE, T_FALSE, T_PAIR, T_PROCEDURE};

enum {P_PRIMITIVE, P_LAMBDA};

struct token;
struct pair;
struct procedure;

struct token {
  int type;
  char symbol[MAXSTR];
  double number;
  struct pair *pair;
  struct procedure *procedure;
};
typedef struct token Token;

struct pair {
  Token *item;
  struct pair *next;
};
typedef struct pair Pair;

struct bNode {
  char key[MAXSTR];
  struct token *item;
  struct bNode *next;
};
typedef struct bNode BNode;

struct binding {
  BNode *head;
  struct binding *parent;
};
typedef struct binding Binding;

struct procedure {
  int type;
  Token *(*func)(Token *, Token *);
  Pair *args_list;
  Binding *env;
  Token *rest;
};
typedef struct procedure Procedure;

void printList();
void printPair();
void printToken();
Token *scheme_read();
Token *tokenalloc();
Pair *read_tail();
Token *cons();
void addBinding();
Token *eval();
Token *apply();
Token *tFromNumber();
Token *tFromStr();
Binding *makeNewFrame();

Token tokenBuffer[MAX_TOKENS];
Token *tp;
char *ip;


Procedure *procalloc() {
  return (Procedure *) malloc(sizeof(Procedure));
}

void bindPrimitiveProc(Binding *b, char *symbol,
                       Token *(*func)(Token *, Token *)) {
  Procedure *p = procalloc();
  p->type = P_PRIMITIVE;
  p->func = func;
  Token *t = tokenalloc();
  t->type = T_PROCEDURE;
  t->procedure = p;
  addBinding(b, symbol, t);
}

Token *makeLambda(Binding *b, Pair *formals, Token *value) {
  Procedure *p = procalloc();
  Binding *callframe = makeNewFrame(b);
  p->type = P_LAMBDA;
  p->args_list = formals;
  p->env = callframe;
  p->rest = value;
  Token *t = tokenalloc();
  t->type = T_PROCEDURE;
  t->procedure = p;
  return t;
}

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

void addBinding(Binding *b, char *key, Token *i) {
  BNode *temp = b->head;
  BNode *new = bnodealloc();
  Token *t = tokenalloc();
  strcpy(&((new->key)[0]), key);
  memcpy(t, i, sizeof(Token));
  new->next = temp;  
  new->item = t;
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

Pair *makePair(Token *item, Pair *next) {
  Pair *p;
  p = pairalloc();
  p->item = item;
  p->next = next;
  return p;
}

void printToken(Token *t) {
  switch (t->type) {
  case T_SYMBOL:
    printf("%s", t->symbol);
    break;
  case T_NUMBER:
    printf("%.12g", t->number);
    break;
  case T_TRUE:
    printf("#t");
    break;
  case T_FALSE:
    printf("#f");
    break;
  case T_PAIR:
    printList(t->pair);
    break;
  case T_PROCEDURE:
    printf("<procedure>");
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

Token *tFromNumber(double d) {
  Token *t;
  t = tokenalloc();
  t->type = T_NUMBER;
  t->number = d;
  return t;
}

Token *tFromStr(char *s) {
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
      strcpy(&((t->symbol)[0]), s);
    }
  } else {
    t->type = T_NUMBER;
    t->number = number;
  }
  return t;
}

Token *tFromPair(Pair *p) {
  Token *t;
  t = tokenalloc();
  t->type = T_PAIR;
  t->pair = p;
  return t;
}

Token *tFromPredicate(short i) {
  Token *t;
  t = tokenalloc();
  if (i)
    t->type = T_TRUE;
  else
    t->type = T_FALSE;
  return t;
}

Token *tokenize(char *input) {  
  char *currtok;
  char inputcp[MAX_INPUT];
  tp = &tokenBuffer[0];  
  strcpy(inputcp, input);
  currtok = strtok(inputcp, SINGLE_CHAR_TOKENS);
  do {
    *(tp++) = *tFromStr(currtok);
  } while ((currtok = strtok(NULL, SINGLE_CHAR_TOKENS)) != NULL);
  ip = input;
  tp = &tokenBuffer[0];
  return scheme_read();
}

void run_down_symbol() {
  while (strchr(SINGLE_CHAR_TOKENS, *(++ip)) == NULL)
    ;
}

Token *scheme_read() {
  Token *temp;
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
    return tp++;
  }
}

Pair *read_tail() {
  Token *first;
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

Token *car(Token *t, Token *dummy) {
  if (t == NULL)
    return NULL;
  if (t->type == T_PAIR)
    return (t->pair)->item;
  return NULL;
}

Token *cdr(Token *t, Token *dummy) {
  if (t->type == T_PAIR)
    return tFromPair((t->pair)->next);
}

Token *cons(Token *t1, Token *t2) {
  if (t2->type == T_PAIR)
    return tFromPair(makePair(t1, t2->pair));
}

Token* scheme_add(Token *t1, Token *t2) {
  if (t1->type == T_NUMBER && t2->type == T_NUMBER)
    return tFromNumber(t1->number + t2->number);
}

Token* scheme_sub(Token *t1, Token *t2) {
  if (t1->type == T_NUMBER && t2->type == T_NUMBER)
    return tFromNumber(t1->number - t2->number);
}

Token* scheme_mul(Token *t1, Token *t2) {
  if (t1->type == T_NUMBER && t2->type == T_NUMBER)
    return tFromNumber(t1->number * t2->number);
}

Token* scheme_div(Token *t1, Token *t2) {
  if (t1->type == T_NUMBER && t2->type == T_NUMBER)
    return tFromNumber(t1->number / t2->number);
}

Token* scheme_lt(Token *t1, Token *t2) {
  if (t1->type == T_NUMBER && t2->type == T_NUMBER)
    return tFromPredicate(t1->number < t2->number);
}

Token* scheme_gt(Token *t1, Token *t2) {
  if (t1->type == T_NUMBER && t2->type == T_NUMBER)
    return tFromPredicate(t1->number > t2->number);
}

short scheme_true(Token *t) {
  return (t->type != T_FALSE);
}

short nullp(Token *t) {
  return (t->type == T_PAIR && t->pair == NULL);
}

Binding *makeGlobalFrame() {
  Binding *b = makeNewFrame(NULL);
  bindPrimitiveProc(b, "car", (Token *(*)(Token *, Token *)) car);
  bindPrimitiveProc(b, "cdr", (Token *(*)(Token *, Token *)) cdr);
  bindPrimitiveProc(b, "cons", (Token *(*)(Token *, Token *)) cons);
  bindPrimitiveProc(b, "+", (Token *(*)(Token *, Token *)) scheme_add);
  bindPrimitiveProc(b, "-", (Token *(*)(Token *, Token *)) scheme_sub);
  bindPrimitiveProc(b, "*", (Token *(*)(Token *, Token *)) scheme_mul);
  bindPrimitiveProc(b, "/", (Token *(*)(Token *, Token *)) scheme_div);
  bindPrimitiveProc(b, "<", (Token *(*)(Token *, Token *)) scheme_lt);
  bindPrimitiveProc(b, ">", (Token *(*)(Token *, Token *)) scheme_gt);
  return b;
}

Token *eval(Token *t, Binding *b) {
  Token *first;
  Token *rest;
  switch (t->type) {
  case T_SYMBOL:
    return findBinding(b, t->symbol)->item;
  case T_NUMBER:
  case T_TRUE:
  case T_FALSE:
    return t;   
  case T_PAIR:
    if (t->pair == NULL)
      return tFromStr("nil");
    first = car(t, NULL);
    rest = cdr(t, NULL); 
    if (strcmp(first->symbol, "define") == 0) {
      addBinding(b, car(rest, NULL)->symbol,
                 eval(car(cdr(rest, NULL), NULL), b));
      return car(rest, NULL);
    } else if (strcmp(first->symbol, "quote") == 0) {
      return car(rest, NULL);
    } else if (strcmp(first->symbol, "if") == 0) {
      if (scheme_true(eval(car(rest, NULL), b)))
        return eval(car(cdr(rest, NULL), NULL), b);
      else
        return eval(car(cdr(cdr(rest, NULL), NULL), NULL), b);
    } else if (strcmp(first->symbol, "lambda") == 0) {
      return makeLambda(b, rest->pair->item->pair, rest->pair->next->item);
    } else {
      return apply(eval(first, b)->procedure, rest, b);
    }
    break;
  }
}

Token *apply(Procedure *p, Token *args, Binding *b) {
  Pair *formals;
  Pair *pargs;
  Binding *callframe;
  switch (p->type) {
  case P_PRIMITIVE:
    return (p->func)(eval(car(args, NULL), b),
                     eval(car(cdr(args, NULL), NULL), b));
  case P_LAMBDA:
    formals = p->args_list;
    callframe = p->env;
    pargs = args->pair;
    while (formals != NULL) {
      addBinding(b, formals->item->symbol, eval(pargs->item, b));
      pargs = pargs->next;
      formals = formals->next;
    }
    return eval(p->rest, callframe);
  }
}

void test() {
  clrscr();
  Binding *global = makeGlobalFrame();
  printToken(eval(tokenize("(define f (lambda (x) (lambda (y) (+ x y))))"), global));
  printf("\n");
  printToken(eval(tokenize("((f 4) 3)"), global));
  printf("\n");
  printToken(eval(tokenize("(+ (* 3 (+ (* 2 4) (+ 3 5))) (+ (- 10 7) 6))"), global));  
//   Token *null = tFromStr("nil");
//   Token *t1 = tFromStr("vas");
//   Token *t2 = tFromStr("102.32");
//   Token *t3 = cons(t1, cons(t2, null));
//   Token *t0 = cons(t3, cons(null, null));
//   Token *dummy = NULL;
//   printToken(t0);
//   printf("\n");
//   printToken(car(t0, dummy));
//   printf("\n");
//   printToken(cdr(cdr(car(t0, dummy), dummy), dummy));
//   printf("\n");
//   printToken(tokenize("(define fact (lambda (n) (if (<= n 1) 1 (* n (fact (- n 1))))))"));  
//   ngetchx();
//   clrscr();
//   char *ca = "a";
//   char *cb = "b";
//   char *cc = "c";  
//   Binding *b = makeNewFrame(NULL);
//   Binding *c = makeNewFrame(b);
//   addBinding(b, ca, t1);
//   addBinding(b, cc, null);
//   addBinding(c, cb, t2);
//   addBinding(c, cc, t3);
//   printToken(findBinding(b, ca)->item);
//   printf("\n");
//   printToken(findBinding(c, cb)->item);
//   printf("\n");
//   printToken(findBinding(c, cc)->item);
//   printf("\n");
//   printToken(findBinding(c, ca)->item);
//   printf("\n");
//   printBinding(b);
//   printf("\n");
//   printBinding(c);
//   printf("\n");
//   ngetchx();
//   clrscr();
//   b = makeGlobalFrame();
//   printBinding(b);
  ngetchx();
}

char *strtobuf(char *bp, char *str) {
  while (*str != '\0') {
    putchar(*(bp++) = *(str++));
  }
  return bp;
}

short getstr(char *buf) {
  char *bp = buf; // points to next empty spot on buffer
  short c;
  short result;
  while (1) {
    c = ngetchx();
    switch (c) {
    case KEY_ESC:
      return 0;      
    case 13: // Enter
      *bp = '\0';
      return 1;
    case 173: // space
      putchar(*(bp++) = ' ');      
      break;
    case 32: // minus sign
      putchar(*(bp++) = (char) 173);      
      break;
    case 257: // backspace
      *(--bp) = '\0';
      clrscr();
      printf("%s", buf);
      break;    
    case '+':
      bp = strtobuf(bp, "(+ ");
      break;
    case '-':
      bp = strtobuf(bp, "(- ");
      break;
    case '*':
      bp = strtobuf(bp, "(* ");
      break;
    case '/':
      bp = strtobuf(bp, "(/ ");
      break;
    case '|':
      bp = strtobuf(bp, "(if ");
      break;
    case '=':
      bp = strtobuf(bp, "(define ");
      break;
    case ',':
      bp = strtobuf(bp, "(lambda (");
      break;
    case 263: // clear
      bp = buf;
      clrscr();
      break;
    case 149: // EE
      bp = strtobuf(bp, "nil");
      break;
    case 277: // home
      bp = strtobuf(bp, "(car ");
      break;
    case 266: // mode
      bp = strtobuf(bp, "(cdr ");
      break;
    case 278: // catalog
      bp = strtobuf(bp, "(cons ");
      break;
    default:
      *(bp++) = (char) c;
      putchar(c);
    }
  }
}

void _main() {
  if (1) {
    char buf[MAX_INPUT];
    int cp = 0;
    char *bp;    
    Binding *global = makeGlobalFrame();
    clrscr();
    while (1) {
      if (! getstr(buf))
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
        putchar(')');
        cp--;
      }
      *bp = '\0';
      printf("\n");
      printToken(eval(tokenize(buf), global));
      printf("\n");      
    }
  } else {
    test();
  }
}
