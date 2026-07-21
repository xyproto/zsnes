#include "../cpu/execute.h"
#include "../cpu/memory.h"
#include "../endmem.h"
#include "../gblvars.h"
#include "../initc.h"
#include "sa1regs.h"

// SA-1 65816 flag words and interrupt vectors (defined in initdata.c / sa1regs.c).
extern u4 Sflagnz;
extern u4 Sflagc;
extern u4 Sflago;
extern u4 SA1NMIV;
extern u4 SA1IRQV;

// Build the SA-1 status byte (dl) from its flag words, keeping the caller's
// bits 2-5. Mirrors the `makedl` macro in the assembly.
static u4 SA1makedl(u4 edx)
{
    edx &= 0xFFFFFF3C;
    if ((Sflagnz & 0x00018000) != 0)
        edx |= 0x00000080; // Negative.
    if ((Sflagnz & 0x0000FFFF) == 0)
        edx |= 0x00000002; // Zero.
    if ((Sflagc & 0x000000FF) != 0)
        edx |= 0x00000001; // Carry.
    if ((Sflago & 0x000000FF) != 0)
        edx |= 0x00000040; // Overflow.
    return edx;
}

static void call_membank0w8(u2 const cx, u1 const al)
{
    u4 eax;
    u4 ecx;
    u4 ebx;
    __asm__ volatile("call %P3" : "=a"(eax), "=c"(ecx), "=b"(ebx) : "X"(membank0w8), "a"(al), "c"(cx) : "cc", "memory");
}

// Push the SA-1 return context onto its stack and jump to the NMI/IRQ vector.
// vec is the vector (SA1NMIV or SA1IRQV); irqexec_off selects which SA1IRQExec
// byte is flagged (2 = NMI, 1 = IRQ).
static void SA1switch(u4* const pedx, u1** const pesi, u2 const vec, int const irqexec_off)
{
    ((u1*)&SA1Message)[2] = (u1)SA1Message;
    ((u1*)&SA1IRQExec)[irqexec_off] = 1;

    u2 const xpc = (u2)(*pesi - initaddrl);
    SA1xpc = SA1xpc & 0xFFFF0000 | xpc;

    u2 cx = SA1xs;

    call_membank0w8(cx, (u1)SA1xpb);
    cx = (cx - 1) & stackand | stackor;

    call_membank0w8(cx, (u1)(xpc >> 8));
    cx = (cx - 1) & stackand | stackor;

    call_membank0w8(cx, (u1)xpc);
    cx = (cx - 1) & stackand | stackor;

    u4 const edx = SA1makedl(*pedx);
    call_membank0w8(cx, (u1)edx);
    cx = (cx - 1) & stackand | stackor;

    SA1xs = cx;

    SA1xpb = 0;
    u1* const esi = vec & 0x8000 ? snesmmap[0] : snesmap2[0];
    initaddrl = esi;

    *pedx = edx & 0xFFFFFFF3 | 0x00000004;
    *pesi = esi + vec;
}

void SA1switchtonmi(u4* const pedx, u1** const pesi)
{
    SA1switch(pedx, pesi, (u2)SA1NMIV, 2);
}

void SA1switchtovirq(u4* const pedx, u1** const pesi)
{
    SA1switch(pedx, pesi, (u2)SA1IRQV, 1);
}
