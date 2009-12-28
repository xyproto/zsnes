#ifndef GUIFUNCS_H
#define GUIFUNCS_H

#include "../types.h"

u4*  horizon_get(u4 distance);
void CheatCodeLoad(void);
void CheatCodeSave(void);
void GUILoadData(void);
void GUIQuickLoadUpdate(void);
void GUISaveVars(void);
void GetLoadData(void);
void LoadCheatSearchFile(void);
void SaveCheatSearchFile(void);
void SetMovieForcedLength(void);
void loadquickfname(u1 slot);

#ifndef __MSDOS__
void SetCustomXY(void);
#endif

extern u4 GUIcurrentfilewin;

#endif
