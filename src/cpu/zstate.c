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

#ifdef __MSDOS__
#define clim() __asm__ __volatile__ ("cli");
#define stim() __asm__ __volatile__ ("sti");
#else
#define clim()
#define stim()
#endif

#define memcpyinc(dest, src, size) memcpy(dest, src, size); dest += size;
#define memcpyrinc(src, dest, size) memcpy(dest, src, size); src += size;

/*Let's start converting stuff from execute.asm ^_^;
Big thanks to Nach, TRAC and anomie for helping me out on porting !!*/

extern unsigned int CBackupPos, PHnum2writecpureg, cycpbl;
extern unsigned int *wramdata, *vram, PHspcsave, PHdspsave, *C4Ram, *sfxramdata;
extern unsigned int PHnum2writesa1reg, SA1Mode, prevedi, SA1xpc, sa1dmaptr;
extern unsigned int soundcycleft, spc700read, timer2upd, xa, PHnum2writesfxreg;
extern unsigned int spcnumread, spchalted, opcd, HIRQCycNext, oamaddr;
extern unsigned int SfxR0, ReadHead, *setaramdata, ramsize, *sram;
extern unsigned int tempesi, tempedi, tempedx, tempebp;

extern unsigned char *StateBackup, zsmesg[26], sndrot, spcon, spcRam[65472];
extern unsigned char DSPMem[256], C4Enable, SFXEnable, SA1Enable, SA1Status;
extern unsigned char *SA1RAMArea, DSP1Type, DSP1COp, prevoamptr, SETAEnable;
extern unsigned char BRRBuffer[32];

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
	memcpyinc (curpos, BRRBuffer, PHdspsave);
	memcpyinc (curpos, DSPMem, 16*16);
    }

    if (C4Enable)	{ memcpyinc (curpos, C4Ram, 2048*4); }

    if (SFXEnable)
    {
	memcpyinc (curpos, sfxramdata, 8192*16);
	memcpyinc (curpos, &SfxR0, PHnum2writesfxreg);
    }

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
	memcpyrinc (curpos, BRRBuffer, PHdspsave);
	memcpyrinc (curpos, DSPMem, 16*16);
    }

    if (C4Enable)	{ memcpyrinc (curpos, C4Ram, 2048*4); }

    if (SFXEnable)
    {
	memcpyrinc (curpos, sfxramdata, 8192*16);
	memcpyrinc (curpos, &SfxR0, PHnum2writesfxreg);
    }

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

extern unsigned int spcBuffera;
extern unsigned int Voice0BufPtr, Voice1BufPtr, Voice2BufPtr, Voice3BufPtr;
extern unsigned int Voice4BufPtr, Voice5BufPtr, Voice6BufPtr, Voice7BufPtr;

void PrepareSaveState()
{
    Voice0BufPtr -= spcBuffera;
    Voice1BufPtr -= spcBuffera;
    Voice2BufPtr -= spcBuffera;
    Voice3BufPtr -= spcBuffera;
    Voice4BufPtr -= spcBuffera;
    Voice5BufPtr -= spcBuffera;
    Voice6BufPtr -= spcBuffera;
    Voice7BufPtr -= spcBuffera;
}

extern unsigned int xdb, xpb, xs, xx, xy;
extern unsigned short oamaddrt, xat, xst, xdt, xxt, xyt;
extern unsigned char xdbt, xpbt;

void unpackfunct()
{
    oamaddrt = (oamaddr & 0xFFFF);
    xat = (xa & 0xFFFF);
    xdbt = (xdb & 0xFF);
    xpbt = (xpb & 0xFF);
    xst = (xs & 0xFFFF);
    xdt = (xd & 0xFFFF);
    xxt = (xx & 0xFFFF);
    xyt = (xy & 0xFFFF);
}

#define byteset(byte, checkbit) (byte & (1 << checkbit)) ? 1 : 0

