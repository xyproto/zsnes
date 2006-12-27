/*
Copyright (C) 2005-2007 Nach, grinvader ( http://www.zsnes.com )

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

/*
This is part of a toolkit used to assist in ZSNES development
*/

#ifndef FILEUTIL_H
#define FILEUTIL_H

#include <string.h>
#include <sys/stat.h>

void parse_dir(const char *, void (*func)(const char *, struct stat&));
bool parse_path(const char *, void (*func)(const char *, struct stat&));

inline bool extension_match(const char *filename, const char *ext)
{
  size_t filen_len = strlen(filename);
  size_t ext_len = strlen(ext);
  return((filen_len > ext_len) && !strcasecmp(filename+filen_len-ext_len, ext));
}

inline bool is_c_file(const char *filename)
{
  return(extension_match(filename, ".c") ||
         extension_match(filename, ".h"));
}

inline bool is_cpp_file(const char *filename)
{
  return(extension_match(filename, ".cpp"));
}

inline bool is_psr_file(const char *filename)
{
  return(extension_match(filename, ".psr"));
}

inline bool is_asm_file(const char *filename)
{
  return(extension_match(filename, ".asm") ||
         extension_match(filename, ".inc") ||
         extension_match(filename, ".mac"));
}

#endif
