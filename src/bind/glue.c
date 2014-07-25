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

size_t sizeofVarType(VarType t) {
  size_t s;
  if (t.ptr != 0) {
    s = sizeof(void*);
  } else if (t.type == INT) {
    s = sizeof(int);
  } else if (t.type == CHAR) {
    s = sizeof(char);
  } else if (t.type == ERR) {
    s = sizeof(char);
  } else if (t.type == FLOAT) {
    s = sizeof(float);
  }
  return t.arraySize == 0 ? s : s * t.arraySize;
}

Val* initArray(VarType t, void* addr) {
  Val* v = malloc(sizeof(Val));
  v->type = t;
  v->nxt = NULL;
  size_t s = sizeofVarType(t);
  v->addr = malloc(s);
  memcpy(v->addr, addr, s);
  return v;
}

Val* initPtr(VarType t, void* addr) {
  Val* v = malloc(sizeof(Val));
  v->type = t;
  v->nxt = NULL;
  v->addr = addr;
  return v;
}
Val* initVal(VarType t, void* addr) {
  Val* v = malloc(sizeof(Val));
  v->type = t;
  v->nxt = NULL;
  if (t.ptr == 1 || t.type == ERR) {
    abort(); // Should be using initPtr
  } else {
    if (t.type == INT) {
      int* vx = malloc(sizeof(int));
      *vx = *(int*)addr;
      v->addr = vx;
    } else if (t.type == CHAR) {
      char* vx = malloc(sizeof(char));
      *vx = *(char*)addr;
      v->addr = vx;
    } else if (t.type == TUPLE) {
      v->addr = cpyVals((Val*)addr);
    } else {
      abort(); // FIXME: Better error message.
    }
  }
  return v;
}
Val* cpyVal(Val* v) {
  if (v == NULL) return NULL;
  if (v->type.arraySize != 0) {
    return initArray(v->type, v->addr);
  } else {
    return initVal(v->type, v->addr);
  }
}
Val* cpyVals(Val* v) {
  if (v == NULL) return NULL;
  Val* n = cpyVal(v);
  n->nxt = cpyVals(v->nxt);
  return n;
}
void freeVal(Val* v) {
  if (v != NULL) {
    freeVal(v->nxt);
    if (v->addr != NULL && v->type.ptr == 0) {
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

Val* checkSignature(Val* args, VarType* types, int nArgs) {
  VarType* t;
  Val* v;
  int i;
  for (i = 0, v = args->nxt, t = types; i < nArgs; i++, v = v->nxt, t++) {
    if (v == NULL) {
      return errorStr("Missing arg.");
    } else if (v == NULL || (v->type.type != (*t).type || v->type.ptr != (*t).ptr || v->type.arraySize != (*t).arraySize)) {
      return errorStr("Invalid arg i: Expected type ...");
    }
  }
  if (v != NULL) {
    return errorStr("Supplied too many args.");
  }
  return NULL;
}

Val* errorStr(char* str) {
  return initArray(varType(ERR, 0, 124), str);
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

LoadedDef* addLoadedDef(LoadedDef* p, char* name, CmdType type, Val* (*ptr)(Val* cmd)) {
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

LoadedDef* createLoadedDef(char* name, CmdType type, Val* (*ptr)(Val* cmd)) {
  LoadedDef* d = malloc(sizeof(LoadedDef));
  strcpy(d->name, name);
  d->type = type;
  d->ptr = ptr;
  d->nxt = NULL;
  return d;
}

