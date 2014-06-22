#include "bind.h"

void initCFunctions(LoadedDef* d) {
  addLoadedDef(d, "straddch", 0, bind_straddch);
  addLoadedDef(d, "strdelch", 0, bind_strdelch);
  addLoadedDef(d, "trim", 0, bind_trim);
}

char* straddch(char*, char);
char* strdelch(char*);
char* trim(char*);

void bind_straddch(Cmd* cmd) {
  Cmd* args = cmd->args;
  char* str0 = argstring(&args);
  char c0 = argchar(&args);
  char* ret = straddch(str0, c0);
  addstr(ret);
}

void bind_strdelch(Cmd* cmd) {
  Cmd* args = cmd->args;
  char* str0 = argstring(&args);
  char* ret = strdelch(str0);
  addstr(ret);
}

/*void bind_msg(Cmd* cmd) {
  Cmd* args = cmd->args;
  const char* str0 = argconst char*(&args);
  void ret = msg(str0);
}

void bind_output(Cmd* cmd) {
  Cmd* args = cmd->args;
  const char* str0 = argconst char*(&args);
  void ret = output(str0);
}*/

void bind_trim(Cmd* cmd) {
  Cmd* args = cmd->args;
  char* s0 = argstring(&args);
  char* ret = trim(s0);
  addstr(ret);
}
