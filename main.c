#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

#include <ffi.h>

#include "model.h"
#include "src/bind/glue.h"

#include "src/lib.h"

//#include "bind.h"
#include "src/bind/bind.h"

void initCFunctions(LoadedDef* d);

//#define CURSES_MODE
//#define DEBUG_MODE

// TODO: :d => lists the function prototype
// TODO: x = "hello"; straddch(x, '!'); x

// TODO: Enlever les fonctions hardcoder comme assign, runFunc, etc... Les mettres dans lib au pire. Qu'il n'y ait plus de fonction qui prennent Cmd en param.

// TODO: Enlever Cmd. Petit par petit en faisant tout dans ce qui est actuellement parseCmdR.

// Version 0.1 == Etre capable de tout programmer le programme lui-meme dans celui-ci.
// Version 0.2 == Deux screen. Une console et une qui affiche les variables et les fonctions.
// TODO: Rouler l'application dans une autre fenetre que l'editeur. Vraiment separer app/editeur.

// MODEL

static const int MSG_CONSOLE_SIZE = 10;
static const char DEF_FILE_PATH[] = "defs";

static Type* types = NULL;
// Maybe vars by scope.
static Var* vars = NULL;
static Func* funcs = NULL;

// TODO: Have a list that contains both the loaded defs and the runtime one.
// They should of type struct LoadedDef and the function passed would call the executable.
static LoadedDef* loadedDefs = NULL;

Cmd* cpyCmd(Cmd* cmd) {
  if (cmd == NULL) return NULL;
  
  Cmd* c = initCmd(cmd->type, cmd->name, NULL);
  c->args = cpyCmd(cmd->args);
  c->nxt = cpyCmd(cmd->nxt);
  c->valueType = cmd->valueType;
  return c;
}

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
  f->ret.type = UNDEFINED;
  f->cmd = NULL;
  f->args = NULL;
  f->isOperator = 0;
  f->nxt = NULL;
  return f;
}

