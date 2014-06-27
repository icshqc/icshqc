#include "bind.h"

void initCFunctions(LoadedDef* d) {
  addLoadedDef(d, "isCValue", 0, bind_isCValue);
  addLoadedDef(d, "newCFunc", 0, bind_newCFunc);
  addLoadedDef(d, "newFunc", 0, bind_newFunc);
  addLoadedDef(d, "newArg", 0, bind_newArg);
  addLoadedDef(d, "appendNewArg", 0, bind_appendNewArg);
  addLoadedDef(d, "freeVar", 0, bind_freeVar);
  addLoadedDef(d, "freeType", 0, bind_freeType);
  addLoadedDef(d, "freeLoadedDef", 0, bind_freeLoadedDef);
  addLoadedDef(d, "freeArg", 0, bind_freeArg);
  addLoadedDef(d, "freeCFunc", 0, bind_freeCFunc);
  addLoadedDef(d, "freeCmd", 0, bind_freeCmd);
  addLoadedDef(d, "freeFunc", 0, bind_freeFunc);
  addLoadedDef(d, "setVarVal", 0, bind_setVarVal);
  addLoadedDef(d, "straddch", 0, bind_straddch);
  addLoadedDef(d, "strdelch", 0, bind_strdelch);
  addLoadedDef(d, "msg", 0, bind_msg);
  addLoadedDef(d, "output", 0, bind_output);
  addLoadedDef(d, "outputStr", 0, bind_outputStr);
  addLoadedDef(d, "trim", 0, bind_trim);
  addLoadedDef(d, "catCmdType", 0, bind_catCmdType);
  addLoadedDef(d, "catCmd", 0, bind_catCmd);
  addLoadedDef(d, "printCmd", 0, bind_printCmd);
  addLoadedDef(d, "catVar", 0, bind_catVar);
  addLoadedDef(d, "varByName", 0, bind_varByName);
  addLoadedDef(d, "typeByName", 0, bind_typeByName);
  addLoadedDef(d, "loadedFuncByName", 0, bind_loadedFuncByName);
  addLoadedDef(d, "isFloat", 0, bind_isFloat);
  addLoadedDef(d, "isInteger", 0, bind_isInteger);
  addLoadedDef(d, "typeCmd", 0, bind_typeCmd);
  addLoadedDef(d, "parsePair", 0, bind_parsePair);
  addLoadedDef(d, "parseCmdR", 0, bind_parseCmdR);
}

int isCValue(Cmd* cmd);
CFunc* newCFunc();
Func* newFunc();
Arg* newArg();
Arg* appendNewArg(Arg* arg);
void freeVar(Var* t);
void freeType(Type* t);
void freeLoadedDef(LoadedDef* d);
void freeArg(Arg* f);
void freeCFunc(CFunc* f);
void freeCmd(Cmd* arg);
void freeFunc(Func* f);
void setVarVal(Var* v, Cmd* val);
char* straddch(char* str, char c);
char* strdelch(char* str);
void msg(const char* str);
void output(const char* str);
Cmd* outputStr(const char* str);
char* trim(char* s);
char* catCmdType(char* b, CmdType t);
char* catCmd(char* b, Cmd* cmd);
void printCmd(Cmd* cmd);
void catVar(char* m, Var* v);
Var* varByName(char* name);
Type* typeByName(char* name);
LoadedDef* loadedFuncByName(char* name);
int isFloat(char* str);
int isInteger(char* str);
Cmd* typeCmd(Cmd* cmd);
ParsePair parsePair(Cmd* cmd, char* ptr);
ParsePair parseCmdR(char* command);

Cmd* bind_isCValue(Cmd* cmd) {
  Cmd* args = cmd->args;
  Cmd* cmd0 = argCmd*(&args);
  char ret[52] = "";
  retCmd(INT, cat_argint(ret, isCValue(cmd0)));
}

