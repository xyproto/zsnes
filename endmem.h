#ifndef ENDMEM_H
#define ENDMEM_H

#include "types.h"

extern eop* Bank0datr16[256];
extern eop* Bank0datr8[256];
extern eop* Bank0datw16[256];
extern eop* Bank0datw8[256];
extern eop* opcjmptab[256];

extern eop* tableA[256];
extern eop* tableAc[256];
extern eop* SA1tableA[256];
extern eop* tableB[256];
extern eop* tableBc[256];
extern eop* SA1tableB[256];
extern eop* tableC[256];
extern eop* tableCc[256];
extern eop* SA1tableC[256];
extern eop* tableD[256];
extern eop* tableDc[256];
extern eop* SA1tableD[256];
extern eop* tableE[256];
extern eop* tableEc[256];
extern eop* SA1tableE[256];
extern eop* tableF[256];
extern eop* tableFc[256];
extern eop* SA1tableF[256];
extern eop* tableG[256];
extern eop* tableGc[256];
extern eop* SA1tableG[256];
extern eop* tableH[256];
extern eop* tableHc[256];
extern eop* SA1tableH[256];
extern eop** tablead[256];
extern eop** tableadc[256];
extern eop** SA1tablead[256];

extern u1 SpecialLine[256];
extern u1 cachebg[4][64];
extern u1 sprcnt[256];
extern u1 sprend[256];
extern u1 sprleftpr1[256]; // sprites left for priority 1
extern u1 sprleftpr2[256]; // sprites left for priority 2
extern u1 sprleftpr3[256]; // sprites left for priority 3
extern u1 sprleftpr[256]; // sprites left for priority 0
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
extern u4 FxTable[256];
extern u4 FxTableb[256];
extern u4 FxTablec[256];
extern u4 FxTabled[256];
extern u4 PLOTJmpa[64];
extern u4 PLOTJmpb[64];
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
