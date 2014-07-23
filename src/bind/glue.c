#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "glue.h"

VarType varType(PrimVarType p, int ptr, int arraySize) {
  VarType t;
  t.type = p;
  t.ptr = ptr;
  t.arraySize = arraySize;
  return t;
}

Val* initVal(VarType t, void* addr) {
  Val* v = malloc(sizeof(Val));
  v->type = t;
  if (t.type == ERR) {
    char* err = malloc(sizeof(char) * 52);
    strcpy(err, (char*)addr);
    v->addr = err;
  } else if (t.type == INT) {
    int* vx = malloc(sizeof(int));
    *vx = *(int*)addr;
    v->addr = vx;
  } else if (t.type == CHAR && t.ptr == 1) {
    char* m = malloc(sizeof(char) * 52);
    strcpy(m, (char*)addr);
    v->addr = m;
  }
  v->nxt = NULL;
  return v;
}
void freeVal(Val* v) {
  if (v != NULL) {
    freeVal(v->nxt);
    if (v->addr != NULL) {
      free(v->addr);
    }
    free(v);
  }
}

Cmd* newCmd() {
  Cmd* arg0 = malloc(sizeof(Cmd));
  if (arg0 == NULL) {
    abort(); // FIXME: "Can't allocate memory"
  }
  memset(arg0->name, '\0', sizeof(arg0->name));
  arg0->type = UNKOWN;
  arg0->valueType = NULL;
  arg0->nxt = NULL;
  arg0->args = NULL;
  return arg0;
}

Cmd* nxtCmd(Cmd** cmd) {
  *cmd = (*cmd)->nxt;
  return *cmd;
}

Val* checkSignature(Cmd* args, CmdType* types, int nArgs) {
  CmdType* t;
  Cmd* c;
  int i;
  for (i = 0, c = args, t = types; i < nArgs; i++, c = c->nxt, t++) {
    if (c == NULL) {
      return errorStr("Missing arg.");
    } else if (c == NULL || c->type != *t) {
      return errorStr("Invalid arg i: Expected type ...");
    }
  }
  if (c != NULL) {
    return errorStr("Supplied too many args.");
  }
  return NULL;
}

Val* errorStr(char* str) {
  return initVal(varType(ERR, 1, 0), str);
}

int validArg(Cmd* cmd, CmdType type) {
  if (cmd == NULL || cmd->type != type) return 0;
  return 1;
}

char* cat_argptr(char* b, char* s) {
  return b;
}

char* cat_argstring(char* b, char* s) {
  strcat(b, s);
  return b;
}
char* cat_argchar(char* b, char s) {
  *(b + strlen(b)) = s;
  return b;
}
char* cat_argint(char* b, int s) {
  char buffer[52] = "";
  sprintf(buffer, "%d", s);
  strcat(b, buffer);
  return b;
}

char* argstring(Cmd* cmd) {
  return cmd->name;
}
char argchar(Cmd* cmd) {
  return cmd->name[0];
}
int argint(Cmd* cmd) {
  char* num; // ???
  return strtol(cmd->name, &num, 0);
}
void* argptr(Cmd* cmd) {
  return NULL;
}

Cmd* initCmd(CmdType type, const char* val, Cmd* args) {
  Cmd* c = newCmd();
  if (val != NULL) strcpy(c->name, val);
  c->type = type;
  c->args = args;
  return c;
}

LoadedDef* addLoadedDef(LoadedDef* p, char* name, CmdType type, Val* (*ptr)(Cmd* cmd)) {
  LoadedDef* f = createLoadedDef(name, type, ptr);
  lastLoadedDef(p)->nxt = f;
  return p;
}

LoadedDef* lastLoadedDef(LoadedDef* d) {
  if (d == NULL) return NULL;
  
  LoadedDef* f;
  for (f = d; f->nxt != NULL; f = f->nxt) {}
  return f;
}

LoadedDef* createLoadedDef(char* name, CmdType type, Val* (*ptr)(Cmd* cmd)) {
  LoadedDef* d = malloc(sizeof(LoadedDef));
  strcpy(d->name, name);
  d->type = type;
  d->ptr = ptr;
  d->nxt = NULL;
  return d;
}

