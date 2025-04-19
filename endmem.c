// Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
//
// http://www.zsnes.com
// http://sourceforge.net/projects/zsnes
// https://zsnes.bountysource.com
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// version 2 as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include <stdint.h>

#ifdef __GNUC__
#define ALIGN32 __attribute__((aligned(32)))
#else
#define ALIGN32
#endif

/* SECTION .bss */
uint8_t  wramdataa[65536];
uint8_t  ram7fa[65536];
uint32_t Inbetweendat[4];
uint32_t opcjmptab[256];

uint32_t Bank0datr8[256];
uint32_t Bank0datr16[256];
uint32_t Bank0datw8[256];
uint32_t Bank0datw16[256];

uint32_t tableA[256];
uint32_t tableB[256];
uint32_t tableC[256];
uint32_t tableD[256];
uint32_t tableE[256];
uint32_t tableF[256];
uint32_t tableG[256];
uint32_t tableH[256];

uint32_t tableAb[256];
uint32_t tableBb[256];
uint32_t tableCb[256];
uint32_t tableDb[256];
uint32_t tableEb[256];
uint32_t tableFb[256];
uint32_t tableGb[256];
uint32_t tableHb[256];

uint32_t tableAc[256];
uint32_t tableBc[256];
uint32_t tableCc[256];
uint32_t tableDc[256];
uint32_t tableEc[256];
uint32_t tableFc[256];
uint32_t tableGc[256];
uint32_t tableHc[256];

uint32_t SA1tableA[256];
uint32_t SA1tableB[256];
uint32_t SA1tableC[256];
uint32_t SA1tableD[256];
uint32_t SA1tableE[256];
uint32_t SA1tableF[256];
uint32_t SA1tableG[256];
uint32_t SA1tableH[256];

uint32_t tablead[256];
uint32_t tableadb[256];
uint32_t tableadc[256];
uint32_t SA1tablead[256];

uint32_t memtabler8[256];
uint32_t memtablew8[256];
uint32_t memtabler16[256];
uint32_t memtablew16[256];

uint8_t  vidmemch2[4096];
uint8_t  vidmemch4[4096];
uint8_t  vidmemch8[4096];

uint32_t snesmmap[256];
uint32_t snesmap2[256];

uint8_t  cachebg;
uint8_t  cachebg1[64];
uint8_t  cachebg2[64];
uint8_t  cachebg3[64];
uint8_t  cachebg4[64];

uint8_t  sprlefttot[256];
uint8_t  sprleftpr[256];
uint8_t  sprleftpr1[256];
uint8_t  sprleftpr2[256];
uint8_t  sprleftpr3[256];
uint8_t  sprcnt[256];
uint8_t  sprstart[256];
uint8_t  sprtilecnt[256];
uint8_t  sprend[256];
uint16_t sprendx[256];
uint8_t  sprpriodata[288];
uint8_t  sprprtabc[64];
uint8_t  sprprtabu[64];

uint16_t prevpal[256];

uint8_t  winbgdata[288];
uint8_t  winspdata[288];

uint32_t FxTable[256];
uint32_t FxTableA1[256];
uint32_t FxTableA2[256];
uint32_t FxTableA3[256];
uint32_t FxTableb[256];
uint32_t FxTablebA1[256];
uint32_t FxTablebA2[256];
uint32_t FxTablebA3[256];
uint32_t FxTablec[256];
uint32_t FxTablecA1[256];
uint32_t FxTablecA2[256];
uint32_t FxTablecA3[256];
uint32_t FxTabled[256];
uint32_t FxTabledA1[256];
uint32_t FxTabledA2[256];
uint32_t FxTabledA3[256];

uint32_t SfxMemTable[256];
uint32_t fxxand[256];
uint32_t fxbit01[256];
uint32_t fxbit23[256];
uint32_t fxbit45[256];
uint32_t fxbit67[256];

uint32_t PLOTJmpa[64];
uint32_t PLOTJmpb[64];

uint32_t pal16b[256];
uint32_t pal16bcl[256];
uint32_t pal16bclha[256];
uint32_t pal16bxcl[256];

uint8_t  xtravbuf[576];

