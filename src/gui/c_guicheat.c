#include "../cfg.h"
#include "c_guicheat.h"
#include "gui.h"
#include "guicheat.h"
#include "guikeys.h"
#include "guiwindp.h"


void AddCSCheatCode(void)
{
	if (CSInputDisplay[0] == '_') return;

	curaddrvalcs = curentryval;
	curvaluecs   = CSCurValue;
	u4 ecx = CheatSrcByteSize + 1;
	if (CheatUpperByteOnly != 0)
	{
		ecx = 1;
		while (curvaluecs >= 0xFF)
		{
			curvaluecs >>= 8;
			++curaddrvalcs;
		}
	}
	do
	{
		u4       eax = curaddrvalcs + 0x7E0000;
		u1 const bl  = curvaluecs;
		// write bl at address eax
		u4 ebx = bl;
		asm volatile("call %P2" : "+a" (eax), "+b" (ebx) : "X" (AddCheatCode) : "cc", "memory", "ecx", "edx");

		curvaluecs >>= 8;
		GUItextcolor[0] = 223;

		CheatSearchYPos += 10;
		++curaddrvalcs;
	}
	while (--ecx != 0);
	CheatWinMode = 2;
}