extern unsigned int GlobalVL, GlobalVR, EchoVL, EchoVR, EchoRate[16], MaxEcho;
extern unsigned int EchoFB, NoiseSpeeds[32], dspPAdj, NoiseInc, bg1ptrx;
extern unsigned int bg1ptry, bg2ptrx, bg2ptry, bg3ptrx, bg3ptry, bg4ptrx;
extern unsigned int bg4ptry;
extern signed int FIRTAPVal0, FIRTAPVal1, FIRTAPVal2, FIRTAPVal3, FIRTAPVal4;
extern signed int FIRTAPVal5, FIRTAPVal6, FIRTAPVal7;
extern unsigned short VolumeConvTable[32768], bg1ptr, bg1ptrb, bg1ptrc;
extern unsigned short bg2ptr, bg2ptrb, bg2ptrc, bg3ptr, bg3ptrb, bg3ptrc;
extern unsigned short bg4ptr, bg4ptrb, bg4ptrc;
extern unsigned char VolumeTableb[256], MusicVol, spcres, Voice0Status;
extern unsigned char Voice1Status, Voice2Status, Voice3Status, Voice4Status;
extern unsigned char Voice5Status, Voice6Status, Voice7Status, Voice0Noise;
extern unsigned char Voice1Noise, Voice2Noise, Voice3Noise, Voice4Noise;
extern unsigned char Voice5Noise, Voice6Noise, Voice7Noise, bgtilesz;
extern unsigned char BG116x16t, BG216x16t, BG316x16t, BG416x16t, vramincby8on;
extern unsigned char vramincr;

extern void (**regptw)();
extern void reg2118();
extern void reg2118inc();
extern void reg2118inc8();
extern void reg2118inc8inc();
extern void reg2119();
extern void reg2119inc();
extern void reg2119inc8();
extern void reg2119inc8inc();

void repackfunct()
{
    signed char val;
    unsigned char block;

    // Global/Echo Volumes
    GlobalVL = (VolumeConvTable[(MusicVol << 8) + VolumeTableb[DSPMem[0x0C]]] & 0xFF);
    GlobalVR = (VolumeConvTable[(MusicVol << 8) + VolumeTableb[DSPMem[0x1C]]] & 0xFF);
    EchoVL = (VolumeConvTable[(MusicVol << 8) + VolumeTableb[DSPMem[0x2C]]] & 0xFF);
    EchoVR = (VolumeConvTable[(MusicVol << 8) + VolumeTableb[DSPMem[0x3C]]] & 0xFF);

    // Echo Values
    MaxEcho = EchoRate[(DSPMem[0x7D] & 0xF)];
    EchoFB = VolumeTableb[DSPMem[0x0D]];

    // FIR Filter Values
    val = DSPMem[0x0F];
    FIRTAPVal0 = (signed int)val;
    val = DSPMem[0x1F];
    FIRTAPVal1 = (signed int)val;
    val = DSPMem[0x2F];
    FIRTAPVal2 = (signed int)val;
    val = DSPMem[0x3F];
    FIRTAPVal3 = (signed int)val;
    val = DSPMem[0x4F];
    FIRTAPVal4 = (signed int)val;
    val = DSPMem[0x5F];
    FIRTAPVal5 = (signed int)val;
    val = DSPMem[0x6F];
    FIRTAPVal6 = (signed int)val;
    val = DSPMem[0x7F];
    FIRTAPVal7 = (signed int)val;

    // Noise
    block = DSPMem[0x6C];
    DSPMem[0x6C] &= 0x7F;

    if (block && 0x80)	{ spcres++; }

    if (block && 0xC0)
    {
	Voice0Status = Voice1Status = Voice2Status = Voice3Status = 0;
	Voice4Status = Voice5Status = Voice6Status = Voice7Status = 0; 
    }

    NoiseInc = (((NoiseSpeeds[(block & 0x1F)] * dspPAdj) >> 17) & 0xFFFFFFFF);

    Voice0Noise = byteset (DSPMem[0x3D], 0);
    Voice1Noise = byteset (DSPMem[0x3D], 1);
    Voice2Noise = byteset (DSPMem[0x3D], 2);
    Voice3Noise = byteset (DSPMem[0x3D], 3);
    Voice4Noise = byteset (DSPMem[0x3D], 4);
    Voice5Noise = byteset (DSPMem[0x3D], 5);
    Voice6Noise = byteset (DSPMem[0x3D], 6);
    Voice7Noise = byteset (DSPMem[0x3D], 7);

    bg1ptrx = bg1ptrb - bg1ptr;
    bg1ptry = bg1ptrc - bg1ptr;
    bg2ptrx = bg2ptrb - bg2ptr;
    bg2ptry = bg2ptrc - bg2ptr;
    bg3ptrx = bg3ptrb - bg3ptr;
    bg3ptry = bg3ptrc - bg3ptr;
    bg4ptrx = bg4ptrb - bg4ptr;
    bg4ptry = bg4ptrc - bg4ptr;

    // 16x16 tiles
    BG116x16t = byteset (bgtilesz, 0);
    BG216x16t = byteset (bgtilesz, 1);
    BG316x16t = byteset (bgtilesz, 2);
    BG416x16t = byteset (bgtilesz, 3);

    oamaddr = oamaddrt;
    xa = xat;
    xdb = xdbt;
    xpb = xpbt;
    xs = xst;
    xd = xdt;
    xx = xxt;
    xy = xyt;

    if (vramincby8on == 1)
    {
	if (vramincr == 1)
	{
	    regptw[0x2118] = reg2118inc8inc;
	    regptw[0x2119] = reg2119inc8;
	}
	else
	{
	    regptw[0x2118] = reg2118inc8;
	    regptw[0x2119] = reg2119inc8inc;
	}
    }
    else
    {
	if (vramincr == 1)
	{
	    regptw[0x2118] = reg2118inc;
	    regptw[0x2119] = reg2119;
	}
	else
	{
	    regptw[0x2118] = reg2118;
	    regptw[0x2119] = reg2119inc;
	}
    }
}

