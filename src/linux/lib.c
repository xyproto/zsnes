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

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#include "lib.h"

static int open_cwd()
{
  int cwdfd = open(".", O_RDONLY);
  if (cwdfd == -1) { cwdfd = open(".", O_WRONLY); }
  return(cwdfd);
}

int fstatat(int dirfd, const char *pathname, struct stat *buf, int flags)
{
  int success = -1;

  if ((!flags || (flags == AT_SYMLINK_NOFOLLOW)) && buf)
  {
    if (pathname && *pathname)
    {
      int cwdfd = -1;
      if ((dirfd == AT_FDCWD) || (*pathname == '/') || (((cwdfd=open_cwd()) != -1) && !fchdir(dirfd)))
      {
        success = (!flags) ? stat(pathname, buf) : lstat(pathname, buf);
      }

      if (cwdfd != -1)
      {
        fchdir(cwdfd);
        close(cwdfd);
      }
    }
    else
    {
      success = (dirfd == AT_FDCWD) ? stat(".", buf) : fstat(dirfd, buf);
    }
  }
  else
  {
    errno = EINVAL;
  }

  return(success);
}
