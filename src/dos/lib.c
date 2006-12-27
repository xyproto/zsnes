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

#include <libc/stubs.h>
#include <libc/dosio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <go32.h>
#include <dpmi.h>
#include <dos.h>
#include <sys/stat.h>
#include <dir.h>

//C++ style code in C
#define bool unsigned char
#define true 1
#define false 0

#define BIT(X) (1 << (X))


//This file contains library functions that can be found on other OSs


//Return realpath in 8.3 or LFN format for any given filename
//Based on code from DJGPP website, here was the notice for that code:

/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * This is file TRUENAME.C
 *
 * Copyright (c) 1994, 1995 Eli Zaretskii <eliz@is.elta.co.il>
 *
 * This software may be used freely so long as this copyright notice is
 * left intact.  There is no warranty on this software.
 *
 */

static char *realpath_internal(const char *file, char *buf, bool LFN)
{
  __dpmi_regs     regs;
  unsigned short  dos_mem_selector = _dos_ds;
  unsigned short  our_mem_selector = _my_ds();
  char true_name[FILENAME_MAX];
  char file_name[FILENAME_MAX], *name_end;

  if (!file || !*file)
  {
    errno = EINVAL;
    buf = 0;
  }
  else
  {
    strncpy(file_name, file, FILENAME_MAX);
    file_name[FILENAME_MAX - 1] = 0;

    for (name_end = file_name + strlen(file_name) - 1; name_end >= file_name && isspace((unsigned char)*name_end); )
    {
      *name_end-- = 0;
    }

    if ((strlen(file_name) == 2) && (file_name[1] == ':'))
    {
      strcat(name_end, "\\.");
    }
    else if ((*name_end == '\\') && (name_end-file_name < FILENAME_MAX-2))
    {
      strcat(name_end, ".");
    }

    _put_path(file_name);

    regs.x.ax = _USE_LFN ? 0x7160 : 0x6000;
    regs.x.cx = LFN+1;
    regs.x.ds = regs.x.es = __tb_segment;
    regs.x.si = __tb_offset;
    regs.x.di = __tb_offset + FILENAME_MAX;
    __dpmi_int(0x21, &regs);

    movedata(dos_mem_selector, __tb + FILENAME_MAX, our_mem_selector, (unsigned int)true_name, FILENAME_MAX);

    if (regs.x.flags & 1)
    {
      errno = __doserr_to_errno(regs.x.ax);
      buf = 0;
    }
    else
    {
      if (!buf)
      {
        buf = (char *)malloc(strlen(true_name)+1);
      }

      if (buf)
      {
        strcpy(buf, true_name);
      }
      else
      {
        errno = ENOMEM;
      }
    }
  }
  return(buf);
}

char *realpath_sfn(const char *file, char *buf)
{
  return(realpath_internal(file, buf, false));
}

char *realpath_lfn(const char *file, char *buf)
{
  return(realpath_internal(file, buf, true));
}

//We tested this with Hard Disks, Floppies, CD/DVD-ROM, Network drives, no issues.
//It should also be tested with RAM drives and on more versions of DOS (DR-DOS, MS-DOS 5.0, etc...)
static bool _is_drive(unsigned char drive) //A == 1, B == 2, etc...
{
  __dpmi_regs     regs;

  regs.x.ax = 0x4409;
  regs.x.bx = drive;
  __dpmi_int(0x21, &regs);

  if (regs.x.flags & 1)
  {
    errno = __doserr_to_errno(regs.x.ax);
  }
  else if (((regs.x.dx & (BIT(9)|BIT(12))) == BIT(12)) || (regs.x.dx == 0x800))
  {
    return(true);
  }

  regs.x.ax = 0x4408;
  regs.x.bx = drive;
  __dpmi_int(0x21, &regs);

  if (regs.x.flags & 1)
  {
    errno = __doserr_to_errno(regs.x.ax);
    return(false);
  }
  return(true);
}

//Return bitmask of available drives, A = BIT(0), B = BIT(1), etc...
unsigned int GetLogicalDrives()
{
  unsigned int drives = 0;
  int i;
  for (i = 0; i < 26; i++)
  {
    if (_is_drive(i+1))
    {
      drives |= BIT(i);
    }
  }
  return(drives);
}