extern unsigned int SA1Stat;
extern unsigned char IRAM[2049], *SA1Ptr, *SA1RegPCS, *CurBWPtr, *SA1BWPtr;
extern unsigned char *SNSBWPtr, *romdata;

void SaveSA1()
{
    unsigned int offst=(unsigned int)SA1RegPCS;

    SA1Stat &= 0xFFFFFF00;
    SA1Ptr -= offst;

    if (SA1RegPCS == IRAM)
    {
	SA1Stat = (SA1Stat & 0xFFFFFF00) + 1;
    }

    if (SA1RegPCS == IRAM-0x3000)
    {
	SA1Stat = (SA1Stat & 0xFFFFFF00) + 2;
    }

    offst = (unsigned int)romdata;
    SA1RegPCS -= offst;
    CurBWPtr -= offst;
    SA1BWPtr -= offst;
    SNSBWPtr -= offst;
}

void RestoreSA1()
{
    unsigned int offst=(unsigned int)romdata;

    SA1RegPCS += offst;
    CurBWPtr += offst;
    SA1BWPtr += offst;
    SNSBWPtr += offst;

    if ((SA1Stat & 0xFF) == 1)
    {
	SA1RegPCS = IRAM;
    }

    if ((SA1Stat & 0xFF) == 2)
    {
	SA1RegPCS = IRAM-0x3000;
    }

    offst = (unsigned int)SA1RegPCS;
    SA1Ptr += offst;
    SA1RAMArea = romdata + 4096*1024;
}

#define ResState(Voice_BufPtr) \
    Voice_BufPtr += spcBuffera; \
    if (Voice_BufPtr >= spcBuffera + 65536*4) \
    { \
	Voice_BufPtr = spcBuffera; \
    }

void ResetState()
{
    ResState(Voice0BufPtr);
    ResState(Voice1BufPtr);
    ResState(Voice2BufPtr);
    ResState(Voice3BufPtr);
    ResState(Voice4BufPtr);
    ResState(Voice5BufPtr);
    ResState(Voice6BufPtr);
    ResState(Voice7BufPtr);
}

extern unsigned int Curtableaddr, tableA[256], spcPCRam, spcRamDP;
extern unsigned int statefileloc, CurrentHandle, SfxRomBuffer;
extern unsigned int SfxCROM, SfxLastRamAdr, SfxRAMMem;
extern unsigned int SPCMultA, PHnum2writespc7110reg;
extern unsigned int MsgCount, MessageOn;

extern unsigned char AutoIncSaveSlot, firstsaveinc, fnamest[512];
extern unsigned char SPC7110Enable, cbitmode, NoPictureSave, txtsavemsg[14];
extern unsigned char *Msgptr, txtsavemsgfail[15];

extern unsigned short PrevPicture[64*56];

FILE *fhandle;
extern void SRAMChdir();
extern void CapturePicture();

