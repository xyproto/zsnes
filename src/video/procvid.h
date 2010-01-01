#ifndef PROCVID_H
#define PROCVID_H

#include "../types.h"

extern void copyvid();

extern char* Msgptr;          // Pointer to message
extern u1    ASCII2Font[256];
extern u1    FontData[][8];
extern u1    ForceNonTransp;
extern u1    csounddisable;
extern u1    f3menuen;
extern u1    mousexdir;
extern u1    mouseydir;
extern u1    prevbright;      // previous brightness
extern u1    ssautosw;
extern u2    cgramback[256];
extern u2    mousebuttons;
extern u2    mousexloc;
extern u2    mousexpos;
extern u2    mouseyloc;
extern u2    mouseypos;
extern u2    tempco0;
extern u4    MessageOn;       // Message On Countdown
extern u4    MsgCount;        // How long message will stay (PAL = 100)

#endif
