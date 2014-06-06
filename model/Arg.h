#ifndef ARG_H
#define ARG_H

struct Arg {
  char val[52];
  struct Arg* nxt;
};
typedef struct Arg Arg;

#endif // ARG_h
