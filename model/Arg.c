#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Arg.h"

void freeArg(Arg* arg) {
  if (arg != NULL) {
    freeArg(arg->nxt);
    free(arg);
  }
}

Arg* newArg() {
  Arg* arg0 = malloc(sizeof(Arg));
  if (arg0 == NULL) {
    abort(); // FIXME: "Can't allocate memory"
  }
  memset(arg0->val, '\0', sizeof(arg0->val));
  arg0->nxt = NULL;
  return arg0;
}

char* catArg(char* m, Arg* arg) {
  if (arg != NULL) {
    Arg* n = arg->nxt;
    strcat(m, "Arg[\"");
    strcat(m, arg->val);
    while (n != NULL) {
      strcat(m, "\", \"");
      strcat(m, n->val);
      n = n->nxt;
    }
    strcat(m, "\"]");
  }
  return m;
}

