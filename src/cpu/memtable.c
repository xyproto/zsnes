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
#include "../gblhdr.h"
#define DIR_SLASH "/"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#define DIR_SLASH "\\"
#endif
#include "memtable.h"
#include "../gblvars.h"

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

extern void (*Bank0datr8[256])(), (*Bank0datr16[256])(), (*Bank0datw8[256])(), (*Bank0datw16[256])();
extern void *DPageR8, *DPageR16, *DPageW8, *DPageW16;

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
extern int FIRTAPVal0, FIRTAPVal1, FIRTAPVal2, FIRTAPVal3, FIRTAPVal4;
extern int FIRTAPVal5, FIRTAPVal6, FIRTAPVal7;
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
  FIRTAPVal0 = (char)DSPMem[0x0F];
  FIRTAPVal1 = (char)DSPMem[0x1F];
  FIRTAPVal2 = (char)DSPMem[0x2F];
  FIRTAPVal3 = (char)DSPMem[0x3F];
  FIRTAPVal4 = (char)DSPMem[0x4F];
  FIRTAPVal5 = (char)DSPMem[0x5F];
  FIRTAPVal6 = (char)DSPMem[0x6F];
  FIRTAPVal7 = (char)DSPMem[0x7F];

  // Noise
  block = DSPMem[0x6C];
  DSPMem[0x6C] &= 0x7F;

  if (block & 0xC0)
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

void regaccessbankr8(), regaccessbankw8(), regaccessbankr16(), regaccessbankw16();
void memaccessbankr8(), memaccessbankw8(), memaccessbankr16(), memaccessbankw16();
void wramaccessbankr8(), wramaccessbankw8(), wramaccessbankr16(), wramaccessbankw16();
void sramaccessbankr8(), sramaccessbankw8(), sramaccessbankr16(), sramaccessbankw16();
void eramaccessbankr8(), eramaccessbankw8(), eramaccessbankr16(), eramaccessbankw16();

void regaccessbankr8SA1(), regaccessbankw8SA1(), regaccessbankr16SA1(), regaccessbankw16SA1();
void SA1RAMaccessbankr8(), SA1RAMaccessbankw8(), SA1RAMaccessbankr16(), SA1RAMaccessbankw16();
void SA1RAMaccessbankr8b(), SA1RAMaccessbankw8b(), SA1RAMaccessbankr16b(), SA1RAMaccessbankw16b();

void sramaccessbankr8s(), sramaccessbankw8s(), sramaccessbankr16s(), sramaccessbankw16s();
void DSP1Read8b3F(), DSP1Write8b3F(), DSP1Read16b3F(), DSP1Write16b3F();
void DSP2Read8b(), DSP2Write8b(), DSP2Read16b(), DSP2Write16b();
void DSP3Read8b(), DSP3Write8b(), DSP3Read16b(), DSP3Write16b();
void DSP4Read8b(), DSP4Write8b(), DSP4Read16b(), DSP4Write16b();
void setaaccessbankr8(), setaaccessbankw8(), setaaccessbankr16(), setaaccessbankw16();
void setaaccessbankr8a(), setaaccessbankw8a(), setaaccessbankr16a(), setaaccessbankw16a();
void Seta11Read8_60(), Seta11Write8_60(), Seta11Read16_60(), Seta11Write16_60();
void Seta11Read8_68(), Seta11Write8_68(), Seta11Read16_68(), Seta11Write16_68();
void sfxaccessbankr8(), sfxaccessbankw8(), sfxaccessbankr16(), sfxaccessbankw16();
void sfxaccessbankr8b(), sfxaccessbankw8b(), sfxaccessbankr16b(), sfxaccessbankw16b();
void sfxaccessbankr8c(), sfxaccessbankw8c(), sfxaccessbankr16c(), sfxaccessbankw16c();
void sfxaccessbankr8d(), sfxaccessbankw8d(), sfxaccessbankr16d(), sfxaccessbankw16d();
void OBC1Read8b(), OBC1Write8b(), OBC1Read16b(), OBC1Write16b();
void C4Read8b(), C4Write8b(), C4Read16b(), C4Write16b();
void memaccessspc7110r8(), memaccessspc7110r16(), memaccessspc7110w8(), memaccessspc7110w16();
void SPC7110ReadSRAM8b(), SPC7110ReadSRAM16b(), SPC7110WriteSRAM8b(), SPC7110WriteSRAM16b();
void stsramr8(), stsramr16(), stsramw8(), stsramw16();
void stsramr8b(), stsramr16b(), stsramw8b(), stsramw16b();

