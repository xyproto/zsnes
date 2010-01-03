#ifndef ENDMEM_H
#define ENDMEM_H

#include "types.h"

extern eop*  opcjmptab[256];
extern eop*  tableA[256];
extern eop*  tableAc[256];
extern eop*  tableB[256];
extern eop*  tableBc[256];
extern eop*  tableC[256];
extern eop*  tableCc[256];
extern eop*  tableD[256];
extern eop*  tableDc[256];
extern eop*  tableE[256];
extern eop*  tableEc[256];
extern eop*  tableF[256];
extern eop*  tableFc[256];
extern eop*  tableG[256];
extern eop*  tableGc[256];
extern eop*  tableH[256];
extern eop*  tableHc[256];
extern eop** tablead[256];
extern eop** tableadc[256];
extern u1    SpecialLine[256];
extern u1    sprleftpr1[256];
extern u1    sprleftpr2[256];
extern u1    sprleftpr3[256];
extern u1    sprleftpr[256];
extern u1    sprlefttot[256];
extern u1*   snesmap2[256];
extern u1*   snesmmap[256];
extern u2    PrevPicture[56 * 64];
extern u2    prevpal[256];         // previous palette buffer
extern u4    ngpalcon2b[32];
extern u4    ngpalcon4b[32];

#endif
