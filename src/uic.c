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

#include <string.h>

//C++ style code in C
#define bool unsigned char
#define true 1
#define false 0

extern void outofmemory();
extern int *spc7110romptr;
extern int *StateBackup;
extern int *spcBuffera;

int doMemAlloc(int *ptr, int size)
{
	int result = 0;

	ptr = malloc(size);
	if (ptr) result = 1;

	return result;
}

void allocspc7110()
{
	if (!doMemAlloc(spc7110romptr, 8192*1024+4096)) outofmemory();
}
