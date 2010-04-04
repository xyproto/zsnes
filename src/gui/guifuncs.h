#ifndef GUIFUNCS_H
#define GUIFUNCS_H

#include <stdbool.h>

#include "../types.h"

u4*  horizon_get(u4 distance);
void CheatCodeLoad(void);
void CheatCodeSave(void);
void GUILoadData(void);
void GUIQuickLoadUpdate(void);
void GUISaveVars(void);
void GetLoadData(void);
bool Keep43Check(void);
void LoadCheatSearchFile(void);
void SaveCheatSearchFile(void);
void SetMovieForcedLength(void);
void dumpsound(void);
void loadquickfname(u1 slot);

#ifndef __MSDOS__
void GetCustomXY(void);
void SetCustomXY(void);

extern char GUICustomX[5];
extern char GUICustomY[5];
#endif

extern u4 GUIcurrentfilewin;

#endif
