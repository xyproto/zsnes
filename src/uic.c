/*
Copyright (C) 2003 ZSNES Team ( zsknight@zsnes.com / _demo_@zsnes.com )

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

#include <stdlib.h>

//C++ style code in C
#define bool unsigned char
#define true 1
#define false 0

extern void outofmemory();
extern unsigned char *spc7110romptr;
extern unsigned char *StateBackup;
extern unsigned char *spcBuffera;
extern unsigned char *spritetablea;
extern unsigned char *vbufaptr;
extern unsigned char *vbufeptr;
extern unsigned char *ngwinptrb;
extern unsigned char *vbufdptr;
extern unsigned char *vcache2bs;
extern unsigned char *vcache4bs;
extern unsigned char *vcache8bs;
extern unsigned char *RGBtoYUVPtr;
extern unsigned char *romaptr;


void *doMemAlloc(int size)
{
  void *ptr = NULL;
  ptr = malloc(size);
  if (!ptr) { outofmemory(); }
  return(ptr);
}

void allocspc7110()
{
  spc7110romptr = (unsigned char *)doMemAlloc(8192*1024+4096);
}
