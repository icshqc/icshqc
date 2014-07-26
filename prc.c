#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

#include "model.h"
#include "src/bind/glue.h"

#include "src/lib.h"

static Type* types = NULL;
// Maybe vars by scope.
static Var* vars = NULL;
static Func* funcs = NULL;

Attr* newAttr() {
  Attr* a = malloc(sizeof(Attr));
  if (a == NULL) {
    abort(); // FIXME: "Can't allocate memory"
  }
  a->type.type = UNDEFINED;
  memset(a->name, '\0', sizeof(a->name));
  a->nxt = NULL;
  return a;
}

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

CFunc* newCFunc() {
  CFunc* arg0 = malloc(sizeof(CFunc));
  if (arg0 == NULL) {
    abort(); // FIXME: "Can't allocate memory"
  }
  memset(arg0->name, '\0', sizeof(arg0->name));
  arg0->ret.type = UNDEFINED;
  arg0->args = NULL;
  arg0->nxt = NULL;
  return arg0;
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

void freeVar(Var* t) {
  if (t != NULL) {
    freeVar(t->nxt);
    free(t);
  }
}

void freeAttr(Attr* a) {
  if (a != NULL) {
    freeAttr(a->nxt);
    free(a);
  }
}

void freeType(Type* t) {
  if (t != NULL) {
    freeAttr(t->attrs);
    free(t);
  }
}
void freeTypes(Type* t) {
  if (t != NULL) {
    freeType(t);
    freeTypes(t->nxt);
  }
}

void freeLoadedDef(LoadedDef* d) {
  if (d != NULL) {
    freeLoadedDef(d->nxt);
    free(d);
  }
}

void freeCFunc(CFunc* f) {
  if (f != NULL) {
    freeAttr(f->args);
    freeCFunc(f->nxt);
    free(f);
  }
}

void freeCmd(Cmd* arg) {
  if (arg != NULL) {
    freeCmd(arg->nxt);
    freeCmd(arg->args);
  }
}

void freeFunc(Func* f) {
  if (f != NULL) {
    freeVal(f->cmd);
    freeAttr(f->args);
    freeFunc(f->nxt);
    free(f);
  }
}

static void finish(int sig);

char* catCmdType(char* b, CmdType t) {
  if (t == VALUE) {
    strcat(b, "VALUE");
  } else if (t == OLD_TUPLE) {
    strcat(b, "OLD_TUPLE");
  } else if (t == FUNCTION) {
    strcat(b, "FUNCTION");
  } else if (t == NIL) {
    strcat(b, "NIL");
  } else if (t == PAIR) {
    strcat(b, "PAIR");
  } else if (t == VAR_NAME) {
    strcat(b, "VAR_NAME");
  } else if (t == TYPE) {
    strcat(b, "TYPE");
  } else if (t == BLOCK) {
    strcat(b, "BLOCK");
  } else if (t == CFUNCTION) {
    strcat(b, "CFUNCTION");
  } else if (t == VAR) {
    strcat(b, "VAR");
  } else if (t == MACRO) {
    strcat(b, "MACRO");
  } else if (t == MACRO_OP) {
    strcat(b, "MACRO_OP");
  } else if (t == OPERATOR) {
    strcat(b, "OPERATOR");
  } else if (t == UNKOWN) {
    strcat(b, "UNKOWN");
  } else if (t == STRING) {
    strcat(b, "STRING");
  } else if (t == ERROR) {
    strcat(b, "ERROR");
  } else if (t == ARRAY) {
    strcat(b, "ARRAY");
  } else if (t == POINTER) {
    strcat(b, "POINTER");
  } else {
    strcat(b, "FIXME_UNKOWN_TYPE");
  }
  return b;
}

char* catPrintCmd(char* b, Cmd* cmd) {
  if (cmd == NULL) return b;
  Cmd* n;
  if (cmd->type == ARRAY || cmd->type == OLD_TUPLE) {
    strcat(b, cmd->type == ARRAY ? "[" : "(");
    for (n = cmd->args; n != NULL; n = n->nxt) {
      catPrintCmd(b, n);
      if (n->nxt != NULL) {
        strcat(b, "  ---  ");
      }
    }
    strcat(b, cmd->type == ARRAY ? "]" : ")");
  } else if (cmd->valueType != NULL) {
    Type* t = cmd->valueType;
    if (t->attrs == NULL) {
      strcat(b, cmd->args->name);
    } else {
      strcat(b, "#");
      strcat(b, t->name);
      strcat(b, "{");
      Attr* a;
      for (a = t->attrs, n = cmd->args; n != NULL; n = n->nxt, a = a->nxt) {
        strcat(b, a->name);
        strcat(b, ": ");
        catPrintCmd(b, n);
        if (n->nxt != NULL) {
          strcat(b, ", ");
        }
      }
      strcat(b, "}");
    }
  } else {
    strcat(b, cmd->name);
  }
  return b;
}

char* catCmd(char* b, Cmd* cmd) {
  if (cmd != NULL) {
    catCmdType(b, cmd->type);
    strcat(b, " ");
    strcat(b,cmd->name);
    if (cmd->args != NULL) {
      strcat(b,"(");
      Cmd* a;
      for (a = cmd->args; a != NULL; a = a->nxt) {
        catCmd(b, a);
        if (a->nxt != NULL) {
          strcat(b, ", ");
        }
      }
      strcat(b,")");
    }
  }
  return b;
}

char* catVal(char* b, Val* v) {
  if (v == NULL) {
    strcat(b, "NULL");
  } else if (v->type.type == ERR || (v->type.type == CHAR && (v->type.ptr == 1 || v->type.arraySize != 0))) {
    strcat(b, (char*)v->addr);
  } else if (v->type.type == TUPLE) {
    Val* v2;
    strcat(b, "(");
    for (v2 = (Val*)v->addr; v2 != NULL; v2 = v2->nxt) {
      catVal(b, v2);
      if (v2->nxt != NULL) {
        strcat(b, ", ");
      }
    }
    strcat(b, ")");
  } else if (v->type.type == INT) {
    char buffer[52] = "";
    sprintf(buffer, "%d", *((int*)v->addr));
    strcat(b, buffer);
  } else {
    abort();
  }
  return b;
}

void catVar(char* m, Var* v) {
  if (v != NULL) {
    if (v->type != NULL) {
      strcat(m, v->type->name);
      strcat(m, " ");
    }
    strcat(m, v->name);
    if (v->val != NULL) {
      strcat(m, " ");
      catVal(m, v->val);
    }
  }
}

Func* funcByName(char* name) {
  Func* f = funcs;
  while (f != NULL) {
    if (strcmp(f->name, name) == 0) {
      return f;
    }
    f = f->nxt;
  }
  return NULL;
}

Var* varByName(char* name) {
  Var* t = vars;
  while (t != NULL) {
    if (strcmp(t->name, name) == 0) {
      return t;
    }
    t = t->nxt;
  }
  return NULL;
}

Type* typeByName(char* name) {
  Type* t = types;
  while (t != NULL) {
    if (strcmp(t->name, name) == 0) {
      return t;
    }
    t = t->nxt;
  }
  return NULL;
}

VarType parseVarType(char* str) { 
  char* s = str;
  while (*s != '*' && *s != '\0') {
    ++s;
  }
  char typeName[52] = "";
  strncpy(typeName, str, s - str);
  int ptr = 0;
  while (*s != '\0') {
    if (*s == '*') {
      ++ptr;
    }
    ++s;
  }
  VarType v;
  if (strcmp(typeName, "int") == 0) {
    v.type = INT;
  } else if (strcmp(typeName, "char") == 0) {
    v.type = CHAR;
  } else if (strcmp(typeName, "float") == 0) {
    v.type = FLOAT;
  } else if (strcmp(typeName, "void") == 0) {
    v.type = VOID;
  } else {
    v.type = UNDEFINED;
  }
  v.ptr = ptr;
  v.arraySize = 0;
  return v;
}

char* catPrimVarTypeEnum(char* b, PrimVarType t) {
  if (t == INT) {
    strcat(b, "INT");
  } else if (t == FLOAT) {
    strcat(b, "FLOAT");
  } else if (t == CHAR) {
    strcat(b, "CHAR");
  }
  return b;
}

Cmd* typeCmd(Cmd* cmd) {
  Cmd* c;
  for (c = cmd; c != NULL; c = c->nxt) {
    typeCmd(c->args);
    if (c->type == UNKOWN) {
      if (strlen(c->name) <= 0) {
        c->type = NIL;
      } else {
        char* n = c->name;
        Func* f = funcByName(n);
        Var* v;
        if (f != NULL) {
          c->type = f->type;
        } else if ((v = varByName(n)) != NULL) {
          c->type = VAR;
        } else if (strlen(n) == 3 && n[0] == '\'' && n[2] == '\'') { // FIXME: Does not work '\0'
          c->type = OLD_CHAR;
        } else if (strlen(n) >= 2 && n[0] == '\"' && n[strlen(n)-1] == '\"') {
          c->type = STRING;
        } else if (isInteger(n)) {
          c->type = OLD_INT;
        } else if (isFloat(n)) {
          c->type = OLD_FLOAT;
        } else {
          // TODO: Check for int.
          c->type = UNKOWN;
        }
      }
    }
  }
  return cmd;
}

void addCmd(Cmd** list, Cmd* cmd) {
  if (*list == NULL) {
    *list = cmd;
  } else {
    Cmd* l;
    for (l = *list; l->nxt != NULL; l = l->nxt) {}
    l->nxt = cmd;
  }
}

struct ParsePair {
  Cmd* cmd;
  char* ptr;
};
typedef struct ParsePair ParsePair;
ParsePair parsePair(Cmd* cmd, char* ptr) {
  ParsePair p;
  p.cmd = cmd;
  p.ptr = ptr;
  return p;
}
static char ALLOWED_NAME_CHARS[] = "abcdefghijklmnopqrstuvwxyz_0123456789";
ParsePair parse(char* command, char* allowedChars) {
  Cmd* cmd = newCmd();
  char* c;
  char* ch;
  for (c = trim(command); *c != '\0'; ++c) {
    int found = 0;
    for (ch = allowedChars; *ch != '\0'; ++ch) {
      if (*ch == *c) {
        found = 1;
      }
    }
    if (found == 0) {
      return parsePair(cmd, c);
    } else {
      straddch(cmd->name, *c);
    }
  }
  return parsePair(cmd, trim(c));
}
ParsePair parseBlockAttr(char* command) {
  Cmd* cmd = initCmd(PAIR, NULL, NULL);
  ParsePair p = parse(trim(command), ALLOWED_NAME_CHARS);
  if (*(p.ptr) != ' ') {
    //msg("Error parsing block. Missing arg type.");
    freeCmd(cmd);
    return parsePair(NULL, p.ptr);
  }
  cmd->args = p.cmd;
  cmd->args->type = TYPE;
  p = parse(trim(p.ptr), ALLOWED_NAME_CHARS);
  if (*(p.ptr) == '|' || *(p.ptr) == ',') {
    cmd->args->nxt = p.cmd;
    cmd->args->nxt->type = VAR_NAME;
    if (*(p.ptr) == ',') {
      p = parseBlockAttr(p.ptr+1);
      cmd->nxt = p.cmd;
      return parsePair(cmd, p.ptr);
    } else {
      return parsePair(cmd, p.ptr+1);
    }
  } else {
    //msg("Error parsing block. Missing arg name.");
    freeCmd(cmd);
    return parsePair(NULL, p.ptr);
  }
}
ParsePair parseCmdR(char* command);
ParsePair parseBlock(char* command) {
  char* s = trim(command);
  Cmd* block = initCmd(BLOCK, NULL, NULL);
  ParsePair p;
  if (*s == '|') {
    p = parseBlockAttr(++s);
    block->args = p.cmd;
    s = p.ptr;
  }
  Cmd* arg;
  for (arg = block->args; arg->nxt != NULL; arg = arg->nxt) {}
  p = parseCmdR(trim(s));
  arg->nxt = p.cmd;
  s = p.ptr;
  if (*s != '}') {
    //msg("Error parsing block. Missing end bracket.");
    freeCmd(block);
    return parsePair(NULL, s);
  }
  return parsePair(block, s+1);
}
ParsePair parseArray(char* command) {
  Cmd* ary = initCmd(ARRAY, NULL, NULL);
  Cmd* elem = newCmd();
  ary->args = elem;
  char *s = command;
  while (*s != ']') {
    if (*s == '\0') {
      freeCmd(ary);
      return parsePair(NULL, s);
    } else if (*s == ',') {
      elem->nxt = newCmd();
      elem = elem->nxt;
    } else if(*s != ' ' || strlen(elem->name) > 0) {
      straddch(elem->name, *s);
    }
    ++s;
  }
  return parsePair(ary, s+1);
}
ParsePair parseCmdR(char* command) {
  Cmd* cmds = NULL;
  char* s = trim(command);
  while (*s != '\0') {
    char* i = s;
    while (*s != '\0' && *s != ' ' && *s != '(' && *s != ')' &&
           *s != '{' && *s != '}' && *s != '"' && *s != '[' && *s != '\n') {
      ++s;
    }
    if (s > i) {
      Cmd* cmd = newCmd();
      strncpy(cmd->name, i, s-i);
      addCmd(&cmds, cmd);
    }
    s = trim(s);
    if (*s == '"') {
      char p = *s;
      char* i2 = s;
      while (*(++s) != '\0') {
        if (*s == '"' && p != '\\') {
          break;
        }
        p = *s;
      }
      if (*s != '"') {
        //msg("Error parsing string.");
        freeCmd(cmds);
        return parsePair(NULL, s);
      } else {
        Cmd* cmd = newCmd();
        cmd->type = STRING;
        strncpy(cmd->name, i2 + 1, s-i2-1);
        addCmd(&cmds, cmd);
        ++s;
      }
    } else if (*s == ')') {
      ++s;
      break;
    } else if (*s == '[') {
      ParsePair p = parseArray(s+1);
      s = p.ptr;
      if (p.cmd != NULL) {
        addCmd(&cmds, p.cmd);
      }
    } else if (*s == '}') {
      break;
    } else if (*s == '{') {
      ParsePair p = parseBlock(s+1);
      s = p.ptr;
      if (p.cmd != NULL) {
        addCmd(&cmds, p.cmd);
      }
    } else if (*s == '(') {
      ParsePair p = parseCmdR(s+1);
      s = p.ptr;
      addCmd(&cmds, p.cmd);
    }
  }
  if (cmds == NULL) return parsePair(cmds, s);
  typeCmd(cmds);
  if (cmds->nxt != NULL && (cmds->nxt->type == OPERATOR || cmds->nxt->type == MACRO_OP)) {
    Cmd* cmd = cmds;
    cmds = cmds->nxt;
    cmds->args = cmd;
    cmd->nxt = cmds->nxt;
    cmds->nxt = NULL;
  } else if (cmds->type == FUNCTION || cmds->type == MACRO || cmds->type == CFUNCTION) {
    cmds->args = cmds->nxt;
    cmds->nxt = NULL;
  } else {
    if (cmds->nxt != NULL) {
      return parsePair(initCmd(OLD_TUPLE, NULL, cmds), s);
    } else {
      return parsePair(cmds, s);
    }
  }
  return parsePair(cmds, s);
}

Val* cmdToVal(Cmd* cmd);
Val* cmdArgsToVal(Cmd* cmd) {
  Val* v0 = NULL;
  Val* v = NULL;
  Cmd* c;
  for (c = cmd->args; c != NULL; c = c->nxt) {
    if (v == NULL) {
      v0 = cmdToVal(c);
      v = v0;
    } else {
      v->nxt = cmdToVal(c);
      v = v->nxt;
    }
  }
  return initVal(varType(TUPLE, 0, 0), v0);
}

Val* cmdToVal(Cmd* cmd) {
  if (cmd == NULL) return NULL;
  if (cmd->type == OLD_INT) {
    int x = argint(cmd);
    return initVal(varType(INT, 0, 0), &x);
  } else if (cmd->type == STRING) {
    char name[52] = "";
    strncpy(name, cmd->name, strlen(cmd->name));
    return initArray(varType(CHAR, 0, 52), name);
  } else if (cmd->type == OLD_CHAR) {
    char c = cmd->name[1];
    return initVal(varType(CHAR, 0, 0), &c);
  } else if (cmd->args != NULL) {
    if (cmd->name != NULL && strlen(cmd->name) > 0) {
      Val* v = initArray(varType(CHAR, 0, 52), cmd->name);
      v->nxt = cmdArgsToVal(cmd);
      return initVal(varType(TUPLE, 0, 0), v);
    } else {
      return cmdArgsToVal(cmd);
    }
  } else {
    return initArray(varType(CHAR, 0, 52), cmd->name);
  }
}

void addVal(Val** list, Val* v) {
  while (*list != NULL) {
    ++list;
  }
  *list = v;
}

// Reduces a Cmd down to a primitive type.
// If cmd->type == CFUNCTION, call it and replace the cmd by it's result
// If cmd->type == FUNCTION, call it and replace the cmd by it's result
// If cmd->type == VAR, get it's value and replace the cmd by it's result
Val* cmdVal(Cmd* cmd, Val** garbage) {
  Val* ret = NULL;
  Val* nVal = NULL;
  if (cmd->type == FUNCTION || cmd->type == OPERATOR || cmd->type == CFUNCTION) {
    nVal = initArray(varType(CHAR, 0, 52), cmd->name);
    Val* n = nVal;
    Cmd* arg;
    for (arg = cmd->args; arg != NULL; arg = arg->nxt) {
      n->nxt = cmdVal(arg, garbage);
      if (n->nxt != NULL) {
        n = n->nxt;
      }
    }
    ret = loadedFuncByName(cmd->name)->ptr(nVal);
    addVal(garbage, ret);
  } else if (cmd->type == MACRO || cmd->type == MACRO_OP) {
    nVal = initArray(varType(CHAR, 0, 52), cmd->name);
    Val* n = nVal;
    Cmd* arg;
    for (arg = cmd->args; arg != NULL; arg = arg->nxt) {
      n->nxt = cmdToVal(arg);
      if (n->nxt != NULL) {
        n = n->nxt;
      }
    }
    ret = loadedFuncByName(cmd->name)->ptr(nVal);
    addVal(garbage, ret);
  } else if (cmd->type == VAR) {
    ret = varByName(cmd->name)->val;
  } else {
    ret = cmdToVal(cmd);
    addVal(garbage, ret);
  }
  return ret;
}

void escapeName(char* str) {
  char buf[128] = "";
  int i;
  char *c;
  for (c = str, i = 0; *c != '\0'; ++c, ++i) {
    if (*c == '+') { // TODO: Add all chars that need to be escaped.
      strcat(buf, "__plus__");
      i = strlen(buf) - 1;
    } else if (*c == '-') { // TODO: Add all chars that need to be escaped.
      strcat(buf, "__minus__");
      i = strlen(buf) - 1;
    } else if (*c == '*') { // TODO: Add all chars that need to be escaped.
      strcat(buf, "__mult__");
      i = strlen(buf) - 1;
    } else if (*c == '/') { // TODO: Add all chars that need to be escaped.
      strcat(buf, "__div__");
      i = strlen(buf) - 1;
    } else {
      buf[i] = *c;
    }
  }
  strcpy(str, buf);
}

char* catCmdExe(char* b, Cmd* cmd, int nested) {
  if (cmd != NULL) {
    if (cmd->args == NULL) {
      strcat(b, cmd->name);
    } else {
      if (nested) strcat(b, "(");
      strcat(b, cmd->name);
      Cmd* a;
      for (a = cmd->args; a != NULL; a = a->nxt) {
        strcat(b, " ");
        catCmdExe(b, a, 1);
      }
      if (nested) strcat(b, ")");
    }
  }
  return b;
}

void save() { // .qc extension. Quick C, Quebec!!!
  FILE* s = fopen("app.qc", "w"); // FIXME: Check if valid file. Not NULL.
  Func* f;
  for (f = funcs; f != NULL; f = f->nxt) {
    fprintf(s, "%s %s {", f->name, f->isOperator ? ":::" : "::");
    if (f->args != NULL) {
      fprintf(s, "|");
      Attr* a;
      for (a = f->args; a != NULL; a = a->nxt) {
        char aType[52] = "";
        catVarType(aType, a->type);
        fprintf(s, "%s %s", aType, a->name);
        if (a->nxt != NULL) {
          fprintf(s, ", ");
        }
      }
      fprintf(s, "| ");
    }
    char cmdExe[256] = "";
    //catCmdExe(cmdExe, f->cmd, 0); FIXME
    fprintf(s, "%s }\n", cmdExe);
  }
  fclose(s);
}

Val* construct(Val* args) {
  Type* t = typeByName((char*)args->addr + 1);
  if (t == NULL) {
    return errorStr("Unkown constructor.");
  }
  /*Cmd* ret = initCmd(VALUE, NULL, NULL);
  ret->valueType = t;
  if (t->attrs != NULL) {
    addCmd(&ret->args, cpyCmd(cmd->args));
  }
  return cmdToVal(ret); FIXME*/
  return NULL;
}

void addType(Type* type) {
  Type* oldFirst = types;
  types = type;
  type->nxt = oldFirst;
  typeConstructor(type);
}

void load() {
  int c;
  FILE* s = fopen("app.qc", "r"); // FIXME: Check if valid file. Not NULL.
  if (s != NULL) {
    char input[512] = "";
    while ((c = getc(s)) != EOF) {
      if (c == '\r' || c == '\n') {
        char err[52] = "";
        /*if (evalCmd(input, err)) {
          input[0] = '\0';
        } else {
          // FIXME: Throw error.
        }*/
      } else {
        straddch(input, c);
      }
    }
    fclose(s);
  }
}

//void runCmd(char* retVal, Cmd* cmd);

Var* addNewVar(Type* type, char* name) {
  Var* var = malloc(sizeof(Var));
  strcpy(var->name, name);
  var->type = type;
  var->val = NULL;

  Var* oldFirst = vars;
  vars = var;
  var->nxt = oldFirst;

  return var;
}

Func* createFunc(Val* args) {
  Func* f = newFunc();
  strcpy(f->name, (char*)args->nxt->addr);
  Val* block = (Val*)args->nxt->nxt->addr;
  Val* arg = block;
  Attr* a = NULL;
  //while(arg != NULL && arg->type == PAIR) {
  while(arg != NULL && arg->nxt != NULL) {
    if (a == NULL) {
      f->args = newAttr();
      a = f->args;
    } else {
      a->nxt = newAttr();
      a = a->nxt;
    }
    Val* pair = (Val*)arg->addr;
    a->type = parseVarType((char*)pair->addr);
    strcpy(a->name, (char*)pair->nxt->addr);
    arg = arg->nxt;
  }
  f->cmd = cpyVal(arg);
  Func* oldFirst = funcs;
  funcs = f;
  f->nxt = oldFirst;
  return f;
}
Val* define(Val* args) {
  Func* f = createFunc(args);
  return NULL;
}
Val* defineOp(Val* args) {
  Func* f = createFunc(args);
  f->isOperator = 1;
  return NULL;
}
Val* defineType(Val* args) { // Type #:: (type name) (type name)
  Type* type = newType();
  strcpy(type->name, (char*)args->nxt->addr);
  Val* v;
  for (v = args->nxt->nxt; v != NULL; v = v->nxt) {
    Val* v2 = (Val*)v->addr;
    Attr* a;
    if (type->attrs == NULL) {
      type->attrs = newAttr();
      a = type->attrs;
    } else {
      a->nxt = newAttr();
      a = a->nxt;
    }
    Type* t = typeByName((char*)v2->addr);
    if (t == NULL) {
      freeType(type);
      return errorStr("Unkown arg type.\n");
    }
    a->type = parseVarType((char*)v2->addr);
    strcpy(a->name, (char*)v2->nxt->addr);
  }
  addType(type);
  return NULL;
}

Val* assign(Val* args) {
  Var* v = varByName((char*)args->nxt->addr);
  if (v == NULL) {
    v = addNewVar(NULL, (char*)args->nxt->addr); // FIXME hardcoded type, but maybe var type is useless anyway.
  } else if (v->val != NULL) {
    freeVal(v->val);
  }
  v->val = cpyVal(args->nxt->nxt);
  return cpyVal(v->val);
}

Val* listVars() {
  Var* v;
  char* m = malloc(sizeof(char) * 2056);
  for (v = vars; v != NULL; v = v->nxt) {
    catVar(m, v);
    if (v->nxt != NULL) {
      strcat(m, "\n");
    }
  }
  return initPtr(varType(CHAR, 0, 2056), m);
}

static void finish(int sig)
{
  freeTypes(types);
  types = NULL;
  freeVar(vars);
  vars = NULL;
  freeFunc(funcs);
  funcs = NULL;

  exit(0);
}

int main()
{
  signal(SIGINT, finish);      /* arrange interrupts to terminate */

  load();

  finish(0);
  return 0;
}
