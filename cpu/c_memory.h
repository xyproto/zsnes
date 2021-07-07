#ifndef C_MEMORY_H
#define C_MEMORY_H

#include "../types.h"

void SA1UpdateDPage(void);
void UpdateDPage(void);

extern eop* DPageR16;
extern eop* DPageR8;
extern eop* DPageW16;
extern eop* DPageW8;
extern eop* SA1DPageR16;
extern eop* SA1DPageR8;
extern eop* SA1DPageW16;
extern eop* SA1DPageW8;

#endif
