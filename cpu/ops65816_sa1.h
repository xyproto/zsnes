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
#define OPS_OWN_COpD6m8

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

void OP(COp40)(u4* const r) /* RTI s */
{
    intrset = 2;
    rti_body(r);
}

void OP(COp58)(u4* const r) /* CLI i - nothing to switch to */
{
    r[R_EDX] &= ~0x04u;
}

RMW(OP(COpD6m8), a_dCx_16ni, o_DECm8, a_dCx_16w) /* DEC d,x - see above */

#endif /* OPS65816_SA1_H */
