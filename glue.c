#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <string.h>

#include "glue.h"

char* argstring(Cmd** cmd) {
  Cmd* c = *cmd;
  if (c == NULL) return NULL;
  *cmd = c->nxt;
  return c->name;
}
char argchar(Cmd** cmd) {
  Cmd* c = *cmd;
  if (c == NULL) return '\0';
  *cmd = c->nxt;
  //if (strlen(c->name) != 1) { error }
  return c->name[0];
}
int argint(Cmd** cmd) {
  Cmd* c = *cmd;
  if (c == NULL) return -1;
  *cmd = c->nxt;
  //if (strlen(c->name) != 1) { error }
  return 1;//c->name[0]; // TODO: string to integer
}

LoadedDef* addLoadedDef(LoadedDef* p, char* name, int priority, void (*ptr)(Cmd* cmd)) {
  LoadedDef* f = createLoadedDef(name, priority, ptr);
  lastLoadedDef(p)->nxt = f;
}

LoadedDef* lastLoadedDef(LoadedDef* d) {
  if (d == NULL) {
    return NULL;
  } else {
    LoadedDef* f;
    for (f = d; f->nxt != NULL; f = f->nxt) {}
    return f;
  }
}

LoadedDef* createLoadedDef(char* name, int isOp, void (*ptr)(Cmd* cmd)) {
  LoadedDef* d = malloc(sizeof(LoadedDef));
  strcpy(d->name, name);
  d->isOperator = isOp;
  d->ptr = ptr;
  d->nxt = NULL;
  return d;
}

