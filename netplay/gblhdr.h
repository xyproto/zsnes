/*
Copyright (C) 1997-2005 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*************************************\
* Global Definitions and Headers File *
\*************************************/

// Standard stuff

#include "fcntl.h"
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <string.h>

#include "config.h"
// General time.h checking

#if TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif

// General dirent.h stuff

#if HAVE_DIRENT_H
#include <dirent.h>
#else
#if HAVE_SYS_NDIR_H
#include <sys/ndir.h>
#endif
#if HAVE_SYS_DIR_H
#include <sys/dir.h>
#endif
#if HAVE_NDIR_H
#include <ndir.h>
#endif
#endif

// more standard stuff

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// unistd.h stuff

//#if HAVE_UNISTD_H
// Small hack for now
#if unix
#include <sys/types.h>
#include <unistd.h>
#endif

// opengl stuff

#ifdef __OPENGL__
#include <GL/gl.h>
#endif

// os specific stuff

#ifdef __QNXNTO__
/* QNX6 has getpagesize() commented out in unistd.h,
however it's a static value that we can just define */
#define getpagesize() 4096
#endif

#ifdef __LINUX__
#include "SDL.h"
#include <limits.h>
#ifndef __BEOS__
#include <arpa/inet.h>
#include <glob.h>
#include <sys/mman.h>
#endif
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <utime.h>
#include <zlib.h>
#ifndef NO_PNG
#include <png.h>
#endif
#endif

#ifdef __LINUX__
#if !defined(__FreeBSD__) && !defined(__BEOS__)
#include <asm/ioctls.h>
#else
#include <sys/filio.h>
#endif
#endif
