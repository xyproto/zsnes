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

extern unsigned char disablehdma;
extern unsigned char Offby1line;
extern unsigned char CacheCheckSkip;
extern unsigned short IRQHack;
extern unsigned char HIRQSkip;
extern unsigned char hdmaearlstart;
extern unsigned int WindowDisables;
extern unsigned char ClearScreenSkip;
extern unsigned char ENVDisable;
extern unsigned char* spcRam;
extern unsigned char latchyr;
extern unsigned char cycpb268;
extern unsigned char cycpb358;
extern unsigned char cycpbl2;
extern unsigned char cycpblt2;
extern unsigned char cycpbl;
extern unsigned char cycpblt;
extern unsigned char opexec268;
extern unsigned char opexec358;
extern unsigned char opexec268cph;
extern unsigned char opexec358cph;
extern unsigned char DSP1Type;
extern unsigned int ewj2hack;
extern unsigned char cycpl;
extern unsigned short ramsize;
extern unsigned short ramsizeand;

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

void headerhack()
{
  char* RomData=(char*) romdata;
  disablehdma=0;
  Offby1line=0;
  CacheCheckSkip=0;
  IRQHack=0;
  HIRQSkip=0;
  hdmaearlstart=0;
  WindowDisables=0;
  ClearScreenSkip=0;
  ENVDisable=0;
  

/*
    mov esi,[romdata]
    add esi,0FFC0h
    cmp dword[esi],'HORA' 
    jne .nothoraigakuen
    cmp dword[esi+4],'I-GA'
    jne .nothoraigakuen
    cmp dword[esi+8],'KUEN'
    jne .nothoraigakuen 
    cmp dword[esi+12],'    '
    jne .nothoraigakuen
    mov al,0h
    mov edi,spcRam
    mov ecx,65472
    rep stosb
    ret
.nothoraigakuen
*/
/*
    mov esi,[romdata]
    add esi,07FC0h
    cmp dword[esi],'EURO'
    jne .noteuropeanprimegoal
    cmp dword[esi+4],'PEAN'
    jne .noteuropeanprimegoal
    cmp dword[esi+8],' PRI'
    jne .noteuropeanprimegoal
    cmp dword[esi+12],'ME G'
    jne .noteuropeanprimegoal
    mov al,0h
    mov edi,spcRam
    mov ecx,65472
    rep stosb
    ret
.noteuropeanprimegoal
*/


/*Horai Gakuen no Bouken! (J)*/
/*90 Minutes - European Prime Goal (E) [!]*/
if(0==strncmp((RomData+0xFFC0), "HORAI-GAKUEN    ", 16) ||
   0==strncmp((RomData+0x7FC0), "EUROPEAN PRIME G", 16))
{
  memset(spcRam, 0, 65472);
}

/*
    mov esi,[romdata]
    add esi,07FC0h
    cmp dword[esi],0DFCAB0BDh
    jne .notfamista1
    cmp dword[esi+4],0D0A7CCB0h
    jne .notfamista1
    cmp dword[esi+8],02020C0BDh
    jne .notfamista1
    cmp dword[esi+12],20202020h
    jne .notfamista1
    mov esi,[romdata]
    add esi,2762Fh
    mov word [esi],0EAEAh ; Skip a check for value FF at 2140 when spc not
                          ; initialized yet?!?
.notfamista1
*/

/*uses non-standard characters*/
/*should be Super Famista (J)*/
if(0==strncmp((RomData+0x7FC0),"\x0bd\x0b0\x0ca\x0df\x0b0\x0cc\x0a7\x0d0\x0bd\x0c0      " ,16))
{
	RomData[0x2762F]=0xEA;
	RomData[0x27630]=0xEA;
} 

/*
    mov esi,[romdata]
    add esi,07FC0h
    cmp dword[esi],0DFCAB0BDh
    jne .notfamista2
    cmp dword[esi+4],0D0A7CCB0h
    jne .notfamista2
    cmp dword[esi+8],03220C0BDh
    jne .notfamista2
    cmp dword[esi+12],20202020h
    jne .notfamista2
    mov esi,[romdata]
    add esi,6CEDh
    mov word [esi],0EAEAh ; Skip a check for value FF at 2140 when spc not
                          ; initialized yet?!?
    mov esi,[romdata]
    add esi,6CF9h
    mov word [esi],0EAEAh ; Skip a check for value FF at 2140 when spc not
                          ; initialized yet?!?
.notfamista2
*/

/*uses non-standard characters*/
/*should be Super Famista 2 (J)*/
if(0==strncmp((RomData+0x7FC0),"\x0bd\x0b0\x0ca\x0df\x0b0\x0cc\x0a7\x0d0\x0bd\x0c0\x032     " ,16))
{
	RomData[0x6CED]=0xEA;
	RomData[0x6CEE]=0xEA;
	RomData[0x6CF9]=0xEA;
	RomData[0x6CFA]=0xEA;

} 

/*
    mov esi,[romdata]
    add esi,07FC0h
    cmp dword[esi],20434653h
    jne .notkamenrider
    cmp dword[esi+4],0D7DDD2B6h
    jne .notkamenrider
    cmp dword[esi+8],0B0DEC0B2h
    jne .notkamenrider
    cmp dword[esi+12],20202020h
    jne .notkamenrider
    mov byte[latchyr],2
.notkamenrider
*/

/*Kamen Rider (J)*/
if(0==strncmp((RomData+0x7FC0),"SFC \x0b6\x0d2\x0dd\x0d7\x0b2\x0c0\x0de\x0b0    " ,16))
{
latchyr=2;
} 




/*
    mov esi,[romdata]
    add esi,07FC0h
    cmp dword[esi],'CYBE'
    jne .notcyberknight2
    cmp dword[esi+4],'R KN'
    jne .notcyberknight2
    cmp dword[esi+8],'IGHT'
    jne .notcyberknight2
    cmp dword[esi+12],' 2  '
    jne .notcyberknight2
    mov byte[cycpb268],75
    mov byte[cycpb358],77
    mov byte[cycpbl2],75
    mov byte[cycpblt2],75
    mov byte[cycpbl],75
    mov byte[cycpblt],75
.notcyberknight2
*/

/*Cyber Knight II - Tikyu Teikoku no Yabou (J)*/
if(0==strncmp((RomData+0x7FC0),"CYBER KNIGHT 2  " ,16))
{
cycpb268=75;
cycpb358=77;
cycpbl2=75;
cycpblt2=75;
cycpbl=75;
cycpblt=75;
} 


/*
    mov esi,[romdata]
    add esi,07FC0h
    cmp dword[esi],0B4B1DEC3h
    jne .notdeasomething
    cmp dword[esi+4],0CFBBC9C4h
    jne .notdeasomething
    cmp dword[esi+8],0CAAFB120h
    jne .notdeasomething
    mov esi,[romdata]
    add esi,017837Ch
    mov word [esi],0EAEAh    
.notdeasomething
*/

/*Deae Tonosama Appare Ichiban (J)*/
if(0==strncmp((RomData+0x7FC0),"\x0c3\x0de\x0b1\x0b4\x0c4\x0c9\x0bb\x0cf \x0b1\x0af\x0ca" ,12))
{
	RomData[0x17837]=0xEA;
	RomData[0x17838]=0xEA;
}

/*
    mov esi,[romdata]
    add esi,07FC0h
    cmp dword[esi],'HUMA'
    jne .nothumangrandprix3
    cmp dword[esi+4],'N GR'
    jne .nothumangrandprix3
    cmp dword[esi+8],'ANDP'
    jne .nothumangrandprix3
    mov byte[cycpb268],135
    mov byte[cycpb358],157
    mov byte[cycpbl2],125
    mov byte[cycpblt2],125
    mov byte[cycpbl],125
    mov byte[cycpblt],125
.nothumangrandprix3
*/

/* the asm indicates the hack is for HGP3, but all of these are affected*/
/* Human Grand Prix IV is a HiROM and is not affected*/
/*Human Grand Prix (J)*/
/*Human Grand Prix II (J)*/
/*Human Grand Prix III - F1 Triple Battle (J)*/
if(0==strncmp((RomData+0x7FC0),"HUMAN GRANDP" ,12))
{
cycpb268=135;
cycpb358=157;
cycpbl2=125;
cycpblt2=125;
cycpbl=125;
cycpblt=125;
} 


/*
    mov esi,[romdata]
    add esi,07FC0h
    cmp dword[esi],'ACCE'
    jne .notaccelebrid
    cmp dword[esi+4],'LEBR'
    jne .notaccelebrid
    cmp dword[esi+8],'ID  '
    jne .notaccelebrid
    mov esi,[romdata]
    add esi,034DA2h
    mov byte[esi],000h
    mov esi,[romdata]
    add esi,034DA3h
    mov byte[esi],000h
.notaccelebrid
*/

/*Accele Brid (J)*/
if(0==strncmp((RomData+0x7FC0),"ACCELEBRID  " ,12))
{
	RomData[0x34DA2]=0;
	RomData[0x34DA3]=0;
}


/*
    mov esi,[romdata]
    add esi,07FC0h
    cmp dword[esi],'BATT'
    jne .notbattlegp
    cmp dword[esi+4],'LE G'
    jne .notbattlegp
    cmp dword[esi+8],'RAND'
    jne .notbattlegp
    mov esi,[romdata]
    add esi,018089h
    mov byte[esi],0FBh
    mov esi,[romdata]
    add esi,006C95h
    mov byte[esi],0FBh
.notbattlegp
*/

/*Battle Grand Prix (J)*/
if(0==strncmp((RomData+0x7FC0),"BATTLE GRAND" ,12))
{
	RomData[0x18089]=0xFB;
	RomData[0x6C95]=0xFB;
}


/*
    mov esi,[romdata]
    add esi,07FC0h
    cmp dword[esi],'Neug'
    jne .notneugiertrans
    cmp dword[esi+4],'ier '
    jne .notneugiertrans
    cmp dword[esi+8],'(tr.'
    jne .notneugiertrans

    mov esi,[romdata]
    add esi,0D4150h
    mov byte[esi],0F9h
.notneugiertrans

    mov esi,[romdata]
    add esi,07FC0h
    cmp dword[esi],'NEUG'
    jne .notneugier
    cmp dword[esi+4],'IER '
    jne .notneugier
    cmp dword[esi+8],'    '
    jne .notneugier

    mov esi,[romdata]
    add esi,0D4150h
    mov byte[esi],0F9h
.notneugier
*/

/*Neugier (J)*/
/*Also the English patched version*/
if(0==strncmp((RomData+0x7FC0),"NEUGIER     " ,12)||
   0==strncmp((RomData+0x7FC0),"Neugier (tr." ,12))
{
	RomData[0xD4150]=0xF9;
}


/*
    mov esi,[romdata]
    add esi,07FC0h
    cmp dword[esi],'HOME'
    jne .nothomealone
    cmp dword[esi+4],' ALO'
    jne .nothomealone
    mov esi,[romdata]
    add esi,0666Bh
    mov byte[esi],0EEh ; RTS instead of jumping to a rts 
    mov byte[esi+1],0BCh ; RTS instead of jumping to a rts 
.nothomealone
*/

/*Home Alone (J/E/U)*/
if(0==strncmp((RomData+0x7FC0),"HOME ALO" ,8))
{
	RomData[0x666B]=0xEE;
	RomData[0x666C]=0xBC;
}


/*
    mov esi,[romdata]
    add esi,0FFC0h
    cmp dword[esi],'EMER'
    jne .notemeralddragon
    cmp dword[esi+4],'ALD '
    jne .notemeralddragon
    cmp dword[esi+8],'DRAG'
    jne .notemeralddragon
    mov byte[ENVDisable],1
.notemeralddragon
*/

/*Emerald Dragon (J)*/
if(0==strncmp((RomData+0xFFC0),"EMERALD DRAG" ,12))
{
ENVDisable=1;
}

/*
    mov esi,[romdata]
    add esi,07FC0h
    cmp dword[esi],'YOSH'
    jne .notyoshi
    cmp dword[esi+8],'ISLA'
    jne .notyoshi
    jmp .yoshi
.notyoshi
    cmp dword[esi],'YOSS'
    jne .notyoshi2
    cmp dword[esi+8],'ISLA'
    jne .notyoshi2
.yoshi
    mov byte[hdmaearlstart],2
    mov byte[opexec268],116
    mov byte[opexec358],126
.notyoshi2
*/

/*Super Mario World 2 - Yoshi's Island (U/E)*/
/*Super Mario - Yossy Island (J)*/
/*All Variants*/
if(0==strncmp((RomData+0x7FC0),"YOSSY'S ISLA" ,12)||
   0==strncmp((RomData+0x7FC0),"YOSHI'S ISLA" ,12))

{
hdmaearlstart=2;
opexec268=116;
opexec358=126;
}

/*
    mov esi,[romdata]
    add esi,0FFC0h
    cmp dword[esi],'BUBS'
    jne .notbubsy2
    cmp dword[esi+4],'Y II'
    jne .notbubsy2
    mov byte[cycpb268],125
    mov byte[cycpb358],147
    mov byte[cycpbl2],125
    mov byte[cycpblt2],125
    mov byte[cycpbl],125
    mov byte[cycpblt],125
.notbubsy2
*/

/*Bubsy II (U/E).zip*/
if(0==strncmp((RomData+0xFFC0),"BUBSY II" ,8))
{
cycpb268=125;
cycpb358=147;
cycpbl2=125;
cycpblt2=125;
cycpbl=125;
cycpblt=125;

}


/*
    mov esi,[romdata]
    add esi,7FC0h
    cmp dword[esi],0DEB3B0CFh
    je .marvelous
    cmp dword[esi],'REND'
    jne .notrend
    mov byte[cycpb268],157
    mov byte[cycpb358],157
    mov byte[cycpbl2],157
    mov byte[cycpblt2],157
    mov byte[cycpbl],157
    mov byte[cycpblt],157
    jmp .notrend
.marvelous
.notrend
*/

/*Marvelous (J): note, this game isn't hacked in the asm*/
/*Rendering Ranger R2*/
/*
Marvelous-inclusive version
if(0==strncmp((RomData+0x7FC0),"\x0cf\x0bo\x0b3\x0de", 4)||
0==strncmp((RomData+0x7FC0),"REND", 4))
*/
if(0==strncmp((RomData+0x7FC0),"REND", 4))
{
cycpb268=157;
cycpb358=157;
cycpbl2=157;
cycpblt2=157;
cycpbl=157;
cycpblt=157;
}

/*
    mov esi,[romdata]
    add esi,07FC0h
    cmp dword[esi],'STAR'
    jne .notds9
    cmp dword[esi+4],' TRE'
    jne .notds9
    cmp dword[esi+8],'K: D'
    jne .notds9
    mov byte[opexec268],187
    mov byte[opexec358],187
.notds9
*/

/*Star Trek - Deep Space Nine - Crossroads of Time (U/E)*/
if(0==strncmp((RomData+0x7FC0),"STAR TREK: D" ,12))
{
opexec268=187;
opexec358=187;
}


/*
    mov esi,[romdata]
    add esi,0FFC0h
    cmp dword[esi],'CLAY'
    jne .notclayfighter
    cmp dword[esi+4],' FIG'
    jne .notclayfighter
    cmp dword[esi+8],'HTER'
    jne .notclayfighter
    cmp dword[esi+12],'    '
    jne .notclayfighter
    mov esi,[romdata]  ; In intro
    add esi,01A10B9h
    mov byte[esi],0DEh

    mov esi,[romdata]  ; In game
    add esi,01A1996h
    mov byte[esi],0DEh
    mov esi,[romdata]
    add esi,01AE563h
    mov byte[esi],0DEh
    mov esi,[romdata]
    add esi,01AE600h
    mov byte[esi],0DEh
.notclayfighter
*/

/*Clay Fighter (U)*/
/*Foreign Versions are CLAYFIGHTER with no space*/
if(0==strncmp((RomData+0xFFC0),"CLAY FIGHTER    " ,16))
{
RomData[0x01A10B9]=0xDE;
RomData[0x01A1996]=0xDE;
RomData[0x01AE563]=0xDE;
RomData[0x01AE600]=0xDE;
}


/*
    mov esi,[romdata]
    add esi,0FFC0h
    cmp dword[esi],'Baha'
    jne .notbahamutlagoon
    cmp dword[esi+4],'mut '
    jne .notbahamutlagoon
    cmp dword[esi+8],'Lago'
    jne .notbahamutlagoon
    mov esi,[romdata]
    add esi,010254h
    mov byte[esi],0EEh
.notbahamutlagoon
*/

/*Bahamut Lagoon (J) and all known translations*/
if(0==strncmp((RomData+0xFFC0),"Bahamut Lago" ,12))
{
RomData[0x010254]=0xEE;
}


/*
    mov esi,[romdata]
    add esi,07FC0h
    cmp dword[esi],'MORT'
    jne .notmk
    cmp dword[esi+4],'AL K'
    jne .notmk
    cmp dword[esi+8],'OMBA'
    jne .notmk
    cmp dword[esi+12],'T   '
    jne .notmk
    mov byte[disablehdma],1
.notmk
*/
/*
    mov esi,[romdata]
    add esi,07FC0h
    cmp dword[esi],'DRAG'
    jne .notdq5
    cmp dword[esi+4],'ONQU'
    jne .notdq5
    cmp dword[esi+8],'EST5'
    jne .notdq5
    mov byte[disablehdma],1
.notdq5
*/


/*Mortal Kombat (J/U/E)*/
/*Super Punch-Out*/
/*Dragon Quest 5 (J)*/
if(0==strncmp((RomData+0x7FC0),"DRAGONQUEST5" ,12)||
   0==strncmp((RomData+0x7FC0),"MORTAL KOMBAT   " ,16) ||
   0==strncmp((RomData+0x7FC0), "Super Punch-Out!!   ", 20))
{
  disablehdma=1;
}

/*
    mov esi,[romdata]
    add esi,0FFC0h
    cmp dword[esi],'CLOC'
    jne .notclocktower
    cmp dword[esi+4],'K TO'
    jne .notclocktower
    cmp dword[esi+8],'WER '
    jne .notclocktower
    mov byte[opexec268],187
    mov byte[opexec358],182
    mov byte[opexec268cph],47
    mov byte[opexec358cph],47
.notclocktower
*/

/*Clock Tower (J) and AGTP translation*/
if(0==strncmp((RomData+0xFFC0),"CLOCK TOWER " ,12))
{
opexec268=187;
opexec358=182;
opexec268cph=47;
opexec358cph=47;

}


/*
    ; Lamborgini Challenge - -p 110
    mov esi,[romdata]
    add esi,07FC0h
    cmp dword[esi],'DIGI'
    jne .nodigitaldevil
    mov byte[opexec268],187
    mov byte[opexec358],187
.nodigitaldevil
*/

/*Shin Megami Tensei (J)*/
/*Shin Megami Tensei if... (J)*/
/*Shin Megami Tensei II (J)*/
/*Does not fix the AGTP translations*/
if(0==strncmp((RomData+0x7FC0),"DIGI" ,4))
{
opexec268=187;
opexec358=187;
}

/*
    mov esi,[romdata]
    add esi,7FC0h
    cmp dword[esi],'SP F'
    jne .notfmatchtennis
    mov byte[cycpb268],145
    mov byte[cycpb358],147
    mov byte[cycpbl2],145
    mov byte[cycpblt2],145
    mov byte[cycpbl],145
    mov byte[cycpblt],145
.notfmatchtennis
*/

/*Super Final Match Tennis (J)*/
  if(0==strncmp((RomData+0x7FC0),"SP F", 4))
  {
    cycpb268=145;
    cycpb358=147;
    cycpbl2=145;
    cycpblt2=145;
    cycpbl=145;
    cycpblt=145;
  }


/*
    mov esi,[romdata]
    add esi,7FC0h
    cmp dword[esi],'DEAD'
    je .deaddance
    cmp dword[esi],'TUFF'
    jne .nottuffenuff
.deaddance
    mov byte[cycpb268],75
    mov byte[cycpb358],77
    mov byte[cycpbl2],75
    mov byte[cycpblt2],75
    mov byte[cycpbl],75
    mov byte[cycpblt],75
.nottuffenuff
*/

/*Tuff E Nuff (U/E)*/
/*Dead Dance (J)*/
  if(0==strncmp((RomData+0x7FC0),"DEAD", 4)||
     0==strncmp((RomData+0x7FC0),"TUFF", 4))
  {
    cycpb268=75;
    cycpb358=77;
    cycpbl2=75;
    cycpblt2=75;
    cycpbl=75;
    cycpblt=75;
  }


/*
    cmp byte[DSP1Type],0
    je .notdis
    mov byte[disablehdma],1
.notdis
*/

  if(DSP1Type!=0)
    disablehdma=1;

/*
    ; Here are the individual game hacks.  Some of these probably can
    ;   be removed since many of them were created very early in ZSNES
    ;   development.

    cmp dword[esi+0FFC0h],'FINA'
    jne .notff
.notff
    mov esi,[romdata]
    add esi,9AB0h
    cmp dword[esi],0F00F2908h
    jne .notff3
    mov byte[opexec268],163
    mov byte[opexec358],157
    mov byte[opexec268cph],39
    mov byte[opexec358cph],39
.notff3
*/

if(0==strncmp((RomData+0xFFC0), "FINA", 4)&& *(unsigned int*)(RomData+0x9AB0)==0xF00F2908)
{
opexec268=163;
opexec358=157;
opexec268cph=39;
opexec358cph=39;
}

/*
    ; Earth Worm Jim 2 - IRQ hack (reduce sound static)
    mov esi,[romdata]
    add esi,0FFC0h
    mov edi,.ewj2head
    call Checkheadersame
    cmp al,0
    jne .noromhead2
    mov esi,[romdata]
    add esi,02A9C1Ah
    mov word [esi],0
    add esi, 5
    mov word [esi],0
    mov dword [ewj2hack],1
.noromhead2
*/

/*Earthworm Jim 2 (all regions?)*/
if(0==strncmp((RomData+0x7FC0),"EARTHWORM JIM 2     " ,20))
{
romdata[0x2A9C1A]=0;
romdata[0x2A9C1B]=0;
romdata[0x2A9C1A+0x5]=0;
romdata[0x2A9C1B+0x5]=0;
ewj2hack=1;
}


/*
    ; Lamborgini Challenge - -p 110
    mov esi,[romdata]
    add esi,07FC0h
    mov edi,.lambhead
    call Checkheadersame
    cmp al,0
    jne .noromheadlamb
    mov byte[opexec268],187
    mov byte[opexec358],187
.noromheadlamb
*/

/*Lamborghini - American Challenge (U/E)*/
if(0==strncmp((RomData+0x7FC0), "LAMBORGHINI AMERICAN", 20))
{
opexec268=187;
opexec358=187;
}

/*
    ; Addams Family Values - -p 75
    mov esi,[romdata]
    add esi,07FC0h
    mov edi,.adm3head
    call Checkheadersame
    cmp al,0
    jne .noromheadadm3
    mov byte[opexec268],120
    mov byte[opexec358],100
.noromheadadm3
*/

/*Addams Family Values (U/E)*/
if(0==strncmp((RomData+0x7FC0), "ADDAMS FAMILY VALUES", 20))
{
opexec268=120;
opexec358=100;
}


/*
    ; Bubsy -p 115
    mov esi,[romdata]
    add esi,07FC0h
    mov edi,.bubshead
    call Checkheadersame
    cmp al,0
    jne .noromhead3
    mov byte[opexec268],220
    mov byte[opexec358],220
    mov byte[opexec268cph],64
    mov byte[opexec358cph],64
.noromhead3
*/

/*Bubsy in Claws Encounters of the Furred Kind (U/E)*/
if(0==strncmp((RomData+0x7FC0), "Bubsy               ", 20))
{
opexec268=220;
opexec358=220;
opexec268cph=64;
opexec358cph=64;
}

/*
    ; BToad vs DD - 197/192/47/47 -p 120
    mov esi,[romdata]
;    add esi,07FC0h
    cmp dword[esi+640h],0E2FA85F6h
    jne .noromhead4
    mov byte[opexec268],187
    mov byte[opexec358],187
    mov byte[opexec268cph],47
    mov byte[opexec358cph],47
    mov bl,[cycpb358]
    mov byte[cycpblt],bl
    mov al,[opexec358]
    mov byte[cycpl],al
.noromhead4
*/

/*Battletoads & Double Dragon (extent unknown!)*/
if(0xE2FA85F6==*(unsigned int*)(RomData+0x640+0x7FC0))
{
    opexec268=187;
    opexec358=187;
    opexec268cph=47;
    opexec358cph=47;
    cycpblt=cycpb358;
    cycpl=opexec358;
}

/*
    ; Chrono Trigger - 187/182/47/47 -p 120 / res change clear screen disable
    mov esi,[romdata]
;    add esi,0FFC0h
    cmp dword[esi+8640h],0E243728Dh
    jne .noromhead6
    cmp byte[opexec358],182
    ja .noromhead6
    mov byte[ClearScreenSkip],1
    mov byte[opexec268],187
    mov byte[opexec358],182
    mov byte[opexec268cph],47
    mov byte[opexec358cph],47
    mov bl,[cycpb358]
    mov byte[cycpblt],bl
    mov al,[opexec358]
    mov byte[cycpl],al
.noromhead6
*/

/*Battletoads & Double Dragon (extent unknown!)*/
if(*(unsigned int*)(RomData+0xFFC0+0x8640)==0xE243728D&&opexec358<=182)
{
    ClearScreenSkip=1;
    opexec268=187;
    opexec358=182;
    opexec268cph=47;
    opexec358cph=47;
    cycpblt=cycpb358;
    cycpl=opexec358;
}

/*
    ; Front Mission - -p 140
    mov esi,[romdata]
    add esi,0FFC0h
    cmp dword[esi],0C4DDDBCCh
    jne .noromheadfm
    cmp dword[esi+4],0AEBCAFD0h
    jne .noromheadfm
    mov byte[opexec268],226
    mov byte[opexec358],226
    mov byte[opexec268cph],80
    mov byte[opexec358cph],80
.noromheadfm

    ; Front Mission - -p 140
    mov esi,[romdata]
    add esi,0FFC0h
    cmp dword[esi],'FRON'
    jne .noromheadfm2
    cmp dword[esi+4],'T MI'
    jne .noromheadfm2
    mov byte[opexec268],226
    mov byte[opexec358],226
    mov byte[opexec268cph],80
    mov byte[opexec358cph],80
.noromheadfm2
*/

if(((0==strncmp((RomData+0xFFC0), "\x0cc\x0db\x0dd\x0c4\x0d0\x0af\x0bc\x0ae\x0dd           ", 20)))||
   (0==strncmp((RomData+0xFFC0), "FRONT MI", 8)))
{
opexec268=226;
opexec358=226;
opexec268cph=80;
opexec358cph=80;
}

/*
    ; Clayfighter 2 - -p 120
    mov esi,[romdata]
    add esi,0FFC0h
    cmp dword[esi],'Clay'
    jne .noromheadcf2
    cmp byte[esi+12],'2'
    jne .noromheadcf2
    mov byte[opexec268],187
    mov byte[opexec358],182
    mov byte[opexec268cph],47
    mov byte[opexec358cph],47
.noromheadcf2
*/

if(0==strncmp((RomData+0xFFC0), "Clay", 4) && ('2'==*(RomData+0xFFCC)))
{
    opexec268=187;
    opexec358=182;
    opexec268cph=47;
    opexec358cph=47;
}

/*
    mov esi,[romdata]
    add esi,0FFC0h
    cmp dword[esi],'Donk'
    jne .noromheaddk
    cmp word[ramsize],2048
    jne .noromheaddk
    mov word[ramsize],4096
    mov word[ramsizeand],4095
.noromheaddk
    ret
*/

if(ramsize!=2048 && 0==strncmp((RomData+0xFFC0), "Donk", 4))
{
ramsize=4096;
ramsizeand=4095;
}
/*.ewj2head db 58,62,45,43,55,40,48,45,50,95,53,54,50,95,77,95,95,95,95,95*/
/*.lambhead db 51,62,50,61,48,45,56,55,54,49,54,95,62,50,58,45,54,60,62,49*/
/*.adm3head db 62,59,59,62,50,44,95,57,62,50,54,51,38,95,41,62,51,42,58,44*/
/*
.bubshead db 61,10,29,12,06,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95
.btvdhead db 61,62,43,43,51,58,43,48,62,59,44,95,59,81,59,81,95,95,95,95
.pouthead db 44,10,15,26,13,95,47,10,17,28,23,82,48,10,11,94,94,95,95,95
.drcxhead db 41,62,50,47,54,45,58,44,95,52,54,44,44,95,95,95,95,95,95,95
.drx2head db 60,62,44,43,51,58,41,62,49,54,62,95,59,45,62,60,42,51,62,95
.ctrghead db 60,55,45,48,49,48,95,43,45,54,56,56,58,45,95,95,95,95,95,95
.fmishead db 57,13,16,17,11,95,50,22,12,12,22,16,17,95,87,58,86,95,95,95
*/
return;
}