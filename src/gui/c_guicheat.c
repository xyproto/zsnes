#include "../cfg.h"
#include "../ui.h"
#include "c_gui.h"
#include "c_guicheat.h"
#include "gui.h"
#include "guikeys.h"
#include "guiwindp.h"


static void AddCheatCode(u4 const eax, u1 const bl)
{
	GUICBHold = 0;
	if (NumCheats == 255) return;

	u1* const edx = cheatdata + NumCheats++ * 28;
	{ // transfer description
		u1*         eax = edx;
		char const* ebx = CSDescDisplay;
		u4          ecx = 20;
		do
		{
			char const dl = *ebx++;
			eax[8]          = dl;
			eax[8 + 18]     = dl;
			eax[8 + 18 * 2] = dl;
		}
		while (++eax, --ecx != 0);
	}
	// toggle, value, address, pvalue, name(12)
	edx[0] = 0;
	edx[1] = bl;
	u1* const eax_ = wramdata + (eax - 0x7E0000);
	u1  const bh   = *eax_;
	*eax_           = bl;
	*(u4*)(edx + 2) = eax; // XXX ugly cast
	edx[5]          = bh;

	u1 const al = GUIpmenupos;
	CheckMenuItemHelp(7);
	GUIpmenupos = al;
	CheatOn     = 1;
}


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
		AddCheatCode(eax, bl);

		curvaluecs >>= 8;
		++curaddrvalcs;
	}
	while (--ecx != 0);
	CheatWinMode = 2;
}
