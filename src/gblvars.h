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

#include <stdbool.h>
#ifdef __MSVC__
#include "win/vc_head/stdint.h"
#else
#include <stdint.h>
#endif

extern uint32_t cycpbl;
extern uint32_t PHspcsave, PHdspsave;
extern uint32_t PHnum2writesa1reg, SA1Mode, prevedi, SA1xpc, sa1dmaptr;
extern uint32_t soundcycleft, spc700read, timer2upd, xa, PHnum2writesfxreg;
extern uint32_t opcd, HIRQCycNext, oamaddr, curexecstate, nmiprevaddrl;
extern uint32_t nmirept, nmiprevline, nmistatus, joycontren;
extern uint32_t SfxR0, *setaramdata, ramsize, nmiprevaddrh;
extern uint32_t tempesi, tempedi, tempedx, tempebp;
extern uint32_t SPCMultA, PHnum2writespc7110reg, PHdspsave2;

extern uintptr_t *wramdata, *vram, *C4Ram, *romdata, *setaramdata, *sram, *sfxramdata, *SA1RAMArea;

extern unsigned char sndrot, SPCRAM[65472], DSPMem[256], SA1Status;
extern unsigned char DSP1Enable, DSP1COp, prevoamptr, BRRBuffer[32];
extern unsigned char curcyc, echoon0, spcnumread, NextLineCache, HIRQNextExe;
extern unsigned char vidmemch4[4096], vidmemch8[4096], vidmemch2[4096];

extern bool C4Enable, SFXEnable, SA1Enable, SPC7110Enable, SETAEnable, DSP4Enable, spcon;

extern int16_t C4WFXVal, C41FXVal, Op00Multiplicand, Op04Angle, Op08X, Op18X;
extern int16_t Op28X, Op0CA, Op02FX, Op0AVS, Op06X, Op01m, Op0DX, Op03F, Op14Zr;
extern int16_t Op0EH, Op10Coefficient;
