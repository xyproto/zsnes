#ifndef LIB_H
#define LIB_H

#include <io.h>
#include <windows.h>

#ifdef _MSC_VER
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif

char* realpath(const char* path, char* resolved_path);

#endif
