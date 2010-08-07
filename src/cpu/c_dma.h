#ifndef C_DMA_H
#define C_DMA_H

#include "regs.h"

void c_reg420Bw(u4 eax);
void c_reg420Cw(u4 eax);
void exechdma(void);
void setuphdma(u4 eax, HDMAInfo* edx, DMAInfo* esi); // HDMA Settings
void starthdma(void); // HDMA enable register

extern u1 AddrNoIncr;

#endif
