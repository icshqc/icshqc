#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <signal.h>
#include <string.h>

#include "model.h"
#include "glue.h"

#include "bind.h"

// TODO: Assign values to variables. int, char and string

// MODEL

static const int MSG_CONSOLE_SIZE = 10;
static const char DEF_FILE_PATH[] = "defs";

static Type* types = NULL;
// Maybe vars by scope.
static Var* vars = NULL;

// TODO: Have a list that contains both the loaded defs and the runtime one.
// They should of type struct LoadedDef and the function passed would call the executable.
static LoadedDef* loadedDefs = NULL;

int isCValue(Cmd* cmd) {
  return cmd->name[0] == '#';
}

// A block is a Cmd with two args. The first is the args, the second is the body
static const char BLOCK[] = "BLOCK";

Cmd* newCmd() {
  Cmd* arg0 = malloc(sizeof(Cmd));
  if (arg0 == NULL) {
    abort(); // FIXME: "Can't allocate memory"
  }
  memset(arg0->name, '\0', sizeof(arg0->name));
  arg0->nxt = NULL;
  arg0->args = NULL;
  arg0->body = NULL;
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
    freeCmd(arg->body);
  }
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
  if (v->val != NULL) {
    freeCmd(v->val);
  } else {
    v->val = newCmd();
    strcpy(v->val->name, val->name);
    v->val->args = val->args;
    v->val->nxt = val->nxt;
    v->val->body = val->body;
  }
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
static int indent = 0;
static int msgLine = 0;
void msg(const char* str) {
  int y, x, i;
  int line = LINES - MSG_CONSOLE_SIZE + msgLine;
  msgLine = (msgLine + 1) % MSG_CONSOLE_SIZE;
  for (i=indent; i<COLS; i++) {
    mvdelch(line,i);
  }
  getyx(curscr, y, x);
  move(line, indent);
  addstr(str);
  move(y, x);
  refresh();
}

