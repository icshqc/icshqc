#include "bind.h"

#include "/usr/include/string.h"

void initCFunctions(LoadedDef* d) {
  addLoadedDef(d, "stpncpy", CFUNCTION, bind_stpncpy);
  addLoadedDef(d, "__stpncpy", CFUNCTION, bind___stpncpy);
  addLoadedDef(d, "stpcpy", CFUNCTION, bind_stpcpy);
  addLoadedDef(d, "__stpcpy", CFUNCTION, bind___stpcpy);
  addLoadedDef(d, "strsignal", CFUNCTION, bind_strsignal);
  addLoadedDef(d, "strsep", CFUNCTION, bind_strsep);
  addLoadedDef(d, "strncasecmp", CFUNCTION, bind_strncasecmp);
  addLoadedDef(d, "strcasecmp", CFUNCTION, bind_strcasecmp);
  addLoadedDef(d, "ffs", CFUNCTION, bind_ffs);
  addLoadedDef(d, "rindex", CFUNCTION, bind_rindex);
  addLoadedDef(d, "index", CFUNCTION, bind_index);
  addLoadedDef(d, "bcmp", CFUNCTION, bind_bcmp);
  addLoadedDef(d, "bzero", CFUNCTION, bind_bzero);
  addLoadedDef(d, "bcopy", CFUNCTION, bind_bcopy);
  addLoadedDef(d, "__bzero", CFUNCTION, bind___bzero);
  addLoadedDef(d, "strerror_r", CFUNCTION, bind_strerror_r);
  addLoadedDef(d, "strerror", CFUNCTION, bind_strerror);
  addLoadedDef(d, "strnlen", CFUNCTION, bind_strnlen);
  addLoadedDef(d, "strlen", CFUNCTION, bind_strlen);
  addLoadedDef(d, "strtok_r", CFUNCTION, bind_strtok_r);
  addLoadedDef(d, "__strtok_r", CFUNCTION, bind___strtok_r);
  addLoadedDef(d, "strtok", CFUNCTION, bind_strtok);
  addLoadedDef(d, "strstr", CFUNCTION, bind_strstr);
  addLoadedDef(d, "strpbrk", CFUNCTION, bind_strpbrk);
  addLoadedDef(d, "strspn", CFUNCTION, bind_strspn);
  addLoadedDef(d, "strcspn", CFUNCTION, bind_strcspn);
  addLoadedDef(d, "strrchr", CFUNCTION, bind_strrchr);
  addLoadedDef(d, "strchr", CFUNCTION, bind_strchr);
  addLoadedDef(d, "strndup", CFUNCTION, bind_strndup);
  addLoadedDef(d, "strdup", CFUNCTION, bind_strdup);
  addLoadedDef(d, "strxfrm", CFUNCTION, bind_strxfrm);
  addLoadedDef(d, "strcoll", CFUNCTION, bind_strcoll);
  addLoadedDef(d, "strncmp", CFUNCTION, bind_strncmp);
  addLoadedDef(d, "strcmp", CFUNCTION, bind_strcmp);
  addLoadedDef(d, "strncat", CFUNCTION, bind_strncat);
  addLoadedDef(d, "strcat", CFUNCTION, bind_strcat);
  addLoadedDef(d, "strncpy", CFUNCTION, bind_strncpy);
  addLoadedDef(d, "strcpy", CFUNCTION, bind_strcpy);
  addLoadedDef(d, "memchr", CFUNCTION, bind_memchr);
  addLoadedDef(d, "memcmp", CFUNCTION, bind_memcmp);
  addLoadedDef(d, "memset", CFUNCTION, bind_memset);
  addLoadedDef(d, "memccpy", CFUNCTION, bind_memccpy);
  addLoadedDef(d, "memmove", CFUNCTION, bind_memmove);
  addLoadedDef(d, "memcpy", CFUNCTION, bind_memcpy);
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

