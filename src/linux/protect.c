//Copyright (C) 1997-2001 ZSNES Team ( zsknight@zsnes.com / _demo_@zsnes.com )
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

extern void GuiAsmStart();
extern void GuiAsmEnd();
extern void SfxProcAsmStart();
extern void SfxProcAsmEnd();
extern void FxEmu2AsmStart();
extern void FxEmu2AsmEnd();
extern void Dsp1ProcAsmStart();
extern void Dsp1ProcAsmEnd();
extern void FxEmu2BAsmStart();
extern void FxEmu2BAsmEnd();
extern void FxEmu2CAsmStart();
extern void FxEmu2CAsmEnd();
extern void FxTableAsmStart();
extern void FxTableAsmEnd();
extern void ExecuteAsmStart();
extern void ExecuteAsmEnd();
extern void Sa1ProcAsmStart();
extern void Sa1ProcAsmEnd();
extern void Sa1RegsAsmStart();
extern void Sa1RegsAsmEnd();
extern void WinIntRFAsmStart();
extern void WinIntRFAsmEnd();
extern void ProcVidAsmStart();
extern void ProcVidAsmEnd();
extern void CfgLoadAsmStart();
extern void CfgLoadAsmEnd();
extern void IrqAsmStart();
extern void IrqAsmEnd();
extern void MemoryAsmStart();
extern void MemoryAsmEnd();
extern void Spc700AsmStart();
extern void Spc700AsmEnd();
extern void StableAsmStart();
extern void StableAsmEnd();
extern void DspProcAsmStart();
extern void DspProcAsmEnd();
extern void DmaAsmStart();
extern void DmaAsmEnd();
extern void DspAsmStart();
extern void DspAsmEnd();
extern void TableAsmStart();
extern void TableAsmEnd();
extern void TableBAsmStart();
extern void TableBAsmEnd();
extern void TableCAsmStart();
extern void TableCAsmEnd();
extern void CopyVWinAsmStart();
extern void CopyVWinAsmEnd();
extern void DebugAsmStart();
extern void DebugAsmEnd();
extern void JoyAsmStart();
extern void JoyAsmEnd();
extern void InitAsmStart();
extern void InitAsmEnd();
extern void UIAsmStart();
extern void UIAsmEnd();
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
extern void MenuAsmStart();
extern void MenuAsmEnd();
extern void MakeV16BAsmStart();
extern void MakeV16BAsmEnd();
extern void MakeV16TAsmStart();
extern void MakeV16TAsmEnd();
extern void MakeVidAsmStart();
extern void MakeVidAsmEnd();
extern void Mode716AsmStart();
extern void Mode716AsmEnd();
extern void Mode716BAsmStart();
extern void Mode716BAsmEnd();
extern void Mode716DAsmStart();
extern void Mode716DAsmEnd();
extern void Mode716EAsmStart();
extern void Mode716EAsmEnd();
extern void Mode716TAsmStart();
extern void Mode716TAsmEnd();
extern void Mode7AsmStart();
extern void Mode7AsmEnd();
extern void Mode7ExtAsmStart();
extern void Mode7ExtAsmEnd();
extern void M716TExtAsmStart();
extern void M716TExtAsmEnd();
extern void TwoxSaiWAsmStart();
extern void TwoxSaiWAsmEnd();
extern void EndMemAsmStart();
extern void EndMemAsmEnd();
extern void MV16TMSAsmStart();
extern void MV16TMSAsmEnd();
extern void NewG162AsmStart();
extern void NewG162AsmEnd();
extern void NewGfx16AsmStart();
extern void NewGfx16AsmEnd();
extern void NewGfx2AsmStart();
extern void NewGfx2AsmEnd();
extern void NewGfxAsmStart();
extern void NewGfxAsmEnd();
extern void VCacheAsmStart();
extern void VCacheAsmEnd();

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
	MakeCodeWriteable((long) SfxProcAsmStart, (long) SfxProcAsmEnd - (long) SfxProcAsmStart);
	MakeCodeWriteable((long) FxEmu2AsmStart, (long) FxEmu2AsmEnd - (long) FxEmu2AsmStart);
	MakeCodeWriteable((long) Dsp1ProcAsmStart, (long) Dsp1ProcAsmEnd - (long) Dsp1ProcAsmStart);
	MakeCodeWriteable((long) FxEmu2BAsmStart, (long) FxEmu2BAsmEnd - (long) FxEmu2BAsmStart);
	MakeCodeWriteable((long) FxEmu2CAsmStart, (long) FxEmu2CAsmEnd - (long) FxEmu2CAsmStart);
	MakeCodeWriteable((long) FxTableAsmStart, (long) FxTableAsmEnd - (long) FxTableAsmStart);
	MakeCodeWriteable((long) ExecuteAsmStart, (long) ExecuteAsmEnd - (long) ExecuteAsmStart);
	MakeCodeWriteable((long) Sa1ProcAsmStart, (long) Sa1ProcAsmEnd - (long) Sa1ProcAsmStart);
	MakeCodeWriteable((long) Sa1RegsAsmStart, (long) Sa1RegsAsmEnd - (long) Sa1RegsAsmStart);
	MakeCodeWriteable((long) WinIntRFAsmStart, (long) WinIntRFAsmEnd - (long) WinIntRFAsmStart);
	MakeCodeWriteable((long) GuiAsmStart, (long) GuiAsmEnd - (long) GuiAsmStart);
	MakeCodeWriteable((long) ProcVidAsmStart, (long) ProcVidAsmEnd - (long) ProcVidAsmStart);
	MakeCodeWriteable((long) CfgLoadAsmStart, (long) CfgLoadAsmEnd - (long) CfgLoadAsmStart);
	MakeCodeWriteable((long) IrqAsmStart, (long) IrqAsmEnd - (long) IrqAsmStart);
	MakeCodeWriteable((long) MemoryAsmStart, (long) MemoryAsmEnd - (long) MemoryAsmStart);
	MakeCodeWriteable((long) Spc700AsmStart, (long) Spc700AsmEnd - (long) Spc700AsmStart);
	MakeCodeWriteable((long) StableAsmStart, (long) StableAsmEnd - (long) StableAsmStart);
	MakeCodeWriteable((long) DspProcAsmStart, (long) DspProcAsmEnd - (long) DspProcAsmStart);
	MakeCodeWriteable((long) DmaAsmStart, (long) DmaAsmEnd - (long) DmaAsmStart);
	MakeCodeWriteable((long) DspAsmStart, (long) DspAsmEnd - (long) DspAsmStart);
	MakeCodeWriteable((long) TableAsmStart, (long) TableAsmEnd - (long) TableAsmStart);
	MakeCodeWriteable((long) TableBAsmStart, (long) TableBAsmEnd - (long) TableBAsmStart);
	MakeCodeWriteable((long) TableCAsmStart, (long) TableCAsmEnd - (long) TableCAsmStart);
	MakeCodeWriteable((long) CopyVWinAsmStart, (long) CopyVWinAsmEnd - (long) CopyVWinAsmStart);
	MakeCodeWriteable((long) DebugAsmStart, (long) DebugAsmEnd - (long) DebugAsmStart);
	MakeCodeWriteable((long) JoyAsmStart, (long) JoyAsmEnd - (long) JoyAsmStart);
	MakeCodeWriteable((long) InitAsmStart, (long) InitAsmEnd - (long) InitAsmStart);
	MakeCodeWriteable((long) UIAsmStart, (long) UIAsmEnd - (long) UIAsmStart);
	MakeCodeWriteable((long) DosModemRTNAsmStart, (long) DosModemRTNAsmEnd - (long) DosModemRTNAsmStart);
	MakeCodeWriteable((long) Vesa2AsmStart, (long) Vesa2AsmEnd - (long) Vesa2AsmStart);
	MakeCodeWriteable((long) InitVidAsmStart, (long) InitVidAsmEnd - (long) InitVidAsmStart);
	MakeCodeWriteable((long) SWAsmStart, (long) SWAsmEnd - (long) SWAsmStart);
	MakeCodeWriteable((long) GPProAsmStart, (long) GPProAsmEnd - (long) GPProAsmStart);
	MakeCodeWriteable((long) Vesa12AsmStart, (long) Vesa12AsmEnd - (long) Vesa12AsmStart);
	MakeCodeWriteable((long) MenuAsmStart, (long) MenuAsmEnd - (long) MenuAsmStart);
	MakeCodeWriteable((long) MakeV16BAsmStart, (long) MakeV16BAsmEnd - (long) MakeV16BAsmStart);
	MakeCodeWriteable((long) MakeV16TAsmStart, (long) MakeV16TAsmEnd - (long) MakeV16TAsmStart);
	MakeCodeWriteable((long) MakeVidAsmStart, (long) MakeVidAsmEnd - (long) MakeVidAsmStart);
	MakeCodeWriteable((long) Mode716AsmStart, (long) Mode716AsmEnd - (long) Mode716AsmStart);

	MakeCodeWriteable((long) Mode716BAsmStart, (long) Mode716BAsmEnd - (long) Mode716BAsmStart);
	MakeCodeWriteable((long) Mode716DAsmStart, (long) Mode716DAsmEnd - (long) Mode716DAsmStart);
	MakeCodeWriteable((long) Mode716EAsmStart, (long) Mode716EAsmEnd - (long) Mode716EAsmStart);
	MakeCodeWriteable((long) Mode716TAsmStart, (long) Mode716TAsmEnd - (long) Mode716TAsmStart);
	MakeCodeWriteable((long) Mode7AsmStart, (long) Mode7AsmEnd - (long) Mode7AsmStart);
	MakeCodeWriteable((long) Mode7ExtAsmStart, (long) Mode7ExtAsmEnd - (long) Mode7ExtAsmStart);
	MakeCodeWriteable((long) M716TExtAsmStart, (long) M716TExtAsmEnd - (long) M716TExtAsmStart);
	MakeCodeWriteable((long) TwoxSaiWAsmStart, (long) TwoxSaiWAsmEnd - (long) TwoxSaiWAsmStart);
	MakeCodeWriteable((long) EndMemAsmStart, (long) EndMemAsmEnd - (long) EndMemAsmStart);
	MakeCodeWriteable((long) MV16TMSAsmStart, (long) MV16TMSAsmEnd - (long) MV16TMSAsmStart);
	MakeCodeWriteable((long) NewG162AsmStart, (long) NewG162AsmEnd - (long) NewG162AsmStart);
	MakeCodeWriteable((long) NewGfx16AsmStart, (long) NewGfx16AsmEnd - (long) NewGfx16AsmStart);
	MakeCodeWriteable((long) NewGfx2AsmStart, (long) NewGfx2AsmEnd - (long) NewGfx2AsmStart);
	MakeCodeWriteable((long) NewGfxAsmStart, (long) NewGfxAsmEnd - (long) NewGfxAsmStart);
	MakeCodeWriteable((long) VCacheAsmStart, (long) VCacheAsmEnd - (long) VCacheAsmStart);
}
