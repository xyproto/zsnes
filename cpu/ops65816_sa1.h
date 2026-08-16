/*
 * cpu/ops65816_sa1.h - the SA-1's instantiation of the 65816 core.
 *
 * Include this instead of ops65816.h: it sets the override guards, pulls the
 * shared handlers in, and then defines the six the SA-1 does differently. The
 * caller supplies the register file (see cpu/c_ops65816_sa1.c) - which for the
 * SA-1 means its own A/X/Y/S/D, its own flags, its own opcode and direct-page
 * tables, and everything else shared with the 65816.
 */
#ifndef OPS65816_SA1_H
#define OPS65816_SA1_H

#ifndef NO_DEBUGGER
extern u1 debstop4;
#endif
extern u1 IRAM[2049];

/*
 * The six the SA-1 does differently. Five are deliberate - it is always in
 * native mode, it has no IRQ to switch to, and BRK is a stop rather than a
 * vector - but the sixth is a defect in the original: SA1COpD6m8 is the 8-bit
 * DEC d,x and it reads and writes sixteen bits while decrementing only al, so
 * it touches the following byte and writes it back unchanged. The 65816 core
 * next door uses the 8-bit modes. Reproduced here rather than fixed, so the
 * port stays bit-identical; see the bug backlog.
 */
#define OPS_OWN_COp00
#define OPS_OWN_COp1B
#define OPS_OWN_COp40
#define OPS_OWN_COp58

/* The stack macros differ: the SA-1's start with `mov eax,[wramdata]`, which
   leaves that pointer's upper three bytes in eax. */
#define OPS_OWN_COp48m8
#define OPS_OWN_COp8B
#define OPS_OWN_COp4B
#define OPS_OWN_COpDAx8
#define OPS_OWN_COp5Ax8
#define OPS_OWN_COp08
#define OPS_OWN_COp68m8
#define OPS_OWN_COpAB
#define OPS_OWN_COpFAx8
#define OPS_OWN_COp7Ax8
#define OPS_OWN_COp68m16
#define OPS_OWN_COpFAx16
#define OPS_OWN_COp7Ax16
#define OPS_OWN_COp2B
#define OPS_OWN_COp28
/* And these each diverge on their own: no DMA routing, no emulation early-out,
   an IRAM window, a simpler RTI. */
#define OPS_OWN_COpFB
#define OPS_OWN_COpCB
#define OPS_OWN_COp4C
#define OPS_OWN_COp20
#define OPS_OWN_COpFC
#define OPS_OWN_COp22
#define OPS_OWN_COp02

#include "ops65816.h"

void OP(COp00)(u4* const r) /* BRK s - backs up and stops */
{
#ifndef NO_DEBUGGER
    debstop4 = 1;
#endif
    r[R_ESI]--;
}

void OP(COp1B)(u4* const r) /* TCS i - the SA-1 is always native */
{
    AX(r, GET16(xa));
    SET16(xs, GET16(r[R_EAX]));
}

void OP(COp58)(u4* const r) /* CLI i - nothing to switch to */
{
    r[R_EDX] &= ~0x04u;
}

/* DEC d,x is deliberately NOT overridden: the shared 8-bit handler is the fix.
   See the comment above - this is the one place the port does not reproduce the
   assembly, and test/difftest_sa1.c records it as a known divergence. */

WRAM_PUSH8(COp48m8, xa) /* PHA s */
WRAM_PUSH8(COp8B, xdb) /* PHB s */
WRAM_PUSH8(COp4B, xpb) /* PHK s */
WRAM_PUSH8(COpDAx8, xx) /* PHX s */
WRAM_PUSH8(COp5Ax8, xy) /* PHY s */
WRAM_POP8(COp68m8, xa) /* PLA s */
WRAM_POP8(COpAB, xdb) /* PLB s */
WRAM_POP8(COpFAx8, xx) /* PLX s */
WRAM_POP8(COp7Ax8, xy) /* PLY s */
WRAM_POP16(COp68m16, xa)
WRAM_POP16(COpFAx16, xx)
WRAM_POP16(COp7Ax16, xy)

void OP(COp08)(u4* const r) /* PHP s */
{
    r[R_EAX] = (u4)(uintptr_t)wramdata;
    r[R_EDX] = makedl(r[R_EDX]);
    SET16(r[R_ECX], GET16(xs));
    push8(r, (u1)r[R_EDX]);
    SET16(xs, GET16(r[R_ECX]));
}

void OP(COp2B)(u4* const r) /* PLD s */
{
    u1 hi;
    r[R_EAX] = (u4)(uintptr_t)wramdata;
    SET16(r[R_ECX], GET16(xs));
    SET8(xd, pop8(r));
    SET16(xs, GET16(r[R_ECX]));
    hi = pop8(r);
    xd = (xd & 0xFFFF00FFu) | (u4)hi << 8;
    UpdateDPage();
    SET16(xs, GET16(r[R_ECX]));
    AX(r, (u2)((u2)hi << 8 | GET8(xd)));
    setnz16(r, GET16(r[R_EAX]));
}

