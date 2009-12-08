#ifndef C_GUI_H
#define C_GUI_H

#include "../types.h"

void StartGUI(void);
void guimencodermsg(void);
void guilamemsg(void);
void guiprevideo(void);
void guicheaterror(void);
void GUIMenuDisplay(u4 n_cols, u4 n_rows, u1* dst, char const* text);

void GUIBox3D(u4 const x1, u4 const y1, u4 const x2, u4 const y2);
void GUIOuttextShadowed(u4 const x, u4 const y, char const* const text);

u1* GetAnyPressedKey(void);

#endif
