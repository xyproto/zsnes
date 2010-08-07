#ifndef C_DMA_H
#define C_DMA_H

#include "regs.h"

void exechdma(void);
void setuphdma(u4 eax, HDMAInfo* edx, DMAInfo* esi); // HDMA Settings
void starthdma(void); // HDMA enable register
void transdma(DMAInfo* esi);

extern u1 AddrNoIncr;

#endif
