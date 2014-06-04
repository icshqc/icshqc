#include <stdlib.h>
#include <ncurses.h>
#include <signal.h>
#include <string.h>

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
    str[i-1] = '\0';
  }
  return str;
}

void msg(const char* str) {
  int y, x, i;
  for (i=0; i<COLS; i++) {
    mvdelch(LINES-1,i);
  }
  getyx(curscr, y, x);
  move(LINES-1, 0);
  addstr(str);
  move(y, x);
  refresh();
}

void run()
{
  char args[12][64];
  args[0][0] = '\0';
  args[1][0] = '\0';
  args[2][0] = '\0';
  args[3][0] = '\0';
  args[4][0] = '\0';
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
        getyx(curscr, y, x);
        mvdelch(y, x-1);
        argc--;
        refresh();
      }
    } else if (ch == '\n' || ch == '\r') {
      if (strstr(args[0], "def ") == args[0]) {
        //args[0] = args[0] + 4;
      }
      getyx(curscr, y, x);
      mvaddstr(y+1, 0, ">> ");
      refresh();
    } else if (ch == ',') {
      addch(','); 
      argc++;
      refresh();
    //} else if (ch == ':') {
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
