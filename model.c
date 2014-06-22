#include "model.h"

LoadedDef* lastLoadedDef(LoadedDef* d) {
  if (d == NULL) {
    return NULL;
  } else {
    LoadedDef* f;
    for (f = d; f->nxt != NULL; f = f->nxt) {}
    return f;
  }
}

LoadedDef* createLoadedDef(char* name, int isOp, void (*ptr)(Cmd* cmd)) {
  LoadedDef* d = malloc(sizeof(LoadedDef));
  strcpy(d->name, name);
  d->isOperator = isOp;
  d->ptr = ptr;
  d->nxt = NULL;
  return d;
}

