#ifndef GLUE_H
#define GLUE_H

#include "../../model.h"

#define GET_VAL(type_t, vals) (*((type_t*)(vals = vals->nxt)->addr))
#define GET_PTR(type_t, vals) ((type_t)(vals = vals->nxt)->addr)

char* argstring(Cmd* cmd);
char argchar(Cmd* cmd);
int argint(Cmd* cmd);
void* argptr(Cmd* cmd);

char* cat_argstring(char* b, char* s);
char* cat_argchar(char* b, char s);
char* cat_argint(char* b, int s);
char* cat_argptr(char* b, char* s);

char* catPrimVarTypeEnum(char* b, PrimVarType t);
char* catPrimVarType(char* b, PrimVarType t);
char* catVarType(char* b, VarType t);

Cmd* newCmd();
Cmd* initCmd(CmdType type, const char* val, Cmd* args);

Val* errorStr(char* str);

Type* typeByName(Type* types, char* name);

VarType varType(PrimVarType p, int ptr, int arraySize);

Val* cpyVal(Val* v);
Val* cpyVals(Val* v);
Val* initVal(VarType t, void* addr);
Val* initPtr(VarType t, void* addr);
Val* initArray(VarType t, void* addr);
void freeVal(Val* v);

Val* checkSignature(Val* args, VarType* types, int nArgs);
Val* checkSignatureAttrs(Val* args, Attr* attrs);

int validArg(Cmd* cmd, CmdType type);

LoadedDef* lastLoadedDef();
LoadedDef* createLoadedDef(Func* func, CmdType type, Val* (*ptr)(Val* cmd));
LoadedDef* addLoadedDef(LoadedDef* def, Func* func, CmdType type, Val* (*ptr)(Val* cmd));

Func* createFunc(char* name, Attr* attrs);
Attr* createAttr(char* name, VarType t, Attr* nxt);
Func* newFunc();
Attr* newAttr();

#endif // GLUE_H
