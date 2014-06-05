#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <signal.h>
#include <string.h>

static const int MSG_CONSOLE_SIZE = 10;
static const char DEF_FILE_PATH[] = "defs";

static void finish(int sig);

struct Arg {
  char name[52];
  char type[32];
  struct Arg* nxt;
};
typedef struct Arg Arg;

struct Func {
  char name[52];
  Arg* args;
  struct Func* nxt;
};
typedef struct Func Func;

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

static int msgLine = 0;
// TODO: debug(), fatal(), error(), warn(), log()
void msg(const char* str) {
  int y, x, i;
  int line = LINES - MSG_CONSOLE_SIZE + msgLine;
  msgLine = (msgLine + 1) % MSG_CONSOLE_SIZE;
  for (i=0; i<COLS; i++) {
    mvdelch(line,i);
  }
  getyx(curscr, y, x);
  move(line, 0);
  addstr(str);
  move(y, x);
  refresh();
}

void list(Func* d) {
  char m[80] = "";
  Arg* arg;
  if (d != NULL) {
    strcat(m, "Name: ");
    strcat(m, d->name);
    strcat(m, ", Args: ");
    arg = d->args;
    while (arg != NULL) {
      strcat(m, "name: ");
      strcat(m, arg->name);
      strcat(m, "type: ");
      strcat(m, arg->type);
      strcat(m, ", ");
      arg = arg->nxt;
    }
    msg(m);
    list(d->nxt);
  }
}

void def(char* d) {
  char* c = d;
  char* i = d;
  Arg* arg = NULL;
  FILE *f = fopen(DEF_FILE_PATH, "a");
  Func* def = defs;
  if (f == NULL) {
    abort(); // FIXME: "Can't open definition file."
  }

  fputs(d, f);
  fclose(f);

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
  while (*c != '\0') {
    if (*c == ',') {
      if (strlen(def->name) == 0) {
        strncpy(def->name, i, c - i);
      } else {
        if (def->args == NULL) {
          def->args = malloc(sizeof (struct Arg));
          arg = def->args;
        } else {
          def->args->nxt = malloc(sizeof (struct Arg));
          arg = def->args->nxt;
        }
        strncpy(arg->type, i, c - i);
      }
      i = c;
    }
    c++;
  }
  if (i+1 != c) {
    if (strlen(def->name) == 0) {
      strncpy(def->name, i, c - i);
    } else {
      if (def->args == NULL) {
        def->args = malloc(sizeof (struct Arg));
        arg = def->args;
      } else {
        def->args->nxt = malloc(sizeof (struct Arg));
        arg = def->args->nxt;
      }
      strncpy(arg->type, i, c - i);
    }
  }
}

void run()
{
  char cmd[256] = "";
  int y, x;
  addstr(">> ");
  while (true) {
    int ch = getch();
    if (ch == 'q')
      return;
    else if (ch == KEY_BACKSPACE) {
      if (strlen(cmd) > 0) {
        getyx(curscr, y, x);
        mvdelch(y, x-1);
        strdelch(cmd);
        refresh();
      }
    } else if (ch == '\n' || ch == '\r') {
      if (strstr(cmd, "def ") == cmd) {
        def(((char *)cmd) + 4);
      }
      if (strstr(cmd, "list") == cmd) {
        list(defs);
      }
      getyx(curscr, y, x);
      mvaddstr(y+1, 0, ">> ");
      refresh();
      cmd[0] = '\0';
    //} else if (ch == ':') {
    } else {
      addch(ch);
      straddch(cmd, ch);
      refresh();
    }
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
