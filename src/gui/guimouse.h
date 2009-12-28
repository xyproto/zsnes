#ifndef GUIMOUSE_H
#define GUIMOUSE_H

#include "../types.h"

extern void DisplayGUIOptnsClick();
extern void DisplayGUISoundClick();
extern void DisplayGUISpeedClick();
extern void DisplayGUIVideoClick();
extern void DrawMouse();
extern void GUINTSCPreset();
extern void GUIProcCustomVideo();
extern void GUIWinClicked();
extern void GUIWindowMove();
extern void SwitchFullScreen();

extern char const* guipressptr;
extern u1          ntscCurVar;
extern u1          ntscWhVar;
extern u4*         guicpressptr;

#endif
