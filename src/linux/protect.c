//Copyright (C) 1997-2004 ZSNES Team ( zsknight@zsnes.com / _demo_@zsnes.com )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later
//version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "gblhdr.h"

extern void CfgLoadAsmStart();
extern void CfgLoadAsmEnd();
extern void CopyVWinAsmStart();
extern void CopyVWinAsmEnd();
extern void DebugAsmStart();
extern void DebugAsmEnd();
extern void JoyAsmStart();
extern void JoyAsmEnd();
extern void DosModemRTNAsmStart();
extern void DosModemRTNAsmEnd();
extern void Vesa2AsmStart();
extern void Vesa2AsmEnd();
extern void InitVidAsmStart();
extern void InitVidAsmEnd();
extern void SWAsmStart();
extern void SWAsmEnd();
extern void GPProAsmStart();
extern void GPProAsmEnd();
extern void Vesa12AsmStart();
extern void Vesa12AsmEnd();

#define MakeCodeWriteable(a, b) MakeCodeWriteable_f (a, b, #a)

// Thanks QuakeForge 
void MakeCodeWriteable_f (unsigned long startaddr, unsigned long length, const char *name) 
{ 
	int r; 
	unsigned int addr; 
	int psize = getpagesize(); 

	//fprintf(stderr, "Unprotecting 0x%x to 0x%x\n", startaddr, startaddr+length); 
	addr = (startaddr & ~(psize -1)) - psize; 

	/* Using 7 is a very stupid thing to do, but I'll leave it in commented form for the non-posix compliant zealots to ph34r... */ 
	//r = mprotect ((char *) addr, length + startaddr - addr + psize, 7); 
	r = mprotect ((char *) addr, length + startaddr - addr + psize, PROT_READ | PROT_WRITE | PROT_EXEC); 

	if (r < 0) 
		fprintf (stderr, "Error! Memory *NOT* unprotected. startaddr = 0x%08lx (%s)\n", startaddr, name); 
} 


void UnProtectMemory(void)
{
	MakeCodeWriteable((long) CopyVWinAsmStart, (long) CopyVWinAsmEnd - (long) CopyVWinAsmStart);
	MakeCodeWriteable((long) DebugAsmStart, (long) DebugAsmEnd - (long) DebugAsmStart);
	MakeCodeWriteable((long) JoyAsmStart, (long) JoyAsmEnd - (long) JoyAsmStart);
	MakeCodeWriteable((long) DosModemRTNAsmStart, (long) DosModemRTNAsmEnd - (long) DosModemRTNAsmStart);
	MakeCodeWriteable((long) Vesa2AsmStart, (long) Vesa2AsmEnd - (long) Vesa2AsmStart);
	MakeCodeWriteable((long) InitVidAsmStart, (long) InitVidAsmEnd - (long) InitVidAsmStart);
	MakeCodeWriteable((long) SWAsmStart, (long) SWAsmEnd - (long) SWAsmStart);
	MakeCodeWriteable((long) GPProAsmStart, (long) GPProAsmEnd - (long) GPProAsmStart);
	MakeCodeWriteable((long) Vesa12AsmStart, (long) Vesa12AsmEnd - (long) Vesa12AsmStart);
}
