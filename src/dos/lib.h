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

#ifndef LIB_H
#define LIB_H

char *realpath_sfn(const char *file, char *buf);
char *realpath_lfn(const char *file, char *buf);

//Only beta versions of DJGPP currently have this function
#if (__DJGPP__ < 3) && (__DJGPP_MINOR__ < 4)
#define realpath(path, resolved_path) realpath_sfn(path, resolved_path)
#endif

unsigned int GetLogicalDrives();

#endif
