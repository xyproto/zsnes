#ifndef UI_H
#define UI_H

#include "types.h"

#define REGPTR(x) (regptra[(x) - 0x2000])
#define REGPTW(x) (regptwa[(x) - 0x2000])

extern eop* dspRptr[256];
extern eop* dspWptr[256];
extern eop* regptra[0x3000];
extern eop* regptwa[0x3000];
extern u1   DSPDisable;             // Disable DSP emulation
extern u1   MusicVol;
extern u1   V8Mode;                 // Vegetable mode! =) (Greyscale mode)
extern u1   cbitmode;               // bit mode, 0=8bit, 1=16bit
extern u1   newgfx16b;
extern u1*  romdata;                // rom data  (4MB = 4194304)
extern u1*  spcBuffera;
extern u1*  vidbuffer;              // video buffer (1024x239 = 244736)
extern u1*  vidbufferofsa;          // offset 1
extern u1*  vidbufferofsb;          // offset 2
extern u1*  wramdata;               // stack (64K = 65536)
extern u2   VolumeConvTable[32768];
extern u2   fulladdtab[65536];
extern u2   selc0040;
extern u2   selcA000;
extern u2   selcB800;

#endif