static int silent = false;
void output(const char* str) {
  if (!silent) {
    addstr(str);
    refresh();
  }
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

char* catCmd(char* b, Cmd* cmd) {
  if (cmd != NULL) {
    if (cmd->args != NULL) {
      strcat(b,"(");
      strcat(b,cmd->name);
      Cmd* a;
      for (a = cmd->args; a != NULL; a = a->nxt) {
        strcat(b," ");
        catCmd(b, a);
      }
      strcat(b,")");
    } else {
      strcat(b,cmd->name);
    }
    if (cmd->nxt != NULL) {
      strcat(b, " ");
      catCmd(b, cmd->nxt);
    }
  }
  return b;
}
void printCmd(Cmd* cmd) {
  char b[1024] = "";
  catCmd(b, cmd);
  output(b);
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
ParsePair parseCmdR(char* command);
ParsePair parseBlock(char* command) {
  char* s = trim(command);
  Cmd* block = newCmd();
  Cmd* arg = NULL;
  if (*s == '|') { // parse args
    char* i = ++s;
    while (true) {
      if (*s == '\0') {
        msg("Error parsing block. Missing end args pipe.");
        freeCmd(block);
        return parsePair(NULL, s);
      } else if (*s == ',' || *s == '|') {
        if (arg == NULL) {
          block->args = newCmd();
          arg = block->args;
        } else {
          arg->nxt = newCmd();
          arg = arg->nxt;
        }
        strncpy(arg->name, i, s-i);
        if (*s == '|') {
          break;
        }
      }
      ++s;
    }
  }
  ParsePair p = parseCmdR(s);
  s = p.ptr;
  if (*s != '}') {
    msg("Error parsing block. Missing end bracket.");
    freeCmd(block);
    return parsePair(NULL, s);
  }
  block->body = p.cmd;
  return parsePair(block, ++s);
}
ParsePair parseCmdR(char* command) { // FIXME: Does not work for "(add 12 12)"
  Cmd* cmds = newCmd();
  Cmd* cmd = cmds;
  char* s = trim(command);
  while (*s != '\0') {
    char* i = s;
    while (*s != '\0' && *s != ' ' && *s != '(' && *s != ')' &&
           *s != '{' && *s != '}' && *s != '#' && *s != '\n') {
      ++s;
    }
    strncpy(cmd->name, i, s-i);
    s = trim(s);
    if (*s == ')') {
      ++s;
      break;
    } else if (*s == '}') {
      break;
    } else if (*s == '#') {
      char* i = s; // Keep the '#' on purpose.
      while (*(++s) != '#' && *s != '\0') {
      }
      cmd->nxt = newCmd();
      cmd = cmd->nxt;
      strncpy(cmd->name, i, s-i);
      ++s;
      if (*s != '\0') {
        cmd->nxt = newCmd();
        cmd = cmd->nxt;
      }
    } else if (*s == '{') {
      ParsePair p = parseBlock(s+1);
      if (p.cmd != NULL) {
        s = p.ptr;
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
  LoadedDef* f;
  if (cmds->nxt != NULL && (f = loadedFuncByName(cmds->nxt->name)) != NULL && f->isOperator == true) {
    cmd = cmds;
    cmds = cmds->nxt;
    cmds->args = cmd;
    cmd->nxt = cmds->nxt;
    cmds->nxt = NULL;
  } else if ((f = loadedFuncByName(cmds->name)) != NULL && f->isOperator == false) {
    cmds->args = cmds->nxt;
    cmds->nxt = NULL;
  } else if (strlen(cmds->name) > 0) {
    // FIXME: TMP because the funcByName should return the loaded functions too.
    msg("Error parsing. Is not a function");
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
  while (true) {
    int ch = getch();
    // FIXME: KEY_BACKSPACE and KEY_DC does not work.
    if (ch == KEY_BACKSPACE || ch == KEY_DC || ch == 8 || ch == 127) {
      if (strlen(input) > 0) {
        strdelch(input);
        getyx(curscr, y, x);
        mvdelch(y, x-1);
        refresh();
      }
    } else if (ch == '\n' || ch == '\r') {
      if (nested > 0) {
        straddch(input, ' '); // Treat as whitespace maybe???
        addch('\n');
        int i;
        for (i = 0; i < nested; i++) {
          addch(' ');
          addch(' ');
        }
      } else {
        break;
      }
    } else {
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
    msg("Invalid block syntax.");
    return NULL;
  }
  return parseCmd(input);
}

/*Lambda* parseLambda(char* s) {
  Lambda* l = malloc(sizeof(Lambda)); // newLambda...
  if (l == NULL) {
    abort();
  }
  l->args = NULL;
  memset(l->body, '\0', (sizeof l->body));
  char* c = trim(s);
  if (*c != '\\' || strlen(c) < 3) {
    msg("Unable to parse lambda");
    return NULL;
  }
  c = trim(c+1);
  char* i = c;
  Arg* arg = NULL;
  while (!(*c == '-' && *(c+1) == '>')) {
    if (*(c+1) == '\0') {
      msg("Unable to parse lambda. Missing value.");
    } else if (*c == ' ') {
      if (i == c) {
        i++;
      } else {
        arg = appendNewArg(arg);
        if (l->args == NULL) {
          l->args = arg;
        }
        strncpy(arg->val, i, c - i);
        i = c + 1;
      }
    } else if (*c < 'a' || *c > 'z') {
      msg("Unable to parse lambda. Invalid arg name.");
    }
    c++;
  }
  strcpy(l->body, trim(c+2));
  return l;
}*/

int isOperator(char* opName) {
  if (strcmp(opName, "::") == 0 ||
      strcmp(opName, "=") == 0) {
    return true; 
  }
  return false;
}

/*ArgTree* sortCmd(Arg* arg) {
  if (arg == NULL) return NULL;
  ArgTree* t = newArgTree();
  if (arg->nxt == NULL) {
    t->arg = arg;
  } else if (isOperator(arg->nxt->val)) {
    t->arg = arg->nxt;
    t->child = arg;
    arg->nxt = arg->nxt->nxt;
  } else {
    t->arg = arg;
  }
  return t;
}*/

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

void save(Cmd* cmd) { // .qc extension. Quick C, Quebec!!!
  /*Func* f = NULL;
  FILE* s = fopen("app.qc", "w"); // FIXME: Check if valid file. Not NULL.
  char m[1024] = "";
  for (f = defs; f != NULL; f = f->nxt) {
    m[0] = '\0';
    fprintf(s, "\n%s", catFunc(m, f));
  }
  fclose(s);*/
}

void debug() {}

void bindCFunctionsHeader(CFunc* fs) {
  CFunc* f;
  Arg* a;

  FILE* s = fopen("tmp/bind.h", "w");

  fprintf(s, "#ifndef BIND_H\n");
  fprintf(s, "#define BIND_H\n\n");
  fprintf(s, "#include <stdlib.h>\n"); // FIXME: Use include given
  fprintf(s, "#include <stdio.h>\n");
  fprintf(s, "#include <ncurses.h>\n");
  fprintf(s, "#include <signal.h>\n");
  fprintf(s, "#include <string.h>\n\n");
  fprintf(s, "#include \"glue.h\"\n\n");

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
    fprintf(s, "void bind_%s(Cmd* cmd);\n", f->name);
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

void bindCFunctionsSource(CFunc* fs) {
  CFunc* f;
  Arg* a;

  FILE* s = fopen("tmp/bind.c", "w");

  fprintf(s, "#include \"bind.h\"\n\n");

  for (f = fs; f != NULL; f = f->nxt) {
    fprintf(s, "void bind_%s(Cmd* cmd) {\n", f->name);
    fprintf(s, "  Cmd* args = cmd->args;\n");
    for (a = f->args; a != NULL; a = a->nxt) {
      char argTypeFuncName[52] = "";
      strcpy(argTypeFuncName, a->type);
      fprintf(s, "  %s %s0 = %s(&args);\n", a->type, a->name, argTypeFunc(argTypeFuncName));
    }
    if (strlen(f->ret) > 0) {
      fprintf(s, "  %s ret = %s(", f->ret, f->name);
    } else {
      fprintf(s, "  %s(", f->name);
    }
    for (a = f->args; a != NULL; a = a->nxt) {
      fprintf(s, "%s0", a->name);
      if (a->nxt != NULL) {
        fprintf(s, ", ");
      }
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

void parseCIncludeFile(Cmd* cmd) {
  FILE* s = fopen(cmd->args->name, "r");
  char c;
  char p = EOF;
  int lineNumber = 1;
  CFunc* f0 = NULL;
  CFunc* f = NULL;
  if (s != NULL) {
    char input[512] = "";
    char debugInput[512] = "";
    int inMultiComment = false;
    int nested = 0;
    int nestedP = 0;
    int discardToEOL = false;
    int discardToSemiColon = false;
    while ((c = getc(s)) != EOF) {
      if (c == '\n' || c == '\r') {
        ++lineNumber;
        if (lineNumber >= 98) {
          debug();
        }
        debugInput[0] = '\0';
      } else {
        straddch(debugInput, c);
      }
      if (inMultiComment) {
        if (p == '*' && c == '/') {
          inMultiComment = false;
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
          discardToEOL = false;
        }
      } else if ((c == ' ' || c == '\t') && strlen(input) <= 0) {
        // Discard trailing whitespaces
      } else if ((p == ' ' || p == '\t') && (c == ' ' || c == '\t')) {
        // Discard double whitespaces
      } else if (discardToSemiColon) {
        if (c == ';') {
          discardToSemiColon = false;
        }
      } else if (discardToEOL) {
        // Discard comments
      } else if (c == '#') {
        // Discard preprocessor
        discardToEOL = true;
      } else if (p == '/' && c == '/') {
        discardToEOL = true;
        strdelch(input);
      } else if (p == '/' && c == '*') {
        inMultiComment = true;
        strdelch(input);
      } else if (strncmp(input, "__", 2) == 0) {
        discardToEOL = true;
        input[0] = '\0';
      //} else if (strncmp(input, "typedef", 7) == 0) { // case: __extension typedef...
      } else if (strstr(input, "typedef") != NULL || strstr(input, "struct") != NULL) {
        discardToSemiColon = true;
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
  } else {
    output("Invalid include file.");
  }
  bindCFunctionsHeader(f0);
  bindCFunctionsSource(f0);
  freeCFunc(f);
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

void runCFunction(Cmd* cmd) {
  // TODO: Be able to run printf, only by including stdlib.h
  // TODO: Run a function dynamically using assembler.
  if (strcmp(cmd->name, "addstr")) {
    addstr(cmd->args->name);
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
void run(Cmd* cmd) {
  char retVal[1024];
  //runCmd(retVal, cmd);
  output(retVal);
}

void printVar(Cmd* cmd) {
  if (cmd->args == NULL) {
    char b[1024] = "";
    catVar(b, varByName(cmd->name));
    output(b);
  } else {

  }
}

void createVar(Cmd* cmd) {
  Var* var = malloc(sizeof(Var));
  strcpy(var->name, cmd->args->name);
  var->type = typeByName(cmd->name);
  Var* oldFirst = vars;
  vars = var;
  var->nxt = oldFirst;
  var->val = NULL;
  // FIXME: Not sure about that. Probably should not be doing this.
  addLoadedDef(loadedDefs, var->name, 0, printVar); 
}

void createType(Cmd* cmd) {
  Type* type = malloc(sizeof(Type));
  strcpy(type->name, cmd->args->name);
  Type* oldFirst = types;
  types = type;
  type->nxt = oldFirst;
  addLoadedDef(loadedDefs, type->name, 0, createVar);
}

void assign(Cmd* cmd) {
  Var* v = varByName(cmd->args->name);
  setVarVal(v, cmd->args->nxt);
}

/*void edit(Func* func) {
  if (func != NULL) {
    int y, x;
    char m[256] = "";
    char i[256] = "";
    getyx(curscr, y, x);
    move(y+1, 0);
    addstr(catDef(m,func));
    refresh();
    move(y+2, 0);
    getInput(i);
    func->lambda = parseLambda(i);
  }
}*/

void listTypes(Cmd* cmd) {
  Type* t;
  char m[1024] = "";
  for (t = types; t != NULL; t = t->nxt) {
    strcat(m, t->name);
    if (t->nxt != NULL) {
      strcat(m, "\n");
    }
  }
  output(m);
}

void listVars(Cmd* cmd) {
  Var* v;
  char m[1024] = "";
  for (v = vars; v != NULL; v = v->nxt) {
    catVar(m, v);
    if (v->nxt != NULL) {
      strcat(m, "\n");
    }
  }
  output(m);
}

void eval(Cmd* cmd) {
  if (cmd == NULL) return;
  
  if (!silent) {
    int y, x;
    getyx(curscr, y, x);
    move(y+1, indent);
    addstr("=> ");
    refresh();
  }

  if (strlen(cmd->name) > 0) {
    LoadedDef* d;
    for (d = loadedDefs; d != NULL; d = d->nxt) {
      if (strcmp(cmd->name, d->name) == 0) {
        d->ptr(cmd);
        break;
      }
    }
  }
  eval(cmd->nxt);
}

void loop()
{
  int y, x;
  int continuer = true;
  addstr(">> ");
  while (continuer) {
    Cmd* cmd = getInput();
    if (strcmp(cmd->name, "exit") == 0 ||
               strcmp(cmd->name, "quit") == 0 ||
               strcmp(cmd->name, "q") == 0) {
      freeCmd(cmd);
      return;
    }
    eval(cmd);
    freeCmd(cmd);
    getyx(curscr, y, x);
    mvaddstr(y+1, 0, ">> ");
    refresh();
  }
}

static void finish(int sig)
{
  endwin();

  freeLoadedDef(loadedDefs);
  loadedDefs = NULL;
  freeType(types);
  types = NULL;
  freeVar(vars);
  vars = NULL;

  exit(0);
}

void initLoadedDefs() {
  loadedDefs = createLoadedDef("save", 0, save);
  addLoadedDef(loadedDefs, "=", 1, assign);
  addLoadedDef(loadedDefs, "type", 0, createType);
  addLoadedDef(loadedDefs, "$vars", 0, listVars);
  addLoadedDef(loadedDefs, "$types", 0, listTypes);
  addLoadedDef(loadedDefs, ":d", 0, printCmd);
  addLoadedDef(loadedDefs, "include", 0, parseCIncludeFile);
}

void main()
{
  signal(SIGINT, finish);      /* arrange interrupts to terminate */

  initLoadedDefs();
  initCFunctions(loadedDefs);

  silent = true;
  load();
  silent = false;

  initscr();
  keypad(stdscr, TRUE);
  nonl();         /* tell curses not to do NL->CR/NL on output */
  cbreak();       /* take input chars one at a time, no wait for \n */
  noecho();       /* dont echo the input char */

  loop();

  finish(0);
}
