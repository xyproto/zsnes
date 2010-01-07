#ifndef INIT_H
#define INIT_H

#include "types.h"

extern u1 ZMVRawDump;
extern u1 autoloadmovie;
extern u1 autoloadstate;    // auto load state slot number
extern u1 cacheud;          // update cache every ? frames
extern u1 ccud;             // current cache increment
extern u1 curcyc;           // cycles left in scanline
extern u1 debugger;         // Start with debugger (1=yes,0=no)
extern u1 forceromtype;
extern u1 regsbackup[3019];
extern u1 romtype;          // ROM type in bytes
extern u1 spcon;            // SPC Enable (1=enabled)
extern u1 writeon;          // Write enable/disable on snes rom memory
extern u1 xpb;
extern u1 yesoutofmemory;
extern u2 resetv;           // reset vector
extern u2 xd;
extern u2 xpc;
extern u4 flagc;
extern u4 flagnz;
extern u4 flago;

#endif
