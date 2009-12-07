#ifndef EXECUTE_H
#define EXECUTE_H

#include "../types.h"

extern void endprog();
extern void reexecute();

extern u1 NoSoundReinit;
extern u1 exiter;
extern u1 pressed[256 + 128 + 64]; // keyboard pressed keys in scancode
extern u1 romloadskip;
extern u2 t1cc;

#ifdef __MSDOS__
typedef void IRQHandler(void);

extern IRQHandler* oldhand8o;
extern IRQHandler* oldhand9o;
extern IRQHandler* oldhandSBo;
extern u2          oldhand8s;
extern u2          oldhand9s;
#endif

#endif