Cmd* bind_newCFunc(Cmd* cmd) {
  Cmd* args = cmd->args;
  char ret[52] = "";
  retCmd(INT, cat_argCFunc*(ret, newCFunc()));
}

Cmd* bind_newFunc(Cmd* cmd) {
  Cmd* args = cmd->args;
  char ret[52] = "";
  retCmd(INT, cat_argFunc*(ret, newFunc()));
}

Cmd* bind_newArg(Cmd* cmd) {
  Cmd* args = cmd->args;
  char ret[52] = "";
  retCmd(INT, cat_argArg*(ret, newArg()));
}

Cmd* bind_appendNewArg(Cmd* cmd) {
  Cmd* args = cmd->args;
  Arg* arg0 = argArg*(&args);
  char ret[52] = "";
  retCmd(INT, cat_argArg*(ret, appendNewArg(arg0)));
}

Cmd* bind_freeVar(Cmd* cmd) {
  Cmd* args = cmd->args;
  Var* t0 = argVar*(&args);
  char ret[52] = "";
  retCmd(INT, cat_argvoid(ret, freeVar(t0)));
}

Cmd* bind_freeType(Cmd* cmd) {
  Cmd* args = cmd->args;
  Type* t0 = argType*(&args);
  char ret[52] = "";
  retCmd(INT, cat_argvoid(ret, freeType(t0)));
}

Cmd* bind_freeLoadedDef(Cmd* cmd) {
  Cmd* args = cmd->args;
  LoadedDef* d0 = argLoadedDef*(&args);
  char ret[52] = "";
  retCmd(INT, cat_argvoid(ret, freeLoadedDef(d0)));
}

Cmd* bind_freeArg(Cmd* cmd) {
  Cmd* args = cmd->args;
  Arg* f0 = argArg*(&args);
  char ret[52] = "";
  retCmd(INT, cat_argvoid(ret, freeArg(f0)));
}

Cmd* bind_freeCFunc(Cmd* cmd) {
  Cmd* args = cmd->args;
  CFunc* f0 = argCFunc*(&args);
  char ret[52] = "";
  retCmd(INT, cat_argvoid(ret, freeCFunc(f0)));
}

Cmd* bind_freeCmd(Cmd* cmd) {
  Cmd* args = cmd->args;
  Cmd* arg0 = argCmd*(&args);
  char ret[52] = "";
  retCmd(INT, cat_argvoid(ret, freeCmd(arg0)));
}

Cmd* bind_freeFunc(Cmd* cmd) {
  Cmd* args = cmd->args;
  Func* f0 = argFunc*(&args);
  char ret[52] = "";
  retCmd(INT, cat_argvoid(ret, freeFunc(f0)));
}

Cmd* bind_setVarVal(Cmd* cmd) {
  Cmd* args = cmd->args;
  Var* v0 = argVar*(&args);
  Cmd* val0 = argCmd*(&args);
  char ret[52] = "";
  retCmd(INT, cat_argvoid(ret, setVarVal(v0, val0)));
}

Cmd* bind_straddch(Cmd* cmd) {
  Cmd* args = cmd->args;
  char* str0 = argstring(&args);
  char c0 = argchar(&args);
  char ret[52] = "";
  retCmd(INT, cat_argchar*(ret, straddch(str0, c0)));
}

Cmd* bind_strdelch(Cmd* cmd) {
  Cmd* args = cmd->args;
  char* str0 = argstring(&args);
  char ret[52] = "";
  retCmd(INT, cat_argchar*(ret, strdelch(str0)));
}

Cmd* bind_msg(Cmd* cmd) {
  Cmd* args = cmd->args;
  const char* str0 = argconst char*(&args);
  char ret[52] = "";
  retCmd(INT, cat_argvoid(ret, msg(str0)));
}

