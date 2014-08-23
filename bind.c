#include "bind.h"

void initCFunctions(LoadedDef* d, Type* t) {
  addType(createType("struct IntPair", createAttr("x", varType(INT,0,0), createAttr("y", varType(INT,0,0), NULL))));

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
  addLoadedDef(d, createFunc("startsWith", createAttr("mustEqual", varType(CHAR, 1, 0), createAttr("str1", varType(CHAR, 1, 0), NULL))), CFUNCTION, bind_startsWith);
  addLoadedDef(d, createFunc("add2", createAttr("p1", typeStruct("struct IntPair", 0, 0), createAttr("p2", typeStruct("struct IntPair", 0, 0), NULL))), CFUNCTION, bind_add2);
}

Val* bind_isInteger(Val* args) {
  char* str_ = GET_PTR(char*, args);
  int r = isInteger(str_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_isFloat(Val* args) {
  char* str_ = GET_PTR(char*, args);
  int r = isFloat(str_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_replace(Val* args) {
  char* str_ = GET_PTR(char*, args);
  char a_ = GET_VAL(char, args);
  char b_ = GET_VAL(char, args);
  char* r = replace(str_, a_, b_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_trim(Val* args) {
  char* s_ = GET_PTR(char*, args);
  char* r = trim(s_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_trimEnd(Val* args) {
  char* s_ = GET_PTR(char*, args);
  char* r = trimEnd(s_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_trimCEnd(Val* args) {
  char* s_ = GET_PTR(char*, args);
  char* r = trimCEnd(s_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_strinsertch(Val* args) {
  char* cursor_ = GET_PTR(char*, args);
  char c_ = GET_VAL(char, args);
  strinsertch(cursor_, c_);
  return NULL;
}

Val* bind_strdelch(Val* args) {
  char* str_ = GET_PTR(char*, args);
  char* r = strdelch(str_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_straddch(Val* args) {
  char* str_ = GET_PTR(char*, args);
  char c_ = GET_VAL(char, args);
  char* r = straddch(str_, c_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_divide(Val* args) {
  int x_ = GET_VAL(int, args);
  int y_ = GET_VAL(int, args);
  int r = divide(x_, y_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_mult(Val* args) {
  int x_ = GET_VAL(int, args);
  int y_ = GET_VAL(int, args);
  int r = mult(x_, y_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_sus(Val* args) {
  int x_ = GET_VAL(int, args);
  int y_ = GET_VAL(int, args);
  int r = sus(x_, y_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_add(Val* args) {
  int x_ = GET_VAL(int, args);
  int y_ = GET_VAL(int, args);
  int r = add(x_, y_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_startsWith(Val* args) {
  char* mustEqual_ = GET_PTR(char*, args);
  char* str1_ = GET_PTR(char*, args);
  int r = startsWith(mustEqual_, str1_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_add2(Val* args) {
  IntPair p1_ = GET_VAL(IntPair, args);
  IntPair p2_ = GET_VAL(IntPair, args);
  IntPair r = add2(p1_, p2_);
  return initVal(typeStruct("struct IntPair", 0, 0), &r);
}

