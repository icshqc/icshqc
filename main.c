#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <signal.h>
#include <string.h>

#include "app.h"
#include "model.h"

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

// MODEL

static const int MSG_CONSOLE_SIZE = 10;
static const char DEF_FILE_PATH[] = "defs";

static void finish(int sig);

// FIXME: Made static so I can free in finalize. Should not be static.
// Or fuck conventions and let it be static...
static Func* defs = NULL;

static Alias* aliases = NULL;

void freeFunc(Func* f) {
  if (f != NULL) {
    freeFunc(f->nxt);
    freeArg(f->args);
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
  memset(func->body, '\0', sizeof(func->body));
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
  catDef(m, f);
  strcat(m, "\n");
  if (strlen(f->body)) {
    strcat(m, f->body);
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

void editFunc(Func* func) {
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
    strcpy(func->body, i);
  }
}

// Splits the cmd at spaces, removing when many in a row.
Arg* parseCmd(const char* cmd) {
  const char* c = cmd;
  const char* i = cmd;
  while (*c != '\0') {
    if (*c == ' ') {
      if (i == c) {
        i++;
      } else {
        Arg* arg = newArg();
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

void run(Func* f) { // FIXME: Fonction dependencies must be added too.
  if (f == NULL) return;
  if (f->args == NULL) return; // Invalid function. Needs return type. FIXME: Better error handling

  Arg* arg;
  Arg* ret;
  int n;
  int i;

  FILE* s = fopen("tmp/app.c", "w");
  fprintf(s, "#include <stdlib.h>\n");
  fprintf(s, "#include <stdio.h>\n\n");

  for (arg = f->args; arg->nxt != NULL; arg = arg->nxt ) {
  }
  ret = arg;

  fprintf(s, "%s %s(", ret->val, f->name);
 
  for (n = 0, arg = f->args; arg->nxt != NULL; arg = arg->nxt, n++ ) {
    fprintf(s, "%s arg%d", arg->val, n);
    if (arg->nxt->nxt != NULL) {
      fprintf(s, ", ");
    }
  }

  fprintf(s, ") {\n");
  fprintf(s, "%s", f->body);
  fprintf(s, "\n}\n\n");

  fprintf(s, "int main(int argc, char* argv[]) {\n");

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

  msg("Fonction runned.");
}

void edit(Arg* arg) {
  if (arg != NULL) {
    editFunc(funcByName(arg->val));
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
  Arg *arg, *n;
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
  n = args;
  arg = def->args;
  while (n != NULL) {
    if (strlen(def->name) == 0) {
      strcpy(def->name, args->val);
    } else {
      if (arg == NULL) {
        def->args = newArg();
        arg = def->args;
      } else {
        arg->nxt = newArg();
        arg = arg->nxt;
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

void loop()
{
  int y, x;
  char cmd[256] = "";
  addstr(">> ");
  while (true) {
    getInput(cmd);
    Arg* args = parseCmd(cmd);
    if (args != NULL) {
      if (strcmp(args->val, "def") == 0) {
        def(args->nxt);
      } else if (strcmp(args->val, "list") == 0) {
        list(defs);
      } else if (strcmp(args->val, "show") == 0) {
        show(args->nxt);
      } else if (strcmp(args->val, "edit") == 0) {
        edit(args->nxt);
      } else if (strcmp(args->val, "run") == 0) {
        run(funcByName(args->nxt->val));
      //} else if (strcmp(args->val, "gen") == 0) {
      //  gen(args->nxt);
      } else if (strcmp(args->val, "alias") == 0) {
        alias(args->nxt);
      } else if (strcmp(args->val, "exit") == 0 ||
                 strcmp(args->val, "quit") == 0) {
        freeArg(args);
        return;
      } else {
        output("Unkown function.");
      }
    }
    freeArg(args);
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
  freeAlias(aliases);

  exit(0);
}
