#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Tes.h"

void freeTes(Tes* a) {
  if (a != NULL) {
    free(a);
  }
}

Tes* newTes() {
  Tes* a = malloc(sizeof(Tes));
  if (a == NULL) {
    abort(); // FIXME msg: Can't allocate memory
  }
  return a;
}

char* catTes(char* m, Tes* a) {
  if (a != NULL) {
    strcat(m, "Tes[\"");
    strcat(m, "\"]");
  }
  return m;
}
