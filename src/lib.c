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

char* strdelch(char* str) {
  int i = strlen(str);
  if (i > 0) {
    str[i-1] = '\0';
  }
  return str;
}

void strinsertch(char* cursor, char c) { //FIXME: Not buffer safe
  char* s = cursor;
  char n = *s;
  char n2;
  while (n != '\0') {
    n2 = *(++s);
    *s = n;
    n = n2;
  }
  *cursor = c;
}

char* trimCEnd(char* s) {
  char* init = s;
  s = s + strlen(s) - 1;
  while (s != init) {
    if (*s == ' ' || *s == '\t' || *s == ';') {
      *s = '\0';
    } else {
      break;
    }
    --s;
  }
  return init;
}
char* trimEnd(char* s) {
  char* init = s;
  s = s + strlen(s) - 1;
  while (s != init) {
    if (*s == ' ' || *s == '\t') {
      *s = '\0';
    } else {
      break;
    }
    --s;
  }
  return init;
}

// Return a pointer to the first non whitespace char of the string.
char* trim(char* s) {
  char* c = s;
  while (*c != '\0' && (*c == ' ' || *c == '\n')) {
    c++;
  }
  return c;
}

// APP

