#include <string.h>

#include "../cpu/c_dispatch.h"
#include "../cpu/execute.h"
#include "../cpu/memory.h"
#include "../cpu/memseam.h"
#include "../cpu/table.h"
#include "../endmem.h"
#include "../gblvars.h"
#include "../initc.h"
#include "../ui.h" /* wramdata */
#include "sa1proc.h"
#include "sa1regs.h"

// SA-1 65816 flag words and interrupt vectors (defined in initdata.c / sa1regs.c).
extern u4 Sflagnz;
extern u4 Sflagc;
extern u4 Sflago;
extern u4 SA1NMIV;
extern u4 SA1IRQV;

// The rest of the swap state, from the save-state block in chips/sa1regs.c and
// from cpu/execute.asm / chips/sa1proc.asm.
extern u1* SA1Ptr;
extern u1* SNSPtr;
extern u1* SNSRegPCS;
extern u1 SNSRegP;
extern u4 SA1TimerVal;
extern u1 CurrentExecSA1;
extern u1 SA1SHb; // low byte of a dword in cpu/execute.asm
extern u1 wramdataa[65536];

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
    u4 const b = MemSeamB, c = MemSeamC, a = MemSeamA, d = MemSeamD;

    MemSeamC = cx;
    MemSeamA = al;
    membank0w8();
    MemSeamB = b;
    MemSeamC = c;
    MemSeamA = a;
    MemSeamD = d;
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

// dh is the scanline cycle counter; the assembly's `add dh,n` wraps in 8 bits.
static u4 dh_plus(u4 const edx, u1 const n)
{
    return edx & 0xFFFF00FF | (u4)(u1)((u1)(edx >> 8) + n) << 8;
}

static u4 peek32(u1 const* const p)
{
    u4 v;
    memcpy(&v, p, sizeof v);
    return v;
}

// Idle-loop detection: these opcode words are the SA-1 spinning on a flag the
// 65816 has not set yet, so the slot is skipped and only cycles are charged.
// Returns the cycle charge for a skipped slot, or 0 to run an instruction.
static u1 SA1IdleCharge(u1 const* const p)
{
    if (SA1DoIRQ & 1)
        return 0;
    if (IRAM[0x00] == 0 && (peek32(p) == 0xFCF000A5 || peek32(p - 2) == 0xFCF000A5))
        return 18;
    if (SA1SHb == 1)
        return 15;
    if (*(u2 const*)(SA1BWPtr + 0x72A4) == 0 && peek32(p) == 0xF072A4AD)
        return 15;
    if (IRAM[0x72] == 0 && peek32(p) == 0xF03072AD)
        return 15;
    return 0;
}

static u4 SA1SwapEnter(u4* const r)
{
    u1* const p = SA1Ptr;

    r[R_ECX] = 0;

    u1 const idle = SA1IdleCharge(p);
    if (idle != 0) {
        r[R_EDX] = dh_plus(r[R_EDX], idle);
        r[R_EAX] = (u4)p;
        CurrentExecSA1 += 2;
        SA1Status = 0;
        return 0;
    }

    // Save the 65816 context, install the SA-1's.
    SNSRegP = (u1)r[R_EDX];
    SNSRegPCS = initaddrl;
    prevedi = r[R_EDI];
    SNSPtr = (u1*)r[R_ESI];

    u4 edx = r[R_EDX] & 0xFFFFFF00 | SA1RegP;
    initaddrl = SA1RegPCS;
    CurBWPtr = SA1BWPtr;
    snesmap2[0] = IRAM;
    wramdata = IRAM;

    u4 const eax = (u1)edx;
    edx = dh_plus(edx, 20);
    SA1Status = 1;

    r[R_EAX] = eax;
    r[R_EDX] = edx;
    r[R_ESI] = (u4)SA1Ptr;
    r[R_EDI] = (u4)SA1tablead[eax];

    if (SA1DoIRQ & 0xFF000003) {
        if (SA1DoIRQ & 3) {
            u1* esi = (u1*)r[R_ESI];
            if (SA1DoIRQ & 1) {
                SA1DoIRQ &= 0xFFFFFFFE;
                SA1switchtovirq(&r[R_EDX], &esi);
            } else {
                SA1DoIRQ &= 0xFFFFFFFD;
                SA1switchtonmi(&r[R_EDX], &esi);
            }
            r[R_ESI] = (u4)esi;
        } else if (--((u1*)&SA1DoIRQ)[3] == 0) {
            ((u1*)&SA1DoIRQ)[0] |= 8;
        }
    }
    return 1;
}

static void SA1SwapLeave(u4* const r)
{
    // Save the SA-1 context, restore the 65816's.
    SA1RegP = (u1)r[R_EDX];
    SA1RegPCS = initaddrl;
    SA1Ptr = (u1*)r[R_ESI];

    initaddrl = SNSRegPCS;
    CurBWPtr = SNSBWPtr;
    wramdata = wramdataa;
    snesmap2[0] = wramdata;

    r[R_EDX] = dh_plus(r[R_EDX] & 0xFFFFFF00 | SNSRegP, 11);
    r[R_ESI] = (u4)SNSPtr;
    r[R_EDI] = prevedi;
    r[R_EAX] = 0;

    CurrentExecSA1++;
    SA1Status = 0;
    SA1TimerVal += 23;
}

// SA1Swap - give the SA-1 one instruction slot.
//
// Install the SA-1's context, run a chain of its instructions until the
// scanline's cycles are spent, then hand the 65816 its context back. The SA-1
// core's opcode tails jumped to one another through their own `endloop`, which
// unlike the 65816's does not step the SPC700; the loop here is that macro.
void SA1Swap(u4* const r)
{
    if (SA1SwapEnter(r) == 0)
        return;

    set_bl(r, *(u1*)r[R_ESI]);
    r[R_ESI]++;

    for (;;) {
        ((opfn**)r[R_EDI])[r[R_EBX]](r);

        set_bl(r, *(u1*)r[R_ESI]);
        r[R_ESI]++;

        u1 const c = cpucycle[r[R_EBX]];
        u1 const dh = DH(r);
        set_dh(r, (u1)(dh - c));
        if (dh < c)
            break;
    }

    r[R_ESI]--;
    SA1SwapLeave(r);
}
