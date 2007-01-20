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



#ifdef __UNIXSDL__
#include "gblhdr.h"
#include "linux/audio.h"
#define DIR_SLASH "/"
#else
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#define DIR_SLASH "\\"
#endif
#include "asm_call.h"
#include "cfg.h"
#include "input.h"
#include "zpath.h"
#include "cpu/memtable.h"

#define NUMCONV_FR4
#include "numconv.h"

#ifndef __GNUC__
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif

//C++ style code in C
#define bool unsigned char
#define true 1
#define false 0

//NSRT Goodness
#define Lo 0x7FC0
#define Hi 0xFFC0
#define EHi 0x40FFC0

#define MB_bytes 0x100000
#define Mbit_bytes 0x20000

//Offsets to add to infoloc start to reach particular variable
#define BankOffset       21 //Contains Speed as well
#define TypeOffset       22
#define ROMSizeOffset    23
#define SRAMSizeOffset   24
#define CountryOffset    25
#define CompanyOffset    26
#define VersionOffset    27
#define InvCSLowOffset   28
#define InvCSHiOffset    29
#define CSLowOffset      30
#define CSHiOffset       31
//Additional defines for the BS header
#define BSYearOffset     21 //Not sure how to calculate year yet
#define BSMonthOffset    22
#define BSDayOffset      23
#define BSBankOffset     24
#define BSSizeOffset     25 //Contains Type as well
//26 - 31 is the same
#define ResetLoOffset    60
#define ResetHiOffset    61


// Some archaic code from an unfinished Dynarec
extern unsigned int curexecstate;
extern unsigned char spcon;

void procexecloop()
{
  curexecstate &= 0xFFFFFF00;

  if (spcon)  { curexecstate += 3; }
  else  { curexecstate += 1; }
}

void Debug_WriteString(char *str)
{
  FILE *fp = 0;
  fp = fopen_dir(ZCfgPath, "zsnes.dbg", "w");
  if (!fp) { return; }
  fputs(str, fp);
  fclose(fp);
}

//I want to port over the more complicated
//functions from init.asm, or replace with
//better versions from NSRT. -Nach

//init.asm goodness
extern unsigned int NumofBanks;
extern unsigned int NumofBytes;
extern unsigned int *romdata;
extern unsigned char romtype;
extern unsigned char Interleaved;

unsigned int maxromspace;
unsigned int curromspace;
unsigned int infoloc;
unsigned int ramsize;
unsigned int ramsizeand;

bool SplittedROM;
unsigned int addOnStart;
unsigned int addOnSize;


//Deinterleave functions
bool validChecksum(unsigned char *ROM, int BankLoc)
{
  if (ROM[BankLoc + InvCSLowOffset] + (ROM[BankLoc + InvCSHiOffset] << 8) +
      ROM[BankLoc + CSLowOffset] + (ROM[BankLoc + CSHiOffset] << 8) == 0xFFFF)
  {
    return(true);
  }
  return(false);
}

bool valid_normal_bank(unsigned char bankbyte)
{
  switch (bankbyte)
  {
    case 32: case 33: case 48: case 49:
    return(true);
    break;
  }
  return(false);
}

bool EHiHeader(unsigned char *ROM, int BankLoc)
{
  if (validChecksum(ROM, BankLoc) && (ROM[BankLoc+BankOffset] == 53 || ROM[BankLoc+BankOffset] == 37))
  {
    return(true);
  }
  return(false);
}

void SwapData(unsigned int *loc1, unsigned int *loc2, unsigned int amount)
{
  unsigned int temp;
  while (amount--)
  {
    temp = *loc1;
    *loc1++ = *loc2;
    *loc2++ = temp;
  }
}

void swapBlocks(char *blocks)
{
  unsigned int i, j;
  for (i = 0; i < NumofBanks; i++)
  {
    for (j = 0; j < NumofBanks; j++)
    {
      if (blocks[j] == (char)i)
      {
        char b;
        SwapData(romdata + blocks[i]*0x2000, romdata + blocks[j]*0x2000, 0x2000);
        b = blocks[j];
        blocks[j] = blocks[i];
        blocks[i] = b;
        break;
      }
    }
  }
}

void deintlv1()
{
  char blocks[256];
  int i, numblocks = NumofBanks/2;
  for (i = 0; i < numblocks; i++)
  {
    blocks[i * 2] = i + numblocks;
    blocks[i * 2 + 1] = i;
  }
  swapBlocks(blocks);
}

void CheckIntl1(unsigned char *ROM)
{
  unsigned int ROMmidPoint = NumofBytes / 2;
  if (validChecksum(ROM, ROMmidPoint + Lo) &&
     !validChecksum(ROM, Lo) &&
      ROM[ROMmidPoint+Lo+CountryOffset] < 14) //Country Code
  {
    deintlv1();
    Interleaved = true;
  }
  else if (validChecksum(ROM, Lo) && !validChecksum(ROM, Hi) &&
           ROM[Lo+CountryOffset] < 14 && //Country code
           //Rom make up
          (ROM[Lo+BankOffset] == 33 || ROM[Lo+BankOffset] == 49 ||
           ROM[Lo+BankOffset] == 53 || ROM[Lo+BankOffset] == 58))
  {
    if (ROM[Lo+20] == 32 ||//Check that Header name did not overflow
      !(ROM[Lo+BankOffset] == ROM[Lo+20] || ROM[Lo+BankOffset] == ROM[Lo+19] ||
        ROM[Lo+BankOffset] == ROM[Lo+18] || ROM[Lo+BankOffset] == ROM[Lo+17]))
    {
      deintlv1();
      Interleaved = true;
    }
  }
}

void CheckIntlEHi(unsigned char *ROM)
{
  if (EHiHeader(ROM, Lo))
  {
    unsigned int oldNumBanks = NumofBanks;

    //Swap 4MB ROM with the other one
    SwapData(romdata, romdata+((NumofBytes-0x400000)/4), 0x100000);

    //Deinterleave the 4MB ROM first
    NumofBanks = 128;
    deintlv1();

    //Now the other one
    NumofBanks = oldNumBanks - 128;
    romdata += 0x100000; //Ofset pointer
    deintlv1();

    //Now fix the data and we're done
    NumofBanks = oldNumBanks;
    romdata -= 0x100000;

    Interleaved = true;
  }
}

//ROM loading functions, which some strangly enough were in guiload.inc
bool AllASCII(unsigned char *b, int size)
{
  int i;
  for (i = 0; i < size; i++)
  {
    if (b[i] && (b[i] < 32 || b[i] > 126))
    {
      return(false);
    }
  }
  return(true);
}

//Code to detect if opcode sequence is a valid and popular one for an SNES ROM
//Code by Cowering
static bool valid_start_sequence(unsigned char opcode1, unsigned char opcode2, unsigned char opcode3)
{
  switch (opcode1)
  {
    case 0x78: case 0x5c: case 0x18: case 0xad:
      return(true);
      break;
    case 0x4b:
      if (opcode2 == 0xab && (opcode3 == 0x18 || opcode3 == 0x20))
      {
        return(true);
      }
      break;
    case 0x4c:
      if ((opcode2 == 0x00 || opcode2 == 0xc0) && opcode3 == 0x84)
      {
        return(true);
      }
      if (opcode2 == 0x6d && opcode3 == 0x86)
      {
        return(true);
      }
      if (opcode2 == 0x00 && opcode3 == 0x80)
      {
        return(true);
      }
      break;
    case 0xc2:
      if (opcode2 == 0x30 && opcode3 == 0xa9)
      {
        return(true);
      }
      break;
    case 0x20:
      if ((opcode2 == 0x16 || opcode2 == 0x06) && opcode3 == 0x80)
      {
        return(true);
      }
      break;
    case 0x80:
      if ((opcode2 == 0x16 && opcode3 == 0x4c) ||
          (opcode2 == 0x07 && opcode3 == 0x82))
      {
        return(true);
      }
      break;
    case 0x9c:
      if (opcode2 == 0x00 && opcode3 == 0x21)
      {
        return(true);
      }
      break;
    case 0xa2:
      if (opcode2 == 0xff && opcode3 == 0x86)
      {
        return(true);
      }
      break;
    case 0xa9:
      if ((opcode2 == 0x00 && (opcode3 = 0x48 || opcode3  == 0x4b)) ||
          (opcode2 == 0x8f && opcode3 == 0x8d) ||
          (opcode2 == 0x20 && opcode3 == 0x4b) ||
          (opcode2 == 0x1f && opcode3 == 0x4b))
      {
        return(true);
      }
      break;
  }
  return(false);
}

static int valid_reset(unsigned char *Buffer)
{
  unsigned char *ROM = (unsigned char *)romdata;
  unsigned short Reset = Buffer[ResetLoOffset] | ((unsigned short)Buffer[ResetHiOffset] << 8);
  if ((Reset != 0xFFFF) && (Reset & 0x8000))
  {
    unsigned char opcode1 = ROM[(Reset+0) & 0x7FFF];
    unsigned char opcode2 = ROM[(Reset+1) & 0x7FFF];
    unsigned char opcode3 = ROM[(Reset+2) & 0x7FFF];

    if (valid_start_sequence(opcode1, opcode2, opcode3))
    {
      return(10);
    }
    return(2);
  }
  return(-4);
}

int InfoScore(unsigned char *Buffer)
{
  int score = valid_reset(Buffer);
  if (validChecksum(Buffer, 0))                 { score += 5; }
  if (Buffer[CompanyOffset] == 0x33)            { score += 3; }
  if (!Buffer[ROMSizeOffset])                   { score += 2; }
  if ((1 << (Buffer[ROMSizeOffset] - 7)) > 48)  { score -= 2; }
  if ((8 << Buffer[SRAMSizeOffset]) > 1024)     { score -= 2; }
  if (Buffer[CountryOffset] < 14)               { score += 2; }
  if (!AllASCII(Buffer, 20))                    { score -= 2; }
  if (valid_normal_bank(Buffer[BSBankOffset]))  { score += 2; }
  return(score);
}

extern unsigned char ForceHiLoROM;
extern unsigned char forceromtype;

