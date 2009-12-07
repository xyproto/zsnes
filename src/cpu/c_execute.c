#include <string.h>

#include "../asm_call.h"
#include "../c_intrf.h"
#include "../cfg.h"
#include "../gblvars.h"
#include "../gui/c_gui.h"
#include "../ui.h"
#include "../vcache.h"
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
