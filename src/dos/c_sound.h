#ifndef C_SOUND_H
#define C_SOUND_H

#include "../types.h"

u1   SB_dsp_read(void);
void DeInitSPC(void);
void SB_alloc_dma(void);
void SB_dsp_reset(void);
void SB_dsp_write(u1 al);
void SB_quality_limiter(void);
void getblaster(void);

extern u1 SBInt;

#endif
