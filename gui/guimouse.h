#ifndef GUIMOUSE_H
#define GUIMOUSE_H

// Mouse implementation

#include "../types.h"

void SwitchFullScreen(void);
void ProcessMouse(void);
u4 guipresstest(void);
void guipresstestb(u4* guicpressptr, char const* guipressptr);
void DrawMouse(void);

extern u1 GUIcwinpress;
extern u1 lastmouseholded;

#endif