void OP(COp28)(u4* const r) /* PLP s - no `xor bh,bh` here */
{
    u1 p;
    SET16(r[R_ECX], GET16(xs));
    p = pop8(r);
    SET16(xs, GET16(r[R_ECX]));
    SET8(r[R_EDX], p);
    restoredl(r[R_EDX]);
    if (xe & 1) {
        r[R_EDX] |= 0x30u;
        reload_table(r);
        return;
    }
    reload_table(r);
    if (r[R_EDX] & 0x10u) {
        xx &= 0xFFFF00FFu;
        xy &= 0xFFFF00FFu;
    }
}

void OP(COpFB)(u4* const r) /* XCE i - always swaps, no early-out */
{
    AL(r, (u1)(flagc & 1u));
    flagc = 0;
    if (xe != 0)
        flagc = 0xFF;
    xe = GET8(r[R_EAX]);
    if (xe & 1) {
        r[R_EDX] |= 0x30u;
        reload_table(r);
        xx &= 0xFFFF00FFu;
        xy &= 0xFFFF00FFu;
        xs = (xs & 0xFFFF00FFu) | 0x0100u;
        stackand = 0x01FF;
        stackor = 0x0100;
    } else {
        r[R_EDX] |= 0x20u;
        stackand = 0xFFFF;
        stackor = 0x0000;
    }
}

void OP(COpCB)(u4* const r) /* WAI i - does not clear doirqnext */
{
    if (intrset == 1) {
        r[R_ESI]--;
        return;
    }
    if (intrset != 0) {
        if (intrset == 2) {
            intrset = 0;
            return;
        }
        r[R_ESI]--;
        return;
    }
    intrset = 1;
    r[R_ESI]--;
}

void OP(COp4C)(u4* const r) /* JMP a - no $4300 routing */
{
    r[R_EAX] = 0;
    AX(r, *(u2 const*)(uintptr_t)r[R_ESI]);
    SET8(r[R_EBX], GET8(xpb));
    xpc = GET16(r[R_EAX]);
    jump_to(r, 0);
}

void OP(COp20)(u4* const r) /* JSR a - no $4300 routing */
{
    r[R_EBX] = r[R_ESI] - (u4)(uintptr_t)initaddrl;
    SET16(r[R_EBX], (u2)(GET16(r[R_EBX]) + 1));
    xpc = GET16(r[R_EBX]);
    push_pc(r);
    SET16(xs, GET16(r[R_ECX]));
    r[R_EAX] = 0;
    AX(r, *(u2 const*)(uintptr_t)r[R_ESI]);
    r[R_EBX] &= 0xFFFF00FFu;
    xpc = GET16(r[R_EAX]);
    SET8(r[R_EBX], GET8(xpb));
    jump_to(r, 0);
}

void OP(COpFC)(u4* const r) /* JSR (a,x) */
{
    r[R_EBX] = r[R_ESI] - (u4)(uintptr_t)initaddrl;
    SET16(r[R_EBX], (u2)(GET16(r[R_EBX]) + 1));
    xpc = GET16(r[R_EBX]);
    push_pc(r);
    SET16(xs, GET16(r[R_ECX]));
    r[R_EAX] = 0;
    r[R_EBX] &= 0xFFFF00FFu;
    SET16(r[R_ECX], *(u2 const*)(uintptr_t)r[R_ESI]);
    SET8(r[R_EBX], GET8(xpb));
    SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) + GET16(xx)));
    TABR16(r);
    xpc = GET16(r[R_EAX]);
    SET8(r[R_EBX], GET8(xpb));
    jump_to(r, 0);
}

/* JSL reaches a window of IRAM that the 65816 has no equivalent of, and it
   omits the stackor fixup after pushing the bank. */
void OP(COp22)(u4* const r) /* JSL al */
{
    r[R_EBX] = r[R_ESI] - (u4)(uintptr_t)initaddrl;
    SET16(r[R_EBX], (u2)(GET16(r[R_EBX]) + 2));
    xpc = GET16(r[R_EBX]);
    SET16(r[R_ECX], GET16(xs));
    AL(r, GET8(xpb));
    bank0_call(r, c_membank0w8);
    SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) - 1)); /* no `or cx,stackor` */
    push8(r, (u1)(xpc >> 8));
    push8(r, (u1)xpc);
    SET16(xs, GET16(r[R_ECX]));
    r[R_EAX] = 0;
    r[R_EBX] &= 0xFFFF00FFu;
    AX(r, *(u2 const*)(uintptr_t)r[R_ESI]);
    SET8(r[R_EBX], *(u1 const*)(uintptr_t)(r[R_ESI] + 2));
    xpc = GET16(r[R_EAX]);
    SET8(xpb, GET8(r[R_EBX]));
    if (r[R_EAX] & 0x8000u) {
        jump_to(r, 0);
        return;
    }
    if (r[R_EBX] == 0 && r[R_EAX] > 0x2000u) {
        initaddrl = IRAM - 0x3000;
        r[R_EAX] = (r[R_EAX] & 0x7FFu) + 0x3000u;
        r[R_ESI] = (u4)(uintptr_t)initaddrl + r[R_EAX];
        return;
    }
    initaddrl = snesmap2[r[R_EBX]];
    r[R_ESI] = (u4)(uintptr_t)initaddrl + r[R_EAX];
}

