#include <string.h>

void main(int argc, char* argv[]) {
  // FLAGS
  int debug_mode = 0;

  int i;
  for (i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-d") == 0) {
      debug_mode = 1;
    }
  }

  // TODO: list src/bind and add all the source files there.
  char cmd[256] = "gcc -o build/icsh ";
  if (debug_mode) {
    strcat(cmd, "-g ");
  }
  strcat(cmd, "lib.c src/bind/bind_lib.c src/bind/glue.c main.c");
  strcat(cmd, " -lncurses");
  if (debug_mode) {
    strcat(cmd, " && gdb build/debug");
  } else {
    strcat(cmd, " && ./build/icsh");
  }
  system(cmd);
}
