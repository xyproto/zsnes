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
#include <sys/stat.h>
#define DIR_SLASH "\\"
#endif
#include "gblvars.h"

extern unsigned int Curtableaddr, tableA[256];

void PrepareOffset()
{
  Curtableaddr -= (unsigned int)tableA;
}

void ResetOffset()
{
  Curtableaddr += (unsigned int)tableA;
}

extern unsigned int snesmmap[256], snesmap2[256];

void BankSwitchSDD1C (unsigned char bankval, unsigned int offset)
{
  unsigned int curbankval = bankval, i;

  curbankval &= 7;
  curbankval <<= 20;
  curbankval += (unsigned int)romdata;

  for (i=0; i<16 ; i++)
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
    BankSwitchSDD1C(SDD1BankA, 0x0C0);
    BankSwitchSDD1C(SDD1BankB, 0x0D0);
    BankSwitchSDD1C(SDD1BankC, 0x0E0);
    BankSwitchSDD1C(SDD1BankD, 0x0F0);
  }
}

extern void (*Bank0datr8[256])(), (*Bank0datr16[256])(), (*Bank0datw8[256])();
extern void (*Bank0datw16[256])(), *DPageR8, *DPageR16, *DPageW8, *DPageW16;
extern unsigned int xdb, xpb, xs, xx, xy, xd;
extern unsigned short oamaddrt, xat, xst, xdt, xxt, xyt;
extern unsigned char xdbt, xpbt;

void UpdateDPageC()
{
  DPageR8 = Bank0datr8[(xd >> 8) & 0xFF];
  DPageR16 = Bank0datr16[(xd >> 8) & 0xFF];
  DPageW8 = Bank0datw8[(xd >> 8) & 0xFF];
  DPageW16 = Bank0datw16[(xd >> 8) & 0xFF];
}

extern unsigned int SA1xd;
extern void *SA1DPageR8, *SA1DPageR16, *SA1DPageW8, *SA1DPageW16;

void SA1UpdateDPageC()
{
  SA1DPageR8 = Bank0datr8[(SA1xd >> 8) & 0xFF];
  SA1DPageR16 = Bank0datr16[(SA1xd >> 8) & 0xFF];
  SA1DPageW8 = Bank0datw8[(SA1xd >> 8) & 0xFF];
  SA1DPageW16 = Bank0datw16[(SA1xd >> 8) & 0xFF];
}

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

#define bit_test(byte, checkbit) (byte & (1 << checkbit)) ? 1 : 0

extern unsigned int GlobalVL, GlobalVR, EchoVL, EchoVR, EchoRate[16], MaxEcho;
extern unsigned int EchoFB, NoiseSpeeds[32], dspPAdj, NoiseInc, bg1ptrx;
extern unsigned int bg1ptry, bg2ptrx, bg2ptry, bg3ptrx, bg3ptry, bg4ptrx;
extern unsigned int bg4ptry;
extern signed int FIRTAPVal0, FIRTAPVal1, FIRTAPVal2, FIRTAPVal3, FIRTAPVal4;
extern signed int FIRTAPVal5, FIRTAPVal6, FIRTAPVal7;
extern unsigned short VolumeConvTable[32768], bg1ptr, bg1ptrb, bg1ptrc;
extern unsigned short bg2ptr, bg2ptrb, bg2ptrc, bg3ptr, bg3ptrb, bg3ptrc;
extern unsigned short bg4ptr, bg4ptrb, bg4ptrc;
extern unsigned char VolumeTableb[256], MusicVol, Voice0Status;
extern unsigned char Voice1Status, Voice2Status, Voice3Status, Voice4Status;
extern unsigned char Voice5Status, Voice6Status, Voice7Status, Voice0Noise;
extern unsigned char Voice1Noise, Voice2Noise, Voice3Noise, Voice4Noise;
extern unsigned char Voice5Noise, Voice6Noise, Voice7Noise, bgtilesz;
extern unsigned char BG116x16t, BG216x16t, BG316x16t, BG416x16t, vramincby8on;
extern unsigned char vramincr;

