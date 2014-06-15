#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <signal.h>
#include <string.h>

#include "model.h"

// MODEL

static const int MSG_CONSOLE_SIZE = 10;
static const char DEF_FILE_PATH[] = "defs";

static void finish(int sig);

// FIXME: Made static so I can free in finalize. Should not be static.
// Or fuck conventions and let it be static...
static Func* defs = NULL;
// Only the :: and = operators are defined at the start maybe. Only them are hard coded.
static Func* operators = NULL; // Operators are function with the form arg1 op arg2 ... argN

static Alias* aliases = NULL;

ArgTree* newArgTree() {
  ArgTree* arg0 = malloc(sizeof(ArgTree));
  if (arg0 == NULL) {
    abort(); // FIXME: "Can't allocate memory"
  }
  arg0->arg = NULL;
  arg0->child = NULL;
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

void freeArg(Arg* arg) {
  if (arg != NULL) {
    freeArg(arg->nxt);
    free(arg);
  }
}

void freeArgTree(ArgTree* arg) {
  if (arg != NULL) {
    if (arg->arg != NULL) {
      freeArg(arg->arg);
    }
    if (arg->child != NULL) {
      freeArg(arg->child);
    }
  }
}

char* catArg(char* m, Arg* arg) {
  if (arg != NULL) {
    Arg* n = arg->nxt;
    strcat(m, arg->val);
    while (n != NULL) {
      strcat(m, " -> ");
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
  func->lambda = NULL;
  func->args = NULL;
  //func->ret = NULL;
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

static int indent = 0;

static int msgLine = 0;
// TODO: debug(), fatal(), error(), warn(), log()
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

void output(const char* str) {
  int y, x;
  getyx(curscr, y, x);
  move(y+1, indent);
  addstr("=> ");
  addstr(str);
  //move(y+2, x);
  refresh();
}

// APP

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

char* getInput(char* i) {
  int y, x;
  while (true) {
    int ch = getch();
    if (ch == KEY_BACKSPACE) {
      if (strlen(i) > 0) {
        getyx(curscr, y, x);
        mvdelch(y, x-1);
        strdelch(i);
        refresh();
      }
    } else if (ch == '\n' || ch == '\r') {
      return i;
    //} else if (ch == ':') {
    } else {
      addch(ch);
      straddch(i, ch);
      refresh();
    }
  }
}

// Return a pointer to the first non whitespace char of the string.
char* trim(char* s) {
  char* c = s;
  while (*c != '\0' && *c == ' ') {
    c++;
  }
  return c;
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
      arg = appendNewArg(arg);
      if (l->args == NULL) {
        l->args = arg;
      }
      strncpy(arg->val, i, c - i);
      i = c;
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

// Splits the cmd at spaces, removing when many in a row.
// The children of functions are their arguments.
// TODO: Parse parentheses.
Arg* parseCmd(char* cmd) {
  Arg* arg = NULL;
  char* c = cmd;
  char* i = cmd;
  while (*c != '\0') {
    if (*c == ' ') {
      if (i == c) {
        i++;
      } else {
        arg = newArg();
        strncpy(arg->val, i, c - i);
        arg->nxt = parseCmd(c+1);
        return arg;
      }
    }
    c++;
  }
  if (c - i > 0) {
    Arg* arg = newArg();
    strcpy(arg->val, i);
    return arg;
  } else {
    return NULL;
  }
}

void editFunc(Func* func) {
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

  sprintf(tmp, "%s %s(", ret->val, f->name);
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

void save() { // .qc extension. Quick C, Quebec!!!
  Func* f = NULL;
  FILE* s = fopen("app.qc", "w"); // FIXME: Check if valid file. Not NULL.
  char m[1024] = "";
  for (f = defs; f != NULL; f = f->nxt) {
    m[0] = '\0';
    fprintf(s, "\n%s", catFunc(m, f));
  }
  fclose(s);
}

bool eval(char *cmd);

void load() {
  int c;
  char cmd[1000] = "";
  size_t n = 0;
  FILE* s = fopen("app.qc", "r"); // FIXME: Check if valid file. Not NULL.
  if (s != NULL) {
    while ((c = fgetc(s)) != EOF) {
      if (c == '\n' || c == '\r') {
        if (!eval(cmd)) {
          // FIXME
        }
        n = 0;
        cmd[0] = '\0';
      } else {
        cmd[n] = (char) c; 
        cmd[++n] = '\0';
      }
    }
    fclose(s);
  }
}

void run(Arg* args) { // FIXME: Fonction dependencies must be added too.
  Func* f = funcByName(args->val);
  if (f == NULL) return;
  if (f->args == NULL) return; // Invalid function. Needs return type. FIXME: Better error handling

  Arg* arg;
  Arg* ret;
  int n;
  int i;

  FILE* s = fopen("tmp/app.c", "w"); // FIXME: Check if valid file. Not NULL.
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
    fprintf(s, "  %s(", f->name);
  } else {
    fprintf(s, "  %s r = %s(", ret->val, f->name);
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

  char retVal[1024] = "";
  char cmd[256] = "";
  strcat(cmd, "gcc -o tmp/app tmp/app.c && ./tmp/app");
  for (arg = args->nxt; arg != NULL; arg = arg->nxt) {
    strcat(cmd, " "); 
    strcat(cmd, arg->val); 
  }
  FILE *fp = popen(cmd, "r"); // TODO: Args

  fscanf(fp, "%s", retVal);
  pclose(fp);
  
  output(retVal);
}

void assign(Arg* args) {
  Func* f = funcByName(args->val);
  if (f == NULL) return;

  char i[512] = "";
  Arg* arg;
  for (arg = args->nxt; arg != NULL; arg = arg->nxt) {
    strcat(i, arg->val);
    if (arg->nxt != NULL) {
      strcat(i, " ");
    }
  }
  f->lambda = parseLambda(i);
}

void edit(Func* func) {
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
}

void list(Func* d) {
  char m[256] = "";
  if (d != NULL) {
    output(catDef(m,d));
    list(d->nxt);
  }
}

void def(Arg* args) {
  Func* def = defs;
  Arg* arg = NULL;
  Arg* n = args;
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
      strcpy(def->name, args->val);
    } else {
      arg = appendNewArg(arg);
      if (def->args == NULL) {
        def->args = arg;
      }
      strcpy(arg->val, n->val);
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
  list(def); // FIXME: just show one
}

void show(Arg* arg) {
  Func* f = funcByName(arg->val);
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

bool eval(char* cmd) {
  Arg* args = parseCmd(cmd);
  if (args != NULL) {
    if (strcmp(args->val, "::") == 0) {
      def(args->nxt);
    } else if (strcmp(args->val, "l") == 0) {
      list(defs);
    } else if (strcmp(args->val, "save") == 0) {
      save(); // TODO: tmp, should always save automatically.
    } else if (strcmp(args->val, "s") == 0) {
      show(args->nxt);
    } else if (strcmp(args->val, "e") == 0) {
      edit(funcByName(args->nxt->val));
    } else if (strcmp(args->val, "c") == 0) {
      compile(funcByName(args->nxt->val));
    } else if (strcmp(args->val, "r") == 0) {
      run(args->nxt);
    } else if (strcmp(args->val, "load") == 0) {
      load();
    } else if (strcmp(args->val, "=") == 0) {
      assign(args->nxt);
    //} else if (strcmp(args->val, "gen") == 0) {
    //  gen(args->nxt);
    } else if (strcmp(args->val, "a") == 0) {
      alias(args->nxt);
    } else if (strcmp(args->val, "exit") == 0 ||
               strcmp(args->val, "quit") == 0 ||
               strcmp(args->val, "q") == 0) {
      freeArg(args);
      return false;
    } else {
      if (funcByName(args->val) != NULL) {
        if (args->nxt == NULL) {
          show(args);
        } else {
          run(args);
        }
      } else {
        output("Unkown function.");
      }
    }
  }
  freeArg(args);
  return true;
}

void loop()
{
  int y, x;
  bool continuer = true;
  char cmd[256] = "";
  addstr(">> ");
  while (continuer) {
    getInput(cmd);
    continuer = eval(cmd);
    getyx(curscr, y, x);
    mvaddstr(y+1, 0, ">> ");
    refresh();
    cmd[0] = '\0';
  }
}

void main()
{
  signal(SIGINT, finish);      /* arrange interrupts to terminate */

  /* initialize your non-curses data structures here */

  initscr();
  keypad(stdscr, TRUE);
  nonl();         /* tell curses not to do NL->CR/NL on output */
  cbreak();       /* take input chars one at a time, no wait for \n */
  noecho();       /* dont echo the input char */

  loop();

  finish(0);
}

static void finish(int sig)
{
  endwin();

  freeFunc(defs);
  defs = NULL;
  freeAlias(aliases);
  aliases = NULL;

  exit(0);
}


/*// TYPE

struct Type {
  enum { STRUCT, POINTER, INT } type;
  struct Type* subtype; // Some types have subtype. e.g. Pointer, Array
  char* (*catType)(char*, struct Type*);
  // freeType
  // initType set to zero
};
typedef struct Type Type;

struct TypeVal {
  Type type;
  char* val;
};
typedef struct TypeVal TypeVal;

TypeVal typeVal(Type type, char* val) {
  TypeVal v;
  v.type = type;
  v.val = val;
  return v;
}

char* intCatType(char* buf, Type* type) {
  return buf;
}

Type types[] = {
  {INT, NULL, intCatType},
  {POINTER, NULL, intCatType},
  {STRUCT, NULL, intCatType}
}; */

// Fuck gen. A la place, creer des types dans le nouveau language de prog et ca rajoute des fonctions automatique.
/*void gen(Arg* args) {
  if (args == NULL) return;

  TypeVal attr[12];
  memset(attr, 0, sizeof(attr));
  int nattr = 0;
  int i;

  // FIXME: Array does not work. Need to add char name[24]; !!! Need to add [24] after name.
  Arg* arg = args->nxt;
  while (arg != NULL) {
    char* c = strchr(arg->val, ':');
    if (c == NULL) {
      msg("Error: Wrong syntax to generate. Missing ':' for attr.");
      return;
    }
    strncpy(attr[nattr].val,arg->val,c-arg->val);
    // FIXME: Ugly hack. Not decided yet what syntax should be anyway...
    if (strcmp(c+1, "struct") == 0) {
      strcpy(attr[nattr].val, "struct ");
      strcpy(attr[nattr].val + 7, arg->nxt->val);
      arg = arg->nxt;
    } else {
      strcpy(attr[nattr][1], c+1);
    }
    nattr++;  
    arg = arg->nxt;
  }

  char* name = args->val;

  char buf[52];
  sprintf(buf, "model/%s.h", name);
  FILE* s = fopen(buf, "w");

  fprintf(s, "// WARNING: AUTOGENERATED, DO NOT OVERWRITE\n\n");

  fprintf(s, "#ifndef %s_H\n", name);
  fprintf(s, "#define %s_H\n\n", name);
 
  fprintf(s, "struct %s {\n", name);
  for (i = 0; i < nattr; i++) {
    fprintf(s, "  %s %s;\n", attr[i][1], attr[i][0]);
  }
  // TODO: Args
  fprintf(s, "};\n");
  fprintf(s, "typedef struct %s %s;\n\n", name, name);
 
  fprintf(s, "%s* new%s();\n", name, name);
  fprintf(s, "void free%s(%s* a);\n", name, name);
  fprintf(s, "char* cat%s(char* m, %s* a);\n\n", name, name);
 
  fprintf(s, "#endif // %s_h", name);
  fclose(s);
  
  sprintf(buf, "model/%s.c", name);
  s = fopen(buf, "w");
  fprintf(s, "#include <stdlib.h>\n");
  fprintf(s, "#include <stdio.h>\n");
  fprintf(s, "#include <string.h>\n\n");
 
  fprintf(s, "#include \"%s.h\"\n\n", name);
 
  fprintf(s, "void free%s(%s* a) {\n", name, name);
  fprintf(s, "  if (a != NULL) {\n");
  // FIXME: Check for pointers and free those. Yet should you always free?
  // fprintf(s, "    freeArg(arg->nxt);");
  fprintf(s, "    free(a);\n");
  fprintf(s, "  }\n");
  fprintf(s, "}\n\n");
 
  fprintf(s, "%s* new%s() {\n", name, name);
  fprintf(s, "  %s* a = malloc(sizeof(%s));\n", name, name);
  fprintf(s, "  if (a == NULL) {\n");
  fprintf(s, "    abort(); // FIXME msg: Can't allocate memory\n");
  fprintf(s, "  }\n");
  // FIXME: Init args
  //fprintf(s, "  memset(arg0->val, '\0', sizeof(arg0->val));");
  //fprintf(s, "  arg0->nxt = NULL;");
  fprintf(s, "  return a;\n");
  fprintf(s, "}\n\n");
 
//  fprintf(s, "char* cat%s(char* m, %s* a) {\n", name, name);
//  fprintf(s, "  if (a != NULL) {\n");
  //fprintf(s, "    Arg* n = arg->nxt;"); // FIXME
//  fprintf(s, "    strcat(m, \"%s[\\\"\");\n", name);
  //fprintf(s, "    strcat(m, arg->val);"); // FIXME
  //fprintf(s, "    while (n != NULL) {");
  //fprintf(s, "      strcat(m, \"\\\", \\\"\");");
  //fprintf(s, "      strcat(m, n->val);");
  //fprintf(s, "      n = n->nxt;");
  //fprintf(s, "    }");
//  fprintf(s, "    strcat(m, \"\\\"]\");\n");
//  fprintf(s, "  }\n");
//  fprintf(s, "  return m;\n");
//  fprintf(s, "}\n");

  fclose(s);
}*/

