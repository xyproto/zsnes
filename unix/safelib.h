#ifndef SAFELIB_H
#define SAFELIB_H

#include <stdio.h>
#include <sys/types.h>

pid_t safe_fork(int*, size_t);

FILE* safe_popen(char*, const char*);
void safe_pclose(FILE*);

#define popen safe_popen
#define pclose safe_pclose

#endif
