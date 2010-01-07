#ifndef NCURSES
#include <curses.h>
#else
#include <ncurses.h>
#endif

#include "c_debugasm.h"
#include "cpu/65816d.h"
#include "cpu/c_memory.h"
#include "cpu/execute.h"
#include "cpu/memory.h"
#include "cpu/memtable.h"
#include "cpu/regs.h"
#include "cpu/spc700.h"
#include "debugger.h"
#include "endmem.h"
#include "init.h"
#include "initc.h"
#include "types.h"


void breakops(void)
{
	u4        const page      = PrevBreakPt_page;
	u4        const offset    = PrevBreakPt_offset;
	u1 const* const map       = offset & 0x8000 ? snesmmap[page] : snesmap2[page];
	u1 const* const breakarea = map + offset; // add program counter to address

	u4  const pc   = xpc;
	u4  const pb   = xpb;
	u1* const addr =
		pc & 0x8000                                      ? snesmmap[pb] :
		pc < 0x4300 || memtabler8[pb] != regaccessbankr8 ? snesmap2[pb] :
		dmadata - 0x4300;
	initaddrl = addr;

	u4  ecx = 0;
	u4  edx = curcyc /* cycles */ << 8 | xp /* flags */;
	u1* ebp = spcPCRam;
	u1* esi = addr + pc; // add program counter to address
	u4  edi = Curtableaddr;
	UpdateDPage();
	// execute
	do
	{
		asm volatile("call %P0" :: "X" (splitflags), "d" (edx) : "cc", "memory");
		u4 ebx;
		// XXX hack: GCC cannot handle ebp as input/output, so take the detour over eax
		asm volatile("push %%ebp;  mov %0, %%ebp;  call %P6;  mov %%ebp, %0;  pop %%ebp" : "+a" (ebp), "+c" (ecx), "+d" (edx), "=b" (ebx), "+S" (esi), "+D" (edi) : "X" (execsingle) : "cc", "memory");
		asm volatile("call %P1" : "+d" (edx) : "X" (joinflags) : "cc", "memory");
    edx = edx & 0xFFFF00FF | pdh << 8;
		if ((++numinst & 0xFF) == 0 && getch() == 27) break;
	}
	while (esi != breakarea);
	// copy back data
	spcPCRam     = ebp;
	Curtableaddr = edi;
	xp           = edx;
	curcyc       = edx >> 8;
	xpc          = esi - initaddrl; // subtract program counter by address
}
