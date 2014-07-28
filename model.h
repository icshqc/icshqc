#ifndef MODEL_H
#define MODEL_H

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

enum CmdType {UNKOWN, VAR, OPERATOR, CFUNCTION, FUNCTION, VALUE, STRING, PAIR, VAR_NAME, NIL,
              POINTER, ERROR, MACRO, MACRO_OP, ARRAY, BLOCK_ARG, BLOCK, TYPE, OLD_TUPLE, OLD_INT,
              OLD_FLOAT, OLD_CHAR};
typedef enum CmdType CmdType;

struct Type;

// When it is a tuple, the addr contains a linked list of Val.
enum PrimVarType {
  INT, CHAR, FLOAT, UNDEFINED, VOID, ERR, TUPLE, STRUCT
};
typedef enum PrimVarType PrimVarType;

// Probablement devrait etre une union au lieu d'une struct.
struct VarType {
  char raw_type[64]; // to handle types like (fd_set *__restrict)
  struct Type* typeStruct;
  PrimVarType type;
  int ptr; // 1 for pointer, 2 for pointer of a pointer...
  unsigned char isConst;
  unsigned char isUnsigned; // -1 = signed, 0 not specified, 1 = unsigned
  unsigned int arraySize; // 0 if not array
};
typedef struct VarType VarType;

struct Val {
  VarType type;
  void* addr;
  struct Val* nxt;
};
typedef struct Val Val;

struct Attr {
  char name[64];
  struct VarType type;
  struct Attr* nxt;
};
typedef struct Attr Attr;

struct TypeDef {
  char name[64];
  VarType type;
  struct TypeDef* nxt;
};
typedef struct TypeDef TypeDef;

struct Type {
  char name[64];
  Attr* attrs;
  struct Type* nxt;
};
typedef struct Type Type;

struct CFunc {
  char name[52];
  VarType ret;
  Attr* args;
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

struct Func {
  char name[52];
  int isOperator;
  Attr* args;
  Val* cmd;
  struct Func* nxt;
};
typedef struct Func Func;

struct LoadedDef {
  CmdType type; // FUNCTION || OPERATOR || CFUNCTION || MACRO || MACRO_OP
  Val* (*ptr)(Val* args);
  Func* func;
  struct LoadedDef* nxt;
};
typedef struct LoadedDef LoadedDef;

struct Var {
  char name[32];
  Type* type;
  Val* val;
  struct Var* nxt;
};
typedef struct Var Var;

#endif // MODEL.h
