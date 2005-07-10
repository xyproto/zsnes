/*
Copyright (C) 2005 Nach, grinvader ( http://www.zsnes.com )

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

/*
This is part of a toolkit used to assist in ZSNES development
*/

#include <unistd.h>
#include <dirent.h>

#include "fileutil.h"

bool parse_dir(const char *dir_loc, void (*func)(const char *, struct stat&))
{
  char cwd[16384];
  
  if (getcwd(cwd, sizeof(cwd)) && !chdir(dir_loc)) //chdir() returns 0 on success
  {
    DIR *curDir = opendir(".");
    dirent *curFile;
    while ((curFile = readdir(curDir)))
    {
      char *filename = curFile->d_name;

      if (!strcmp(filename, ".") || !strcmp(filename, ".."))
      {
        continue;
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
    chdir(cwd);
    return(true);
  }
  return(false);
}

bool parse_path(const char *path, void (*func)(const char *, struct stat&))
{
  struct stat stat_buffer;
  if (!stat(path, &stat_buffer))
  {
    if (S_ISDIR(stat_buffer.st_mode))
    {
      return(parse_dir(path, func));
    }
    func(path, stat_buffer);
    return(true);
  }
  return(false);
}
