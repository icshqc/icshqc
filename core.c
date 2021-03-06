#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "core.h"

Type* newType() {
  Type* a = malloc(sizeof(Type));
  if (a == NULL) {
    abort(); // FIXME: "Can't allocate memory"
  }
  memset(a->name, '\0', sizeof(a->name));
  a->attrs = NULL;
  a->nxt = NULL;
  return a;
}

VarType varType(PrimVarType p, int ptr, int arraySize) {
  VarType t;
  t.type = p;
  t.ptr = ptr;
  t.isConst = 0;
  t.isUnsigned = 0;
  t.arraySize = arraySize;
  t.typeStruct = NULL;
  return t;
}

VarType typeStructType(Type* type, int ptr, int arraySize) {
  VarType t = varType(STRUCT, ptr, arraySize);
  t.typeStruct = type;
  return t;
}

VarType typeStruct(char* name, int ptr, int arraySize) {
  VarType t = varType(STRUCT, ptr, arraySize);
  t.typeStruct = typeByName(types, name);
  if (t.typeStruct == NULL) {
    abort();
  }
  return t;
}

char* catPrimVarType(char* b, PrimVarType t) {
  if (t == INT) {
    strcat(b, "int");
  } else if (t == FLOAT) {
    strcat(b, "float");
  } else if (t == CHAR) {
    strcat(b, "char");
  } else if (t == VOID) {
    strcat(b, "void");
  } else if (t == STRUCT) {
    strcat(b, "STRUCT");
  } else {
    abort();
  }
  return b;
}

char* catPrimVarTypeEnum(char* b, PrimVarType t) {
  if (t == INT) {
    strcat(b, "INT");
  } else if (t == FLOAT) {
    strcat(b, "FLOAT");
  } else if (t == CHAR) {
    strcat(b, "CHAR");
  } else if (t == VOID) {
    strcat(b, "VOID");
  } else if (t == STRUCT) {
    strcat(b, "STRUCT");
  } else {
    abort();
  }
  return b;
}

char* catVarType(char* b, VarType t) {
  if (t.type == STRUCT && t.typeStruct != NULL) {
    strcat(b, t.typeStruct->name);
  } else {
    catPrimVarType(b, t.type);
  }
  int i;
  for (i = 0; i < t.ptr; i++) {
    strcat(b, "*");
  }
  return b;
}

size_t sizeofVarType(VarType t) {
  size_t s = 0;
  if (t.ptr != 0) {
    s = sizeof(void*);
  } else if (t.type == INT) {
    s = sizeof(int);
  } else if (t.type == CHAR) {
    s = sizeof(char);
  } else if (t.type == FLOAT) {
    s = sizeof(float);
  } else if (t.type == STRUCT) {
    Attr* a;
    for (a = t.typeStruct->attrs; a != NULL; a = a->nxt) {
      s += sizeofVarType(a->type);
    }
  } else {
    abort();
  }
  return t.arraySize == 0 ? s : s * t.arraySize;
}

Val* initVal(VarType t, void* addr) {
  void* nval;
  if (t.type == TUPLE) {
    nval = cpyVals((Val*)addr);
  } else {
    size_t s = sizeofVarType(t);
    nval = malloc(s);
    memcpy(nval, addr, s);
  }
  return initPtr(t, nval);
}

Val* initPtr(VarType t, void* addr) {
  Val* v = malloc(sizeof(Val));
  v->options = 0;
  v->type = t;
  v->nxt = NULL;
  v->addr = addr;
  return v;
}

// Converts tuple to struct.
Val* initStruct(VarType t, Val* tuple) {
  void* addr = malloc(sizeofVarType(t));
  void* ptr = addr;
  Val* v;
  for (v = tuple; v != NULL; v = v->nxt) {
    size_t s = sizeofVarType(v->type);
    memcpy(ptr, v->addr, s);
    ptr += s;
  }
  return initPtr(t, addr);
}

