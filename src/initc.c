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

//C++ style code in C
#define bool unsigned char
#define true 1
#define false 0

#define Lo 0x7FC0
#define Hi 0xFFC0
#define EHi 0x40FFC0


//I want to port over the more complicated
//functions from init.asm, or replace with
//better versions from NSRT. -Nach


//init.asm goodness
extern unsigned int NumofBanks;
extern unsigned int NumofBytes;
extern unsigned int *romdata;
extern unsigned char romtype;
extern unsigned char Interleaved;





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
  int i,j,k;
  for (i = 0; i < NumofBanks; i++)
  {
    for (j = 0; j < NumofBanks; j++)
    {
      if (blocks[j] == i)
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
bool AllASCII(char *b, int size)
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

int InfoScore(char *Buffer)
{
  int score = 0;
  if (validChecksum(Buffer, 0))      { score += 3; }
  if (Buffer[26] == 0x33)            { score += 2; }
  if ((Buffer[21] & 0xf) < 4)        { score += 2; }
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
    int loscore, hiscore;

    //Deinterleave if neccesary
    CheckIntl1(ROM);
    
    loscore = InfoScore(ROM+Lo);
    hiscore = InfoScore(ROM+Hi);

    switch(ROM[Lo + 21])
    {
      case 32: case 35: case 48: case 50:
      case 128: case 156: case 176: case 188: case 252: //BS
        loscore += 1;      
        break;
    }
    switch(ROM[Hi + 21])
    {
      case 33: case 49: case 53:
      case 128: case 156: case 176: case 188: case 252: //BS
        hiscore += 1;      
        break;
    }
    
    if(ForceHiLoROM)
    {
      //asm volatile("int $3");
      if (forceromtype == 1) { loscore += 50; }
      else if (forceromtype == 2) { hiscore += 50; }
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
  if (array + size > (unsigned char *)romdata + NumofBytes)
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
  unsigned short Mbit = NumofBanks >> 2;
  
  if ((Mbit == 10 || Mbit == 20 || Mbit == 40) && !SPC7110Enable)
  {
    unsigned int P1Size = 512 << ROM[infoloc + 23];
    unsigned short part1 = sum(ROM, P1Size),
                   part2 = sum(ROM+P1Size, NumofBytes-P1Size);
    Checksumvalue = part1 + part2*4;
  }
  else if ((Mbit == 12 || Mbit == 24 || Mbit == 48) && !SPC7110Enable)
  {
    unsigned int P1Size = 512 << ROM[infoloc + 23];
    unsigned short part1 = sum(ROM, P1Size),
                   part2 = sum(ROM+P1Size, NumofBytes-P1Size);
    Checksumvalue = part1 + part2 + part2;
  }
  else
  {
    Checksumvalue = sum(ROM, NumofBytes);
    if (BSEnable)
    {
      Checksumvalue -= sum(&ROM[infoloc - 16], 48); //Fix for BS Dumps
    }
    else if (Mbit == 24)
    { 
      Checksumvalue += Checksumvalue; //Fix for 24Mb SPC7110 ROMs
    }
  }
}

