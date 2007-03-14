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

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "zpath.h"
#include "zdir.h"

#ifndef __UNIXSDL__

#ifdef __MSDOS__
#include <dos.h>
#define _findfirst(regex, info) findfirst(regex, info, FA_HIDDEN|FA_DIREC)
#define _findnext(handle, info) findnext(info)
#define _findclose(handle)
#define FIND_GOOD(handle) (!(handle))
#define FIND_FAIL(handle) (handle)
#define S_IWRITE S_IWUSR
#define S_IREAD S_IRUSR
#else
#define FIND_GOOD(handle) ((handle) != -1)
#define FIND_FAIL(handle) ((handle) == -1)
#define ff_name name
#define ff_fsize size
#define ff_attrib attrib
#endif

//Note, these are faster than the built in DJGPP/MinGW ones
z_DIR *z_opendir(const char *path)
{
  z_DIR *dir = 0;
  if (path && *path)
  {
    char search[PATH_SIZE];
    strcpy(search, path);
    strcatslash(search);
#ifdef __MSDOS__
    strcat(search, "*.*");
#else
    strcat(search, "*");
#endif

    dir = malloc(sizeof(z_DIR));
    if (dir)
    {
      dir->find_first_handle = _findfirst(search, &dir->fileinfo);
      if (FIND_FAIL(dir->find_first_handle))
      {
        //ENOENT set by findfirst already
        free(dir);
        dir = 0;
      }
    }
    else
    {
      errno = ENOMEM;
    }
  }
  else
  {
    errno = EINVAL;
  }

  return(dir);
}

struct z_dirent *z_readdir(z_DIR *dir)
{
  struct z_dirent *entry = 0;
  if (FIND_GOOD(dir->find_first_handle))
  {
    entry = &dir->entry;
    strcpy(entry->d_name, dir->fileinfo.ff_name);
    if (FIND_FAIL(_findnext(dir->find_first_handle, &dir->fileinfo)))
    {
      _findclose(dir->find_first_handle);
      dir->find_first_handle = -1;
    }
  }
  else
  {
    errno = EBADF;
  }

  return(entry);
}

int z_closedir(z_DIR *dir)
{
  int result = 0;

  if (dir)
  {
    if (FIND_GOOD(dir->find_first_handle))
    {
      _findclose(dir->find_first_handle);
    }
    free(dir);
  }
  else
  {
    result = -1;
    errno = EBADF;
  }

  return(result);
}

struct dirent_info *readdir_info(z_DIR *dir)
{
  static struct dirent_info info;
  struct dirent_info *infop = 0;

  if (FIND_GOOD(dir->find_first_handle))
  {
    strcpy(dir->entry.d_name, dir->fileinfo.ff_name);

    info.name = dir->entry.d_name;
    info.size = dir->fileinfo.ff_fsize;
    info.mode = S_IREAD;
    if (!(dir->fileinfo.ff_attrib & _A_RDONLY)) { info.mode |= S_IWRITE; }
    if (dir->fileinfo.ff_attrib & _A_SUBDIR) { info.mode |= S_IFDIR; }
    else { info.mode |= S_IFREG; }
    infop = &info;

    if (FIND_FAIL(_findnext(dir->find_first_handle, &dir->fileinfo)))
    {
      _findclose(dir->find_first_handle);
      dir->find_first_handle = -1;
    }
  }
  else
  {
    errno = EBADF;
  }

  return(infop);
}

#else
#include "linux/lib.h"

struct dirent_info *readdir_info(z_DIR *dir)
{
  static struct dirent_info info;
  struct dirent_info *infop = 0;

  struct dirent *entry = readdir(dir);
  if (entry)
  {
    struct stat stat_buffer;
    if (!fstatat(dirfd(dir), entry->d_name, &stat_buffer, 0))
    {
      info.name = entry->d_name;
      info.size = stat_buffer.st_size;
      info.mode = stat_buffer.st_mode;
      infop = &info;
    }
    else
    {
      infop = readdir_info(dir);
    }
  }
  return(infop);
}
#endif
