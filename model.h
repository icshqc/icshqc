#ifndef MODEL_H
#define MODEL_H

struct Arg {
  char val[52];
  struct Arg* nxt;
};
typedef struct Arg Arg;

struct Func {
  char name[52];
  Arg* args;
  struct Func* nxt;
};
typedef struct Func Func;

#endif // MODEL.h
