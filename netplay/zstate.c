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
#include "gblvars.h"
#include "asm_call.h"

#ifdef __MSDOS__
#define clim() __asm__ __volatile__ ("cli");
#define stim() __asm__ __volatile__ ("sti");
#else
#define clim()
#define stim()
#endif

void SA1UpdateDPageC(), unpackfunct(), repackfunct();
void PrepareOffset(), ResetOffset(), initpitch(), UpdateBanksSDD1();
void procexecloop(), outofmemory();

static void copy_snes_data(unsigned char **buffer, void (*copy_func)(unsigned char **, void *, size_t))
{
  //65816 status, etc.
  copy_func(buffer, &curcyc, PH65816regsize);
  //SPC Timers
  copy_func(buffer, &cycpbl, 2*4);
  //SNES PPU Register status
  copy_func(buffer, &sndrot, 3019);
}

static void copy_spc_data(unsigned char **buffer, void (*copy_func)(unsigned char **, void *, size_t))
{
  //SPC stuff, DSP stuff
  copy_func(buffer, spcRam, PHspcsave);
  copy_func(buffer, &BRRBuffer, PHdspsave);
  copy_func(buffer, &DSPMem, sizeof(DSPMem));
}

static void copy_extra_data(unsigned char **buffer, void (*copy_func)(unsigned char **, void *, size_t))
{
  copy_func(buffer, &soundcycleft, 33);
  copy_func(buffer, &spc700read, 10*4);
  copy_func(buffer, &timer2upd, 1*4);
  copy_func(buffer, &xa, 14*4);
  copy_func(buffer, &spcnumread, 4);
  copy_func(buffer, &spchalted, 4);
  copy_func(buffer, &opcd, 6*4);
  copy_func(buffer, &HIRQCycNext, 5);
  copy_func(buffer, &oamaddr, 14*4);
  copy_func(buffer, &prevoamptr, 1);
}

static size_t load_save_size;
static unsigned int zst_version;

//For compatibility with old save states (pre v1.43)
#define loading_old_state (!buffer && read && (zst_version < 143))
#define loading_state_no_sram (!buffer && read && !SRAMState)

static void copy_state_data(unsigned char *buffer, void (*copy_func)(unsigned char **, void *, size_t), bool read)
{
  copy_snes_data(&buffer, copy_func);

  //WRAM (128k), VRAM (64k)
  copy_func(&buffer, wramdata, 8192*16);
  copy_func(&buffer, vram, 4096*16);

  if (spcon)
  {
    copy_spc_data(&buffer, copy_func);
    if (buffer) //Rewind stuff
    {
      copy_func(&buffer, &echoon0, PHdspsave2);
    }
  }

  if (C4Enable)
  {
    copy_func(&buffer, C4Ram, 2048*4);
  }

  if (SFXEnable)
  {
    copy_func(&buffer, sfxramdata, 8192*16);
    copy_func(&buffer, &SfxR0, PHnum2writesfxreg);
  }

  if (SA1Enable)
  {
    copy_func(&buffer, &SA1Mode, PHnum2writesa1reg);
    copy_func(&buffer, SA1RAMArea, 8192*16);
    if (!loading_old_state)
    {
      copy_func(&buffer, &SA1Status, 3);
      copy_func(&buffer, &SA1xpc, 1*4);
      copy_func(&buffer, &sa1dmaptr, 2*4);
    }
  }

  if (DSP1Type && !loading_old_state)
  {
    copy_func(&buffer, &DSP1COp, 70+128);
    copy_func(&buffer, &Op00Multiplicand, 3*4+128);
    copy_func(&buffer, &Op10Coefficient, 4*4+128);
    copy_func(&buffer, &Op04Angle, 4*4+128);
    copy_func(&buffer, &Op08X, 5*4+128);
    copy_func(&buffer, &Op18X, 5*4+128);
    copy_func(&buffer, &Op28X, 4*4+128);
    copy_func(&buffer, &Op0CA, 5*4+128);
    copy_func(&buffer, &Op02FX, 11*4+3*4+28*8+128);
    copy_func(&buffer, &Op0AVS, 5*4+14*8+128);
    copy_func(&buffer, &Op06X, 6*4+10*8+4+128);
    copy_func(&buffer, &Op01m, 4*4+128);
    copy_func(&buffer, &Op0DX, 6*4+128);
    copy_func(&buffer, &Op03F, 6*4+128);
    copy_func(&buffer, &Op14Zr, 9*4+128);
    copy_func(&buffer, &Op0EH, 4*4+128);
  }

  if (SETAEnable)
  {
    copy_func(&buffer, setaramdata, 256*16);

    //Todo: copy the SetaCmdEnable?  For completeness we should do it
    //but currently we ignore it anyway.
  }

  if (SPC7110Enable)
  {
    copy_func(&buffer, romdata+0x510000, 65536);
    copy_func(&buffer, &SPCMultA, PHnum2writespc7110reg);
  }

  if (!loading_old_state)
  {
    copy_extra_data(&buffer, copy_func);

    if (!loading_state_no_sram)
    {
      copy_func(&buffer, sram, ramsize);
    }

    if (buffer) //Not to a file, i.e. rewind
    {
      copy_func(&buffer, &tempesi, 4);
      copy_func(&buffer, &tempedi, 4);
      copy_func(&buffer, &tempedx, 4);
      copy_func(&buffer, &tempebp, 4);
    }
  }
}