void BankCheck()
{
  unsigned char *ROM = (unsigned char *)romdata;
  infoloc = 0;
  Interleaved = false;

  if (NumofBytes < Lo)
  {
    romtype = 1;
    infoloc = 1; //Whatever, we just need a valid location
  }

  if (NumofBytes < Hi)
  {
    romtype = 1;
    infoloc = Lo;
  }

  if (NumofBytes >= 0x500000)
  {
    //Deinterleave if neccesary
    CheckIntlEHi(ROM);

    if (EHiHeader(ROM, EHi))
    {
      romtype = 2;
      infoloc = EHi;
    }
  }

  if (!infoloc)
  {
    static bool CommandLineForce2 = false;
    int loscore, hiscore;

    //Deinterleave if neccesary
    CheckIntl1(ROM);

    loscore = InfoScore(ROM+Lo);
    hiscore = InfoScore(ROM+Hi);

    switch(ROM[Lo + BankOffset])
    {
      case 32: case 35: case 48: case 50:
        loscore += 3;
        break;
    }
    switch(ROM[Hi + BankOffset])
    {
      case 33: case 49: case 53: case 58:
        hiscore += 3;
        break;
    }

    /*
    Force code.
    ForceHiLoROM is from the GUI.
    forceromtype is from Command line, we have a static var
    to prevent forcing a secong game loaded from the GUI when
    the first was loaded from the command line with forcing.
    */
    if (ForceHiLoROM == 1 ||
        (forceromtype == 1 && !CommandLineForce2))
    {
      CommandLineForce2 = true;
      loscore += 50;
    }
    else if (ForceHiLoROM == 2 ||
             (forceromtype == 2 && !CommandLineForce2))
    {
      CommandLineForce2 = true;
      hiscore += 50;
    }

    if (hiscore > loscore)
    {
      romtype = 2;
      infoloc = Hi;
    }
    else
    {
      romtype = 1;
      infoloc = Lo;
    }
  }
}

//Chip detection functions
bool CHIPBATT, BSEnable, C4Enable, DSP1Enable, DSP2Enable, DSP3Enable;
bool DSP4Enable, OBCEnable, RTCEnable, SA1Enable, SDD1Enable, SFXEnable;
bool SETAEnable; //ST010 & 11
bool SGBEnable, SPC7110Enable, ST18Enable;

void chip_detect()
{
  unsigned char *ROM = (unsigned char *)romdata;

  C4Enable = RTCEnable = SA1Enable = SDD1Enable = OBCEnable = CHIPBATT = false;
  SGBEnable = ST18Enable = DSP1Enable = DSP2Enable = DSP3Enable = false;
  DSP4Enable = SPC7110Enable = BSEnable = SFXEnable = SETAEnable = false;

  //DSP Family
  if (ROM[infoloc+TypeOffset] == 3)
  {
    if (ROM[infoloc+BankOffset] == 48) { DSP4Enable = true; }
    else { DSP1Enable = true; }
    return;
  }

  if (ROM[infoloc+TypeOffset] == 5)
  {
    CHIPBATT = true;
    if (ROM[infoloc+BankOffset] == 32) { DSP2Enable = true; }
    else if (ROM[infoloc+BankOffset] == 48 && ROM[infoloc+CompanyOffset] == 0xB2) //Bandai
    { DSP3Enable = true; }
    else { DSP1Enable = true; }
    return;
  }

  switch((unsigned short)ROM[infoloc+BankOffset] | (ROM[infoloc+TypeOffset] << 8))
  {
    case 0x1320:                             //Mario Chip 1
    case 0x1420:                             //GSU-x
      SFXEnable = true;
      return;
      break;

    case 0x1520:                            //GSU-x + Battery
    case 0x1A20:                            //GSU-1 + Battery + Start in 21MHz
      SFXEnable = true;
      CHIPBATT = true;
      return;
      break;

    case 0x2530:
      OBCEnable = true;
      CHIPBATT = true;
      return;
      break;

    case 0x3423:
      SA1Enable = true;
      return;
      break;

    case 0x3223: //One sample game seems to use this for some reason
    case 0x3523:
      SA1Enable = true;
      CHIPBATT = true;
      return;
      break;

    case 0x4332:
      SDD1Enable = true;
      return;
      break;

    case 0x4532:
      SDD1Enable = true;
      CHIPBATT = true;
      return;
      break;

    case 0x5535:
      RTCEnable = true;
      CHIPBATT = true;
      return;
      break;

    case 0xE320:
      SGBEnable = true;
      return;
      break;

    case 0xF320:
      C4Enable = true;
      return;
      break;

    case 0xF530:
      ST18Enable = true;
      CHIPBATT = true; //Check later if this should be removed
      return;
      break;

    case 0xF53A:
      SPC7110Enable = true;
      CHIPBATT = true;
      return;
      break;

    case 0xF630:
      SETAEnable = true;
      CHIPBATT = true;
      return;
      break;

    case 0xF93A:
      SPC7110Enable = true;
      RTCEnable = true;
      CHIPBATT = true;
      return;
      break;
  }

  //BS Dump
  if ((ROM[infoloc+CompanyOffset] == 0x33 || ROM[infoloc+CompanyOffset] == 0xFF) &&
      (!ROM[infoloc+BSYearOffset] || (ROM[infoloc+BSYearOffset] & 131) == 128) &&
      valid_normal_bank(ROM[infoloc+BSBankOffset]))
  {
    unsigned char m = ROM[infoloc+BSMonthOffset];
    if (!m && !ROM[infoloc+BSDayOffset])
    {
      //BS Add-on cart
      return;
    }
    if ((m == 0xFF && ROM[infoloc+BSDayOffset] == 0xFF) ||
        (!(m & 0xF) && ((m >> 4) - 1 < 12)))
    {
      BSEnable = true;
      return;
    }
  }
}

//Checksum functions
unsigned short sum(unsigned char *array, unsigned int size)
{
  unsigned short theSum = 0;
  unsigned int i;

  //Prevent crashing by reading too far (needed for messed up ROMs)
  if (array + size > (unsigned char *)romdata + maxromspace)
  {
    return(0xFFFF);
  }

  for (i = 0; i < size; i++)
  {
    theSum += array[i];
  }
  return(theSum);
}

static unsigned short Checksumvalue;
void CalcChecksum()
{
  unsigned char *ROM = (unsigned char *)romdata;

  if (SplittedROM)
  {
    Checksumvalue = sum(ROM+addOnStart, addOnSize);
    Checksumvalue -= sum(ROM+infoloc+addOnStart-16, 48);
  }
  else if (SPC7110Enable)
  {
    Checksumvalue = sum(ROM, curromspace);
  }
  else
  {
    Checksumvalue = sum(ROM, curromspace);
    if (NumofBanks > 128 && maxromspace == 6*MB_bytes)
    {
      Checksumvalue += sum(ROM+4*MB_bytes, 2*MB_bytes);
    }
    if (BSEnable)
    {
      Checksumvalue -= sum(&ROM[infoloc - 16], 48); //Fix for BS Dumps
    }
  }
}

static void rom_memcpy(unsigned char *dest, unsigned char *src, size_t len)
{
  unsigned char *endrom = (unsigned char *)romdata+maxromspace;
  while (len-- && (dest < endrom) && (src < endrom))
  {
    *dest++ = *src++;
  }
}

//This will mirror up non power of two ROMs to powers of two
static unsigned int mirror_rom(unsigned char *start, unsigned int length)
{
  unsigned int mask = 0x800000;
  while (!(length & mask)) { mask >>= 1; }

  length -= mask;
  if (length)
  {
    start += mask;
    length = mirror_rom(start, length);

    while (length != mask)
    {
      rom_memcpy(start+length, start, length);
      length += length;
    }
  }

  return(length+mask);
}

//Misc functions
void MirrorROM(unsigned char *ROM)
{
  unsigned int ROMSize, StartMirror = 0;
  if (!SPC7110Enable)
  {
    curromspace = mirror_rom((unsigned char *)romdata, curromspace);
  }
  else if (curromspace == 0x300000)
  {
    memcpy((unsigned char *)romdata+curromspace, romdata, curromspace);
    curromspace += curromspace;
  }

  if (curromspace > maxromspace)
  {
    curromspace = maxromspace;
  }
  NumofBanks = curromspace >> 15;

  //This will mirror (now) full sized ROMs through the ROM buffer
  ROMSize = curromspace;
  while (ROMSize < maxromspace)
  {
    ROM[ROMSize++] = ROM[StartMirror++];
  }

  //If ROM was too small before, but now decent size with mirroring, adjust location
  if (infoloc < Lo)
  {
    infoloc = Lo;
  }
}


void SetupSramSize()
{
  unsigned char *ROM = (unsigned char *)romdata;
  if (BSEnable)
  {
    ramsize = 0;
  }
  else if (SFXEnable)
  {
    if (ROM[infoloc+CompanyOffset] == 0x33) //Extended header
    {
      ramsize = 8 << ((unsigned int)ROM[infoloc-3]);
    }
    else
    {
      ramsize = 256;
    }
  }
  else if (SETAEnable)
  {
    ramsize = 32;
  }
  else if (!strncmp((char *)ROM, "BANDAI SFC-ADX", 14))
  {  // For the Sufami Turbo
    ramsize = 8 << ((unsigned int)ROM[0x100032]);
  }
  else
  {
    ramsize = ((ROM[infoloc+SRAMSizeOffset]) ? (8 << ((unsigned int)ROM[infoloc+SRAMSizeOffset])) : 0);
  }

  //Fix if some ROM goes nuts on size
  if (ramsize > 1024)
  {
    ramsize = 1024;
  }

  //Convert from Kb to bytes;
  ramsize *= 128;
  ramsizeand = ramsize-1;
}

//File loading code
bool Header512;

char CSStatus[41], CSStatus2[41], CSStatus3[41], CSStatus4[41];

void DumpROMLoadInfo()
{
  FILE *fp = 0;

  if (RomInfo) //rominfo.txt info dumping enabled?
  {
    fp = fopen_dir(ZCfgPath, "rominfo.txt", "w");
    if (!fp) { return; }
    fputs("This is the info for the last game you ran.\n\nFile: ", fp);
    fputs(ZCartName, fp);
    fputs(" Header: ", fp);
    fputs(Header512 ? "Yes\n" : "No\n", fp);
    fputs(CSStatus, fp);
    fputs("\n", fp);
    fputs(CSStatus2, fp);
    fputs("\n", fp);
    fputs(CSStatus3, fp);
    fputs("\n", fp);
    fputs(CSStatus4, fp);
    fputs("\n", fp);
    fclose(fp);
  }
}

void loadFile(char *filename)
{
  bool multifile = false;
  char *incrementer = 0;
  unsigned char *ROM = (unsigned char *)romdata;

  if (strlen(filename) >= 3) //Char + ".1"
  {
    char *ext = filename+strlen(filename)-2;
    if (!strcmp(ext, ".1") || !strcasecmp(ext, ".A"))
    {
      incrementer = ext + 1;
      multifile = true;
    }
  }

  for (;;)
  {
    struct stat stat_results;
    stat_dir(ZRomPath, filename, &stat_results);

    if ((unsigned int)stat_results.st_size <= maxromspace+512-curromspace)
    {
      FILE *fp = 0;
      fp = fopen_dir(ZRomPath, filename, "rb");

      if (!fp) { return; }

      if (curromspace && ((stat_results.st_size & 0x7FFF) == 512))
      {
        stat_results.st_size -= 512;
        fseek(fp, 512, SEEK_SET);
      }

      fread(ROM+curromspace, stat_results.st_size, 1, fp);
      fclose(fp);

      curromspace += stat_results.st_size;

      if (!multifile) { return; }

      (*incrementer)++;
    }
    else
    {
      return;
    }
  }
}

