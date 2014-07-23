#include "bind_src_lib.h"

void initCFunctions(LoadedDef* d) {
  addLoadedDef(d, "add", CFUNCTION, bind_add);
  //addLoadedDef(d, "sus", CFUNCTION, bind_sus);
  //addLoadedDef(d, "mult", CFUNCTION, bind_mult);
  //addLoadedDef(d, "divide", CFUNCTION, bind_divide);
  //addLoadedDef(d, "straddch", CFUNCTION, bind_straddch);
}

int add(int x, int y);
int sus(int x, int y);
int mult(int x, int y);
int divide(int x, int y);
char* straddch(char* str, char c);

Val* bind_add(Cmd* cmd) {
  Cmd* args = cmd->args;
  Val* m; CmdType t[] = {OLD_INT, OLD_INT};
  if ((m = checkSignature(cmd->args, t, 2)) != NULL) return m;
  int x_ = argint(args);
  int y_ = argint(nxtCmd(&args));
  int r = add(x_, y_);
  return initVal(varType(INT, 0, 0), &r);
}

/*Val* bind_sus(Cmd* cmd) {
  Cmd* args = cmd->args;
  char r[52] = ""; Cmd* m; CmdType t[] = {INT, INT};
  if ((m = checkSignature(cmd->args, t, 2)) != NULL) return m;
  int x_ = argint(args);
  int y_ = argint(nxtCmd(&args));
  return initCmd(INT, cat_argint(r, sus(x_, y_)), NULL);
}

Val* bind_mult(Cmd* cmd) {
  Cmd* args = cmd->args;
  char r[52] = ""; Cmd* m; CmdType t[] = {INT, INT};
  if ((m = checkSignature(cmd->args, t, 2)) != NULL) return m;
  int x_ = argint(args);
  int y_ = argint(nxtCmd(&args));
  return initCmd(INT, cat_argint(r, mult(x_, y_)), NULL);
}

Val* bind_divide(Cmd* cmd) {
  Cmd* args = cmd->args;
  char r[52] = ""; Cmd* m; CmdType t[] = {INT, INT};
  if ((m = checkSignature(cmd->args, t, 2)) != NULL) return m;
  int x_ = argint(args);
  int y_ = argint(nxtCmd(&args));
  return initCmd(INT, cat_argint(r, divide(x_, y_)), NULL);
}

Cmd* bind_straddch(Cmd* cmd) {
  Cmd* args = cmd->args;
  char r[52] = ""; Cmd* m; CmdType t[] = {INT, INT};
  if ((m = checkSignature(cmd->args, t, 2)) != NULL) return m;
  char* str_ = (char*)argptr(args);
  char c_ = argchar(nxtCmd(&args));
  return initCmd(INT, cat_argptr(r, straddch(str_, c_)), NULL);
}
*/
