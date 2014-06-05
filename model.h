#ifndef MODEL_H
#define MODEL_H

struct Arg {
  char val[52];
  struct Arg* nxt;
};
typedef struct Arg Arg;

struct OldFonc {
  char name[52];
  Arg* args;
  struct OldFonc* nxt;
};
typedef struct OldFonc OldFonc;

#endif // MODEL.h
