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

void bind_straddch(Cmd* cmd);
void bind_strdelch(Cmd* cmd);
//void msg(Cmd* cmd);
//void output(Cmd* cmd);
void bind_trim(Cmd* cmd);

#endif // BIND_H
