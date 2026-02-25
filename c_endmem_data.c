/*
 * endmem.c — C replacement for endmem.asm
 *
 * Pure data declarations: memory buffers, lookup tables, and caches used
 * throughout the emulator.  Every symbol matches the NEWSYM in endmem.asm
 * exactly (name, size, initial value, section).
 *
 * Built only when NO_ASM=1.
 */

#include "types.h"

/* ── .bss (zero-initialized) ── */

u1 wramdataa[65536];
u1 ram7fa[65536];
u4 Inbetweendat[4];
u4 opcjmptab[256];

u4 Bank0datr8[256];
u4 Bank0datr16[256];
u4 Bank0datw8[256];
u4 Bank0datw16[256];

u4 tableA[256];
u4 tableB[256];
u4 tableC[256];
u4 tableD[256];
u4 tableE[256];
u4 tableF[256];
u4 tableG[256];
u4 tableH[256];

u4 tableAc[256];
u4 tableBc[256];
u4 tableCc[256];
u4 tableDc[256];
u4 tableEc[256];
u4 tableFc[256];
u4 tableGc[256];
u4 tableHc[256];

u4 SA1tableA[256];
u4 SA1tableB[256];
u4 SA1tableC[256];
u4 SA1tableD[256];
u4 SA1tableE[256];
u4 SA1tableF[256];
u4 SA1tableG[256];
u4 SA1tableH[256];

u4 tablead[256];
u4 tableadc[256];
u4 SA1tablead[256];

u4 memtabler8[256];
u4 memtablew8[256];
u4 memtabler16[256];
u4 memtablew16[256];

u1 vidmemch2[4096];
u1 vidmemch4[4096];
u1 vidmemch8[4096];

u4 snesmmap[256];
u4 snesmap2[256];

u1 cachebg[4][64];

u1 sprlefttot[256];
u1 sprleftpr[256];
u1 sprleftpr1[256];
u1 sprleftpr2[256];
u1 sprleftpr3[256];
u1 sprcnt[256];
u1 sprstart[256];
u1 sprtilecnt[256];
u1 sprend[256];
u2 sprendx[256];
u1 sprpriodata[288];
u1 sprprtabc[64];
u1 sprprtabu[64];
u2 prevpal[256];
u1 winbgdata[288];
u1 winspdata[288];

u4 FxTable[256];
u4 FxTableA1[256];
u4 FxTableA2[256];
u4 FxTableA3[256];
u4 FxTableb[256];
u4 FxTablebA1[256];
u4 FxTablebA2[256];
u4 FxTablebA3[256];
u4 FxTablec[256];
u4 FxTablecA1[256];
u4 FxTablecA2[256];
u4 FxTablecA3[256];
u4 FxTabled[256];
u4 FxTabledA1[256];
u4 FxTabledA2[256];
u4 FxTabledA3[256];
u4 SfxMemTable[256];
u4 fxxand[256];
u4 fxbit01[256];
u4 fxbit23[256];
u4 fxbit45[256];
u4 fxbit67[256];
u4 PLOTJmpa[64];
u4 PLOTJmpb[64];

u4 pal16b[256];
u4 pal16bcl[256];
u4 pal16bclha[256];
u4 pal16bxcl[256];
u1 xtravbuf[576];
u2 BG1SXl[256];
u2 BG2SXl[256];
u2 BG3SXl[256];
u2 BG4SXl[256];
u2 BG1SYl[256];
u2 BG2SYl[256];
u2 BG3SYl[256];
u2 BG4SYl[256];
u1 BGMA[256];
u1 BGFB[256];
u1 BG3PRI[256];
u2 BGOPT1[256];
u2 BGOPT2[256];
u2 BGOPT3[256];
u2 BGOPT4[256];
u2 BGPT1[256];
u2 BGPT2[256];
u2 BGPT3[256];
u2 BGPT4[256];
u2 BGPT1X[256];
u2 BGPT2X[256];
u2 BGPT3X[256];
u2 BGPT4X[256];
u2 BGPT1Y[256];
u2 BGPT2Y[256];
u2 BGPT3Y[256];
u2 BGPT4Y[256];
u2 BGMS1[1024];
u1 prdata[256];
u1 prdatb[256];
u1 prdatc[256];
u4 ngpalcon2b[32];
u4 ngpalcon4b[32];
u4 ngpalcon8b[32];
u1 tltype2b[4096];
u1 tltype4b[2048];
u1 tltype8b[1024];

u4 ngptrdat[1024];
u4 ngceax[1024];
u4 ngcedi[1024];
u2 bgtxad[1024];
u4 sprtbng[256];
u1 sprtlng[256];
u1 mosszng[256];
u1 mosenng[256];

/* ── .data (explicit initial values) ── */

__attribute__((aligned(32)))
u1 vidmemch2s[4096] = { [0 ... 4095] = 0xFF };
__attribute__((aligned(32)))
u1 vidmemch4s[2048] = { [0 ... 2047] = 0xFF };
__attribute__((aligned(32)))
u1 vidmemch8s[1024] = { [0 ... 1023] = 0xFF };

/* ── .bss (continued) ── */

u4 mode7ab[256];
u4 mode7cd[256];
u4 mode7xy[256];
u1 mode7st[256];

u1 t16x161[256];
u1 t16x162[256];
u1 t16x163[256];
u1 t16x164[256];

u1 intrlng[256];
u1 mode7hr[256];

u1 scadsng[256];
u1 scadtng[256];

u2 scbcong[256];

u4 cpalval[256];
u1 cgfxmod[256];

u4 winboundary[256];
u1 winbg1enval[256];
u1 winbg2enval[256];
u1 winbg3enval[256];
u1 winbg4enval[256];
u1 winbgobjenval[256];
u1 winbgbackenval[256];
u2 winlogicaval[256];

u1 winbg1envals[256];
u1 winbg2envals[256];
u1 winbg3envals[256];
u1 winbg4envals[256];
u1 winbgobjenvals[256];
u1 winbgbackenvals[256];
u1 winbg1envalm[256];
u1 winbg2envalm[256];
u1 winbg3envalm[256];
u1 winbg4envalm[256];
u1 winbgobjenvalm[256];
u1 winbgbackenvalm[256];

u1 FillSubScr[256];

u4 objclineptr[256];

/* ── .data (explicit initial values) ── */

__attribute__((aligned(32)))
u4 objwlrpos[256] = { [0 ... 255] = 0xFFFFFFFF };
__attribute__((aligned(32)))
u2 objwen[256] = { [0 ... 255] = 0xFFFF };

/* ── .bss (continued) ── */

u1 SpecialLine[256];

u1 bgallchange[256];
u1 bg1change[256];
u1 bg2change[256];
u1 bg3change[256];
u1 bg4change[256];
u1 bgwinchange[256];

u2 PrevPicture[64 * 56];