uint16_t BG1SXl[256];
uint16_t BG2SXl[256];
uint16_t BG3SXl[256];
uint16_t BG4SXl[256];
uint16_t BG1SYl[256];
uint16_t BG2SYl[256];
uint16_t BG3SYl[256];
uint16_t BG4SYl[256];

uint8_t  BGMA[256];
uint8_t  BGFB[256];
uint8_t  BG3PRI[256];

uint16_t BGOPT1[256];
uint16_t BGOPT2[256];
uint16_t BGOPT3[256];
uint16_t BGOPT4[256];

uint16_t BGPT1[256];
uint16_t BGPT2[256];
uint16_t BGPT3[256];
uint16_t BGPT4[256];

uint16_t BGPT1X[256];
uint16_t BGPT2X[256];
uint16_t BGPT3X[256];
uint16_t BGPT4X[256];

uint16_t BGPT1Y[256];
uint16_t BGPT2Y[256];
uint16_t BGPT3Y[256];
uint16_t BGPT4Y[256];

uint16_t BGMS1[1024];

uint8_t  prdata[256];
uint8_t  prdatb[256];
uint8_t  prdatc[256];

uint32_t ngpalcon2b[32];
uint32_t ngpalcon4b[32];
uint32_t ngpalcon8b[32];

uint8_t  tltype2b[4096];
uint8_t  tltype4b[2048];
uint8_t  tltype8b[1024];

uint32_t ngptrdat[1024];
uint32_t ngceax[1024];
uint32_t ngcedi[1024];
uint16_t bgtxad[1024];
uint32_t sprtbng[256];
uint8_t  sprtlng[256];
uint8_t  mosszng[256];
uint8_t  mosenng[256];

/* SECTION .data */
ALIGN32 uint8_t vidmemch2s[4096] = { [0 ... 4095] = 0xFF };
ALIGN32 uint8_t vidmemch4s[2048] = { [0 ... 2047] = 0xFF };
ALIGN32 uint8_t vidmemch8s[1024] = { [0 ... 1023] = 0xFF };

/* SECTION .bss */
uint32_t mode7ab[256];
uint32_t mode7cd[256];
uint32_t mode7xy[256];
uint8_t  mode7st[256];

uint8_t  t16x161[256];
uint8_t  t16x162[256];
uint8_t  t16x163[256];
uint8_t  t16x164[256];

uint8_t  intrlng[256];
uint8_t  mode7hr[256];

uint8_t  scadsng[256];
uint8_t  scadtng[256];

uint16_t scbcong[256];

uint32_t cpalval[256];
uint8_t  cgfxmod[256];

uint32_t winboundary[256];
uint8_t  winbg1enval[256];
uint8_t  winbg2enval[256];
uint8_t  winbg3enval[256];
uint8_t  winbg4enval[256];
uint8_t  winbgobjenval[256];
uint8_t  winbgbackenval[256];
uint16_t winlogicaval[256];

uint8_t  winbg1envals[256];
uint8_t  winbg2envals[256];
uint8_t  winbg3envals[256];
uint8_t  winbg4envals[256];
uint8_t  winbgobjenvals[256];
uint8_t  winbgbackenvals[256];
uint8_t  winbg1envalm[256];
uint8_t  winbg2envalm[256];
uint8_t  winbg3envalm[256];
uint8_t  winbg4envalm[256];
uint8_t  winbgobjenvalm[256];
uint8_t  winbgbackenvalm[256];

uint8_t  FillSubScr[256];

uint32_t objclineptr[256];  /* l1,r1,l2,r2,en,log,ptr */

/* SECTION .data */
ALIGN32 uint32_t objwlrpos[256] = { [0 ... 255] = 0xFFFFFFFF };
ALIGN32 uint16_t objwen[256]   = { [0 ... 255] = 0xFFFF };

/* SECTION .bss */
uint8_t  SpecialLine[256];

uint8_t  bgallchange[256];
uint8_t  bg1change[256];
uint8_t  bg2change[256];
uint8_t  bg3change[256];
uint8_t  bg4change[256];
uint8_t  bgwinchange[256];

uint8_t  PrevPicture[7168];
