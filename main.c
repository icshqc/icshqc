#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

#include "model.h"
#include "src/bind/glue.h"

#include "src/lib.h"

//#include "bind.h"
#include "src/bind/bind.h"

//#define CURSES_MODE
//#define DEBUG_MODE

#ifdef CURSES_MODE
#include <ncurses.h>
#else
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#endif

// x = []

// TODO: Juste garder =, pas ::: et autres. Un operateur est juste des symbols, comme dans haskell.

// TODO: Enlever les fonctions hardcoder comme assign, runFunc, etc... Les mettres dans lib au pire. Qu'il n'y ait plus de fonction qui prennent Cmd en param.

// TODO: Enlever Cmd. Petit par petit en faisant tout dans ce qui est actuellement parseCmdR.

// Version 0.1 == Etre capable de tout programmer le programme lui-meme dans celui-ci.
// Version 0.2 == Deux screen. Une console et une qui affiche les variables et les fonctions.
// TODO: Rouler l'application dans une autre fenetre que l'editeur. Vraiment separer app/editeur.

// MODEL

static Type* types = NULL;

static TypeDef* typedefs = NULL;
// Maybe vars by scope.
static Var* vars = NULL;

static CFunc* cfuncs = NULL;

// TODO: Have a list that contains both the loaded defs and the runtime one.
// They should of type struct LoadedDef and the function passed would call the executable.
static LoadedDef* loadedDefs = NULL;

static unsigned int cursorX = 0;
static unsigned int cursorY = 0;

