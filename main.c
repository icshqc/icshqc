#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

#include <ffi.h>

#include "model.h"
#include "src/bind/glue.h"

//#include "bind.h"
#include "src/bind/bind.h"

//#define CURSES_MODE
//#define DEBUG_MODE

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

Cmd* initCmd(CmdType type, const char* val, Cmd* args) {
  Cmd* c = newCmd();
  if (val != NULL) strcpy(c->name, val);
  c->type = type;
  c->args = args;
  return c;
}

Cmd* cpyCmd(Cmd* cmd) {
  if (cmd != NULL) {
    Cmd* c = initCmd(cmd->type, cmd->name, NULL);
    c->args = cpyCmd(cmd->args);
    c->nxt = cpyCmd(cmd->nxt);
    c->valueType = cmd->valueType;
    return c;
  }
}

Attr* newAttr() {
  Attr* a = malloc(sizeof(Attr));
  if (a == NULL) {
    abort(); // FIXME: "Can't allocate memory"
  }
  memset(a->name, '\0', sizeof(a->name));
  a->type.type = NULL;
  a->type.arraySize = 0;
  a->type.ptr = 0;
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

void freeArg(Arg* f) {
  if (f != NULL) {
    freeArg(f->nxt);
    free(f);
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

// HELPER

char* straddch(char* str, char c) { //FIXME: Not buffer safe
  int i = strlen(str);
  str[i] = c;
  str[i+1] = '\0';
  return str;
}

void strinsertch(char* cursor, char c) { //FIXME: Not buffer safe
  char* s = cursor;
  char n = *s;
  char n2;
  while (n != '\0') {
    n2 = *(s+1);
    *(s+1) = n;
    n = n2;
  }
  *cursor = c;
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
  return initCmd(STRING, str, NULL);
}

char* trimCEnd(char* s) {
  char* init = s;
  s = s + strlen(s) - 1;
  while (s != init) {
    if (*s == ' ' || *s == '\t' || *s == ';') {
      *s = '\0';
    } else {
      break;
    }
    --s;
  }
  return init;
}
char* trimEnd(char* s) {
  char* init = s;
  s = s + strlen(s) - 1;
  while (s != init) {
    if (*s == ' ' || *s == '\t') {
      *s = '\0';
    } else {
      break;
    }
    --s;
  }
  return init;
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
  } else if (t == VALUE) {
    strcat(b, "VALUE");
  } else if (t == TUPLE) {
    strcat(b, "TUPLE");
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
  if (cmd->type == ARRAY || cmd->type == TUPLE) {
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

void catVar(char* m, Var* v) {
  if (v != NULL) {
    if (v->type != NULL) {
      strcat(m, v->type->name);
      strcat(m, " ");
    }
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
ParsePair parseArg(char* command) {
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
ParsePair parseCmdR(char* command) { // FIXME: Does not work for "(add 12 12)"
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
      return parsePair(initCmd(TUPLE, NULL, cmds), s);
    } else {
      return parsePair(cmds, s);
    }
  }
  return parsePair(cmds, s);
}
Cmd* parseCmd(char* command) {
  return parseCmdR(command).cmd;
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

Cmd* getInput() {  
  char input[256] = "";
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
  fclose(s);
}

void bindCFunctionsHeader(char* fname, CFunc* fs) {
  CFunc* f;
  Arg* a;

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

Cmd* construct(Cmd* cmd) {
  Type* t = typeByName(cmd->name + 1);
  if (t == NULL) {
    return errorStr("Unkown constructor.");
  }
  Cmd* ret = initCmd(VALUE, NULL, NULL);
  ret->valueType = t;
  if (t->attrs != NULL) {
    addCmd(&ret->args, cpyCmd(cmd->args));
  }
  return ret;
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

Attr* parseVarDefs(CLine* l) {
  if (l != NULL) {
    char* space = strrchr(trimEnd(l->val), ' ');
    if (space == NULL) return NULL;
    char* bracket = strchr(l->val, '[');
    Attr* a = newAttr();
    char type[32] = "";
    char array[12] = "";
    int arraySize = 0;
    char* star = strchr(l->val, '*');
    strncpy(type, l->val, star == NULL ? space - l->val : star - l->val);
    if (bracket) {
      strncpy(array, bracket + 1, strlen(bracket) - 2);
      char* num; // ???
      arraySize = strtol(array, &num, 0);
      strncpy(a->name, space + 1, bracket - space - 1);
    } else {
      strncpy(a->name, space + 1, strlen(space) - 2);
    }
    a->type.arraySize = arraySize;
    a->type.type = typeByName(type);
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
    if (a->type.type == NULL) {
      freeAttr(a);
      return NULL;
    }
    a->nxt = parseVarDefs(l->nxt);
    return a;
  }
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
    t->attrs = parseVarDefs(l->block);
    if (t->attrs == NULL) {
      types = oldFirst;
      freeType(t);
      return NULL;
    }
  }
  return t;
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

Cmd* parseCIncludeFile(char* filename) {
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
        Cmd* secRet = parseCIncludeFile(filename);
        if (secRet != NULL && secRet->type == ERROR) {
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
Cmd* parseCIncludeFileCmd(Cmd* cmd) {
  return parseCIncludeFile(cmd->args->name);
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
Cmd* defineType(Cmd* cmd) { // Type #:: (type name) (type name)
  Type* type = newType();
  strcpy(type->name, cmd->args->name);
  Cmd* c;
  for (c = cmd->args->nxt; c != NULL; c = c->nxt) {
    Attr* a;
    if (type->attrs == NULL) {
      type->attrs = newAttr();
      a = type->attrs;
    } else {
      a->nxt = newAttr();
      a = a->nxt;
    }
    Type* t = typeByName(c->args->name);
    if (t == NULL) {
      freeType(type);
      return errorStr("Unkown arg type.\n");
    }
    a->type.type = t;
    strcpy(a->name, c->args->nxt->name);
  }
  addType(type);
  return NULL;
}

Cmd* assign(Cmd* cmd) {
  Var* v = varByName(cmd->args->name);
  if (v == NULL) {
    v = addNewVar(NULL, cmd->args->name); // FIXME hardcoded type, but maybe var type is useless anyway.
  } else if (v->val != NULL) {
    freeCmd(v->val);
  }
  Cmd* val = cmd->args->nxt;
  v->val = initCmd(val->type, val->name, val->args);
  v->val->nxt = val->nxt;
  return val;
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
      strcat(m, a->type.type->name);
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
  Cmd* arr = initCmd(ARRAY, NULL, NULL);
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
    ret = varByName(cmd->name)->val;
  } else {
    ret = cmd;
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
  }
  if (ret != cmd) freeCmd(ret);
 
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
                   strcmp(name, "l") == 0 ||
                   strcmp(name, "list") == 0 ||
                   strcmp(name, "help") == 0) {
          char m[1024] = "";
          output("\n");
          output(catPrintCmd(m, listDefs(NULL)));
        } else if (strcmp(name, "v") == 0 ||
                   strcmp(name, "vars") == 0) {
          char m[1024] = "";
          output("\n");
          output(catPrintCmd(m, listVars(NULL)));
        } else if (strcmp(name, "t") == 0 ||
                   strcmp(name, "types") == 0) {
          output("\n");
          listTypes(NULL);
        } else if (strcmp(name, "s") == 0 ||
                   strcmp(name, "save") == 0) {
          save();
        } else {
          char err[128] = "\n";
          strcat(err, cmd->name);
          strcat(err, ": unkown command");
          output(err);
        }
      } else if (cmd->type == UNKOWN) {
        char err[128] = "\n";
        strcat(err, cmd->name);
        strcat(err, ": undefined variable or function");
        output(err);
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
  ffi_cif cif;
  ffi_type *args[2];
  void *values[2];
  char b[52] = "";
  char *s = b;
  unsigned char c;
  char* rc;
  
  /* Initialize the argument info vectors */
  args[0] = &ffi_type_pointer;
  args[1] = &ffi_type_uchar;
  values[0] = &s;
  values[1] = &c;
  
  /* Initialize the cif */
  if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 2, &ffi_type_pointer, args) == FFI_OK) {
    strcpy(b, "Hello world!");
    c = 'a';
    ffi_call(&cif, straddch, &rc, values);
    printf("%s", s);
    /* rc now holds the result of the call to puts */
                                                                                                                              
    /* values holds a pointer to the function's arg, so to
    call puts() again all we need to do is change the
    value of s */
    strcpy(b, "This is cool!");
    c = 'b';
    ffi_call(&cif, straddch, &rc, values);
    printf("%s", s);
  }
    
  return 0;
}
#else
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
}
#endif
