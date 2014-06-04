#include <stdlib.h>
#include <ncurses.h>
#include <signal.h>

static void finish(int sig);

void msg(const char* str) {
  int y, x;
  getyx(curscr, y, x);
  move(LINES-1, 0); // COLS pour x
  addstr(str);
  move(y, x);
}

void run()
{
  char str[512]; //FIXME
  str[0] = '\0';
  int i = 0;
  int y, x;
  addch('>');
  addch('>');
  addch(' ');
  msg("Hello world!");
  while (true) {
    int ch = getch();
    if (ch == 'q')
      return;
    else if (ch == KEY_BACKSPACE) {
      if (i > 0) {
        getyx(curscr, y, x);
        mvdelch(y, x-1);
        str[i] = '\0';
        i--;
      }
    } else if (ch == '\n' || ch == '\r') {
      getyx(curscr, y, x);
      mvaddch(y+1, 0, '>');
      addch('>');
      addch(' ');
    } else {
      addch(ch);
      str[i++] = ch;
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
