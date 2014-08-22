#ifndef MODEL_H
#define MODEL_H

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

enum CmdType {UNKOWN, VAR, OPERATOR, CFUNCTION, FUNCTION, VALUE, STRING, PAIR, VAR_NAME, NIL,
              POINTER, ERROR, OLD_MACRO, MACRO_OP, ARRAY, BLOCK_ARG, BLOCK, TYPE, OLD_TUPLE, OLD_INT,
              OLD_FLOAT, OLD_CHAR};
typedef enum CmdType CmdType;

struct Type;

// When it is a tuple, the addr contains a linked list of Val.
enum PrimVarType {
  INT, CHAR, FLOAT, VOID, TUPLE, STRUCT // Tuple and struct is the same thing right???
};
typedef enum PrimVarType PrimVarType;

// Probablement devrait etre une union au lieu d'une struct.
struct VarType {
  struct Type* typeStruct;
  PrimVarType type;
  unsigned int ptr; // 1 for pointer, 2 for pointer of a pointer...
  unsigned char isConst;
  unsigned char isUnsigned; // -1 = signed, 0 not specified, 1 = unsigned
  unsigned int arraySize; // 0 if not array
};
typedef struct VarType VarType;

enum ValOptions {
  VAL_ERROR = 0x01,
  VAL_UNKOWN = 0x02,
  VAL_BLOCK = 0x04,
  //OPT_D = 0x08,
  //OPT_E = 0x10,
  //OPT_F = 0x20,

  // Maybe one field for wheter the val needs to be freed or not.
  // Like if it belongs to a var then no, else yes.
};

struct Val {
  VarType type;
  void* addr;
  enum ValOptions options;
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
  // TODO: VarType ret;
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
