/*
Copyright (C) 1997-2006 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

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



#ifdef __UNIXSDL__
#include "gblhdr.h"
#define DIR_SLASH "/"
#else
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#define DIR_SLASH "\\"
#endif
#include "cpu/memtable.h"
#include "zip/zunzip.h"
#include "jma/zsnesjma.h"
#include "asm_call.h"

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

// Some archaic code from an unfinished Dynarec
extern unsigned int curexecstate;
extern unsigned char spcon;

void procexecloop()
{
  curexecstate &= 0xFFFFFF00;

  if (spcon)	{ curexecstate += 3; }
  else	{ curexecstate += 1; }
}

void Debug_WriteString(char *str)
{
  FILE *fp = 0;
  fp = fopen("zsnes.dbg", "w");
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

bool EHiHeader(unsigned char *ROM, int BankLoc)
{
  if (validChecksum(ROM, BankLoc) && ROM[BankLoc+BankOffset] == 53)
  {
    return(true);
  }
  return(false);
}

void SwapData(unsigned int *loc1, unsigned int *loc2, unsigned int amount)
{
  unsigned int temp, i;
  for (i = 0; i < amount; i++)
  {
    temp = loc1[i];
    loc1[i] = loc2[i];
    loc2[i] = temp;
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

//These two functions interleave, yes interleave, because ZSNES has it's large ROM map backwards
void intlv1()
{
  char blocks[256];
  int i, numblocks = NumofBanks/2;
  for (i = 0; i < numblocks; i++)
  {
    blocks[i + numblocks] = i * 2;
    blocks[i] = i * 2 + 1;
  }
  swapBlocks(blocks);
}

//This is a mess, I wish we didn't need this, but it kicks the old asm code
void IntlEHi()
{
  SwapData(romdata, romdata + 0x100000, 0x80000);
  SwapData(romdata + 0x80000, romdata + 0x100000, 0x80000);

  NumofBanks = 64;
  intlv1();
  NumofBanks = 192;
}

//ROM loading functions, which some strangly enough were in guiload.inc
bool AllASCII(unsigned char *b, int size)
{
  int i;
  for (i = 0; i < size; i++)
  {
    if (b[i] < 32 || b[i] > 126)
    {
      return(false);
    }
  }
  return(true);
}

int InfoScore(unsigned char *Buffer)
{
  int score = 0;
  if (validChecksum(Buffer, 0))      { score += 4; }
  if (Buffer[CompanyOffset] == 0x33)            { score += 2; }
  if (!(Buffer[61] & 0x80))          { score -= 4; }
  if ((1 << (Buffer[ROMSizeOffset] - 7)) > 48)  { score -= 1; }
  if (Buffer[CountryOffset] < 14)               { score += 1; }
  if (!AllASCII(Buffer, 20))         { score -= 1; }
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
        loscore += 2;
      case 128: case 156: case 176: case 188: case 252: //BS
        loscore += 1;
        break;
    }
    switch(ROM[Hi + BankOffset])
    {
      case 33: case 49: case 53: case 58:
        hiscore += 2;
      case 128: case 156: case 176: case 188: case 252: //BS
        hiscore += 1;
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
bool CHIPBATT;
bool BSEnable;
bool C4Enable;
bool DSP1Enable;
bool DSP2Enable;
bool DSP3Enable;
bool DSP4Enable;
bool OBCEnable;
bool RTCEnable;
bool SA1Enable;
bool SDD1Enable;
bool SETAEnable; //ST010 & 11
bool SFXEnable;
bool SGBEnable;
bool SPC7110Enable;
bool ST18Enable;

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

void chip_detect()
{
  unsigned char *ROM = (unsigned char *)romdata;

  C4Enable = false;
  RTCEnable = false;
  SA1Enable = false;
  SDD1Enable = false;
  OBCEnable = false;
  CHIPBATT = false;
  SGBEnable = false;
  ST18Enable = false;
  DSP1Enable = false;
  DSP2Enable = false;
  DSP3Enable = false;
  DSP4Enable = false;
  SPC7110Enable = false;
  BSEnable = false;
  SFXEnable = false;
  SETAEnable = false;

  //DSP Family
  if (ROM[infoloc+TypeOffset] == 3)
  {
    if (ROM[infoloc+BankOffset] == 48)
    {
      DSP4Enable = true;
    }
    else
    {
      DSP1Enable = true;
    }
    return;
  }
  if (ROM[infoloc+TypeOffset] == 5)
  {
    CHIPBATT = true;
    if (ROM[infoloc+BankOffset] == 32)
    {
      DSP2Enable = true;
    }
    else if (ROM[infoloc+BankOffset] == 48 && ROM[infoloc+CompanyOffset] == 0xB2) //Bandai
    {
      DSP3Enable = true;
    }
    else
    {
      DSP1Enable = true;
    }
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

extern unsigned short Checksumvalue;
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
    Checksumvalue = sum(ROM, NumofBytes);
    if (NumofBytes == 0x300000) //Fix for 24Mb SPC7110 ROMs
    {
      Checksumvalue += Checksumvalue;
    }
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

//Misc functions
void MirrorROM()
{
  unsigned char *ROM = (unsigned char *)romdata;
  unsigned int size, StartMirror = 0, ROMSize = curromspace;
  //This will mirror up non power of two ROMs to powers of two
  for (size = 1; size <= 64; size +=size)
  {
    unsigned int fullSize = size * Mbit_bytes,
                 halfSize = fullSize >> 1;
    if ((ROMSize > halfSize) && (ROMSize < fullSize))
    {
      for (StartMirror = halfSize;
           ROMSize < fullSize && ROMSize < maxromspace;)
      {
        ROM[ROMSize++] = ROM[StartMirror++];
      }
      curromspace = ROMSize;
      break;
    }
  }
  //This will mirror (now) full sized ROMs through the ROM buffer
  for (StartMirror = 0; ROMSize < maxromspace;)
  {
    ROM[ROMSize++] = ROM[StartMirror++];
  }

  NumofBanks = curromspace >> 15;

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
extern char *ZOpenFileName;
bool Header512;


extern char CSStatus[40];
extern char CSStatus2[40];
extern char CSStatus3[40];
extern bool RomInfo;
char *lastROMFileName;
void DumpROMLoadInfo()
{
  FILE *fp = 0;

  if (RomInfo) //rominfo.txt info dumping enabled?
  {
    fp = fopen("rominfo.txt", "w");
    if (!fp) { return; }
    fputs("This is the info for the last game you ran.\n\nFile: ", fp);
    fputs(lastROMFileName, fp);
    fputs(" Header: ", fp);
    fputs(Header512 ? "Yes\n" : "No\n", fp);
    fputs(CSStatus, fp);
    fputs("\n", fp);
    fputs(CSStatus2, fp);
    fputs("\n", fp);
    fputs(CSStatus3, fp);
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
    stat(filename, &stat_results);

    if ((unsigned int)stat_results.st_size <= maxromspace+512-curromspace)
    {
      FILE *fp = 0;
      fp = fopen(filename, "rb");

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
  int size, err;
  gzFile GZipFile;
  FILE *fp = 0;
  fp = fopen(filename, "rb");
  if (!fp) { return; }
  fseek(fp, -4, SEEK_END);
  //Size is read like this due to VC screwing up with optimizations
  size = fgetc(fp);
  size |= fgetc(fp) << 8;
  size |= fgetc(fp) << 16;
  size |= fgetc(fp) << 24;
  fclose(fp);

  if ((unsigned int)size > maxromspace+512) { return; }

  //Open GZip file for decompression
  GZipFile = gzopen(filename, "rb");

  //Decompress file into memory
  err = gzread(GZipFile, romdata, size);

  //Close compressed file
  gzclose(GZipFile);

  if (err != size) { return; }

  curromspace = size;
}

//void Output_Text();
//asm volatile("movl _ZOpenFileName, %edx   \n"
//             "movb $9, %ah                \n"
//             "call _Output_Text           \n");
//system("pause");

void loadZipFile(char *filename)
{
  int err, fileSize;
  unsigned char *ROM = (unsigned char *)romdata;
  bool multifile = false, NSS = false;
  char *incrementer = 0;

  unzFile zipfile = unzOpen(filename); //Open zip file
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

  unsigned int pathlen = strlen(path);
  const char *ext = path+pathlen-4;
  if (pathlen >= 5 && !strcasecmp(ext, ".jma"))
  {
    load_jma_file(path);
  }
  else if (pathlen >= 5 && !strcasecmp(ext, ".zip"))
  {
    loadZipFile(path);
  }
  else if (pathlen >= 4 && !strcasecmp(ext+1, ".gz"))
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
void SplitSetup(char *basepath, char *basefile, unsigned int MirrorSystem)
{
  unsigned char *ROM = (unsigned char *)romdata;

  curromspace = 0;
  if (maxromspace < addOnStart+addOnSize) { return; }
  memcpy(ROM+addOnStart, ROM, addOnSize);

  if (STCart2)
  {
    if (maxromspace < 0x300000) { return; }
    curromspace = 0;
    load_file_fs(STCart2);
    memcpy(ROM+0x200000, ROM, addOnSize);
    curromspace = 0;
  }

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
      if (STCart2 && addOnSize < 0x100000)
      {
        memcpy(ROM+0x180000, ROM+0x100000, 0x80000);
        memcpy(ROM+0x280000, ROM+0x200000, 0x80000);
      }
      break;
  }

  curromspace = addOnStart+addOnSize;
  if (STCart2)
  {
    curromspace = 0x300000;
  }
  SplittedROM = true;
}

extern char STPath[1024];
extern char GNextPath[1024];
extern char SGPath[1024];
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
    addOnStart = 0x200000;
  }

  //Sufami Turbo
  if (!strncmp(ROM, "BANDAI SFC-ADX", 14))
  {
    addOnStart = 0x100000;
    addOnSize = curromspace;
    SplitSetup(STPath, "STBIOS.ZIP", 3);
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

extern bool EMUPause;
extern bool Sup48mbit;
extern bool Sup16mbit;
extern unsigned char snesmouse;
unsigned char snesinputdefault;
bool input1gp;
bool input1mouse;
bool input2gp;
bool input2mouse;
bool input2scope;
bool input2just;
void findZipIPS(char *);
void loadROM()
{
  bool isCompressed = false, isZip = false;

  EMUPause = false;
  curromspace = 0;

  maxromspace = 4194304;
  if (Sup48mbit) { maxromspace += 2097152; }
  if (Sup16mbit) { maxromspace -= 2097152; } //I don't get it either

  lastROMFileName = ZOpenFileName;

  if (strlen(ZOpenFileName) >= 5) //Char + ".jma"
  {
    char *ext = ZOpenFileName+strlen(ZOpenFileName)-4;
    if (!strcasecmp(ext, ".jma"))
    {
      isCompressed = true;
      load_jma_file(ZOpenFileName);
    }
  }

  if (strlen(ZOpenFileName) >= 5) //Char + ".zip"
  {
    char *ext = ZOpenFileName+strlen(ZOpenFileName)-4;
    if (!strcasecmp(ext, ".zip"))
    {
      isCompressed = true;
      isZip = true;
      loadZipFile(ZOpenFileName);
    }
  }

  if (strlen(ZOpenFileName) >= 4) //Char + ".gz"
  {
    char *ext = ZOpenFileName+strlen(ZOpenFileName)-3;
    if (!strcasecmp(ext, ".gz"))
    {
      isCompressed = true;
      loadGZipFile(ZOpenFileName);
    }
  }

  if (!isCompressed) { loadFile(ZOpenFileName); }

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

  snesmouse = 0;
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
      switch (ROM[0x1ED])
      {
        default: break;

        case 0:
          input1mouse = false;
          input2mouse = false;
          input2scope = false;
          input2just = false;
          break;

        case 0x01: //Mouse port 2
          snesmouse = 2;
          input2gp = false;
          input2scope = false;
          input2just = false;
          input1mouse = false;
          break;

        case 0x03: //Super Scope port 2
          snesmouse = 3;
          input2gp = false;
          input2mouse = false;
          input2just = false;
          input1mouse = false;
          break;

        case 0x04: //Super Scope or Gamepad port 2
          snesmouse = 3;
          input2mouse = false;
          input2just = false;
          input1mouse = false;
          break;

        case 0x05: //Justifier (Lethal Enforcer gun) port 2
          snesmouse = 4;
          input2mouse = false;
          input2scope = false;
          input1mouse = false;
          break;

        case 0x06: //Multitap port 2
          input2gp = false;
          input2mouse = false;
          input2just = false;
          input2scope = false;
          input1mouse = false;
          break;

        case 0x08: //Mouse or Multitap port 2
          snesmouse = 2;
          input2just = false;
          input2scope = false;
          input1mouse = false;
          break;

        case 0x10: //Mouse port 1
          snesmouse = 1;
          input2mouse = false;
          input2just = false;
          input2scope = false;
          input1gp = false;
          break;

        case 0x20: //Mouse or Gamepad port 1
          snesmouse = 1;
          input2mouse = false;
          input2just = false;
          input2scope = false;
          break;

        case 0x22: //Mouse or Gamepad port 1 and port 2
          snesmouse = 1;
          input2just = false;
          input2scope = false;
          break;

        case 0x27: //Mouse or Gamepad port 1, Mouse, Super Scope, or Gamepad port 2
          input2just = false;
          break;

        case 0x99: break; //Lasabirdie
        case 0x0A: break; //Barcode Battler
      }
    }
    curromspace -= 512;
    memmove((unsigned char *)romdata, ((unsigned char *)romdata)+512, curromspace);
  }

  snesinputdefault = snesmouse;

  SplitSupport();

  if (isZip) { findZipIPS(ZOpenFileName); }
}


//Memory Setup functions
extern unsigned char wramdataa[65536];
extern unsigned char ram7fa[65536];
extern unsigned char srama[65536];
extern unsigned char debugbufa[80000];
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
  memset(sram, 0xFF, 16384);
  clearSPCRAM();
}

void clearmem()
{
  memset(vidbuffer, 0, 131072);
  memset(wramdataa, 0, 65536);
  memset(ram7fa, 0, 65536);
  memset(vram, 0, 65536);
  memset(srama, 0, 65536);
  memset(debugbufa, 0, 80000);
  memset(regptra, 0, 49152);
  memset(regptwa, 0, 49152);
  memset(vcache2b, 0, 262144);
  memset(vcache4b, 0, 131072);
  memset(vcache8b, 0, 65536);
  memset(vidmemch2, 0, 4096);
  memset(vidmemch4, 0, 4096);
  memset(vidmemch8, 0, 4096);
  memset(pal16b, 0, 1024);
  memset(pal16bcl, 0, 1024);
  memset(pal16bclha, 0, 1024);
  memset(pal16bxcl, 0xFF, 256);
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
  memset(&BRRBuffer, 0, PHdspsave);
  memset(&echoon0, 0, PHdspsave2);
  memset(echobuf, 0, 90000);
  memset(spcBuffera, 0, 65536*4+4096);
  memset(DSPMem, 0, 256);
}

/*

--------------Caution Hack City--------------

Would be nice to trash this section in the future
*/


