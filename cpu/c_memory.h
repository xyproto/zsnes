#ifndef C_MEMORY_H
#define C_MEMORY_H

#include "../types.h"

void SA1UpdateDPage(void);
void UpdateDPage(void);

extern mr8*  DPageR8;
extern mr16* DPageR16;
extern mw8*  DPageW8;
extern mw16* DPageW16;
extern mr8*  SA1DPageR8;
extern mr16* SA1DPageR16;
extern mw8*  SA1DPageW8;
extern mw16* SA1DPageW16;

extern mr8*  Bank0datr8[256];
extern mr16* Bank0datr16[256];
extern mw8*  Bank0datw8[256];
extern mw16* Bank0datw16[256];

#endif
