/*
Copyright (C) 1997-2005 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes

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

extern bool input1gp;
extern bool input1mouse;
extern bool input2gp;
extern bool input2mouse;
extern bool input2scope;
extern bool input2just;
extern unsigned char snesmouse;
void cycleinputdevice()
{
  for (;;)
  {
    snesmouse++;
    if (snesmouse >= 5)
    {
      snesmouse = 0;
    }
    if (snesmouse == 0)
    {
      if (input1gp && input2gp) { return; }
      snesmouse++;   
    }
    if (snesmouse == 1)
    {
      if (input1mouse) { return; }
      snesmouse++;   
    }
    if (snesmouse == 2)
    {
      if (input2mouse) { return; }
      snesmouse++;   
    }
    if (snesmouse == 3)
    {
      if (input2scope) { return; }
      snesmouse++;   
    }
    if (snesmouse == 4)
    {
      if (input2just) { return; }
    }
  }
}


