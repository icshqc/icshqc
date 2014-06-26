#ifndef GLUE_H
#define GLUE_H

#include "model.h"

char* argstring(Cmd** cmd);
char argchar(Cmd** cmd);
int argint(Cmd** cmd);

char* cat_argstring(char* b, char* s);
char* cat_argchar(char* b, char s);
char* cat_argint(char* b, int s);

Cmd* newCmd();
Cmd* retCmd(CmdType type, char* name);

LoadedDef* lastLoadedDef();
LoadedDef* createLoadedDef(char* name, int isOp, Cmd* (*ptr)(Cmd* cmd));
LoadedDef* addLoadedDef(LoadedDef* def, char* name, int priority, Cmd* (*ptr)(Cmd* cmd));

#endif // GLUE_H