void statesaver()
{
    unsigned int offst;

    clim();

    offst = (unsigned int)tableA;
    Curtableaddr -= offst;
    offst = (unsigned int)spcRam;
    spcPCRam -= offst;
    spcRamDP -= offst;

    PrepareSaveState();
    unpackfunct();

// 'Auto increment savestate slot' code

    if (AutoIncSaveSlot)
    {
	if (firstsaveinc)
	{
	    firstsaveinc = 0;
	}
	else
	{
	    switch (fnamest[statefileloc])
	    {
	      case 't':
		fnamest[statefileloc] = '1';
		break;
	      case '9':
		fnamest[statefileloc] = 't';
		break;
	      default:
		fnamest[statefileloc]++;
	    }
	}
    }

// Save State code

    #ifdef __LINUX__
    SRAMChdir();
    #endif

    if ((fhandle = fopen(fnamest+1,"wb")) != NULL)
    {
	// Save 65816 status, etc.

	fwrite (zsmesg, 1, PHnum2writecpureg, fhandle);
	fwrite (&cycpbl, 1, 2*4, fhandle);
	fwrite (&sndrot, 1, 3019, fhandle);	// Save SNES PPU Register status
	fwrite (wramdata, 1, 8192*16, fhandle);
	fwrite (vram, 1, 4096*16, fhandle);

	if (spcon)	// SPC stuff, DSP stuff
	{
	    fwrite (spcRam, 1, PHspcsave, fhandle);
	    fwrite (BRRBuffer, 1, PHdspsave, fhandle);
	    fwrite (DSPMem, 1, 16*16, fhandle);
	}

	if (C4Enable)	{ fwrite (C4Ram, 1, 2048*4, fhandle); }

	if (SFXEnable)
	{
	    SfxRomBuffer -= SfxCROM;
	    SfxLastRamAdr -= SfxRAMMem;
	    fwrite (sfxramdata, 1, 8192*16, fhandle);
	    fwrite (&SfxR0, 1, PHnum2writesfxreg, fhandle);
	    SfxRomBuffer += SfxCROM;
	    SfxLastRamAdr += SfxRAMMem;
	}

	if (SETAEnable)	{ fwrite (setaramdata, 1, 256*16, fhandle); }
	// TODO: save the SetaCmdEnable?  For completeness we should do it
	// but currently we ignore it anyway.

	if (SPC7110Enable)
	{
	    fwrite (romdata+0x510000, 1, 65536, fhandle);
	    fwrite (&SPCMultA, 1, PHnum2writespc7110reg, fhandle);
	}

	if (SA1Enable)
	{
	    SaveSA1();// Convert SA-1 stuff to standard, non displacement format

	    fwrite (&SA1Mode, 1, PHnum2writesa1reg, fhandle);
	    fwrite (SA1RAMArea, 1, 8192*16, fhandle);

	    RestoreSA1();	// Convert back SA-1 stuff
	}

	if (cbitmode && !NoPictureSave)
	{
	    CapturePicture();

	    fwrite (PrevPicture, 1, 64*56*2, fhandle);
	}

	fclose (fhandle);

	if (fnamest[statefileloc] == 't')
	{
	    txtsavemsg[6]='0';
	}
	else
	{
	    txtsavemsg[6]=fnamest[statefileloc];
	}

	Msgptr = txtsavemsg;
	MessageOn = MsgCount;
    }
    else
    {
	Msgptr = txtsavemsgfail;
	MessageOn = MsgCount;
    }

    offst = (unsigned int)tableA;
    Curtableaddr += offst;
    offst = (unsigned int)spcRam;
    spcPCRam += offst;
    spcRamDP += offst;

    ResetState();
    stim();
}

extern unsigned int snesmmap[256], snesmap2[256];
/*extern unsigned int NumofBanks;
extern unsigned char SA1BankVal[4];

void BankSwitchC (unsigned char bank, unsigned int offset1, unsigned int offset2, unsigned int pointer)
{
    unsigned int curbankval=SA1BankVal[bank], membankval, i;

    if ((NumofBanks & 0xFF) == 64)	{ curbankval &= 1 ; }

    curbankval &= 7;
    curbankval <<= 20;

    if (SA1BankVal[bank] & 0x80)
    {
	membankval = (pointer + (unsigned int)romdata - 0x8000);
    }
    else
    {
	membankval = (curbankval + (unsigned int)romdata - 0x8000);
    }

    for (i=0 ; i<32 ; i++)
    {
	snesmmap[offset1+i] = membankval;
	membankval += 0x8000;
    }

    membankval = curbankval + (unsigned int)romdata;

    for (i=0 ; i<16 ; i++)
    {
	snesmap2[offset2+i] = membankval;
	snesmmap[offset2+i] = membankval;
	membankval += 0x10000;
    }
}

extern unsigned int SA1BankSw;

void UpdateBanks()
{
    if ((SA1BankSw & 0xFF) == 1)
    {
	BankSwitchC (0, 0x000, 0x0C0, 0x000000) ;
	BankSwitchC (1, 0x020, 0x0D0, 0x100000) ;
	BankSwitchC (2, 0x080, 0x0E0, 0x200000) ;
	BankSwitchC (3, 0x0A0, 0x0F0, 0x300000) ;
    }
}*/

