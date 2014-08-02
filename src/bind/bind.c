#include "bind.h"

#include "../../src/lib.h"

void initCFunctions(LoadedDef* d) {
  addLoadedDef(d, createFunc("isInteger", createAttr("str", varType(CHAR, 1, 0), NULL)), CFUNCTION, bind_isInteger);
  addLoadedDef(d, createFunc("isFloat", createAttr("str", varType(CHAR, 1, 0), NULL)), CFUNCTION, bind_isFloat);
  addLoadedDef(d, createFunc("replace", createAttr("str", varType(CHAR, 1, 0), createAttr("a", varType(CHAR, 0, 0), createAttr("b", varType(CHAR, 0, 0), NULL)))), CFUNCTION, bind_replace);
  addLoadedDef(d, createFunc("trim", createAttr("s", varType(CHAR, 1, 0), NULL)), CFUNCTION, bind_trim);
  addLoadedDef(d, createFunc("trimEnd", createAttr("s", varType(CHAR, 1, 0), NULL)), CFUNCTION, bind_trimEnd);
  addLoadedDef(d, createFunc("trimCEnd", createAttr("s", varType(CHAR, 1, 0), NULL)), CFUNCTION, bind_trimCEnd);
  addLoadedDef(d, createFunc("strinsertch", createAttr("cursor", varType(CHAR, 1, 0), createAttr("c", varType(CHAR, 0, 0), NULL))), CFUNCTION, bind_strinsertch);
  addLoadedDef(d, createFunc("strdelch", createAttr("str", varType(CHAR, 1, 0), NULL)), CFUNCTION, bind_strdelch);
  addLoadedDef(d, createFunc("straddch", createAttr("str", varType(CHAR, 1, 0), createAttr("c", varType(CHAR, 0, 0), NULL))), CFUNCTION, bind_straddch);
  addLoadedDef(d, createFunc("divide", createAttr("x", varType(INT, 0, 0), createAttr("y", varType(INT, 0, 0), NULL))), CFUNCTION, bind_divide);
  addLoadedDef(d, createFunc("mult", createAttr("x", varType(INT, 0, 0), createAttr("y", varType(INT, 0, 0), NULL))), CFUNCTION, bind_mult);
  addLoadedDef(d, createFunc("sus", createAttr("x", varType(INT, 0, 0), createAttr("y", varType(INT, 0, 0), NULL))), CFUNCTION, bind_sus);
  addLoadedDef(d, createFunc("add", createAttr("x", varType(INT, 0, 0), createAttr("y", varType(INT, 0, 0), NULL))), CFUNCTION, bind_add);
}

int isInteger(char* str);
int isFloat(char* str);
char* replace(char* str, char a, char b);
char* trim(char* s);
char* trimEnd(char* s);
char* trimCEnd(char* s);
void strinsertch(char* cursor, char c);
char* strdelch(char* str);
char* straddch(char* str, char c);
int divide(int x, int y);
int mult(int x, int y);
int sus(int x, int y);
int add(int x, int y);

Val* bind_isInteger(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0)};
  if ((m = checkSignature(args, t, 1)) != NULL) return m;
  char* str_ = GET_PTR(char*, args);
  int r = isInteger(str_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_isFloat(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0)};
  if ((m = checkSignature(args, t, 1)) != NULL) return m;
  char* str_ = GET_PTR(char*, args);
  int r = isFloat(str_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_replace(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(CHAR, 0, 0), varType(CHAR, 0, 0)};
  if ((m = checkSignature(args, t, 3)) != NULL) return m;
  char* str_ = GET_PTR(char*, args);
  char a_ = GET_VAL(char, args);
  char b_ = GET_VAL(char, args);
  char* r = replace(str_, a_, b_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_trim(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0)};
  if ((m = checkSignature(args, t, 1)) != NULL) return m;
  char* s_ = GET_PTR(char*, args);
  char* r = trim(s_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_trimEnd(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0)};
  if ((m = checkSignature(args, t, 1)) != NULL) return m;
  char* s_ = GET_PTR(char*, args);
  char* r = trimEnd(s_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_trimCEnd(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0)};
  if ((m = checkSignature(args, t, 1)) != NULL) return m;
  char* s_ = GET_PTR(char*, args);
  char* r = trimCEnd(s_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_strinsertch(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(CHAR, 0, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  char* cursor_ = GET_PTR(char*, args);
  char c_ = GET_VAL(char, args);
  strinsertch(cursor_, c_);
  return NULL;
}

Val* bind_strdelch(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0)};
  if ((m = checkSignature(args, t, 1)) != NULL) return m;
  char* str_ = GET_PTR(char*, args);
  char* r = strdelch(str_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_straddch(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(CHAR, 0, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  char* str_ = GET_PTR(char*, args);
  char c_ = GET_VAL(char, args);
  char* r = straddch(str_, c_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_divide(Val* args) {
  Val* m; VarType t[] = {varType(INT, 0, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  int x_ = GET_VAL(int, args);
  int y_ = GET_VAL(int, args);
  int r = divide(x_, y_);
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

Val* bind_sus(Val* args) {
  Val* m; VarType t[] = {varType(INT, 0, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  int x_ = GET_VAL(int, args);
  int y_ = GET_VAL(int, args);
  int r = sus(x_, y_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_add(Val* args) {
  Val* m; VarType t[] = {varType(INT, 0, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  int x_ = GET_VAL(int, args);
  int y_ = GET_VAL(int, args);
  int r = add(x_, y_);
  return initVal(varType(INT, 0, 0), &r);
}

