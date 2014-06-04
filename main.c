#include <stdlib.h>
#include <ncurses.h>
#include <signal.h>
#include <string.h>

// HELPER: C strings: http://en.wikipedia.org/wiki/C_string_handling

static void finish(int sig);

char* straddch(char* str, char c) { //FIXME: Not buffer safe
  int i = strlen(str);
  str[i] = c;
  str[i+1] = '\0';
  return str;
}
char* strdelch(char* str) {
  int i = strlen(str);
  if (i > 0) {
    str[i] = '\0';
  }
  return str;
}

void msg(const char* str) {
  int y, x;
  getyx(curscr, y, x);
  move(LINES-1, 0); // COLS pour x
  addstr(str);
  move(y, x);
  refresh();
}

void run()
{
  char args[12][64];
  int argc = 0;
  int y, x;
  addstr(">> ");
  while (true) {
    int ch = getch();
    if (ch == 'q')
      return;
    else if (ch == KEY_BACKSPACE) {
      if (strlen(args[argc]) > 0) {
        getyx(curscr, y, x);
        mvdelch(y, x-1);
        strdelch(args[argc]);
        refresh();
      } else if (argc > 0) {
        argc--;
      }
    } else if (ch == '\n' || ch == '\r') {
      getyx(curscr, y, x);
      mvaddstr(y+1, 0, ">> ");
    } else {
      addch(ch);
      straddch(args[argc], ch);
      refresh();
    }
    msg(args[argc]);
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
