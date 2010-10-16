#ifndef DSPPROC_H
#define DSPPROC_H

#include "../types.h"

extern void DSPInterpolate_4();
extern void DSPInterpolate_4_mmx();
extern void DSPInterpolate_8();
extern void ProcessSoundBuffer();

extern eop* DSPInterpolate;
extern eop* spcRptr[16];
extern eop* spcWptr[16];
extern u1   SBHDMA;
extern u1   Voice0Status[8]; // 0=Not Playing 1=Playing
extern u2   DSPInterP[1024];
extern u2   Voice0Pitch[8];
extern u4   AttackRate[];
extern u4   BufferSizeB;
extern u4   BufferSizeW;
extern u4   DecayRate[];
extern u4   DecreaseRateExp[];
extern u4   Decrease[];
extern u4   EchoRate[];
extern u4   IncreaseBent[];
extern u4   Increase[];
extern u4   SustainRate[];
extern u4   dspPAdj;

#endif
