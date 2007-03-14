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

#ifndef ZDIR_H
#define ZDIR_H

struct dirent_info
{
  char *name;
  mode_t mode;
  off_t size;
};

#ifndef __UNIXSDL__
#include <dir.h>

#ifdef __MSDOS__
#include <stdint.h>
#define _finddata_t ffblk
#else
#include <windows.h>
#endif

//Avoid clashing with DJGPP and MinGW extras

struct z_dirent
{
  char d_name[256];
};

typedef struct
{
  intptr_t find_first_handle;
  struct _finddata_t fileinfo;
  struct z_dirent entry;
} z_DIR;

z_DIR *z_opendir(const char *path);
struct z_dirent *z_readdir(z_DIR *dir);
int z_closedir(z_DIR *dir);

#ifndef NO_ZDIR_TYPEDEF
#define dirent z_dirent
typedef z_DIR DIR;
#define opendir z_opendir
#define readdir z_readdir
#define closedir z_closedir
#endif

#else
#include <dirent.h>
typedef DIR z_DIR;
#endif

struct dirent_info *readdir_info(z_DIR *dir);

#endif
