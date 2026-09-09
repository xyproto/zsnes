#ifndef C_DMA_H
#define C_DMA_H

#include "regs.h"

void c_reg420Bw(u1 al);
void c_reg420Cw(u1 al);
void exechdma(void);
void setuphdma(u4 eax, HDMAInfo* edx, DMAInfo* esi); // HDMA Settings
void starthdma(void); // HDMA enable register

extern u1 AddrNoIncr;

#endif
