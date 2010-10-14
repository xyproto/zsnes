#ifndef DSPPROC_H
#define DSPPROC_H

#include "../types.h"

extern void AdjustFrequency();
extern void ProcessSoundBuffer();

extern eop* spcRptr[16];
extern eop* spcWptr[16];
extern u1   SBHDMA;
extern u1   Voice0Status[8]; // 0=Not Playing 1=Playing
extern u4   BufferSizeB;
extern u4   BufferSizeW;
extern u4   SBToSPC;

#endif
