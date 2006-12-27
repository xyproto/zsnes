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

#include "lib.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "../zpath.h"

#define fullpath _fullpath


//This file contains library functions that can be found on other OSs


#ifdef _MSC_VER

DIR *opendir(const char *path)
{
  DIR *dir = 0;
  if (path && *path)
  {
    char search[MAX_PATH];
    strcpy(search, path);
    strcatslash(search);
    strcat(search, "*");

    dir = malloc(sizeof(DIR));
    if (dir)
    {
      dir->find_first_handle = _findfirst(search, &dir->fileinfo);
      if (dir->find_first_handle == -1)
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

struct dirent *readdir(DIR *dir)
{
  struct dirent *entry = 0;
  if (dir->find_first_handle != -1)
  {
    entry = &dir->entry;
    strcpy(entry->d_name, dir->fileinfo.name);
    if (_findnext(dir->find_first_handle, &dir->fileinfo) == -1)
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

int closedir(DIR *dir)
{
  int result = 0;

  if (dir)
  {
    if (dir->find_first_handle != -1)
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

#endif


char *realpath(const char *path, char *resolved_path)
{
  char *ret = 0;

  if (!path || !resolved_path) { errno = EINVAL; }
  else if (!access(path, F_OK))
  {
    ret = fullpath(resolved_path, path, PATH_SIZE);
  }

  return(ret);
}
