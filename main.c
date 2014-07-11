#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

#include "model.h"
#include "src/bind/glue.h"

//#define CURSES_MODE
#ifdef CURSES_MODE
#include <ncurses.h>
#else
#include <SDL/SDL.h>
static SDL_Surface* screen = NULL;

void move(int y, int x) {
}

void delch() {
}

void addch(char ch) {
}

void refresh() {
}

int getch() {
}

void deleteln() {
}

void addstr(const char* str) {
}
#endif

int getLines() {
#ifdef CURSES_MODE
  return LINES;
#else
#endif
}

int getX() {
#ifdef CURSES_MODE
  int y, x;
  getyx(curscr, y, x);
  return x;
#else
  return 0;
#endif
}

int getY() {
#ifdef CURSES_MODE
  int y, x;
  getyx(curscr, y, x);
  return y;
#else
  return 0;
#endif
}


//#include "bind.h"
#include "src/bind/bind.h"

// Version 0.1 == Etre capable de tout programmer le programme lui-meme dans celui-ci.

// MODEL

static const int MSG_CONSOLE_SIZE = 10;
static const char DEF_FILE_PATH[] = "defs";

static Type* types = NULL;
// Maybe vars by scope.
static Var* vars = NULL;
static Func* funcs = NULL;
static CStruct* structs = NULL;

// TODO: Have a list that contains both the loaded defs and the runtime one.
// They should of type struct LoadedDef and the function passed would call the executable.
static LoadedDef* loadedDefs = NULL;

CStruct* newCStruct() {
  CStruct* arg0 = malloc(sizeof(CStruct));
  if (arg0 == NULL) {
    abort(); // FIXME: "Can't allocate memory"
  }
  memset(arg0->name, '\0', sizeof(arg0->name));
  arg0->attrs = NULL;
  return arg0;
}

CFunc* newCFunc() {
  CFunc* arg0 = malloc(sizeof(CFunc));
  if (arg0 == NULL) {
    abort(); // FIXME: "Can't allocate memory"
  }
  memset(arg0->name, '\0', sizeof(arg0->name));
  memset(arg0->ret, '\0', sizeof(arg0->ret));
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
  memset(f->ret, '\0', sizeof(f->ret));
  f->cmd = NULL;
  f->args = NULL;
  f->isOperator = 0;
  f->nxt = NULL;
  return f;
}

Arg* newArg() {
  Arg* arg0 = malloc(sizeof(Arg));
  if (arg0 == NULL) {
    abort(); // FIXME: "Can't allocate memory"
  }
  memset(arg0->name, '\0', sizeof(arg0->name));
  memset(arg0->type, '\0', sizeof(arg0->type));
  arg0->nxt = NULL;
  return arg0;
}

Arg* appendNewArg(Arg* arg) {
  if (arg == NULL) {
    return newArg();
  } else {
    arg->nxt = newArg();
    return arg->nxt;
  }
}

void freeVar(Var* t) {
  if (t != NULL) {
    freeVar(t->nxt);
    free(t);
  }
}

void freeType(Type* t) {
  if (t != NULL) {
    freeType(t->nxt);
    free(t);
  }
}

void freeLoadedDef(LoadedDef* d) {
  if (d != NULL) {
    freeLoadedDef(d->nxt);
    free(d);
  }
}

void freeArg(Arg* f) {
  if (f != NULL) {
    freeArg(f->nxt);
    free(f);
  }
}

void freeCStruct(CStruct* s) {
  if (s != NULL) {
    freeArg(s->attrs);
    free(s);
  }
}

void freeCFunc(CFunc* f) {
  if (f != NULL) {
    freeArg(f->args);
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
    freeCmd(f->cmd);
    freeArg(f->args);
    freeFunc(f->nxt);
    free(f);
  }
}

char* strVal(char* val, Cmd* cmd) {
  strncpy(val, cmd->name + 1, strlen(cmd->name) - 2);
  return val;
}

/*char* catArg(char* m, Arg* arg) {
  if (arg != NULL) {
    Arg* n = arg->nxt;
    strcat(m, arg->val);
    while (n != NULL) {
      strcat(m, " ");
      //strcat(m, " -> ");
      strcat(m, n->val);
      n = n->nxt;
    }
  }
  return m;
}*/

void setVarVal(Var* v, Cmd* val) {
  if (v == NULL) return;
  if (v->val != NULL) {
    freeCmd(v->val);
  }
  v->val = newCmd();
  strcpy(v->val->name, val->name);
  v->val->args = val->args;
  v->val->nxt = val->nxt;
}

