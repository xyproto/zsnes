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

#ifdef __UNIXSDL__
#include "gblhdr.h"
#include <pwd.h>
#else
#include <io.h>
#include <string.h>
#include <stdlib.h>
#endif
#include <errno.h>

#include "zpath.h"
#include "cfg.h"

#ifdef __WIN32__
#define fullpath _fullpath
#else
#define mkdir_p(path) mkdir(path, (S_IRWXU|S_IRWXG|S_IRWXO)) //0777
#endif

#define PATH_SIZE 4096

#ifdef __MSDOS__
char ZCfgFile[] = "zsnes.cfg";
#elif defined(__WIN32__)
char ZCfgFile[] = "zsnesw.cfg";
#else
char ZCfgFile[] = "zsnesl.cfg";
#endif

char *ZStartPath = 0, *ZCfgPath = 0, *ZSramPath = 0;
static bool ZStartAlloc = false, ZCfgAlloc = false, ZSramAlloc = false;

#ifdef __UNIXSDL__

void cfgpath_ensure()
{
  struct passwd *userinfo;
  const char *const zpath = ".zsnes";

  if ((userinfo = getpwuid(getuid())))
  {
    ZCfgPath = malloc(strlen(userinfo->pw_dir)+strlen(zpath)+1);
    ZCfgAlloc = true;
  }
  else
  {
    puts("Error obtaining info about your user.");
  }

  if (ZCfgPath)
  {
    strcpy(ZCfgPath, userinfo->pw_dir);
    strcatslash(ZCfgPath);
    strcat(ZCfgPath, zpath);

    if (access(ZCfgPath, F_OK) && mkdir(ZCfgPath, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)))
    {
      printf("Error creating: %s\n", ZCfgPath);
      free(ZCfgPath);
      ZCfgAlloc = false;

      ZCfgPath = ZStartPath;
    }
    else
    {
      strcatslash(ZCfgPath);
    }
  }
  else
  {
    ZCfgPath = ZStartPath;
  }
}

#else

void cfgpath_ensure()
{
  ZCfgPath = ZStartPath;
}

#endif

#ifdef __WIN32__

//Windows for some reason lacks this function, but DOS doesn't
char *realpath(const char *path, char *resolved_path)
{
  char *ret = 0;

  if (!path || !resolved_path) { errno = EINVAL; }
  else if (!access(path, F_OK))
  {
    ret = fullpath(resolved_path, path, MAX_PATH);
  }

  return (ret);
}

#endif

void deinit_paths()
{
  if (ZStartAlloc && ZStartPath) { free(ZStartPath); }
  if (ZCfgAlloc && ZCfgPath) { free(ZCfgPath); }
  if (ZSramAlloc && ZSramPath) { free(ZSramPath); }
}

bool init_paths(char *launch_command)
{
  void GUIRestoreVars();

  ZStartPath = malloc(PATH_SIZE);
  if (ZStartPath)
  {
    ZStartAlloc = true;

    if (realpath(launch_command, ZStartPath))
    {
      char *p = strrchr(ZStartPath, DIR_SLASH_C);
      if (p)
      {
        *p = 0;
      }
    }
    else
    {
      getcwd(ZStartPath, PATH_SIZE);
    }
    strcatslash(ZStartPath);

    cfgpath_ensure();

    GUIRestoreVars();

    //TODO - Get this working nicely for saving in ROM directory on DOS/Win
    if (*SRAMDir)
    {
      ZSramPath = SRAMDir;
    }
    else
    {
      ZSramPath = ZCfgPath;
    }

    atexit(deinit_paths);
    return(true);
  }
  return(false);
}

char *strdupcat(const char *str1, const char *str2)
{
  char *strnew = malloc(strlen(str1)+strlen(str2)+1);
  if (strnew)
  {
    strcpy(strnew, str1);
    strcat(strnew, str2);
    return(strnew);
  }
  return(0);
}

//This function is only for this file, and it uses an internal buffer
static const char *strdupcat_internal(const char *str1, const char *str2)
{
  static char buffer_dir[PATH_SIZE*2];
  strcpy(buffer_dir, str1);
  strcat(buffer_dir, str2);
  return(buffer_dir);
}

int access_dir(const char *path, const char *file, int mode)
{
  return(access(strdupcat_internal(path, file), mode));
}

int stat_dir(const char *path, const char *file, struct stat *buf)
{
  return(stat(strdupcat_internal(path, file), buf));
}

FILE *fopen_dir(const char *path, const char *file, const char *mode)
{
  return(fopen(strdupcat_internal(path, file), mode));
}

int remove_dir(const char *path, const char *file)
{
  return(remove(strdupcat_internal(path, file)));
}

int mkdir_dir(const char *path, const char *dir)
{
  return(mkdir_p(strdupcat_internal(path, dir)));
}

void strcatslash(char *str)
{
  if (str[strlen(str)-1] != DIR_SLASH_C)
  {
    strcat(str, DIR_SLASH);
  }
}
