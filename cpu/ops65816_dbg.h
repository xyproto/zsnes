/*
 * The debug 65816 core, from cpu/e65816c.inc: the third instantiation of
 * cpu/ops65816.h, sharing the main core's register file and differing only in
 * the opcode table, since the debugger single-steps through its own dispatch.
 *
 * What actually differs: BRK's flag byte, BRL recomputing the bank base rather
 * than trusting initaddrl, RTI checking for a WAI in one more branch, CLI
 * having no IRQ to switch to, and the thirteen push/pull opcodes that start by
 * loading wramdata into eax.
 */
#ifndef OPS65816_DBG_H
#define OPS65816_DBG_H

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
#define OPS_OWN_COp00
#define OPS_OWN_COp82
#define OPS_OWN_COp40
#define OPS_OWN_COp58

#include "ops65816.h"

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

void OP(COp08)(zreg* const r) /* PHP s */
{
    r[R_EAX] = (zreg)(uintptr_t)wramdata;
    r[R_EDX] = makedl(r[R_EDX]);
    SET16(r[R_ECX], GET16(xs));
    push8(r, (u1)r[R_EDX]);
    SET16(xs, GET16(r[R_ECX]));
}

void OP(COp2B)(zreg* const r) /* PLD s */
{
    u1 hi;
    r[R_EAX] = (zreg)(uintptr_t)wramdata;
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

void OP(COp00)(zreg* const r) /* BRK s - one flag bit apart from the 65816's */
{
    brk_cop(r, brkv, brkv8, 0x04u);
}

void OP(COp58)(zreg* const r) /* CLI i - nothing to switch to */
{
    r[R_EDX] &= ~0x04u;
}

/* BRL works the displacement out from the bank base it recomputes here, rather
   than from initaddrl as the 65816's does. */
void OP(COp82)(zreg* const r) /* BRL rl */
{
    AX(r, xpc);
    SET8(r[R_EBX], GET8(xpb));
    r[R_EAX] = (zreg)(uintptr_t)((r[R_EAX] & 0x8000u) ? snesmmap[r[R_EBX]]
                                                    : snesmap2[r[R_EBX]]);
    r[R_EBX] = r[R_ESI] - r[R_EAX];
    SET16(r[R_EBX], (u2)(GET16(r[R_EBX]) + 2));
    r[R_EAX] = 0;
    SET16(r[R_EBX], (u2)(GET16(r[R_EBX]) + *(u2 const*)r[R_ESI]));
    AX(r, GET16(r[R_EBX]));
    r[R_EBX] = 0;
    xpc = GET16(r[R_EAX]);
    SET8(r[R_EBX], GET8(xpb));
    jump_to(r, 0);
}

/* RTI, with the WAI re-arm in the high branch as well as the low one. */
void OP(COp40)(zreg* const r) /* RTI s */
{
    int const emul = (xe & 1) != 0;

    if (nmistatus == 3) {
        if (curexecstate & 0x01u)
            curexecstate &= 0xFEu;
        if (curexecstate == 0)
            r[R_EDX] &= 0xFFFF00FFu;
    }
    curnmi = 0;

    SET16(r[R_ECX], GET16(xs));
    SET8(r[R_EDX], pop8(r));
    SET16(xs, GET16(r[R_ECX]));
    if (emul)
        r[R_EDX] |= 0x30u;
    restoredl(r[R_EDX]);

    SET16(r[R_ECX], GET16(xs));
    r[R_EAX] = 0;
    xpc = (u2)((xpc & 0xFF00u) | pop8(r));
    r[R_EAX] = 0;
    xpc = (u2)((xpc & 0x00FFu) | (u2)pop8(r) << 8);
    if (!emul) {
        r[R_EAX] = 0;
        SET8(xpb, pop8(r));
    }
    SET16(xs, GET16(r[R_ECX]));

    r[R_EBX] &= 0xFFFF00FFu;
    r[R_EAX] = 0;
    AX(r, xpc);
    SET8(r[R_EBX], (u1)r[R_EDX]);
    r[R_EDI] = (zreg)(uintptr_t)tablead[r[R_EBX]];
    SET8(r[R_EBX], emul ? 0 : GET8(xpb));
    xpc = GET16(r[R_EAX]);

    if (emul) {
        jump_to(r, 0);
        return;
    }
    {
        u1* const base = bank_base(r[R_EAX], r[R_EBX], 1);
        int const low = (r[R_EAX] & 0x8000u) == 0;
        int const dma = low && r[R_EAX] >= 0x4300u;
        if (dma && memtabler8[r[R_EBX]] != regaccessbankr8)
            doirqnext = 0;
        initaddrl = base;
        r[R_ESI] = (zreg)(uintptr_t)base + r[R_EAX];
        /* The WAI re-arm happens in the high branch and below $4300, but in
           neither of the two branches above it. */
        if (!dma && *(u1 const*)r[R_ESI] == 0xCBu)
            intrset = 2;
    }
    if (r[R_EDX] & 0x10u) {
        xx &= 0xFFFF00FFu;
        xy &= 0xFFFF00FFu;
    }
}

#endif /* OPS65816_DBG_H */
