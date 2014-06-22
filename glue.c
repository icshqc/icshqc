#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <string.h>

#include "model.h"

char* argstring(Cmd** cmd) {
  Cmd* c = *cmd;
  if (c == NULL) return NULL;
  *cmd = c->nxt;
  return c->name;
}
char argchar(Cmd** cmd) {
  Cmd* c = *cmd;
  if (c == NULL) return '\0';;
  *cmd = c->nxt;
  //if (strlen(c->name) != 1) { error }
  return c->name[0];
}
