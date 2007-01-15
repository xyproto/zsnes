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

#ifdef __UNIXSDL__
#include "gblhdr.h"
#include "linux/safelib.h"
#include <pwd.h>
#else
#ifdef __WIN32__
#include <io.h>
#include <direct.h>
#include "win/safelib.h"
#include "win/lib.h"
#else
#include "dos/lib.h"
#include <unistd.h>
#endif
#include <string.h>
#include <stdlib.h>
#endif
#include <errno.h>

#include "zpath.h"
#include "cfg.h"

#ifdef __WIN32__
#define mkdir_p(path) mkdir(path)
#else
#define mkdir_p(path) mkdir(path, mmode)
#endif

#ifdef __MSDOS__
char ZCfgFile[] = "zsnes.cfg";
#elif defined(__WIN32__)
char ZCfgFile[] = "zsnesw.cfg";
#else
char ZCfgFile[] = "zsnesl.cfg";
#endif

char *ZStartPath = 0, *ZCfgPath = 0, *ZSramPath = 0, *ZRomPath = 0;
char *ZSnapPath = 0, *ZSpcPath = 0;
char *ZCartName = 0, *ZSaveName = 0, *ZStateName = 0, *ZSaveST2Name = 0;

static bool ZStartAlloc = false, ZCfgAlloc = false, ZSramAlloc = false, ZRomAlloc = false;
static bool ZCartAlloc = false, ZSaveAlloc = false, ZStateAlloc = false, ZSaveST2Alloc = false;

#ifdef __UNIXSDL__

void cfgpath_ensure(const char *launch_command)
{
  struct passwd *userinfo;
#ifdef ZCONF
  const char *const zpath = ZCONF;
#else
#ifndef __MACOSX__
  const char *const zpath = ".zsnes";
#else
  const char *const zpath = "Library/Application Support/ZSNES";
#endif
#endif

  if ((userinfo = getpwuid(getuid())))
  {
    ZCfgPath = malloc(PATH_SIZE);
  }
  else
  {
    puts("Error obtaining info about your user.");
  }

  if (ZCfgPath)
  {
    struct stat stat_buffer;

    ZCfgAlloc = true;
    strcpy(ZCfgPath, userinfo->pw_dir);
    strcatslash(ZCfgPath);
    strcat(ZCfgPath, zpath);

    if (mkpath(ZCfgPath, 0755) && !stat(ZCfgPath, &stat_buffer) && S_ISDIR(stat_buffer.st_mode) && !access(ZCfgPath, W_OK))
    {
      strcatslash(ZCfgPath);
    }
    else
    {
      printf("Error creating: %s\n", ZCfgPath);
      free(ZCfgPath);
      ZCfgAlloc = false;

      ZCfgPath = ZStartPath;
    }
  }
  else
  {
    ZCfgPath = ZStartPath;
  }
}

#else

void cfgpath_ensure(const char *launch_command)
{
  ZCfgPath = malloc(PATH_SIZE);
  if (ZCfgPath)
  {
    char *p = 0;
    ZCfgAlloc = true;

    if (isextension(launch_command, "exe"))
    {
      p = realpath(launch_command, ZCfgPath);
    }
    else
    {
      char buff[PATH_SIZE];
      strcpy(buff, launch_command);
      setextension(buff, "exe");
      p = realpath(buff, ZCfgPath);
    }

    if (p)
    {
      strdirname(ZCfgPath);
      strcatslash(ZCfgPath);
    }
    else
    {
      free(ZCfgPath);
      ZCfgAlloc = false;
      ZCfgPath = ZStartPath;
    }
  }
  else
  {
    ZCfgPath = ZStartPath;
  }
}

#endif

void SaveGameSpecificInput();

void deinit_paths()
{
  //Save data that depends on paths before deinit of them
  void SaveSramData();
  void GUISaveVars();

  strcpy(ROMPath, ZRomPath);

  SaveSramData();
  GUISaveVars();
  SaveGameSpecificInput();

  //Now deallocate the paths
  if (ZStartAlloc && ZStartPath) { free(ZStartPath); }
  if (ZCfgAlloc && ZCfgPath) { free(ZCfgPath); }
  if (ZSramAlloc && ZSramPath) { free(ZSramPath); }
  if (ZRomAlloc && ZRomPath) { free(ZRomPath); }

  if (ZCartAlloc && ZCartName) { free(ZCartName); }
  if (ZSaveAlloc && ZSaveName) { free(ZSaveName); }
  if (ZStateAlloc && ZStateName) { free(ZStateName); }
  if (ZSaveST2Alloc && ZSaveST2Name) { free(ZSaveST2Name); }
}

#define INIT_PATH_HELPER(x) if ((x##Path = malloc(PATH_SIZE))) { x##Alloc = true; } else { return(false); }
#define INIT_NAME_HELPER(x) if ((x##Name = malloc(NAME_SIZE))) { x##Alloc = true; *x##Name = 0; } else { return(false); }

