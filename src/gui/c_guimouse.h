#ifndef C_GUIMOUSE_H
#define C_GUIMOUSE_H

#include "../types.h"

void ProcessMouse(void);
u4   guipresstest(void);
void guipresstestb(void);

extern u1 GUIcwinpress;
extern u1 lastmouseholded;

#endif