static void memcpyinc(unsigned char **dest, void *src, size_t len)
{
  memcpy(*dest, src, len);
  *dest += len;
}

static void memcpyrinc(unsigned char **src, void *dest, size_t len)
{
  memcpy(dest, *src, len);
  *src += len;
}

extern unsigned int RewindTimer;
extern unsigned char RewindStates;
unsigned char *StateBackup = 0;
size_t rewind_state_size, cur_zst_size, old_zst_size;

/* A nice idea, but needs more ported from the assembly first.
size_t RewindPos, RewindEarliestPos;

void BackupCVFrame()
{
  unsigned char *RewindBufferPos = StateBackup + RewindPos*rewind_state_size;
  printf("Backing up rewind in slot #%u\n", RewindPos);
  copy_state_data(RewindBufferPos, memcpyinc, false);
  RewindPos++;
  if (RewindPos == RewindStates)
  {
    RewindPos = 0;
  }
  if (RewindPos == RewindEarliestPos)
  {
    RewindEarliestPos++;
    if (RewindEarliestPos == RewindStates)
    {
      RewindEarliestPos = 0;
    }
  }
  RewindTimer = 60*3;
}

void RestoreCVFrame()
{
  if (RewindPos != RewindEarliestPos)
  {
    unsigned char *RewindBufferPos;

    if (!RewindPos)
    {
      RewindPos = RewindStates;
    }
    RewindPos--;

    RewindBufferPos = StateBackup + RewindPos*rewind_state_size;
    printf("Restoring rewind in slot #%u\n", RewindPos);
    copy_state_data(RewindBufferPos, memcpyrinc, true);

    //Clear Cache Check
    memset(vidmemch2, 1, sizeof(vidmemch2));
    memset(vidmemch4, 1, sizeof(vidmemch4));
    memset(vidmemch8, 1, sizeof(vidmemch8));

    RewindTimer = 60*3;
  }
}

void MultipleFrameBack(unsigned int i)
{
  while (i--)
  {
    if (RewindPos != RewindEarliestPos)
    {
      if (!RewindPos)
      {
        RewindPos = RewindStates;
      }
      RewindPos--;
    }
    else
    {
      break;
    }
  }
}

void SetupRewindBuffer()
{
  if (StateBackup){ free(StateBackup); }
  for (; RewindStates; RewindStates--)
  {
    StateBackup = 0;
    StateBackup = (unsigned char *)malloc(rewind_state_size*RewindStates);
    if (StateBackup) { break; }
  }
}
*/

extern unsigned int CBackupPos, PBackupPos, RewindPos, RewindOldPos;
extern unsigned char RewindFrames, romispal;
extern unsigned char MovieProcessing;
void zmv_rewind_save(size_t, bool);
void zmv_rewind_load(size_t, bool);

#define ActualRewindFrames (RewindFrames * (romispal ? 25 : 30))

void BackupCVFrame()
{
  unsigned char *RewindBufferPos = StateBackup + CBackupPos*rewind_state_size;
  //printf("Backing up rewind in slot #%u\n", CBackupPos);
  copy_state_data(RewindBufferPos, memcpyinc, false);
  if (MovieProcessing == 1) { zmv_rewind_save(CBackupPos, true); }
  else if (MovieProcessing == 2) { zmv_rewind_save(CBackupPos, false); }
  RewindTimer = ActualRewindFrames;
}

