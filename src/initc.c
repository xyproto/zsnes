/*
Copyright (C) 2003 ZSNES Team ( zsknight@zsnes.com / _demo_@zsnes.com )

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

#include <string.h>

//C++ style code in C
#define bool unsigned char
#define true 1
#define false 0


#define Lo 0x7FC0
#define Hi 0xFFC0
#define EHi 0x40FFC0

#define MB_bytes 0x100000
#define Mbit_bytes 0x20000


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

//Deinterleave functions
bool validChecksum(unsigned char *ROM, int BankLoc)
{
  if (ROM[BankLoc + 28] + (ROM[BankLoc + 29] << 8) +
      ROM[BankLoc + 30] + (ROM[BankLoc + 31] << 8) == 0xFFFF)
  {
    return(true);
  }
  return(false);
}

bool EHiHeader(unsigned char *ROM, int BankLoc)
{
  if (validChecksum(ROM, BankLoc) && ROM[BankLoc+21] == 53)
  {
    return(true);
  }
  return(false);
}

void swapBlocks(char *blocks)
{
  unsigned int i,j,k;
  for (i = 0; i < NumofBanks; i++)
  {
    for (j = 0; j < NumofBanks; j++)
    {
      if (blocks[j] == (char)i)
      {
        char b;
        unsigned int temp,
                    *loc1 = romdata + blocks[i]*0x2000,
                    *loc2 = romdata + blocks[j]*0x2000;
        for (k = 0; k < 0x2000; k++)
        {
          temp = loc1[k];
          loc1[k] = loc2[k];
          loc2[k] = temp;
        }
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
      ROM[ROMmidPoint+Lo+25] < 14) //Country Code
  {    
    deintlv1(); 
    Interleaved = true;
  }
  else if (validChecksum(ROM, Lo) && !validChecksum(ROM, Hi) &&
           ROM[Lo+25] < 14 && //Country code
           //Rom make up
          (ROM[Lo+21] == 33 || ROM[Lo+21] == 49 ||
           ROM[Lo+21] == 53 || ROM[Lo+21] == 58))
  {
    if (ROM[Lo+20] == 32 ||//Check that Header name did not overflow
      !(ROM[Lo+21] == ROM[Lo+20] || ROM[Lo+21] == ROM[Lo+19] ||
        ROM[Lo+21] == ROM[Lo+18] || ROM[Lo+21] == ROM[Lo+17]))
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
    unsigned int temp, i, oldNumBanks = NumofBanks,
                *loc1 = romdata, 
                *loc2 = romdata + ((NumofBytes - 0x400000)/4);
  
    //Swap 4MB ROM with the other one
    for (i = 0; i < 0x100000; i++)
    {
      temp = loc1[i];
      loc1[i] = loc2[i];
      loc2[i] = temp;
    }
    
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
  if (Buffer[26] == 0x33)            { score += 2; }
  if (!(Buffer[61] & 0x80))          { score -= 4; }
  if ((1 << (Buffer[23] - 7)) > 48)  { score -= 1; }
  if (Buffer[25] < 14)               { score += 1; }
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

    switch(ROM[Lo + 21])
    {
      case 32: case 35: case 48: case 50:
        loscore += 2;      
      case 128: case 156: case 176: case 188: case 252: //BS
        loscore += 1;      
        break;
    }
    switch(ROM[Hi + 21])
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

extern unsigned char SPC7110Enable;
extern unsigned char BSEnable;
extern unsigned short Checksumvalue;
void CalcChecksum()
{
  unsigned char *ROM = (unsigned char *)romdata;
  if (SPC7110Enable)
  {
    Checksumvalue = sum(ROM, NumofBytes);
    if (NumofBanks == 96)
    { 
      Checksumvalue += Checksumvalue; //Fix for 24Mb SPC7110 ROMs
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
extern unsigned char spcRam[65472];

extern unsigned char *sram;
extern unsigned char *vidbuffer;
extern unsigned char *vram;
extern unsigned char *vcache2b;
extern unsigned char *vcache4b;
extern unsigned char *vcache8b;

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
}

void clearmem2()
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
    memset(spcRam+i, 0, 0x20);
    memset(spcRam+i+0x20, 0xFF, 0x20);
  }    

  memset(sram, 0xFF, 16384);
}


/*

--------------Caution Hack City--------------

Would be nice to trash this section in the future
*/


extern unsigned char  disablehdma;
extern unsigned char  Offby1line;
extern unsigned char  CacheCheckSkip;
extern unsigned short IRQHack;
extern unsigned char  HIRQSkip;
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
extern unsigned char  opexec268cph;
extern unsigned char  opexec358cph;
extern unsigned char  DSP1Type;
extern unsigned int   ewj2hack;
extern unsigned char  cycpl;

void headerhack()
{
  char *RomData = (char *)romdata;
  disablehdma = 0;
  Offby1line = 0;
  CacheCheckSkip = 0;
  IRQHack = 0;
  HIRQSkip = 0;
  hdmaearlstart = 0;
  WindowDisables = 0;
  ClearScreenSkip = 0;
  ENVDisable = 0;
  

  //These next fiew look like RAM init hacks, should be looked into

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

  //Star Trek - Deep Space Nine: Crossroads of Time (U/E)
  if (!strncmp((RomData+Lo),"STAR TREK: D" ,12))
  {
    opexec268 = 187;
    opexec358 = 187;
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

  //Clock Tower (J) and AGTP translation
  if (!strncmp((RomData+Hi),"CLOCK TOWER " ,12))
  {
    opexec268 = 187;
    opexec358 = 182;
    opexec268cph = 47;
    opexec358cph = 47;
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

  //Bubsy in Claws Encounters of the Furred Kind (U/E)
  if (!strncmp((RomData+Lo), "Bubsy               ", 20))
  {
    opexec268 = 220;
    opexec358 = 220;
    opexec268cph = 64;
    opexec358cph = 64;
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

  //Clay Fighter 2
  if (!strncmp((RomData+Hi), "Clay", 4) && RomData[Hi+12] == '2')
  {
    opexec268 = 187;
    opexec358 = 182;
    opexec268cph = 47;
    opexec358cph = 47;
  }

  return;
}

