#ifndef C_MV16DRAW_H
#define C_MV16DRAW_H

#include "c_m716gate.h"

/* The 8x8 tile drawers, from video/mv16tms.asm and video/makev16t.asm. Each
   returns non-zero when the mosaic tail is due and the caller runs
   domosaic16b, because the assembly jumped to it rather than calling.

   draw16x816t and draw8x816toffset live alone in video/c_mv16leaf.c: the
   difftests stub exactly those two to see which way a dispatch went, and a
   call within one translation unit cannot be intercepted. */
u4 draw16x816t(m7regs* r);
u4 draw8x816toffset(m7regs* r);

u4 draw8x816t(m7regs* r);
u4 draw8x816bt(m7regs* r);
u4 draw8x816tms(m7regs* r); /* video/c_mv16msgate.c */
u4 draw8x816twinonms(m7regs* r);

u4 draw16x1616tms(m7regs* r); /* video/c_mv16msgate.c */

extern u1 curmosaicsz;

/* dh = curmosaicsz, which the *ms tails set before jumping. The leaves do not. */
static inline void set_dh_mosaic(m7regs* const r)
{
    r->dx = (r->dx & ~(zreg)0xFF00u) | ((zreg)curmosaicsz << 8);
}

#define SEAM_IN(P, r) \
    P##AX = (r)->ax;  \
    P##BX = (r)->bx;  \
    P##CX = (r)->cx;  \
    P##DX = (r)->dx;  \
    P##SI = (r)->si;  \
    P##DI = (r)->di;  \
    P##BP = (r)->bp

#define SEAM_OUT(P, r) \
    (r)->ax = P##AX;   \
    (r)->bx = P##BX;   \
    (r)->cx = P##CX;   \
    (r)->dx = P##DX;   \
    (r)->si = P##SI;   \
    (r)->di = P##DI;   \
    (r)->bp = P##BP

/* A drawer with a seam and a tail flag of its own. */
#define TILE_LEAF(name, P, body) \
    u4 name(m7regs* const r)     \
    {                            \
        SEAM_IN(P, r);           \
        body();                  \
        SEAM_OUT(P, r);          \
        return P##Tail != 0;     \
    }

#endif