Val* cpyVal(Val* v) {
  if (v == NULL) return NULL;
  Val* v2 = initVal(v->type, v->addr);
  v2->options = v->options;
  return v2;
}
Val* cpyVals(Val* v) {
  if (v == NULL) return NULL;
  Val* n = cpyVal(v);
  n->nxt = cpyVals(v->nxt);
  return n;
}
void freeVal(Val* v) {
  if (v != NULL) {
    // Wait, that doesnt free tuples correctly maybe...
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

int isValArray(VarType t) {
  return t.arraySize == 0 ? 0 : 1;
}

int nPtr(VarType t) {
  return t.ptr + (isValArray(t) ? 1 : 0);
}

int canCast(VarType casted, VarType castee) {
  // FIXME: This does not work
  if (casted.type == castee.type) return 1;
  return sizeofVarType(casted) <= sizeofVarType(castee) ? 1 : 0;
}

int sameCastType(VarType t1, VarType t2) {
  return t1.typeStruct == t2.typeStruct && canCast(t1, t2) && nPtr(t1) == nPtr(t2) ? 1 : 0;
}

Val* checkSignatureAttrs(Val* args, Attr* attrs) {
  Attr* a;
  Val* v;
  int i;
  for (i = 0, v = args, a = attrs; a != NULL; i++, v = v->nxt, a = a->nxt) {
    if (v == NULL) {
      return errorStr("Missing arg.");
    } else if (sameCastType(v->type, a->type) == 0) {
      char m[52] = "";
      sprintf(m, "Invalid arg %d: Expected type ", i);
      catVarType(m, a->type);
      return errorStr(m);
    }
  }
  if (v != NULL) {
    return errorStr("Supplied too many args.");
  }
  return NULL;
}

Val* errorStr(char* str) {
  Val* v = initVal(varType(CHAR, 0, 124), str);
  v->options = VAL_ERROR;
  return v;
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

Attr* copyAttrs(Attr* attrs) {
  if (attrs == NULL) return NULL;
  Attr* a = newAttr();
  strcpy(a->name, attrs->name);
  a->type = attrs->type;
  a->nxt = copyAttrs(attrs->nxt);
  return a;
}

Val* construct(Val* args) {
  char fullname[64] = "";
  strcat(fullname, "struct ");
  strcat(fullname, (char*)args->addr + 1);
  Type* t = typeByName(types, fullname);
  if (t == NULL) {
    return errorStr("Unkown constructor.");
  }
  Val* err = checkSignatureAttrs(args->nxt, t->attrs);
  if (err != NULL) return err;

  return initStruct(typeStructType(t, 0, 0), args->nxt);
}

void typeConstructor(Type* type) {
  if (type->attrs != NULL) {
    char constructorName[52] = "#";
    if (startsWith("struct ", type->name)) {
      strcat(constructorName, type->name+7);
    } else {
      strcat(constructorName, type->name);
    }
    Attr* attrs = copyAttrs(type->attrs);
    addLoadedDef(loadedDefs, createFunc(constructorName, attrs), FUNCTION, construct);
  }
}

void addType(Type* type) {
  Type* oldFirst = types;
  types = type;
  type->nxt = oldFirst;
  typeConstructor(type);
}

Cmd* initCmd(CmdType type, const char* val, Cmd* args) {
  Cmd* c = newCmd();
  if (val != NULL) strcpy(c->name, val);
  c->type = type;
  c->args = args;
  return c;
}

LoadedDef* addLoadedDef(LoadedDef* p, Func* fc, CmdType type, Val* (*ptr)(Val* cmd)) {
  LoadedDef* f = createLoadedDef(fc, type, ptr);
  lastLoadedDef(p)->nxt = f;
  return p;
}

LoadedDef* lastLoadedDef(LoadedDef* d) {
  if (d == NULL) return NULL;
  
  LoadedDef* f;
  for (f = d; f->nxt != NULL; f = f->nxt) {}
  return f;
}

LoadedDef* createLoadedDef(Func* f, CmdType type, Val* (*ptr)(Val* cmd)) {
  LoadedDef* d = malloc(sizeof(LoadedDef));
  d->func = f;
  d->type = type;
  d->ptr = ptr;
  d->nxt = NULL;
  return d;
}

Attr* newAttr() {
  Attr* a = malloc(sizeof(Attr));
  if (a == NULL) {
    abort(); // FIXME: "Can't allocate memory"
  }
  a->type.type = 0;
  memset(a->name, '\0', sizeof(a->name));
  a->nxt = NULL;
  return a;
}

Func* newFunc() {
  Func* f = malloc(sizeof(Func));
  if (f == NULL) {
    abort(); // FIXME: "Can't allocate memory"
  }
  memset(f->name, '\0', sizeof(f->name));
  f->cmd = NULL;
  f->args = NULL;
  f->isOperator = 0;
  f->nxt = NULL;
  return f;
}

Attr* createAttr(char* name, VarType t, Attr* nxt) {
  Attr* a = newAttr();
  strcpy(a->name, name);
  a->type = t;
  a->nxt = nxt;
  return a;
}

Func* createFunc(char* name, Attr* attrs) {
  Func* f = newFunc();
  strcpy(f->name, name);
  f->args = attrs;
  return f;
}

Type* typeByName(Type* types, char* name) {
  Type* t = types;
  while (t != NULL) {
    if (strcmp(t->name, name) == 0) {
      return t;
    }
    t = t->nxt;
  }
  return NULL;
}

Type* createType(char* name, Attr* attrs) {
  Type* type = newType();
  strcpy(type->name, name);
  type->attrs = attrs;
  return type;
}

