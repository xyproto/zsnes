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

// C++ style code in C
#define bool unsigned char
#define true 1
#define false 0

extern unsigned int cycpbl, PH65816regsize;
extern unsigned int *wramdata, *vram, PHspcsave, PHdspsave, *C4Ram, *sfxramdata;
extern unsigned int PHnum2writesa1reg, SA1Mode, prevedi, SA1xpc, sa1dmaptr;
extern unsigned int soundcycleft, spc700read, timer2upd, xa, PHnum2writesfxreg;
extern unsigned int spcnumread, spchalted, opcd, HIRQCycNext, oamaddr;
extern unsigned int SfxR0, *setaramdata, ramsize, *sram;
extern unsigned int tempesi, tempedi, tempedx, tempebp;
extern unsigned int SPCMultA, PHnum2writespc7110reg, PHdspsave2;
extern unsigned char sndrot, spcRam[65472], DSPMem[256], SA1Status, *SA1RAMArea;
extern unsigned char DSP1Type, DSP1COp, prevoamptr, BRRBuffer[32], *romdata;
extern unsigned char curcyc, echoon0;
extern unsigned char vidmemch4[4096], vidmemch8[4096], vidmemch2[4096];

extern bool C4Enable, SFXEnable, SA1Enable, SPC7110Enable, SETAEnable, spcon;
extern bool SRAMState;

extern short C4WFXVal, C41FXVal, Op00Multiplicand, Op04Angle, Op08X, Op18X;
extern short Op28X, Op0CA, Op02FX, Op0AVS, Op06X, Op01m, Op0DX, Op03F, Op14Zr;
extern short Op0EH;
extern signed short Op10Coefficient;
