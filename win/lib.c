#include "lib.h"
#include "../zpath.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define fullpath _fullpath

// This file contains library functions that can be found on other OSs

char* realpath(const char* path, char* resolved_path)
{
    char* ret = 0;

    if (!path || !resolved_path) {
        errno = EINVAL;
    } else if (!access(path, F_OK)) {
        ret = fullpath(resolved_path, path, PATH_SIZE);
    }

    return (ret);
}
