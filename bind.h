#ifndef BIND_H
#define BIND_H

#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <signal.h>
#include <string.h>

#include "model.h"
#include "glue.h"

void initCFunctions(LoadedDef* d);

void bind_add(Cmd* cmd);
void bind_sus(Cmd* cmd);
void bind_mult(Cmd* cmd);
void bind_divide(Cmd* cmd);
#endif // BIND_H
