#ifndef MODEL_H
#define MODEL_H

struct Func {
  char name[52];
  char body[512];
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