void RestoreCVFrame()
{
  unsigned char *RewindBufferPos = StateBackup + PBackupPos*rewind_state_size;
  //printf("Restoring rewind in slot #%u\n", PBackupPos);
  copy_state_data(RewindBufferPos, memcpyrinc, true);
  if (MovieProcessing == 1) { zmv_rewind_load(PBackupPos, true); }
  else if (MovieProcessing == 2) { zmv_rewind_load(PBackupPos, false); }
  RewindTimer = ActualRewindFrames;
}

void SetupRewindBuffer()
{
  if (StateBackup){ free(StateBackup); }
  RewindStates = 16;
  StateBackup = 0;
  StateBackup = (unsigned char *)malloc(rewind_state_size*RewindStates);
  if (!StateBackup)
  {
    asm_call(outofmemory);
  }
}

static size_t state_size;

static void state_size_tally(unsigned char **dest, void *src, size_t len)
{
  state_size += len;
}

void InitRewindVars()
{
#ifndef __MSDOS__  //When all the code is ported to C, we can make this work with DOS too
  unsigned char almost_useless_array[1]; //An array is needed for copy_state_data to give the correct size
  state_size = 0;
  copy_state_data(almost_useless_array, state_size_tally, false);
  rewind_state_size = state_size;

  SetupRewindBuffer();
  RewindPos = 0;
  RewindOldPos = 0;
  //RewindEarliestPos = 0;
  RewindTimer = ActualRewindFrames;
#endif
}

//This is used to preserve system load state between loads
static unsigned char BackupSystemBuffer[0x800000]; //Half a megabyte, should be enough for a while

void BackupSystemVars()
{
  unsigned char *buffer = BackupSystemBuffer;
  copy_snes_data(&buffer, memcpyinc);
  copy_spc_data(&buffer, memcpyinc);
  copy_extra_data(&buffer, memcpyinc);
}

void RestoreSystemVars()
{
  unsigned char *buffer = BackupSystemBuffer;
  InitRewindVars();
  copy_snes_data(&buffer, memcpyrinc);
  copy_spc_data(&buffer, memcpyrinc);
  copy_extra_data(&buffer, memcpyrinc);
}

extern unsigned int spcBuffera;
extern unsigned int Voice0BufPtr, Voice1BufPtr, Voice2BufPtr, Voice3BufPtr;
extern unsigned int Voice4BufPtr, Voice5BufPtr, Voice6BufPtr, Voice7BufPtr;
extern unsigned int spcPCRam, spcRamDP;

void PrepareSaveState()
{
  spcPCRam -= (unsigned int)spcRam;
  spcRamDP -= (unsigned int)spcRam;

  Voice0BufPtr -= spcBuffera;
  Voice1BufPtr -= spcBuffera;
  Voice2BufPtr -= spcBuffera;
  Voice3BufPtr -= spcBuffera;
  Voice4BufPtr -= spcBuffera;
  Voice5BufPtr -= spcBuffera;
  Voice6BufPtr -= spcBuffera;
  Voice7BufPtr -= spcBuffera;
}

extern unsigned int SA1Stat;
extern unsigned char IRAM[2049], *SA1Ptr, *SA1RegPCS, *CurBWPtr, *SA1BWPtr;
extern unsigned char *SNSBWPtr;

void SaveSA1()
{
  SA1Stat &= 0xFFFFFF00;
  SA1Ptr -= (unsigned int)SA1RegPCS;

  if (SA1RegPCS == IRAM)
  {
    SA1Stat = (SA1Stat & 0xFFFFFF00) + 1;
  }

  if (SA1RegPCS == IRAM-0x3000)
  {
    SA1Stat = (SA1Stat & 0xFFFFFF00) + 2;
  }

  SA1RegPCS -= (unsigned int)romdata;
  CurBWPtr -= (unsigned int)romdata;
  SA1BWPtr -= (unsigned int)romdata;
  SNSBWPtr -= (unsigned int)romdata;
}

void RestoreSA1()
{
  SA1RegPCS += (unsigned int)romdata;
  CurBWPtr += (unsigned int)romdata;
  SA1BWPtr += (unsigned int)romdata;
  SNSBWPtr += (unsigned int)romdata;

  if ((SA1Stat & 0xFF) == 1)
  {
    SA1RegPCS = IRAM;
  }

  if ((SA1Stat & 0xFF) == 2)
  {
    SA1RegPCS = IRAM-0x3000;
  }

  SA1Ptr += (unsigned int)SA1RegPCS;
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
  spcPCRam += (unsigned int)spcRam;
  spcRamDP += (unsigned int)spcRam;

  ResState(Voice0BufPtr);
  ResState(Voice1BufPtr);
  ResState(Voice2BufPtr);
  ResState(Voice3BufPtr);
  ResState(Voice4BufPtr);
  ResState(Voice5BufPtr);
  ResState(Voice6BufPtr);
  ResState(Voice7BufPtr);
}

