#ifndef BIND_H
#define BIND_H

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

#include "../../model.h"
#include "glue.h"

void initCFunctions(LoadedDef* d);

Val* bind_isInteger(Val* args);
Val* bind_isFloat(Val* args);
Val* bind_replace(Val* args);
Val* bind_trim(Val* args);
Val* bind_trimEnd(Val* args);
Val* bind_trimCEnd(Val* args);
Val* bind_strinsertch(Val* args);
Val* bind_strdelch(Val* args);
Val* bind_straddch(Val* args);
Val* bind_divide(Val* args);
Val* bind_mult(Val* args);
Val* bind_sus(Val* args);
Val* bind_add(Val* args);
Val* bind_startsWith(Val* args);
#endif // BIND_H