extern void (**regptw)();
void reg2118();
void reg2118inc();
void reg2118inc8();
void reg2118inc8inc();
void reg2119();
void reg2119inc();
void reg2119inc8();
void reg2119inc8inc();

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

  if (block && 0xC0)
  {
    Voice0Status = Voice1Status = Voice2Status = Voice3Status = 0;
    Voice4Status = Voice5Status = Voice6Status = Voice7Status = 0;
  }

  NoiseInc = (((NoiseSpeeds[(block & 0x1F)] * dspPAdj) >> 17) & 0xFFFFFFFF);

  Voice0Noise = bit_test(DSPMem[0x3D], 0);
  Voice1Noise = bit_test(DSPMem[0x3D], 1);
  Voice2Noise = bit_test(DSPMem[0x3D], 2);
  Voice3Noise = bit_test(DSPMem[0x3D], 3);
  Voice4Noise = bit_test(DSPMem[0x3D], 4);
  Voice5Noise = bit_test(DSPMem[0x3D], 5);
  Voice6Noise = bit_test(DSPMem[0x3D], 6);
  Voice7Noise = bit_test(DSPMem[0x3D], 7);

  bg1ptrx = bg1ptrb - bg1ptr;
  bg1ptry = bg1ptrc - bg1ptr;
  bg2ptrx = bg2ptrb - bg2ptr;
  bg2ptry = bg2ptrc - bg2ptr;
  bg3ptrx = bg3ptrb - bg3ptr;
  bg3ptry = bg3ptrc - bg3ptr;
  bg4ptrx = bg4ptrb - bg4ptr;
  bg4ptry = bg4ptrc - bg4ptr;

  // 16x16 tiles
  BG116x16t = bit_test(bgtilesz, 0);
  BG216x16t = bit_test(bgtilesz, 1);
  BG316x16t = bit_test(bgtilesz, 2);
  BG416x16t = bit_test(bgtilesz, 3);

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

extern void (*memtabler8[256])();
extern void (*memtablew8[256])();
extern void (*memtabler16[256])();
extern void (*memtablew16[256])();

void regaccessbankr8(), regaccessbankr8SA1();
void memaccessbankr8();
void sramaccessbankr8(), SA1RAMaccessbankr8(), SA1RAMaccessbankr8b();
void wramaccessbankr8(), eramaccessbankr8();

void regaccessbankw8(), regaccessbankw8SA1();
void memaccessbankw8();
void sramaccessbankw8(), SA1RAMaccessbankw8(), SA1RAMaccessbankw8b();
void wramaccessbankw8(), eramaccessbankw8();

void regaccessbankr16(), regaccessbankr16SA1();
void memaccessbankr16();
void sramaccessbankr16(), SA1RAMaccessbankr16(), SA1RAMaccessbankr16b();
void wramaccessbankr16(), eramaccessbankr16();

void regaccessbankw16(), regaccessbankw16SA1();
void memaccessbankw16();
void sramaccessbankw16(), SA1RAMaccessbankw16(), SA1RAMaccessbankw16b();
void wramaccessbankw16(), eramaccessbankw16();

/*
rep_stosd is my name for a 'copy <num> times a function pointer <func_ptr> into
a function pointer array <dest>' function, in honour of the almighty asm
instruction rep stosd, which is able to do that (and much more).
Since ZSNES is just full of func pointer arrays, it'll probably come in handy.
*/

void rep_stosd(void (**dest)(), void (*func_ptr), unsigned int num)
{
  while (num--)	{ dest[num] = func_ptr; }
}

