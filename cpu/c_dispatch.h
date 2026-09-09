#ifndef C_DISPATCH_H
#define C_DISPATCH_H

#include "../endmem.h"
#include "../types.h"

/* pushad register order. */
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

static inline void set_dh(zreg* const r, u1 const v)
{
    r[R_EDX] = (r[R_EDX] & 0xFFFF00FFu) | (u4)v << 8;
}

static inline void add_dh(zreg* const r, u1 const n)
{
    set_dh(r, (u1)(DH(r) + n));
}

/* bl is the opcode index; the dispatcher only ever loads the low byte, which
   is what keeps the upper three of ebx zero and the index inside the table. */
static inline void set_bl(zreg* const r, u1 const v)
{
    r[R_EBX] = (r[R_EBX] & 0xFFFFFF00u) | v;
}

/* Run one SPC700 opcode. */
static inline void spc_step(zreg* const r, u1 const op)
{
    r[R_EBP] = r[R_EAX] = (zreg)opcjmptab[op]((u1*)r[R_EBP]);
    r[R_EBX] = 0;
}

#endif
