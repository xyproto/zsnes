#include <string.h>

#include "../asm.h"
#include "../asm_call.h"
#include "../c_init.h"
#include "../c_intrf.h"
#include "../cfg.h"
#include "../debugger.h"
#include "../endmem.h"
#include "../gblvars.h"
#include "../gui/c_gui.h"
#include "../gui/gui.h"
#include "../gui/guimisc.h"
#include "../gui/menu.h"
#include "../init.h"
#include "../initc.h"
#include "../macros.h"
#include "../ui.h"
#include "../vcache.h"
#include "../video/c_mode716.h"
#include "../video/procvid.h"
#include "../zmovie.h"
#include "../zstate.h"
#include "c_65816d.h"
#include "c_execute.h"
#include "c_memory.h"
#include "execute.h"
#include "memory.h"
#include "memtable.h"
#include "regs.h"
#include "regsw.h"
#include "spc700.h"


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


static void reexecuteb2(void)
{
	if (NoSoundReinit != 1) SetupPreGame();

	UpdateDPage();
	SA1UpdateDPage();
	Makemode7Table();
	if (SFXEnable != 0) asm_call(UpdateSFX);

	curexecstate  |= 2;
	NoSoundReinit  = 0;
	csounddisable  = 0;
	NextNGDisplay  = 0;

	u4  const pc   = xpc;
	u4  const pb   = xpb;
	u1* const addr =
		pc & 0x8000                                      ? snesmmap[pb] :
		pc < 0x4300 || memtabler8[pb] != regaccessbankr8 ? snesmap2[pb] :
		dmadata - 0x4300;
	initaddrl = addr;

	// initialize variables (Copy from variables)
	u4    ecx = 0;
	u4    edx = curcyc /* cycles */ << 8 | xp /* flags */;
	u4    ebx = 0;
	u1*   ebp = spcPCRam;
	u1*   esi = addr + pc; // add program counter to address
	eop** edi = tableadc[xp];

	splitflags(edx);
	// XXX hack: GCC cannot handle ebp as input/output, so take the detour over eax
	asm volatile("push %%ebp;  mov %0, %%ebp;  call %P6;  mov %%ebp, %0;  pop %%ebp" : "+a" (ebp), "+c" (ecx), "+d" (edx), "+b" (ebx), "+S" (esi), "+D" (edi) : "X" (execute) : "cc", "memory");
	edx = joinflags(edx);

	// de-init variables (copy to variables)
	spcPCRam     = ebp;
	Curtableaddr = (u4)edi; // XXX TODO type
	xp           = edx;
	curcyc       = edx >> 8;
	xpc          = esi - initaddrl; // subtract program counter by address

#ifdef __MSDOS__
	asm_call(ResetTripleBuf);
#endif

	if (pressed[KeySaveState] & 1 || pressed[KeyLoadState] & 1)
	{
		NoSoundReinit = 1;
		csounddisable = 1;
	}

	if (NoSoundReinit != 1) DeInitPostGame();

	// Multipass Movies
	if (MoviePassWaiting == 1)
	{
		MovieDumpRaw();
		continueprog();
		return;
	}

	// clear all keys
	while (Check_Key() != 0) Get_Key();

	if (nextmenupopup == 1)
	{
		showmenu();
	}
	else if (ReturnFromSPCStall == 1)
	{
		goto activatereset;
	}
	else if (pressed[KeySaveState] & 1)
	{
		pressed[1]            = 0;
		pressed[KeySaveState] = 2;
		statesaver();
		reexecuteb();
	}
	else if (pressed[KeyLoadState] & 1)
	{
		loadstate();
		reexecuteb();
	}
	else if (pressed[KeyInsrtChap] & 1)
	{
		pressed[KeyInsrtChap] = 0;
		MovieInsertChapter();
		continueprognokeys();
	}
	else if (pressed[KeyNextChap] & 1)
	{
		pressed[KeyNextChap] = 0;
		multchange           = 1;
		MovieSeekAhead();
		continueprognokeys();
	}
	else if (pressed[KeyPrevChap] & 1)
	{
		pressed[KeyPrevChap] = 0;
		multchange           = 1;
		MovieSeekBehind();
		continueprognokeys();
	}
	else if (SSKeyPressed == 1 || SPCKeyPressed == 1)
	{
		showmenu();
	}
#ifndef NO_DEBUGGER
	else if (debugdisble == 0 && pressed[59] & 1)
	{
		startdebugger();
	}
#endif
	else if (pressed[59] & 1)
	{
		showmenu();
	}
	else if (pressed[KeyQuickRst] & 1)
	{
activatereset:
		GUIReset = 1;
		if (MovieProcessing == 2)
		{ // Recording
			ResetDuringMovie();
		}
		else
		{
			GUIDoReset();
		}
		ReturnFromSPCStall = 0;
		continueprog();
	}
	else if (guioff == 1 || pressed[KeyQuickExit] & 1)
	{
		endprog();
	}
	else
	{
		StartGUI();
	}
}


static void reexecute(void)
{
	// clear keyboard presses
	u1* i = pressed;
	do
	{
		if (*i == 2) *i = 0;
	}
	while (++i != endof(pressed));
	reexecuteb2();
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
	reexecute();
}


void continueprognokeys(void)
{
	romloadskip = 0;
#ifndef NO_DEBUGGER
	debuggeron  = 0;
#endif
	exiter      = 0;

	InitPreGame();
	reexecuteb2();
}


// Incorrect
void reexecuteb(void)
{
#ifndef __MSDOS__
	reexecuteb2();
#else
	reexecute();
#endif
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
