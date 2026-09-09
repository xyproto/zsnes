#ifndef GBLHDR_H
#define GBLHDR_H

#include "types.h" /* IGNORE_RESULT */

/*************************************\
* Global Definitions and Headers File *
\*************************************/

// Standard stuff

#include "fcntl.h"
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <strings.h> /* strcasecmp: glibc leaks it through string.h, Darwin does not */

/* These were autoconf feature tests. Nothing defines them and there is no
   configure step, so every branch was false: <time.h> alone was included and
   dirent.h was not. Consumers include <dirent.h> themselves (zdir.h). */

#include <time.h>

// more standard stuff

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// unistd.h stuff

/* Not `unix`: that spelling is only predefined in GNU mode, so under -std=c11
   this branch never fired and unistd.h was never pulled in here. */
#if defined(__unix__) || defined(__APPLE__)
#include <sys/types.h>
#include <unistd.h>
#endif

// opengl stuff

#ifdef __OPENGL__
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#endif

// os specific stuff

#ifdef __QNXNTO__
/* QNX6 has getpagesize() commented out in unistd.h,
however it's a static value that we can just define */
#define getpagesize() 4096
#endif

#ifdef __UNIXSDL__
#include <SDL3/SDL.h>
#include <arpa/inet.h>
#include <glob.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pwd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <utime.h>
#include <zlib.h>
#ifndef NO_PNG
#include <png.h>
#endif
#endif

#ifdef __UNIXSDL__
#ifdef __linux__
#include <asm/ioctls.h>
#else
#include <sys/filio.h>
#endif
#endif

#endif
