#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <signal.h>
#include <string.h>

#include "model.h"

// MODEL

static const int MSG_CONSOLE_SIZE = 10;
static const char DEF_FILE_PATH[] = "defs";

static Type* types = NULL;
static Var* vars = NULL;

// FIXME: Made static so I can free in finalize. Should not be static.
// Or fuck conventions and let it be static...
static Func* defs = NULL;

// TODO: Have a list that contains both the loaded defs and the runtime one.
// They should of type struct LoadedFunc and the function passed would call the executable.
static LoadedFunc* loadedDefs = NULL;

LoadedFunc* lastLoadedFunc() {
  if (loadedDefs == NULL) {
    return NULL;
  } else {
    LoadedFunc* f;
    for (f = loadedDefs; f->nxt != NULL; f = f->nxt) {}
    return f;
  }
}

LoadedFunc* createLoadedFunc(char* name, bool isOp, void (*ptr)(Cmd* cmd)) {
  LoadedFunc* d = malloc(sizeof(LoadedFunc));
  strcpy(d->name, name);
  d->isOperator = isOp;
  d->ptr = ptr;
  d->nxt = NULL;
  return d;
}

LoadedFunc* addLoadedFunc(char* name, int priority, void (*ptr)(Cmd* cmd)) {
  LoadedFunc* f = createLoadedFunc(name, priority, ptr);
  if (loadedDefs == NULL) {
    loadedDefs = f;
  } else {
    lastLoadedFunc()->nxt = f;
  }
}

// A block is a Cmd with two args. The first is the args, the second is the body
static const char BLOCK[] = "BLOCK";

static Alias* aliases = NULL;

Cmd* newCmd() {
  Cmd* arg0 = malloc(sizeof(Cmd));
  if (arg0 == NULL) {
    abort(); // FIXME: "Can't allocate memory"
  }
  memset(arg0->name, '\0', sizeof(arg0->name));
  arg0->nxt = NULL;
  arg0->args = NULL;
  return arg0;
}

