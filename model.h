#ifndef MODEL_H
#define MODEL_H

struct Lambda {
  Arg* args;      // x y z
  char body[256];
};
typedef struct Lambda Lambda;

struct Func {
  char name[52];
  Lambda* lambda;
  Arg* args;      // int int int int
  //Arg ret;
  struct Func* nxt;
};
typedef struct Func Func;

struct FuncCall {
  Func* func;
  Lambda* lambda;
  Arg* args;      // 4 9 3
};

struct Alias {
  char name[52];
  Func* func;
  struct Alias* nxt;
};
typedef struct Alias Alias;

#endif // MODEL.h
