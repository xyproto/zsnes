#ifndef C_SOUND_H
#define C_SOUND_H

#include "../types.h"

void DeInitSPC(void);
void GetCDMAPos(void);
void InitSB(void);
void SB_alloc_dma(void);
void SB_quality_limiter(void);
void getblaster(void);

extern u1 SBInt;

#endif
