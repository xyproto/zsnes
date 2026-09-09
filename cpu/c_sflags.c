/* Ssplitflags / Sjoinflags from cpu/s65816d.inc: what cpu/flags65816.h does
   for the 65816, over the SA-1's copy of the flag globals. A separate
   translation unit so the header can be instantiated twice. */
#include "../types.h"

extern u4 Sflagnz, Sflago, Sflagc;

#define flagnz Sflagnz
#define flago Sflago
#define flagc Sflagc

#include "flags65816.h"

u4 Sjoinflags_c(u4 const edx) { return makedl(edx); }
void Ssplitflags_c(u4 const edx) { restoredl(edx); }
