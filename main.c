#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <signal.h>
#include <string.h>

static const int MSG_CONSOLE_SIZE = 5;
static const char DEF_FILE_PATH[] = "defs";

static void finish(int sig);

struct Func {
  char name[52];
  char args[5][52];
};
typedef struct Func Func;

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

void list() {
}

void def(Func* defs, char* d) {
  int ndef, j = 0;
  char* c = d;
  char* i = d;
  FILE *f = fopen(DEF_FILE_PATH, "a");
  if (f == NULL) {
    abort(); // FIXME: "Can't open definition file."
  }

  fputs(d, f);
  fclose(f);

  for (ndef = 0; strlen(defs[ndef].name); i++) {}
  while (*c != '\0') {
    if (*c == ',') {
      if (j == 0) {
        strncpy(defs[ndef].name, i, c - i);
        i = c;
      } else {
        strncpy(defs[ndef].args[j-1], i, c - i);
        i = c;
      }
      j++;
    }
    c++;
  }

  msg(defs[ndef].name);
}

void run()
{
  Func defs[52] = { { "", {"","","","",""} } };
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
        def(defs, ((char *)cmd) + 4);
      }
      getyx(curscr, y, x);
      mvaddstr(y+1, 0, ">> ");
      refresh();
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

  /* do your non-curses wrapup here */

  exit(0);
}