TypeDef* newTypeDef() {
  TypeDef* a = malloc(sizeof(TypeDef));
  if (a == NULL) {
    abort(); // FIXME: "Can't allocate memory"
  }
  memset(a->name, '\0', sizeof(a->name));
  a->type = varType(0, 0, 0);
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
  arg0->ret.type = 0;
  arg0->args = NULL;
  arg0->nxt = NULL;
  return arg0;
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

void freeTypeDefs(TypeDef* t) {
  if (t != NULL) {
    freeTypeDefs(t->nxt);
    free(t);
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

void freeFunc(Func* f) {
  if (f != NULL) {
    freeVal(f->cmd);
    freeAttr(f->args);
    freeFunc(f->nxt);
    free(f);
  }
}

void freeLoadedDef(LoadedDef* d) {
  if (d != NULL) {
    freeFunc(d->func);
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

static void finish(int sig);

int getX() {
#ifdef CURSES_MODE
  int y, x;
  getyx(curscr, y, x);
  return x;
#else
  return cursorX;
#endif
}

int getY() {
#ifdef CURSES_MODE
  int x,y;
  getyx(curscr,y,x);
  return y;
#else
  return cursorY;
#endif
}

#ifndef CURSES_MODE
static SDL_Surface* screen = NULL;
static TTF_Font* font = NULL;

static int charWidth = 0;
static int charHeight = 0;

// A pointer to an array of array of chars.
static char** screenChars = NULL;
static unsigned int screenLines = 0;

SDL_Rect cursorRect() {
  SDL_Rect p;
  p.x = getX() * charWidth;
  p.y = getY() * charHeight;
  return p;
}

void move(int y, int x) {
  cursorY = y;
  cursorX = x;
}

void drawch(char ch) {
  SDL_Surface* texte;
  SDL_Color blanc = {255, 255, 255};
  SDL_Rect position = cursorRect();
  char str[10] = "";
  if (ch == '\t') {
    // TODO: Add the correct amount of spaces based on cursor position so it's square.
    str[0] = ' ';
    str[1] = ' ';
  } else {
    str[0] = ch;
  }
  texte = TTF_RenderText_Blended(font, str, blanc);
  SDL_BlitSurface(texte, NULL, screen, &position);
}

void addch(char ch) {
  drawch(ch);
  screenChars[cursorY][cursorX] = ch;
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

void clearscreen() {
  SDL_Surface *s = SDL_CreateRGBSurface(0, screen->w, screen->h, 32, 0, 0, 0, 0);
  SDL_FillRect(s, NULL, SDL_MapRGB(s->format, 0, 0, 0));
  SDL_Rect position;
  position.x = 0;
  position.y = 0;
  SDL_BlitSurface(s, NULL, screen, &position);
}

void redraw() {
  clearscreen();
  int i, j;
  char* s;
  int cX = getX();
  int cY = getY();
  for (i = 0; i < screenLines; i++) {
    for (s = screenChars[i], j = 0; *s != '\0'; s++, j++) {
      move(i,j);
      drawch(*s);
    }
  }
  move(cX, cY);
}

void deleteln() {
  int i;
  char* first = screenChars[0];
  for (i = 0; i < screenLines - 1; i++) {
    screenChars[i] = screenChars[i+1];
  }
  screenChars[screenLines-1] = first;
  first[0] = '\0';
  redraw();
}

#endif

int getCols() {
#ifdef CURSES_MODE
  return COLS;
#else
  return screen->w / charWidth;
#endif
}

int getLines() {
#ifdef CURSES_MODE
  return LINES;
#else
  return screenLines;
#endif
}

void printch(char ch) {
  if (ch == '\n' || getX() >= getCols()) {
    if (getY() >= getLines() - 1){
      move(0,0);
      deleteln();
      move(getLines()-1,0);
      refresh();
    } else {
      move(getY()+1, 0);
    }
  }
  if (ch != '\n') {
    addch(ch);
    move(getY(), getX()+1);
  }
}

void printstr(const char* str) {
  while (*str != '\0') {
    printch(*str);
    ++str;
  }
}

void dellastch(char ch) {
  if (getX() > 0) move(getY(), getX()-1);
#ifdef CURSES_MODE
  int y, x;
  getyx(curscr, y, x);
  mvdelch(y, x-1);
#else
  SDL_Surface *s = SDL_CreateRGBSurface(0, charWidth, charHeight, 32, 0, 0, 0, 0);
  SDL_FillRect(s, NULL, SDL_MapRGB(s->format, 0, 0, 0));
  SDL_Rect position = cursorRect();
  SDL_BlitSurface(s, NULL, screen, &position);
#endif
}

// HELPER

// NCURSES HELPER

// TODO: debug(), fatal(), error(), warn(), log()

static int silent = 0;
void output(const char* str) {
  if (silent) return;

  printstr(str);
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
  } else if (t == OLD_MACRO) {
    strcat(b, "OLD_MACRO");
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
  } else if (v->type.type == CHAR && (v->type.ptr == 1 || v->type.arraySize != 0)) {
    strcat(b, (char*)v->addr);
  } else if (v->type.type == TUPLE || v->type.type == STRUCT) {
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

TypeDef* typeDefByName(char* name) {
  TypeDef* t = typedefs;
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
    if (strcmp(d->func->name, name) == 0) {
      return d;
    }
    d = d->nxt;
  }
  return NULL;
}

VarType parseVarType(char* str) { 
  VarType v = varType(0, 0, 0);
  char* s = trim(str);
  char* b = s;
  char typeName[52] = "";
  do {
    ++s;
    if (*s == ' ' || *s == '*' || *s == '\0') {
      char token[64] = "";
      strncpy(token, b, s - b);
      if (strcmp(token, "const") == 0) {
        v.isConst = 1;
      } else if (strcmp(token, "__const") == 0) {
        v.isConst = 1;
      } else if (strcmp(token, "extern") == 0) {
      } else if (strcmp(token, "signed") == 0) {
        v.isUnsigned = -1;
      } else if (strcmp(token, "unsigned") == 0) {
        v.isUnsigned = 1;
      } else if (strlen(typeName) > 0) {
        abort();
      } else {
        strcpy(typeName, token);
        break; // FIXME: Should not break. This stops at "long" for "long unsigned int"
      }
      b = s+1;
    }
  } while (*s != '*' && *s != '\0');
  while (*s != '\0') {
    if (*s == '*') {
      v.ptr++;
    }
    ++s;
  }
  Type* ctype = NULL;
  if (strcmp(typeName, "int") == 0) {
    v.type = INT;
  } else if (strcmp(typeName, "short") == 0) { // FIXME: tmp
    v.type = INT;
  } else if (strcmp(typeName, "long") == 0) { // FIXME: tmp
    v.type = INT;
  } else if (strcmp(typeName, "short int") == 0) { // FIXME: tmp
    v.type = INT;
  } else if (strcmp(typeName, "long int") == 0) { // FIXME: tmp
    v.type = INT;
  } else if (strcmp(typeName, "long long") == 0) { // FIXME: tmp
    v.type = INT;
  } else if (strcmp(typeName, "long long int") == 0) { // FIXME: tmp
    v.type = INT;
  } else if (strcmp(typeName, "double") == 0) { // FIXME: tmp
    v.type = FLOAT;
  } else if (strcmp(typeName, "long double") == 0) { // FIXME: tmp
    v.type = FLOAT;
  } else if (strcmp(typeName, "char") == 0) {
    v.type = CHAR;
  } else if (strcmp(typeName, "float") == 0) {
    v.type = FLOAT;
  } else if (strcmp(typeName, "void") == 0) {
    v.type = VOID;
  } else {
    v.typeStruct = typeByName(types, typeName);
    if (ctype != NULL) {
      v.type = STRUCT;
    } else {
      TypeDef* def = typeDefByName(typeName);
      if (def != NULL) {
        v.type = def->type.type;
        v.ptr += def->type.ptr;
        v.typeStruct = def->type.typeStruct;
        // FIXME: What to do about the other fields???
      } else {
        abort();
      }
    }
  }
  return v;
}

// doubler :: {|int x| x + x}
// fArgs = x
// cmdArgs = x, x
// args = 10
Val* runLambda(Func* f, Val* args) {
  Val* cmd = (Val*)f->cmd->addr;
  Attr* fArgs = f->args;
  LoadedDef* d = loadedFuncByName((char*)cmd->addr);
  Val* nArgs = cpyVal(cmd);
  Val* cmdArgs = (Val*)cmd->nxt->addr;
  Val* a;
  Attr* b;
  Val* c;
  Val* n = nArgs;
  for (b = fArgs, c = args->nxt; b != NULL && c != NULL; b = b->nxt, c = c->nxt) {}
  if (b != NULL || c != NULL) {
    return errorStr("Invalid amount of args supplied.");
  }
  for (a = cmdArgs; a != NULL; a = a->nxt) {
    if (a->options & VAL_VAR) {
      int found = 0;
      for (b = fArgs, c = args->nxt; b != NULL && c != NULL; b = b->nxt, c = c->nxt) {
        if (strcmp((char*)a->addr, b->name) == 0) {
          n->nxt = cpyVal(c);
          n = n->nxt;
          found = 1;
          break;
        }
      }
      if (found == 0) {
        return errorStr("Missing args to runFunc");
      }
    } else {
      n->nxt = cpyVal(a);
      n = n->nxt;
    }
  }

  return (d != NULL) ? d->ptr(nArgs) : NULL;
}

Val* runFunc(Val* args) {
  return runLambda(loadedFuncByName((char*)args->addr)->func, args);
}

int containsChars(char* str, char* chs) {
  for(;*str != '\0'; str++) {
    char* c;
    for(c = chs; *c != '\0'; c++) {
      if (*str == *c) return 1;
    }
  }
  return 0;
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
          if (v->val->options & VAL_BLOCK) {
            if (containsChars(v->name, "abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ")) {
              c->type = FUNCTION;
            } else {
              c->type = OPERATOR;
            }
          } else {
            c->type = VAR;
          }
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
  p = parseCmdR(trim(s));
  if (block->args == NULL) {
    block->args = p.cmd;
  } else {
    Cmd* arg;
    for (arg = block->args; arg->nxt != NULL; arg = arg->nxt) {}
    arg->nxt = p.cmd;
  }
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

void removeComments(char* s) {
  char* ptr = s;
  for (; *s != '\0'; s++, ptr++) {
    if (*s == '/' && *(s+1) == '/') {
      *ptr = '\0';
      return;
    }
    *ptr = *s;
  }
}

ParsePair parseCmdR(char* command) {
  Cmd* cmds = NULL;
  removeComments(command);
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
  } else if (cmds->type == FUNCTION || cmds->type == OLD_MACRO || cmds->type == CFUNCTION) {
    cmds->args = cmds->nxt;
    cmds->nxt = NULL;
  } else {
    if (cmds->nxt != NULL) {
      if (cmds->type == VAR) {
        cmds->args = cmds->nxt;
        cmds->nxt = NULL;
        return parsePair(cmds, s);
      }
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
    Val* v = initArray(varType(CHAR, 0, 52), cmd->name);
    v->options = VAL_UNKOWN;
    v->options = VAL_VAR;
    return v;
  }
}

Func* setFuncLambda(Func* f, Val* block) {
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
  return f;
}

Func* initFunc(Val* args, CmdType type) {
  Func* f = newFunc();
  strcpy(f->name, (char*)args->nxt->addr);
  setFuncLambda(f, (Val*)args->nxt->nxt->addr);
  return f;
}

void addVal(Val** list, Val* v) {
  while (*list != NULL) {
    ++list;
  }
  *list = v;
}

void addValOption(Val* v, enum ValOptions option) {
  if (v != NULL) {
    v->options = v->options | option;
  }
}

Val* cmdVal(Cmd* cmd);
Val* cmdVals(Cmd* cmd) {
  Val* nVal = NULL;
  nVal = initArray(varType(CHAR, 0, 52), cmd->name);
  Val* n = nVal;
  Cmd* arg;
  for (arg = cmd->args; arg != NULL; arg = arg->nxt) {
    if (strcmp(cmd->name, "=") == 0 && n == nVal) {
      n->nxt = cmdToVal(arg);
    } else {
    n->nxt = cmdVal(arg);
    }
    if (n->nxt != NULL) {
      n = n->nxt;
    }
  }
  return nVal;
}

// Reduces a Cmd down to a primitive type.
// If cmd->type == CFUNCTION, call it and replace the cmd by it's result
// If cmd->type == FUNCTION, call it and replace the cmd by it's result
// If cmd->type == VAR, get it's value and replace the cmd by it's result
Val* cmdVal(Cmd* cmd) {
  Val* ret = NULL;
  Val* nVal = NULL;
  if (cmd->type == FUNCTION || cmd->type == OPERATOR || cmd->type == CFUNCTION) {
    nVal = cmdVals(cmd);
    LoadedDef* func = loadedFuncByName(cmd->name);
    if (func == NULL) {
      ret = varByName(cmd->name)->val;
      Val* args = cmdVals(cmd);
      Func* f = setFuncLambda(newFunc(), (Val*)ret->addr);
      ret = runLambda(f, args);
      freeFunc(f);
    } else {
      if (cmd->type == CFUNCTION) {
        ret = checkSignatureAttrs(nVal->nxt, func->func->args);
      }
      if (ret == NULL) ret = func->ptr(nVal);
      addValOption(ret, VAL_LOCAL);
    }
  } else if (cmd->type == OLD_MACRO || cmd->type == MACRO_OP) {
    nVal = initArray(varType(CHAR, 0, 52), cmd->name);
    Val* n = nVal;
    Cmd* arg;
    for (arg = cmd->args; arg != NULL; arg = arg->nxt) {
      n->nxt = cmdToVal(arg);
      // n->nxt = initArray(varType(CHAR, 0, 52), arg->name); FIXME
      if (n->nxt != NULL) {
        n = n->nxt;
      }
    }
    ret = loadedFuncByName(cmd->name)->ptr(nVal);
    addValOption(ret, VAL_LOCAL);
  } else if (cmd->type == VAR) {
    ret = varByName(cmd->name)->val;
    addValOption(ret, VAL_VAR);
  } else if (cmd->type == BLOCK) {
    ret = cmdToVal(cmd);
    addValOption(ret, VAL_LOCAL);
    addValOption(ret, VAL_BLOCK);
  } else {
    ret = cmdToVal(cmd);
    if (cmd->type == UNKOWN) {
      addValOption(ret, VAL_VAR);
    }
    addValOption(ret, VAL_LOCAL);
  }
  return ret;
}

void eval(Cmd* cmd) {
  if (cmd == NULL) return;

  Val* v = cmdVal(cmd);
  if (v != NULL && v->options & VAL_ERROR) {
    output("\nError: ");
  } else {
    output("\n=> ");
  }
  char out[52] = "";
  catVal(out, v);
  output(out);
  while (v != NULL) {
    Val* n = v->nxt;
    if (v->options & VAL_LOCAL) {
      freeVal(v);
    }
    v = n;
  }
 
  eval(cmd->nxt);
}


int evalCmd(char* command, char* err) {
  if (strlen(command) <= 0) return 2;
  Cmd* cmd = parseCmdR(command).cmd;
  if (cmd == NULL) {
    //strcat(err, "\nNull cmd.");
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
          output("  ");
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
      char out[] = {ch, '\0'};
      output(out);
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
  LoadedDef* d;
  for (d = loadedDefs; d != NULL; d = d->nxt) {
    Func* f = d->func;
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

void bindHeader(char* fname, CFunc* fs) {
  CFunc* f;

  char filename[52] = "";
  sprintf(filename, "src/bind/bind.h");
  FILE* s = fopen(filename, "w");

  fprintf(s, "#ifndef BIND_H\n");
  fprintf(s, "#define BIND_H\n\n");
  fprintf(s, "#include <stdlib.h>\n"); // FIXME: Use include given
  fprintf(s, "#include <stdio.h>\n");
  fprintf(s, "#include <signal.h>\n");
  fprintf(s, "#include <string.h>\n\n");
  fprintf(s, "#include \"../../model.h\"\n");
  fprintf(s, "#include \"glue.h\"\n\n");

  fprintf(s, "void initCFunctions(LoadedDef* d, Type* t);\n\n");

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

void catCreateFuncAttr(char* b, Attr* a) {
  if (a == NULL) {
    strcat(b, "NULL");
  } else {
    strcat(b, "createAttr(\"");
    strcat(b, a->name);
    strcat(b, "\", varType(");
    catPrimVarTypeEnum(b, a->type.type);
    char varType[52] = "";
    sprintf(varType, ", %d, %d), ", a->type.ptr, a->type.arraySize);
    strcat(b, varType);
    catCreateFuncAttr(b, a->nxt);
    strcat(b, ")");
  }
}

void bindSource(char* originalName, char* fname, CFunc* fs) {
  CFunc* f;
  Attr* a;

  char filename[52] = "";
  sprintf(filename, "src/bind/bind.c");
  FILE* s = fopen(filename, "w");

  fprintf(s, "#include \"bind.h\"\n\n");

  fprintf(s, "#include \"%s\"\n\n", originalName);

  fprintf(s, "void initCFunctions(LoadedDef* d, Type* t) {\n");
  for (f = fs; f != NULL; f = f->nxt) {
    char attrs[248] = "";
    catCreateFuncAttr(attrs, f->args);
    fprintf(s, "  addLoadedDef(d, createFunc(\"%s\", %s), CFUNCTION, bind_%s);\n", f->name, attrs, f->name);
  }
  fprintf(s, "}\n\n");

  // In case the header was not defined, that it was just
  // a source file, define the prototype of the function.
  /*for (f = fs; f != NULL; f = f->nxt) {
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
  fprintf(s, "\n");*/

  for (f = fs; f != NULL; f = f->nxt) {
    fprintf(s, "Val* bind_%s(Val* args) {\n", f->name);
    int i;
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
      if (f->ret.ptr != 0) {
        fprintf(s, ");\n  return initPtr(varType(%s, %d, %d), r);\n}\n\n", paType, f->ret.ptr, f->ret.arraySize);
      } else {
        fprintf(s, ");\n  return initVal(varType(%s, %d, %d), &r);\n}\n\n", paType, f->ret.ptr, f->ret.arraySize);
      }
    } else {
      fprintf(s, ");\n  return NULL;\n}\n\n");
    }
  }
  fclose(s);
}

Attr* parseAttrs(char* l, char seperator, char delimiter_opposite, char delimiter, unsigned int includeLast) {
  char* s = l;
  char* lastAttr = s;
  Attr* attrs = NULL;
  Attr* a = NULL;
  char* lastSpace = NULL;
  int delimiter_count = 1;
  for (; *s != '\0'; s++) {
    if (*(s-1) == delimiter && delimiter_count == 0) return attrs;

    if (*s == delimiter) {
      delimiter_count--;
    } else if (*s == delimiter_opposite) {
      delimiter_count++;
    }

    if (*s == seperator || (*s == delimiter && delimiter_count == 0)) {
      if (*s == delimiter && includeLast == 0) return attrs;
      if (a == NULL) {
        attrs = newAttr();
        a = attrs;
      } else {
        a->nxt = newAttr();
        a = a->nxt;
      }
      char aType[52] = "";
      if (lastSpace == NULL) { // f(int, int)
        strncpy(aType, lastAttr, s - lastAttr);
        a->type = parseVarType(aType);
        lastAttr = trim(s+1);
      } else {
        strncpy(aType, lastAttr, lastSpace - lastAttr + 1);
        trimEnd(aType);
        a->type = parseVarType(aType);
        strncpy(a->name, lastSpace+1, s - (lastSpace+1));
        lastAttr = trim(s+1);
        lastSpace = NULL;
      }
    } else if ((*s == ' ' || *s == '*') && s >= lastAttr) {
      lastSpace = s;
    }
  }
  return attrs;
}

char* lastSpaceOrStar(char* str) {
  char* space = strrchr(str, ' ');
  char* star = strrchr(str, '*');
  return space == NULL ? star : (star > space ? star : space);
}

CFunc* parseCFunction(char* s0) {
  char* argStart = strchr(s0, '(');
  if (argStart == NULL) return NULL;
  
  char retAndFname[128] = "";
  strncpy(retAndFname, s0, argStart - s0);
  trimEnd(retAndFname);

  // TODO: use lastSpaceOrStar when it's working again.
  char* space = lastSpaceOrStar(retAndFname);
  if (space == NULL) {
    return NULL;
  }

  CFunc* f = newCFunc();

  char ret[52] = "";
  strncpy(ret, retAndFname, space - retAndFname + 1);
  trimEnd(ret);
  f->ret = parseVarType(ret);
  strcpy(f->name, space + 1);

  f->args = parseAttrs(argStart + 1, ',', '(', ')', 1);

  return f;
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
  return initVal(varType(STRUCT, 0, 0), args->nxt);
}

Attr* copyAttrs(Attr* attrs) {
  if (attrs == NULL) return NULL;
  Attr* a = newAttr();
  strcpy(a->name, attrs->name);
  a->type = attrs->type;
  a->nxt = copyAttrs(attrs->nxt);
  return a;
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

struct CLine {
  char val[512];
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

Type* parseCStruct(char* l) {
  // Because the arithmetic of array size can be complicated
  if (strchr(l, '[') != NULL) return NULL; 
  Type* t = newType();
  char* defStart = strchr(l, '{');
  if (defStart) {
    strncpy(t->name, l, defStart-l);
    t->attrs = parseAttrs(defStart + 1, ';', '{', '}', 0);
  } else {
    strcpy(t->name, l);
  }
  trimEnd(t->name);

  addType(t);
  return t;
}

TypeDef* parseTypedef(char* line) {
  TypeDef* t = newTypeDef();
  char* tName = trim(lastSpaceOrStar(trimCEnd(line)) + 1);
  strcpy(t->name, tName);

  char* space = strchr(line, ' ') + 1; // skip the "typedef " part
  if (startsWith("struct", space)) {
    parseCStruct(space);
    t->type = varType(STRUCT, 0, 0);
  } else {
    char type[64] = "";
    strncpy(type, space, tName - space);
    trimEnd(type);
    t->type = parseVarType(type);
  }
  // TODO: Type attributes.

  TypeDef* oldFirst = typedefs;
  typedefs = t;
  t->nxt = oldFirst;
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

int debug(char* str) {
  int x = 0;
  return x;
}

Val* parseCFile(char* filename) {
  FILE* s = fopen(filename, "r");
  if (s == NULL) {
    char msg[52] = "";
    sprintf(msg, "Invalid include file %s.\n", filename);
    return errorStr(msg);
  }

  char line[10000] = "";
  char c;
  char* lastSpace = line;
  int nested = 0;
  while ((c = getc(s)) != EOF) {
    if (strlen(line) >= 9980) {
      abort();
    }
    if (startsWith("__extension__", lastSpace)) {
      *lastSpace = '\0';
    } else if (startsWith("__attribute__", lastSpace)) {
      int nestedP = 0;
      while ((c = getc(s)) != EOF) {
        if (c == '(') {
          nestedP++;
        } else if (c == ')') {
          nestedP--;
          if (nestedP == 0) {
            break;
          }
        }
      }
      *lastSpace = '\0';
      if (c == EOF) break;
    } else if ((c == ' ' || c == '\n' || c == '\t') && strlen(line) <= 0) {
      // Discard trailing whitespaces
    } else if (c == ';' && nested == 0) {
      if (startsWith("typedef", line)) {
        parseTypedef(line);
      } else if (startsWith("enum", line)) {
      //  parseEnum(l);
      } else if (startsWith("struct", line)) {
        parseCStruct(line);
      } else if (startsWith("union", line)) {
      } else if (strchr(line, '(')) { // It is a function.
        if (cfuncs == NULL) {
          cfuncs = parseCFunction(line);
        } else {
          CFunc* oldFirst = cfuncs;
          cfuncs = parseCFunction(line);
          if (cfuncs == NULL) {
            cfuncs = oldFirst;
          } else {
            cfuncs->nxt = oldFirst;
          }
        }
      }
      memset(line, '\0', sizeof(line));
      lastSpace = line;
    } else {
      if (c == '{') {
        nested++;
      } else if (c == '}') {
        nested--;
      }
      straddch(line, c);
      if (c == ' ') {
        lastSpace = line + strlen(line);
      }
    }
  }
  fclose(s);

  if (nested > 0) {
    abort();
    //return errorStr("Missing end bracket.");
  }
  return NULL;
}

Val* parseCIncludeFile(char* filename) {
  char fname[64] = "";
  strcpy(fname, filename);
  replace(fname, '/', '_');
  char tmpFilename[64] = "";
  sprintf(tmpFilename, "tmp/%s", fname);
  char cmds[256] = "";
  strcat(cmds, "gcc -P -E ");
  strcat(cmds, filename);
  strcat(cmds, " > ");
  strcat(cmds, tmpFilename);
  FILE *fp = popen(cmds, "r");
  //fscanf(fp, "%s", retVal);
  pclose(fp);

  return parseCFile(tmpFilename);
}
Val* includeFileCmd(Val* args) {
  return parseCIncludeFile((char*)args->nxt->addr);
}
Val* bindFileCmd(Val* args) {
  char* filename = (char*)args->nxt->addr;
  Val* v = parseCIncludeFile(filename);
  if (cfuncs != NULL) {
    char bind_filename[52] = "bind_";
    strcat(bind_filename, filename);
    char* ext;
    if ((ext = strchr(bind_filename, '.')) != NULL) {
      *ext = '\0';
    }
    replace(bind_filename, '/', '_');
    bindHeader(bind_filename, cfuncs);
    char filepath[128] = "";
    if (filename[0] == '/') {
      strcpy(filepath, filename);
    } else {
      strcat(filepath, "../../");
      strcat(filepath, filename);
    }
    bindSource(filepath, bind_filename, cfuncs);
  }
  freeCFunc(cfuncs);
  cfuncs = NULL;
  return v;
}

void load() {
  int c;
  FILE* s = fopen("app.qc", "r"); // FIXME: Check if valid file. Not NULL.
  if (s != NULL) {
    char input[512] = "";
    while ((c = getc(s)) != EOF) {
      if (c == '\r' || c == '\n') {
        char err[52] = "";
        evalCmd(input, err);
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
    Type* t = typeByName(types, (char*)v2->addr);
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
  v->val->options &= ~VAL_LOCAL;
  return cpyVal(v->val);
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
  TypeDef* d;
  for (d = typedefs; d != NULL; d = d->nxt) {
    strcat(m, "\n");
    strcat(m, d->name);
    strcat(m, " ::== ");
    catVarType(m, d->type);
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
  return initPtr(varType(CHAR, 0, 2056), m);
}

Val* descFunc(LoadedDef* d) {
  if (d == NULL) return errorStr("It is not a valid func.");
  char* m = malloc(sizeof(char) * 512);
  //catVarType(d->func->ret);
  //strcat(m, " ");
  strcat(m, d->func->name);
  strcat(m, "(");
  Attr* a;
  for (a = d->func->args; a != NULL; a = a ->nxt) {
    catVarType(m, a->type);
    strcat(m, " ");
    strcat(m, a->name);
    if (a->nxt != NULL) {
      strcat(m, ", ");
    }
  }
  strcat(m, ")");
  return initPtr(varType(CHAR, 0, 512), m);
}

Val* listDefs() {
  LoadedDef* d;
  char* m = malloc(sizeof(char) * 2056);
  for (d = loadedDefs; d != NULL; d = d->nxt) {
    strcat(m, d->func->name);
    if (d->nxt != NULL) {
      strcat(m, ", ");
    }
  }
  return initPtr(varType(CHAR, 0, 2056), m);
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
      output("\n");
      if (strcmp(name, "exit") == 0 ||
                 strcmp(name, "quit") == 0 ||
                 strcmp(name, "q") == 0) {
        return;
      } else if (strcmp(name, "h") == 0 ||
                 strcmp(name, "help") == 0) {
        char b[] = ":h\t           This help message\n"
                   ":l\t           Lists all the loaded functions\n"
                   ":v\t           Lists all the variables\n"
                   ":t\t           Lists all the types\n"
                   ":d\t           Shows the prototype of a function";
        output(b);
      } else if (strcmp(name, "l") == 0 ||
                 strcmp(name, "list") == 0) {
        Val* v = listDefs();
        output((char*)v->addr);
        free(v);
      } else if (strcmp(name, "v") == 0 ||
                 strcmp(name, "vars") == 0) {
        Val* v = listVars();
        output((char*)v->addr);
        free(v);
      } else if (strcmp(name, "t") == 0 ||
                 strcmp(name, "types") == 0) {
        listTypes(NULL);
      } else if (startsWith("d", name)) {
        if (strlen(name) <= 2) {
          output("You need to specify a function name.");
        } else {
          LoadedDef* d = loadedFuncByName(name + 2);
          if (d == NULL) {
            output("Unkown function.");
          } else {
            Val* v = descFunc(d);
            output((char*)v->addr);
          }
        }
      } else if (strcmp(name, "s") == 0 ||
                 strcmp(name, "save") == 0) {
        save();
      } else {
        char err[128] = "";
        strcat(err, input);
        strcat(err, ": unkown command");
        output(err);
      }
    } else {
      char err[128] = "";
      evalCmd(input, err);
      if (strlen(err) > 0) {
        output(err);
      }
    }
    output("\n>> ");
    refresh();
  }
}

void initLoadedDefs() {
  // FIXME: Those types are not right.

  // Assigns a value to an existing variable.
  loadedDefs = createLoadedDef(createFunc("=", createAttr("name", varType(CHAR, 0, 52),
                                               createAttr("val", varType(CHAR, 0, 52), NULL))), OPERATOR, assign); 

  addLoadedDef(loadedDefs, createFunc("#::", createAttr("name", varType(CHAR, 0, 52),
                                             createAttr("val", varType(CHAR, 0, 52), NULL))), MACRO_OP, defineType); 

  addLoadedDef(loadedDefs, createFunc("include", createAttr("includeFile", varType(CHAR, 0, 52), NULL)),
                                      FUNCTION, includeFileCmd);
  
  addLoadedDef(loadedDefs, createFunc("bind", createAttr("includeFile", varType(CHAR, 0, 52), NULL)),
                                      FUNCTION, bindFileCmd);
}

static void finish(int sig)
{
#ifdef CURSES_MODE
  endwin();
#else
  SDL_Quit();
  TTF_CloseFont(font);
  TTF_Quit();
 
  int i;
  for(i = 0; i < screenLines; i++) {
    free(screenChars[i]);
  }
  free(screenChars);
  screenChars = NULL;
#endif

  freeTypeDefs(typedefs);
  typedefs = NULL;
  freeLoadedDef(loadedDefs);
  loadedDefs = NULL;
  freeTypes(types);
  types = NULL;
  freeVar(vars);
  vars = NULL;

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
  initCFunctions(loadedDefs, types);

  silent = 1;
  load();
  silent = 0;

#ifdef CURSES_MODE
  initscr();
  keypad(stdscr, TRUE);
  nonl();         /* tell curses not to do NL->CR/NL on output */
  cbreak();       /* take input chars one at a time, no wait for \n */
  noecho();       /* dont echo the input char */

  // If idlok is called with TRUE as second argument, curses considers using the hardware insert/delete line feature of terminals so equipped. Calling idlok with FALSE as second argument disables use of line insertion and deletion. This option should be enabled only if the application needs insert/delete line, for example, for a screen editor. It is disabled by default because insert/delete line tends to be visually annoying when used in applications where it is not really needed. If insert/delete line cannot be used, curses redraws the changed portions of all lines.
  //idlok(curscr, 1);

  //scrollok(curscr, 1);
  
  // The setscrreg and wsetscrreg routines allow the application programmer to set a software scrolling region in a window. top and bot are the line numbers of the top and bottom margin of the scrolling region. (Line 0 is the top line of the window.) If this option and scrollok are enabled, an attempt to move off the bottom margin line causes all lines in the scrolling region to scroll one line in the direction of the first line. Only the text of the window is scrolled. (Note that this has nothing to do with the use of a physical scrolling region capability in the terminal, like that in the VT100. If idlok is enabled and the terminal has either a scrolling region or insert/delete line capability, they will probably be used by the output routines.)
#else
  SDL_Init( SDL_INIT_EVERYTHING );
  TTF_Init();

  // Only monospaced fonts are supported for now.
  font = TTF_OpenFont("fonts/DroidSansMono.ttf", 15);

  if (font == NULL) {
    finish(-1);
  }

  char str[] = {'w', '\0'};
  TTF_SizeText(font, str, &charWidth, &charHeight);

  screen = SDL_SetVideoMode(1024, 720, 32, SDL_SWSURFACE);

  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
  SDL_EnableUNICODE(1);

  screenLines = screen->h / charHeight;
  screenChars = malloc(sizeof(char*)*screenLines);
  int i;
  for (i = 0; i < screenLines; i++) {
    size_t lineSize = sizeof(char)*((screen->w / charWidth) + 1);
    screenChars[i] = malloc(lineSize);
    memset(screenChars[i], '\0', lineSize);
  }
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

