#define _POSIX_
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <fcntl.h>

#include "../argv.h"

//These are here because I don't believe in MSVC's prefixing affixation
#define dup _dup
#define dup2 _dup2
#define pipe _pipe
#define flushall _flushall
#define fdopen _fdopen
#define P_NOWAIT _P_NOWAIT

#ifndef STDIN_FILENO
#define STDIN_FILENO fileno(stdin)
#endif

#ifndef STDOUT_FILENO
#define STDOUT_FILENO fileno(stdout)
#endif

//Introducing a popen which doesn't return until it knows for sure of program launched or couldn't open -Nach

#define READ_FD 0
#define WRITE_FD 1

FILE *safe_popen(char *command, const char *mode)
{
  FILE *ret = 0;
  char **argv = build_argv(command);
  if (argv)
  {
    int filedes[2];

    if ((*mode == 'r' || *mode == 'w') &&
        !pipe(filedes, 512, (mode[1] == 'b' ? O_BINARY : O_TEXT) | O_NOINHERIT))
    {
      int fd_original;
      FILE *fp;

      if (*mode == 'r')
      {
        fd_original = dup(STDOUT_FILENO);
        dup2(filedes[WRITE_FD], STDOUT_FILENO);
        if (!(fp = fdopen(filedes[READ_FD], mode)))
        {
          close(filedes[READ_FD]);
        }
      }
      else
      {
        fd_original = dup(STDIN_FILENO);
        dup2(filedes[READ_FD], STDIN_FILENO);
        if (!(fp = fdopen(filedes[WRITE_FD], mode)))
        {
          close(filedes[WRITE_FD]);
        }
      }

      if (fp)
      {
        int status;
        flushall();

        status = spawnvp(P_NOWAIT, argv[0], (const char* const*)argv);
        if (status > 0)
        {
          ret = fp;
        }
        else
        {
          fclose(fp);
        }
      }

      if (*mode == 'r')
      {
        dup2(fd_original, STDIN_FILENO);
      }
      else
      {
        dup2(fd_original, STDOUT_FILENO);
      }
      close(fd_original);
    }
    free(argv);
  }
  return(ret);
}