Arg* newArg() {
  Arg* arg0 = malloc(sizeof(Arg));
  if (arg0 == NULL) {
    abort(); // FIXME: "Can't allocate memory"
  }
  memset(arg0->val, '\0', sizeof(arg0->val));
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

void freeLoadedFunc(LoadedFunc* d) {
  if (d != NULL) {
    freeLoadedFunc(d->nxt);
    free(d);
  }
}

void freeArg(Arg* arg) {
  if (arg != NULL) {
    freeArg(arg->nxt);
    free(arg);
  }
}

void freeCmd(Cmd* arg) {
  if (arg != NULL) {
    freeCmd(arg->nxt);
    freeCmd(arg->args);
  }
}

char* catArg(char* m, Arg* arg) {
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
}

void freeFunc(Func* f) {
  if (f != NULL) {
    freeFunc(f->nxt);
    freeArg(f->args);
    if (f->lambda != NULL) {
      free(f->lambda);
    }
    free(f);
  }
}

void freeAlias(Alias* a) {
  if (a != NULL) {
    freeAlias(a->nxt);
    free(a);
  }
}

Func* newFunc() {
  Func* func = malloc(sizeof(Func));
  if (func == NULL) {
    abort(); // FIXME: "Can't allocate memory"
  }
  memset(func->name, '\0', sizeof(func->name));
  func->isOperator = false;
  func->lambda = NULL;
  func->args = NULL;
  func->nxt = NULL;
  return func;
}

Alias* newAlias() {
  Alias* a = malloc(sizeof(Alias));
  if (a == NULL) {
    abort(); // FIXME: "Can't allocate memory"
  }
  memset(a->name, '\0', sizeof(a->name));
  a->func = NULL;
  return a;
}

char* catDef(char* m, Func* f) {
  strcat(m, f->name);
  strcat(m, " :: ");
  catArg(m, f->args); 
  return m;
}

char* catFunc(char* m, Func* f) {
  Arg* arg;
  catDef(m, f);
  strcat(m, "\n");
  strcat(m, f->name);
  strcat(m, " = ");
  if (f->lambda != NULL) {
    strcat(m, "\\");
    for (arg = f->lambda->args; arg != NULL; arg = arg->nxt) {
      strcat(m, arg->val);
      strcat(m, " ");
    }
    strcat(m, "-> ");
    strcat(m, f->lambda->body);
  } else {
    strcat(m, "\"\"");
  }
  return m;
}

void setVarVal(Var* v, Cmd* val) {
  if (v->val != NULL) {
    freeCmd(v->val);
  } else {
    v->val = newCmd();
    strcpy(v->val->name, val->name);
    v->val->args = val->args;
    v->val->nxt = val->nxt;
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

static bool silent = false;
void output(const char* str) {
  if (!silent) {
    int y, x;
    getyx(curscr, y, x);
    move(y+1, indent);
    addstr("=> ");
    addstr(str);
    //move(y+2, x);
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
  if (cmd-> args != NULL) {
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
  return b;
}
void printCmd(Cmd* cmd) {
  char b[1024] = "";
  catCmd(b, cmd);
  output(b);
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

LoadedFunc* loadedFuncByName(char* name) {
  LoadedFunc* d = loadedDefs;
  while (d != NULL) {
    if (strcmp(d->name, name) == 0) {
      return d;
    }
    d = d->nxt;
  }
  return NULL;
}
Func* funcByName(char* name) {
  Func* d = defs;
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
ParsePair parseCmdR(char* command, int nested) { // FIXME: Does not work for "(add 12 12)"
  Cmd* cmds = newCmd();
  Cmd* cmd = cmds;
  char* s = trim(command);
  while (*s != '\0') {
    char* i = s;
    while (*s != '\0' && *s != ' ' && *s != '(' && *s != ')' && *s != '\n') {
      ++s;
    }
    strncpy(cmd->name, i, s-i);
    s = trim(s);
    if (*s == ')') {
      ++s;
      break;
    } else if (*s == '(') {
      ParsePair p = parseCmdR(s+1, nested + 1);
      s = p.ptr;
      cmd->nxt = p.cmd;
      cmd = cmd->nxt;
    } else if (*s != '\0') {
      cmd->nxt = newCmd();
      cmd = cmd->nxt;
    }
  }
  LoadedFunc* f;
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
  return parseCmdR(command, 0).cmd;
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

Lambda* parseLambda(char* s) {
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
}

bool isOperator(char* opName) {
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

void compileFunc(char* s, Func* f) {
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
}

void compile(Func* f) {
  if (f != NULL) {
    char fs[2056] = "";
    compileFunc(fs, f);
    output(fs);
  }
}

void save(Cmd* cmd) { // .qc extension. Quick C, Quebec!!!
  Func* f = NULL;
  FILE* s = fopen("app.qc", "w"); // FIXME: Check if valid file. Not NULL.
  char m[1024] = "";
  for (f = defs; f != NULL; f = f->nxt) {
    m[0] = '\0';
    fprintf(s, "\n%s", catFunc(m, f));
  }
  fclose(s);
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

void runCmd(char* retVal, Cmd* cmd);

char* argVal(char* buf, Cmd* arg) {
  if (arg->args) {
    char argValue[128] = "";
    runCmd(argValue, arg);
    strcat(buf, argValue);
  } else {
    strcat(buf, arg->name);
  }
}

void runCmd(char* retVal, Cmd* cmd) { // FIXME: Fonction dependencies must be added too.
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
}
void run(Cmd* cmd) {
  char retVal[1024];
  runCmd(retVal, cmd);
  output(retVal);
}

void createVar(Cmd* cmd) {
  Var* var = malloc(sizeof(Var));
  strcpy(var->name, cmd->args->name);
  var->type = typeByName(cmd->name);
  Var* oldFirst = vars;
  vars = var;
  var->nxt = oldFirst;
  var->val = NULL;
}

void createType(Cmd* cmd) {
  Type* type = malloc(sizeof(Type));
  strcpy(type->name, cmd->args->name);
  Type* oldFirst = types;
  types = type;
  type->nxt = oldFirst;
  addLoadedFunc(type->name, 0, createVar);
}

void assign2(Cmd* cmd) {
  Var* v = varByName(cmd->args->name);
  setVarVal(v, cmd->args->nxt);
}

void assign(Cmd* cmd) {
  Func* f = funcByName(cmd->args->name);
  if (f == NULL) return;

  char i[512] = "";
  Cmd* c;
  for (c = cmd->args->nxt; c != NULL; c = c->nxt) {
    strcat(i, c->name);
    if (c->nxt != NULL) {
      strcat(i, " ");
    }
  }
  f->lambda = parseLambda(i);
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
    strcat(m, v->type->name);
    strcat(m, " ");
    strcat(m, v->name);
    if (v->val != NULL) {
      strcat(m, " ");
      catCmd(m, v->val);
    }
    if (v->nxt != NULL) {
      strcat(m, "\n");
    }
  }
  output(m);
}

void list(Cmd* cmd) {
  Func* d = defs;
  while (d != NULL) {
    char m[256] = "";
    output(catDef(m,d));
    d = d->nxt;
  }
}

Func* defFunc(Cmd* cmd, int isOp) {
  Func* def = defs;
  Arg* arg = NULL;
  Cmd* n = cmd->args;
  /*FILE *f = fopen(DEF_FILE_PATH, "a");
  if (f == NULL) {
    abort(); // FIXME: "Can't open definition file."
  }
  fputs(d, f);
  fclose(f);*/

  if (def == NULL) {
    defs = newFunc();
    def = defs;
  } else {
    while (def->nxt != NULL) {
      def = def->nxt;
    }
    def->nxt = newFunc();
    def = def->nxt;
  }
  while (n != NULL) {
    if (strlen(def->name) == 0) {
      strcpy(def->name, n->name);
    } else {
      arg = appendNewArg(arg);
      if (def->args == NULL) {
        def->args = arg;
      }
      strcpy(arg->val, n->name);
    }
    n = n->nxt;
  }
  //if (arg == NULL) {
  //  def->args = newArg();
  //  strdef->args
  //}
  if (def->args == NULL) {
    def->args = newArg();
    strcpy(def->args->val, "void");
  }
  def->isOperator = isOp;
  list(NULL); // FIXME: just show one
  addLoadedFunc(def->name, def->isOperator, run);
  return def;
}
void def(Cmd* cmd) {
  defFunc(cmd, 0);
}

void defOp(Cmd* cmd) {
  Func* f = defFunc(cmd, 1);
}

void show(Cmd* cmd) {
  Func* f = funcByName(cmd->name);
  char m[600] = "";
  if (f == NULL) {
    output("Unkown function");
  } else {
    output(catFunc(m, f));
  }
}

void alias(Arg* arg) { // FIXME: Check arg count
  Func* f = funcByName(arg->nxt->val);
  Alias* a;
  if (f != NULL) {
    if (aliases == NULL) {
      aliases = newAlias();
      a = aliases;
    } else {
      aliases->nxt = newAlias();
      a = aliases->nxt;
    }
    strcpy(a->name, arg->val);
    a->func = f;
  }
}

void eval(Cmd* cmd) {
  if (cmd == NULL) return;

  if (strlen(cmd->name) > 0) {
    LoadedFunc* d;
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
  bool continuer = true;
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

  freeFunc(defs);
  defs = NULL;
  freeLoadedFunc(loadedDefs);
  loadedDefs = NULL;
  freeType(types);
  types = NULL;
  freeVar(vars);
  vars = NULL;
  freeAlias(aliases);
  aliases = NULL;

  exit(0);
}

void initLoadedFuncs() {
  addLoadedFunc("=", 1, assign);
  addLoadedFunc("save", 0, save);
  addLoadedFunc("::", 1, def);
  addLoadedFunc(":::", 1, defOp);
  addLoadedFunc(":=", 1, assign2);
  addLoadedFunc("type", 0, createType);
  addLoadedFunc("$vars", 0, listVars);
  addLoadedFunc("$types", 0, listTypes);
  addLoadedFunc(":d", 0, printCmd);
  addLoadedFunc("l", 0, list);
}

void main()
{
  signal(SIGINT, finish);      /* arrange interrupts to terminate */

  initLoadedFuncs();

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
