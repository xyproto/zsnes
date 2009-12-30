#ifndef C_PROCVID_H
#define C_PROCVID_H

#include "../types.h"

// Processes & displays video
void showvideo(void);

u4 SwapMouseButtons(u4 buttons);

void processmouse1(void);
void processmouse2(void);

#ifdef __MSDOS__
// Outputs val at buf
void outputhex(u1* buf, u1 val);
#endif

void OutputText16b(u2* dst, u1 const* src, u4 edx);

void outputhex16(u2* buf, u1 val);

#endif
