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


// Clear Backarea, 16-bit mode
void clearback16b(void)
{
	u2 c;
	if (scaddtype & 0x20 && !(scaddtype & 0x80))
	{
		u2 const dx = cgram[0];
		c = 0;

		u2 r = (dx & 0x1F) + coladdr;
		if (r > 31) r = 31;
		c += r * vidbright << vesa2_rpos;

		u2 g = (dx >> 5 & 0x1F) + coladdg;
		if (g > 31) g = 31;
		c += g * vidbright << vesa2_gpos;

		u2 b = (dx >> 10 & 0x1F) + coladdb;
		if (b > 31) b = 31;
		c += b * vidbright << vesa2_bpos;
	}
	else
	{
		c = pal16b[0];
	}
	u4  eax = c * 0x00010001;
	u1* buf = curvidoffset;
	u4  n   = 128;
	do *(u4*)buf = eax; while (buf += 4, --n != 0);
}


void procspritesmain16b(u4 const ebp)
{
	if (scrndis  & 0x10)  return;
	if (!(scrnon & 0x10)) return;
	if (winonsp == 0xFF)  return;
	u1 const cl = cursprloc[curypos & 0x00FF];
	if (sprprifix == 0) cursprloc += 256;
	if (cl == 0) return;
	u4 eax;
	u4 edx;
	u4 ebx;
	u4 esi;
	u4 edi;
	u1 cl_ = cl;
	asm volatile("push %%ebp;  mov %7, %%ebp;  call %P6;  pop %%ebp" : "=a" (eax), "+c" (cl_), "=d" (edx), "=b" (ebx), "=S" (esi), "=D" (edi) : "X" (drawsprites16b), "nr" (ebp) : "cc", "memory");
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
	clearback16b();
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
	procspritesmain16b(0);
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
	procspritesmain16b(1);
	// do background 2
	curbgpr  = 0x00;
	curbgnum = 0x02;
	asm volatile("push %%ebp;  mov %1, %%ebp;  call %P0;  pop %%ebp" :: "X" (drawbackgrndmain16b), "n" (0x01) : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");
	// do background 1
	curbgnum = 0x01;
	asm volatile("push %%ebp;  mov %1, %%ebp;  call %P0;  pop %%ebp" :: "X" (drawbackgrndmain16b), "n" (0x00) : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");
	procspritesmain16b(2);
	// do background 2
	curbgpr  = 0x20;
	curbgnum = 0x02;
	asm volatile("push %%ebp;  mov %1, %%ebp;  call %P0;  pop %%ebp" :: "X" (drawbackgrndmain16b), "n" (0x01) : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");
	// do background 1
	curbgnum = 0x01;
	asm volatile("push %%ebp;  mov %1, %%ebp;  call %P0;  pop %%ebp" :: "X" (drawbackgrndmain16b), "n" (0x00) : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");
	procspritesmain16b(3);
	if (bg3high2 == 1)
	{ // do background 3
		curbgpr  = 0x20;
		curbgnum = 0x04;
		asm volatile("push %%ebp;  mov %1, %%ebp;  call %P0;  pop %%ebp" :: "X" (drawbackgrndmain16b), "n" (0x02) : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");
	}
}