void loadGZipFile(char *filename)
{
  //Open file for size reading
  FILE *fp = fopen_dir(ZRomPath, filename, "rb");
  if (fp)
  {
    int fsize, gzsize;
    gzFile GZipFile;

    fseek(fp, -4, SEEK_END);
    gzsize = fread4(fp);
    fsize = ftell(fp);
    rewind(fp);

    //Open GZip file for decompression, use existing file handle
    if ((GZipFile = gzdopen(fileno(fp), "rb")))
    {
      int len = gzdirect(GZipFile) ? fsize : gzsize;
      if (len && ((unsigned int)len <= maxromspace+512) && (gzread(GZipFile, romdata, len) == len))
      {
        curromspace = len; //Success
      }
      gzclose(GZipFile);
    }
    fclose(fp);
  }
}

void loadZipFile(char *filename)
{
  int err, fileSize;
  unsigned char *ROM = (unsigned char *)romdata;
  bool multifile = false, NSS = false;
  char *incrementer = 0;

  unzFile zipfile = unzopen_dir(ZRomPath, filename); //Open zip file
  int cFile = unzGoToFirstFile(zipfile); //Set cFile to first compressed file
  unz_file_info cFileInfo; //Create variable to hold info for a compressed file

  int LargestGoodFile = 0; //To keep track of largest file

  //Variables for the file we pick
  char ourFile[256];
  ourFile[0] = '\n';

  while(cFile == UNZ_OK) //While not at end of compressed file list
  {
    //Temporary char array for file name
    char cFileName[256];

    //Gets info on current file, and places it in cFileInfo
    unzGetCurrentFileInfo(zipfile, &cFileInfo, cFileName, 256, NULL, 0, NULL, 0);

    //Get the file's size
    fileSize = cFileInfo.uncompressed_size;

    //Find split files
    if (strlen(cFileName) >= 3) //Char + ".1"
    {
      char *ext = cFileName+strlen(cFileName)-2;
      if (!strcmp(ext, ".1") || !strcasecmp(ext, ".A"))
      {
        strcpy(ourFile, cFileName);
        incrementer = ourFile+strlen(ourFile)-1;
        multifile = true;
        break;
      }
    }

    //Find Nintendo Super System ROMs
    if (strlen(cFileName) >= 5) //Char + ".IC2"
    {
      char *ext = cFileName+strlen(cFileName)-4;
      if (!strncasecmp(ext, ".IC", 3))
      {
        strcpy(ourFile, cFileName);
        incrementer = ourFile+strlen(ourFile)-1;
        *incrementer = '7';
        NSS = true;
        break;
      }
    }

    //Check for valid ROM based on size
    if (((unsigned int)fileSize <= maxromspace+512) &&
        (fileSize > LargestGoodFile))
    {
      strcpy(ourFile, cFileName);
      LargestGoodFile = fileSize;
    }

    //Go to next file in zip file
    cFile = unzGoToNextFile(zipfile);
  }

  //No files found
  if (ourFile[0] == '\n')
  {
    unzClose(zipfile);
    return;
  }

  for (;;)
  {
    //Sets current file to the file we liked before
    if (unzLocateFile(zipfile, ourFile, 1) != UNZ_OK)
    {
      if (NSS)
      {
        (*incrementer)--;
        continue;
      }
      unzClose(zipfile);
      return;
    }

    //Gets info on current file, and places it in cFileInfo
    unzGetCurrentFileInfo(zipfile, &cFileInfo, ourFile, 256, NULL, 0, NULL, 0);

    //Get the file's size
    fileSize = cFileInfo.uncompressed_size;

    //Too big?
    if (curromspace + fileSize > maxromspace+512)
    {
      unzClose(zipfile);
      return;
    }

    //Open file
    unzOpenCurrentFile(zipfile);

    //Read file into memory
    err = unzReadCurrentFile(zipfile, ROM+curromspace, fileSize);

    //Close file
    unzCloseCurrentFile(zipfile);

    //Encountered error?
    if (err != fileSize)
    {
      unzClose(zipfile);
      return;
    }

    if (curromspace && ((fileSize & 0x7FFF) == 512))
    {
      fileSize -= 512;
      memmove(ROM+curromspace, ROM+curromspace+512, fileSize);
    }

    curromspace += fileSize;

    if (NSS)
    {
      if (!*incrementer) { return; }
      (*incrementer)--;
      continue;
    }

    if (!multifile)
    {
      unzClose(zipfile);
      return;
    }
    (*incrementer)++;
  }
}

void load_file_fs(char *path)
{
  unsigned char *ROM = (unsigned char *)romdata;

  if (isextension(path, "jma"))
  {
    #ifdef NO_JMA
    puts("This binary was built without JMA support.");
    #else
    load_jma_file_dir(ZRomPath, path);
    #endif
  }
  if (isextension(path, "zip"))
  {
    loadZipFile(path);
  }
  if (isextension(path, "gz"))
  {
    loadGZipFile(path);
  }
  else
  {
    loadFile(path);
  }

  if ((curromspace & 0x7FFF) == 512)
  {
    memmove(ROM, ROM+512, addOnStart);
    curromspace -= 512;
  }
}

char *STCart2 = 0;
unsigned char *sram2;
extern unsigned char *sram;

void SplitSetup(char *basepath, char *basefile, unsigned int MirrorSystem)
{
  unsigned char *ROM = (unsigned char *)romdata;

  curromspace = 0;
  if (maxromspace < addOnStart+addOnSize) { return; }
  memmove(ROM+addOnStart, ROM, addOnSize);

  if (!*basepath)
  {
    load_file_fs(basefile);
  }
  else
  {
    load_file_fs(basepath);
  }

  if (!curromspace) { return; }

  switch (MirrorSystem)
  {
    case 1:
      memcpy(ROM+0x100000, ROM, 0x100000); //Mirror 8 to 16
      break;

    case 2:
      memcpy(ROM+0x180000, ROM+0x100000, 0x80000); //Mirrors 12 to 16
      memcpy(ROM+0x200000, ROM+0x400000, 0x80000); //Copy base over
      memset(ROM+0x280000, 0, 0x180000);           //Blank out rest
      break;

    case 3:
      memcpy(ROM+0x40000, ROM, 0x40000);
      memcpy(ROM+0x80000, ROM, 0x80000);
      break;
  }

  curromspace = addOnStart+addOnSize;
  SplittedROM = true;
}

void SplitSupport()
{
  char *ROM = (char *)romdata;
  SplittedROM = false;

  //Same Game add on
  if (curromspace == 0x80000 && ROM[Hi+CompanyOffset] == 0x33 &&
      !ROM[Hi+BankOffset] && !ROM[Hi+BSMonthOffset] && !ROM[Hi+BSDayOffset])
  {
    addOnStart = 0x200000;
    addOnSize = 0x80000;
    SplitSetup(SGPath, "SAMEGAME.ZIP", 1);
  }

  //SD Gundam G-Next add on
  if (curromspace == 0x80000 && ROM[Lo+CompanyOffset] == 0x33 &&
      !ROM[Lo+BankOffset] && !ROM[Lo+BSMonthOffset] && !ROM[Lo+BSDayOffset] && !strncmp(ROM+Lo, "GNEXT", 5))
  {
    addOnStart = 0x400000;
    addOnSize = 0x80000;
    SplitSetup(GNextPath, "G-NEXT.ZIP", 2);
    addOnStart = 0x200000; //Correct for checksum calc
  }

  //Sufami Turbo
  if (!strncmp(ROM, "BANDAI SFC-ADX", 14))
  {
    if (!STCart2)
    {
      addOnStart = 0x100000;
      addOnSize = curromspace;
      SplitSetup(STPath, "STBIOS.ZIP", 3);
    }
    else if (maxromspace >= (curromspace<<2)+0x100000)
    {
      memcpy(ROM+curromspace+curromspace, ROM, curromspace);
      memcpy(ROM+curromspace*3, ROM, curromspace);
      curromspace = 0;
      load_file_fs(STCart2);
      memcpy(ROM+curromspace, ROM, curromspace);
      SwapData(romdata, romdata+(curromspace>>1), curromspace>>1);
      addOnSize = curromspace<<2;
      addOnStart = 0x100000;
      SplitSetup(STPath, "STBIOS.ZIP", 3);
      addOnSize = (curromspace-addOnStart) >> 2; //Correct for checksum calc
      sram2 = sram+65536;
    }
  }
}

bool NSRTHead(unsigned char *ROM)
{
  unsigned char *NSRTHead = ROM + 0x1D0; //NSRT Header Location

  if (!strncmp("NSRT", (char*)&NSRTHead[24],4) && NSRTHead[28] == 22)
  {
    if ((sum(NSRTHead, 32) & 0xFF) != NSRTHead[30] ||
        NSRTHead[30] + NSRTHead[31] !=  255 ||
        (NSRTHead[0] & 0x0F) > 13 ||
        ((NSRTHead[0] & 0xF0) >> 4) > 3 ||
        ((NSRTHead[0] & 0xF0) >> 4) == 0)
    {
      return(false); //Corrupt
    }
    return(true); //NSRT header
  }
  return(false); //None
}

void calculate_state_sizes(), InitRewindVars(), zst_init();
bool findZipIPS(char *, char *);
extern bool EMUPause;
extern unsigned char device1, device2;
unsigned char lorommapmode2, curromsize, snesinputdefault1, snesinputdefault2;
bool input1gp, input1mouse, input2gp, input2mouse, input2scope, input2just;

