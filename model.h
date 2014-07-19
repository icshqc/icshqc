#ifndef MODEL_H
#define MODEL_H

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

enum CmdType {UNKOWN, VOID, VAR, EDITOR, OPERATOR, CFUNCTION, FUNCTION, INT, STRING, PAIR, VAR_NAME, NIL,
              BOOL, CHAR, POINTER, FLOAT, ERROR, MACRO, MACRO_OP, ARRAY, BLOCK_ARG, BLOCK, TYPE, TUPLE};
typedef enum CmdType CmdType;

struct Arg {
  char name[52];
  char type[52];
  struct Arg* nxt;
};
typedef struct Arg Arg;

struct CStruct { // Maybe just use Type instead of CStruct
  char name[52];
  Arg* attrs;
  struct CStruct* nxt;
};
typedef struct CStruct CStruct;

struct CFunc {
  char name[52];
  char ret[52];
  Arg* args;
  struct CFunc* nxt;
};
typedef struct CFunc CFunc;

struct Cmd {
  CmdType type;
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

struct Type;

struct Attr {
  char name[32];
  struct Type* type;
  struct Attr* nxt;
};
typedef struct Attr Attr;

struct Type {
  char name[32];
  Attr* attrs;
  struct Type* nxt;
};
typedef struct Type Type;

struct Var {
  char name[32];
  Type* type;
  Cmd* val;
  struct Var* nxt;
};
typedef struct Var Var;

struct Func {
  char name[52];
  char ret[52];
  int isOperator;
  Arg* args;
  Cmd* cmd;
  struct Func* nxt;
};
typedef struct Func Func;

#endif // MODEL.h
