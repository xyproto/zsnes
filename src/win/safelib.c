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

#include <windows.h>
#include <process.h>
#include <io.h>
#define _POSIX_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "safelib.h"
#include "../argv.h"

//These are here because I don't believe in MSVC's prefixing affixation
#define dup _dup
#define dup2 _dup2
#define pipe _pipe
#define flushall _flushall
#define cwait _cwait


//Introducing a popen which doesn't return until it knows for sure of program launched or couldn't open -Nach

#define READ_FD 0
#define WRITE_FD 1

static struct fp_pid_link
{
  FILE *fp;
  int pid;
  struct fp_pid_link *next;
} fp_pids = { 0, 0, 0 };

FILE *safe_popen(char *command, const char *mode)
{
  FILE *ret = 0;
  char **argv = build_argv(command);
  if (argv)
  {
    int filedes[2];

    if (mode && (*mode == 'r' || *mode == 'w') &&
        !pipe(filedes, 512, (mode[1] == 'b' ? O_BINARY : O_TEXT) | O_NOINHERIT))
    {
      int fd_original;
      FILE *fp;

      if (*mode == 'r')
      {
        fd_original = dup(STDOUT_FILENO);
        dup2(filedes[WRITE_FD], STDOUT_FILENO);
        close(filedes[WRITE_FD]);
        if (!(fp = fdopen(filedes[READ_FD], mode)))
        {
          close(filedes[READ_FD]);
        }
      }
      else
      {
        fd_original = dup(STDIN_FILENO);
        dup2(filedes[READ_FD], STDIN_FILENO);
        close(filedes[READ_FD]);
        if (!(fp = fdopen(filedes[WRITE_FD], mode)))
        {
          close(filedes[WRITE_FD]);
        }
      }

      if (fp)
      {
        intptr_t childpid;
        flushall();

        childpid = spawnvp(P_NOWAIT, argv[0], (const char* const*)argv);
        if (childpid > 0)
        {
          struct fp_pid_link *link = &fp_pids;
          while (link->next)
          {
            link = link->next;
          }

          link->next = (struct fp_pid_link *)malloc(sizeof(struct fp_pid_link));
          if (link->next)
          {
            link->next->fp = fp;
            link->next->pid = childpid;
            link->next->next = 0;
            ret = fp;
          }
          else
          {
            fclose(fp);
            TerminateProcess((HANDLE)childpid, 0);
            cwait(0, childpid, WAIT_CHILD);
          }
        }
        else
        {
          fclose(fp);
        }
      }

      if (*mode == 'r')
      {
        dup2(fd_original, STDOUT_FILENO);
      }
      else
      {
        dup2(fd_original, STDIN_FILENO);
      }
      close(fd_original);
    }
    free(argv);
  }
  return(ret);
}

void safe_pclose(FILE *fp)
{
  struct fp_pid_link *link = &fp_pids;

  while (link->next && link->next->fp != fp)
  {
    link = link->next;
  }
  if (link->next->fp == fp)
  {
    struct fp_pid_link *dellink = link->next;
    fclose(fp);
    cwait(0, link->next->pid, WAIT_CHILD);
    link->next = link->next->next;
    free(dellink);
  }
}
