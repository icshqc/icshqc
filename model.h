#ifndef MODEL_H
#define MODEL_H

struct Arg {
  char val[52];
  struct Arg* nxt;
};
typedef struct Arg Arg;

struct Func {
  char name[52];
  char body[512];
  Arg* args;
  Arg ret;
  struct Func* nxt;
};
typedef struct Func Func;

#endif // MODEL.h