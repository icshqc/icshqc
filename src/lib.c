#include "lib.h"

#include <stdlib.h>
#include <string.h>

int add(int x, int y) {
  return x + y;
}

int sus(int x, int y) {
  return x - y;
}

int mult(int x, int y) {
  return x * y;
}

int divide(int x, int y) {
  return x / y;
}

char* straddch(char* str, char c) { //FIXME: Not buffer safe
  int i = strlen(str);
  str[i] = c;
  str[i+1] = '\0';
  return str;
}

