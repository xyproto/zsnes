#ifndef C_GUITOOLS_H
#define C_GUITOOLS_H

#include "../types.h"

char const* GUIOutputString(u1* dst, char const* text);
void GUIOuttext(u4 x, u4 y, char const* text, u1 colour);
void GUIDrawBox(u1* dst, u4 w, u4 h, u1 colour);
void GUIBox(u4 x1, u4 y1, u4 x2, u4 y2, u1 colour);
void GUIHLine(s4 x1, s4 x2, s4 y, u1 colour);
void GUIShadow(u4 x1, u4 y1, u4 x2, u4 y2);
void GUIDrawShadow2(u1* buf, u4 w, u4 h);
void GUIOuttextwin(u4 x, u4 y, char const* text);

#endif
