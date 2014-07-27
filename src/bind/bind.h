#ifndef BIND_H
#define BIND_H

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

#include "../../model.h"
#include "glue.h"

void initCFunctions(LoadedDef* d);

Val* bind_stpncpy(Val* args);
Val* bind___stpncpy(Val* args);
Val* bind_stpcpy(Val* args);
Val* bind___stpcpy(Val* args);
Val* bind_strsignal(Val* args);
Val* bind_strsep(Val* args);
Val* bind_strncasecmp(Val* args);
Val* bind_strcasecmp(Val* args);
Val* bind_ffs(Val* args);
Val* bind_rindex(Val* args);
Val* bind_index(Val* args);
Val* bind_bcmp(Val* args);
Val* bind_bzero(Val* args);
Val* bind_bcopy(Val* args);
Val* bind___bzero(Val* args);
Val* bind_strerror_l(Val* args);
Val* bind_strerror_r(Val* args);
Val* bind_strerror(Val* args);
Val* bind_strnlen(Val* args);
Val* bind_strlen(Val* args);
Val* bind_strtok_r(Val* args);
Val* bind___strtok_r(Val* args);
Val* bind_strtok(Val* args);
Val* bind_strstr(Val* args);
Val* bind_strpbrk(Val* args);
Val* bind_strspn(Val* args);
Val* bind_strcspn(Val* args);
Val* bind_strrchr(Val* args);
Val* bind_strchr(Val* args);
Val* bind_strndup(Val* args);
Val* bind_strdup(Val* args);
Val* bind_strxfrm_l(Val* args);
Val* bind_strcoll_l(Val* args);
Val* bind_strxfrm(Val* args);
Val* bind_strcoll(Val* args);
Val* bind_strncmp(Val* args);
Val* bind_strcmp(Val* args);
Val* bind_strncat(Val* args);
Val* bind_strcat(Val* args);
Val* bind_strncpy(Val* args);
Val* bind_strcpy(Val* args);
Val* bind_memchr(Val* args);
Val* bind_memcmp(Val* args);
Val* bind_memset(Val* args);
Val* bind_memccpy(Val* args);
Val* bind_memmove(Val* args);
Val* bind_memcpy(Val* args);
#endif // BIND_H