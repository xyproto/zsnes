#ifndef SA1REGS_H
#define SA1REGS_H

#include "../types.h"

extern void RTCReset();
extern void RTCReset2();
extern void SDD1Reset();
extern void initSA1regs();
extern void initSA1regsw();
extern void initSDD1regs();

extern u2 SA1xd;

#endif
