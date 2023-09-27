/*
Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

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

#ifndef ZPATH_H
#define ZPATH_H

#include <stdio.h>
#include <sys/stat.h>
#include "zip/zunzip.h"

// Max ROM Space
#define MAXROMSPACE 0x800000

#ifdef _MSC_VER
#define F_OK 0
#define X_OK F_OK // Drop down to F_OK because MSVC is stupid
#define W_OK 2
#define R_OK 4
typedef unsigned short mode_t;
#endif

#if !defined(__cplusplus) && !defined(bool)
// C++ style code in C
#include <stdbool.h>
#endif

#define DIR_SLASH "/"
#define DIR_SLASH_C '/'
#define DIR_SLASH_C_OTHER '\\'
#define ROOT_LEN 1 //"/"
#define DIR_R_ACCESS (R_OK | X_OK)
#define IS_ABSOLUTE(path) ((*(path) == '/') || (*(path) == '~'))

#define PATH_SIZE 4096
#define NAME_SIZE 512
#define realpath_native realpath

#ifndef S_ISDIR
#define S_ISDIR(m) (((m)&S_IFMT) == S_IFDIR)
#endif

extern char ZCfgFile[];
extern char *ZStartPath, *ZCfgPath, *ZSramPath, *ZRomPath;
extern char *ZSnapPath, *ZSpcPath, *ZIpsPath, *ZMoviePath;
extern char *ZChtPath, *ZComboPath, *ZInpPath, *ZSStatePath;
extern char *ZCartName, *ZSaveName, *ZStateName, *ZSaveST2Name;

bool init_paths(char *launch_command);
void init_save_paths(void);
bool init_rom_path(char *path);

char *strdupcat(const char *str1, const char *str2);

int access_dir(const char *path, const char *file, int mode);
int stat_dir(const char *path, const char *file, struct stat *buf);
FILE *fopen_dir(const char *path, const char *file, const char *mode);
gzFile gzopen_dir(const char *path, const char *file, const char *mode);
unzFile unzopen_dir(const char *path, const char *file);
int remove_dir(const char *path, const char *file);
int mkdir_dir(const char *path, const char *dir);
char *realpath_dir(const char *path, const char *file, char *buf);
FILE *fdreopen_dir(const char *path, const char *file, const char *mode, int fd);
int system_dir(const char *path, const char *command);
FILE *popen_dir(const char *path, char *command, const char *type);

void natify_slashes(char *str);
char *strcutslash(char *str);
char *strcatslash(char *str);
void setextension(char *base, const char *ext);
bool isextension(const char *fname, const char *ext);
void strdirname(char *str);
void strbasename(char *str);
bool mkpath(const char *path, mode_t mode);
char *realpath_link(const char *path, char *resolved_path);
char *realpath_tilde(const char *path, char *resolved_path);

void psr_cfg_run(unsigned char (*psr_func)(const char *), const char *dir, const char *fname);

#endif
