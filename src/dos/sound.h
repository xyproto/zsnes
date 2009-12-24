#ifndef SOUND_H
#define SOUND_H

#include "../types.h"

extern void SBHandler(void);

extern u1 PICMaskP;
extern u1 SBInt;
extern u1 SBIrq;
extern u2 oldhandSBs;

#endif