void loadROM()
{
  bool isCompressed = false, isZip = false;

  zst_init();

  EMUPause = false;
  curromspace = 0;

  if (isextension(ZCartName, "jma"))
  {
    #ifdef NO_JMA
    puts("This binary was built without JMA support.");
    #else
    isCompressed = true;
    load_jma_file_dir(ZRomPath, ZCartName);
    #endif
  }
  else if (isextension(ZCartName, "zip"))
  {
    isCompressed = true;
    isZip = true;
    loadZipFile(ZCartName);
  }
  else if (isextension(ZCartName, "gz"))
  {
    isCompressed = true;
    loadGZipFile(ZCartName);
  }

  if (!isCompressed) { loadFile(ZCartName); }

  Header512 = false;

  if (!curromspace) { return; }

  if (!strncmp("GAME DOCTOR SF 3", (char *)romdata, 16) ||
      !strncmp("SUPERUFO", (char *)romdata+8, 8))
  {
    Header512 = true;
  }
  else
  {
    int HeadRemain = (curromspace & 0x7FFF);
    switch(HeadRemain)
    {
      case 0:
        break;

      case 512:
        Header512 = true;
        break;

      default:
      {
        unsigned char *ROM = (unsigned char *)romdata;

        //SMC/SWC header
        if (ROM[8] == 0xAA && ROM[9]==0xBB && ROM[10]== 4)
        {
          Header512 = true;
        }
        //FIG header
        else if ((ROM[4] == 0x77 && ROM[5] == 0x83) ||
                 (ROM[4] == 0xDD && ROM[5] == 0x82) ||
                 (ROM[4] == 0xDD && ROM[5] == 2) ||
                 (ROM[4] == 0xF7 && ROM[5] == 0x83) ||
                 (ROM[4] == 0xFD && ROM[5] == 0x82) ||
                 (ROM[4] == 0x00 && ROM[5] == 0x80) ||
                 (ROM[4] == 0x47 && ROM[5] == 0x83) ||
                 (ROM[4] == 0x11 && ROM[5] == 2))
        {
          Header512 = true;
        }
        break;
      }
    }
  }

  device1 = 0;
  device2 = 0;
  input1gp = true;
  input1mouse = true;
  input2gp = true;
  input2mouse = true;
  input2scope = true;
  input2just = true;

  if (Header512)
  {
    unsigned char *ROM = (unsigned char *)romdata;
    if (NSRTHead(ROM))
    {
      switch (ROM[0x1ED] & 0xF0) //Port 1
      {
        case 0x00: //Gamepad
          input1mouse = false;
          break;

        case 0x10: //Mouse port 1
          device1 = 1;
          input1gp = false;
          break;

        case 0x20: //Mouse or Gamepad port 1
          device1 = 1;
          break;

        case 0x90: //Lasabirdie - not yet supported
          input1gp = false;
          input1mouse = false;
          break;
      }

      switch (ROM[0x1ED] & 0x0F) //Port 1
      {
        case 0x00: //Gamepad
          input2mouse = false;
          input2scope = false;
          input2just = false;
          break;

        case 0x01: //Mouse port 2
          device2 = 1;
          input2gp = false;
          input2scope = false;
          input2just = false;
          break;

        case 0x02: //Mouse or Gamepad port 2
          device1 = 2;
          input2just = false;
          input2scope = false;
          break;

        case 0x03: //Super Scope port 2
          device2 = 2;
          input2gp = false;
          input2mouse = false;
          input2just = false;
          break;

        case 0x04: //Super Scope or Gamepad port 2
          device2 = 2;
          input2mouse = false;
          input2just = false;
          break;

        case 0x05: //Justifier (Lethal Enforcer gun) port 2
          device2 = 3;
          input2mouse = false;
          input2scope = false;
          break;

        case 0x06: //Multitap port 2
          input2gp = false;
          input2mouse = false;
          input2just = false;
          input2scope = false;
          break;

        case 0x07: //Mouse or Gamepad port 1, Mouse, Super Scope, or Gamepad port 2
          input2just = false;
          break;

        case 0x08: //Mouse or Multitap port 2
          device2 = 1;
          input2just = false;
          input2scope = false;
          break;

        case 0x09: //Lasabirdie - not yet supported
          input2gp = false;
          input2mouse = false;
          input2just = false;
          input2scope = false;
          break;

        case 0x0A: //Barcode Battler - not yet supported
          input2gp = false;
          input2mouse = false;
          input2just = false;
          input2scope = false;
          break;
      }
    }
    curromspace -= 512;
    memmove((unsigned char *)romdata, ((unsigned char *)romdata)+512, curromspace);
  }

  snesinputdefault1 = device1;
  snesinputdefault2 = device2;

  SplitSupport();

  if (isZip)
  {
    int i;
    char ext[4];

    strcpy(ext, "ips");
    for (i = 0; findZipIPS(ZCartName, ext); i++)
    {
      if (i > 9) { break; }
      ext[2] = i+'0';
    }
  }

  if (curromspace)
  {
    unsigned char *ROM = (unsigned char *)romdata;
    NumofBytes = curromspace;
    NumofBanks = curromspace >> 15;
    BankCheck();
    curromsize = ROM[infoloc+ROMSizeOffset];
    chip_detect();
    SetupSramSize();
    calculate_state_sizes();
    InitRewindVars();
  }
}

//Memory Setup functions
extern unsigned char wramdataa[65536];
extern unsigned char ram7fa[65536];
extern unsigned char regptra[49152];
extern unsigned char regptwa[49152];
extern unsigned char vidmemch2[4096];
extern unsigned char vidmemch4[4096];
extern unsigned char vidmemch8[4096];
extern unsigned char pal16b[1024];
extern unsigned char pal16bcl[1024];
extern unsigned char pal16bclha[1024];
extern unsigned char pal16bxcl[256];
extern unsigned char SPCRAM[65472];

extern unsigned char *sram;
extern unsigned char *vidbuffer;
extern unsigned char *vram;
extern unsigned char *vcache2b;
extern unsigned char *vcache4b;
extern unsigned char *vcache8b;
#ifdef OLD_DEBUGGER
extern unsigned char *debugbuf;
#endif
extern unsigned char *sram;

void clearSPCRAM()
{
  /*
  SPC RAM is filled with alternating 0x00 and 0xFF for 0x20 bytes.

  Basically the SPCRAM is initialized as follows:
  xx00 - xx1f: $00
  xx20 - xx3f: $ff
  xx40 - xx5f: $00
  xx60 - xx7f: $ff
  xx80 - xx9f: $00
  xxa0 - xxbf: $ff
  xxc0 - xxdf: $00
  xxe0 - xxff: $ff
  */
  unsigned int i;
  for (i = 0; i < 65472; i += 0x40)
  {
    memset(SPCRAM+i, 0, 0x20);
    memset(SPCRAM+i+0x20, 0xFF, 0x20);
  }
}

void clearmem2()
{
  memset(sram, 0xFF, 65536);
  clearSPCRAM();
}

void clearmem()
{
  int i;

  memset(vidbuffer, 0, 131072);
  memset(wramdataa, 0, 65536);
  memset(ram7fa, 0, 65536);
  memset(vram, 0, 65536);
  memset(sram, 0, 65536*2);
#ifdef OLD_DEBUGGER
  memset(debugbuf, 0, 80000);
#endif
  memset(regptra, 0, 49152);
  memset(regptwa, 0, 49152);
  memset(vcache2b, 0, 262144+256);
  memset(vcache4b, 0, 131072+256);
  memset(vcache8b, 0, 65536+256);
  memset(vidmemch2, 0, 4096);
  memset(vidmemch4, 0, 4096);
  memset(vidmemch8, 0, 4096);
  memset(pal16b, 0, 1024);
  memset(pal16bcl, 0, 1024);
  memset(pal16bclha, 0, 1024);
  for (i=0 ; i<1024 ; i+=4)
  {
    memset(pal16bxcl+i, 255, 2);
    memset(pal16bxcl+i+2, 0, 2);
  }
  memset(romdata, 0xFF, maxromspace+32768);
  clearmem2();
}

extern unsigned char BRRBuffer[32];
extern unsigned char echoon0;
extern unsigned int PHdspsave;
extern unsigned int PHdspsave2;
unsigned char echobuf[90000];
extern unsigned char *spcBuffera;
extern unsigned char DSPMem[256];

void clearvidsound()
{
  memset(vram, 0, 65536);
  memset(vidmemch2, 0, 4096);
  memset(vidmemch4, 0, 4096);
  memset(vidmemch8, 0, 4096);
  memset(BRRBuffer, 0, PHdspsave);
  memset(&echoon0, 0, PHdspsave2);
  memset(echobuf, 0, 90000);
  memset(spcBuffera, 0, 65536*4+4096);
  memset(DSPMem, 0, 256);
}

/*

--------------Caution Hack City--------------

Would be nice to trash this section in the future
*/

extern unsigned char ENVDisable, cycpb268, cycpb358, cycpbl2, cycpblt2, cycpbl;
extern unsigned char cycpblt, opexec268, opexec358, opexec268b, opexec358b;
extern unsigned char opexec268cph, opexec358cph, opexec268cphb, opexec358cphb;
bool HacksDisable;

void headerhack()
{
  char *RomData = (char *)romdata;
  ENVDisable = 0;

  if (curromspace < Lo || HacksDisable) { return; }

  //Super Famista (J)
  //Shows black screen after one screen.
  if (!strncmp((RomData+Lo),"\xbd\xb0\xca\xdf\xb0\xcc\xa7\xd0\xbd\xc0  ", 12))
  {
    RomData[0x2762F] = 0xEA;
    RomData[0x27630] = 0xEA;
  }

  //Super Famista 2 (J)
  //Shows black screen after loading the ROM.
  if (!strncmp((RomData+Lo),"\xbd\xb0\xca\xdf\xb0\xcc\xa7\xd0\xbd\xc0 2", 12))
  {
    //Skip a check for value FF at 2140 when spc not initialized yet?!?
    RomData[0x6CED] = 0xEA;
    RomData[0x6CEE] = 0xEA;
    //Skip a check for value FF at 2140 when spc not initialized yet?!?
    RomData[0x6CF9] = 0xEA;
    RomData[0x6CFA] = 0xEA;
  }

  //Deae Tonosama Appare Ichiban (J)
  //Shows some screen and hangs there.
  if (!strncmp((RomData+Lo),"\xc3\xde\xb1\xb4\xc4\xc9\xbb\xcf", 8))
  {
    RomData[0x17837C] = 0xEA;
    RomData[0x17837D] = 0xEA;
  }

  //Human Grand Prix III - F1 Triple Battle (J)
  //Shows black screen after loading the ROM.
  if (!strncmp((RomData+Lo),"HUMAN GRANDPRIX 3   ", 20))
  {
    cycpb268 = 135;
    cycpb358 = 157;
    cycpbl2  = 125;
    cycpblt2 = 125;
    cycpbl   = 125;
    cycpblt  = 125;
  }

  //Accele Brid (J)
  //Hangs after some time in the first level.
  if (!strncmp((RomData+Lo),"ACCELEBRID  ", 12))
  {
    RomData[0x34DA2] = 0;
    RomData[0x34DA3] = 0;
  }

  //Home Alone (J/E/U)
  //Hangs after starting a new game.
  if (!strncmp((RomData+Lo),"HOME ALONE  ", 12))
  {
    RomData[0x666B] = 0xEE;
    RomData[0x666C] = 0xBC;
  }

  //Emerald Dragon (J)
  //Hangs while drawing the logo after loading the ROM.
  if (!strncmp((RomData+Hi),"EMERALD DRAG", 12))
  {
    ENVDisable = true;
  }

  //Rendering Ranger R2
  //Shows black screen after loading the ROM.
  if (!strncmp((RomData+Lo),"REND", 4))
  {
    cycpb268 = 157;
    cycpb358 = 157;
    cycpbl2  = 157;
    cycpblt2 = 157;
    cycpbl   = 157;
    cycpblt  = 157;
  }

  //Tuff E Nuff (U/E), Dead Dance (J),
  //Cyber Knight II - Tikyu Teikoku no Yabou (J)
  //Shows black screen after loading the ROM. (Tuff E Nuff, Dead Dance)
  //Shows black screen after two screens. (Cyber Knight II)
  if (!strncmp((RomData+Lo),"CYBER KNIGHT 2  ", 16) ||
      !strncmp((RomData+Lo),"DEAD", 4) ||
      !strncmp((RomData+Lo),"TUFF", 4))
  {
    cycpb268 = 75;
    cycpb358 = 77;
    cycpbl2  = 75;
    cycpblt2 = 75;
    cycpbl   = 75;
    cycpblt  = 75;
  }

  //Addams Family Values (U/E)
  //Restarts or shows a black screen after starting a new game.
  if (!strncmp((RomData+Lo), "ADDAMS FAMILY VALUES", 20))
  {
    opexec268 = 120;
    opexec358 = 100;
  }

  //Front Mission
  //Flickering worldmap and statusbar.
  if (!strncmp((RomData+Hi), "\xcc\xdb\xdd\xc4\xd0\xaf\xbc\xae", 8) ||
      !strncmp((RomData+Hi), "FRONT MI", 8))
  {
    opexec268 = 226;
    opexec358 = 226;
    opexec268cph = 80;
    opexec358cph = 80;
  }
}

