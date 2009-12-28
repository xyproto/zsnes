#ifndef GUIMOUSE_H
#define GUIMOUSE_H

#include "../types.h"

extern void DisplayGUICheatClick2();
extern void DisplayGUICheatSearchClick2();
extern void DisplayGUIComboClick2();
extern void DisplayGUIConfirmClick2();
extern void DisplayGUIInputClick2();
extern void DisplayGUIOptnsClick();
extern void DisplayGUISoundClick();
extern void DisplayGUISpeedClick();
extern void DisplayGUIVideoClick();
extern void DisplayGUIVideoClick2();
extern void DrawMouse();
extern void GUIWinClicked();

extern char const* guipressptr;
extern u1          ntscCurVar;
extern u1          ntscWhVar;
extern u4*         guicpressptr;

#endif
