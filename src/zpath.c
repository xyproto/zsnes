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
#ifdef __WIN32__
#include <io.h>
#else
#include <unistd.h>
#endif
#include <string.h>
#include <stdlib.h>
#endif
#include <errno.h>

#include "zpath.h"
#include "cfg.h"

#ifdef __WIN32__
#define fullpath _fullpath
#define mkdir_p(path) mkdir(path)
#else
#define mkdir_p(path) mkdir(path, (S_IRWXU|S_IRWXG|S_IRWXO)) //0777
#endif

#define PATH_SIZE 4096
#define NAME_SIZE 512

#ifdef __MSDOS__
char ZCfgFile[] = "zsnes.cfg";
#elif defined(__WIN32__)
char ZCfgFile[] = "zsnesw.cfg";
#else
char ZCfgFile[] = "zsnesl.cfg";
#endif

char *ZStartPath = 0, *ZCfgPath = 0, *ZSramPath = 0, *ZRomPath = 0;
char *ZCartName = 0;

static bool ZStartAlloc = false, ZCfgAlloc = false, ZSramAlloc = false, ZRomAlloc = false;
static bool ZCartAlloc = false;

#ifdef __UNIXSDL__

void cfgpath_ensure()
{
  struct passwd *userinfo;
  const char *const zpath = ".zsnes";

  if ((userinfo = getpwuid(getuid())))
  {
    ZCfgPath = malloc(PATH_SIZE);
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
  if (ZRomAlloc && ZRomPath) { free(ZRomPath); }

  if (ZCartAlloc && ZCartName) { free(ZCartName); }
}

bool init_paths(char *launch_command)
{
  void GUIRestoreVars();

  ZStartPath = malloc(PATH_SIZE);
  if (ZStartPath)
  {
    ZStartAlloc = true;

    ZRomPath = malloc(PATH_SIZE);
    if (ZRomPath)
    {
      ZRomAlloc = true;

      ZCartName = malloc(NAME_SIZE);
      if (ZCartName)
      {
        ZCartAlloc = true;
        *ZCartName = 0;

        if (realpath(launch_command, ZStartPath))
        {
          strdirname(ZStartPath);
        }
        else
        {
          getcwd(ZStartPath, PATH_SIZE);
        }
        strcatslash(ZStartPath);

        cfgpath_ensure();

        GUIRestoreVars();

        if (*SRAMDir)
        {
          ZSramPath = SRAMDir;
        }
        else
        {
          #ifdef __UNIXSDL__
          ZSramPath = ZCfgPath;
          #else
          ZSramPath = ZRomPath;
          #endif
        }
        strcatslash(ZSramPath);

        if (*LoadDir)
        {
          strcpy(ZRomPath, LoadDir);
        }
        strcatslash(ZRomPath);

        atexit(deinit_paths);
        return(true);
      }
    }
  }
  return(false);
}

bool init_rom_path(char *path)
{
  if (realpath(path, ZRomPath))
  {
    char *p;

    natify_slashes(ZRomPath);
    p = strrchr(ZRomPath, DIR_SLASH_C);
    if (p)
    {
      strcpy(ZCartName, p+1);
    }
    else
    {
      strcpy(ZCartName, ZRomPath);
    }
    strdirname(ZRomPath);
    strcatslash(ZRomPath);

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

#ifndef DEBUG
//This function is only for this file, and it uses an internal buffer
static const char *strdupcat_internal(const char *str1, const char *str2)
{
  static char buffer_dir[PATH_SIZE*2];
  strcpy(buffer_dir, str1);
  strcat(buffer_dir, str2);
  return(buffer_dir);
}

#else

static const char *strdupcat_internal(const char *str1, const char *str2, const char *func)
{
  static char buffer_dir[PATH_SIZE*2];
  strcpy(buffer_dir, str1);
  strcat(buffer_dir, str2);

  printf("%s: %s\n", func, buffer_dir);

  return(buffer_dir);
}

#define strdupcat_internal(x, y) strdupcat_internal(x, y, __func__)
#endif

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

gzFile gzopen_dir(const char *path, const char *file, const char *mode)
{
  return(gzopen(strdupcat_internal(path, file), mode));
}

unzFile unzopen_dir(const char *path, const char *file)
{
  return(unzOpen(strdupcat_internal(path, file)));
}

#ifndef NO_JMA
void load_jma_file_dir(const char *path, const char *file)
{
  return(load_jma_file(strdupcat_internal(path, file)));
}
#endif

int remove_dir(const char *path, const char *file)
{
  return(remove(strdupcat_internal(path, file)));
}

int mkdir_dir(const char *path, const char *dir)
{
  return(mkdir_p(strdupcat_internal(path, dir)));
}

void natify_slashes(char *str)
{
  while (*str)
  {
    if (*str == DIR_SLASH_C_OTHER)
    {
      *str = DIR_SLASH_C;
    }
    str++;
  }
}

void strcatslash(char *str)
{
  natify_slashes(str);
  if (str[strlen(str)-1] != DIR_SLASH_C)
  {
    strcat(str, DIR_SLASH);
  }
}

void strdirname(char *str)
{
  char *p;
  size_t str_len = strlen(str);

  natify_slashes(str);

  do
  {
    str_len--;
  } while (str_len && (str[str_len] == DIR_SLASH_C));
  str[str_len+1] = 0;

  p = strrchr(str, DIR_SLASH_C);
  if (p > str)
  {
    *p = 0;
  }
}

void strbasename(char *str)
{
  char *p;

  natify_slashes(str);

  if ((p = strrchr(str, DIR_SLASH_C)))
  {
    memmove(str, p+1, strlen(p));
  }
}
