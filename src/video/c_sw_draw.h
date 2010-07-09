#ifndef C_SW_DRAW_H
#define C_SW_DRAW_H

#include "../types.h"

void ClearWin16(void);
void ClearWin32(void);
void DrawWin256x224x16(void);

extern u1* ScreenPtr;
extern u1* SurfBufD;

#endif
