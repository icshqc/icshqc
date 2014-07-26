#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string.h>

#include <stdlib.h>
#include <stdio.h>

void main(int argc, char* argv[]) {
  // FLAGS
  int debug_mode = 0;
  int debug_command_mode = 0;

  int i;
  for (i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-d") == 0) {
      debug_mode = 1;
    } else if (strcmp(argv[i], "-?") == 0) {
      debug_command_mode = 1;
    }
  }

  char cmd[256] = "gcc -Wall -o ";
  if (debug_mode) {
    strcat(cmd, "build/prc_debug -g ");
  } else {
    strcat(cmd, "build/prc ");
  }

  strcat(cmd, " src/lib.c");
  strcat(cmd, " prc.c");
  if (debug_mode) {
    strcat(cmd, " && gdb -q build/prc_debug");
  } else {
    strcat(cmd, " && ./build/prc");
  }
  if (debug_command_mode) {
    printf("%s\n", cmd);
  } else {
    system(cmd);
  }
}
