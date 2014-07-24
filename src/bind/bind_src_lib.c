#include "bind_src_lib.h"

void initCFunctions(LoadedDef* d) {
  addLoadedDef(d, "add", CFUNCTION, bind_add);
  addLoadedDef(d, "sus", CFUNCTION, bind_sus);
  addLoadedDef(d, "mult", CFUNCTION, bind_mult);
  addLoadedDef(d, "divide", CFUNCTION, bind_divide);
  addLoadedDef(d, "straddch", CFUNCTION, bind_straddch);
}

int add(int x, int y);
int sus(int x, int y);
int mult(int x, int y);
int divide(int x, int y);
char* straddch(char* str, char c);

Val* bind_add(Val* args) {
  Val* m; VarType t[] = {varType(INT, 0, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  int x_ = GET_VAL(int, args);
  int y_ = GET_VAL(int, args);
  int r = add(x_, y_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_sus(Val* args) {
  Val* m; VarType t[] = {varType(INT, 0, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  int x_ = GET_VAL(int, args);
  int y_ = GET_VAL(int, args);
  int r = sus(x_, y_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_mult(Val* args) {
  Val* m; VarType t[] = {varType(INT, 0, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  int x_ = GET_VAL(int, args);
  int y_ = GET_VAL(int, args);
  int r = mult(x_, y_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_divide(Val* args) {
  Val* m; VarType t[] = {varType(INT, 0, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  int x_ = GET_VAL(int, args);
  int y_ = GET_VAL(int, args);
  int r = divide(x_, y_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_straddch(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(CHAR, 0, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  char* str_ = GET_PTR(char*, args);
  char c_ = GET_VAL(char, args);
  char* r = straddch(str_, c_);
  return initVal(varType(CHAR, 1, 0), r);
}

