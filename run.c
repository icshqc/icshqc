#include <string.h>

void main(int argc, char* argv[]) {
  // TODO: list src/bind and add all the source files there.
  char cmd[256] = "gcc -o build/icsh ";
  strcat(cmd, "lib.c src/bind/bind_lib.c src/bind/glue.c main.c");
  strcat(cmd, " -lncurses");
  system(cmd);
  system("./build/icsh");
//gcc -g -o build/debug lib.c src/bind/bind.c src/bind/glue.c main.c -lncurses && gdb build/debug
}
