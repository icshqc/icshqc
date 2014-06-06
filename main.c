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

void freeArg(Arg* arg) {
  if (arg != NULL) {
    freeArg(arg->nxt);
    free(arg);
  }
}

void freeFunc(Func* f) {
  if (f != NULL) {
    freeFunc(f->nxt);
    freeArg(f->args);
    free(f);
  }
}

char* catArg(char* m, Arg* arg) {
  if (arg != NULL) {
    Arg* n = arg->nxt;
    strcat(m, "Arg[\"");
    strcat(m, arg->val);
    while (n != NULL) {
      strcat(m, "\", \"");
      strcat(m, n->val);
      n = n->nxt;
    }
    strcat(m, "\"]");
  }
  return m;
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
        Arg* arg = malloc(sizeof (struct Arg));
        strncpy(arg->val, i, c - i);
        arg->nxt = parseCmd(c+1);
        return arg;
      }
    }
    c++;
  }
  if (i+1 != c) {
    Arg* arg = malloc(sizeof (struct Arg));
    strcpy(arg->val, i);
    return arg;
  } else {
    return NULL;
  }
}

void genApp(Func* f) { // FIXME: Fonction dependencies must be added too.
  if (f != NULL) {
    Arg* arg = f->args;
    int n;

    FILE* s = fopen("app.c", "w");
    fprintf(s, "#include <stdlib.h>\n");
    fprintf(s, "#include <stdio.h>\n\n");
    fprintf(s, "int main(int argc, char* argv[])\n");
    fprintf(s, "int main(int argc, char* argv[])\n");

    while (arg != NULL) {
      fprintf(s, "%s arg%d;\n", arg->val, n);
      n++;
      arg = arg->nxt;
    }

    fprintf(s, "if (argc != %d) {\n", n + 1);
    fprintf(s, "  fprintf(stderr, \"Invalid amout of parameters.\n\");\n");
    fprintf(s, "  return -1;\n");
    fprintf(s, "}\n\n");
    fprintf(s, "sscanf(argv[1],\"%%d\",&arg1);");
    fprintf(s, "sscanf(argv[1],\"%%d\",&arg1);");
    fprintf(s, "%s r = add(arg1, arg2);", f->ret.val);
    fprintf(s, "printf(\"%%d\", r);"); // FIXME: Not always interger.
    fprintf(s, "return 0;");
    fprintf(s, "}");
  }
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
    defs = malloc (sizeof (struct Func));
    def = defs;
  } else {
    while (def->nxt != NULL) {
      def = def->nxt;
    }
    def->nxt = malloc (sizeof (struct Func));
    def = def->nxt;
  }
  if (def == NULL) {
    abort(); // FIXME: "Can't allocate memory"
  }
  arg = def->args;
  n = args;
  while (n != NULL) {
    if (strlen(def->name) == 0) {
      strcpy(def->name, args->val);
    } else {
      if (arg == NULL) {
        def->args = malloc(sizeof (struct Arg));
        arg = def->args;
      } else {
        arg->nxt = malloc(sizeof (struct Arg));
        arg = arg->nxt;
      }
      strcpy(arg->val, n->val);
    }
    n = n->nxt;
  }
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

void run()
{
  int y, x;
  char cmd[256] = "";
  addstr(">> ");
  while (true) {
    getInput(cmd);
    if (strstr(cmd, "def ") == cmd) {
      Arg* args = parseCmd(((char *)cmd) + 4);
      def(args);
      free(args);
    } else if (strstr(cmd, "list") == cmd) {
      list(defs);
    } else if (strstr(cmd, "show ") == cmd) {
      Arg* args = parseCmd(((char *)cmd) + 5);
      show(args);
      free(args);
    } else if (strstr(cmd, "edit ") == cmd) {
      Arg* args = parseCmd(((char *)cmd) + 5);
      edit(args);
      free(args);
    } else if (strstr(cmd, "exit") == cmd ||
               strstr(cmd, "quit") == cmd) {
      return;
    }
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

  run();

  finish(0);
}

static void finish(int sig)
{
  endwin();

  freeFunc(defs);

  exit(0);
}
