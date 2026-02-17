#ifndef EXECUTE_H
#define EXECUTE_H

#include "../types.h"

extern void cpuover();
extern void execsingle();

extern u1 EMUPause;
extern u1 ExecExitOkay;
extern u1 NextLineCache;
extern u1 NextNGDisplay;
extern u1 NoSoundReinit;
extern u1 SPCKeyPressed;
extern u1 SSKeyPressed;
extern u1 exiter;
extern u4 nextframe; // tick count for timer
extern u1 pdh;
extern u1 pressed[256 + 128 + 64]; // keyboard pressed keys in scancode
extern u1 romloadskip;
extern u1* initaddrl; // initial address location
extern u2 t1cc;
extern u4 NumberOfOpcodes2;
extern u4 timercount;

#endif
