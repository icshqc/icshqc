#ifndef MODEL_H
#define MODEL_H

struct Arg {
  char val[52];
  struct Arg* nxt;
};
typedef struct Arg Arg;

struct Cmd { // x (y z) (a (b c d))
  char name[52];
  struct Cmd* nxt;
  struct Cmd* args;
};
typedef struct Cmd Cmd;

struct Lambda {
  Arg* args;      // x y z
  char body[256];
};
typedef struct Lambda Lambda;

struct LoadedFunc {
  char name[52];
  bool isOperator;
  void (*ptr)(Cmd* cmd);
  struct LoadedFunc* nxt;
};
typedef struct LoadedFunc LoadedFunc;

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

/*struct FuncCall {
  Func* func;
  Lambda* lambda;
  Arg* args;      // 4 9 3
};*/

struct Alias {
  char name[52];
  Func* func;
  struct Alias* nxt;
};
typedef struct Alias Alias;

#endif // MODEL.h
