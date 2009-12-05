#ifndef UI_H
#define UI_H

#include "types.h"

#define REGPTR(x) (regptra[(x) - 0x2000])
#define REGPTW(x) (regptwa[(x) - 0x2000])

extern eop* regptra[0x3000];
extern eop* regptwa[0x3000];
extern u1   MusicVol;
extern u1*  vidbufferofsa;   // offset 1

#endif
