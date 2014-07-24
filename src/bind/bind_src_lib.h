#ifndef BIND_H
#define BIND_H

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

#include "../../model.h"
#include "glue.h"

void initCFunctions(LoadedDef* d);

Val* bind_add(Val* args);
Val* bind_sus(Val* args);
Val* bind_mult(Val* args);
Val* bind_divide(Val* args);
Val* bind_straddch(Val* args);
#endif // BIND_H