// HELPER

char* straddch(char* str, char c) { //FIXME: Not buffer safe
  int i = strlen(str);
  str[i] = c;
  str[i+1] = '\0';
  return str;
}
char* strdelch(char* str) {
  int i = strlen(str);
  if (i > 0) {
    str[i-1] = '\0';
  }
  return str;
}

// NCURSES HELPER

// TODO: debug(), fatal(), error(), warn(), log()

static int silent = 0;
void output(const char* str) {
  if (silent) return;

  const char* c = str;
  int l = getY();
  int n = 0;
  while (*c != '\0') {
    if (*c == '\n') {
      ++l;
      if (l >= getLines()) {
        move(0,0);
        deleteln();
        move(getLines()-(++n),0);
        refresh();
      }
    }
    ++c;
  }
  addstr(str);
  refresh();
}

Cmd* outputStr(const char* str) {
  if (strlen(str) >= 52) return outputStr("Error, string too big.");
  Cmd* cmd = newCmd();
  cmd->type = STRING;
  strcpy(cmd->name, str);
  return cmd;
}

// Return a pointer to the first non whitespace char of the string.
char* trim(char* s) {
  char* c = s;
  while (*c != '\0' && (*c == ' ' || *c == '\n')) {
    c++;
  }
  return c;
}

// APP

char* catCmdType(char* b, CmdType t) {
  if (t == INT) {
    strcat(b, "INT");
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
  } else if (t == FLOAT) {
    strcat(b, "FLOAT");
  } else if (t == EDITOR) {
    strcat(b, "EDITOR");
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
  } else if (t == BOOL) {
    strcat(b, "BOOL");
  } else if (t == CHAR) {
    strcat(b, "CHAR");
  } else if (t == ERROR) {
    strcat(b, "ERROR");
  } else if (t == ARRAY) {
    strcat(b, "ARRAY");
  } else if (t == POINTER) {
    strcat(b, "POINTER");
  } else {
    strcat(b, "FIXME_UNKOWN_TYPE");
  }
}

