#ifndef GUIFUNCS_H
#define GUIFUNCS_H

#include "../types.h"

u4*  horizon_get(u4 distance);
void GUIQuickLoadUpdate(void);
void GUISaveVars(void);
void GetLoadData(void);
void LoadCheatSearchFile(void);
void SaveCheatSearchFile(void);
void loadquickfname(u1 slot);

extern u4 GUIcurrentfilewin;

#endif
