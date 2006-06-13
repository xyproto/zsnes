/*
Copyright (C) 1997-2006 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes

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

#ifndef ZPATH_H
#define ZPATH_H

#include <stdio.h>
#include <sys/stat.h>

#if !defined(__cplusplus) && !defined(bool)
//C++ style code in C
#define bool unsigned char
#define true 1
#define false 0
#endif

#ifdef __UNIXSDL__
#define DIR_SLASH_C '/'
#define DIR_SLASH "/"
#else
#define DIR_SLASH_C '\\'
#define DIR_SLASH "\\"
#endif

extern char ZCfgFile[];
extern char *ZStartPath, *ZCfgPath, *ZSramPath;

bool init_paths(char *launch_command);

char *strdupcat(const char *str1, const char *str2);

int access_dir(const char *path, const char *file, int mode);
int stat_dir(const char *path, const char *file, struct stat *buf);
FILE *fopen_dir(const char *path, const char *file, const char *mode);
int remove_dir(const char *path, const char *file);
int mkdir_dir(const char *path, const char *dir);

void strcatslash(char *str);
void strdirname(char *str);

#endif