bool init_paths(char *launch_command)
{
  void GUIRestoreVars();

  INIT_PATH_HELPER(ZStart);
  INIT_PATH_HELPER(ZRom);
  INIT_NAME_HELPER(ZCart);
  INIT_NAME_HELPER(ZSave);
  INIT_NAME_HELPER(ZState);
  INIT_NAME_HELPER(ZSaveST2);

  if (getcwd(ZStartPath, PATH_SIZE))
  {
    strcatslash(ZStartPath);

    cfgpath_ensure(launch_command);

    GUIRestoreVars();

    if (*ROMPath && !access(strcutslash(ROMPath), DIR_R_ACCESS))
    {
      strcpy(ZRomPath, ROMPath);
    }
    else
    {
      strcpy(ZRomPath, ZStartPath);
    }
    strcatslash(ZRomPath);

    init_save_paths();

#ifdef DEBUG
#ifndef __UNIXSDL__
    fdreopen_dir(ZCfgPath, "stderr.txt", "w", STDERR_FILENO);
    fdreopen_dir(ZCfgPath, "stdout.txt", "w", STDOUT_FILENO);
#endif

    printf("ZStartPath: %s\n", ZStartPath);
    printf("ZCfgPath: %s\n", ZCfgPath);
    printf("ZRomPath: %s\n", ZRomPath);
    printf("ZSramPath: %s\n", ZSramPath);
    printf("ZSnapPath: %s\n", ZSnapPath);
    printf("ZSpcPath: %s\n", ZSpcPath);
#endif
    return(true);
  }
  return(false);
}

