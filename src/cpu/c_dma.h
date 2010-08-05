#ifndef C_DMA_H
#define C_DMA_H

#include "regs.h"

void setuphdma(u4 eax, HDMAInfo* edx, DMAInfo* esi); // HDMA Settings
void setuphdmars(HDMAInfo* edx, DMAInfo const* esi);
void transdma(DMAInfo* esi);

#endif
