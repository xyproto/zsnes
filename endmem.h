// endmem.h
#ifndef ENDMEM_H
#define ENDMEM_H

#include <stdint.h>

/* BSS allocations */
extern uint32_t   Bank0datr8[256];
extern uint32_t   Bank0datr16[256];
extern uint32_t   Bank0datw8[256];
extern uint32_t   Bank0datw16[256];
extern uint32_t   opcjmptab[256];

extern uint32_t   tableA[256];
extern uint32_t   tableAc[256];
extern uint32_t   tableB[256];
extern uint32_t   tableBc[256];
extern uint32_t   tableC[256];
extern uint32_t   tableCc[256];
extern uint32_t   tableD[256];
extern uint32_t   tableDc[256];
extern uint32_t   tableE[256];
extern uint32_t   tableEc[256];
extern uint32_t   tableF[256];
extern uint32_t   tableFc[256];
extern uint32_t   tableG[256];
extern uint32_t   tableGc[256];
extern uint32_t   tableH[256];
extern uint32_t   tableHc[256];

extern uint32_t   tablead[256];
extern uint32_t   tableadc[256];

extern uint8_t    SpecialLine[256];

/* background cache */
extern uint8_t    cachebg1[64];
extern uint8_t    cachebg2[64];
extern uint8_t    cachebg3[64];
extern uint8_t    cachebg4[64];

/* sprite state */
extern uint8_t    sprcnt[256];
extern uint8_t    sprend[256];
extern uint8_t    sprleftpr1[256];
extern uint8_t    sprleftpr2[256];
extern uint8_t    sprleftpr3[256];
extern uint8_t    sprleftpr[256];
extern uint8_t    sprlefttot[256];
extern uint8_t    sprpriodata[288];
extern uint8_t    sprstart[256];
extern uint8_t    sprtilecnt[256];
extern uint16_t   sprendx[256];

/* window buffers */
extern uint8_t    winbgdata[288];
extern uint8_t    winspdata[288];

/* SNES memory maps */
extern uint32_t   snesmmap[256];
extern uint32_t   snesmap2[256];

/* palettes */
extern uint16_t   prevpal[256];

/* extra video buffers */
extern uint8_t    vidmemch2[4096];
extern uint8_t    vidmemch4[4096];
extern uint8_t    vidmemch8[4096];
extern uint8_t    xtravbuf[576];

/* FX tables */
extern uint32_t   FxTable[256];
extern uint32_t   FxTableb[256];
extern uint32_t   FxTablec[256];
extern uint32_t   FxTabled[256];

/* plot jump tables */
extern uint32_t   PLOTJmpa[64];
extern uint32_t   PLOTJmpb[64];

/* bit‐mask tables */
extern uint32_t   fxbit01[256];
extern uint32_t   fxbit23[256];
extern uint32_t   fxbit45[256];
extern uint32_t   fxbit67[256];

/* palette constants */
extern uint32_t   ngpalcon2b[32];
extern uint32_t   ngpalcon4b[32];
extern uint32_t   ngpalcon8b[32];

/* object‐window registers */
extern uint32_t   objwlrpos[256];
extern uint16_t   objwen[256];

/* 16‑bit palettes */
extern uint32_t   pal16b[256];
extern uint32_t   pal16bcl[256];
extern uint32_t   pal16bxcl[256];

/* previous full‐screen image */
extern uint8_t    PrevPicture[7168];

#endif // ENDMEM_H
