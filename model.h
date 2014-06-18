#ifndef MODEL_H
#define MODEL_H

struct Arg {
  char val[52];
  struct Arg* nxt;
};
typedef struct Arg Arg;

struct Cmd { // x (y z) (a (b c d))
  char name[128];
  struct Cmd* body; // If it is a block;
  struct Cmd* nxt;
  struct Cmd* args;
};
typedef struct Cmd Cmd;

struct Lambda {
  Arg* args;      // x y z
  char body[256];
};
typedef struct Lambda Lambda;

struct LoadedDef {
  char name[52];
  bool isOperator;
  void (*ptr)(Cmd* cmd);
  struct LoadedDef* nxt;
};
typedef struct LoadedDef LoadedDef;

struct Type {
  char name[32];
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
  bool isOperator;
  Lambda* lambda;
  Arg* args;      // int int int int
  //Arg ret;
  struct Func* nxt;
};
typedef struct Func Func;

#endif // MODEL.h