Cmd* bind_output(Cmd* cmd) {
  Cmd* args = cmd->args;
  const char* str0 = argconst char*(&args);
  char ret[52] = "";
  retCmd(INT, cat_argvoid(ret, output(str0)));
}

Cmd* bind_outputStr(Cmd* cmd) {
  Cmd* args = cmd->args;
  const char* str0 = argconst char*(&args);
  char ret[52] = "";
  retCmd(INT, cat_argCmd*(ret, outputStr(str0)));
}

Cmd* bind_trim(Cmd* cmd) {
  Cmd* args = cmd->args;
  char* s0 = argstring(&args);
  char ret[52] = "";
  retCmd(INT, cat_argchar*(ret, trim(s0)));
}

Cmd* bind_catCmdType(Cmd* cmd) {
  Cmd* args = cmd->args;
  char* b0 = argstring(&args);
  CmdType t0 = argCmdType(&args);
  char ret[52] = "";
  retCmd(INT, cat_argchar*(ret, catCmdType(b0, t0)));
}

Cmd* bind_catCmd(Cmd* cmd) {
  Cmd* args = cmd->args;
  char* b0 = argstring(&args);
  Cmd* cmd0 = argCmd*(&args);
  char ret[52] = "";
  retCmd(INT, cat_argchar*(ret, catCmd(b0, cmd0)));
}

Cmd* bind_printCmd(Cmd* cmd) {
  Cmd* args = cmd->args;
  Cmd* cmd0 = argCmd*(&args);
  char ret[52] = "";
  retCmd(INT, cat_argvoid(ret, printCmd(cmd0)));
}

Cmd* bind_catVar(Cmd* cmd) {
  Cmd* args = cmd->args;
  char* m0 = argstring(&args);
  Var* v0 = argVar*(&args);
  char ret[52] = "";
  retCmd(INT, cat_argvoid(ret, catVar(m0, v0)));
}

Cmd* bind_varByName(Cmd* cmd) {
  Cmd* args = cmd->args;
  char* name0 = argstring(&args);
  char ret[52] = "";
  retCmd(INT, cat_argVar*(ret, varByName(name0)));
}

Cmd* bind_typeByName(Cmd* cmd) {
  Cmd* args = cmd->args;
  char* name0 = argstring(&args);
  char ret[52] = "";
  retCmd(INT, cat_argType*(ret, typeByName(name0)));
}

Cmd* bind_loadedFuncByName(Cmd* cmd) {
  Cmd* args = cmd->args;
  char* name0 = argstring(&args);
  char ret[52] = "";
  retCmd(INT, cat_argLoadedDef*(ret, loadedFuncByName(name0)));
}

Cmd* bind_isFloat(Cmd* cmd) {
  Cmd* args = cmd->args;
  char* str0 = argstring(&args);
  char ret[52] = "";
  retCmd(INT, cat_argint(ret, isFloat(str0)));
}

Cmd* bind_isInteger(Cmd* cmd) {
  Cmd* args = cmd->args;
  char* str0 = argstring(&args);
  char ret[52] = "";
  retCmd(INT, cat_argint(ret, isInteger(str0)));
}

Cmd* bind_typeCmd(Cmd* cmd) {
  Cmd* args = cmd->args;
  Cmd* cmd0 = argCmd*(&args);
  char ret[52] = "";
  retCmd(INT, cat_argCmd*(ret, typeCmd(cmd0)));
}

Cmd* bind_parsePair(Cmd* cmd) {
  Cmd* args = cmd->args;
  Cmd* cmd0 = argCmd*(&args);
  char* ptr0 = argstring(&args);
  char ret[52] = "";
  retCmd(INT, cat_argParsePair(ret, parsePair(cmd0, ptr0)));
}

Cmd* bind_parseCmdR(Cmd* cmd) {
  Cmd* args = cmd->args;
  char* command0 = argstring(&args);
  char ret[52] = "";
  retCmd(INT, cat_argParsePair(ret, parseCmdR(command0)));
}

