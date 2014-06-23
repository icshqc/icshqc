#ifndef GLUE_H
#define GLUE_H

#include "model.h"

char* argstring(Cmd** cmd);
char argchar(Cmd** cmd);

LoadedDef* lastLoadedDef();
LoadedDef* createLoadedDef(char* name, int isOp, void (*ptr)(Cmd* cmd));
LoadedDef* addLoadedDef(LoadedDef* def, char* name, int priority, void (*ptr)(Cmd* cmd));

#endif // GLUE_H