void Setper2exec()
{
  if (per2exec != 100)
  { // Decrease standard % of execution by 5% to replace branch and 16bit
    // cycle deductions
    opexec268b = (unsigned char)((opexec268 * 95 * per2exec) / 10000);
    opexec358b = (unsigned char)((opexec358 * 87 * per2exec) / 10000); // 82
    opexec268cphb = (unsigned char)((opexec268cph * 95 * per2exec) / 10000);
    opexec358cphb = (unsigned char)((opexec358cph * 87 * per2exec) / 10000); // 82
  }
}

extern unsigned int SPC7110TempPosition, SPC7110TempLength, SPCDecmPtr;
static char *SPC7110path, SPC7110fname[8+1+6+4+1]; //dir / 12345 .bin
char *SPC7110filep;
extern unsigned char *SPC7110IndexPtr, *SPC7110PackPtr;
unsigned int SPC7110IndexSize;

static void SPC7PathSetup(char *PathVar, const char *Default)
{
  if (*PathVar)
  {
    SPC7110path = PathVar;
    strcatslash(SPC7110path);
    *SPC7110fname = 0;
  }
  else
  {
    SPC7110path = ZSramPath;
    strcpy(SPC7110fname, Default);
    strcatslash(SPC7110fname);
  }
}

extern unsigned int MsgCount, MessageOn, CRC32;
extern char *Msgptr;
unsigned int SPC7110Entries;

void SPC7PackIndexLoad()
{
  char *ROM = (char *)romdata;
  FILE *fp = 0;
  SPC7110IndexSize = 0;

  //Get correct path for the ROM we just loaded
  if (!strncmp(ROM+infoloc, "HU TENGAI MAKYO ZERO ", 21))
  { SPC7PathSetup(FEOEZPath, "FEOEZSP7"); }
  else if (!strncmp(ROM+infoloc, "JUMP TENGAIMAKYO ZERO", 21))
  { SPC7PathSetup(SJNSPath, "SJNS-SP7"); }
  else if (!strncmp(ROM+infoloc, "MOMOTETSU HAPPY      ", 21))
  { SPC7PathSetup(MDHPath, "MDH-SP7"); }
  else if (!strncmp(ROM+infoloc, "SUPER POWER LEAG 4   ", 21))
  { SPC7PathSetup(SPL4Path, "SPL4-SP7"); }
  else
  {
    Msgptr = "DECOMPRESSED PACK NOT FOUND";
    MessageOn = 360;
    return;
  }

  //Set the pointer to after the slash - needed for the case converters
  SPC7110filep = SPC7110fname+strlen(SPC7110fname);

  //Index file;
  strcat(SPC7110fname, "index.bin");

  //Load the index
  fp = fopen_dir(SPC7110path, SPC7110fname, "rb");
  if (fp)
  {
    SPC7110IndexSize = fread(SPC7110IndexPtr, 1, 12*4608, fp);
    fclose(fp);

    //Get file pointer ready for individual pack files
    strcpy(SPC7110filep, "123456.bin"); //Extension Lower Case
  }

  SPC7110Entries = 0;

  if (!SPC7110IndexSize)
  {
    Msgptr = "DECOMPRESSED PACK NOT FOUND";
    MessageOn = 360;
  }
}

void SPC7_Convert_Upper()
{
  char *i = SPC7110filep;
  while (*i)
  {
    *i = toupper(*i); //To make extension Upper case
    i++;
  }
}

void SPC7_Convert_Lower()
{
  char *i = SPC7110filep;
  while (*i)
  {
    *i = tolower(*i); //To make everything Lower case
    i++;
  }
}

void SPC7_Data_Load()
{
  FILE *fp = fopen_dir(SPC7110path, SPC7110fname, "rb");
  if (!fp)
  {
    SPC7_Convert_Upper();
    fp = fopen_dir(SPC7110path, SPC7110fname, "rb");

    if (!fp)
    {
      SPC7_Convert_Lower();
      fp = fopen_dir(SPC7110path, SPC7110fname, "rb");
    }
  }

  if (fp)
  {
    fseek(fp, SPC7110TempPosition, SEEK_SET);
    fread(SPC7110PackPtr, 1, SPC7110TempLength, fp);
    fclose(fp);
  }
}

extern unsigned char IPSPatched;

unsigned int showinfogui()
{
  unsigned int i;
  unsigned char *ROM = (unsigned char *)romdata;

  strcpy(CSStatus, "                          TYPE:         ");
  strcpy(CSStatus2, "INTERLEAVED:                 CHKSUM:    ");
  strcpy(CSStatus3, "VIDEO:        BANK:       CRC32:        ");
  strcpy(CSStatus4, "                                        ");

  for (i=0 ; i<21 ; i++)
  { CSStatus[i] = (ROM[infoloc + i]) ? ROM[infoloc + i] : 32; }

  if (Interleaved)
  {
    memcpy(CSStatus2+12, "Yes ", 4);
    memcpy(CSStatus4+10, "PLEASE DEINTERLEAVE ROM", 23);
  }
  else
  {
    memcpy(CSStatus2+12, "No  ", 4);
    memset(CSStatus4+10, ' ', 23);
  }

  memcpy(CSStatus2+20, (IPSPatched) ? "IPS ":"    ", 4);
  memcpy(CSStatus3+6, (ROM[infoloc + 25] < 2 || ROM[infoloc + 25] > 12) ? "NTSC":"PAL ", 4);

  if (infoloc == EHi) { memcpy(CSStatus3+19, "EHi ", 4); }
  else { memcpy(CSStatus3+19, (romtype == 2) ? "Hi  ":"Lo  ", 4); }

  memcpy(CSStatus+31, "NORMAL   ", 9);
  if (SA1Enable)     { memcpy(CSStatus+31, "SA-1     ", 9); }
  if (RTCEnable)     { memcpy(CSStatus+31, "RTC      ", 9); }
  if (SPC7110Enable) { memcpy(CSStatus+31, "SPC7110  ", 9); }
  if (SFXEnable)     { memcpy(CSStatus+31, "SUPER FX ", 9); }
  if (C4Enable)      { memcpy(CSStatus+31, "C4       ", 9); }
  if (DSP1Enable)    { memcpy(CSStatus+31, "DSP-1    ", 9); }
  if (DSP2Enable)    { memcpy(CSStatus+31, "DSP-2    ", 9); }
  if (DSP3Enable)    { memcpy(CSStatus+31, "DSP-3    ", 9); }
  if (DSP4Enable)    { memcpy(CSStatus+31, "DSP-4    ", 9); }
  if (SDD1Enable)    { memcpy(CSStatus+31, "S-DD1    ", 9); }
  if (OBCEnable)     { memcpy(CSStatus+31, "OBC1     ", 9); }
  if (SETAEnable)    { memcpy(CSStatus+31, "SETA DSP ", 9); }
  if (ST18Enable)    { memcpy(CSStatus+31, "ST018    ", 9); }
  if (SGBEnable)     { memcpy(CSStatus+31, "SGB      ", 9); }
  if (BSEnable)      { memcpy(CSStatus+31, "BROADCAST", 9);
  // dummy out date so CRC32 matches
    ROM[infoloc+BSMonthOffset] = 0x42;
    ROM[infoloc+BSDayOffset] = 0x00; }
  // 42 is the answer, and the uCONSRT standard

  // calculate CRC32 for the whole ROM, or Add-on ROM only
  CRC32 = (SplittedROM) ? crc32(0, ROM+addOnStart, addOnSize) : crc32(0, ROM, NumofBytes);
  // place CRC32 on line
  sprintf(CSStatus3+32, "%08X", CRC32);

  i = (SplittedROM) ? infoloc + 0x1E + addOnStart: infoloc + 0x1E;

  if ((ROM[i] == (Checksumvalue & 0xFF)) && (ROM[i+1] == (Checksumvalue >> 8)))
  { memcpy(CSStatus2+36, "OK  ", 4); }
  else
  {
    memcpy(CSStatus2+36, "FAIL", 4);
    if (!IPSPatched) { memcpy(CSStatus4, "BAD ROM ",8); }
    else { memset(CSStatus4, ' ', 7); }
  }

  DumpROMLoadInfo();

  MessageOn = 300;
  Msgptr = CSStatus;
  return (MsgCount);
}

extern unsigned int nmiprevaddrl, nmiprevaddrh, nmirept, nmiprevline, nmistatus;
extern unsigned char spcnumread, yesoutofmemory;
extern unsigned char NextLineCache, sramsavedis, sndrot, regsbackup[3019];
extern unsigned int Voice0Freq, Voice1Freq, Voice2Freq, Voice3Freq;
extern unsigned int Voice4Freq, Voice5Freq, Voice6Freq, Voice7Freq;
extern unsigned int dspPAdj;
extern unsigned short Voice0Pitch, Voice1Pitch, Voice2Pitch, Voice3Pitch;
extern unsigned short Voice4Pitch, Voice5Pitch, Voice6Pitch, Voice7Pitch;
void outofmemfix(), GUIDoReset();

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

extern unsigned int SfxR1, SfxR2, SetaCmdEnable, SfxSFR, SfxSCMR;
extern unsigned char disablespcclr, *sfxramdata, SramExists;
extern unsigned char *setaramdata, *wramdata, *SA1RAMArea, cbitmode;
extern unsigned char ForcePal, ForceROMTiming, romispal, MovieWaiting, DSP1Type;
extern unsigned short totlines;
void SetAddressingModes(), GenerateBank0Table();
void SetAddressingModesSA1(), GenerateBank0TableSA1();
void InitDSP(), InitDSP2(), InitDSP3(), InitDSP4(), InitOBC1(), InitFxTables();
void initregr(), initregw();

#ifdef __MSDOS__
void dosmakepal();
#endif

