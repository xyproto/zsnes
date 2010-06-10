#include <string.h>

#include "../cfg.h"
#include "../ui.h"
#include "c_gui.h"
#include "c_guicheat.h"
#include "c_guikeys.h"
#include "gui.h"
#include "guicheat.h"
#include "guiwindp.h"


u1 CopyRamToggle;


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


#define SearchMacro(CMP)    \
do                          \
{                           \
	u1 dl = 0xFE;             \
	do                        \
	{                         \
		if (!(CMP)) *eax &= dl; \
		++edi;                  \
		++esi;                  \
		if (dl == 0x7F) ++eax;  \
		dl = dl << 1 | dl >> 7; \
	}                         \
	while (--ecx != 0);       \
}                           \
while (0)


#define SearchMacroB(L, R)                                   \
{                                                            \
	switch (CheatCompareValue)                                 \
	{                                                          \
		case 0:  SearchMacro((L) >  (R)); break; /* Greater   */ \
		case 1:  SearchMacro((L) <  (R)); break; /* Less      */ \
		case 3:  SearchMacro((L) != (R)); break; /* Not equal */ \
		default: SearchMacro((L) == (R)); break; /* Equal     */ \
	}                                                          \
}


void CheatCodeSearchProcess(void)
{
	if (CheatSrcSearchType == 1)
	{ // Comparative
		CSInputDisplay[0] = '_';
		CSInputDisplay[1] = '\0';
		u1*       eax = vidbuffer + 129600 + 65536 * 2;
		u1 const* esi = vidbuffer + 129600;
		u1 const* edi = wramdata;
		u4        ecx = 65536 * 2;
		switch (CheatSrcByteSize)
		{
			default: ecx -= 0; SearchMacroB(*(u1 const*)edi,              *(u1 const*)esi);              break;
			case 1:  ecx -= 1; SearchMacroB(*(u2 const*)edi,              *(u2 const*)esi);              break;
			case 2:  ecx -= 2; SearchMacroB(*(u4 const*)edi & 0x00FFFFFF, *(u2 const*)esi & 0x00FFFFFF); break;
			case 3:  ecx -= 3; SearchMacroB(*(u4 const*)edi,              *(u4 const*)esi);              break;
		}
		CopyRamToggle = 1;
	}
	else if (CSInputDisplay[0] != '\0' && CSInputDisplay[0] != '_' && CSOverValue != 1)
	{
		CSInputDisplay[0] = '_';
		CSInputDisplay[1] = '\0';
		// Process Cheat Search
		u1 const  ebx = CheatSrcByteSize;
		u4        ecx = 65536 * 2 - ebx;
		u4 const  edx = SrcMask[ebx];
		u1*       edi = vidbuffer + 129600 + 65536 * 2;
		u1 const* esi = wramdata;
		u1        bl  = 0xFE;
		u1        bh  = 0x01;
		do
		{
			u4 const eax = *(u4 const*)esi & edx; // XXX unaligned
			if (eax == CSCurValue)
			{
				if (!(edi[16384] & bh)) goto failedfind;
			}
			else if ((eax + 1) & edx == CSCurValue)
			{
				if (FirstSearch == 1)
				{
					edi[16384] &= bl;
				}
				else
				{
					if (edi[16384] & bh) goto failedfind;
				}
			}
			else
			{
failedfind:
				*edi &= bl;
			}
			if (bl == 0x7F) ++edi;
			bl = bl << 1 | bl >> 7;
			bh = bh << 1 | bh >> 7;
		}
		while (++esi, --ecx != 0);
		CopyRamToggle     = 1;
		CheatSearchStatus = 1;
	}
}


void CheatCodeSearchInit(void)
{
	CSInputDisplay[0] = '_';
	CSInputDisplay[1] = '\0';
	CheatWinMode      = 1;
	CheatSearchStatus = 0;
	FirstSearch       = 1;
	// copy 128k ram
	memcpy(vidbuffer + 129600, wramdata, 131072);
	// fill searched buffer with 0xFF
	memset(vidbuffer + 129600 + 65536 * 2, 0xFF, 32768 * 4);
	if (CheatSrcSearchType == 1) CheatSearchStatus = 1;
	CheatCompareValue = 0;
	u1 val;
	switch (CheatSrcByteSize)
	{
		case 1: val = 0x7F; break;
		case 2: val = 0x3F; break;
		case 3: val = 0x1F; break;
		default: return;
	}
	vidbuffer[129600 + 65536 * 2 + 16383] &= val;
}


void DisableCheatsOnLoad(void)
{
	// Disable all codes
	u1* esi = cheatdata;
	for (u4 ecx = NumCheats; ecx != 0; esi += 28, --ecx)
	{
		if (esi[0] & 0x04) continue;
		u1* esi_ = esi;
		asm volatile("call %P1" : "+S" (esi_) : "X" (DisableCheatCode) : "cc", "memory", "eax", "ecx", "ebx");
	}
}


void EnableCheatsOnLoad(void)
{
	// Enable all ON toggled cheat codes
	u1* esi = cheatdata;
	for (u4 ecx = NumCheats; ecx != 0; esi += 28, --ecx)
	{
		if (esi[0] & 0x04) continue;
		u1* esi_ = esi;
		asm volatile("call %P1" : "+S" (esi_) : "X" (EnableCheatCode) : "cc", "memory", "eax", "ecx", "ebx");
	}
}
