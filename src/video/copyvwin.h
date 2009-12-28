#ifndef COPYVWIN_H
#define COPYVWIN_H

#include "../types.h"

extern void HighResProc();
extern void Process2xSaIwin();
extern void interpolate640x480x16bwin();

extern u1* WinVidMemStart;
extern u4  AddEndBytes;
extern u4  NumBytesPerLine;

#endif