void BankSwitchSDD1C (unsigned char bankval, unsigned int offset)
{
    unsigned int curbankval = bankval, i;

    curbankval &= 7;
    curbankval <<= 20;
    curbankval += (unsigned int)romdata;

    for (i=0 ; i<16 ; i++)
    {
	snesmap2[offset+i] = curbankval;
	snesmmap[offset+i] = curbankval;
	curbankval += 0x10000;
    }
}

extern unsigned char SDD1BankA, SDD1BankB, SDD1BankC, SDD1BankD;

void UpdateBanksSDD1()
{
    if (SDD1BankA)
    {
	BankSwitchSDD1C (SDD1BankA, 0x0C0);
	BankSwitchSDD1C (SDD1BankB, 0x0D0);
	BankSwitchSDD1C (SDD1BankC, 0x0E0);
	BankSwitchSDD1C (SDD1BankD, 0x0F0);
    }
}

extern unsigned int Voice0Freq, Voice1Freq, Voice2Freq, Voice3Freq;
extern unsigned int Voice4Freq, Voice5Freq, Voice6Freq, Voice7Freq;
extern unsigned short Voice0Pitch, Voice1Pitch, Voice2Pitch, Voice3Pitch;
extern unsigned short Voice4Pitch, Voice5Pitch, Voice6Pitch, Voice7Pitch;

void initpitch()
{
	Voice0Pitch = DSPMem[2+0*0x10];
	Voice0Freq = ((((Voice0Pitch & 0x3FFF) * dspPAdj) >> 8) & 0xFFFFFFFF);
	Voice1Pitch = DSPMem[2+1*0x10];
	Voice1Freq = ((((Voice1Pitch & 0x3FFF) * dspPAdj) >> 8) & 0xFFFFFFFF);
	Voice2Pitch = DSPMem[2+2*0x10];
	Voice2Freq = ((((Voice2Pitch & 0x3FFF) * dspPAdj) >> 8) & 0xFFFFFFFF);
	Voice3Pitch = DSPMem[2+3*0x10];
	Voice3Freq = ((((Voice3Pitch & 0x3FFF) * dspPAdj) >> 8) & 0xFFFFFFFF);
	Voice4Pitch = DSPMem[2+4*0x10];
	Voice4Freq = ((((Voice4Pitch & 0x3FFF) * dspPAdj) >> 8) & 0xFFFFFFFF);
	Voice5Pitch = DSPMem[2+5*0x10];
	Voice5Freq = ((((Voice5Pitch & 0x3FFF) * dspPAdj) >> 8) & 0xFFFFFFFF);
	Voice6Pitch = DSPMem[2+6*0x10];
	Voice6Freq = ((((Voice6Pitch & 0x3FFF) * dspPAdj) >> 8) & 0xFFFFFFFF);
	Voice7Pitch = DSPMem[2+7*0x10];
	Voice7Freq = ((((Voice7Pitch & 0x3FFF) * dspPAdj) >> 8) & 0xFFFFFFFF);
}

extern unsigned int KeyLoadState, Totalbyteloaded, SfxMemTable[256], SfxCPB;
extern unsigned int SfxPBR, SfxROMBR, SfxRAMBR;
extern unsigned char pressed[256+128+64], multchange, txtloadmsg[15];
extern unsigned char txtconvmsg[16], txtnfndmsg[23], vidmemch2[4096];
extern unsigned char vidmemch4[4096], vidmemch8[4096], versn, MovieProcessing;
extern unsigned char ioportval, SDD1Enable, nexthdma;

extern void procexecloop();

