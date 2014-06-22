#include "bind.h"

void initCFunctions(LoadedDef* d) {
}

char* straddch(char*, char);
char* strdelch(char*);
char* trim(char*);

void bind_straddch(Cmd* cmd) {
  Cmd* args = cmd->args;
  char* str0 = argstring(&args);
  char c0 = argchar(&args);
  char* ret = straddch(str0, c0);
}

void bind_strdelch(Cmd* cmd) {
  Cmd* args = cmd->args;
  char* str0 = argstring(&args);
  char* ret = strdelch(str0);
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
}
