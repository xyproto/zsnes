#ifndef C_GUICHEAT_H
#define C_GUICHEAT_H

void AddCSCheatCode(void);
void CheatCodeFix(void);
void CheatCodeRemove(void);
void CheatCodeSearchInit(void);
void CheatCodeSearchProcess(void);
void CheatCodeToggle(void);
void DisableCheatsOnLoad(void);
void EnableCheatCodeNoPrevMod(u1* esi);
void EnableCheatsOnLoad(void);
void ProcessCheatCode(void);

extern u1 CopyRamToggle;

#endif
