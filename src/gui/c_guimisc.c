#include <string.h>

#include "../cpu/dspproc.h"
#include "../cpu/spc700.h"
#include "../gblvars.h"
#include "../initc.h"
#include "../video/procvidc.h"
#include "../zmovie.h"
#include "../zstate.h"
#include "c_guimisc.h"
#include "gui.h"

#ifdef __MSDOS__
#	include "../asm_call.h"
#endif


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
