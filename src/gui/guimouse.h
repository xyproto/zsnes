#ifndef GUIMOUSE_H
#define GUIMOUSE_H

#include "../types.h"

extern void DrawMouse();
extern void ProcessMouse();

extern char* guipressptr;
extern u1    lastmouseholded;
extern u4*   guicpressptr;

#endif