void stateloader (unsigned char *statename, unsigned char keycheck, unsigned char xfercheck)
{
    unsigned int offst;
    unsigned char statevalue;

    if (keycheck)
    {
	pressed[1] = 0;
	pressed[KeyLoadState] = 2;
	multchange = 1;

	clim();
	#ifdef __LINUX__
	SRAMChdir();
	#endif

	// Get the state number
	if (fnamest[statefileloc] == 't')
	{
	    statevalue = '0';
	}
	else
	{
	    statevalue = fnamest[statefileloc];
	}

	txtloadmsg[6] = statevalue;
	txtconvmsg[6] = statevalue;
	txtnfndmsg[21] = statevalue;
    }

    // Actual state loading code
    if ((fhandle = fopen(statename,"rb")) != NULL)
    {
	if (xfercheck)	{ Totalbyteloaded = 0; }

	MovieProcessing = 0;
	prevoamptr = 0xFF;

	// Load 65816 status, etc.
	Totalbyteloaded += fread (zsmesg, 1, PHnum2writecpureg, fhandle);
	if (zsmesg[25] >= '6')	// just drop older states
	{
	    // Load SPC timers
	    Totalbyteloaded += fread (&cycpbl, 1, 2*4, fhandle);
	    // Load SNES PPU Register status
	    Totalbyteloaded += fread (&sndrot, 1, 3019, fhandle);
	    ioportval = 0xFF;
	    // Load WRAM (128k), VRAM (64k)
	    Totalbyteloaded += fread (wramdata, 1, 8192*16, fhandle);
	    Totalbyteloaded += fread (vram, 1, 4096*16, fhandle);

	    if (spcon)	// SPC stuff, DSP stuff
	    {
		Totalbyteloaded += fread (spcRam, 1, PHspcsave, fhandle);
		Totalbyteloaded += fread (BRRBuffer, 1, PHdspsave, fhandle);
		Totalbyteloaded += fread (DSPMem, 1, 16*16, fhandle);
	    }

	    if (C4Enable) { Totalbyteloaded += fread (C4Ram, 1, 2048*4, fhandle); }

	    if (SFXEnable)
	    {
		Totalbyteloaded += fread (sfxramdata, 1, 8192*16, fhandle);
		Totalbyteloaded += fread (&SfxR0, 1, PHnum2writesfxreg, fhandle);
		SfxCPB = SfxMemTable[(SfxPBR & 0xFF)];
		SfxCROM = SfxMemTable[(SfxROMBR & 0xFF)];
		SfxRAMMem = (unsigned int)sfxramdata + ((SfxRAMBR & 0xFF) << 16);
		SfxRomBuffer += SfxCROM;
		SfxLastRamAdr += SfxRAMMem;
	    }

	    if (SETAEnable) { Totalbyteloaded += fread (setaramdata, 1, 256*16, fhandle); }
	// TODO: load the SetaCmdEnable?  For completeness we should do it
	// but currently we ignore it anyway.

	    if (SPC7110Enable)
	    {
		Totalbyteloaded += fread (romdata+0x510000, 1, 65536, fhandle);
		Totalbyteloaded += fread (&SPCMultA, 1, PHnum2writespc7110reg, fhandle);
	    }

	    if (SA1Enable)
	    {
		Totalbyteloaded += fread (&SA1Mode, 1, PHnum2writesa1reg, fhandle);
		Totalbyteloaded += fread (SA1RAMArea, 1, 8192*16, fhandle);

		RestoreSA1();	// Convert back SA-1 stuff
//		UpdateBanks();
// It was in the asm, but the only thing it does is break Oshaberi Parodius...
// The C port is still present, just commented out.
		SA1UpdateDPageC();
	    }

	    if (SDD1Enable)
	    {
		UpdateBanksSDD1();
	    }

	    fclose (fhandle);
	    repackfunct();

	    spcnumread = 0;
	    spchalted = 0xFFFFFFFF;
	    nexthdma = 0;

//	    ;call headerhack	; ASM ONLY ! I'm not porting hacks ever.
	    initpitch();
	    
	    // Clear cache check if state loaded
	    memset (vidmemch2, 1, 4096);
	    memset (vidmemch4, 1, 4096);
	    memset (vidmemch8, 1, 4096);

	    offst = (unsigned int)tableA;
	    Curtableaddr += offst;
	    offst = (unsigned int)spcRam;
	    spcPCRam += offst;
	    spcRamDP += offst;

	    ResetState();
	    procexecloop();

	    if (keycheck)	{ stim(); }
	}

	if (keycheck)
	{
	    if (!xfercheck)
	    {
		if (zsmesg[25] < '6')	{ Msgptr = txtconvmsg; }
	    	else	{ Msgptr = txtloadmsg; }
	    }

	MessageOn = MsgCount;
	}	
    }
    else
    {
	if (keycheck)
	{
	    Msgptr = txtnfndmsg;
	    MessageOn = MsgCount;
	    stim();
	}
    }
}

void debugloadstate()
{
    stateloader (fnamest+1, 0, 0);
}

void loadstate()
{
    stateloader (fnamest+1, 1, 0);
}

void loadstate2()
{
    stateloader (fnamest+1, 0, 1);
}

extern unsigned char Netfname[11];

void loadstate3()
{
    stateloader (Netfname, 0, 1);
}
