#ifndef NEWGFX_H
#define NEWGFX_H

#include "../types.h"

extern void preparesprpr();

extern u1 Mode7HiRes;     // XXX always 0
extern u4 bgcmsung;
extern u4 modeused[2];
extern u4 ngextbg;
extern u4 ngwintable[32];
extern u4 scfbl;

#ifdef __MSDOS__
extern eop* mosjmptab[15];

extern void mosdraw2();
extern void mosdraw3();
extern void mosdraw4();
extern void mosdraw5();
extern void mosdraw6();
extern void mosdraw7();
extern void mosdraw8();
extern void mosdraw9();
extern void mosdraw10();
extern void mosdraw11();
extern void mosdraw12();
extern void mosdraw13();
extern void mosdraw14();
extern void mosdraw15();
extern void mosdraw16();
#endif

#endif
