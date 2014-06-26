#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <string.h>

#include "glue.h"

Cmd* newCmd() {
  Cmd* arg0 = malloc(sizeof(Cmd));
  if (arg0 == NULL) {
    abort(); // FIXME: "Can't allocate memory"
  }
  memset(arg0->name, '\0', sizeof(arg0->name));
  arg0->type = UNKOWN;
  arg0->nxt = NULL;
  arg0->args = NULL;
  arg0->body = NULL;
  return arg0;
}

char* cat_argstring(char* b, char* s) {
  strcat(b, s);
}
char* cat_argchar(char* b, char s) {
  *(b + strlen(b)) = s;
}
char* cat_argint(char* b, int s) {
  char buffer[52] = "";
  sprintf(buffer, "%d", s);
  strcat(b, buffer);
}

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
  char* num; // ???
  return strtol(&c->name[0], &num, 0); // TODO: string to integer
}

Cmd* retCmd(CmdType type, char* name) {
  Cmd* cmd = newCmd();
  cmd->type = type;
  strcpy(cmd->name, name);
  return cmd;
}

LoadedDef* addLoadedDef(LoadedDef* p, char* name, int priority, Cmd* (*ptr)(Cmd* cmd)) {
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

LoadedDef* createLoadedDef(char* name, int isOp, Cmd* (*ptr)(Cmd* cmd)) {
  LoadedDef* d = malloc(sizeof(LoadedDef));
  strcpy(d->name, name);
  d->isOperator = isOp;
  d->ptr = ptr;
  d->nxt = NULL;
  return d;
}

