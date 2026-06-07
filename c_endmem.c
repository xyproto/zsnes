/* C port of endmem.asm: global BSS/data arrays shared across the codebase.
 * All BSS entries are zero-initialised by the C runtime (same as SECTION .bss).
 * Initialised entries match the `times N db VAL` data in endmem.asm.
 */

#include "types.h"

/* Work RAM mirrors */
u1 wramdataa[65536];
u1 ram7fa[65536];

/* Misc */
u4 Inbetweendat[4];

/* CPU opcode dispatch tables */
eop* opcjmptab[256];
eop* Bank0datr8[256];
eop* Bank0datr16[256];
eop* Bank0datw8[256];
eop* Bank0datw16[256];

/* 65816 opcode dispatch tables (8 mode combinations, plain + cached + SA-1) */
eop* tableA[256];
eop* tableB[256];
eop* tableC[256];
eop* tableD[256];
eop* tableE[256];
eop* tableF[256];
eop* tableG[256];
eop* tableH[256];

eop* tableAc[256];
eop* tableBc[256];
eop* tableCc[256];
eop* tableDc[256];
eop* tableEc[256];
eop* tableFc[256];
eop* tableGc[256];
eop* tableHc[256];

eop* SA1tableA[256];
eop* SA1tableB[256];
eop* SA1tableC[256];
eop* SA1tableD[256];
eop* SA1tableE[256];
eop* SA1tableF[256];
eop* SA1tableG[256];
eop* SA1tableH[256];

/* Dispatch table selectors */
eop** tablead[256];
eop** tableadc[256];
eop** SA1tablead[256];

/* Memory bank dispatch tables */
eop* memtabler8[256];
eop* memtablew8[256];
eop* memtabler16[256];
eop* memtablew16[256];

/* Video memory change flags (byte per tile) */
u1 vidmemch2[4096];
u2 vidmemch4[2048]; /* endmem.h declares u2[2048]; same 4096 bytes as endmem.asm's resb 4096 */
u1 vidmemch8[4096];

/* Bank memory maps (pointer per bank) */
u1* snesmmap[256];
u1* snesmap2[256];

/* Sprite tile cache and priority bookkeeping */
u1  cachebg[4][64]; /* cachebg == &cachebg[0]; cachebg1–4 are cachebg[0–3] */
u1  sprlefttot[256];
u1  sprleftpr[256];
u1  sprleftpr1[256];
u1  sprleftpr2[256];
u1  sprleftpr3[256];
u1  sprcnt[256];
u1  sprstart[256];
u1  sprtilecnt[256];
u1  sprend[256];
u2  sprendx[256];
u1  sprpriodata[288];
u1  sprprtabc[64];
u1  sprprtabu[64];
u2  prevpal[256];
u1  winbgdata[288];
u1  winspdata[288];

/* SuperFX lookup tables */
u4  FxTable[256];
u4  FxTableA1[256];
u4  FxTableA2[256];
u4  FxTableA3[256];
u4  FxTableb[256];
u4  FxTablebA1[256];
u4  FxTablebA2[256];
u4  FxTablebA3[256];
u4  FxTablec[256];
u4  FxTablecA1[256];
u4  FxTablecA2[256];
u4  FxTablecA3[256];
u4  FxTabled[256];
u4  FxTabledA1[256];
u4  FxTabledA2[256];
u4  FxTabledA3[256];
u4  SfxMemTable[256];
u4  fxxand[256];
u4  fxbit01[256];
u4  fxbit23[256];
u4  fxbit45[256];
u4  fxbit67[256];
u4  PLOTJmpa[64];
u4  PLOTJmpb[64];

/* Palette buffers */
u4  pal16b[256];
u4  pal16bcl[256];
u4  pal16bclha[256];
u4  pal16bxcl[256];
u2  xtravbuf[288];

