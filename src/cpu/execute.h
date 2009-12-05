#ifndef EXECUTE_H
#define EXECUTE_H

#include "../types.h"

extern void reexecute();

extern u1 exiter;
extern u1 pressed[256 + 128 + 64]; // keyboard pressed keys in scancode
extern u1 romloadskip;

#endif
