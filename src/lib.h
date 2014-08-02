#ifndef LIB_H
#define LIB_H

typedef struct IntPair {
  int x;
  int y;
} IntPair;

//IntPair add2(IntPair p1, IntPair p2);
int add(int x, int y);
int sus(int x, int y);
int mult(int x, int y);
int divide(int x, int y);
char* straddch(char* str, char c);
char* strdelch(char* str);
void strinsertch(char* cursor, char c);
char* trimCEnd(char* s);
char* trimEnd(char* s);
char* trim(char* s);
char* replace(char* str, char a, char b);
int isFloat(char* str);
int isInteger(char* str);

#endif // LIB_H