char* catPrintCmd(char* b, Cmd* cmd) {
  if (cmd == NULL) return b;
  Cmd* n;
  if (cmd->type == ARRAY) {
    strcat(b, "[");
    for (n = cmd->args; n != NULL; n = n->nxt) {
      catPrintCmd(b, n);
      if (n->nxt != NULL) {
        strcat(b, " ");
      }
    }
    strcat(b, "]");
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

void catVar(char* m, Var* v) {
  if (v != NULL) {
    strcat(m, v->type->name);
    strcat(m, " ");
    strcat(m, v->name);
    if (v->val != NULL) {
      strcat(m, " ");
      catCmd(m, v->val);
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

LoadedDef* loadedFuncByName(char* name) {
  LoadedDef* d = loadedDefs;
  while (d != NULL) {
    if (strcmp(d->name, name) == 0) {
      return d;
    }
    d = d->nxt;
  }
  return NULL;
}

int isFloat(char* str) {
  char* c;
  int hasDecimal = 0;
  for (c = str; *c != '\0'; ++c) {
    if (*c == '.' || *c == ',') {
      if (hasDecimal) {
        return 0;
      }
      hasDecimal = 1;
    } else if (*c < '0' || *c > '9') {
      return 0;
    }
  }
  return hasDecimal ? 1 : 0;
}
int isInteger(char* str) {
  char* c;
  for (c = str; *c != '\0'; ++c) {
    if (*c < '0' || *c > '9') {
      return 0;
    }
  }
  return 1;
}

Cmd* runFunc(Cmd* cmd) {
  // TODO, replace all of f->args in f->cmd by cmd->args
  Func* f = funcByName(cmd->name);
  LoadedDef* d = loadedFuncByName(f->cmd->name);
  return (d != NULL) ? d->ptr(cmd) : NULL;
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
        LoadedDef* f = loadedFuncByName(n);
        Var* v;
        if (f != NULL) {
          c->type = f->type;
        } else if ((v = varByName(n)) != NULL) {
          c->type = VAR;
        } else if (strlen(n) == 3 && n[0] == '\'' && n[2] == '\'') { // FIXME: Does not work '\0'
          c->type = CHAR;
        } else if (strlen(n) >= 2 && n[0] == '\"' && n[strlen(n)-1] == '\"') {
          c->type = STRING;
        } else if (n[0] == ':') {
          c->type = EDITOR;
        } else if (isInteger(n)) {
          c->type = INT;
        } else if (isFloat(n)) {
          c->type = FLOAT;
        } else {
          // TODO: Check for int.
          c->type = UNKOWN;
        }
      }
    }
  }
  return cmd;
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
ParsePair parseArg(char* command) {
  Cmd* cmd = newCmd();
  cmd->type = PAIR;
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
      p = parseArg(p.ptr+1);
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
  Cmd* block = newCmd();
  block->type = BLOCK;
  ParsePair p = parse(command, ALLOWED_NAME_CHARS);
  s = p.ptr;
  if (*s != ':') {
    //msg("Error parsing block. Missing return type.");
    freeCmd(block);
    return parsePair(NULL, s);
  }
  block->args = p.cmd;
  block->args->type = TYPE;
  s = trim(s+1);
  if (*s == '|') {
    p = parseArg(++s);
    block->args->nxt = p.cmd;
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
  Cmd* ary = newCmd();
  ary->type = ARRAY;
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
ParsePair parseCmdR(char* command) { // FIXME: Does not work for "(add 12 12)"
  Cmd* cmds = newCmd();
  Cmd* cmd = cmds;
  char* s = trim(command);
  while (*s != '\0') {
    char* i = s;
    while (*s != '\0' && *s != ' ' && *s != '(' && *s != ')' &&
           *s != '{' && *s != '}' && *s != '[' && *s != '\n') {
      ++s;
    }
    strncpy(cmd->name, i, s-i);
    s = trim(s);
    if (*s == ')') {
      ++s;
      break;
    } else if (*s == '[') {
      ParsePair p = parseArray(s+1);
      s = p.ptr;
      if (p.cmd != NULL) {
        cmd->nxt = p.cmd;
        cmd = cmd->nxt;
      }
    } else if (*s == '}') {
      break;
    } else if (*s == '{') {
      ParsePair p = parseBlock(s+1);
      s = p.ptr;
      if (p.cmd != NULL) {
        cmd->nxt = p.cmd;
        cmd = cmd->nxt;
      }
    } else if (*s == '(') {
      ParsePair p = parseCmdR(s+1);
      s = p.ptr;
      cmd->nxt = p.cmd;
      cmd = cmd->nxt;
    } else if (*s != '\0') {
      cmd->nxt = newCmd();
      cmd = cmd->nxt;
    }
  }
  typeCmd(cmds);
  if (cmds->nxt != NULL && (cmds->nxt->type == OPERATOR || cmds->nxt->type == MACRO_OP)) {
    cmd = cmds;
    cmds = cmds->nxt;
    cmds->args = cmd;
    cmd->nxt = cmds->nxt;
    cmds->nxt = NULL;
  } else if (cmds->type == FUNCTION || cmds->type == MACRO || cmds->type == CFUNCTION || strlen(cmds->name) > 0) {
    cmds->args = cmds->nxt;
    cmds->nxt = NULL;
  }
  return parsePair(cmds, s);
}
Cmd* parseCmd(char* command) {
  return parseCmdR(command).cmd;
}

Cmd* getInput() {  
  char input[256] = "";
  int y, x;
  int nested = 0;
  while (1) {
#ifdef CURSES_MODE
#else
    SDL_Event event;     
    /* Poll for events */
    while( SDL_PollEvent( &event ) ){
      switch( event.type ) {
      case SDL_QUIT:
        return NULL;
        break;
      }
    }
#endif
    int ch = getch();
    if (ch == 8 || ch == 127) {
      if (strlen(input) > 0) {
        strdelch(input);
        move(getY(), getX()-1);
        delch();
        refresh();
      }
    } else if (ch == '\n' || ch == '\r') {
      if (nested > 0) {
        straddch(input, ' '); // Treat as whitespace maybe???
        output("\n");
        int i;
        for (i = 0; i < nested; i++) {
          addch(' ');
          addch(' ');
        }
      } else {
        break;
      }
    } else if (ch >= ' ' && ch < '~') { // Only show printable characters.
      if (ch == '{') {
        nested++;
      } else if (ch == '}') {
        nested--;
      }
      addch(ch);
      refresh();
      straddch(input, ch);
    }
  }
  if (nested != 0) {
    //msg("Invalid block syntax.");
    return NULL;
  }
  return parseCmd(input);
}

int isOperator(char* opName) {
  if (strcmp(opName, "::") == 0 ||
      strcmp(opName, "=") == 0) {
    return 1; 
  }
  return 0;
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

/*void compileFunc(char* s, Func* f) {
  char tmp[1024] = "";
  Arg* ret;
  Arg* arg;
  Arg* arg2 = NULL;
  int n;

  for (arg = f->args; arg->nxt != NULL; arg = arg->nxt ) {
  }
  ret = arg;

  char escName[128] = "";
  strcpy(escName, f->name);
  escapeName(escName);
  sprintf(tmp, "%s %s(", ret->val, escName);
  strcat(s, tmp);
 
  for (n = 0, arg = f->args; arg->nxt != NULL; arg = arg->nxt, n++ ) {
    if (f->lambda) {
      arg2 = (arg2 == NULL) ? f->lambda->args : arg2->nxt;
      sprintf(tmp, "%s %s", arg->val, arg2->val);
    } else {
      sprintf(tmp, "%s arg%d", arg->val, n);
    }
    strcat(s, tmp);
    if (arg->nxt->nxt != NULL) {
      strcat(s, ", ");
    }
  }

  strcat(s, ") {\n");
  if (f->lambda) {
    strcat(s, f->lambda->body);
  }
  strcat(s, "\n}");
}*/

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
    fprintf(s, "%s %s {%s: ", f->name, f->isOperator ? ":::" : "::", f->ret);
    if (f->args != NULL) {
      fprintf(s, "|");
      Arg* a;
      for (a = f->args; a != NULL; a = a->nxt) {
        fprintf(s, "%s %s", a->type, a->name);
        if (a->nxt != NULL) {
          fprintf(s, ", ");
        }
      }
      fprintf(s, "| ");
    }
    char cmdExe[256] = "";
    catCmdExe(cmdExe, f->cmd, 0);
    fprintf(s, "%s }\n", cmdExe);
  }
  /*Func* f = NULL;
  char m[1024] = "";
  for (f = defs; f != NULL; f = f->nxt) {
    m[0] = '\0';
    fprintf(s, "\n%s", catFunc(m, f));
  }
  fclose(s);*/
}

void bindCFunctionsHeader(char* fname, CFunc* fs) {
  CFunc* f;
  Arg* a;

  char filename[52] = "";
  sprintf(filename, "src/bind/bind_%s.h", fname);
  FILE* s = fopen(filename, "w");

  fprintf(s, "#ifndef BIND_H\n");
  fprintf(s, "#define BIND_H\n\n");
  fprintf(s, "#include <stdlib.h>\n"); // FIXME: Use include given
  fprintf(s, "#include <stdio.h>\n");
  fprintf(s, "#include <signal.h>\n");
  fprintf(s, "#include <string.h>\n\n");
  fprintf(s, "#include \"../../model.h\"\n");
  fprintf(s, "#include \"glue.h\"\n\n");

  fprintf(s, "void initCFunctions(LoadedDef* d);\n\n");

  for (f = fs; f != NULL; f = f->nxt) {
    fprintf(s, "Cmd* bind_%s(Cmd* cmd);\n", f->name);
  }

  fprintf(s, "#endif // BIND_H");
  fclose(s);
}

char* argTypeFunc(char* name) {
  char type[52];
  strcpy(type, name); 
  name[0] = '\0';
  if (strcmp(type, "char*") == 0) {
    sprintf(name, "argstring");
  } else {
    sprintf(name, "arg%s", type);
  }
  return name;
}

void bindCFunctionsSource(char* fname, CFunc* fs) {
  CFunc* f;
  Arg* a;

  char filename[52] = "";
  sprintf(filename, "src/bind/bind_%s.c", fname);
  FILE* s = fopen(filename, "w");

  fprintf(s, "#include \"bind_%s.h\"\n\n", fname);

  fprintf(s, "void initCFunctions(LoadedDef* d) {\n");
  for (f = fs; f != NULL; f = f->nxt) {
    fprintf(s, "  addLoadedDef(d, \"%s\", CFUNCTION, bind_%s);\n", f->name, f->name);
  }
  fprintf(s, "}\n\n");

  // In case the header was not defined, that it was just
  // a source file, define the prototype of the function.
  for (f = fs; f != NULL; f = f->nxt) {
    fprintf(s, "%s %s(", f->ret, f->name);
    for (a = f->args; a != NULL; a = a->nxt) {
      fprintf(s, "%s %s", a->type, a->name);
      if (a->nxt != NULL) {
        fprintf(s, ", ");
      }
    }
    fprintf(s, ");\n");
  }
  fprintf(s, "\n");

  for (f = fs; f != NULL; f = f->nxt) {
    fprintf(s, "Cmd* bind_%s(Cmd* cmd) {\n", f->name);
    fprintf(s, "  Cmd* args = cmd->args;\n");
    int i;
    for (i = 1, a = f->args; a != NULL; a = a->nxt, i++) {
      char argTypeFuncName[52] = "";
      char type[52] = "INT"; // FIXME
      strcpy(argTypeFuncName, a->type);
      fprintf(s, "  if (!validArg(&args, %s)) return errorStr(\"Invalid arg %d: Expected type %s\");\n",
                 type, i, type);
      fprintf(s, "  %s %s0 = %s(&args);\n", a->type, a->name, argTypeFunc(argTypeFuncName));
    }
    if (strlen(f->ret) > 0) {
      fprintf(s, "  char ret[52] = \"\";\n");
      fprintf(s, "  retCmd(INT, cat_arg%s(ret, %s(", f->ret, f->name);
    } else {
      fprintf(s, "  %s(", f->name);
    }
    for (a = f->args; a != NULL; a = a->nxt) {
      fprintf(s, "%s0", a->name);
      if (a->nxt != NULL) {
        fprintf(s, ", ");
      }
    }
    if (strlen(f->ret) > 0) {
      fprintf(s, "))");
    }
    fprintf(s, ");\n}\n\n");
  }
  fclose(s);
}

CFunc* parseCFunction(char* s0) {
  char* s = s0;
  CFunc* f = newCFunc();
  char* lastSpace = NULL;
  char* lastArg = s0;
  Arg* a = NULL;
  int nested = 0;
  while (*s != '\0') {
    if (*s == ' ' && lastArg == s) {
      // discard trailing spaces;
      ++lastArg;
    } else if (*s == ' ') {
      lastSpace = s;
    } else if (a == NULL && *s == '(') {
      strncpy(f->ret, s0, lastSpace - s0);
      strncpy(f->name, lastSpace+1, s - (lastSpace+1));
      lastArg = s+1;
    } else if (*s == ',' || (nested == 1 && *s == ')' && lastArg != s && lastSpace > lastArg)) {
      if (a == NULL) {
        f->args = newArg();
        a = f->args;
      } else {
        a->nxt = newArg();
        a = a->nxt;
      }
      strncpy(a->type, lastArg, lastSpace - lastArg);
      strncpy(a->name, lastSpace+1, s - (lastSpace+1));
      lastArg = s+1;
    }
    if (*s == '(') ++nested;
    if (*s == ')') --nested;
    ++s;
  }
  return f;
}

Cmd* parseCIncludeFile(Cmd* cmd) {
  FILE* s = fopen(cmd->args->nxt->name, "r");
  char c;
  char p = EOF;
  CFunc* f0 = NULL;
  CFunc* f = NULL;
  if (s == NULL) {
    return errorStr("Invalid include file.");
  }
  char input[512] = "";
  int inMultiComment = 0;
  int nested = 0;
  int nestedP = 0;
  int discardToEOL = 0;
  int discardToSemiColon = 0;
  while ((c = getc(s)) != EOF) {
    if (inMultiComment) {
      if (p == '*' && c == '/') {
        inMultiComment = 0;
      }
    } else if (c == '{') {
      ++nested;
    } else if (nested > 0) {
      if (c == '}') {
        --nested;
      }
    } else if (c == '\r' || c == '\n') {
      if (p != '\\' && nestedP == 0) {
        if (strlen(input) > 0) {
          if (strchr(input, '(')) {
            if (f0 == NULL) {
              f0 = parseCFunction(input);
              f = f0;
            } else {
              f->nxt = parseCFunction(input);
              f = f->nxt;
            }
          }
          input[0] = '\0';
        }
        discardToEOL = 0;
      }
    } else if ((c == ' ' || c == '\t') && strlen(input) <= 0) {
      // Discard trailing whitespaces
    } else if ((p == ' ' || p == '\t') && (c == ' ' || c == '\t')) {
      // Discard double whitespaces
    } else if (discardToSemiColon) {
      if (c == ';') {
        discardToSemiColon = 0;
      }
    } else if (discardToEOL) {
      // Discard comments
    } else if (c == '#') {
      // Discard preprocessor
      discardToEOL = 1;
    } else if (p == '/' && c == '/') {
      discardToEOL = 1;
      strdelch(input);
    } else if (p == '/' && c == '*') {
      inMultiComment = 1;
      strdelch(input);
    } else if (strncmp(input, "__", 2) == 0) {
      discardToEOL = 1;
      input[0] = '\0';
    //} else if (strncmp(input, "typedef", 7) == 0) { // case: __extension typedef...
    } else if (strstr(input, "typedef") != NULL || strstr(input, "struct") != NULL) {
      discardToSemiColon = 1;
      input[0] = '\0';
    } else {
      if (c == '(') {
        ++nestedP;
      } else if (c == ')') {
        --nestedP;
      }
      if (c == '\t') {
        straddch(input, ' ');
      } else {
        straddch(input, c);
      }
    }
    p = c;
  }
  bindCFunctionsHeader(cmd->args->name, f0);
  bindCFunctionsSource(cmd->args->name, f0);
  freeCFunc(f);
  return NULL;
}

void eval(Cmd* cmd);

void load() {
  int c;
  size_t n = 0;
  FILE* s = fopen("app.qc", "r"); // FIXME: Check if valid file. Not NULL.
  if (s != NULL) {
    char input[512] = "";
    while ((c = getc(s)) != EOF) {
      if (c == '\r' || c == '\n') {
        Cmd* cmd = parseCmd(input);
        eval(cmd);
        freeCmd(cmd);
        input[0] = '\0';
      } else {
        straddch(input, c);
      }
    }
    fclose(s);
  }
}

//void runCmd(char* retVal, Cmd* cmd);

char* argVal(char* buf, Cmd* arg) {
  if (arg->args) {
    char argValue[128] = "";
    //runCmd(argValue, arg);
    strcat(buf, argValue);
  } else {
    strcat(buf, arg->name);
  }
}

/*void runCmd(char* retVal, Cmd* cmd) { // FIXME: Fonction dependencies must be added too.
  Func* f = funcByName(cmd->name);
  if (f == NULL) return;
  if (f->args == NULL) return; // Invalid function. Needs return type. FIXME: Better error handling

  Cmd* c;
  Arg* arg;
  Arg* ret;
  int n;
  int i;

  // TODO: Move this part to a function
  char escName[128] = "";
  strcpy(escName, cmd->name);
  escapeName(escName);
  char filename[64] = "";
  sprintf(filename, "tmp/%s.c", escName);
  FILE* s = fopen(filename, "w"); // FIXME: Check if valid file. Not NULL.
  fprintf(s, "#include <stdlib.h>\n");
  fprintf(s, "#include <stdio.h>\n\n");

  for (arg = f->args; arg->nxt != NULL; arg = arg->nxt ) {
  }
  ret = arg;
  
  char fs[2056] = "";
  compileFunc(fs, f);
  fprintf(s, "%s", fs);

  fprintf(s, "\n\nint main(int argc, char* argv[]) {\n");

  for (n = 0, arg = f->args; arg->nxt != NULL; arg = arg->nxt, n++ ) {
    fprintf(s, "  %s arg%d;\n", arg->val, n);
  }

  fprintf(s, "  if (argc != %d) {\n", n + 1);
  fprintf(s, "    fprintf(stderr, \"Invalid amout of parameters.\\n\");\n");
  fprintf(s, "    return -1;\n");
  fprintf(s, "  }\n\n");
  for (n = 0, arg = f->args; arg->nxt != NULL; arg = arg->nxt, n++ ) {
    fprintf(s, "  sscanf(argv[%d],\"%%d\",&arg%d);\n", n+1, n);
  }
  if (strcmp(ret->val, "void") == 0) {
    fprintf(s, "  %s(", escName);
  } else {
    fprintf(s, "  %s r = %s(", ret->val, escName);
  }
  for (i = 0; i < n; i++) {
    fprintf(s, "arg%d", i);
    if (i < n-1) {
      fprintf(s, ", ");
    }
  }
  fprintf(s, ");\n");
  if (strcmp(ret->val, "void") != 0) {
    fprintf(s, "  printf(\"%%d\", r);\n"); // FIXME: Not always interger.
  }
  fprintf(s, "  return 0;\n");
  fprintf(s, "}\n");
  fclose(s);

  char exeFilename[64] = "";
  sprintf(exeFilename, "tmp/%s", escName);
  char cmds[256] = "";
  strcat(cmds, "gcc -o ");
  strcat(cmds, exeFilename);
  strcat(cmds, " ");
  strcat(cmds, filename);
  strcat(cmds, " && ./");
  strcat(cmds, exeFilename);
  for (c = cmd->args; c != NULL; c = c->nxt) {
    strcat(cmds, " "); 
    argVal(cmds, c);
  }
  FILE *fp = popen(cmds, "r"); // TODO: Args

  fscanf(fp, "%s", retVal);
  pclose(fp);
}*/

Cmd* printVar(Cmd* cmd) {
  char b[1024] = "";
  catVar(b, varByName(cmd->name));
  return outputStr(b);
}

Var* addNewVar(char* type, char* name) {
  Var* var = malloc(sizeof(Var));
  strcpy(var->name, name);
  var->type = typeByName(type);
  var->val = NULL;

  Var* oldFirst = vars;
  vars = var;
  var->nxt = oldFirst;

  return var;
}

Cmd* createVar(Cmd* cmd) {
  addNewVar(cmd->name, cmd->args->name);
  return NULL;
}

Cmd* createType(Cmd* cmd) {
  Type* type = malloc(sizeof(Type));
  char name[52] = "";
  strcpy(type->name, strVal(name, cmd->args));
  Type* oldFirst = types;
  types = type;
  type->nxt = oldFirst;
  addLoadedDef(loadedDefs, type->name, MACRO, createVar);
  return NULL;
}

Func* createFunc(Cmd* cmd) {
  Func* f = newFunc();
  strcpy(f->name, cmd->args->name);
  Cmd* block = cmd->args->nxt;
  strcpy(f->ret, block->args->name);
  Cmd* arg = block->args->nxt;
  Arg* a = NULL;
  while(arg != NULL && arg->type == PAIR) {
    if (a == NULL) {
      f->args = newArg();
      a = f->args;
    } else {
      a->nxt = newArg();
      a = a->nxt;
    }
    strcpy(a->type, arg->args->name);
    strcpy(a->name, arg->args->nxt->name);
    arg = arg->nxt;
  }
  f->cmd = arg;
  Func* oldFirst = funcs;
  funcs = f;
  f->nxt = oldFirst;
  return f;
}
Cmd* define(Cmd* cmd) {
  Func* f = createFunc(cmd);
  addLoadedDef(loadedDefs, f->name, FUNCTION, runFunc);
  return NULL;
}
Cmd* defineOp(Cmd* cmd) {
  Func* f = createFunc(cmd);
  f->isOperator = 1;
  addLoadedDef(loadedDefs, f->name, OPERATOR, runFunc);
  return NULL;
}

Cmd* assign(Cmd* cmd) {
  Var* v = varByName(cmd->args->name);
  if (v == NULL) {
    v = addNewVar("int", cmd->args->name); // FIXME hardcoded type, but maybe var type is useless anyway.
  }
  if (v == NULL) return NULL;
  setVarVal(v, cmd->args->nxt);
  Cmd* c = newCmd();
  strcpy(c->name, v->val->name);
  c->type = v->val->type;
  return c;
}

Cmd* listTypes(Cmd* cmd) {
  Type* t;
  char m[1024] = "";
  for (t = types; t != NULL; t = t->nxt) {
    strcat(m, t->name);
    if (t->nxt != NULL) {
      strcat(m, "\n");
    }
  }
  return outputStr(m);
}

Cmd* listVars(Cmd* cmd) {
  Var* v;
  char m[1024] = "";
  for (v = vars; v != NULL; v = v->nxt) {
    catVar(m, v);
    if (v->nxt != NULL) {
      strcat(m, "\n");
    }
  }
  return outputStr(m);
}
Cmd* listDefs(Cmd* cmd) {
  LoadedDef* d;
  Cmd* arr = newCmd();
  arr->type = ARRAY;
  Cmd* n = NULL;
  for (d = loadedDefs; d != NULL; d = d->nxt) {
    if (n == NULL) {
      arr->args = newCmd();
      n = arr->args;
    } else {
      n->nxt = newCmd();
      n = n->nxt;
    }
    strcpy(n->name, d->name);
    n->type = STRING;
  }
  return arr;
}

// Reduces a Cmd down to a primitive type.
// If cmd->type == CFUNCTION, call it and replace the cmd by it's result
// If cmd->type == FUNCTION, call it and replace the cmd by it's result
// If cmd->type == VAR, get it's value and replace the cmd by it's result
Cmd* cmdVal(Cmd* cmd) {
  Cmd* ret = NULL;
  if (cmd->type == FUNCTION || cmd->type == OPERATOR || cmd->type == CFUNCTION) {
    Cmd* nCmd = newCmd();
    Cmd* n = NULL;
    Cmd* arg;
    for (arg = cmd->args; arg != NULL; arg = arg->nxt) {
      if (n == NULL) {
        nCmd->args = cmdVal(arg); 
        n = nCmd->args;
      } else {
        n->nxt = cmdVal(arg);
        if (n->nxt != NULL) {
          n = n->nxt;
        }
      }
    }
    strcpy(nCmd->name, cmd->name);
    nCmd->type = cmd->type;
    // Do I copy the body???
    ret = loadedFuncByName(cmd->name)->ptr(nCmd);
    freeCmd(nCmd);
  } else if (cmd->type == MACRO || cmd->type == MACRO_OP) {
    ret = loadedFuncByName(cmd->name)->ptr(cmd);
  } else if (cmd->type == VAR) {
    Var* v = varByName(cmd->name);
    if (v->val != NULL) {
      ret = newCmd();
      strcpy(ret->name, v->val->name);
      ret->type = v->val->type;
    }
  } else {
    ret = newCmd();
    strcpy(ret->name, cmd->name);
    ret->type = cmd->type;
  }
  return ret;
}

void eval(Cmd* cmd) {
  if (cmd == NULL) return;

  Cmd* ret = cmdVal(cmd);
  if (ret == NULL) {
    output("\n=> ");
    output(cmd->name);
  } else if (ret->type == ERROR) {
    output("\nError: ");
    output(ret->name);
  } else {
    output("\n=> ");
    char m[1024] = "";
    output(catPrintCmd(m, ret));
    freeCmd(ret);
  }
 
  eval(cmd->nxt);
}

void loop()
{
  int continuer = 1;
  output(">> ");
  while (continuer) {
    Cmd* cmd = getInput();
    if (cmd != NULL) {
      if (cmd->type == EDITOR) {
        char* name = cmd->name + 1;
        if (strcmp(name, "exit") == 0 ||
                   strcmp(name, "quit") == 0 ||
                   strcmp(name, "q") == 0) {
          freeCmd(cmd);
          return;
        } else if (strcmp(name, "h") == 0 ||
                   strcmp(name, "help") == 0) {
          char m[1024] = "";
          output("\n");
          output(catPrintCmd(m, listDefs(NULL)));
        } else if (strcmp(name, "s") == 0 ||
                   strcmp(name, "save") == 0) {
          save();
        }
      } else if (cmd->type == UNKOWN) {
        output("\nError. Unkown function.");
      } else {
        eval(cmd);
      }
      freeCmd(cmd);
    } else {
      return;
    }
    output("\n>> ");
    refresh();
  }
}

static void finish(int sig)
{
#ifdef CURSES_MODE
  endwin();
#else
  SDL_Quit();
#endif

  freeLoadedDef(loadedDefs);
  loadedDefs = NULL;
  freeType(types);
  types = NULL;
  freeVar(vars);
  vars = NULL;
  freeFunc(funcs);
  funcs = NULL;

  exit(0);
}

void initLoadedDefs() {
  loadedDefs = createLoadedDef("=", MACRO_OP, assign); // Assigns a value to an existing variable.
  addLoadedDef(loadedDefs, "::", MACRO_OP, define); // Assigns a function to a new variable.
  addLoadedDef(loadedDefs, ":::", MACRO_OP, defineOp); // Assigns a function to a new variable.
  addLoadedDef(loadedDefs, "type", MACRO, createType);
  addLoadedDef(loadedDefs, "$vars", FUNCTION, listVars);
  addLoadedDef(loadedDefs, "$types", FUNCTION, listTypes);
  addLoadedDef(loadedDefs, "$defs", FUNCTION, listDefs);
  addLoadedDef(loadedDefs, "include", FUNCTION, parseCIncludeFile);
}

void main()
{
  signal(SIGINT, finish);      /* arrange interrupts to terminate */

  initLoadedDefs();
  initCFunctions(loadedDefs);

  silent = 1;
  load();
  silent = 0;

#ifdef CURSES_MODE
  initscr();
  keypad(stdscr, TRUE);
  nonl();         /* tell curses not to do NL->CR/NL on output */
  cbreak();       /* take input chars one at a time, no wait for \n */
  noecho();       /* dont echo the input char */
#else
  SDL_Init( SDL_INIT_EVERYTHING );

  screen = SDL_SetVideoMode(1024, 768, 32, SDL_SWSURFACE);
#endif

  loop();

  finish(0);
}
