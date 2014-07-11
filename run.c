#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string.h>
#include <dirent.h>

#include <stdlib.h>
#include <stdio.h>

void includeFilesR(char* cmd, char* dirname) {
  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir (dirname)) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
        char path[272] = "";
        strcpy(path, dirname);
        if (dirname[strlen(dirname)-1] != '/') {
          strcat(path, "/");
        }
        strcat(path, ent->d_name);
        struct stat s;
        if (stat(path,&s) == 0) {
          if (s.st_mode & S_IFDIR) { // it's a directory
            includeFilesR(cmd, path);
          } else if (s.st_mode & S_IFREG) { // it's a file
            if (path[strlen(path)-2] == '.' && path[strlen(path)-1] == 'c') {
              strcat(cmd, path);
              strcat(cmd, " ");
            }
          }
        }
      }
    }
    closedir(dir);
  }
}

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

  char cmd[256] = "gcc -o ";
  if (debug_mode) {
    strcat(cmd, "build/debug -g ");
  } else {
    strcat(cmd, "build/icsh ");
  }

  // TODO: list src/bind and add all the source files there.
  includeFilesR(cmd, "src");

  strcat(cmd, " main.c");
  strcat(cmd, " -lncurses");
  strcat(cmd, " -lSDL");
  if (debug_mode) {
    strcat(cmd, " && gdb build/debug");
  } else {
    strcat(cmd, " && screen ./build/icsh");
  }
  if (debug_command_mode) {
    printf("%s\n", cmd);
  } else {
    system(cmd);
  }
}