void SetAddressingModes()
{
	// set 8-bit read memory tables                    banks
  rep_stosd(memtabler8+0x00, regaccessbankr8,  0x40);   // 00 - 3F
  rep_stosd(memtabler8+0x40, memaccessbankr8,  0x30);   // 40 - 6F
  rep_stosd(memtabler8+0x70, sramaccessbankr8, 0x08);   // 70 - 77
  rep_stosd(memtabler8+0x78, memaccessbankr8,  0x06);   // 78 - 7D
  memtabler8[0x7E] = wramaccessbankr8;                  // 7E
  memtabler8[0x7F] = eramaccessbankr8;                  // 7F
  rep_stosd(memtabler8+0x80, regaccessbankr8,  0x40);   // 80 - BF
  rep_stosd(memtabler8+0xC0, memaccessbankr8,  0x40);   // C0 - FF

	// set 8-bit write memory tables                   banks
  rep_stosd(memtablew8+0x00, regaccessbankw8,  0x40);   // 00 - 3F
  rep_stosd(memtablew8+0x40, memaccessbankw8,  0x30);   // 40 - 6F
  rep_stosd(memtablew8+0x70, sramaccessbankw8, 0x08);   // 70 - 77
  rep_stosd(memtablew8+0x78, memaccessbankw8,  0x06);   // 78 - 7D
  memtablew8[0x7E] = wramaccessbankw8;                  // 7E
  memtablew8[0x7F] = eramaccessbankw8;                  // 7F
  rep_stosd(memtablew8+0x80, regaccessbankw8,  0x40);   // 80 - BF
  rep_stosd(memtablew8+0xC0, memaccessbankw8,  0x40);   // C0 - FF

	// set 16-bit read memory tables                   banks
  rep_stosd(memtabler16+0x00, regaccessbankr16,  0x40); // 00 - 3F
  rep_stosd(memtabler16+0x40, memaccessbankr16,  0x30); // 40 - 6F
  rep_stosd(memtabler16+0x70, sramaccessbankr16, 0x08); // 70 - 77
  rep_stosd(memtabler16+0x78, memaccessbankr16,  0x06); // 78 - 7D
  memtabler16[0x7E] = wramaccessbankr16;                // 7E
  memtabler16[0x7F] = eramaccessbankr16;                // 7F
  rep_stosd(memtabler16+0x80, regaccessbankr16,  0x40); // 80 - BF
  rep_stosd(memtabler16+0xC0, memaccessbankr16,  0x40); // C0 - FF

	// set 16-bit write memory tables                  banks
  rep_stosd(memtablew16+0x00, regaccessbankw16,  0x40); // 00 - 3F
  rep_stosd(memtablew16+0x40, memaccessbankw16,  0x30); // 40 - 6F
  rep_stosd(memtablew16+0x70, sramaccessbankw16, 0x08); // 70 - 77
  rep_stosd(memtablew16+0x78, memaccessbankw16,  0x06); // 78 - 7D
  memtablew16[0x7E] = wramaccessbankw16;                // 7E
  memtablew16[0x7F] = eramaccessbankw16;                // 7F
  rep_stosd(memtablew16+0x80, regaccessbankw16,  0x40); // 80 - BF
  rep_stosd(memtablew16+0xC0, memaccessbankw16,  0x40); // C0 - FF
}

void SetAddressingModesSA1()
{
	// set 8-bit read memory tables                       banks
  rep_stosd(memtabler8+0x00, regaccessbankr8SA1,  0x40);   // 00 - 3F
  rep_stosd(memtabler8+0x40, SA1RAMaccessbankr8,  0x20);   // 40 - 5F
  rep_stosd(memtabler8+0x60, SA1RAMaccessbankr8b, 0x10);   // 60 - 6F
  rep_stosd(memtabler8+0x70, sramaccessbankr8,    0x08);   // 70 - 77
  rep_stosd(memtabler8+0x78, memaccessbankr8,     0x06);   // 78 - 7D
  memtabler8[0x7E] = wramaccessbankr8;                     // 7E
  memtabler8[0x7F] = eramaccessbankr8;                     // 7F
  rep_stosd(memtabler8+0x80, regaccessbankr8SA1,  0x40);   // 80 - BF
  rep_stosd(memtabler8+0xC0, memaccessbankr8,     0x40);   // C0 - FF

	// set 8-bit write memory tables                      banks
  rep_stosd(memtablew8+0x00, regaccessbankw8SA1,  0x40);   // 00 - 3F
  rep_stosd(memtablew8+0x40, SA1RAMaccessbankw8,  0x20);   // 40 - 5F
  rep_stosd(memtablew8+0x60, SA1RAMaccessbankw8b, 0x10);   // 60 - 6F
  rep_stosd(memtablew8+0x70, sramaccessbankw8,    0x08);   // 70 - 77
  rep_stosd(memtablew8+0x78, memaccessbankw8,     0x06);   // 78 - 7D
  memtablew8[0x7E] = wramaccessbankw8;                     // 7E
  memtablew8[0x7F] = eramaccessbankw8;                     // 7F
  rep_stosd(memtablew8+0x80, regaccessbankw8SA1,  0x40);   // 80 - BF
  rep_stosd(memtablew8+0xC0, memaccessbankw8,     0x40);   // C0 - FF

	// set 16-bit read memory tables                      banks
  rep_stosd(memtabler16+0x00, regaccessbankr16SA1,  0x40); // 00 - 3F
  rep_stosd(memtabler16+0x40, SA1RAMaccessbankr16,  0x20); // 40 - 5F
  rep_stosd(memtabler16+0x60, SA1RAMaccessbankr16b, 0x10); // 60 - 6F
  rep_stosd(memtabler16+0x70, sramaccessbankr16,    0x08); // 70 - 77
  rep_stosd(memtabler16+0x78, memaccessbankr16,     0x06); // 78 - 7D
  memtabler16[0x7E] = wramaccessbankr16;                   // 7E
  memtabler16[0x7F] = eramaccessbankr16;                   // 7F
  rep_stosd(memtabler16+0x80, regaccessbankr16SA1,  0x40); // 80 - BF
  rep_stosd(memtabler16+0xC0, memaccessbankr16,     0x40); // C0 - FF

	// set 16-bit write memory tables                     banks
  rep_stosd(memtablew16+0x00, regaccessbankw16SA1,  0x40); // 00 - 3F
  rep_stosd(memtablew16+0x40, SA1RAMaccessbankw16,  0x20); // 40 - 5F
  rep_stosd(memtablew16+0x60, SA1RAMaccessbankw16b, 0x10); // 60 - 6F
  rep_stosd(memtablew16+0x70, sramaccessbankw16,    0x08); // 70 - 77
  rep_stosd(memtablew16+0x78, memaccessbankw16,     0x06); // 78 - 7D
  memtablew16[0x7E] = wramaccessbankw16;                   // 7E
  memtablew16[0x7F] = eramaccessbankw16;                   // 7F
  rep_stosd(memtablew16+0x80, regaccessbankw16SA1,  0x40); // 80 - BF
  rep_stosd(memtablew16+0xC0, memaccessbankw16,     0x40); // C0 - FF
}

