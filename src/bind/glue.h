#ifndef GLUE_H
#define GLUE_H

#include "../../model.h"

char* argstring(Cmd* cmd);
char argchar(Cmd* cmd);
int argint(Cmd* cmd);
void* argptr(Cmd* cmd);

char* cat_argstring(char* b, char* s);
char* cat_argchar(char* b, char s);
char* cat_argint(char* b, int s);
char* cat_argptr(char* b, char* s);

Cmd* newCmd();
Cmd* initCmd(CmdType type, const char* val, Cmd* args);

Val* errorStr(char* str);

VarType varType(PrimVarType p, int ptr, int arraySize);

Val* initVal(VarType t, void* addr);
void freeVal(Val* v);

Cmd* nxtCmd(Cmd** cmd);
Val* checkSignature(Cmd* args, CmdType* types, int nArgs);

int validArg(Cmd* cmd, CmdType type);

LoadedDef* lastLoadedDef();
LoadedDef* createLoadedDef(char* name, CmdType type, Val* (*ptr)(Cmd* cmd));
LoadedDef* addLoadedDef(LoadedDef* def, char* name, CmdType type, Val* (*ptr)(Cmd* cmd));

#endif // GLUE_H