void init_save_paths()
{
  if (*SRAMPath)
  {
    ZSramPath = SRAMPath;
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
  if (realpath_link(path, ZRomPath))
  {
    char *p;
    SaveGameSpecificInput();

    natify_slashes(ZRomPath);
    p = strrchr(ZRomPath, DIR_SLASH_C);
    strcpy(ZCartName, (p) ? p+1 : ZRomPath);
    strcpy(ZSaveName, ZCartName);
    strcpy(ZStateName, ZCartName);
    setextension(ZStateName, "zst");

    strdirname(ZRomPath);
    strcatslash(ZRomPath);

#ifdef DEBUG
    printf("ZRomPath: %s\n", ZRomPath);
    printf("ZCartName: %s\n", ZCartName);
    printf("ZStateName: %s\n", ZStateName);
#endif

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
//This function is only for this file, and it uses an internal buffer, and is intended for path file merging
static const char *strdupcat_internal(const char *path, const char *file)
{
  static char buffer_dir[PATH_SIZE*2];
  if (!IS_ABSOLUTE(file))
  {
    strcpy(buffer_dir, path);
  }
  else
  {
    *buffer_dir = 0;
  }
  strcat(buffer_dir, file);
  return(buffer_dir);
}

#define chdir_dir(path) chdir(path);

#else

static const char *strdupcat_internal(const char *path, const char *file, const char *func, const char *mode)
{
  static char buffer_dir[PATH_SIZE*2];
  if (!IS_ABSOLUTE(file))
  {
    strcpy(buffer_dir, path);
  }
  else
  {
    *buffer_dir = 0;
  }
  strcat(buffer_dir, file);

#ifndef NO_DEBUGGER
  // maybe checking isendwin() would be better anyway, but only after we scrap
  // the old debugger, because that won't work when not actually using curses
  if (!debuggeron) {
#endif
  if (mode)
  {
    printf("%s_%s: %s\n", func, mode, buffer_dir);
  }
  else
  {
    printf("%s: %s\n", func, buffer_dir);
  }
#ifndef NO_DEBUGGER
  }
#endif

  return(buffer_dir);
}

//This is to keep the modeless functions working right
static const char *mode = 0;
static const char *mode_text = 0;

#define strdupcat_internal(x, y) strdupcat_internal(x, y, __func__, mode ? mode : mode_text)

int chdir_internal(const char *path, const char *func, const char *command)
{
  printf("%s: %s: %s\n", func, path, command);
  return(chdir(path));
}

#define chdir_dir(path) chdir_internal(path, __func__, command);

#endif


int access_dir(const char *path, const char *file, int amode)
{
#ifdef DEBUG
  char mode_text[5];
  strcpy(mode_text, "f");
  if (amode & R_OK) { strcat(mode_text, "r"); }
  if (amode & W_OK) { strcat(mode_text, "w"); }
  if (amode & X_OK) { strcat(mode_text, "x"); }
#endif

  return(access(strdupcat_internal(path, file), amode));
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
  load_jma_file(strdupcat_internal(path, file));
}
#endif

int remove_dir(const char *path, const char *file)
{
  return(remove(strdupcat_internal(path, file)));
}

int mkdir_dir(const char *path, const char *dir)
{
  mode_t mmode = 0755;
  return(mkdir_p(strdupcat_internal(path, dir)));
}

char *realpath_dir(const char *path, const char *file, char *buf)
{
#ifdef __UNIXSDL__
  return(realpath_tilde(strdupcat_internal(path, file), buf));
#else
  return(realpath(strdupcat_internal(path, file), buf));
#endif
}

#ifdef __MSDOS__
char *realpath_sfn_dir(const char *path, const char *file, char *buf)
{
  return(realpath_sfn(strdupcat_internal(path, file), buf));
}
#endif

FILE *fdreopen_dir(const char *path, const char *file, const char *mode, int fd)
{
  //Because DOSBox and Windows is stupid, we're implementing this manually;
  FILE *fp = fopen(strdupcat_internal(path, file), mode);
  if (fp)
  {
    dup2(fileno(fp), fd);
  }
  return(fp);
}

int system_dir(const char *path, const char *command)
{
  int ret_val;
  chdir_dir(path);
  ret_val = system(command);
  chdir(ZStartPath);
  return(ret_val);
}

FILE *popen_dir(const char *path, char *command, const char *type)
{
  FILE *ret_val;
  chdir_dir(path);
  ret_val = popen(command, type);
  chdir(ZStartPath);
  return(ret_val);
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

char *strcutslash(char *str)
{
  char *last_char = str+(strlen(str)-1);

  natify_slashes(str);
  if (*last_char == DIR_SLASH_C)
  {
    *last_char = 0;
  }
  return(str);
}

char *strcatslash(char *str)
{
  natify_slashes(str);
  if (str[strlen(str)-1] != DIR_SLASH_C)
  {
    strcat(str, DIR_SLASH);
  }
  return(str);
}

void setextension(char *base, const char *ext)
{
  char *p = strrchr(base, '.');

  if(p)
  {
    strcpy(p+1, ext);
  }
  else
  {
    strcat(base, ".");
    strcat(base, ext);
  }
}

bool isextension(const char *fname, const char *ext)
{
  size_t fname_len = strlen(fname),
         ext_len = strlen(ext);
  return((fname[fname_len-(ext_len+1)] == '.') && !strcasecmp(fname+fname_len-ext_len, ext));
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
  else if (p == str)
  {
    str[1] = 0;
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

static bool mkpath_help(char *path, char *element, mode_t mmode)
{
  bool success = true;
  if (*path)
  {
    char *p;
    bool created;

    while (*element == DIR_SLASH_C) { element++; }

    if (*element)
    {
      p = strchr(element, DIR_SLASH_C);
      if (p) { *p = 0; }
      if ((created = !mkdir_p(path)) || (errno == EEXIST)) //Current path fragment created or already exists
      {
        if (p)
        {
          *p = DIR_SLASH_C;
          if (!mkpath_help(path, p+1, mmode)) //If creation of next fragment fails
          {
            if (created)
            {
              *p = 0;
              rmdir(path);
            }
            success = false;
          }
        }
      }
      else { success = false; }
    }
  }
  return(success);
}

bool mkpath(const char *path, mode_t mode)
{
  bool success = true;
  if (path && *path)
  {
    char *p = strdup(path);
    if (p)
    {
      natify_slashes(p);
      success = mkpath_help(p, p, mode);
      free(p);
    }
    else { success = false; }
  }
  return(success);
}

#ifdef __UNIXSDL__

//Like realpath(), but will return the last element as the link it is
char *realpath_link(const char *path, char *resolved_path)
{
  char buffer[PATH_SIZE], *p, *base, *last_element;
  strcpy(buffer, path);
  natify_slashes(buffer);
  p = strrchr(buffer, DIR_SLASH_C);
  if (p)
  {
    *p = 0;
    base = buffer;
    last_element = p+1;
  }
  else
  {
    base = ".";
    last_element = buffer;
  }

  p = realpath(base, resolved_path);
  if (p)
  {
    strcatslash(resolved_path);
    strcat(resolved_path, last_element);
    return(resolved_path);
  }
  return(0);
}

//realpath() with ~ support
char *realpath_tilde(const char *path, char *resolved_path)
{
  if (*path == '~')
  {
    char buffer[PATH_SIZE];
    struct passwd *userinfo;

    strcpy(buffer, "~");
    path++;

    if (isalpha(*path))
    {
      char *p = buffer+1;
      while (isalnum(*path))
      {
        *p++ = *path++;
      }
      *p = 0;
      if ((userinfo = getpwnam(buffer+1)))
      {
        strcpy(buffer, userinfo->pw_dir);
      }
    }
    else
    {
      if ((userinfo = getpwuid(getuid())))
      {
        strcpy(buffer, userinfo->pw_dir);
      }
    }
    strcat(buffer, path);

    return(realpath(buffer, resolved_path));
  }
  return(realpath(path, resolved_path));
}

#endif

void psr_cfg_run(unsigned char (*psr_func)(const char *), const char *dir, const char *fname)
{
  char *path = strdupcat(dir, fname);
  if (path)
  {
    psr_func(path);
    free(path);
  }
  else
  {
    psr_func(fname);
  }
}

