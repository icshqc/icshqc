#ifndef MODEL_H
#define MODEL_H

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

enum CmdType {UNKOWN, VOID, VAR, EDITOR, OPERATOR, CFUNCTION, FUNCTION, INT, STRING, PAIR, VAR_NAME, NIL,
              BOOL, CHAR, POINTER, FLOAT, ERROR, MACRO, MACRO_OP, ARRAY, BLOCK_ARG, BLOCK, TYPE, TUPLE, VALUE};
typedef enum CmdType CmdType;

struct Type;

struct VarType {
  struct Type* type;
  int arraySize; // 0 if not array
  int ptr; // 1 for pointer, 2 for pointer of a pointer...
};
typedef struct VarType VarType;

struct Arg {
  char name[52];
  char type[52];
  struct Arg* nxt;
};
typedef struct Arg Arg;

struct Attr {
  char name[32];
  struct VarType type;
  struct Attr* nxt;
};
typedef struct Attr Attr;

struct Type {
  char name[32];
  Attr* attrs;
  struct Type* nxt;
};
typedef struct Type Type;

struct CFunc {
  char name[52];
  char ret[52];
  Arg* args;
  struct CFunc* nxt;
};
typedef struct CFunc CFunc;

struct Cmd {
  CmdType type;
  Type* valueType;
  char name[52];
  struct Cmd* nxt;
  struct Cmd* args;
};
typedef struct Cmd Cmd;

struct LoadedDef {
  char name[52];
  CmdType type; // FUNCTION || OPERATOR || CFUNCTION || MACRO || MACRO_OP
  Cmd* (*ptr)(Cmd* cmd);
  struct LoadedDef* nxt;
};
typedef struct LoadedDef LoadedDef;

struct Var {
  char name[32];
  Type* type;
  Cmd* val;
  struct Var* nxt;
};
typedef struct Var Var;

struct Func {
  char name[52];
  VarType ret;
  int isOperator;
  Arg* args;
  Cmd* cmd;
  struct Func* nxt;
};
typedef struct Func Func;

#endif // MODEL.h
