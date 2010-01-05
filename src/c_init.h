#ifndef C_INIT_H
#define C_INIT_H

#include "types.h"

void init(void);

u4 ProcessCombo(u4 i);

void DosExit(void);
void MMXCheck(void);
void outofmemfix(void);

extern u1 MMXSupport;

#endif
