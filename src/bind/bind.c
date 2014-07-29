#include "bind.h"

#include "/usr/include/string.h"

void initCFunctions(LoadedDef* d) {
  addLoadedDef(d, createFunc("stpncpy", createAttr("__dest", varType(CHAR, 1, 0), createAttr("__src", varType(CHAR, 1, 0), createAttr("__n", varType(INT, 0, 0), NULL)))), CFUNCTION, bind_stpncpy);
  addLoadedDef(d, createFunc("__stpncpy", createAttr("__dest", varType(CHAR, 1, 0), createAttr("__src", varType(CHAR, 1, 0), createAttr("__n", varType(INT, 0, 0), NULL)))), CFUNCTION, bind___stpncpy);
  addLoadedDef(d, createFunc("stpcpy", createAttr("__dest", varType(CHAR, 1, 0), createAttr("__src", varType(CHAR, 1, 0), NULL))), CFUNCTION, bind_stpcpy);
  addLoadedDef(d, createFunc("__stpcpy", createAttr("__dest", varType(CHAR, 1, 0), createAttr("__src", varType(CHAR, 1, 0), NULL))), CFUNCTION, bind___stpcpy);
  addLoadedDef(d, createFunc("strsignal", createAttr("__sig", varType(INT, 0, 0), NULL)), CFUNCTION, bind_strsignal);
  addLoadedDef(d, createFunc("strsep", createAttr("__stringp", varType(CHAR, 2, 0), createAttr("__delim", varType(CHAR, 1, 0), NULL))), CFUNCTION, bind_strsep);
  addLoadedDef(d, createFunc("strncasecmp", createAttr("__s1", varType(CHAR, 1, 0), createAttr("__s2", varType(CHAR, 1, 0), createAttr("__n", varType(INT, 0, 0), NULL)))), CFUNCTION, bind_strncasecmp);
  addLoadedDef(d, createFunc("strcasecmp", createAttr("__s1", varType(CHAR, 1, 0), createAttr("__s2", varType(CHAR, 1, 0), NULL))), CFUNCTION, bind_strcasecmp);
  addLoadedDef(d, createFunc("ffs", createAttr("__i", varType(INT, 0, 0), NULL)), CFUNCTION, bind_ffs);
  addLoadedDef(d, createFunc("rindex", createAttr("__s", varType(CHAR, 1, 0), createAttr("__c", varType(INT, 0, 0), NULL))), CFUNCTION, bind_rindex);
  addLoadedDef(d, createFunc("index", createAttr("__s", varType(CHAR, 1, 0), createAttr("__c", varType(INT, 0, 0), NULL))), CFUNCTION, bind_index);
  addLoadedDef(d, createFunc("bcmp", createAttr("__s1", varType(VOID, 1, 0), createAttr("__s2", varType(VOID, 1, 0), createAttr("__n", varType(INT, 0, 0), NULL)))), CFUNCTION, bind_bcmp);
  addLoadedDef(d, createFunc("bzero", createAttr("__s", varType(VOID, 1, 0), createAttr("__n", varType(INT, 0, 0), NULL))), CFUNCTION, bind_bzero);
  addLoadedDef(d, createFunc("bcopy", createAttr("__src", varType(VOID, 1, 0), createAttr("__dest", varType(VOID, 1, 0), createAttr("__n", varType(INT, 0, 0), NULL)))), CFUNCTION, bind_bcopy);
  addLoadedDef(d, createFunc("__bzero", createAttr("__s", varType(VOID, 1, 0), createAttr("__n", varType(INT, 0, 0), NULL))), CFUNCTION, bind___bzero);
  addLoadedDef(d, createFunc("strerror_r", createAttr("__errnum", varType(INT, 0, 0), createAttr("__buf", varType(CHAR, 1, 0), createAttr("__buflen", varType(INT, 0, 0), NULL)))), CFUNCTION, bind_strerror_r);
  addLoadedDef(d, createFunc("strerror", createAttr("__errnum", varType(INT, 0, 0), NULL)), CFUNCTION, bind_strerror);
  addLoadedDef(d, createFunc("strnlen", createAttr("__string", varType(CHAR, 1, 0), createAttr("__maxlen", varType(INT, 0, 0), NULL))), CFUNCTION, bind_strnlen);
  addLoadedDef(d, createFunc("strlen", createAttr("__s", varType(CHAR, 1, 0), NULL)), CFUNCTION, bind_strlen);
  addLoadedDef(d, createFunc("strtok_r", createAttr("__s", varType(CHAR, 1, 0), createAttr("__delim", varType(CHAR, 1, 0), createAttr("__save_ptr", varType(CHAR, 2, 0), NULL)))), CFUNCTION, bind_strtok_r);
  addLoadedDef(d, createFunc("__strtok_r", createAttr("__s", varType(CHAR, 1, 0), createAttr("__delim", varType(CHAR, 1, 0), createAttr("__save_ptr", varType(CHAR, 2, 0), NULL)))), CFUNCTION, bind___strtok_r);
  addLoadedDef(d, createFunc("strtok", createAttr("__s", varType(CHAR, 1, 0), createAttr("__delim", varType(CHAR, 1, 0), NULL))), CFUNCTION, bind_strtok);
  addLoadedDef(d, createFunc("strstr", createAttr("__haystack", varType(CHAR, 1, 0), createAttr("__needle", varType(CHAR, 1, 0), NULL))), CFUNCTION, bind_strstr);
  addLoadedDef(d, createFunc("strpbrk", createAttr("__s", varType(CHAR, 1, 0), createAttr("__accept", varType(CHAR, 1, 0), NULL))), CFUNCTION, bind_strpbrk);
  addLoadedDef(d, createFunc("strspn", createAttr("__s", varType(CHAR, 1, 0), createAttr("__accept", varType(CHAR, 1, 0), NULL))), CFUNCTION, bind_strspn);
  addLoadedDef(d, createFunc("strcspn", createAttr("__s", varType(CHAR, 1, 0), createAttr("__reject", varType(CHAR, 1, 0), NULL))), CFUNCTION, bind_strcspn);
  addLoadedDef(d, createFunc("strrchr", createAttr("__s", varType(CHAR, 1, 0), createAttr("__c", varType(INT, 0, 0), NULL))), CFUNCTION, bind_strrchr);
  addLoadedDef(d, createFunc("strchr", createAttr("__s", varType(CHAR, 1, 0), createAttr("__c", varType(INT, 0, 0), NULL))), CFUNCTION, bind_strchr);
  addLoadedDef(d, createFunc("strndup", createAttr("__string", varType(CHAR, 1, 0), createAttr("__n", varType(INT, 0, 0), NULL))), CFUNCTION, bind_strndup);
  addLoadedDef(d, createFunc("strdup", createAttr("__s", varType(CHAR, 1, 0), NULL)), CFUNCTION, bind_strdup);
  addLoadedDef(d, createFunc("strxfrm", createAttr("__dest", varType(CHAR, 1, 0), createAttr("__src", varType(CHAR, 1, 0), createAttr("__n", varType(INT, 0, 0), NULL)))), CFUNCTION, bind_strxfrm);
  addLoadedDef(d, createFunc("strcoll", createAttr("__s1", varType(CHAR, 1, 0), createAttr("__s2", varType(CHAR, 1, 0), NULL))), CFUNCTION, bind_strcoll);
  addLoadedDef(d, createFunc("strncmp", createAttr("__s1", varType(CHAR, 1, 0), createAttr("__s2", varType(CHAR, 1, 0), createAttr("__n", varType(INT, 0, 0), NULL)))), CFUNCTION, bind_strncmp);
  addLoadedDef(d, createFunc("strcmp", createAttr("__s1", varType(CHAR, 1, 0), createAttr("__s2", varType(CHAR, 1, 0), NULL))), CFUNCTION, bind_strcmp);
  addLoadedDef(d, createFunc("strncat", createAttr("__dest", varType(CHAR, 1, 0), createAttr("__src", varType(CHAR, 1, 0), createAttr("__n", varType(INT, 0, 0), NULL)))), CFUNCTION, bind_strncat);
  addLoadedDef(d, createFunc("strcat", createAttr("__dest", varType(CHAR, 1, 0), createAttr("__src", varType(CHAR, 1, 0), NULL))), CFUNCTION, bind_strcat);
  addLoadedDef(d, createFunc("strncpy", createAttr("__dest", varType(CHAR, 1, 0), createAttr("__src", varType(CHAR, 1, 0), createAttr("__n", varType(INT, 0, 0), NULL)))), CFUNCTION, bind_strncpy);
  addLoadedDef(d, createFunc("strcpy", createAttr("__dest", varType(CHAR, 1, 0), createAttr("__src", varType(CHAR, 1, 0), NULL))), CFUNCTION, bind_strcpy);
  addLoadedDef(d, createFunc("memchr", createAttr("__s", varType(VOID, 1, 0), createAttr("__c", varType(INT, 0, 0), createAttr("__n", varType(INT, 0, 0), NULL)))), CFUNCTION, bind_memchr);
  addLoadedDef(d, createFunc("memcmp", createAttr("__s1", varType(VOID, 1, 0), createAttr("__s2", varType(VOID, 1, 0), createAttr("__n", varType(INT, 0, 0), NULL)))), CFUNCTION, bind_memcmp);
  addLoadedDef(d, createFunc("memset", createAttr("__s", varType(VOID, 1, 0), createAttr("__c", varType(INT, 0, 0), createAttr("__n", varType(INT, 0, 0), NULL)))), CFUNCTION, bind_memset);
  addLoadedDef(d, createFunc("memccpy", createAttr("__dest", varType(VOID, 1, 0), createAttr("__src", varType(VOID, 1, 0), createAttr("__c", varType(INT, 0, 0), createAttr("__n", varType(INT, 0, 0), NULL))))), CFUNCTION, bind_memccpy);
  addLoadedDef(d, createFunc("memmove", createAttr("__dest", varType(VOID, 1, 0), createAttr("__src", varType(VOID, 1, 0), createAttr("__n", varType(INT, 0, 0), NULL)))), CFUNCTION, bind_memmove);
  addLoadedDef(d, createFunc("memcpy", createAttr("__dest", varType(VOID, 1, 0), createAttr("__src", varType(VOID, 1, 0), createAttr("__n", varType(INT, 0, 0), NULL)))), CFUNCTION, bind_memcpy);
}

