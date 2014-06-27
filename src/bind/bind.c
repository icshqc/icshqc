#include "bind.h"

void initCFunctions(LoadedDef* d) {
  addLoadedDef(d, "add", 0, bind_add);
  addLoadedDef(d, "sus", 0, bind_sus);
  addLoadedDef(d, "mult", 0, bind_mult);
  addLoadedDef(d, "divide", 0, bind_divide);
}

int add(int x, int y);
int sus(int x, int y);
int mult(int x, int y);
int divide(int x, int y);

Cmd* bind_add(Cmd* cmd) {
  Cmd* args = cmd->args;
  int x0 = argint(&args);
  int y0 = argint(&args);
  char ret[52] = "";  retCmd(INT, catargint(add(x0, y0)));
}

Cmd* bind_sus(Cmd* cmd) {
  Cmd* args = cmd->args;
  int x0 = argint(&args);
  int y0 = argint(&args);
  char ret[52] = "";  retCmd(INT, catargint(sus(x0, y0)));
}

Cmd* bind_mult(Cmd* cmd) {
  Cmd* args = cmd->args;
  int x0 = argint(&args);
  int y0 = argint(&args);
  char ret[52] = "";  retCmd(INT, catargint(mult(x0, y0)));
}

Cmd* bind_divide(Cmd* cmd) {
  Cmd* args = cmd->args;
  int x0 = argint(&args);
  int y0 = argint(&args);
  char ret[52] = "";  retCmd(INT, catargint(divide(x0, y0)));
}

