#ifndef GUIMOUSE_H
#define GUIMOUSE_H

#include "../types.h"

extern void DrawMouse();
extern void ProcessMouseButtons();
extern void ProcessMouseWrap();

extern char const* guipressptr;
extern u1          MouseMoveOkay;
extern u1          lastmouseholded;
extern u1          ntscCurVar;
extern u1          ntscWhVar;
extern u2          mousebuttonstat;
extern u4*         guicpressptr;

#endif