/*
 * COP. The two halves push differently: native mode goes through the memory
 * routines, emulation mode writes work RAM directly the way BRK does. The
 * emulation half also omits `xor ebx,ebx` before loading S, which is safe only
 * because the preceding subtraction leaves the top of ebx clear.
 */
void OP(COp02)(u4* const r) /* COP s */
{
    int const emul = (xe & 1) != 0;

    r[R_ESI]++;
    SET8(r[R_EBX], GET8(xpb));
    AX(r, xpc);
    r[R_EAX] = (u4)(uintptr_t)((r[R_EAX] & 0x8000u) ? snesmmap[r[R_EBX]]
                                                    : snesmap2[r[R_EBX]]);
    r[R_EBX] = r[R_ESI] - r[R_EAX];
    xpc = GET16(r[R_EBX]);

    if (emul) {
        u1* const ram = wramdata;
        u2 sp;
        r[R_EAX] = (u4)(uintptr_t)ram;
        SET16(r[R_EBX], GET16(xs));
        sp = GET16(r[R_EBX]);
#define SA1_PUSHRAM(v)                              \
    do {                                            \
        SET8(r[R_ECX], (v));                        \
        ram[sp] = GET8(r[R_ECX]);                   \
        sp = (u2)(((sp - 1) & stackand) | stackor); \
    } while (0)
        SA1_PUSHRAM((u1)(xpc >> 8));
        SA1_PUSHRAM((u1)xpc);
#undef SA1_PUSHRAM
        r[R_EDX] = makedl(r[R_EDX]);
        ram[sp] = (u1)r[R_EDX];
        sp = (u2)(((sp - 1) & stackand) | stackor);
        SET16(r[R_EBX], sp);
        SET16(xs, sp);
    } else {
        SET16(r[R_ECX], GET16(xs));
        AL(r, GET8(xpb));
        bank0_call(r, c_membank0w8);
        SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) - 1)); /* no stackor fixup */
        push8(r, (u1)(xpc >> 8));
        push8(r, (u1)xpc);
        r[R_EDX] = makedl(r[R_EDX]);
        push8(r, (u1)r[R_EDX]);
        SET16(xs, GET16(r[R_ECX]));
    }

    r[R_EBX] &= 0xFFFF00FFu;
    SET8(r[R_EBX], emul ? GET8(xpb) : GET8(xirqb));
    if (!emul)
        SET8(xpb, GET8(r[R_EBX]));
    r[R_EAX] = 0;
    AX(r, emul ? copv8 : copv);
    xpc = GET16(r[R_EAX]);
    SET8(r[R_EDX], (u1)((r[R_EDX] & 0xF3u) | 0x04u));
    jump_to(r, 0);
}

/* The SA-1's RTI is much shorter: no emulation path, no NMI bookkeeping and no
   WAI re-arm, but it does keep the $4300 routing. */
void OP(COp40)(u4* const r) /* RTI s */
{
    intrset = 2;
    if (curexecstate == 0)
        r[R_EDX] &= 0xFFFF00FFu;
    curnmi = 0;

    SET16(r[R_ECX], GET16(xs));
    SET8(r[R_EDX], pop8(r));
    restoredl(r[R_EDX]);
    xpc = (u2)((xpc & 0xFF00u) | pop8(r));
    xpc = (u2)((xpc & 0x00FFu) | (u2)pop8(r) << 8);
    SET8(xpb, pop8(r));
    SET16(xs, GET16(r[R_ECX]));

    r[R_EBX] &= 0xFFFF00FFu;
    r[R_EAX] = 0;
    SET8(r[R_EBX], (u1)r[R_EDX]);
    r[R_EDI] = (u4)(uintptr_t)tablead[r[R_EBX]];
    SET8(r[R_EBX], GET8(xpb));
    AX(r, xpc);
    jump_to(r, 1);
    if (r[R_EDX] & 0x10u) {
        xx &= 0xFFFF00FFu;
        xy &= 0xFFFF00FFu;
    }
}

#endif /* OPS65816_SA1_H */
