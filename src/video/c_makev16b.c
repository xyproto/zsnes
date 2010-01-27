#include <string.h>

#include "../asm_call.h"
#include "../c_vcache.h"
#include "../cpu/regs.h"
#include "../endmem.h"
#include "../initc.h"
#include "../ui.h"
#include "../vcache.h"
#include "c_makev16b.h"
#include "makev16b.h"
#include "makev16t.h"
#include "makevid.h"
#include "newgfx.h"


static void blanker16b(void)
{
	// calculate current video offset
	memset(vidbuffer + curypos * 576 + 32, 0, 512);
}


void drawline16b(void)
{
	cwinenabm = winenabs;

	bg3high2 = bgmode == 1 ? bg3highst : 0;
	if (curblank != 0) return;
	if (maxbr < vidbright) maxbr = vidbright;
	if (forceblnk != 0)
	{
		blanker16b();
		return;
	}
	alreadydrawn = 0;
	colormodeofs = colormodedef[bgmode];

	if (scrnon == 0x0117 && scaddset == 0x02 && scaddtype == 0x82) scrnon = 0x0116;

	if (scaddset & 0x02 ||
			(scaddtype & 0x3F && (coladdr != 0 || coladdg != 0 || coladdb != 0 || colnull != 0)))
	{
		asm_call(drawline16t);
		return;
	}
	if (bgmode == 7)
	{
		asm_call(processmode716b);
		return;
	}
	// calculate current video offset
	curvidoffset = vidbuffer + curypos * 576 + 32;
	// do sprite windowing
	asm_call(makewindowsp);
	// set palette
	asm_call(setpalette16b);
	// clear back area w/ back color
	asm_call(clearback16b);
	// get current sprite table
	currentobjptr = spritetablea + (curypos & 0x00FF) * 512;
	// setup priorities
	if (sprprifix != 0)
	{
		cursprloc = sprlefttot;
		asm_call(preparesprpr);
	}
	else
	{
		cursprloc = sprleftpr;
	}
	// process backgrounds
	// do background 2
	curbgnum = 0x02;
	asm volatile("push %%ebp;  mov %1, %%ebp;  call %P0;  pop %%ebp" :: "X" (procbackgrnd), "n" (0x01) : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");
	// do background 1
	curbgnum = 0x01;
	asm volatile("push %%ebp;  mov %1, %%ebp;  call %P0;  pop %%ebp" :: "X" (procbackgrnd), "n" (0x00) : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");
	// do background 4
	curbgnum = 0x08;
	asm volatile("push %%ebp;  mov %1, %%ebp;  call %P0;  pop %%ebp" :: "X" (procbackgrnd), "n" (0x03) : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");
	// do background 3
	curbgnum = 0x04;
	asm volatile("push %%ebp;  mov %1, %%ebp;  call %P0;  pop %%ebp" :: "X" (procbackgrnd), "n" (0x02) : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");

	if (bgmode > 1)
	{
		asm_call(priority216b);
		return;
	}
	cwinenabm = winenabm;
	curbgpr   = 0x00;
	// do background 4
	curbgnum = 0x08;
	asm volatile("push %%ebp;  mov %1, %%ebp;  call %P0;  pop %%ebp" :: "X" (drawbackgrndmain16b), "n" (0x03) : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");
	// do background 3
	curbgnum = 0x04;
	asm volatile("push %%ebp;  mov %1, %%ebp;  call %P0;  pop %%ebp" :: "X" (drawbackgrndmain16b), "n" (0x02) : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");
	asm volatile("push %%ebp;  mov %1, %%ebp;  call %P0;  pop %%ebp" :: "X" (procspritesmain16b),  "n" (0x00) : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");
	curbgpr = 0x20;
	// do background 4
	curbgnum = 0x08;
	asm volatile("push %%ebp;  mov %1, %%ebp;  call %P0;  pop %%ebp" :: "X" (drawbackgrndmain16b), "n" (0x03) : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");
	// do background 3
	if (bg3high2 != 1)
	{
		curbgnum = 0x04;
		asm volatile("push %%ebp;  mov %1, %%ebp;  call %P0;  pop %%ebp" :: "X" (drawbackgrndmain16b), "n" (0x02) : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");
	}
	asm volatile("push %%ebp;  mov %1, %%ebp;  call %P0;  pop %%ebp" :: "X" (procspritesmain16b),  "n" (0x01) : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");
	// do background 2
	curbgpr  = 0x00;
	curbgnum = 0x02;
	asm volatile("push %%ebp;  mov %1, %%ebp;  call %P0;  pop %%ebp" :: "X" (drawbackgrndmain16b), "n" (0x01) : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");
	// do background 1
	curbgnum = 0x01;
	asm volatile("push %%ebp;  mov %1, %%ebp;  call %P0;  pop %%ebp" :: "X" (drawbackgrndmain16b), "n" (0x00) : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");
	asm volatile("push %%ebp;  mov %1, %%ebp;  call %P0;  pop %%ebp" :: "X" (procspritesmain16b),  "n" (0x02) : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");
	// do background 2
	curbgpr  = 0x20;
	curbgnum = 0x02;
	asm volatile("push %%ebp;  mov %1, %%ebp;  call %P0;  pop %%ebp" :: "X" (drawbackgrndmain16b), "n" (0x01) : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");
	// do background 1
	curbgnum = 0x01;
	asm volatile("push %%ebp;  mov %1, %%ebp;  call %P0;  pop %%ebp" :: "X" (drawbackgrndmain16b), "n" (0x00) : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");
	asm volatile("push %%ebp;  mov %1, %%ebp;  call %P0;  pop %%ebp" :: "X" (procspritesmain16b),  "n" (0x03) : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");
	if (bg3high2 == 1)
	{ // do background 3
		curbgpr  = 0x20;
		curbgnum = 0x04;
		asm volatile("push %%ebp;  mov %1, %%ebp;  call %P0;  pop %%ebp" :: "X" (drawbackgrndmain16b), "n" (0x02) : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");
	}
}
