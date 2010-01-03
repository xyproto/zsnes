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


static void set_timer_interval(u4 const ticks)
{
	timercount = ticks;
	outb(0x43, 0x36);
	outb(0x40, ticks);
	outb(0x40, ticks >> 8);
}


void init60hz(void)
{
	u4 const hz    = romispal != 0 ? 50 : 60;
	u4 const ticks = 1193182 /* frequency of the 8253/8254 */ / hz;
	set_timer_interval(ticks);
}


void init18_2hz(void)
{
	set_timer_interval(65536);
}
