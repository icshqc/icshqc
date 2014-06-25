#ifndef GLUE_H
#define GLUE_H

#include "model.h"

char* argstring(Cmd** cmd);
char argchar(Cmd** cmd);
int argint(Cmd** cmd);

LoadedDef* lastLoadedDef();
LoadedDef* createLoadedDef(char* name, int isOp, Cmd* (*ptr)(Cmd* cmd));
LoadedDef* addLoadedDef(LoadedDef* def, char* name, int priority, Cmd* (*ptr)(Cmd* cmd));

#endif // GLUE_H
