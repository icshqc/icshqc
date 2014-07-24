#ifndef BIND_H
#define BIND_H

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

#include "../../model.h"
#include "glue.h"

void initCFunctions(LoadedDef* d);

Val* bind_add(Val* cmd);
Val* bind_sus(Val* cmd);
Val* bind_mult(Val* cmd);
Val* bind_divide(Val* cmd);
Val* bind_straddch(Val* cmd);
#endif // BIND_H
