#ifndef BIND_H
#define BIND_H

#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <signal.h>
#include <string.h>

#include "../../model.h"
#include "glue.h"

void initCFunctions(LoadedDef* d);

Cmd* bind_add(Cmd* cmd);
Cmd* bind_sus(Cmd* cmd);
Cmd* bind_mult(Cmd* cmd);
Cmd* bind_divide(Cmd* cmd);
#endif // BIND_H
