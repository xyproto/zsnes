#ifndef C_DSPPROC_H
#define C_DSPPROC_H

#include "../types.h"

void AdjustFrequency(void);
void InitSPC(void);
void LPFexit(void);
void LPFstereo(s4* esi);
void MixEcho(void);
void MixEcho2(void);
void ProcessVoiceStuff(u4 ebp, u4 p1, u4 p3, u4 p4);
void VoiceStart(u4 voice);
void VoiceStarter(u1 voice);

#endif