Attr* appendNewAttr(Attr* arg) {
  if (arg == NULL) {
    return newAttr();
  } else {
    arg->nxt = newAttr();
    return arg->nxt;
  }
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

#ifdef CURSES_MODE
#include <ncurses.h>
#else
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
static SDL_Surface* screen = NULL;
static TTF_Font* font = NULL;
static SDL_Rect position;

void move(int y, int x) {
}

void addch(char ch) {
  int w, h;
  if (ch == '\n' || ch == '\r') {
    char str[] = {'w', '\0'};
    TTF_SizeText(font, str, &w, &h);
    position.y = position.y + h;
    position.x = 0;
  } else {
    SDL_Surface* texte;
    SDL_Color blanc = {255, 255, 255};
    char str[] = {ch, '\0'};
    texte = TTF_RenderText_Blended(font, str, blanc); // FIXME: WTF...
    SDL_BlitSurface(texte, NULL, screen, &position);
    int w, h;
    TTF_SizeText(font, str, &w, &h);
    position.x = position.x + w;
  }
}

void refresh() {
  SDL_Flip(screen);
}

int getch() {
  int unicode;
  SDL_Event event;     
  while (1) {
    SDL_WaitEvent(&event);
    switch (event.type) {
    case SDL_KEYDOWN:
      unicode = event.key.keysym.unicode;
      if ((unicode & 0xFF80) == 0) {
          return unicode & 0x7F;
      }// else {
      //  printf("An International Character.\n");
      //}
      break;
    case SDL_QUIT:
      finish(0);
      break;
    }
  }
}

void deleteln() {
}

void addstr(const char* str) {
  while (*str != '\0') {
    addch(*str);
    ++str;
  }
}
#endif

int getLines() {
#ifdef CURSES_MODE
  return LINES;
#else
  return 30;
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

void dellastch(char ch) {
#ifdef CURSES_MODE
  int y, x;
  getyx(curscr, y, x);
  mvdelch(y, x-1);
#else
  int w, h;
  char str[] = {ch, '\0'};
  TTF_SizeText(font, str, &w, &h);
  position.x = position.x - w;

  SDL_Surface *s;
  s = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
  SDL_FillRect(s, NULL, SDL_MapRGB(s->format, 0, 0, 0));
  SDL_BlitSurface(s, NULL, screen, &position);
#endif
}

char* strVal(char* val, Cmd* cmd) {
  strncpy(val, cmd->name + 1, strlen(cmd->name) - 2);
  return val;
}

/*char* catAttr(char* m, Attr* arg) {
  if (arg != NULL) {
    Attr* n = arg->nxt;
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

// HELPER

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

char* replace(char* str, char a, char b) {
  char* s = str;
  for (;*s != '\0'; ++s) {
    if (*s == a) {
      *s = b;
    }
  }
  return str;
}

char* catVal(char* b, Val* v) {
  if (v == NULL) {
    strcat(b, "NULL");
  } else if (v->type.type == TUPLE) {
    Val* v2 = (Val*)v->addr;
    strcat(b, "(");
    while (v2 != NULL) {
      catVal(b, v2);
      if (v2->nxt != NULL) {
        strcat(b, ", ");
      }
    }
    strcat(b, ")");
  } else if (v->type.type == ERR || (v->type.type == CHAR && v->type.ptr == 1)) {
    strcat(b, (char*)v->addr);
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

char* catPrimVarType(char* b, PrimVarType t) {
  if (t == INT) {
    strcat(b, "int");
  } else if (t == FLOAT) {
    strcat(b, "float");
  } else if (t == CHAR) {
    strcat(b, "char");
  }
  return b;
}

char* catVarType(char* b, VarType t) {
  catPrimVarType(b, t.type);
  int i;
  for (i = 0; i < t.ptr; i++) {
    strcat(b, "*");
  }
  return b;
}

Val* runFunc(Val* args) {
  // TODO, replace all of f->args in f->cmd by cmd->args
  Func* f = funcByName((char*)args->addr);
  LoadedDef* d = loadedFuncByName((char*)((Val*)f->cmd->addr)->addr);
  return (d != NULL) ? d->ptr(args) : NULL;
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
    p = parseBlockAttr(++s);
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
           *s != '{' && *s != '}' && *s != '[' && *s != '\n') {
      ++s;
    }
    if (s > i) {
      Cmd* cmd = newCmd();
      strncpy(cmd->name, i, s-i);
      addCmd(&cmds, cmd);
    }
    s = trim(s);
    if (*s == ')') {
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
    char nName[52] = "";
    strncpy(nName, cmd->name + 1, strlen(cmd->name) - 2);
    return initVal(varType(CHAR, 1, 0), nName);
  } else if (cmd->type == OLD_CHAR) {
    char c = cmd->name[1];
    return initVal(varType(CHAR, 0, 0), &c);
  } else if (cmd->args != NULL) {
    if (cmd->name != NULL && strlen(cmd->name) > 0) {
      Val* v = initVal(varType(CHAR, 1, 0), cmd->name);
      v->nxt = cmdArgsToVal(cmd);
      return initVal(varType(TUPLE, 0, 0), v);
    } else {
      return cmdArgsToVal(cmd);
    }
  } else {
    return initVal(varType(CHAR, 1, 0), cmd->name);
  }
}

Cmd* valToCmd(Val* v) {
  if (v == NULL) return NULL;
  if (v->type.type == CHAR && v->type.ptr == 1) {
    Cmd* cmd = newCmd();
    cmd->type = STRING;
    strcpy(cmd->name, (char*)v->addr);
    return cmd;
  } else if (v->type.type == TUPLE) {
    Cmd* cmd = newCmd();
    Cmd* c = NULL;
    Val* v2 = (Val*)v->addr;
    for (; v2 != NULL; v2 = v2->nxt) {
      if (cmd->args == NULL) {
        cmd->args = valToCmd(v2);
        c = cmd->args;
      } else {
        c->nxt = valToCmd(v2);
        c = c->nxt;
      }
    }
    return cmd;
  } else if (v->type.type == INT) {
    char r[52] = "";
    cat_argint(r, *(int*)v->addr);
    return initCmd(OLD_INT, r, NULL);
  } else {
    return NULL;
  }
}

// FIXME: Free val!!!!

// Reduces a Cmd down to a primitive type.
// If cmd->type == CFUNCTION, call it and replace the cmd by it's result
// If cmd->type == FUNCTION, call it and replace the cmd by it's result
// If cmd->type == VAR, get it's value and replace the cmd by it's result
Val* cmdVal(Cmd* cmd) {
  Val* ret = NULL;
  if (cmd->type == FUNCTION || cmd->type == OPERATOR || cmd->type == CFUNCTION) {
    Val* nVal = initVal(varType(CHAR, 1, 0), cmd->name);
    Val* n = nVal;
    Cmd* arg;
    for (arg = cmd->args; arg != NULL; arg = arg->nxt) {
      n->nxt = cmdVal(arg);
      if (n->nxt != NULL) {
        n = n->nxt;
      }
    }
    ret = loadedFuncByName(cmd->name)->ptr(nVal);
    freeVal(nVal);
  } else if (cmd->type == MACRO || cmd->type == MACRO_OP) {
    Val* nVal = initVal(varType(CHAR, 1, 0), cmd->name);
    Val* n = nVal;
    Cmd* arg;
    for (arg = cmd->args; arg != NULL; arg = arg->nxt) {
      n->nxt = cmdToVal(arg);
      if (n->nxt != NULL) {
        n = n->nxt;
      }
    }
    ret = loadedFuncByName(cmd->name)->ptr(nVal);
  } else if (cmd->type == VAR) {
    ret = cpyVal(varByName(cmd->name)->val);
  } else {
    ret = cmdToVal(cmd);
  }
  return ret;
}

void eval(Cmd* cmd) {
  if (cmd == NULL) return;

  Val* v = cmdVal(cmd);
  if (v == NULL || v->type.type != ERR) {
    output("\n=> ");
  } else {
    output("\nError: ");
  }
  char out[52] = "";
  catVal(out, v);
  output(out);
  freeVal(v);
 
  eval(cmd->nxt);
}


int evalCmd(char* command, char* err) {
  Cmd* cmd = parseCmdR(command).cmd;
  if (cmd == NULL) {
    strcat(err, "\nNull cmd.");
    return 0;
  }
  if (cmd->type == UNKOWN) {
    strcat(err, "\n");
    strcat(err, cmd->name);
    strcat(err, ": undefined variable or function");
  } else {
    eval(cmd);
  }
  freeCmd(cmd);
  return 1;
}

int insertInputCh(char ch, char* input, char* cursor, int *nested) {
  return 1;
}

#ifndef KEY_BACKSPACE
#define KEY_BACKSPACE 8
#endif

#ifndef KEY_DL
#define KEY_DL 127
#endif

char* getInput(char* input) {  
  int nested = 0;
  char *cursor = input;
  while (1) {
    int ch = getch();
    if (ch == 8 || ch == 127 || ch == KEY_BACKSPACE || ch == KEY_DL) {
      if (strlen(input) > 0) {
        dellastch(input[strlen(input)-1]);
        strdelch(input);
        refresh();
        --cursor;
      }
    } else if (ch == '\033') { // if the first value is esc
      getch(); // skip the [
      ch = getch(); // the real value
      if (ch == 'A') { // code for arrow up. Last command.
      } else if (ch == 'B') { // code for arrow down. Undo last command.
      } else if (ch == 'C') { // code for arrow right
      } else if (ch == 'D') { // code for arrow left
        if (cursor > input) {
          cursor--;
        }
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
      strinsertch(cursor, ch);
      cursor++;
    }
  }
  if (nested != 0) {
    //msg("Invalid block syntax.");
    return NULL;
  }
  return input;
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
  Attr* ret;
  Attr* arg;
  Attr* arg2 = NULL;
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
    char vType[52] = "";
    catVarType(vType, f->ret);
    fprintf(s, "%s %s {%s: ", f->name, f->isOperator ? ":::" : "::", vType);
    if (f->args != NULL) {
      fprintf(s, "|");
      Attr* a;
      for (a = f->args; a != NULL; a = a->nxt) {
        char aType[52] = "";
        catVarType(aType, f->ret);
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

void bindCFunctionsHeader(char* fname, CFunc* fs) {
  CFunc* f;

  char filename[52] = "";
  sprintf(filename, "src/bind/%s.h", fname);
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
    fprintf(s, "Val* bind_%s(Val* args);\n", f->name);
  }

  fprintf(s, "#endif // BIND_H");
  fclose(s);
}

char* catArgTypeGetter(char* b, Attr* a, int i) {
  strcat(b, "  ");
  catVarType(b, a->type);
  strcat(b, " ");
  strcat(b, a->name);
  strcat(b, "_ = ");
  if (a->type.ptr != 0) {
    strcat(b, "GET_PTR(");
  } else {
    strcat(b, "GET_VAL(");
  }
  catVarType(b, a->type);
  strcat(b, ", args);\n");
  return b;
}

void bindCFunctionsSource(char* fname, CFunc* fs) {
  CFunc* f;
  Attr* a;

  char filename[52] = "";
  sprintf(filename, "src/bind/%s.c", fname);
  FILE* s = fopen(filename, "w");

  fprintf(s, "#include \"%s.h\"\n\n", fname);

  fprintf(s, "void initCFunctions(LoadedDef* d) {\n");
  for (f = fs; f != NULL; f = f->nxt) {
    fprintf(s, "  addLoadedDef(d, \"%s\", CFUNCTION, bind_%s);\n", f->name, f->name);
  }
  fprintf(s, "}\n\n");

  // In case the header was not defined, that it was just
  // a source file, define the prototype of the function.
  for (f = fs; f != NULL; f = f->nxt) {
    char fRetType[52] = "";
    catVarType(fRetType, f->ret);
    fprintf(s, "%s %s(", fRetType, f->name);
    for (a = f->args; a != NULL; a = a->nxt) {
      char aTypeT[52] = "";
      catVarType(aTypeT, a->type);
      fprintf(s, "%s %s", aTypeT, a->name);
      if (a->nxt != NULL) {
        fprintf(s, ", ");
      }
    }
    fprintf(s, ");\n");
  }
  fprintf(s, "\n");

  /*int r = add(x_, y_);
  return initVal(varType(INT, 0, 0), &r);
}*/

  for (f = fs; f != NULL; f = f->nxt) {
    fprintf(s, "Val* bind_%s(Val* args) {\n", f->name);
    fprintf(s, "  Val* m; VarType t[] = {");
    int nAttrs = 0;
    for (nAttrs = 0, a = f->args; a != NULL; a = a->nxt, nAttrs++) {
      char vType[52] = "";
      catPrimVarTypeEnum(vType, a->type.type);
      fprintf(s, "varType(%s, %d, %d)", vType, a->type.ptr, a->type.arraySize);
      if (a->nxt != NULL) {
        fprintf(s, ", ");
      }
    }
    int i;
    fprintf(s, "};\n");
    fprintf(s, "  if ((m = checkSignature(args, t, %d)) != NULL) return m;\n", nAttrs);
    for (i = 0, a = f->args; a != NULL; a = a->nxt, i++) {
      char argTypeG[124] = "";
      catArgTypeGetter(argTypeG, a, i);
      fprintf(s, "%s", argTypeG);
    }
    if (f->ret.type != VOID) {
      char paType[52] = "";
      catVarType(paType, f->ret);
      fprintf(s, "  %s r = %s(", paType, f->name);
    } else {
      fprintf(s, "  %s(", f->name);
    }
    for (a = f->args; a != NULL; a = a->nxt) {
      fprintf(s, "%s_", a->name);
      if (a->nxt != NULL) {
        fprintf(s, ", ");
      }
    }
    if (f->ret.type != VOID) {
      char paType[52] = "";
      catPrimVarTypeEnum(paType, f->ret.type);
      fprintf(s, ");\n  return initVal(varType(%s, %d, %d), %s);\n}\n\n", paType, f->ret.ptr, f->ret.arraySize,
                              f->ret.ptr != 0 ? "r" : "&r");
    } else {
      fprintf(s, ");\n  return NULL;\n}\n\n");
    }
  }
  fclose(s);
}

CFunc* parseCFunction(char* s0) {
  char* s = s0;
  CFunc* f = newCFunc();
  char* lastSpace = NULL;
  char* lastAttr = s0;
  Attr* a = NULL;
  int nested = 0;
  while (*s != '\0') {
    if (*s == ' ' && lastAttr == s) {
      // discard trailing spaces;
      ++lastAttr;
    } else if (*s == ' ') {
      lastSpace = s;
    } else if (a == NULL && *s == '(') {
      char ret[52] = "";
      strncpy(ret, s0, lastSpace - s0);
      f->ret = parseVarType(ret);
      strncpy(f->name, lastSpace+1, s - (lastSpace+1));
      lastAttr = s+1;
    } else if (*s == ',' || (nested == 1 && *s == ')' && lastAttr != s && lastSpace > lastAttr)) {
      if (a == NULL) {
        f->args = newAttr();
        a = f->args;
      } else {
        a->nxt = newAttr();
        a = a->nxt;
      }
      char aType[52] = "";
      strncpy(aType, lastAttr, lastSpace - lastAttr);
      a->type = parseVarType(aType);
      strncpy(a->name, lastSpace+1, s - (lastSpace+1));
      lastAttr = s+1;
    }
    if (*s == '(') ++nested;
    if (*s == ')') --nested;
    ++s;
  }
  return f;
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
  return cmdToVal(ret);*/
  return NULL;
}

void typeConstructor(Type* type) {
  if (type->attrs != NULL) {
    char constructorName[52] = "#";
    strcat(constructorName, type->name);
    addLoadedDef(loadedDefs, constructorName, FUNCTION, construct);
  }
}

void addType(Type* type) {
  Type* oldFirst = types;
  types = type;
  type->nxt = oldFirst;
  typeConstructor(type);
}

struct CLine {
  char val[200];
  struct CLine* nxt;
  struct CLine* block;
};
typedef struct CLine CLine;

CLine* newCLine() {
  CLine* arg0 = malloc(sizeof(CLine));
  if (arg0 == NULL) {
    abort(); // FIXME: "Can't allocate memory"
  }
  memset(arg0->val, '\0', sizeof(arg0->val));
  arg0->block = NULL;
  arg0->nxt = NULL;
  return arg0;
}

void freeCLine(CLine* s) {
  if (s != NULL) {
    freeCLine(s->nxt);
    freeCLine(s->block);
    free(s);
  }
}

void appendChars(FILE* s, char* str, char* stopAt) {
  char c;
  char* at;
  while ((c = getc(s)) != EOF) {
    if (str != NULL) {
      straddch(str, (c == '\t') ? ' ' : c);
    }
    if (c == '\\') {
      c = getc(s);
      if (str != NULL) {
        straddch(str, (c == '\t') ? ' ' : c);
      }
      if (c == EOF) return;
    } else {
      for (at = stopAt; *at != '\0'; at++) {
        if (c == *at) return;
      }
    }
  }
}

CLine* getCLines(FILE* s, int nested) {
  CLine* lines = newCLine();
  CLine* line = lines;
  char c;
  char p = EOF;
  int nestedP = 0;
  while ((c = getc(s)) != EOF) {
    if (c == '}') {
      if (nested <= 0) {
        //printf("Unexpected end bracket.");
        //return NULL;
      }
      return lines;
    } else if (c == '{') {
      line->block = getCLines(s, nested + 1);
      line->nxt = newCLine();
      line = line->nxt;
    } else if ((c == '\r' || c == '\n') && p != '\\') {
      if (nestedP == 0) {
        if (strlen(line->val) > 0) {
          line->nxt = newCLine();
          line = line->nxt;
        }
      }
    } else if ((c == ' ' || c == '\t') && strlen(line->val) <= 0) { // Discard trailing whitespaces
    } else if ((p == ' ' || p == '\t') && (c == ' ' || c == '\t')) { // Discard double whitespaces
    } else if (p == '/' && c == '/') {
      strdelch(line->val);
      appendChars(s, NULL, "\r\n\0");
      if (nestedP == 0) {
        if (strlen(line->val) > 0) {
          line->nxt = newCLine();
          line = line->nxt;
        }
      }
    } else if (p == '/' && c == '*') {
      strdelch(line->val);
      while ((c = getc(s)) != EOF) {
        if (p == '*' && c == '/') break;
        p = c;
      }
    } else {
      straddch(line->val, (c == '\t') ? ' ' : c);
      if (c == '(') {
        ++nestedP;
      } else if (c == ')') {
        --nestedP;
      } else if (p != '\\' && c == '\'') {
        appendChars(s, line->val, "\'\0");
      } else if (p != '\\' && c == '"') {
        appendChars(s, line->val, "\"\0");
      }
    }
    p = c;
  }

  if (nested > 0) {
    printf("Missing end bracket.");
    return NULL;
  }
  return lines;
}

int startsWith(char* mustEqual, char* str1) {
  return strncmp(mustEqual, str1, strlen(mustEqual)) == 0;
}

FILE* fopenWithExtension(char* extension, char* filename) {
  char buf[64] = "";
  strcat(buf, extension);
  strcat(buf, filename);
  return fopen(buf, "r");
}

Attr* parseAttr(char* val) {
  char* space = strrchr(trimEnd(val), ' ');
  if (space == NULL) return NULL;
  char* bracket = strchr(val, '[');
  Attr* a = newAttr();
  char type[32] = "";
  char array[12] = "";
  int arraySize = 0;
  char* star = strchr(val, '*');
  strncpy(type, val, star == NULL ? space - val : star - val);
  if (bracket) {
    strncpy(array, bracket + 1, strlen(bracket) - 2);
    char* num; // ???
    arraySize = strtol(array, &num, 0);
    strncpy(a->name, space + 1, bracket - space - 1);
  } else {
    strncpy(a->name, space + 1, strlen(space) - 2);
  }
  a->type.arraySize = arraySize;
  a->type = parseVarType(type);
  if (star != NULL) {
    int nStar = 0;
    while (*star == ' ' || *star == '*' || *star == '\t') {
      if (*star == '*') {
        nStar++;
      }
      star++;
    }
    a->type.ptr = nStar;
  }
  if (a->type.type == UNDEFINED) {
    freeAttr(a);
    return NULL;
  }
  return a;
}

Type* parseTypedef(CLine* l) {
  Type* t = newType();
  strcpy(t->name, trim(strrchr(trimCEnd(l->val), ' ')));
  Type* oldFirst = types;
  types = t;
  t->nxt = oldFirst;
  if (l->block != NULL) {
  }
  return t;
}

Type* parseEnum(CLine* l) {
  Type* t = newType();
  strcpy(t->name, trimCEnd(l->val));
  Type* oldFirst = types;
  types = t;
  t->nxt = oldFirst;
  if (l->block != NULL) {
  }
  return t;
}

Type* parseCStruct(CLine* l) {
  Type* t = newType();
  strcpy(t->name, trimCEnd(l->val));
  Type* oldFirst = types;
  types = t;
  t->nxt = oldFirst;
  if (l->block != NULL) {
    t->attrs = parseAttr(l->block->val);
    Attr* a = t->attrs;
    CLine* li;
    for (li = l->block->nxt; li != NULL; li = li->nxt) {
      a->nxt = parseAttr(li->val);
      a = a->nxt;
    }
    if (t->attrs == NULL) {
      types = oldFirst;
      freeType(t);
      return NULL;
    }
  }
  return t;
}

Val* parseCIncludeFile(char* filename) {
  FILE* s = fopen(filename, "r");
  //if (s == NULL) s = fopenWithExtension("/usr/include/", filename);
  if (s == NULL) {
    //return errorStr("Invalid include file.\n"); FIXME: TMP commented because always gives error
    return NULL;
  }

  CLine* lines = getCLines(s, 0);
  CLine* l;
  CFunc* f0 = NULL;
  CFunc* f = NULL;
  for (l = lines; l != NULL; l = l->nxt) {
    if (l->val[0] == '#') { // TODO: All preprocessor directives.
      if (startsWith("#include", l->val)) {
        char filename[64] = "";
        strncpy(filename, l->val + strlen("#include <"), strlen(l->val) - strlen("#include <") - 1);
        Val* secRet = parseCIncludeFile(filename);
        if (secRet != NULL && secRet->type.type == ERR) {
          freeCFunc(f0);
          free(lines);
          return secRet;
        }
      }
    } else if (strchr(l->val, '(')) { // It is a function.
      if (f0 == NULL) {
        f0 = parseCFunction(l->val);
        f = f0;
      } else {
        f->nxt = parseCFunction(l->val);
        f = f->nxt;
      }
    } else if (startsWith("typedef", l->val)) {
      parseTypedef(l);
    } else if (startsWith("enum", l->val)) {
      parseEnum(l);
    } else if (startsWith("struct", l->val)) {
      if (parseCStruct(l) == NULL) {
        freeCFunc(f0);
        free(lines);
        return errorStr("Could not parse struct.");
      }
    }
  }
  char bind_filename[52] = "bind_";
  strcat(bind_filename, filename);
  char* ext;
  if ((ext = strchr(bind_filename, '.')) != NULL) {
    *ext = '\0';
  }
  replace(bind_filename, '/', '_');
  if (f0 != NULL) {
    bindCFunctionsHeader(bind_filename, f0);
    bindCFunctionsSource(bind_filename, f0);
  }
  freeCFunc(f0);
  free(lines);
  return NULL;
}
Val* parseCIncludeFileCmd(Val* args) {
  return parseCIncludeFile((char*)args->nxt->addr);
}

void load() {
  int c;
  FILE* s = fopen("app.qc", "r"); // FIXME: Check if valid file. Not NULL.
  if (s != NULL) {
    char input[512] = "";
    while ((c = getc(s)) != EOF) {
      if (c == '\r' || c == '\n') {
        char err[52] = "";
        if (evalCmd(input, err)) {
          input[0] = '\0';
        } else {
          // FIXME: Throw error.
        }
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
  return buf;
}

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
  f->ret = parseVarType((char*)block->addr);
  Val* arg = block->nxt;
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
  addLoadedDef(loadedDefs, f->name, FUNCTION, runFunc);
  return NULL;
}
Val* defineOp(Val* args) {
  Func* f = createFunc(args);
  f->isOperator = 1;
  addLoadedDef(loadedDefs, f->name, OPERATOR, runFunc);
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
  return v->val;
}

void listTypes(Cmd* cmd) {
  Type* t;
  char m[2056] = "";
  for (t = types; t != NULL; t = t->nxt) {
    strcat(m, t->name);
    if (t->attrs != NULL) {
      strcat(m, " #:: ");
    }
    Attr* a;
    for (a = t->attrs; a != NULL; a = a->nxt) {
      strcat(m, "(");
      catVarType(m, a->type);
      strcat(m, " ");
      strcat(m, a->name);
      strcat(m, ") ");
    }
    if (t->nxt != NULL) {
      strcat(m, "\n");
    }
  }
  output(m);
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
  return initPtr(CHAR, m);
}

Val* listDefs() {
  LoadedDef* d;
  char* m = malloc(sizeof(char) * 2056);
  for (d = loadedDefs; d != NULL; d = d->nxt) {
    strcat(m, d->name);
    if (d->nxt != NULL) {
      strcat(m, ", ");
    }
  }
  return initPtr(CHAR, m);
}

void loop()
{
  int continuer = 1;
  output(">> ");
  while (continuer) {
    char input[124] = "";
    getInput(input);
    if (input[0] == ':') {
      char* name = input + 1;
      if (strcmp(name, "exit") == 0 ||
                 strcmp(name, "quit") == 0 ||
                 strcmp(name, "q") == 0) {
        return;
      } else if (strcmp(name, "h") == 0 ||
                 strcmp(name, "l") == 0 ||
                 strcmp(name, "list") == 0 ||
                 strcmp(name, "help") == 0) {
        output("\n");
        Val* v = listDefs();
        output((char*)v->addr);
        free(v);
      } else if (strcmp(name, "v") == 0 ||
                 strcmp(name, "vars") == 0) {
        output("\n");
        Val* v = listVars();
        output((char*)v->addr);
        free(v);
      } else if (strcmp(name, "t") == 0 ||
                 strcmp(name, "types") == 0) {
        output("\n");
        listTypes(NULL);
      } else if (strcmp(name, "s") == 0 ||
                 strcmp(name, "save") == 0) {
        save();
      } else {
        char err[128] = "\n";
        strcat(err, input);
        strcat(err, ": unkown command");
        output(err);
      }
    } else {
      char err[128] = "";
      if (evalCmd(input, err)) {
        if (strlen(err) > 0) {
          output(err);
        }
      } else {
        return;
      }
    }
    output("\n>> ");
    refresh();
  }
}

void initLoadedDefs() {
  loadedDefs = createLoadedDef("=", MACRO_OP, assign); // Assigns a value to an existing variable.
  addLoadedDef(loadedDefs, "::", MACRO_OP, define); // Assigns a function to a new variable.
  addLoadedDef(loadedDefs, ":::", MACRO_OP, defineOp); // Assigns a function to a new variable.
  addLoadedDef(loadedDefs, "#::", MACRO_OP, defineType); // Assigns a function to a new variable.
  addLoadedDef(loadedDefs, "include", FUNCTION, parseCIncludeFileCmd);
}

static void finish(int sig)
{
#ifdef CURSES_MODE
  endwin();
#else
  SDL_Quit();
  TTF_CloseFont(font);
  TTF_Quit();
#endif

  freeLoadedDef(loadedDefs);
  loadedDefs = NULL;
  freeTypes(types);
  types = NULL;
  freeVar(vars);
  vars = NULL;
  freeFunc(funcs);
  funcs = NULL;

  exit(0);
}

void printCLine(CLine* lines, int nested) {
  CLine* l;
  int i;
  for (l = lines; l != NULL; l = l->nxt) {
    for (i = 0; i < nested; i++) {printf("  ");}
    printf("%s\n", l->val);
    if (l->block != NULL) {
      for (i = 0; i < nested; i++) {printf("  ");}
      printf("{");
      printCLine(l->block, nested + 1);
      printf("\n");
      for (i = 0; i < nested; i++) {printf("  ");}
      printf("}");
    }
    if (nested == 0) {
      printf("\n");
    }
    //printf("    ---------------   \n");
  }
}

#ifdef DEBUG_MODE
int main(int argc, char* argv[])
{
  return 0;
}
#else
int main()
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
  TTF_Init();
  font = TTF_OpenFont("fonts/OpenSans-Regular.ttf", 15);

  if (font == NULL) {
    finish(-1);
  }

  position.x = 0;
  position.y = 0;

  screen = SDL_SetVideoMode(1024, 768, 32, SDL_SWSURFACE);

  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
  SDL_EnableUNICODE(1);
#endif

  loop();

  finish(0);
  return 0;
}
#endif

/*void runCmd(char* retVal, Cmd* cmd) { // FIXME: Fonction dependencies must be added too.
  Func* f = funcByName(cmd->name);
  if (f == NULL) return;
  if (f->args == NULL) return; // Invalid function. Needs return type. FIXME: Better error handling

  Cmd* c;
  Attr* arg;
  Attr* ret;
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
  FILE *fp = popen(cmds, "r"); // TODO: Attrs

  fscanf(fp, "%s", retVal);
  pclose(fp);
}*/

