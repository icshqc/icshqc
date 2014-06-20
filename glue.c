#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <string.h>

#include "model.h"

char* argStr(Cmd** cmd) {
  Cmd* c = *cmd;
  if (c == NULL) {
    return NULL;
  }
  *cmd = c->nxt;
  return c->name;
}

void addstrBind(Cmd* cmd) {
  Cmd* args = cmd->args;
  char* arg0 = argStr(&args);
  addstr(arg0);
}
