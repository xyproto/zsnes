#ifndef GUIMOUSE_H
#define GUIMOUSE_H

#include "../types.h"

extern void DisplayGUIAboutClick();
extern void DisplayGUIAddOnClick();
extern void DisplayGUICheatClick();
extern void DisplayGUICheatClick2();
extern void DisplayGUICheatSearchClick();
extern void DisplayGUICheatSearchClick2();
extern void DisplayGUIChipClick();
extern void DisplayGUIChoseSaveClick();
extern void DisplayGUIComboClick();
extern void DisplayGUIComboClick2();
extern void DisplayGUIConfirmClick();
extern void DisplayGUIConfirmClick2();
extern void DisplayGUIInputClick();
extern void DisplayGUIInputClick2();
extern void DisplayGUIMovieClick();
extern void DisplayGUIOptionClick();
extern void DisplayGUIOptnsClick();
extern void DisplayGUIPathsClick();
extern void DisplayGUIResetClick();
extern void DisplayGUISaveClick();
extern void DisplayGUISoundClick();
extern void DisplayGUISpeedClick();
extern void DisplayGUIStatesClick();
extern void DisplayGUIVideoClick();
extern void DisplayGUIVideoClick2();
extern void DisplayGameOptnsClick();
extern void DisplayNetOptnsClick();
extern void DrawMouse();

extern char const* guipressptr;
extern u1          ntscCurVar;
extern u1          ntscWhVar;
extern u4*         guicpressptr;

#endif
