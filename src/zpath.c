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
#ifdef __MSDOS__
#include <dos.h>
#include <ctype.h>
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
char *ZSnapPath = 0, *ZSpcPath = 0;
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

//Windows for some reason lacks this function
char *realpath(const char *path, char *resolved_path)
{
  char *ret = 0;

  if (!path || !resolved_path) { errno = EINVAL; }
  else if (!access(path, F_OK))
  {
    ret = fullpath(resolved_path, path, MAX_PATH);
  }

  return(ret);
}

#elif defined(__MSDOS__) && (__DJGPP__ < 3) && (__DJGPP_MINOR__ < 4)

/*
Memory Swapper - designed to swap the positions of two chunks in an array
p1 is first chunk in array
p2 is second chunk in array
p2len is the length of the second chunk

p2 will be moved to before p1
*/
static void memswap(void *p1, void *p2, size_t p2len)
{
  char *ptr1 = (char *)p1;
  char *ptr2 = (char *)p2;

  size_t p1len = ptr2 - ptr1;
  unsigned char byte;
  while (p2len--)
  {
    byte = *ptr2++;
    memmove(ptr1+1, ptr1, p1len);
    *ptr1++ = byte;
  }
}

//Only beta versions of DJGPP currently have this function
char *realpath(const char *path, char *resolved_path)
{
  char *ret = 0;
  if (!path || !resolved_path) { errno = EINVAL; }
  else if (!access(path, F_OK))
  {
    unsigned int saved_drive = 0;
    char *p;
    ret = strcpy(resolved_path, path);
    natify_slashes(resolved_path);

    //Make sure path is absolute
    if (*resolved_path && (resolved_path[1] == ':') && (resolved_path[2] != DIR_SLASH_C))
    {
      unsigned int available_drives;
      _dos_getdrive(&saved_drive);
      _dos_setdrive((toupper(*resolved_path)+1)-'A', &available_drives);
      memmove(resolved_path, resolved_path+2, strlen(resolved_path+2)+1);
    }
    if ((strlen(resolved_path) < 2) || (resolved_path[1] != ':'))
    {
      if (*resolved_path == DIR_SLASH_C)
      {
        unsigned int drive;
        _dos_getdrive(&drive);
        memmove(resolved_path+2, resolved_path, strlen(resolved_path)+1);
        *resolved_path = ((char)drive-1)+'A';
        resolved_path[1] = ':';
      }
      else
      {
        memmove(resolved_path+1, resolved_path, strlen(resolved_path)+1);
        *resolved_path = DIR_SLASH_C;
        p = getcwd(resolved_path+strlen(resolved_path)+1, PATH_SIZE-strlen(resolved_path)+1);
        if (saved_drive) { _dos_setdrive(saved_drive, &saved_drive); }
        if (!p)
        {
          errno = ENAMETOOLONG;
          return(0); //I didn't want this here, but at the moment, I can't think of a way to restructure the code
        }
        natify_slashes(p);
        memswap(resolved_path, p, strlen(p)+1);
        memmove(resolved_path+strlen(resolved_path), resolved_path+strlen(resolved_path)+1, strlen(resolved_path+strlen(resolved_path)+1)+1);
      }
    }

    //Remove doubled over backslashes
    p = resolved_path;
    while (*p)
    {
      while ((*p == DIR_SLASH_C) && (p[1] == DIR_SLASH_C))
      {
        memmove(p, p+1, strlen(p));
      }
      p++;
    }

    //Remove uselss current directory characters
    while ((p = strstr(resolved_path, "\\.\\")))
    {
      memmove(p+1, p+3, strlen(p+3)+1);
    }

    //Convert "\.." to "\..\" at end of path
    if (!memcmp(resolved_path+strlen(resolved_path)-3, "\\..", 4))
    {
      strcat(resolved_path, DIR_SLASH);
    }

    //Remove useless previous directory characters
    while ((p = strstr(resolved_path, "\\..\\")))
    {
      if (p[-1] == ':')
      {
        memmove(p, p+3, strlen(p+3)+1);
      }
      else
      {
        char *p2;
        *p = 0;
        p2 = strrchr(resolved_path, DIR_SLASH_C); //This can't fail
        memmove(p2+1, p+4, strlen(p+4)+1);
      }
    }
  }
  return(ret);
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
        init_save_paths();

        atexit(deinit_paths);
        return(true);
      }
    }
  }
  return(false);
}

void init_save_paths()
{
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
  else
  {
    strcpy(ZRomPath, ZStartPath);
  }
  strcatslash(ZRomPath);

  if (*SnapPath)
  {
    ZSnapPath = SnapPath;
  }
  else
  {
    ZSnapPath = ZSramPath;
  }
  strcatslash(ZSnapPath);

  if (*SPCPath)
  {
    ZSpcPath = SPCPath;
  }
  else
  {
    ZSpcPath = ZSramPath;
  }
  strcatslash(ZSpcPath);
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