extern unsigned int statefileloc, CurrentHandle, SfxRomBuffer;
extern unsigned int SfxCROM, SfxLastRamAdr, SfxRAMMem;
extern unsigned int MsgCount, MessageOn;
extern unsigned char AutoIncSaveSlot, firstsaveinc, fnamest[512];
extern unsigned char cbitmode, NoPictureSave, txtsavemsg[14];
extern unsigned char *Msgptr, txtsavemsgfail[15];
extern unsigned short PrevPicture[64*56];

static FILE *fhandle;
void SRAMChdir();
void CapturePicture();

static void write_save_state_data(unsigned char **dest, void *data, size_t len)
{
  fwrite(data, 1, len, fhandle);
}

static const char zst_header_old[] = "ZSNES Save State File V0.6\x1a\x3c";
static const char zst_header_cur[] = "ZSNES Save State File V143\x1a\x8f";

void calculate_state_sizes()
{
  state_size = 0;
  zst_version = 143;
  copy_state_data(0, state_size_tally, false);
  cur_zst_size = state_size + sizeof(zst_header_cur)-1;

  state_size = 0;
  zst_version = 60;
  copy_state_data(0, state_size_tally, true);
  old_zst_size = state_size + sizeof(zst_header_old)-1;
}

void zst_save(FILE *fp, bool Thumbnail)
{
  fwrite(zst_header_cur, 1, sizeof(zst_header_cur)-1, fp); //-1 for null

  PrepareOffset();
  PrepareSaveState();
  unpackfunct();

  if (SFXEnable)
  {
    SfxRomBuffer -= SfxCROM;
    SfxLastRamAdr -= SfxRAMMem;
  }

  if (SA1Enable)
  {
    SaveSA1(); //Convert SA-1 stuff to standard, non displacement format
  }

  fhandle = fp; //Set global file handle
  copy_state_data(0, write_save_state_data, false);

  if (SFXEnable)
  {
    SfxRomBuffer += SfxCROM;
    SfxLastRamAdr += SfxRAMMem;
  }

  if (SA1Enable)
  {
    RestoreSA1(); //Convert back SA-1 stuff
  }

  if (Thumbnail)
  {
    CapturePicture();
    fwrite(PrevPicture, 1, 64*56*sizeof(unsigned short), fp);
  }

  ResetOffset();
  ResetState();
}

void statesaver()
{
  //Save State code
  #ifdef __LINUX__
  SRAMChdir();
  #endif

  if (MovieProcessing == 2)
  {
    bool mzt_save(char *, bool, bool);
    if (mzt_save(fnamest+1, (cbitmode && !NoPictureSave) ? true : false, false))
    {
      Msgptr = "RR STATE SAVED.";
      MessageOn = MsgCount;
    }
    return;
  }

  //'Auto increment savestate slot' code
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

  clim();

  if ((fhandle = fopen(fnamest+1,"wb")))
  {
    zst_save(fhandle, (cbitmode && !NoPictureSave) ? true : false);

    fclose(fhandle);

    //Display message on the screen, 'STATE X SAVED.'
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
    //Display message on the screen, 'UNABLE TO SAVE.'
    Msgptr = txtsavemsgfail;
    MessageOn = MsgCount;
  }

  stim();
}

extern unsigned int KeyLoadState, Totalbyteloaded, SfxMemTable[256], SfxCPB;
extern unsigned int SfxPBR, SfxROMBR, SfxRAMBR;
extern unsigned char pressed[256+128+64], multchange, txtloadmsg[15];
extern unsigned char txtconvmsg[16], txtnfndmsg[23], ioportval, SDD1Enable;
extern unsigned char nexthdma;

static void read_save_state_data(unsigned char **dest, void *data, size_t len)
{
  load_save_size += fread(data, 1, len, fhandle);
}

