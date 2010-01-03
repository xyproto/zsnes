#include <string.h>

#include "../asm.h"
#include "../asm_call.h"
#include "../c_init.h"
#include "../c_intrf.h"
#include "../cfg.h"
#include "../gblvars.h"
#include "../gui/c_gui.h"
#include "../ui.h"
#include "../vcache.h"
#include "../zmovie.h"
#include "c_execute.h"
#include "execute.h"


void start65816(void)
{
	initvideo();
	if (videotroub == 1) return;

	memset(vidbufferofsa, 0, 150072);

	if (romloadskip == 1)
		StartGUI();
	else
		continueprog();
}


void continueprog(void)
{
	// clear keyboard presses
	memset(pressed, 0, sizeof(pressed));

	romloadskip = 0;
#ifndef NO_DEBUGGER
	debuggeron  = 0;
#endif
	exiter      = 0;

	InitPreGame();
	asm_call(reexecute);
}


void endprog(void)
{
	deinitvideo();
	MovieStop();
	DosExit();
}


void interror(void)
{
#ifdef __MSDOS__
	sti();
#endif
	deinitvideo();
	PrintStr("Cannot process interrupt handler!\r\n");
	DosExit();
}


void init60hz(void)
{
	u4 const ticks = romispal != 0 ?
		23863 : // 65536/(50/((65536*24+175)/(60*60*24)))
		19900;  // 65536/(60/((65536*24+175)/(60*60*24))) // XXX off, should be 19886
	timercount = ticks;
	outb(0x43, 0x36);
	outb(0x40, ticks);
	outb(0x40, ticks >> 8);
}
