#include "bind_lib.h"

void initCFunctions(LoadedDef* d) {
  addLoadedDef(d, "add", CFUNCTION, bind_add);
  addLoadedDef(d, "sus", CFUNCTION, bind_sus);
  addLoadedDef(d, "mult", CFUNCTION, bind_mult);
  addLoadedDef(d, "divide", CFUNCTION, bind_divide);
}

int add(int x, int y);
int sus(int x, int y);
int mult(int x, int y);
int divide(int x, int y);

Cmd* bind_add(Cmd* cmd) {
  Cmd* args = cmd->args;
  if (!validArg(&args, INT)) return errorStr("Invalid arg 1: Expected type INT");
  int x0 = argint(&args);
  if (!validArg(&args, INT)) return errorStr("Invalid arg 2: Expected type INT");
  int y0 = argint(&args);
  char ret[52] = "";
  retCmd(INT, cat_argint(ret, add(x0, y0)));
}

Cmd* bind_sus(Cmd* cmd) {
  Cmd* args = cmd->args;
  if (!validArg(&args, INT)) return errorStr("Invalid arg 1: Expected type INT");
  int x0 = argint(&args);
  if (!validArg(&args, INT)) return errorStr("Invalid arg 2: Expected type INT");
  int y0 = argint(&args);
  char ret[52] = "";
  retCmd(INT, cat_argint(ret, sus(x0, y0)));
}

Cmd* bind_mult(Cmd* cmd) {
  Cmd* args = cmd->args;
  if (!validArg(&args, INT)) return errorStr("Invalid arg 1: Expected type INT");
  int x0 = argint(&args);
  if (!validArg(&args, INT)) return errorStr("Invalid arg 2: Expected type INT");
  int y0 = argint(&args);
  char ret[52] = "";
  retCmd(INT, cat_argint(ret, mult(x0, y0)));
}

Cmd* bind_divide(Cmd* cmd) {
  Cmd* args = cmd->args;
  if (!validArg(&args, INT)) return errorStr("Invalid arg 1: Expected type INT");
  int x0 = argint(&args);
  if (!validArg(&args, INT)) return errorStr("Invalid arg 2: Expected type INT");
  int y0 = argint(&args);
  char ret[52] = "";
  retCmd(INT, cat_argint(ret, divide(x0, y0)));
}

