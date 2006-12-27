/*
Copyright (C) 1997-2007 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes
https://zsnes.bountysource.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef LIB_H
#define LIB_H

#include <io.h>
#include <windows.h>

#ifdef _MSC_VER
#define strcasecmp stricmp
#define strncasecmp strnicmp

struct dirent
{
  char d_name[256];
};

typedef struct
{
  intptr_t find_first_handle;
  struct _finddata_t fileinfo;
  struct dirent entry;
} DIR;

DIR *opendir(const char *path);
struct dirent *readdir(DIR *dir);
int closedir(DIR *dir);

#endif

char *realpath(const char *path, char *resolved_path);

#endif
