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

extern unsigned int SA1Stat;
extern unsigned char IRAM[2049], *SA1Ptr, *SA1RegPCS, *CurBWPtr, *SA1BWPtr;
extern unsigned char *SNSBWPtr, *romdata;

void SaveSA1()
{
    unsigned int off1=(unsigned int)SA1RegPCS, off2=(unsigned int)romdata;

    SA1Stat &= 0xFFFFFF00;
    SA1Ptr -= off1;

    if (SA1RegPCS == IRAM)
    {
	SA1Stat = (SA1Stat & 0xFFFFFF00) + 1;
    }

    if (SA1RegPCS == IRAM-0x3000)
    {
	SA1Stat = (SA1Stat & 0xFFFFFF00) + 2;
    }

    SA1RegPCS -= off2;
    CurBWPtr -= off2;
    SA1BWPtr -= off2;
    SNSBWPtr -= off2;
}

extern unsigned int NumofBanks, snesmmap[256], snesmap2[256];
extern unsigned char SA1BankVal[4];

void BankSwitchC (unsigned char bank, unsigned short offset1, unsigned int offset2, unsigned int pointer)
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
}

void RestoreSA1()
{
    unsigned int off1, off2=(unsigned int)romdata;

    SA1RegPCS += off2;
    CurBWPtr += off2;
    SA1BWPtr += off2;
    SNSBWPtr += off2;

    if ((SA1Stat & 0xFF) == 1)
    {
	SA1RegPCS = IRAM;
    }

    if ((SA1Stat & 0xFF) == 2)
    {
	SA1RegPCS = IRAM-0x3000;
    }

    off1 = (unsigned int)SA1RegPCS;
    SA1Ptr += off1;
    SA1RAMArea = romdata + 4096*1024;

    UpdateBanks();
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
extern unsigned int statefileloc, CurrentHandle, PHdspsave, SfxRomBuffer;
extern unsigned int SfxCROM, SfxLastRamAdr, SfxRAMMem, PHnum2writesfxreg;
extern unsigned int *SfxR0, *SPCMultA, PHnum2writespc7110reg;
extern unsigned int MsgCount, MessageOn;

extern unsigned char AutoIncSaveSlot, firstsaveinc, fnamest[512], BRRBuffer[32];
extern unsigned char SPC7110Enable, cbitmode, NoPictureSave, txtsavemsg[15];
extern unsigned char *Msgptr, txtsavemsgfail[16];

extern unsigned short PrevPicture[64*56];

FILE *fhandle;
void SRAMChdir();
void CapturePicture();

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
    SRAMChdir() ;
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
	    fwrite (SfxR0, 1, PHnum2writesfxreg, fhandle);
	    SfxRomBuffer += SfxCROM;
	    SfxLastRamAdr += SfxRAMMem;
	}

	if (SETAEnable)	{ fwrite (setaramdata, 1, 256*16, fhandle); }

	if (SPC7110Enable)
	{
	    fwrite (romdata+0x510000, 1, 65536, fhandle);
	    fwrite (SPCMultA, 1, PHnum2writespc7110reg, fhandle);
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
