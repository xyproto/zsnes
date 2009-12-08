#include <string.h>

#include "../asm_call.h"
#include "../cpu/dspproc.h"
#include "../cpu/execute.h"
#include "../cpu/spc700.h"
#include "../gblvars.h"
#include "../initc.h"
#include "../intrf.h"
#include "../video/procvidc.h"
#include "../zmovie.h"
#include "../zstate.h"
#include "c_gui.h"
#include "c_guimisc.h"
#include "gui.h"
#include "guikeys.h"


void CalibrateDispA(void)
{
	memset(pressed, 0, 256); // XXX Probably should be sizeof(pressed)
	asm_call(GUIUnBuffer);
	asm_call(DisplayBoxes);
	asm_call(DisplayMenu);
	GUIBox3D(75, 103, 192, 135);
	GUIOuttextShadowed(80, 108, "PRESS THE TOP LEFT");
	GUIOuttextShadowed(80, 116, "CORNER AND PRESS A");
	GUIOuttextShadowed(80, 124, "BUTTON OR KEY");
	asm_call(vidpastecopyscr);
	asm_call(GUIWaitForKey);
}


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