mrwp regbank = { regaccessbankr8, regaccessbankw8, regaccessbankr16, regaccessbankw16 };
mrwp membank = { memaccessbankr8, memaccessbankw8, memaccessbankr16, memaccessbankw16 };
mrwp wrambank = { wramaccessbankr8, wramaccessbankw8, wramaccessbankr16, wramaccessbankw16 };
mrwp srambank = { sramaccessbankr8, sramaccessbankw8, sramaccessbankr16, sramaccessbankw16 };
mrwp erambank = { eramaccessbankr8, eramaccessbankw8, eramaccessbankr16, eramaccessbankw16 };

mrwp sa1regbank = { regaccessbankr8SA1, regaccessbankw8SA1, regaccessbankr16SA1, regaccessbankw16SA1 };
mrwp sa1rambank = { SA1RAMaccessbankr8, SA1RAMaccessbankw8, SA1RAMaccessbankr16, SA1RAMaccessbankw16 };
mrwp sa1rambankb = { SA1RAMaccessbankr8b, SA1RAMaccessbankw8b, SA1RAMaccessbankr16b, SA1RAMaccessbankw16b };

mrwp sramsbank = { sramaccessbankr8s, sramaccessbankw8s, sramaccessbankr16s, sramaccessbankw16s };
mrwp dsp1bank = { DSP1Read8b3F, DSP1Write8b3F, DSP1Read16b3F, DSP1Write16b3F };
mrwp dsp2bank = { DSP2Read8b, DSP2Write8b, DSP2Read16b, DSP2Write16b };
mrwp dsp3bank = { DSP3Read8b, DSP3Write8b, DSP3Read16b, DSP3Write16b };
mrwp dsp4bank = { DSP4Read8b, DSP4Write8b, DSP4Read16b, DSP4Write16b };
mrwp setabank = { setaaccessbankr8, setaaccessbankw8, setaaccessbankr16, setaaccessbankw16 };
mrwp setabanka = { setaaccessbankr8a, setaaccessbankw8a, setaaccessbankr16a, setaaccessbankw16a };
mrwp seta11bank = { Seta11Read8_68, Seta11Write8_68, Seta11Read16_68, Seta11Write16_68 };
mrwp seta11banka = { Seta11Read8_60, Seta11Write8_60, Seta11Read16_60, Seta11Write16_60 };
mrwp sfxbank = { sfxaccessbankr8, sfxaccessbankw8, sfxaccessbankr16, sfxaccessbankw16 };
mrwp sfxbankb = { sfxaccessbankr8b, sfxaccessbankw8b, sfxaccessbankr16b, sfxaccessbankw16b };
mrwp sfxbankc = { sfxaccessbankr8c, sfxaccessbankw8c, sfxaccessbankr16c, sfxaccessbankw16c };
mrwp sfxbankd = { sfxaccessbankr8d, sfxaccessbankw8d, sfxaccessbankr16d, sfxaccessbankw16d };
mrwp obc1bank = { OBC1Read8b, OBC1Write8b, OBC1Read16b, OBC1Write16b };
mrwp c4bank = { C4Read8b, C4Write8b, C4Read16b, C4Write16b };
mrwp SPC7110bank = { memaccessspc7110r8, memaccessspc7110w8, memaccessspc7110r16, memaccessspc7110w16 };
mrwp SPC7110SRAMBank = { SPC7110ReadSRAM8b, SPC7110WriteSRAM8b, SPC7110ReadSRAM16b, SPC7110WriteSRAM16b };
mrwp stbanka = { stsramr8, stsramw8, stsramr16, stsramw16 };
mrwp stbankb = { stsramr8b, stsramw8b, stsramr16b, stsramw16b };


