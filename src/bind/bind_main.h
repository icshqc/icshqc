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

Cmd* bind_isCValue(Cmd* cmd);
Cmd* bind_newCFunc(Cmd* cmd);
Cmd* bind_newFunc(Cmd* cmd);
Cmd* bind_newArg(Cmd* cmd);
Cmd* bind_appendNewArg(Cmd* cmd);
Cmd* bind_freeVar(Cmd* cmd);
Cmd* bind_freeType(Cmd* cmd);
Cmd* bind_freeLoadedDef(Cmd* cmd);
Cmd* bind_freeArg(Cmd* cmd);
Cmd* bind_freeCFunc(Cmd* cmd);
Cmd* bind_freeCmd(Cmd* cmd);
Cmd* bind_freeFunc(Cmd* cmd);
Cmd* bind_setVarVal(Cmd* cmd);
Cmd* bind_straddch(Cmd* cmd);
Cmd* bind_strdelch(Cmd* cmd);
Cmd* bind_msg(Cmd* cmd);
Cmd* bind_output(Cmd* cmd);
Cmd* bind_outputStr(Cmd* cmd);
Cmd* bind_trim(Cmd* cmd);
Cmd* bind_catCmdType(Cmd* cmd);
Cmd* bind_catCmd(Cmd* cmd);
Cmd* bind_printCmd(Cmd* cmd);
Cmd* bind_catVar(Cmd* cmd);
Cmd* bind_varByName(Cmd* cmd);
Cmd* bind_typeByName(Cmd* cmd);
Cmd* bind_loadedFuncByName(Cmd* cmd);
Cmd* bind_isFloat(Cmd* cmd);
Cmd* bind_isInteger(Cmd* cmd);
Cmd* bind_typeCmd(Cmd* cmd);
Cmd* bind_parsePair(Cmd* cmd);
Cmd* bind_parseCmdR(Cmd* cmd);
#endif // BIND_H