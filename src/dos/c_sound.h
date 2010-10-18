#ifndef C_SOUND_H
#define C_SOUND_H

#include "../types.h"

void DeInitSPC(void);
void InitSB(void);
void SB_alloc_dma(void);
void SB_blank(void);
void SB_quality_limiter(void);
void c_SBHandler(void);
void getblaster(void);

extern u1 SBHDMA;
extern u1 SBInt;
extern u1 SBIrq;
extern u1 vibracard;

#endif
