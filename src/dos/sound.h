#ifndef SOUND_H
#define SOUND_H

#include "../types.h"

extern void SBHandler(void);

extern u1 PICMaskP;
extern u1 SBInt;
extern u1 SBIrq;
extern u1 vibracard;
extern u2 oldhandSBs;

#endif
