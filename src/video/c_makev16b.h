#ifndef C_MAKEV16B_H
#define C_MAKEV16B_H

#include "../types.h"

void clearback16b(void);
void draw16x1616b(u4 eax, u4 ecx, u2* edx, u1* ebx, u4 esi, u2 const* edi);
void drawline16b(void);
void drawsprites16b(u1 cl, u4 ebp);
void procspritesmain16b(u4 ebp);
void setpalette16b(void);

extern u2 draw16x1616b_yadd;
extern u2 draw16x1616b_yflipadd;

#endif
