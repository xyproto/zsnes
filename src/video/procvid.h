#ifndef PROCVID_H
#define PROCVID_H

#include "../types.h"

extern void SwapMouseButtons();

extern char* Msgptr;          // Pointer to message
extern u1    ASCII2Font[256];
extern u1    prevbright;      // previous brightness
extern u4    MessageOn;       // Message On Countdown

#endif
