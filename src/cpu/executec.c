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




#ifdef __LINUX__
#include "gblhdr.h"
#define DIR_SLASH "/"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DIR_SLASH "\\"
#endif

#define memcpyinc(dest, src, size) memcpy(dest, src, size); dest += size;
#define memcpyrinc(src, dest, size) memcpy(dest, src, size); src += size;


/*Let's start converting stuff from execute.asm ^_^;
Big thanks to Nach and TRAC for helping me out on porting !!*/

extern unsigned int CBackupPos, PHnum2writecpureg, cycpbl;
extern unsigned int *wramdata, *vram, PHspcsave, *C4Ram, *sfxramdata;
extern unsigned int PHnum2writesa1reg, SA1Mode, prevedi, SA1xpc;
extern unsigned int sa1dmaptr, soundcycleft, spc700read, timer2upd, xa;
extern unsigned int spcnumread, spchalted, opcd, HIRQCycNext, oamaddr;
extern unsigned int ReadHead, *setaramdata, ramsize, *sram;
extern unsigned int tempesi, tempedi, tempedx, tempebp;

extern unsigned char *StateBackup, zsmesg[26], sndrot, spcon, spcRam[65472];
extern unsigned char DSPMem[256], C4Enable, SFXEnable, SA1Enable, SA1Status;
extern unsigned char *SA1RAMArea, DSP1Type, DSP1COp, prevoamptr, SETAEnable;

extern short C4WFXVal, C41FXVal, Op00Multiplicand, Op04Angle, Op08X, Op18X;
extern short Op28X, Op0CA, Op02FX, Op0AVS, Op06X, Op01m, Op0DX, Op03F, Op14Zr;
extern short Op0EH;
extern signed short Op10Coefficient;

void BackupCVFrame()
{
    unsigned char *curpos;

    curpos = StateBackup + (CBackupPos << 19) + 1024;
    memcpyinc (curpos, zsmesg, PHnum2writecpureg);
    memcpyinc (curpos, &cycpbl, 2*4);
    memcpyinc (curpos, &sndrot, 3019);
    memcpyinc (curpos, wramdata, 8192*16);
    memcpyinc (curpos, vram, 4096*16);

    if (spcon)
    {
	memcpyinc (curpos, spcRam, PHspcsave);
	memcpyinc (curpos, DSPMem, 16*16);
    }

    if (C4Enable)	{ memcpyinc (curpos, C4Ram, 2048*4); }

    if (SFXEnable)	{ memcpyinc (curpos, sfxramdata, 8192*16); }

    if (SA1Enable)
    {
	memcpyinc (curpos, &SA1Mode, PHnum2writesa1reg);
	memcpyinc (curpos, SA1RAMArea, 8192*16);
	memcpyinc (curpos, &SA1Status, 3);
	memcpyinc (curpos, &prevedi, 1*4);
	memcpyinc (curpos, &SA1xpc, 1*4);
	memcpyinc (curpos, &SA1RAMArea, 6*4);
	memcpyinc (curpos, &sa1dmaptr, 2*4);
    }

    if (DSP1Type)
    {
	memcpyinc (curpos, &DSP1COp, 70+128);
	memcpyinc (curpos, &C4WFXVal, 7*4+7*8+128);
	memcpyinc (curpos, &C41FXVal, 5*4+128);
	memcpyinc (curpos, &Op00Multiplicand, 3*4+128);
	memcpyinc (curpos, &Op10Coefficient, 4*4+128);
	memcpyinc (curpos, &Op04Angle, 4*4+128);
	memcpyinc (curpos, &Op08X, 5*4+128);
	memcpyinc (curpos, &Op18X, 5*4+128);
	memcpyinc (curpos, &Op28X, 4*4+128);
	memcpyinc (curpos, &Op0CA, 5*4+128);
	memcpyinc (curpos, &Op02FX, 11*4+3*4+28*8+128);
	memcpyinc (curpos, &Op0AVS, 5*4+14*8+128);
	memcpyinc (curpos, &Op06X, 6*4+10*8+4+128);
	memcpyinc (curpos, &Op01m, 4*4+128);
	memcpyinc (curpos, &Op0DX, 6*4+128);
	memcpyinc (curpos, &Op03F, 6*4+128);
	memcpyinc (curpos, &Op14Zr, 9*4+128);
	memcpyinc (curpos, &Op0EH, 4*4+128);
    }

    memcpyinc (curpos, &soundcycleft, 33);
    memcpyinc (curpos, &spc700read, 10*4);
    memcpyinc (curpos, &timer2upd, 1*4);
    memcpyinc (curpos, &xa, 14*4);
    memcpyinc (curpos, &spcnumread, 4);
    memcpyinc (curpos, &spchalted, 4);
    memcpyinc (curpos, &opcd, 6*4);
    memcpyinc (curpos, &HIRQCycNext, 5);
    memcpyinc (curpos, &oamaddr, 14*4);
    memcpyinc (curpos, &prevoamptr, 1);
    memcpyinc (curpos, &ReadHead, 1*4);

    if (SETAEnable)	{ memcpyinc (curpos, setaramdata, 256*16); }

    memcpyinc (curpos, sram, ramsize);
    memcpyinc (curpos, &tempesi, 4);
    memcpyinc (curpos, &tempedi, 4);
    memcpyinc (curpos, &tempedx, 4);
    memcpyinc (curpos, &tempebp, 4);
}

