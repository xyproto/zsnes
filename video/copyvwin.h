#ifndef COPYVWIN_H
#define COPYVWIN_H

#include "../types.h"

void copy640x480x16bwin(void);

extern u1* WinVidMemStart;
extern u4 AddEndBytes; // Number of bytes between each line
extern u4 NumBytesPerLine; // Total number of bytes per line (1024+AddEndBytes)

#endif
