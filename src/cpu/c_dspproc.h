#ifndef C_DSPPROC_H
#define C_DSPPROC_H

#include "../types.h"

void AdjustFrequency(void);
void InitSPC(void);
void LPFexit(void);
void LPFstereo(s4* esi);
void VoiceStart(u4 voice);
void VoiceStarter(u1 voice);

#endif
