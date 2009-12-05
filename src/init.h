#ifndef INIT_H
#define INIT_H

#include "types.h"

extern u1 ZMVRawDump;
extern u1 autoloadmovie;
extern u1 autoloadstate;    // auto load state slot number
extern u1 debugger;         // Start with debugger (1=yes,0=no)
extern u1 forceromtype;
extern u1 regsbackup[3019];
extern u1 romtype;          // ROM type in bytes
extern u1 yesoutofmemory;
extern u2 resetv;           // reset vector
extern u2 xpc;

#endif
