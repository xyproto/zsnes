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

#ifndef ZLOADER_H
#define ZLOADER_H

struct backup_cmdline_vars
{
#ifdef __MSDOS__
  unsigned char _Palette0;
#endif
#ifdef __WIN32__
  unsigned char _KitchenSync, _KitchenSyncPAL, _ForceRefreshRate, _SetRefreshRate;
#endif
#ifndef __MSDOS__
  unsigned short _joy_sensitivity;
#endif
  unsigned char _guioff;
  unsigned char _per2exec;
  unsigned char _HacksDisable;
  unsigned char _AllowMMX;
};

extern struct backup_cmdline_vars saved_cmdline_vars;

void swap_backup_vars();

#endif