extern unsigned char  disablehdma;
extern unsigned char  hdmaearlstart;
extern unsigned int   WindowDisables;
extern unsigned char  ClearScreenSkip;
extern unsigned char  ENVDisable;
extern unsigned char  latchyr;
extern unsigned char  cycpb268;
extern unsigned char  cycpb358;
extern unsigned char  cycpbl2;
extern unsigned char  cycpblt2;
extern unsigned char  cycpbl;
extern unsigned char  cycpblt;
extern unsigned char  opexec268;
extern unsigned char  opexec358;
extern unsigned char  opexec268b;
extern unsigned char  opexec358b;
extern unsigned char  opexec268cph;
extern unsigned char  opexec358cph;
extern unsigned char  opexec268cphb;
extern unsigned char  opexec358cphb;
extern unsigned char  DSP1Type;
extern unsigned int   ewj2hack;
extern unsigned char  cycpl;
unsigned char HacksDisable;

void headerhack()
{
  char *RomData = (char *)romdata;
  disablehdma = 0;
  hdmaearlstart = 0;
  WindowDisables = 0;
  ClearScreenSkip = 0;
  ENVDisable = 0;

  if ((curromspace < Lo) || (HacksDisable && !DSP1Type))
  {
    return;
  }

  //These next few look like RAM init hacks, should be looked into

  //Should be Super Famista (J), uses non-standard characters
  if (!strncmp((RomData+Lo),"\x0bd\x0b0\x0ca\x0df\x0b0\x0cc\x0a7\x0d0\x0bd\x0c0      " ,16))
  {
    RomData[0x2762F] = 0xEA;
    RomData[0x27630] = 0xEA;
  }

  //Should be Super Famista 2 (J), uses non-standard characters
  if (!strncmp((RomData+Lo),"\x0bd\x0b0\x0ca\x0df\x0b0\x0cc\x0a7\x0d0\x0bd\x0c0 \x032    " ,16))
  {
    //Skip a check for value FF at 2140 when spc not initialized yet?!?
    RomData[0x6CED] = 0xEA;
    RomData[0x6CEE] = 0xEA;
    //Skip a check for value FF at 2140 when spc not initialized yet?!?
    RomData[0x6CF9] = 0xEA;
    RomData[0x6CFA] = 0xEA;
  }

  //Kamen Rider (J)
  if (!strncmp((RomData+Lo),"SFC \x0b6\x0d2\x0dd\x0d7\x0b2\x0c0\x0de\x0b0    " ,16))
  {
    latchyr = 2;
  }

  //Deae Tonosama Appare Ichiban (J)
  if (!strncmp((RomData+Lo),"\x0c3\x0de\x0b1\x0b4\x0c4\x0c9\x0bb\x0cf \x0b1\x0af\x0ca" ,12))
  {
    RomData[0x17837] = 0xEA;
    RomData[0x17838] = 0xEA;
  }

  /*
  The asm indicates the hack is for HGP3, but all of these are affected
  Human Grand Prix (J), Human Grand Prix II (J),
  Human Grand Prix III - F1 Triple Battle (J).
  Human Grand Prix IV is a HiROM and is not affected
  */
  if (!strncmp((RomData+Lo),"HUMAN GRANDP" ,12))
  {
    cycpb268 = 135;
    cycpb358 = 157;
    cycpbl2  = 125;
    cycpblt2 = 125;
    cycpbl   = 125;
    cycpblt  = 125;
  }

  //Accele Brid (J)
  if (!strncmp((RomData+Lo),"ACCELEBRID  " ,12))
  {
    RomData[0x34DA2] = 0;
    RomData[0x34DA3] = 0;
  }

  //Battle Grand Prix (J)
  if (!strncmp((RomData+Lo),"BATTLE GRAND" ,12))
  {
    RomData[0x18089] = 0xFB;
    RomData[0x6C95]  = 0xFB;
  }

  //Neugier (J), and it's English translation
  if (!strncmp((RomData+Lo),"NEUGIER     " ,12) ||
      !strncmp((RomData+Lo),"Neugier (tr." ,12))
  {
    RomData[0xD4150] = 0xF9;
  }

  //Home Alone (J/E/U)
  if (!strncmp((RomData+Lo),"HOME ALO" ,8))
  {
    RomData[0x666B] = 0xEE;
    RomData[0x666C] = 0xBC;
  }

  //Emerald Dragon (J)
  if (!strncmp((RomData+Hi),"EMERALD DRAG" ,12))
  {
    ENVDisable = true;
  }

  /*
  Super Mario World 2 - Yoshi's Island (U/E),
  Super Mario - Yossy Island (J), and variants
  */
  if (!strncmp((RomData+Lo),"YOSSY'S ISLA" ,12) ||
      !strncmp((RomData+Lo),"YOSHI'S ISLA" ,12))
  {
    hdmaearlstart = 2;
    opexec268 = 116;
    opexec358 = 126;
  }

  //Bubsy II (U/E)
  if (!strncmp((RomData+Hi),"BUBSY II" ,8))
  {
    cycpb268 = 125;
    cycpb358 = 147;
    cycpbl2  = 125;
    cycpblt2 = 125;
    cycpbl   = 125;
    cycpblt  = 125;
  }

  /*
  Marvelous (J) has this hack in the asm, but disabled

  Alternate if for Marvelous-inclusive version
  if (!strncmp((RomData+Lo),"\x0cf\x0bo\x0b3\x0de", 4) ||
      !strncmp((RomData+Lo),"REND", 4))
  */
  //Rendering Ranger R2
  if (!strncmp((RomData+Lo),"REND", 4))
  {
    cycpb268 = 157;
    cycpb358 = 157;
    cycpbl2  = 157;
    cycpblt2 = 157;
    cycpbl   = 157;
    cycpblt  = 157;
  }

  //Clay Fighter (U), other versions are CLAYFIGHTER with no space
  if (!strncmp((RomData+Hi),"CLAY FIGHTER    " ,16))
  {
    //Intro
    RomData[0x1A10B9] = 0xDE;
    //In Game
    RomData[0x1A1996] = 0xDE;
    RomData[0x1AE563] = 0xDE;
    RomData[0x1AE600] = 0xDE;
  }

  //Bahamut Lagoon (J) and all known translations
  if (!strncmp((RomData+Hi),"Bahamut Lago" ,12))
  {
    RomData[0x10254] = 0xEE;
  }

  //Mortal Kombat (J/U/E), Super Punch-Out, Dragon Quest 5 (J)
  if (!strncmp((RomData+Lo),"DRAGONQUEST5" ,12) ||
      !strncmp((RomData+Lo),"MORTAL KOMBAT   " ,16) ||
      !strncmp((RomData+Lo),"Super Punch-Out!!   ", 20))
  {
    disablehdma = true;
  }

  //Super Final Match Tennis (J)
  if (!strncmp((RomData+Lo),"SP F", 4))
  {
    cycpb268 = 145;
    cycpb358 = 147;
    cycpbl2  = 145;
    cycpblt2 = 145;
    cycpbl   = 145;
    cycpblt  = 145;
  }

  //Tuff E Nuff (U/E), Dead Dance (J),
  //Cyber Knight II - Tikyu Teikoku no Yabou (J)
  if (!strncmp((RomData+Lo),"CYBER KNIGHT 2  " ,16) ||
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

  //Okaaay...
  if(DSP1Type) { disablehdma = true; }

  //Earthworm Jim 2 (all regions?)
  if (!strncmp((RomData+Lo),"EARTHWORM JIM 2     " ,20))
  {
    RomData[0x2A9C1A] = 0;
    RomData[0x2A9C1B] = 0;
    RomData[0x2A9C1F] = 0;
    RomData[0x2A9C20] = 0;
    ewj2hack = true;
  }

  //Lamborghini - American Challenge (U/E)
  if (!strncmp((RomData+Lo), "LAMBORGHINI AMERICAN", 20))
  {
    opexec268 = 187;
    opexec358 = 187;
  }

  //Addams Family Values (U/E)
  if (!strncmp((RomData+Lo), "ADDAMS FAMILY VALUES", 20))
  {
    opexec268 = 120;
    opexec358 = 100;
  }

  //Front Mission
  if (!strncmp((RomData+Hi), "\x0cc\x0db\x0dd\x0c4\x0d0\x0af\x0bc\x0ae", 8) ||
      !strncmp((RomData+Hi), "FRONT MI", 8))
  {
    opexec268 = 226;
    opexec358 = 226;
    opexec268cph = 80;
    opexec358cph = 80;
  }
}

extern unsigned char per2exec;

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

extern char FEOEZPath[1024];
extern char SJNSPath[1024];
extern char MDHPath[1024];
extern char SPL4Path[1024];
char *SPC7110filep;
char SPC7110nfname[1024+12]; //12 is / plus 6.3
unsigned int SPC7110IndexSize;
extern unsigned int SPC7110Entries;
void SPC7PackIndexLoad()
{
  char *ROM = (char *)romdata;
  FILE *fp = 0;
  SPC7110IndexSize = 0;

  //Get correct path for the ROM we just loaded
  if (!strncmp(ROM+infoloc, "HU TENGAI MAKYO ZERO ", 21))
  {
    strcpy(SPC7110nfname, *FEOEZPath ? FEOEZPath : "FEOEZSP7");
  }
  else if (!strncmp(ROM+infoloc, "JUMP TENGAIMAKYO ZERO", 21))
  {
    strcpy(SPC7110nfname, *SJNSPath ? SJNSPath : "SJNS-SP7");
  }
  else if (!strncmp(ROM+infoloc, "MOMOTETSU HAPPY      ", 21))
  {
    strcpy(SPC7110nfname, *MDHPath ? MDHPath : "MDH-SP7");
  }
  else if (!strncmp(ROM+infoloc, "SUPER POWER LEAG 4   ", 21))
  {
    strcpy(SPC7110nfname, *SPL4Path ? SPL4Path : "SPL4-SP7");
  }

  //Add a slash if needed
  if (SPC7110nfname[strlen(SPC7110nfname)-1] != *DIR_SLASH)
  {
    strcat(SPC7110nfname, DIR_SLASH);
  }

  //Set the pointer to after the slash - needed for sa1regs.asm
  SPC7110filep = SPC7110nfname+strlen(SPC7110nfname);

  //Index file;
  strcat(SPC7110nfname, "index.bin");

  //Load the index
  fp = fopen(SPC7110nfname, "rb");
  if (!fp) { return; }
  SPC7110IndexSize = fread(ROM+0x580000, 1, 12*32768, fp);
  fclose(fp);

  SPC7110Entries = 0;

  //Get file pointer ready for individual pack files
  strcpy(SPC7110filep, "123456.bin"); //Extension Lower Case
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

unsigned int crc32_table[256] = {
	0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F,
	0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
	0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2,
	0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
	0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9,
	0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
	0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C,
	0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
	0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423,
	0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
	0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190, 0x01DB7106,
	0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
	0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D,
	0x91646C97, 0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
	0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950,
	0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
	0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7,
	0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
	0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA,
	0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
	0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81,
	0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
	0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84,
	0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
	0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB,
	0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
	0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E,
	0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
	0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55,
	0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
	0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28,
	0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
	0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F,
	0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
	0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242,
	0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
	0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69,
	0x616BFFD3, 0x166CCF45, 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
	0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC,
	0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
	0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693,
	0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
	0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D };

unsigned int CalcCRC32 (unsigned char *start, unsigned int size)
{
  unsigned int i, result=0xFFFFFFFF;

  for (i=0 ; i<size ; i++)
  {
    result = (result >> 8) ^ (crc32_table[(result ^ (*(start+i))) & 0xFF]);
  }

  return (~result);
}

extern unsigned int MsgCount, MessageOn, CRC32;
extern unsigned char IPSPatched;
extern char *Msgptr;

unsigned int showinfogui()
{
  unsigned int i;
  unsigned char *ROM = (unsigned char *)romdata;

  if (infoloc == 0x40FFC0)	{ memcpy (CSStatus2+23, "EHi ", 4); }
  else
  {
    if (romtype == 2)	{ memcpy (CSStatus2+23, "Hi  ", 4); }
    else	{ memcpy (CSStatus2+23, "Lo  ", 4); }
  }

  for (i=0 ; i<20 ; i++)
  {
    CSStatus[i] = (ROM[infoloc + i]) ? ROM[infoloc + i] : 32;
  }

  if ((ROM[infoloc + 25] < 2 ) || (ROM[infoloc + 25] > 12))
  {
    memcpy (CSStatus3+6, "NTSC", 4);
  }
  else	{ memcpy (CSStatus3+6, "PAL ", 4); }

  if (IPSPatched)	{ memcpy (CSStatus3+16, "IPS ", 4); }
  else	{ memcpy (CSStatus3+16, "    ", 4); }

  memcpy (CSStatus+29, "NORMAL  ", 8);

  if (SA1Enable)     { memcpy (CSStatus+29, "SA-1    ", 8); }
  if (RTCEnable)     { memcpy (CSStatus+29, "RTC     ", 8); }
  if (SPC7110Enable) { memcpy (CSStatus+29, "SPC7110 ", 8); }
  if (SFXEnable)     { memcpy (CSStatus+29, "SUPER FX", 8); }
  if (C4Enable)      { memcpy (CSStatus+29, "C4      ", 8); }
  if (DSP1Enable)    { memcpy (CSStatus+29, "DSP-1   ", 8); }
  if (DSP2Enable)    { memcpy (CSStatus+29, "DSP-2   ", 8); }
  if (DSP3Enable)    { memcpy (CSStatus+29, "DSP-3   ", 8); }
  if (DSP4Enable)    { memcpy (CSStatus+29, "DSP-4   ", 8); }
  if (SDD1Enable)    { memcpy (CSStatus+29, "S-DD1   ", 8); }
  if (OBCEnable)     { memcpy (CSStatus+29, "OBC1    ", 8); }
  if (SETAEnable)    { memcpy (CSStatus+29, "SETA DSP", 8); }
  if (ST18Enable)    { memcpy (CSStatus+29, "ST018   ", 8); }
  if (SGBEnable)     { memcpy (CSStatus+29, "SGB     ", 8); }
  if (BSEnable)      { memcpy (CSStatus+29, "BROADCST", 8);
	// dummy out date so CRC32 matches
    ROM[infoloc + 22] = 0x42;
    ROM[infoloc + 23] = 0x00;
	// 42 is the answer, and the uCONSRT standard
  }

  if (Interleaved)	{ memcpy (CSStatus2+12, "Yes ", 4); }
  else	{ memcpy (CSStatus2+12, "No  ", 4); }

	// calculate CRC32 for the whole ROM, or Add-on ROM only
  CRC32 = (SplittedROM) ? CalcCRC32(ROM+addOnStart, addOnSize) : CalcCRC32(ROM, NumofBytes);

	// place CRC32 on line
  sprintf (CSStatus3+32, "%08X", CRC32);

  CalcChecksum();

  i = (SplittedROM) ? infoloc + 0x1E + addOnStart: infoloc + 0x1E;

  if ((ROM[i] == (Checksumvalue & 0xFF)) && (ROM[i+1] == (Checksumvalue >> 8)))
	{ memcpy (CSStatus2+36, "OK  ", 4); }
  else	{ memcpy (CSStatus2+36, "FAIL", 4); }

  DumpROMLoadInfo();

  MessageOn = 300;
  Msgptr = CSStatus;
  return (MsgCount);
}

extern unsigned int nmiprevaddrl, nmiprevaddrh, nmirept, nmiprevline, nmistatus;
extern unsigned char spcnumread;
extern unsigned char NextLineCache, sramsavedis, sndrot, regsbackup[3019];
extern unsigned char yesoutofmemory, fnames[512];

void initsnes();
void outofmemfix();
void GUIDoReset();

bool loadSRAM(char *sramname)
{
  FILE *sramfp;
  int sramsize;

  if ((sramfp = fopen(sramname, "rb")))
  {
    fseek(sramfp, 0, SEEK_END);
    sramsize = ftell(sramfp);
    rewind(sramfp);
    if (sramsize)	{ fread(sram, 1, sramsize, sramfp); }
    fclose(sramfp);
    return (true);
  }
  else	{ return(false); }
}

extern unsigned int Voice0Freq, Voice1Freq, Voice2Freq, Voice3Freq;
extern unsigned int Voice4Freq, Voice5Freq, Voice6Freq, Voice7Freq;
extern unsigned int dspPAdj;
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

extern unsigned int SfxR1, SfxR2, SetaCmdEnable, SfxSFR, SfxSCMR;
extern unsigned char lorommapmode2, disablespcclr, *sfxramdata, SramExists;
extern unsigned char *setaramdata, *wramdata, *SA1RAMArea, cbitmode, curromsize;
extern unsigned char ForcePal, ForceROMTiming, romispal, MovieWaiting;
extern unsigned short totlines;
void SetAddressingModes(), GenerateBank0Table();
void SetAddressingModesSA1(), GenerateBank0TableSA1();
void calculate_state_sizes(), InitRewindVars();
void InitDSP(), InitDSP2(), InitDSP4(), InitFxTables(), initregr(), initregw();
void SPC7110Load(), DOSClearScreen(), dosmakepal();

void CheckROMType()
{ // used by both movie and normal powercycles
  unsigned char *ROM = (unsigned char *)romdata;

  BankCheck();
  if (!MovieWaiting) { MirrorROM(); }

  lorommapmode2 = 0;
  if (!strncmp((char *)ROM+0x207FC0, "DERBY STALLION 96", 17) || !strncmp((char *)ROM+Lo, "SOUND NOVEL-TCOOL", 17))
  {
    lorommapmode2 = 1;
  }

  // Setup memmapping
  SetAddressingModes();
  GenerateBank0Table();
  chip_detect();

  disablespcclr = (memcmp(ROM+Hi, "\0x42\0x53\0x20\0x5A", 4)) ? 0 : 1;

  if ((romtype == 1) && (!SDD1Enable))
  {  // Non-SDD1 LoROM SRAM mapping, banks F0 - F3
    map_mem(0xF0, &srambank, 4);
  }

  // Setup DSP-X stuff
  DSP1Type = 0;

  if (DSP1Enable || DSP2Enable)
  {
    if (DSP2Enable)
    {
      asm_call(InitDSP2);
    }

    InitDSP();

    DSP1Type = (romtype == 2) ? 2 : 1;
  }
/*
  if (DSP3Enable)
  {
    InitDSP3();

    // DSP-3 mapping, banks 20 - 3F
    map_mem(0x20, &dsp3bank, 0x20);
  }
*/
  if (DSP4Enable)
  {
    InitDSP4();

    // DSP-4 mapping, banks 30 - 3F
    map_mem(0x30, &dsp4bank, 0x10);
  }

  if (SFXEnable)
  {
    // Setup SuperFX stuff
    if (Sup48mbit)
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
    // Setup SETA 010/011 stuff

    // Really banks 68h-6Fh:0000-7FFF are all mapped the same by the chip but
    // F1 ROC II only uses bank 68h
    map_mem(0x68, &setabank, 1);

    // Control register (and some status?) is in banks 60h-67h:0000-3FFF
    map_mem(0x60, &setabanka, 1);

    SetaCmdEnable = 0x00000080; // 60:0000
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
      if (SramExists)	{ memcpy(SA1RAMArea, sram, 131072); }
    }
  }

  if (DSP1Type == 1)
  {
    map_mem(0x30, &dsp1bank, 0x10);
    map_mem(0xB0, &dsp1bank, 0x10);
    map_mem(0xE0, &dsp1bank, 0x10);

    if (DSP2Enable)
    {
      map_mem(0x3F, &dsp2bank, 1);
    }
  }

  wramdata = wramdataa;

  asm_call(SPC7110Load);
}

extern unsigned short copv, brkv, abortv, nmiv, nmiv2, irqv, irqv2, resetv;
extern unsigned short copv8, brkv8, abortv8, nmiv8, irqv8;

void SetIRQVectors()
{ // get vectors (NMI & reset)
  unsigned char *ROM = (unsigned char *)romdata;

  if (!(ROM[infoloc+21] & 0xF0)) // if not fastrom
  {
    opexec358 = opexec268;
    opexec358cph = opexec268cph;
    cycpb358 = cycpb268;
  }

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
  unsigned char *ROM = (unsigned char *)romdata;

  CheckROMType();
  SetIRQVectors();

  #ifdef __MSDOS__
    asm_call(DOSClearScreen);

    if (!cbitmode) // 8-bit mode uses a palette
    {
      asm_call(dosmakepal);
    }
  #endif

  // get ROM and SRAM sizes
  curromsize = ROM[infoloc+0x17];

  SetupSramSize();
  calculate_state_sizes();
  InitRewindVars();

  // get timing (pal/ntsc)
  ForcePal = ForceROMTiming;

  switch (ForcePal)
  {
    case 1:
      romispal = 0;
      break;
    case 2:
      romispal = (!BSEnable);
      break;
    default:
      romispal = ((!BSEnable) && (ROM[infoloc+0x19] > 1) && (ROM[infoloc+0x19] < 0xD));
  }

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

void powercycle(bool sramload)
{ // currently only used by movies - rom already loaded, no need for init
  memset(sram, 0xFF, 32768);
  clearSPCRAM();

  nmiprevaddrl = 0;
  nmiprevaddrh = 0;
  nmirept = 0;
  nmiprevline = 224;
  nmistatus = 0;
  spcnumread = 0;
  NextLineCache = 0;
  curexecstate = 1;

  if (sramload)	{ loadSRAM((char *)fnames+1); }
  SetupROM();
//  asm_call(initsnes);

  sramsavedis = 0;

  memcpy(&sndrot, regsbackup, 3019);

  if (yesoutofmemory == 1)	{ asm_call(outofmemfix); }

  asm_call(GUIDoReset);
}
