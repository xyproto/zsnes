#include <string.h>

#include "asm_call.h"
#include "c_init.h"
#include "cfg.h"
#include "cpu/execute.h"
#include "cpu/regs.h"
#include "cpu/regsw.h"
#include "cpu/stable.h"
#include "cpu/table.h"
#include "cpu/tablec.h"
#include "debugger.h"
#include "gui/gui.h"
#include "gui/guiwindp.h"
#include "init.h"
#include "initc.h"
#include "intrf.h"
#include "link.h"
#include "macros.h"
#include "ui.h"
#include "video/mode716.h"
#include "zmovie.h"
#include "zpath.h"
#include "zstate.h"


void init(void)
{
#if 0 // XXX This seems very strange
	// prevents a crash if cpalval gets accessed before initializing
	for (void** i = cpalval; i != endof(cpalval); ++i) *i = cpalval;
#endif

	// Initialize snow stuff
	for (u4 i = 0; i != 400; ++i) // XXX Why only 400 of 800?
	{
		SnowData[i]   <<= 8;
		SnowVelDist[i] &= 0xF7;
		if (SnowTimer == 0) SnowVelDist[i] |= 0x08;
	}

	BackupSystemVars();

	forceromtype = romtype;
	romtype      = 0;

	/* XXX sndrot is a global variable, but is treated like the first entry of a
	 * big struct */
	memcpy(regsbackup, &sndrot, sizeof(regsbackup));

	clearmem();

	asm_call(inittable);
	asm_call(inittablec);
	asm_call(SA1inittable);

	// SPC Init
	procexecloop();

	// SNES Init
	Setper2exec();

	asm_call(Makemode7Table);

	if (ZCartName[0] != '\0' || romloadskip != 1)
	{
		romloadskip = 0;
		loadfileGUI();
		SetupROM();
		if (DisplayInfo != 0) showinfogui();
	}

	asm_call(UpdateDevices);
	init65816();
	asm_call(initregr);
	asm_call(initregw);
	initsnes();

	u4 const vol = MusicRelVol * 128 * 0xA3D70A3DULL >> 38;
	MusicVol = vol < 127 ? vol : 127;

	if (AutoState == 1) LoadSecondState();

	// FIX STATMAT
	// Here's the auto-load ZST file stuff
	if (autoloadstate >= 1)
	{
		current_zst = autoloadstate - 1;
		// Load the specified state file
		loadstate2();
	}

	if (1 <= autoloadmovie && autoloadmovie <= 10)
	{
		CMovieExt = autoloadmovie != 1 ? '0' + (autoloadmovie - 1) : 'v';

		if (ZMVRawDump != 0)
		{
			MovieDumpRaw();
		}
		else
		{
			MoviePlay();
		}
	}

	if (yesoutofmemory == 1) asm_call(outofmemfix);

#ifndef NO_DEBUGGER
	if (debugger != 0 && romloadskip != 1)
	{
#	ifndef __MSDOS__
		/* Prevent nasty hang in debugger. Likely not a good way...
		 * If we don't do this, then on the SDL and win32 ports, update_ticks_pc2
		 * won't be set and CheckTimers will hang. */

		/* Most likely it isn't desirable to be checking timers under the
		 * debugger anyway, but this is a much simpler fix. */
#		ifdef __WIN32__
		// need to get "freq" set first
		initwinvideo();
#		endif
		Start60HZ();
#	endif
		startdebugger();
	}
	else
#endif
	{
		asm_call(start65816);
	}
}