bool zst_load(FILE *fp)
{
  char zst_header_check[sizeof(zst_header_cur)-1];
  zst_version = 0;

  Totalbyteloaded += fread(zst_header_check, 1, sizeof(zst_header_check), fp);
  if (!memcmp(zst_header_check, zst_header_cur, sizeof(zst_header_check)-2))
  {
    zst_version = 143; //v1.43+
  }
  if (!memcmp(zst_header_check, zst_header_old, sizeof(zst_header_check)-2))
  {
    zst_version = 60; //v0.60 - v1.42
  }

  if (!zst_version) { return(false); } //Pre v0.60 saves are no longer loaded

  load_save_size = 0;
  fhandle = fp; //Set global file handle
  copy_state_data(0, read_save_state_data, true);
  Totalbyteloaded += load_save_size;
  
  if (SFXEnable)
  {
    SfxCPB = SfxMemTable[(SfxPBR & 0xFF)];
    SfxCROM = SfxMemTable[(SfxROMBR & 0xFF)];
    SfxRAMMem = (unsigned int)sfxramdata + ((SfxRAMBR & 0xFF) << 16);
    SfxRomBuffer += SfxCROM;
    SfxLastRamAdr += SfxRAMMem;
  }

  if (SA1Enable)
  {
    RestoreSA1(); //Convert back SA-1 stuff
    SA1UpdateDPageC();
  }

  if (SDD1Enable)
  {
    UpdateBanksSDD1();
  }

  //Clear cache check if state loaded
  memset(vidmemch2, 1, sizeof(vidmemch2));
  memset(vidmemch4, 1, sizeof(vidmemch4));
  memset(vidmemch8, 1, sizeof(vidmemch8));

  if (zst_version == 60) //Set new vars which old states did not have  
  {    
    prevoamptr = 0xFF;
    ioportval = 0xFF;
    spcnumread = 0;
    spchalted = 0xFFFFFFFF;
    nexthdma = 0;
  }
      
  repackfunct();

  initpitch();
  ResetOffset();
  ResetState();
  procexecloop();

  return(true);
}

void zst_sram_load(FILE *fp)
{
  fseek(fp, sizeof(zst_header_cur)-1 + PH65816regsize + 199635, SEEK_CUR);
  if (spcon)	{ fseek(fp, PHspcsave + PHdspsave + sizeof(DSPMem), SEEK_CUR); }
  if (C4Enable)	{ fseek(fp, 8192, SEEK_CUR); }
  if (SFXEnable)	{ fseek(fp, PHnum2writesfxreg + 131072, SEEK_CUR); }
  if (SA1Enable)
  {
    fseek(fp, PHnum2writesa1reg, SEEK_CUR);
    fread(SA1RAMArea, 1, 131072, fp);	// SA-1 sram
    fseek(fp, 15, SEEK_CUR);
  }
  if (DSP1Type)	{ fseek(fp, 2874, SEEK_CUR); }
  if (SETAEnable)	{ fread(setaramdata, 1, 4096, fp); } // SETA sram
  if (SPC7110Enable)	{ fseek(fp, PHnum2writespc7110reg + 65536, SEEK_CUR); }
  fseek(fp, 227, SEEK_CUR);
  if (ramsize)	{ fread(sram, 1, ramsize, fp); } // normal sram
}

void stateloader (unsigned char *statename, unsigned char keycheck, unsigned char xfercheck)
{
  #ifdef __LINUX__
  SRAMChdir();
  #endif

  if (keycheck)
  {
    unsigned char statevalue;

    pressed[1] = 0;
    pressed[KeyLoadState] = 2;
    multchange = 1;

    //Get the state number
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

  switch (MovieProcessing)
  {
    bool mzt_load(char *, bool);

    case 1:
      if (mzt_load(statename, true))
      {
        Msgptr = "RR STATE LOADED.";
        MessageOn = MsgCount;
      }
      return;

    case 2:
      if (mzt_load(statename, false))
      {
        Msgptr = "CHAPTER LOADED.";
        MessageOn = MsgCount;
      }
      return;
  }

  clim();

  //Actual state loading code
  if ((fhandle = fopen(statename,"rb")) != NULL)
  {
    if (xfercheck) { Totalbyteloaded = 0; }

    if (zst_load(fhandle))
    {
      Msgptr = txtloadmsg; // 'STATE X LOADED.'
    }
    else
    {
      Msgptr = txtconvmsg; // 'STATE X TOO OLD.'
    }

    fclose(fhandle);
  }
  else
  {
    Msgptr = txtnfndmsg; // 'UNABLE TO LOAD STATE X.'
  }

  stim();

  if (keycheck)
  {
    MessageOn = MsgCount;
  }
}

void debugloadstate()
{
  stateloader(fnamest+1, 0, 0);
}

void loadstate()
{
  stateloader(fnamest+1, 1, 0);
}

void loadstate2()
{
  stateloader(fnamest+1, 0, 1);
}

extern unsigned char Netfname[11];

void loadstate3()
{
  stateloader(Netfname, 0, 1);
}
