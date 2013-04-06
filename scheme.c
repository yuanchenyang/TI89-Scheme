#define SAVE_SCREEN         // this directive forces saving/restoring the screen
#define USE_TI89
#define OPTIMIZE_ROM_CALLS

#define MAXINPUT 100
#define ESC 8

#include <stdio.h>
#include <kbd.h>
#include <string.h>

_main() {
  char buf[MAXINPUT];  
  clrscr();
  while (1) {
    getsn(buf, MAXINPUT);
    if (buf[0] == ESC)
      break;    
    printf("\n%s\n", buf);
  }
}