extern unsigned int Bank0datr8[256], Bank0datr16[256], Bank0datw8[256];
extern unsigned int Bank0datw16[256], xd, DPageR8, DPageR16, DPageW8;
extern unsigned int DPageW16;

void UpdateDPageC()
{
    DPageR8 = Bank0datr8[(xd >> 8) & 0xFF];
    DPageR16 = Bank0datr16[(xd >> 8) & 0xFF];
    DPageW8 = Bank0datw8[(xd >> 8) & 0xFF];
    DPageW16 = Bank0datw16[(xd >> 8) & 0xFF];
}

extern unsigned int SA1xd, SA1DPageR8, SA1DPageR16, SA1DPageW8, SA1DPageW16;

void SA1UpdateDPageC()
{
    SA1DPageR8 = Bank0datr8[(SA1xd >> 8) & 0xFF];
    SA1DPageR16 = Bank0datr16[(SA1xd >> 8) & 0xFF];
    SA1DPageW8 = Bank0datw8[(SA1xd >> 8) & 0xFF];
    SA1DPageW16 = Bank0datw16[(SA1xd >> 8) & 0xFF];
}

extern unsigned int PBackupPos;

void RestoreCVFrame()
{
    unsigned char *curpos;

    curpos = StateBackup + (PBackupPos << 19) + 1024;
    memcpyrinc (curpos, zsmesg, PHnum2writecpureg);
    memcpyrinc (curpos, &cycpbl, 2*4);
    memcpyrinc (curpos, &sndrot, 3019);
    memcpyrinc (curpos, wramdata, 8192*16);
    memcpyrinc (curpos, vram, 4096*16);

    if (spcon)
    {
	memcpyrinc (curpos, spcRam, PHspcsave);
	memcpyrinc (curpos, DSPMem, 16*16);
    }

    if (C4Enable)	{ memcpyrinc (curpos, C4Ram, 2048*4); }

    if (SFXEnable)	{ memcpyrinc (curpos, sfxramdata, 8192*16); }

    if (SA1Enable)
    {
	memcpyrinc (curpos, &SA1Mode, PHnum2writesa1reg);
	memcpyrinc (curpos, SA1RAMArea, 8192*16);
	memcpyrinc (curpos, &SA1Status, 3);
	memcpyrinc (curpos, &prevedi, 1*4);
	memcpyrinc (curpos, &SA1xpc, 1*4);
	memcpyrinc (curpos, &SA1RAMArea, 6*4);
	memcpyrinc (curpos, &sa1dmaptr, 2*4);
    }

    if (DSP1Type)
    {
	memcpyrinc (curpos, &DSP1COp, 70+128);
	memcpyrinc (curpos, &C4WFXVal, 7*4+7*8+128);
	memcpyrinc (curpos, &C41FXVal, 5*4+128);
	memcpyrinc (curpos, &Op00Multiplicand, 3*4+128);
	memcpyrinc (curpos, &Op10Coefficient, 4*4+128);
	memcpyrinc (curpos, &Op04Angle, 4*4+128);
	memcpyrinc (curpos, &Op08X, 5*4+128);
	memcpyrinc (curpos, &Op18X, 5*4+128);
	memcpyrinc (curpos, &Op28X, 4*4+128);
	memcpyrinc (curpos, &Op0CA, 5*4+128);
	memcpyrinc (curpos, &Op02FX, 11*4+3*4+28*8+128);
	memcpyrinc (curpos, &Op0AVS, 5*4+14*8+128);
	memcpyrinc (curpos, &Op06X, 6*4+10*8+4+128);
	memcpyrinc (curpos, &Op01m, 4*4+128);
	memcpyrinc (curpos, &Op0DX, 6*4+128);
	memcpyrinc (curpos, &Op03F, 6*4+128);
	memcpyrinc (curpos, &Op14Zr, 9*4+128);
	memcpyrinc (curpos, &Op0EH, 4*4+128);
    }

    memcpyrinc (curpos, &soundcycleft, 33);
    memcpyrinc (curpos, &spc700read, 10*4);
    memcpyrinc (curpos, &timer2upd, 1*4);
    memcpyrinc (curpos, &xa, 14*4);
    memcpyrinc (curpos, &spcnumread, 4);
    memcpyrinc (curpos, &spchalted, 4);
    memcpyrinc (curpos, &opcd, 6*4);
    memcpyrinc (curpos, &HIRQCycNext, 5);
    memcpyrinc (curpos, &oamaddr, 14*4);
    memcpyrinc (curpos, &prevoamptr, 1);
    memcpyrinc (curpos, &ReadHead, 1*4);

    if (SETAEnable)	{ memcpyrinc (curpos, setaramdata, 256*16); }

    memcpyrinc (curpos, sram, ramsize);
    memcpyrinc (curpos, &tempesi, 4);
    memcpyrinc (curpos, &tempedi, 4);
    memcpyrinc (curpos, &tempedx, 4);
    memcpyrinc (curpos, &tempebp, 4);

    UpdateDPageC();
    SA1UpdateDPageC();
}
