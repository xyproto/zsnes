#include "../chips/fxemu2.h"
#include "../endmem.h"
#include "../init.h"
#include "../initc.h"
#include "c_irq.h"
#include "execute.h"
#include "memory.h"
#include "memseam.h"
#include "regs.h"

#include <stdio.h>
#include <stdlib.h>

/* IRQ_LOG=1 records every interrupt entry, so two builds can be diffed to find
   the first one taken at a different point in the instruction stream. Off
   unless the tree is built with WITH_DEBUG_HOOKS=1. */
#ifndef ZSNES_DEBUG_HOOKS
#define irq_log(kind, pc) ((void)0)
#else
static void irq_log(char const* const kind, u4 const pc)
{
    static int checked = 0;
    static FILE* fp = NULL;
    static unsigned long n = 0;
    if (!checked) {
        char const* e = getenv("IRQ_LOG");
        if (e && *e == '1')
            fp = fopen("/tmp/zsnes_irq.txt", "wb");
        checked = 1;
    }
    if (fp) {
        fprintf(fp, "%lu %s pc=%04x ypos=%u cyc=%u\n", n, kind, pc,
            (unsigned)curypos, (unsigned)curcyc);
        fflush(fp);
    }
    n++;
}
#endif


static u4 makedl(u4 edx)
{
	edx &= 0xFFFFFF3C;
	if ((flagnz & 0x00018000) != 0) edx |= 0x00000080; // Negative.
	if ((flagnz & 0x0000FFFF) == 0) edx |= 0x00000002; // Zero.
	if ((flagc  & 0x000000FF) != 0) edx |= 0x00000001; // Carry.
	if ((flago  & 0x000000FF) != 0) edx |= 0x00000040; // Overflow.
	return edx;
}


static void call_membank0w8(u2 const cx, u1 const al)
{
    uintptr_t const b = MemSeamB, c = MemSeamC, a = MemSeamA, d = MemSeamD;

    MemSeamC = cx;
    MemSeamA = al;
    membank0w8();
    MemSeamB = b;
    MemSeamC = c;
    MemSeamA = a;
    MemSeamD = d;
}


void IRQemulmode(zreg* const pedx, zreg* const pesi)
{
	xpc = (u2)((u1*)(uintptr_t)*pesi - initaddrl);

	u2 cx = xs;

	call_membank0w8(cx, (u1)(xpc >> 8));
	cx = ((cx - 1) & stackand) | stackor;

	call_membank0w8(cx, (u1)xpc);
	cx = ((cx - 1) & stackand) | stackor;

	u4 const edx = makedl(*pedx);
	call_membank0w8(cx, (u1)edx);
	cx = ((cx - 1) & stackand) | stackor;

	xs = cx;

	u2 const ax = irqv8;
	xpb = 0;
	xpc = ax;
	u1* const esi = ax & 0x8000 ? snesmmap[0] : snesmap2[0];
	initaddrl = esi;

	*pedx = (edx & 0xFFFFFFF3) | 0x00000004;
	*pesi = (zreg)(uintptr_t)(esi + ax);
}


void switchtovirq(zreg* const pedx, zreg* const pesi)
{
	irqon = 0x80;

#if 0 // XXX 0x00 seems wrong
	if (doirqnext & 0x02) edx = edx & 0xFFFF00FF | ((edx - (3 << 8)) & 0x0000FF00); // Cycle adjust.
#endif

	if (xe & 0x01)
	{ // IRQ emulation mode.
		IRQemulmode(pedx, pesi);
	}
	else
	{
		xpc = (u2)((u1*)(uintptr_t)*pesi - initaddrl);
		irq_log("irq", xpc);

		u2 cx = xs;

		call_membank0w8(cx, xpb);
		cx = ((cx - 1) & stackand) | stackor;

		call_membank0w8(cx, (u1)(xpc >> 8));
		cx = ((cx - 1) & stackand) | stackor;

		call_membank0w8(cx, (u1)xpc);
		cx = ((cx - 1) & stackand) | stackor;

		u4 const edx = makedl(*pedx);
		call_membank0w8(cx, (u1)edx);
		cx = ((cx - 1) & stackand) | stackor;

		xs = cx;

		u1 const bl = xirqb;
		u2 const ax = SfxSCMR & 0x00000010 ? 0x010C /* SFX NMI */ : irqv;
		xpb = bl;
		xpc = ax;
		u1* const esi = ax & 0x8000 ? snesmmap[bl] : snesmap2[bl];
		initaddrl = esi;

		*pedx = (edx & 0xFFFFFFF3) | 0x00000004;
		*pesi = (zreg)(uintptr_t)(esi + ax);
	}
}


void NMIemulmode(zreg* const pedx, zreg* const pesi)
{
	xpc = (u2)((u1*)(uintptr_t)*pesi - initaddrl);

	u2 cx = xs;

	call_membank0w8(cx, (u1)(xpc >> 8));
	cx = ((cx - 1) & stackand) | stackor;

	call_membank0w8(cx, (u1)xpc);
	cx = ((cx - 1) & stackand) | stackor;

	u4 const edx = makedl(*pedx);
	call_membank0w8(cx, (u1)edx);
	cx = ((cx - 1) & stackand) | stackor;

	xs = cx;

	u2 const ax = nmiv8;
	xpb = 0;
	xpc = ax;
	u1* const esi = ax & 0x8000 ? snesmmap[0] : snesmap2[0];
	initaddrl = esi;

	*pedx = (edx & 0xFFFFFFF3) | 0x00000004;
	*pesi = (zreg)(uintptr_t)(esi + ax);
}


void switchtonmi(zreg* const pedx, zreg* const pesi)
{
	curnmi = 1;

	// Clamp the scanline cycle counter (dh) to a floor of 130.
	u1 dh = (u1)(*pedx >> 8);
	dh = dh >= 130 ? (u1)(dh - 130) : 130;
	*pedx = (*pedx & 0xFFFF00FF) | (u4)dh << 8;

	if (xe & 0x01)
	{ // NMI emulation mode.
		NMIemulmode(pedx, pesi);
	}
	else
	{
		xpc = (u2)((u1*)(uintptr_t)*pesi - initaddrl);
		irq_log("irq", xpc);

		u2 cx = xs;

		call_membank0w8(cx, xpb);
		cx = ((cx - 1) & stackand) | stackor;

		call_membank0w8(cx, (u1)(xpc >> 8));
		cx = ((cx - 1) & stackand) | stackor;

		call_membank0w8(cx, (u1)xpc);
		cx = ((cx - 1) & stackand) | stackor;

		u4 const edx = makedl(*pedx);
		call_membank0w8(cx, (u1)edx);
		cx = ((cx - 1) & stackand) | stackor;

		xs = cx;

		u1 const bl = xirqb;
		u2 const ax = nmiv;
		xpb = bl;
		xpc = ax;
		u1* const esi = ax & 0x8000 ? snesmmap[bl] : snesmap2[bl];
		initaddrl = esi;

		*pedx = (edx & 0xFFFFFFF3) | 0x00000004;
		*pesi = (zreg)(uintptr_t)(esi + ax);
	}
}