void CheckROMType()
{
  unsigned char *ROM = (unsigned char *)romdata;

  if (!MovieWaiting)
  {
    MirrorROM((unsigned char *)romdata);
    CalcChecksum();
  }

  lorommapmode2 = 0;
  if (!strncmp((char *)ROM+0x207FC0, "DERBY STALLION 96", 17) ||
      !strncmp((char *)ROM+Lo, "SOUND NOVEL-TCOOL", 17))
  { lorommapmode2 = 1; }

  // Setup memmapping
  SetAddressingModes();
  GenerateBank0Table();

  disablespcclr = (memcmp(ROM+Hi, "BS Z", 4)) ? 0 : 1;

  if (!strncmp((char *)ROM, "BANDAI SFC-ADX", 14))
  {
    map_mem(0x60, &stbanka, 0x08);
    if (STCart2)
    {
      map_mem(0x70, &stbankb, 0x08);
    }
  }
  else if (romtype == 1)   // LoROM SRAM mapping
  {  // banks 70 - 77
    map_mem(0x70, &srambank, 0x08);

    if (!BSEnable)
    {  // banks 78 - 7D (not for BS)
      map_mem(0x78, &srambank, 0x06);
    }

    if (!SDD1Enable)
    {  // banks F0 - FF (not for S-DD1)
      map_mem(0xF0, &srambank, 0x10);
    }
  }

  // Setup DSP-X stuff
  DSP1Type = 0;

  if (DSP1Enable)
  {
    InitDSP();

    if (romtype == 1)
    {
      DSP1Type = 1;
      map_mem(0x30, &dsp1bank, 0x10);
      map_mem(0xB0, &dsp1bank, 0x10);
      map_mem(0xE0, &dsp1bank, 0x10);
    }
    else
    {
      DSP1Type = 2;
    }
  }

  if (DSP2Enable)
  {
    asm_call(InitDSP2);
    map_mem(0x3F, &dsp2bank, 1);
  }

  if (DSP3Enable)
  {
    InitDSP3();

    // DSP-3 mapping, banks 20 - 3F
    map_mem(0x20, &dsp3bank, 0x20);
  }

  if (DSP4Enable)
  {
    InitDSP4();

    // DSP-4 mapping, banks 30 - 3F
    map_mem(0x30, &dsp4bank, 0x10);
  }

  if (OBCEnable)
  {
    InitOBC1();

    map_mem(0x00, &obc1bank, 0x40);
    map_mem(0x80, &obc1bank, 0x40);
  }

  if (C4Enable)
  {
    map_mem(0x00, &c4bank, 0x40);
    map_mem(0x80, &c4bank, 0x40);
  }

  if (SFXEnable)
  {
    // Setup SuperFX stuff
    if (maxromspace >= 0x600000)
    {
      //SuperFX mapping, banks 70 - 73
      map_mem(0x70, &sfxbank, 1);
      map_mem(0x71, &sfxbankb, 1);
      map_mem(0x72, &sfxbankc, 1);
      map_mem(0x73, &sfxbankd, 1);

      //SRAM mapping, banks 78 - 79
      map_mem(0x78, &sramsbank, 2);

      SfxR1 = 0;
      SfxR2 = 0;
      memset(sfxramdata, 0, 262144); // clear 256kB SFX ram

      if (SramExists)
      {
        memcpy(sfxramdata, sram, 65536); // proper SFX sram area
      }

      asm_call(InitFxTables);
    }
    else
    {
      yesoutofmemory = 1;
    }
  }

  if (SETAEnable)
  {
    if (strncmp((char *)ROM+Lo, "2DAN MORITA SHOUGI", 18))
    {
      //Setup Seta 10 stuff

      //Really banks 68h-6Fh:0000-7FFF are all mapped the same by the chip but
      //F1 ROC II only uses bank 68h
      map_mem(0x68, &setabank, 1);

      //Control register (and some status?) is in banks 60h-67h:0000-3FFF
      map_mem(0x60, &setabanka, 1);

      SetaCmdEnable = 0x00000080; // 60:0000
    }
    else
    {
      void ST011_Reset();
      ST011_Reset();
      map_mem(0x68, &seta11bank, 1);
      map_mem(0x60, &seta11banka, 1);
    }
    memset(setaramdata, 0, 4096); // clear 4kB SETA ram

    // proper SETA sram area
    if (SramExists)
    {
      memcpy(setaramdata, sram, 4096);
    }
  }

  // General stuff all mixed together [... wouldn't it be cool to clean that]
  SfxSFR = 0;
  SfxSCMR &= 0xFFFFFF00;
  asm_call(initregr);
  asm_call(initregw);

  if (SA1Enable)
  {
    SA1RAMArea = ROM + 4096*1024;

    GenerateBank0TableSA1();
    SetAddressingModesSA1();

    if (CHIPBATT) // proper SA-1 sram area
    {
      memset(SA1RAMArea, 0, 131072);
      if (SramExists) { memcpy(SA1RAMArea, sram, 131072); }
    }
  }

  wramdata = wramdataa;
}

extern unsigned short copv, brkv, abortv, nmiv, nmiv2, irqv, irqv2, resetv;
extern unsigned short copv8, brkv8, abortv8, nmiv8, irqv8;

void SetIRQVectors()
{ // get vectors (NMI & reset)
  unsigned char *ROM = (unsigned char *)romdata;

  if (!memcmp(ROM+infoloc+36+24, "\0xFF\0xFF", 2)) // if reset error
  {
    memcpy(ROM+infoloc+36+6, "\0x9C\0xFF", 2);
    memcpy(ROM+infoloc+36+24, "\0x80\0xFF", 2);
  }

  memcpy(&copv,   ROM+infoloc+0x24, 2);
  memcpy(&brkv,   ROM+infoloc+0x26, 2);
  memcpy(&abortv, ROM+infoloc+0x28, 2);
  memcpy(&nmiv,   ROM+infoloc+0x2A, 2);
  memcpy(&nmiv2,  ROM+infoloc+0x2A, 2);
  memcpy(&irqv,   ROM+infoloc+0x2E, 2);
  memcpy(&irqv2,  ROM+infoloc+0x2E, 2);

  // 8-bit and reset
  memcpy(&copv8,   ROM+infoloc+0x34, 2);
  memcpy(&abortv8, ROM+infoloc+0x38, 2);
  memcpy(&nmiv8,   ROM+infoloc+0x3A, 2);
  memcpy(&resetv,  ROM+infoloc+0x3C, 2);
  memcpy(&brkv8,   ROM+infoloc+0x3E, 2);
  memcpy(&irqv8,   ROM+infoloc+0x3E, 2);

  if (yesoutofmemory) // failed ?
  {
    resetv = 0x8000;
    memcpy(ROM+0x0000, "\0x80\0xFE", 2);
    memcpy(ROM+0x8000, "\0x80\0xFE", 2);
  }
}

void SetupROM()
{
  static bool CLforce = false;
  unsigned char *ROM = (unsigned char *)romdata;

  CheckROMType();
  SetIRQVectors();

  #ifdef __MSDOS__
  if (!cbitmode) // 8-bit mode uses a palette
  {
    asm_call(dosmakepal);
  }
  #endif

  /* get timing (pal/ntsc)
  ForceROMTiming is from the GUI.
  ForcePal is from Command line, we have a static var
  to prevent forcing a secong game loaded from the GUI when
  the first was loaded from the command line with forcing.
  */
  if (ForcePal && !CLforce) { CLforce = true; }
  else { ForcePal = ForceROMTiming; }

  switch (ForcePal)
  {
    case 1:
      romispal = 0;
      break;
    case 2:
      romispal = (!BSEnable);
      break;
    default:
      romispal = ((!BSEnable) && (ROM[infoloc+CountryOffset] > 1) && (ROM[infoloc+CountryOffset] < 0xD));
  }

  #ifdef __UNIXSDL__
  InitSampleControl();
  #endif

  if (romispal)
  {
    totlines = 314;
    MsgCount = 100;
  }
  else
  {
    totlines = 263;
    MsgCount = 120;
  }
}

extern int NumComboLocl;
extern unsigned char ComboHeader[23];
extern char CombinDataLocl[3300];
extern bool romloadskip;

void SaveCombFile()
{
  if (!romloadskip)
  {
    FILE *fp;

    setextension(ZSaveName, "cmb");

    if (NumComboLocl)
    {
      ComboHeader[22] = NumComboLocl;

      if ((fp = fopen_dir(ZSramPath, ZSaveName, "wb")))
      {
        fwrite(ComboHeader, 1, 23, fp);
        fwrite(CombinDataLocl, 1, NumComboLocl*66, fp);
        fclose(fp);
      }
    }
  }
}

void OpenCombFile()
{
  FILE *fp;

  setextension(ZSaveName, "cmb");
  NumComboLocl = 0;

  if ((fp = fopen_dir(ZSramPath, ZSaveName, "rb")))
  {
    fread(ComboHeader, 1, 23, fp);
    NumComboLocl = ComboHeader[22];

    if (NumComboLocl)
    {
      fread(CombinDataLocl, 1, NumComboLocl*66, fp);
    }

    fclose(fp);
  }
}

unsigned char SFXCounter, SfxAC, ForceNewGfxOff;

void preparesfx()
{
  char *ROM = (char *)romdata, i;

  SFXCounter = SfxAC = 0;

  if (!strncmp(ROM+Lo, "FX S", 4) ||
      !strncmp(ROM+Lo, "DIRT", 4))
  {
    SFXCounter = 1;
  }
  else if (!strncmp(ROM+Lo, "Stun", 4))
  {
    ForceNewGfxOff=1;
  }

  for (i=63;i>=0;i--)
  {
    memcpy(romdata+i*0x4000       ,romdata+i*0x2000,0x8000);
    memcpy(romdata+i*0x4000+0x2000,romdata+i*0x2000,0x8000);
  }
}

void map_set(void **dest, unsigned char *src, size_t count, size_t step)
{
  while (count--)
  {
    *dest = src;
    dest++;
    src += step;
  }
}

extern unsigned char MultiType;
extern void *snesmmap[256];
extern void *snesmap2[256];

unsigned int cromptradd;
extern unsigned char MultiTap;
extern unsigned int SfxR0, SfxR1, SfxR2, SfxR3, SfxR4, SfxR5, SfxR6, SfxR7,
                   SfxR8, SfxR9, SfxR10, SfxR11, SfxR12, SfxR13, SfxR14, SfxR15;
extern void *ram7f;

void map_lorom()
{
  unsigned char *ROM = (unsigned char *)romdata;
  int x;

  // set addresses 8000-FFFF
  // set banks 00-7F (80h x 32KB ROM banks @ 8000h)
  map_set(snesmmap,ROM-0x8000,0x80,0x8000);

  // set banks 80-FF (80h x 32KB ROM banks @ 8000h)
  if (lorommapmode2)
  {
    map_set(snesmmap+0x80,ROM+0x1F8000,0x20,0x8000);
    map_set(snesmmap+0xA0,ROM+0x0F8000,0x60,0x8000);
  }
  else
  {
    map_set(snesmmap+0x80,ROM-0x8000,0x80,0x8000);
  }

  // set addresses 0000-7FFF
  // set banks 00-3F (40h x WRAM)
  map_set(snesmap2,wramdata,0x40,0);

  // set banks 40-7F (40h x 32KB ROM banks @ 8000h)
  map_set(snesmap2+0x40,ROM+0x200000,0x40,0x8000);

  // set banks 80-BF (40h x WRAM)
  map_set(snesmap2+0x80,wramdata,0x40,0);

  // set banks C0-FF (40h x 32KB ROM banks @ 8000h)
  map_set(snesmap2+0xC0,ROM+0x200000,0x40,0x8000);

  // set banks 70-77 (07h x SRAM)
  for (x = 0x70; x <= 0x77; x++) { snesmap2[x] = sram; }

  // set banks 7E/7F (WRAM)
  snesmmap[0x7E] = snesmap2[0x7E] = wramdata;
  snesmmap[0x7F] = snesmap2[0x7F] = ram7f;
}