void SetAddressingModes()
{                                       //  Banks
  map_mem(0x00, &regbank,  0x40);       // 00 - 3F
  map_mem(0x40, &membank,  0x3E);       // 40 - 7D
  map_mem(0x7E, &wrambank, 0x01);       // 7E
  map_mem(0x7F, &erambank, 0x01);       // 7F
  map_mem(0x80, &regbank,  0x40);       // 80 - BF
  map_mem(0xC0, &membank,  0x40);       // C0 - FF
}

void SetAddressingModesSA1()
{
  map_mem(0x00, &sa1regbank,  0x40);    // 00 - 3F
  map_mem(0x40, &sa1rambank,  0x20);    // 40 - 5F
  map_mem(0x60, &sa1rambankb, 0x10);    // 60 - 6F
  map_mem(0x70, &srambank,    0x08);    // 70 - 77
  map_mem(0x78, &membank,     0x06);    // 78 - 7D
  map_mem(0x7E, &wrambank,    0x01);    // 7E
  map_mem(0x7F, &erambank,    0x01);    // 7F
  map_mem(0x80, &sa1regbank,  0x40);    // 80 - BF
  map_mem(0xC0, &membank,     0x40);    // C0 - FF
}

void membank0r8reg(), membank0w8reg(), membank0r16reg(), membank0w16reg();
void membank0r8ram(), membank0w8ram(), membank0r16ram(), membank0w16ram();
void membank0r8rom(), membank0w8rom(), membank0r16rom(), membank0w16rom();
void membank0r8romram(), membank0w8romram(), membank0r16romram(), membank0w16romram();
void membank0r8inv(), membank0w8inv(), membank0r16inv(), membank0w16inv();
void membank0r8chip(), membank0w8chip(), membank0r16chip(), membank0w16chip();
void membank0r8ramSA1(), membank0w8ramSA1(), membank0r16ramSA1(), membank0w16ramSA1();

mrwp regbank0 = { membank0r8reg, membank0w8reg, membank0r16reg, membank0w16reg };
mrwp rambank0 = { membank0r8ram, membank0w8ram, membank0r16ram, membank0w16ram };
mrwp rombank0 = { membank0r8rom, membank0w8rom, membank0r16rom, membank0w16rom };
mrwp romrambank0 = { membank0r8romram, membank0w8romram, membank0r16romram, membank0w16romram };
mrwp invbank0 = { membank0r8inv, membank0w8inv, membank0r16inv, membank0w16inv };
mrwp chipbank0 = { membank0r8chip, membank0w8chip, membank0r16chip, membank0w16chip };
mrwp sa1rambank0 = { membank0r8ramSA1, membank0w8ramSA1, membank0r16ramSA1, membank0w16ramSA1 };

static void map_bank0(size_t dest, mrwp *src, size_t num)
{
  rep_stosd(Bank0datr8+dest, src->memr8, num);
  rep_stosd(Bank0datw8+dest, src->memw8, num);
  rep_stosd(Bank0datr16+dest, src->memr16, num);
  rep_stosd(Bank0datw16+dest, src->memw16, num);
}

void GenerateBank0Table()
{
  map_bank0(0x00, &rambank0,    0x20);   // 00 - 1F
  map_bank0(0x20, &regbank0,    0x28);   // 20 - 47
  map_bank0(0x48, &invbank0,    0x17);   // 48 - 5E
  map_bank0(0x5F, &chipbank0,   0x1F);   // 5F - 7D
  map_bank0(0x7E, &rombank0,    0x81);   // 7E - FE
  map_bank0(0xFF, &romrambank0, 0x01);   // FF
}

void GenerateBank0TableSA1()
{
  map_bank0(0x00, &sa1rambank0, 0x20);   // 00 - 1F
}
