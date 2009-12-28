#ifndef GUICHEAT_H
#define GUICHEAT_H

#include "../types.h"

extern void AddCSCheatCode();
extern void CheatCodeFix();
extern void CheatCodeRemove();
extern void CheatCodeSearchInit();
extern void CheatCodeSearchProcess();
extern void CheatCodeToggle();
extern void ProcessCheatCode();

extern u1 CopyRamToggle;

#endif