void map_hirom()
{
  unsigned char *ROM = (unsigned char *)romdata;
  int x;

  // set addresses 8000-FFFF
  // set banks 00-3F (40h x 64KB ROM banks @10000h)
  map_set(snesmmap,ROM,0x40,0x10000);

  // set banks 40-7F (40h x 64KB ROM banks @10000h)
  map_set(snesmmap+0x40,ROM,0x40,0x10000);

  // set banks 80-BF (40h x 64KB ROM banks @10000h)
  map_set(snesmmap+0x80,ROM,0x40,0x10000);

  // set banks C0-FF (40h x 64KB ROM banks @10000h)
  map_set(snesmmap+0xC0,ROM,0x40,0x10000);

  // set addresses 0000-7FFF
  // set banks 00-3F (40h x WRAM)
  map_set(snesmap2,wramdata,0x40,0);

  // set banks 40-7F (40h x 64KB ROM banks @10000h)
  map_set(snesmap2+0x40,ROM,0x40,0x10000);

  // set banks 80-BF (40h x WRAM)
  map_set(snesmap2+0x80,wramdata,0x40,0);

  // set banks C0-FF (40h x 64KB ROM banks @10000h)
  map_set(snesmap2+0xC0,ROM,0x40,0x10000);

  // set banks 70-77 (07h x SRAM)
  for (x = 0x70; x <= 0x77; x++) { snesmap2[x] = sram; }

  // set banks 7E/7F (WRAM)
  snesmmap[0x7E] = snesmap2[0x7E] = wramdata;
  snesmmap[0x7F] = snesmap2[0x7F] = ram7f;
}

void map_ehirom()
{
  unsigned char *ROM = (unsigned char *)romdata;
  int x;

  // set addresses 8000-FFFF
  // set banks 00-3F (40h x 32KB ROM banks @ 10000h)
  map_set(snesmmap,ROM+0x400000,0x20,0x10000);
  map_set(snesmmap+0x20,ROM+0x400000,0x20,0x10000);

  // set banks 40-7F (40h x 32KB ROM banks @ 10000h)
  map_set(snesmmap+0x40,ROM+0x400000,0x20,0x10000);
  map_set(snesmmap+0x60,ROM+0x400000,0x20,0x10000);

  // set banks 80-BF (40h x 32KB ROM banks @10000h)
  map_set(snesmmap+0x80,ROM+0x400000,0x20,0x10000);
  map_set(snesmmap+0xA0,ROM+0x400000,0x20,0x10000);

  // set banks C0-FF (40h x 64KB ROM banks @10000h)
  map_set(snesmmap+0xC0,ROM,0x40,0x10000);

  // set addresses 0000-7FFF
  // set banks 00-3F (40h x WRAM)
  map_set(snesmap2,wramdata,0x40,0);

  // set banks 40-7F (40h x 32KB ROM banks @ 8000h)
  map_set(snesmap2+0x40,ROM+0x400000,0x20,0x10000);
  map_set(snesmap2+0x60,ROM+0x400000,0x20,0x10000);

  // set banks 80-BF (40h x WRAM)
  map_set(snesmap2+0x80,wramdata,0x40,0);

  // set banks C0-FF (40h x 64KB ROM banks @10000h)
  map_set(snesmap2+0xC0,ROM,0x40,0x10000);

  // set banks 70-77 (07h x SRAM)
  for(x = 0x70; x <= 0x77; x++) { snesmap2[x] = sram; }

  // set banks 7E/7F (WRAM)
  snesmmap[0x7E] = snesmap2[0x7E] = wramdata;
  snesmmap[0x7F] = snesmap2[0x7F] = ram7f;
}

void map_sfx()
{
  unsigned char *ROM = (unsigned char *)romdata;

  // Clear SFX registers
  SfxR0 = SfxR1 = SfxR2 = SfxR3 = SfxR4 = SfxR5 = SfxR6 = SfxR7 = 0;
  SfxR8 = SfxR9 = SfxR10 = SfxR11 = SfxR12 = SfxR13 = SfxR14 = SfxR15 = 0;

  // set addresses 8000-FFFF
  // set banks 00-3F (40h x 64KB ROM banks @10000h)
  map_set(snesmmap,ROM,0x40,0x10000);

  // set banks 40-7F (40h x128KB ROM banks @20000h)
  map_set(snesmmap+0x40,ROM+0x8000,0x30,0x20000);
  map_set(snesmmap+0x70,ROM+0x8000,0x10,0x20000);

  // set banks 80-BF (40h x 64KB ROM banks @10000h)
  map_set(snesmmap+0x80,ROM,0x40,0x10000);

  // set banks C0-FF (40h x128KB ROM banks @20000h)
  map_set(snesmmap+0xC0,ROM+0x8000,0x30,0x20000);
  map_set(snesmmap+0xF0,ROM+0x8000,0x10,0x20000);

  // set addresses 0000-7FFF
  // set banks 00-3F (40h x WRAM)
  map_set(snesmap2,wramdata,0x40,0);

  // set banks 40-7F (40h x128KB ROM banks @20000h)
  map_set(snesmap2+0x40,ROM+0x8000,0x30,0x20000);
  map_set(snesmap2+0x70,ROM+0x8000,0x10,0x20000);

  // set banks 80-BF (40h x WRAM)
  map_set(snesmap2+0x80,wramdata,0x40,0);

  // set banks C0-FF (40h x128KB ROM banks @20000h)
  map_set(snesmap2+0xC0,ROM+0x8000,0x30,0x20000);
  map_set(snesmap2+0xF0,ROM+0x8000,0x10,0x20000);

  // set banks 70-73/78/79 (SFXRAM & SRAM)
  map_set(snesmap2+0x70,sfxramdata,4,0x10000);
  snesmap2[0x78] = snesmap2[0x79] = sram;

  // set banks 7E/7F (WRAM)
  snesmmap[0x7E] = snesmap2[0x7E] = wramdata;
  snesmmap[0x7F] = snesmap2[0x7F] = ram7f;

  preparesfx();
}

void map_sa1()
{
  unsigned char *ROM = (unsigned char *)romdata;
  unsigned char test[] = { 0xA9, 0x10, 0xCF, 0xAD };

  if(!memcmp(ROM+0xB95, test, 4)) { ROM[0xB96] = 0; }

  // set addresses 8000-FFFF
  // set banks 00-3F (40h x 32KB ROM banks @ 8000h)
  map_set(snesmmap,ROM-0x8000,0x40,0x8000);

  // set banks 40-7F (40h x 64KB ROM banks @10000h)
  map_set(snesmmap+0x40,ROM,0x40,0x10000);

  // set banks 80-BF (40h x 32KB ROM banks @ 8000h)
  map_set(snesmmap+0x80,ROM+0x1F8000,0x40,0x8000);

  // set banks C0-FF (40h x 64KB ROM banks @10000h)
  map_set(snesmmap+0xC0,ROM,0x40,0x10000);

  // set addresses 0000-7FFF
  // set banks 00-3F (40h x WRAM)
  map_set(snesmap2,wramdata,0x40,0);

  // set banks 40-7F (40h x 64KB ROM banks @10000h)
  map_set(snesmap2+0x40,ROM,0x40,0x10000);

  // set banks 80-BF (40h x WRAM)
  map_set(snesmap2+0x80,wramdata,0x40,0);

  // set banks C0-FF (40h x 64KB ROM banks @10000h)
  map_set(snesmap2+0xC0,ROM,0x40,0x10000);

  // set banks 7E/7F (WRAM)
  snesmmap[0x7E] = snesmap2[0x7E] = wramdata;
  snesmmap[0x7F] = snesmap2[0x7F] = ram7f;
}

void map_sdd1()
{
  unsigned char *ROM = (unsigned char *)romdata;
  unsigned char test[] = { 0xA9, 0x10, 0xCF, 0xAD };

  if(!memcmp(ROM+0xB95, test, 4)) { ROM[0xB96] = 0; }

  // set addresses 8000-FFFF
  // set banks 00-3F (40h x 32KB ROM banks @ 8000h)
  map_set(snesmmap,ROM-0x8000,0x40,0x8000);

  // set banks 40-7F (40h x 64KB ROM banks @10000h)
  map_set(snesmmap+0x40,ROM,0x40,0x10000);

  // set banks 80-BF (40h x 32KB ROM banks @ 8000h)
  map_set(snesmmap+0x80,ROM-0x8000,0x40,0x8000);

  // set banks C0-FF (40h x 64KB ROM banks @10000h)
  map_set(snesmmap+0xC0,ROM,0x40,0x10000);

  // set addresses 0000-7FFF
  // set banks 00-3F (40h x WRAM)
  map_set(snesmap2,wramdata,0x40,0);

  // set banks 40-7F (40h x 64KB ROM banks @10000h)
  map_set(snesmap2+0x40,ROM,0x40,0x10000);

  // set banks 80-BF (40h x WRAM)
  map_set(snesmap2+0x80,wramdata,0x40,0);

  // set banks C0-FF (40h x 64KB ROM banks @10000h)
  map_set(snesmap2+0xC0,ROM,0x40,0x10000);

  // set banks 7E/7F (WRAM)
  snesmmap[0x7E] = snesmap2[0x7E] = wramdata;
  snesmmap[0x7F] = snesmap2[0x7F] = ram7f;
}

void map_bsx()
{
  unsigned char *ROM = (unsigned char *)romdata;
  int x;

  // set addresses 8000-FFFF
  // set banks 00-7F (80h x 32KB ROM banks @ 8000h)
  map_set(snesmmap,ROM-0x8000,0x80,0x8000);

  // set banks 80-BF (40h x 32KB ROM banks @ 8000h)
  map_set(snesmmap+0x80,ROM-0x8000,0x40,0x8000);

  // set banks C0-FF (40h x 32KB ROM banks @ 8000h)
  map_set(snesmmap+0xC0,ROM+0x8000,0x40,0x8000);

  // set addresses 0000-7FFF
  // set banks 00-3F (40h x WRAM)
  map_set(snesmap2,wramdata,0x40,0);

  // set banks 40-7F (40h x 32KB ROM banks @ 8000h)
  map_set(snesmap2+0x40,ROM+0x200000,0x40,0x8000);

  // set banks 80-BF (40h x WRAM)
  map_set(snesmap2+0x80,wramdata,0x40,0);

  // set banks C0-FF (40h x 32KB ROM banks @ 8000h)
  map_set(snesmap2+0xC0,ROM+0x8000,0x40,0x8000);

  // set banks 70-77 (07h x SRAM)
  for (x = 0x70; x <= 0x77; x++) { snesmap2[x] = sram; }

  // set banks 7E/7F (WRAM)
  snesmmap[0x7E] = snesmap2[0x7E] = wramdata;
  snesmmap[0x7F] = snesmap2[0x7F] = ram7f;
}