void membank0r8ram(), membank0r8ramSA1();
void membank0r8reg(), membank0r8inv(), membank0r8chip();
void membank0r8rom(), membank0r8romram();

void membank0w8ram(), membank0w8ramSA1();
void membank0w8reg(), membank0w8inv(), membank0w8chip();
void membank0w8rom(), membank0w8romram();

void membank0r16ram(), membank0r16ramSA1();
void membank0r16reg(), membank0r16inv(), membank0r16chip();
void membank0r16rom(), membank0r16romram();

void membank0w16ram(), membank0w16ramSA1();
void membank0w16reg(), membank0w16inv(), membank0w16chip();
void membank0w16rom(), membank0w16romram();

void GenerateBank0Table()
{
  rep_stosd(Bank0datr8+0x00, membank0r8ram,  0x20);   // 00 - 1F
  rep_stosd(Bank0datr8+0x20, membank0r8reg,  0x28);   // 20 - 47
  rep_stosd(Bank0datr8+0x48, membank0r8inv,  0x17);   // 48 - 5E
  rep_stosd(Bank0datr8+0x5F, membank0r8chip, 0x1F);   // 5F - 7D
  rep_stosd(Bank0datr8+0x7E, membank0r8rom,  0x81);   // 7E - FE
  Bank0datr8[0xFF] = membank0r8romram;                // FF

  rep_stosd(Bank0datw8+0x00, membank0w8ram,  0x20);   // 00 - 1F
  rep_stosd(Bank0datw8+0x20, membank0w8reg,  0x28);   // 20 - 47
  rep_stosd(Bank0datw8+0x48, membank0w8inv,  0x17);   // 48 - 5E
  rep_stosd(Bank0datw8+0x5F, membank0w8chip, 0x1F);   // 5F - 7D
  rep_stosd(Bank0datw8+0x7E, membank0w8rom,  0x81);   // 7E - FE
  Bank0datw8[0xFF] = membank0w8romram;                // FF

  rep_stosd(Bank0datr16+0x00, membank0r16ram,  0x20); // 00 - 1F
  rep_stosd(Bank0datr16+0x20, membank0r16reg,  0x28); // 20 - 47
  rep_stosd(Bank0datr16+0x48, membank0r16inv,  0x17); // 48 - 5E
  rep_stosd(Bank0datr16+0x5F, membank0r16chip, 0x1F); // 5F - 7D
  rep_stosd(Bank0datr16+0x7E, membank0r16rom,  0x81); // 7E - FE
  Bank0datr16[0xFF] = membank0r16romram;              // FF

  rep_stosd(Bank0datw16+0x00, membank0w16ram,  0x20); // 00 - 1F
  rep_stosd(Bank0datw16+0x20, membank0w16reg,  0x28); // 20 - 47
  rep_stosd(Bank0datw16+0x48, membank0w16inv,  0x17); // 48 - 5E
  rep_stosd(Bank0datw16+0x5F, membank0w16chip, 0x1F); // 5F - 7D
  rep_stosd(Bank0datw16+0x7E, membank0w16rom,  0x81); // 7E - FE
  Bank0datw16[0xFF] = membank0w16romram;              // FF
}

void GenerateBank0TableSA1()
{
  rep_stosd(Bank0datr8,  membank0r8ramSA1,  0x20); // 00 - 1F
  rep_stosd(Bank0datw8,  membank0w8ramSA1,  0x20); // 00 - 1F
  rep_stosd(Bank0datr16, membank0r16ramSA1, 0x20); // 00 - 1F
  rep_stosd(Bank0datw16, membank0w16ramSA1, 0x20); // 00 - 1F
}
