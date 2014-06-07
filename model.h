#ifndef MODEL_H
#define MODEL_H

struct Lambda {
  Arg* args;
  char body[256];
};
typedef struct Lambda Lambda;

struct Func {
  char name[52];
  Lambda* lambda;
  Arg* args;
  //Arg ret;
  struct Func* nxt;
};
typedef struct Func Func;

struct Alias {
  char name[52];
  Func* func;
  struct Alias* nxt;
};
typedef struct Alias Alias;

#endif // MODEL.h
