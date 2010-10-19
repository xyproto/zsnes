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
extern s4   DSPBuffer[320 * 4]; // The play buffer
extern u1   AdsrBlocksLeft[8];
extern u1   AdsrSustLevLoc[8];
extern u1   GainDecBendDataDat[8];
extern u1   GainDecBendDataPos[8];
extern u1   SBHDMA;
extern u1   SoundLooped0[8];
extern u1   StatTemp[8];
extern u1   Voice0End[8];
extern u1   Voice0FirstBlock[8];
extern u1   Voice0Loop[8];
extern u1   Voice0State[8];
extern u1   Voice0Status[8]; // 0=Not Playing 1=Playing
extern u1   echoon0[8];
extern u2   DSPInterP[1024];
extern u2   Voice0Pitch[8];
extern u4   AdsrNextTimeDepth[8];
extern u4   AttackRate[];
extern u4   BRRPlace0[8][2];
extern u4   BufferSizeB;
extern u4   BufferSizeW;
extern u4   DecayRate[];
extern u4   DecreaseRateExp[];
extern u4   Decrease[];
extern u4   EchoRate[];
extern u4   EnvITemp[8];
extern u4   GainDecBendDataTime[8];
extern u4   IncNTemp[8];
extern u4   IncreaseBent[];
extern u4   Increase[];
extern u4   PSampleBuf[8][24];
extern u4   SustainRate[];
extern u4   TimeTemp[8];
extern u4   Voice0EnvInc[8];
extern u4   Voice0Freq[8]; // Frequency of Voice (Delta Freq)
extern u4   Voice0IncNumber[8];
extern u4   Voice0LoopPtr[8];
extern u4   Voice0Prev0[8];
extern u4   Voice0Prev1[8];
extern u4   Voice0Ptr[8];
extern u4   Voice0Time[8];
extern u4   dspPAdj;

#endif
