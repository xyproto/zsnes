#ifndef INTRF_H
#define INTRF_H

#include "types.h"

extern void Get_MousePositionDisplacement();

extern u1 GUIWFVID[];

#ifndef __MSDOS__
extern void InitializeGfxStuff();
#endif

#endif