/* Background scroll/position per-scanline arrays */
u2  BG1SXl[256];
u2  BG2SXl[256];
u2  BG3SXl[256];
u2  BG4SXl[256];
u2  BG1SYl[256];
u2  BG2SYl[256];
u2  BG3SYl[256];
u2  BG4SYl[256];
u1  BGMA[256];
u1  BGFB[256];
u1  BG3PRI[256];
u2  BGOPT1[256];
u2  BGOPT2[256];
u2  BGOPT3[256];
u2  BGOPT4[256];
u2  BGPT1[256];
u2  BGPT2[256];
u2  BGPT3[256];
u2  BGPT4[256];
u2  BGPT1X[256];
u2  BGPT2X[256];
u2  BGPT3X[256];
u2  BGPT4X[256];
u2  BGPT1Y[256];
u2  BGPT2Y[256];
u2  BGPT3Y[256];
u2  BGPT4Y[256];
u2  BGMS1[1024];
u1  prdata[256];
u1  prdatb[256];
u1  prdatc[256];

/* Palette conversion tables */
u4  ngpalcon2b[32];
u4  ngpalcon4b[32];
u4  ngpalcon8b[32];

/* Tile type arrays */
u1  tltype2b[4096];
u1  tltype4b[2048];
u1  tltype8b[1024];

/* New-graphics per-tile metadata */
u4  ngptrdat[1024];
u4  ngceax[1024];
u4  ngcedi[1024];
u2  bgtxad[1024];
u4  sprtbng[256];
u1  sprtlng[256];
u1  mosszng[256];
u1  mosenng[256];

/* Video cache sentinel arrays — initialised to 0xFF (mirrors endmem.asm SECTION .data).
 * Designated-range initialiser is a GCC/Clang extension, but the codebase already
 * requires GCC/Clang for inline asm, so this is acceptable. */
u1  vidmemch2s[4096] = {[0 ... 4095] = 0xFF};
u1  vidmemch4s[2048] = {[0 ... 2047] = 0xFF};
u1  vidmemch8s[1024] = {[0 ... 1023] = 0xFF};

/* Mode-7 per-scanline tables */
u4  mode7ab[256];
u4  mode7cd[256];
u4  mode7xy[256];
u1  mode7st[256];

/* 16×16 tile type arrays */
u1  t16x161[256];
u1  t16x162[256];
u1  t16x163[256];
u1  t16x164[256];

/* Misc per-scanline */
u1  intrlng[256];
u1  mode7hr[256];
u1  scadsng[256];
u1  scadtng[256];
u2  scbcong[256];
u4  cpalval[256];
u1  cgfxmod[256];

/* Window engine per-scanline data */
u4  winboundary[256];
u1  winbg1enval[256];
u1  winbg2enval[256];
u1  winbg3enval[256];
u1  winbg4enval[256];
u1  winbgobjenval[256];
u1  winbgbackenval[256];
u2  winlogicaval[256];
u1  winbg1envals[256];
u1  winbg2envals[256];
u1  winbg3envals[256];
u1  winbg4envals[256];
u1  winbgobjenvals[256];
u1  winbgbackenvals[256];
u1  winbg1envalm[256];
u1  winbg2envalm[256];
u1  winbg3envalm[256];
u1  winbg4envalm[256];
u1  winbgobjenvalm[256];
u1  winbgbackenvalm[256];
u1  FillSubScr[256];
u4  objclineptr[256];
u1  SpecialLine[256];

/* Background change flags */
u1  bgallchange[256];
u1  bg1change[256];
u1  bg2change[256];
u1  bg3change[256];
u1  bg4change[256];
u1  bgwinchange[256];

/* Previous rendered frame */
u2  PrevPicture[64 * 56];

/* OBJ window data — initialised to all-bits-set (endmem.asm SECTION .data) */
u4  objwlrpos[256] = {[0 ... 255] = 0xFFFFFFFFu};
u2  objwen[256]    = {[0 ... 255] = 0xFFFFu};
