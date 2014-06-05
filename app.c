#include "stdlib.h"
#include "stdio.h"

int add(int x, int y) {
  return x + y;
}

int main(int argc, char* argv[]) {
  int arg1;
  int arg2;

  if (argc != 3) {
    fprintf(stderr, "Invalid amout of parameters.\n");
    return -1;
  }

  sscanf(argv[1],"%d",&arg1);
  sscanf(argv[2],"%d",&arg2);
  int r = add(arg1, arg2);
  printf("%d", r);

  return 0;
}
