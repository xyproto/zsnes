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

typedef unsigned char bool8;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef char int8;
typedef short int16;
typedef long int32;

//C++ in C
typedef unsigned char bool;
#define true 1
#define false 0

static uint8 *OBC1_RAM = 0;

int OBC1_Address;
int OBC1_BasePtr;
int OBC1_Shift;

uint16 obc1_address;
uint8 obc1_byte;

void GetOBC1 ()
{
	switch(obc1_address) {
		case 0x7ff0:
			obc1_byte = OBC1_RAM[OBC1_BasePtr + (OBC1_Address << 2)];
			break;

		case 0x7ff1:
			obc1_byte = OBC1_RAM[OBC1_BasePtr + (OBC1_Address << 2) + 1];
			break;

		case 0x7ff2:
			obc1_byte = OBC1_RAM[OBC1_BasePtr + (OBC1_Address << 2) + 2];
			break;

		case 0x7ff3:
			obc1_byte = OBC1_RAM[OBC1_BasePtr + (OBC1_Address << 2) + 3];
			break;

		case 0x7ff4:
			obc1_byte = OBC1_RAM[OBC1_BasePtr + (OBC1_Address >> 2) + 0x200];
			break;

		default:
			obc1_byte = OBC1_RAM[obc1_address & 0x1fff];
	}
}


void SetOBC1 ()
{
	switch(obc1_address) {
		case 0x7ff0:
		{
			OBC1_RAM[OBC1_BasePtr + (OBC1_Address << 2)] = obc1_byte;
			break;
		}

		case 0x7ff1:
		{
			OBC1_RAM[OBC1_BasePtr + (OBC1_Address << 2) + 1] = obc1_byte;
			break;
		}

		case 0x7ff2:
		{
			OBC1_RAM[OBC1_BasePtr + (OBC1_Address << 2) + 2] = obc1_byte;
			break;
		}

		case 0x7ff3:
		{
			OBC1_RAM[OBC1_BasePtr + (OBC1_Address << 2) + 3] = obc1_byte;
			break;
		}

		case 0x7ff4:
		{
			unsigned char Temp;

			Temp = OBC1_RAM[OBC1_BasePtr + (OBC1_Address >> 2) + 0x200];
			Temp = (Temp & ~(3 << OBC1_Shift)) | ((obc1_byte & 3) << OBC1_Shift);
			OBC1_RAM[OBC1_BasePtr + (OBC1_Address >> 2) + 0x200] = Temp;
			break;
		}

		case 0x7ff5:
		{
			if (obc1_byte & 1)
				OBC1_BasePtr = 0x1800;
			else
				OBC1_BasePtr = 0x1c00;

			break;
		}

		case 0x7ff6:
		{
			OBC1_Address = obc1_byte & 0x7f;
			OBC1_Shift = (obc1_byte & 3) << 1;
			break;
		}
	}

	OBC1_RAM[obc1_address & 0x1fff] = obc1_byte;
}

extern unsigned char *romdata;
void InitOBC1()
{
	OBC1_RAM = romdata+0x400000;
	if (OBC1_RAM[0x1ff5] & 1)
		OBC1_BasePtr = 0x1800;
	else
		OBC1_BasePtr = 0x1c00;

	OBC1_Address = OBC1_RAM[0x1ff6] & 0x7f;
	OBC1_Shift = (OBC1_RAM[0x1ff6] & 3) << 1;
}
