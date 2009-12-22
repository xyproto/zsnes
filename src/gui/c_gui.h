#ifndef C_GUI_H
#define C_GUI_H

#include "../types.h"

void GUIinit18_2hz(void);
void GUIinit36_4hz(void);
void GUI36hzcall(void);
void StartGUI(void);
void guimencodermsg(void);
void guilamemsg(void);
void guiprevideo(void);
void guipostvideo(void);
#ifdef __MSDOS__
void guipostvideofail(void);
#endif
void CheckMenuItemHelp(u4 id);
void GUITryMenuItem(void);
void GUIProcStates(void);
void guicheaterror(void);
void DisplayBoxes(void);
void DisplayMenu(void);

void GUIBox3D(u4 const x1, u4 const y1, u4 const x2, u4 const y2);
void GUIOuttextShadowed(u4 const x, u4 const y, char const* const text);

u1* GetAnyPressedKey(void);

extern u1 savecfgforce;
extern u4 NumSnow;
extern u4 SnowTimer;

#endif
