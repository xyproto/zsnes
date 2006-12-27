/*
Copyright (C) 2005-2007 Nach, grinvader ( http://www.zsnes.com )

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

/*
This is part of a toolkit used to assist in ZSNES development
*/

#include <stdio.h>
#include <unistd.h>
#include <dirent.h>

#include "fileutil.h"

void parse_dir(const char *dir_loc, void (*func)(const char *, struct stat&))
{
  char path[4096];

  DIR *curDir = opendir(dir_loc);
  dirent *curFile;
  if (curDir)
  {
    while ((curFile = readdir(curDir)))
    {
      if (!strcmp(curFile->d_name, ".") || !strcmp(curFile->d_name, ".."))
      {
        continue;
      }

      char *filename;
      if (!strcmp(dir_loc, "."))
      {
        filename = curFile->d_name;
      }
      else
      {
        sprintf(path, "%s/%s", dir_loc, curFile->d_name);
        filename = path;
      }

      struct stat stat_buffer;
      if (stat(filename, &stat_buffer)) { continue; }

      //Directory
      if (S_ISDIR(stat_buffer.st_mode))
      {
        parse_dir(filename, func);
        continue;
      }

      func(filename, stat_buffer);
    }
    closedir(curDir);
  }
}

bool parse_path(const char *path, void (*func)(const char *, struct stat&))
{
  struct stat stat_buffer;
  if (!stat(path, &stat_buffer))
  {
    if (S_ISDIR(stat_buffer.st_mode))
    {
      parse_dir(path, func);
    }
    else
    {
      func(path, stat_buffer);
    }
    return(true);
  }
  return(false);
}