void initsnes()
{
  ForceNewGfxOff = 0;

  if (!BSEnable)
  {
    MultiTap = pl12s34 ? 0 : (pl3contrl || pl4contrl || pl5contrl);

    if (SFXEnable) { map_sfx(); }
    else if (SA1Enable) { map_sa1(); }
    else if (SDD1Enable) { map_sdd1(); }
    else if (SPC7110Enable) { map_hirom(); }
    else if (curromsize == 13) { map_ehirom(); }
    else if (romtype == 2) { map_hirom(); }
    else { map_lorom(); }
  }
  else
  {
    map_bsx();
  }
}

bool PatchUsingIPS(char *);
void DosExit(), OpenSramFile(), CheatCodeLoad(), LoadSecondState(), LoadGameSpecificInput();
extern unsigned char GUIOn, GUIOn2;

bool loadfileGUI()
{
  bool result = true;

  spcon = !SPCDisable;
  MessageOn = yesoutofmemory = IPSPatched = 0;

  loadROM();

  if (curromspace)
  {
    SramExists = 0;
    OpenSramFile();
    OpenCombFile();
    LoadGameSpecificInput();

    if (!(GUIOn || GUIOn2))
    {
      puts("File opened successfully !");
    }
    if (!IPSPatched)
    {
      int i;
      char ext[4];

      strcpy(ext, "ips");
      for (i = 0; PatchUsingIPS(ext); i++)
      {
        if (i > 9) { break; }
        ext[2] = i+'0';
      }
    }
  }
  else
  {
    if (GUIOn || GUIOn2) { result = false; }
    else
    {
      puts("Error opening file!\n");
      asm_call(DosExit);
    }
  }

  return (result);
}

extern unsigned int CheatOn, NumCheats;
extern unsigned char CheatWinMode, CheatSearchStatus;
void GUIQuickLoadUpdate();

void powercycle(bool sramload, bool romload)
{
  clearmem2();

  nmiprevaddrl = 0;
  nmiprevaddrh = 0;
  nmirept = 0;
  nmiprevline = 224;
  nmistatus = 0;
  spcnumread = 0;
  NextLineCache = 0;
  curexecstate = 1;

  if (sramload) { OpenSramFile(); }
  if (romload) { romloadskip = 1; }

  if (!romload || (loadfileGUI()))
  {
    if (romload)
    { CheatOn = NumCheats = CheatWinMode = CheatSearchStatus = 0; }

    SetupROM();

    if (romload)
    {
      if (DisplayInfo) { showinfogui(); }
      initsnes();
    }

    sramsavedis = 0;
    memcpy(&sndrot, regsbackup, 3019);

    if (yesoutofmemory) { asm_call(outofmemfix); }
    asm_call(GUIDoReset);

    if (romload)
    {
      GUIQuickLoadUpdate();

      if (AutoLoadCht) { CheatCodeLoad(); }
      if (AutoState) { LoadSecondState(); }
    }
  }
}

extern unsigned char osm2dis, ReturnFromSPCStall, SPCStallSetting, prevoamptr;
extern unsigned char reg1read, reg2read, reg3read, reg4read, NMIEnab, INTEnab;
extern unsigned char doirqnext, vidbright, forceblnk, timeron, spcP, JoyAPos, JoyBPos;
extern unsigned char coladdr, coladdg, coladdb;
extern unsigned char SDD1BankA,SDD1BankB, SDD1BankC, SDD1BankD;
extern unsigned char intrset, curcyc, cycpl, GUIReset;
extern unsigned int numspcvblleft, SPC700read, SPC700write, spc700idle;
extern unsigned int FIRTAPVal0, FIRTAPVal1, FIRTAPVal2, FIRTAPVal3, FIRTAPVal4, FIRTAPVal5, FIRTAPVal6, FIRTAPVal7;
extern unsigned int xa, xdb, xpb, xs, xd, xx, xy, scrndis;
extern unsigned short VIRQLoc, resolutn, xpc;
extern unsigned char spcextraram[64], SPCROM[64];
extern unsigned int tableD[256];
unsigned char SPCSkipXtraROM, bgfixer2 = 0, disableeffects = 0;
//This is saved in states
unsigned char cycpl = 0;   // cycles per scanline
unsigned char cycphb = 0;    // cycles per hblank
unsigned char intrset = 0;   // interrupt set
unsigned short curypos = 0;    // current y position
unsigned short stackand = 0x01FF; // value to and stack to keep it from going to the wrong area
unsigned short stackor = 0x0100; // value to or stack to keep it from going to the wrong area

// 65816 registers
unsigned char xp = 0;
unsigned char xe = 0;
unsigned char xirqb = 0;           // which bank the irqs start at
unsigned int Curtableaddr = 0;     // Current table address

void SA1Reset();
void InitC4();
void RTCinit();
void SPC7110init();

void init65816()
{
    unsigned int i;
    osm2dis = 0;
    bgfixer2 = 0;
    if(SA1Enable)
    {
      SA1Reset();
      SetAddressingModesSA1();
    }

    if(C4Enable)
    {
      osm2dis = 1;
      bgfixer2 = 1;
      InitC4();
    }

    if(RTCEnable)
      RTCinit();

    if(SPC7110Enable)
    {
      if (!SPC7110IndexPtr) SPC7110IndexPtr = malloc(12*4608);
      if (!SPC7110PackPtr) SPC7110PackPtr = malloc(65536);

      if (SPC7110IndexPtr && SPC7110PackPtr)
      {
        SPC7PackIndexLoad();

        SPC7110init();
        map_mem(0x50, &SPC7110bank, 1);
        map_mem(0x00, &SPC7110SRAMBank, 1);
        map_mem(0x30, &SPC7110SRAMBank, 1);
        snesmmap[0x50] = SPC7110PackPtr;
        snesmap2[0x50] = SPC7110PackPtr;
        memset(SPC7110PackPtr, 0, 0x10000);
      }
      else
      {
        puts("You don't have enough memory to run SPC7110 games!");
        Msgptr = "MEMORY ERROR!";
        MessageOn = 360;
        return;
      }
    }
    else
    {
      if (SPC7110IndexPtr) free(SPC7110IndexPtr);
      if (SPC7110PackPtr) free(SPC7110PackPtr);
    }

    cycpb268 = 117;
    cycpb358 = 127;
    cycpbl2 = 117;
    cycpblt2 = 117;
    cycpbl = 117;
    cycpblt = 117;

    SPCSkipXtraROM = 0;
    if(ReturnFromSPCStall)
    {
      cycpb268 = 69;
      cycpb358 = 81;
      cycpbl2 = 69;
      cycpblt2 = 69;
      cycpbl = 69;
      cycpblt = 69;
      SPCSkipXtraROM = 1;
      if(SPCStallSetting == 2)
      {
        cycpb268 = 240;
        cycpb358 = 240;
        cycpbl = 240;
        cycpblt = 240;
        cycpbl2 = 240;
        cycpblt2 = 240;
        SPCSkipXtraROM = 0;
      }
    }
    else
    {
      SPCStallSetting = 0;
    }

    numspcvblleft = 480;
    SPC700write = 0;
    SPC700read = 0;
    spc700idle = 0;

    for(i = 0;i<0x40;i++)
    {
      spcextraram[i] = 0xFF;
      SPCRAM[0xFFC0+i] = SPCROM[i];
    }

    // Clear SPC Memory
    clearSPCRAM();
    clearvidsound();

    prevoamptr = 0xFF;
    disableeffects = 0;
    opexec268 = opexec268b;
    opexec358 = opexec358b;
    opexec268cph = opexec268cphb;
    opexec358cph = opexec358cphb;

    if (!(((unsigned char *)romdata)[infoloc+BankOffset] & 0xF0)) // if not fastrom
    {
      opexec358 = opexec268;
      opexec358cph = opexec268cph;
      cycpb358 = cycpb268;
    }

    FIRTAPVal0 = 0x7F;
    FIRTAPVal1 = 0;
    FIRTAPVal2 = 0;
    FIRTAPVal3 = 0;
    FIRTAPVal4 = 0;
    FIRTAPVal5 = 0;
    FIRTAPVal6 = 0;
    FIRTAPVal7 = 0;

    // Check Headers
    headerhack();

    SPCRAM[0xF4] = 0;
    SPCRAM[0xF5] = 0;
    SPCRAM[0xF6] = 0;
    SPCRAM[0xF7] = 0;
    reg1read = 0;
    reg2read = 0;
    reg3read = 0;
    reg4read = 0;
    cycpbl = 0;
    spcnumread = 0;
    NMIEnab = 1;
    VIRQLoc = 0;
    doirqnext = 0;
    resolutn = 224;
    vidbright = 0;
    forceblnk = 0;
    spcP = 0;
    timeron = 0;
    JoyAPos = 0;
    JoyBPos = 0;
    coladdr = 0;
    coladdg = 0;
    coladdb = 0;
    INTEnab = 0;
    xa = 0;
    xdb = 0;
    xpb = 0;
    xs = 0x01FF;
    xd = 0;
    xx = 0;
    xy = 0;
    SDD1BankA = 0;
    SDD1BankB = 0x01;
    SDD1BankC = 0x02;
    SDD1BankD = 0x03;
    xirqb = 0;
    xp = 52;         // NVMXDIZC
    xe = 1;          // E
    xpc = resetv;

    intrset = 0;

    if (xpc < 0x8000)
    {
      xpc += 0x8000;
      //xpb = 0x40;
    }

    // 2.68 Mhz  / 3.58 Mhz = 228
    curcyc = cycpl = opexec268;
    cycphb = opexec268cph;     // 2.68 Mhz  / 3.58 Mhz = 56
    cycpbl = 110;              // 3.58Mhz = 175
    cycpblt = 110;
    curypos = 0;
    Curtableaddr = *tableD;
    scrndis = 0;
    stackand = 0x01FF;
    stackor = 0x0100;

    nmiprevaddrl = 0;
    nmiprevaddrh = 0;
    nmirept = 0;
    nmiprevline = 224;
    nmistatus = 0;

    if(GUIReset)
    {
      GUIReset = 0;
    }

    else
    {
      memset(wramdataa,0x55,0x10000);
      memset(ram7fa, 0x55,0x10000);
    }

    if(BSEnable)
    {
      memset(wramdataa,0xFF,0x10000);
      memset(ram7fa,0xFF,0x10000);
      if(romtype == 1)
        memset(ram7fa+65528, 0x01, 8);
    }
}

void zexit()
{
  exit(0);
}

