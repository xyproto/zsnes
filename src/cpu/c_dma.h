#ifndef C_DMA_H
#define C_DMA_H

#include "regs.h"

void setuphdma(u4 eax, HDMAInfo* edx, DMAInfo* esi); // HDMA Settings
void transdma(DMAInfo* esi);

#endif
