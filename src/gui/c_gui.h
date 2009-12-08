#ifndef C_GUI_H
#define C_GUI_H

#include "../types.h"

void StartGUI(void);
void guimencodermsg(void);
void guilamemsg(void);
void GUIMenuDisplay(u4 n_cols, u4 n_rows, u1* dst, char const* text);

#endif
