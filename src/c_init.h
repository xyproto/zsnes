#ifndef C_INIT_H
#define C_INIT_H

#include "types.h"

void init(void);
void ReadInputDevice(void);
void DosExit(void);
void MMXCheck(void);
void outofmemfix(void);
void idledetectspc(void);

// Print Hexadecimal (16-bit)
void printhex(u2 ax);

extern u1 ComboCounter;
extern u1 MMXSupport;
extern u1 ReturnFromSPCStall;
extern u1 SPCStallSetting;
extern u1 WhichSW;
extern u4 JoyANow;
extern u4 JoyAOrig;
extern u4 JoyBNow;
extern u4 JoyBOrig;
extern u4 JoyCNow;
extern u4 JoyCOrig;
extern u4 JoyDNow;
extern u4 JoyDOrig;
extern u4 JoyENow;
extern u4 JoyEOrig;
extern u4 numspcvblleft;
extern u4 spc700idle;

#endif
