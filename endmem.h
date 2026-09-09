#ifndef ENDMEM_H
#define ENDMEM_H

#include "types.h"

extern eop* Bank0datr16[256];
extern eop* Bank0datr8[256];
extern eop* Bank0datw16[256];
extern eop* Bank0datw8[256];
extern spcop* opcjmptab[256];

extern opfn* tableA[256];
extern opfn* tableAc[256];
extern opfn* SA1tableA[256];
extern opfn* tableB[256];
extern opfn* tableBc[256];
extern opfn* SA1tableB[256];
extern opfn* tableC[256];
extern opfn* tableCc[256];
extern opfn* SA1tableC[256];
extern opfn* tableD[256];
extern opfn* tableDc[256];
extern opfn* SA1tableD[256];
extern opfn* tableE[256];
extern opfn* tableEc[256];
extern opfn* SA1tableE[256];
extern opfn* tableF[256];
extern opfn* tableFc[256];
extern opfn* SA1tableF[256];
extern opfn* tableG[256];
extern opfn* tableGc[256];
extern opfn* SA1tableG[256];
extern opfn* tableH[256];
extern opfn* tableHc[256];
extern opfn* SA1tableH[256];
extern opfn** tablead[256];
extern opfn** tableadc[256];
extern opfn** SA1tablead[256];

extern u1 SpecialLine[256];
extern u1 cachebg[4][64];
extern u1 sprcnt[256];
extern u1 sprend[256];
extern u1 sprleftpr1[256]; // sprites left for priority 1
extern u1 sprleftpr2[256]; // sprites left for priority 2
extern u1 sprleftpr3[256]; // sprites left for priority 3
extern u1 sprleftpr[256]; // sprites left for priority 0
extern u4 sprleftpr_b[256]; // sprleftpr..sprleftpr3 as dwords
extern u1 sprlefttot[256]; // total sprites left
extern u1 sprpriodata[288];
extern u1 sprstart[256];
extern u1 sprtilecnt[256];
extern u1 winbgbackenval[256];
extern u1 winbgdata[288]; // window buffer for backgrounds
extern u1 winspdata[288]; // window buffer for sprites
extern u1* snesmap2[256];
extern u1* snesmmap[256];
extern u2 PrevPicture[56 * 64];
extern u2 prevpal[256]; // previous palette buffer
extern u2 sprendx[256];
extern u2 vidmemch4[2048];
extern u2 xtravbuf[288];
/* The SuperFX dispatch tables. The "b" and "c" blocks start as copies of the
 * four base tables and are patched by InitFxTables(). Each base is declared
 * 1024 long because the dispatch indexes table[(ALT << 8) | opcode] across the
 * four adjacent 256-entry tables; A1/A2/A3 name the same memory again so one
 * ALT table can be addressed at a time. */
extern zreg FxTable[1024];
extern zreg FxTableA1[256];
extern zreg FxTableA2[256];
extern zreg FxTableA3[256];
extern zreg FxTableb[1024];
extern zreg FxTablebA1[256];
extern zreg FxTablebA2[256];
extern zreg FxTablebA3[256];
extern zreg FxTablec[1024];
extern zreg FxTablecA1[256];
extern zreg FxTablecA2[256];
extern zreg FxTablecA3[256];
extern zreg FxTabled[1024];
extern zreg FxTabledA1[256];
extern zreg FxTabledA2[256];
extern zreg FxTabledA3[256];
extern zreg SfxMemTable[256];
extern zreg PLOTJmpa[64];
extern zreg PLOTJmpb[64];
extern u4 fxxand[256];
extern u4 fxbit01[256];
extern u4 fxbit23[256];
extern u4 fxbit45[256];
extern u4 fxbit67[256];
extern u4 ngpalcon2b[32];
extern u4 ngpalcon4b[32];
extern u4 objwlrpos[256];
extern u4 pal16b[256];
extern u4 pal16bcl[256];
extern u4 pal16bxcl[256];

#endif
