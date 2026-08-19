#ifndef C_DISPATCH_H
#define C_DISPATCH_H

#include "../endmem.h"
#include "../types.h"

/* The pushad register block the 65816 core's halves share.
   eop is an unprototyped function type, so a table of them holds the ported
   opcode bodies (void f(u4*)) and calling one with the block is well formed. */
enum { R_EDI,
    R_ESI,
    R_EBP,
    R_ESP,
    R_EBX,
    R_EDX,
    R_ECX,
    R_EAX };

/* dl is the processor status byte, dh the scanline cycle counter. */
#define DH(r) ((u1)((r)[R_EDX] >> 8))

static inline void set_dh(u4* const r, u1 const v)
{
    r[R_EDX] = (r[R_EDX] & 0xFFFF00FFu) | (u4)v << 8;
}

static inline void add_dh(u4* const r, u1 const n)
{
    set_dh(r, (u1)(DH(r) + n));
}

/* bl is the opcode index; the dispatcher only ever loads the low byte, which
   is what keeps the upper three of ebx zero and the index inside the table. */
static inline void set_bl(u4* const r, u1 const v)
{
    r[R_EBX] = (r[R_EBX] & 0xFFFFFF00u) | v;
}

/* One SPC700 opcode. The handler takes the program counter just past the
   opcode byte and returns the updated one; the assembly kept it in ebp, and
   left the result in eax as well. Every dispatch site cleared ebx after. */
static inline void spc_step(u4* const r, u4 const op)
{
    r[R_EBP] = r[R_EAX] = (u4)opcjmptab[op]((u1*)r[R_EBP]);
    r[R_EBX] = 0;
}

#endif
