#include <string.h>

#include "../asm_call.h"
#include "../c_intrf.h"
#include "../cpu/dspproc.h"
#include "../cpu/execute.h"
#include "../cpu/spc700.h"
#include "../gblvars.h"
#include "../initc.h"
#include "../input.h"
#include "../macros.h"
#include "../video/procvidc.h"
#include "../zmovie.h"
#include "../zstate.h"
#include "c_gui.h"
#include "c_guimisc.h"
#include "c_guimouse.h"
#include "gui.h"
#include "guikeys.h"
#include "guimouse.h"


void CalibrateDispA(void)
{
	memset(pressed, 0, 256); // XXX Probably should be sizeof(pressed)
	GUIUnBuffer();
	DisplayBoxes();
	DisplayMenu();
	GUIBox3D(75, 103, 192, 135);
	GUIOuttextShadowed(80, 108, "PRESS THE TOP LEFT");
	GUIOuttextShadowed(80, 116, "CORNER AND PRESS A");
	GUIOuttextShadowed(80, 124, "BUTTON OR KEY");
	vidpastecopyscr();
	asm_call(GUIWaitForKey);
}


void CalibrateDispB(void)
{
	memset(pressed, 0, 256); // XXX Probably should be sizeof(pressed)
	GUIUnBuffer();
	DisplayBoxes();
	DisplayMenu();
	GUIBox3D(75, 103, 192, 143);
	GUIOuttextShadowed(80, 108, "PRESS THE BOTTOM");
	GUIOuttextShadowed(80, 116, "RIGHT CORNER AND");
	GUIOuttextShadowed(80, 124, "PRESS A BUTTON OR");
	GUIOuttextShadowed(80, 132, "KEY");
	vidpastecopyscr();
	asm_call(GUIWaitForKey);
}


#define ConfigureKey2(i, player)                                 \
do                                                               \
{                                                                \
	switch (i)                                                     \
	{                                                              \
		case  0: guicpressptr = &player##upk;    break; /* Up     */ \
		case  1: guicpressptr = &player##downk;  break; /* Down   */ \
		case  2: guicpressptr = &player##leftk;  break; /* Left   */ \
		case  3: guicpressptr = &player##rightk; break; /* Right  */ \
		case  4: guicpressptr = &player##startk; break; /* Start  */ \
		case  5: guicpressptr = &player##selk;   break; /* Select */ \
		case  6: guicpressptr = &player##Ak;     break; /* A      */ \
		case  7: guicpressptr = &player##Bk;     break; /* B      */ \
		case  8: guicpressptr = &player##Xk;     break; /* X      */ \
		case  9: guicpressptr = &player##Yk;     break; /* Y      */ \
		case 10: guicpressptr = &player##Lk;     break; /* L      */ \
		case 11: guicpressptr = &player##Rk;     break; /* R      */ \
	}                                                              \
}                                                                \
while (0)

void SetAllKeys(void)
{
	static char const guipresstext4b[][21] =
	{
		"FOR UP              ",
		"FOR DOWN            ",
		"FOR LEFT            ",
		"FOR RIGHT           ",
		"FOR START           ",
		"FOR SELECT          ",
		"FOR A (RIGHT BUTTON)",
		"FOR B (DOWN BUTTON) ",
		"FOR X (TOP BUTTON)  ",
		"FOR Y (LEFT BUTTON) ",
		"FOR THE L BUTTON    ",
		"FOR THE R BUTTON    "
	};

	memset(pressed, 0, sizeof(pressed));

	GUICBHold = 0;
	switch (cplayernum)
	{
		default: keycontrolval = &pl1contrl; break;
		case 1:  keycontrolval = &pl2contrl; break;
		case 2:  keycontrolval = &pl3contrl; break;
		case 3:  keycontrolval = &pl4contrl; break;
		case 4:  keycontrolval = &pl5contrl; break;
	}

	// Check if controller is set
	if (*keycontrolval == 0) return; // XXX original compares dword instead of byte, former makes no sense
	u4 i = 0;
	u4 n = lengthof(guipresstext4b);
	guipressptr = guipresstext4b[0];
	do
	{
		switch (cplayernum)
		{
			case 0: ConfigureKey2(i, pl1); break;
			case 1: ConfigureKey2(i, pl2); break;
			case 2: ConfigureKey2(i, pl3); break;
			case 3: ConfigureKey2(i, pl4); break;
			case 4: ConfigureKey2(i, pl5); break;
		}
		guipresstestb();
		guipressptr += lengthof(*guipresstext4b); // XXX ugly, crosses lines of the array
		++i;
	}
	while (--n != 0);
}

#undef ConfigureKey2


void GUIDoReset(void)
{
#ifdef __MSDOS__
	asm_call(DOSClearScreen);
#endif
	Clear2xSaIBuffer();

	MovieStop();
	RestoreSystemVars();

	// reset the snes
	init65816();
	procexecloop();

	spcPCRam = SPCRAM + 0xFFC0;
	spcS     = 0x1EF;
	spcRamDP = SPCRAM;
	spcA     = 0;
	spcX     = 0;
	spcY     = 0;
	spcP     = 0;
	spcNZ    = 0;
	GUIQuit  = 2;
	memset(&Voice0Status, 0, sizeof(Voice0Status));
}
