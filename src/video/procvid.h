#ifndef PROCVID_H
#define PROCVID_H

#include "../types.h"

extern void copyvid();
extern void saveselect();

extern char* Msgptr;          // Pointer to message
extern u1    ASCII2Font[256];
extern u1    mousexdir;
extern u1    mouseydir;
extern u1    prevbright;      // previous brightness
extern u2    mousebuttons;
extern u2    mousexpos;
extern u2    mouseypos;
extern u2    tempco0;
extern u4    MessageOn;       // Message On Countdown

#endif
