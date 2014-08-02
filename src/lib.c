#include "lib.h"

#include <stdlib.h>
#include <string.h>

IntPair add2(IntPair p1, IntPair p2) {
  IntPair p;
  p.x = p1.x + p2.x;
  p.y = p1.y + p2.y;
  return p;
}

int startsWith(char* mustEqual, char* str1) {
  return strncmp(mustEqual, str1, strlen(mustEqual)) == 0;
}

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
    if (*s == ' ' || *s == '\t' || *s == ';' || *s == '\n' || *s == '\r') {
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
    if (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') {
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
  while (*c != '\0' && (*c == ' ' || *c == '\n' || *s == '\r')) {
    c++;
  }
  return c;
}

char* replace(char* str, char a, char b) {
  char* s = str;
  for (;*s != '\0'; ++s) {
    if (*s == a) {
      *s = b;
    }
  }
  return str;
}

int isFloat(char* str) {
  char* c;
  int hasDecimal = 0;
  for (c = str; *c != '\0'; ++c) {
    if (*c == '.' || *c == ',') {
      if (hasDecimal) {
        return 0;
      }
      hasDecimal = 1;
    } else if (*c < '0' || *c > '9') {
      return 0;
    }
  }
  return hasDecimal ? 1 : 0;
}
int isInteger(char* str) {
  char* c;
  for (c = str; *c != '\0'; ++c) {
    if (*c < '0' || *c > '9') {
      return 0;
    }
  }
  return 1;
}

