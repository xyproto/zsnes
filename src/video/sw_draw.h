#ifndef SW_DRAW_H
#define SW_DRAW_H

#include "../types.h"

void ClearWin16(void);
void ClearWin32(void);
void DrawWin256x224x16(void);
void DrawWin320x240x16(void);

#ifdef __WIN32__
void DrawWin256x224x32(void);
#endif

extern u1* ScreenPtr;
extern u1* SurfBufD;

#endif
