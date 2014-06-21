#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <string.h>

#include "model.h"

char* argstring(Cmd** cmd) {
  Cmd* c = *cmd;
  if (c == NULL) {
    return NULL;
  }
  *cmd = c->nxt;
  return c->name;
}
char argchar(Cmd** cmd) {
  return 'a';
}
