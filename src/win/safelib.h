#ifndef SAFELIB_H
#define SAFELIB_H

#include <stdio.h>

FILE *safe_popen(char *, const char *);

#define popen safe_popen
#define pclose fclose

#endif
