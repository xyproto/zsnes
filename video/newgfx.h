#ifndef NEWGFX_H
#define NEWGFX_H

#include "../types.h"

extern void preparesprpr();

extern u1 Mode7HiRes; // XXX always 0
extern u1 sprclprio[4];
extern u1 *ofsmcptr;
extern u4 bgcmsung;
extern u4 bgtxadd;
extern u4 flipyposngom;
extern u4 modeused[2];
extern u4 ngextbg;
extern u4 ngwintable[32];
extern u4 ofsmadx;
extern u4 ofsmady;
extern u4 ofsmcyps;
extern u4 ofsmmptr;
extern u4 ofsmtptr;
extern u4 scfbl;
extern u4 sprsingle;
extern u4 yposngom;

#endif