Val* bind_stpncpy(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(CHAR, 1, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 3)) != NULL) return m;
  char* __dest_ = GET_PTR(char*, args);
  char* __src_ = GET_PTR(char*, args);
  int __n_ = GET_VAL(int, args);
  char* r = stpncpy(__dest_, __src_, __n_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind___stpncpy(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(CHAR, 1, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 3)) != NULL) return m;
  char* __dest_ = GET_PTR(char*, args);
  char* __src_ = GET_PTR(char*, args);
  int __n_ = GET_VAL(int, args);
  char* r = __stpncpy(__dest_, __src_, __n_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_stpcpy(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(CHAR, 1, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  char* __dest_ = GET_PTR(char*, args);
  char* __src_ = GET_PTR(char*, args);
  char* r = stpcpy(__dest_, __src_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind___stpcpy(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(CHAR, 1, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  char* __dest_ = GET_PTR(char*, args);
  char* __src_ = GET_PTR(char*, args);
  char* r = __stpcpy(__dest_, __src_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_strsignal(Val* args) {
  Val* m; VarType t[] = {varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 1)) != NULL) return m;
  int __sig_ = GET_VAL(int, args);
  char* r = strsignal(__sig_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_strsep(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 2, 0), varType(CHAR, 1, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  char** __stringp_ = GET_PTR(char**, args);
  char* __delim_ = GET_PTR(char*, args);
  char* r = strsep(__stringp_, __delim_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_strncasecmp(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(CHAR, 1, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 3)) != NULL) return m;
  char* __s1_ = GET_PTR(char*, args);
  char* __s2_ = GET_PTR(char*, args);
  int __n_ = GET_VAL(int, args);
  int r = strncasecmp(__s1_, __s2_, __n_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_strcasecmp(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(CHAR, 1, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  char* __s1_ = GET_PTR(char*, args);
  char* __s2_ = GET_PTR(char*, args);
  int r = strcasecmp(__s1_, __s2_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_ffs(Val* args) {
  Val* m; VarType t[] = {varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 1)) != NULL) return m;
  int __i_ = GET_VAL(int, args);
  int r = ffs(__i_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_rindex(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  char* __s_ = GET_PTR(char*, args);
  int __c_ = GET_VAL(int, args);
  char* r = rindex(__s_, __c_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_index(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  char* __s_ = GET_PTR(char*, args);
  int __c_ = GET_VAL(int, args);
  char* r = index(__s_, __c_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_bcmp(Val* args) {
  Val* m; VarType t[] = {varType(VOID, 1, 0), varType(VOID, 1, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 3)) != NULL) return m;
  void* __s1_ = GET_PTR(void*, args);
  void* __s2_ = GET_PTR(void*, args);
  int __n_ = GET_VAL(int, args);
  int r = bcmp(__s1_, __s2_, __n_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_bzero(Val* args) {
  Val* m; VarType t[] = {varType(VOID, 1, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  void* __s_ = GET_PTR(void*, args);
  int __n_ = GET_VAL(int, args);
  bzero(__s_, __n_);
  return NULL;
}

Val* bind_bcopy(Val* args) {
  Val* m; VarType t[] = {varType(VOID, 1, 0), varType(VOID, 1, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 3)) != NULL) return m;
  void* __src_ = GET_PTR(void*, args);
  void* __dest_ = GET_PTR(void*, args);
  int __n_ = GET_VAL(int, args);
  bcopy(__src_, __dest_, __n_);
  return NULL;
}

Val* bind___bzero(Val* args) {
  Val* m; VarType t[] = {varType(VOID, 1, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  void* __s_ = GET_PTR(void*, args);
  int __n_ = GET_VAL(int, args);
  __bzero(__s_, __n_);
  return NULL;
}

Val* bind_strerror_r(Val* args) {
  Val* m; VarType t[] = {varType(INT, 0, 0), varType(CHAR, 1, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 3)) != NULL) return m;
  int __errnum_ = GET_VAL(int, args);
  char* __buf_ = GET_PTR(char*, args);
  int __buflen_ = GET_VAL(int, args);
  int r = strerror_r(__errnum_, __buf_, __buflen_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_strerror(Val* args) {
  Val* m; VarType t[] = {varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 1)) != NULL) return m;
  int __errnum_ = GET_VAL(int, args);
  char* r = strerror(__errnum_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_strnlen(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  char* __string_ = GET_PTR(char*, args);
  int __maxlen_ = GET_VAL(int, args);
  int r = strnlen(__string_, __maxlen_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_strlen(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0)};
  if ((m = checkSignature(args, t, 1)) != NULL) return m;
  char* __s_ = GET_PTR(char*, args);
  int r = strlen(__s_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_strtok_r(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(CHAR, 1, 0), varType(CHAR, 2, 0)};
  if ((m = checkSignature(args, t, 3)) != NULL) return m;
  char* __s_ = GET_PTR(char*, args);
  char* __delim_ = GET_PTR(char*, args);
  char** __save_ptr_ = GET_PTR(char**, args);
  char* r = strtok_r(__s_, __delim_, __save_ptr_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind___strtok_r(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(CHAR, 1, 0), varType(CHAR, 2, 0)};
  if ((m = checkSignature(args, t, 3)) != NULL) return m;
  char* __s_ = GET_PTR(char*, args);
  char* __delim_ = GET_PTR(char*, args);
  char** __save_ptr_ = GET_PTR(char**, args);
  char* r = __strtok_r(__s_, __delim_, __save_ptr_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_strtok(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(CHAR, 1, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  char* __s_ = GET_PTR(char*, args);
  char* __delim_ = GET_PTR(char*, args);
  char* r = strtok(__s_, __delim_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_strstr(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(CHAR, 1, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  char* __haystack_ = GET_PTR(char*, args);
  char* __needle_ = GET_PTR(char*, args);
  char* r = strstr(__haystack_, __needle_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_strpbrk(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(CHAR, 1, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  char* __s_ = GET_PTR(char*, args);
  char* __accept_ = GET_PTR(char*, args);
  char* r = strpbrk(__s_, __accept_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_strspn(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(CHAR, 1, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  char* __s_ = GET_PTR(char*, args);
  char* __accept_ = GET_PTR(char*, args);
  int r = strspn(__s_, __accept_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_strcspn(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(CHAR, 1, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  char* __s_ = GET_PTR(char*, args);
  char* __reject_ = GET_PTR(char*, args);
  int r = strcspn(__s_, __reject_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_strrchr(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  char* __s_ = GET_PTR(char*, args);
  int __c_ = GET_VAL(int, args);
  char* r = strrchr(__s_, __c_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_strchr(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  char* __s_ = GET_PTR(char*, args);
  int __c_ = GET_VAL(int, args);
  char* r = strchr(__s_, __c_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_strndup(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  char* __string_ = GET_PTR(char*, args);
  int __n_ = GET_VAL(int, args);
  char* r = strndup(__string_, __n_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_strdup(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0)};
  if ((m = checkSignature(args, t, 1)) != NULL) return m;
  char* __s_ = GET_PTR(char*, args);
  char* r = strdup(__s_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_strxfrm(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(CHAR, 1, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 3)) != NULL) return m;
  char* __dest_ = GET_PTR(char*, args);
  char* __src_ = GET_PTR(char*, args);
  int __n_ = GET_VAL(int, args);
  int r = strxfrm(__dest_, __src_, __n_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_strcoll(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(CHAR, 1, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  char* __s1_ = GET_PTR(char*, args);
  char* __s2_ = GET_PTR(char*, args);
  int r = strcoll(__s1_, __s2_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_strncmp(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(CHAR, 1, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 3)) != NULL) return m;
  char* __s1_ = GET_PTR(char*, args);
  char* __s2_ = GET_PTR(char*, args);
  int __n_ = GET_VAL(int, args);
  int r = strncmp(__s1_, __s2_, __n_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_strcmp(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(CHAR, 1, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  char* __s1_ = GET_PTR(char*, args);
  char* __s2_ = GET_PTR(char*, args);
  int r = strcmp(__s1_, __s2_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_strncat(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(CHAR, 1, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 3)) != NULL) return m;
  char* __dest_ = GET_PTR(char*, args);
  char* __src_ = GET_PTR(char*, args);
  int __n_ = GET_VAL(int, args);
  char* r = strncat(__dest_, __src_, __n_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_strcat(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(CHAR, 1, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  char* __dest_ = GET_PTR(char*, args);
  char* __src_ = GET_PTR(char*, args);
  char* r = strcat(__dest_, __src_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_strncpy(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(CHAR, 1, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 3)) != NULL) return m;
  char* __dest_ = GET_PTR(char*, args);
  char* __src_ = GET_PTR(char*, args);
  int __n_ = GET_VAL(int, args);
  char* r = strncpy(__dest_, __src_, __n_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_strcpy(Val* args) {
  Val* m; VarType t[] = {varType(CHAR, 1, 0), varType(CHAR, 1, 0)};
  if ((m = checkSignature(args, t, 2)) != NULL) return m;
  char* __dest_ = GET_PTR(char*, args);
  char* __src_ = GET_PTR(char*, args);
  char* r = strcpy(__dest_, __src_);
  return initPtr(varType(CHAR, 1, 0), r);
}

Val* bind_memchr(Val* args) {
  Val* m; VarType t[] = {varType(VOID, 1, 0), varType(INT, 0, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 3)) != NULL) return m;
  void* __s_ = GET_PTR(void*, args);
  int __c_ = GET_VAL(int, args);
  int __n_ = GET_VAL(int, args);
  memchr(__s_, __c_, __n_);
  return NULL;
}

Val* bind_memcmp(Val* args) {
  Val* m; VarType t[] = {varType(VOID, 1, 0), varType(VOID, 1, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 3)) != NULL) return m;
  void* __s1_ = GET_PTR(void*, args);
  void* __s2_ = GET_PTR(void*, args);
  int __n_ = GET_VAL(int, args);
  int r = memcmp(__s1_, __s2_, __n_);
  return initVal(varType(INT, 0, 0), &r);
}

Val* bind_memset(Val* args) {
  Val* m; VarType t[] = {varType(VOID, 1, 0), varType(INT, 0, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 3)) != NULL) return m;
  void* __s_ = GET_PTR(void*, args);
  int __c_ = GET_VAL(int, args);
  int __n_ = GET_VAL(int, args);
  memset(__s_, __c_, __n_);
  return NULL;
}

Val* bind_memccpy(Val* args) {
  Val* m; VarType t[] = {varType(VOID, 1, 0), varType(VOID, 1, 0), varType(INT, 0, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 4)) != NULL) return m;
  void* __dest_ = GET_PTR(void*, args);
  void* __src_ = GET_PTR(void*, args);
  int __c_ = GET_VAL(int, args);
  int __n_ = GET_VAL(int, args);
  memccpy(__dest_, __src_, __c_, __n_);
  return NULL;
}

Val* bind_memmove(Val* args) {
  Val* m; VarType t[] = {varType(VOID, 1, 0), varType(VOID, 1, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 3)) != NULL) return m;
  void* __dest_ = GET_PTR(void*, args);
  void* __src_ = GET_PTR(void*, args);
  int __n_ = GET_VAL(int, args);
  memmove(__dest_, __src_, __n_);
  return NULL;
}

Val* bind_memcpy(Val* args) {
  Val* m; VarType t[] = {varType(VOID, 1, 0), varType(VOID, 1, 0), varType(INT, 0, 0)};
  if ((m = checkSignature(args, t, 3)) != NULL) return m;
  void* __dest_ = GET_PTR(void*, args);
  void* __src_ = GET_PTR(void*, args);
  int __n_ = GET_VAL(int, args);
  memcpy(__dest_, __src_, __n_);
  return NULL;
}

