/*
 * The mosaic dispatchers and their leaves, ported from video/mv16tms.asm.
 *
 * A leaf spills the whole register file into the shared MVS seam, calls its C
 * body and reloads; a dispatcher picks one on the colour-maths registers and
 * jumps to it, so the leaf returns to the dispatcher's caller.
 *
 * The mosaic tail is handed back rather than taken - see portasm.md - because
 * test/difftest_mvall.c records the register state at the jump into
 * domosaic16b, which a call from in here could not reproduce.
 *
 * The window path is taken when curmosaicsz *is* 1 and winon is set. Reading
 * that the other way round caught a first attempt.
 */

#include "c_mv16draw.h"

extern u1 bgmode, scaddtype, winon;
extern u2 scrnon;
extern u1 transpbuf[];

extern u4 MVSAX, MVSBX, MVSCX, MVSDX, MVSSI, MVSDI, MVSBP, MVSMosaic;
extern u4 MVAX, MVBX, MVCX, MVDX, MVSI;
extern u4 TTAX, TTBX, TTCX, TTDX, TTSI, TTDI, TTBP, TTTail;
extern u4 T8AX, T8BX, T8CX, T8DX, T8SI, T8DI, T8BP, T8Tail;

void c_draw16tms_setup(void);
void c_draw8x816tms_body(void);
void c_draw8x816twinonms_body(void);
void c_draw8x8fulladdms(void);
void c_draw8x816tsms(void);
void c_draw8x8fulladdwinonms(void);
void c_draw8x816tswinonms(void);
void c_draw16x16tms_setup(void);
void c_draw16x1616tms_body(void);
void c_draw16x1616twinonms(void);
void c_draw16x16fulladdms(void);
void c_draw16x1616tsms(void);
void c_draw16x16fulladdwinonms(void);
void c_draw16x1616tswinonms(void);
void c_draw8x816t(void);
void c_draw8x816bt(void);
/* Seven arguments, in the order the ccallv pushed them. */
void draw8x816boffset(u4 a, u4 c, u4 d, u4 b, u4 bp, u4 si, u4 di);

/* A leaf on the shared MVS seam with no tail of its own. */
#define MVS_LEAF(name, body)        \
    static u4 name(m7regs* const r) \
    {                               \
        SEAM_IN(MVS, r);            \
        body();                     \
        SEAM_OUT(MVS, r);           \
        return 0;                   \
    }

/* The same, but reading MVSMosaic as a *byte* and jumping without setting dh. */
#define MVS_LEAF_MOSAIC(name, body) \
    static u4 name(m7regs* const r) \
    {                               \
        SEAM_IN(MVS, r);            \
        body();                     \
        SEAM_OUT(MVS, r);           \
        return (u1)MVSMosaic != 0;  \
    }

/* A dispatcher's own body: MVSMosaic as a dword, and dh set before the jump. */
#define MVS_BODY(name, body)        \
    static u4 name(m7regs* const r) \
    {                               \
        SEAM_IN(MVS, r);            \
        body();                     \
        SEAM_OUT(MVS, r);           \
        if (MVSMosaic == 0)         \
            return 0;               \
        set_dh_mosaic(r);           \
        return 1;                   \
    }

MVS_LEAF_MOSAIC(leaf_8x8fulladdms, c_draw8x8fulladdms)
MVS_LEAF(leaf_8x816tsms, c_draw8x816tsms)
MVS_LEAF(leaf_8x8fulladdwinonms, c_draw8x8fulladdwinonms)
MVS_LEAF(leaf_8x816tswinonms, c_draw8x816tswinonms)
MVS_BODY(body_8x816tms, c_draw8x816tms_body)
MVS_BODY(body_8x816twinonms, c_draw8x816twinonms_body)

/* Which of the three 8x8 forms the colour-maths registers select. */
static u4 pick_8x8(m7regs* const r, int const windowed)
{
    /* ebp = transpbuf + 32 - 2*eax, the cursor the drawers work from. */
    r->bp = (zreg)(uintptr_t)(transpbuf + 32) - r->ax - r->ax;

    if (scaddtype & 0x80u)
        return windowed ? leaf_8x816tswinonms(r) : leaf_8x816tsms(r);
    if (!(scaddtype & 0x40u) || (scrnon >> 8) == 0)
        return windowed ? leaf_8x8fulladdwinonms(r) : leaf_8x8fulladdms(r);
    return windowed ? body_8x816twinonms(r) : body_8x816tms(r);
}

u4 draw8x816twinonms(m7regs* const r)
{
    return pick_8x8(r, 1);
}

u4 draw8x816tms(m7regs* const r)
{
    if (bgmode == 5)
        return draw16x816t(r);

    /* The shared setup leaves edi and ebp alone, so its seam carries five. */
    MVAX = r->ax;
    MVBX = r->bx;
    MVCX = r->cx;
    MVDX = r->dx;
    MVSI = r->si;
    c_draw16tms_setup();
    r->ax = MVAX;
    r->bx = MVBX;
    r->cx = MVCX;
    r->dx = MVDX;
    r->si = MVSI;

    if (curmosaicsz == 1 && winon != 0)
        return draw8x816twinonms(r);
    return pick_8x8(r, 0);
}

/* --- the 16x16 half ------------------------------------------------------ *
 *
 * Same three-way pick, but every tail here tests MVSMosaic as a byte and jumps
 * without setting dh, and the windowed branch has no mosaic tail at all.
 */
MVS_LEAF_MOSAIC(leaf_16x16fulladdms, c_draw16x16fulladdms)
MVS_LEAF_MOSAIC(leaf_16x1616tsms, c_draw16x1616tsms)
MVS_LEAF_MOSAIC(body_16x1616tms, c_draw16x1616tms_body)
MVS_LEAF(leaf_16x16fulladdwinonms, c_draw16x16fulladdwinonms)
MVS_LEAF(leaf_16x1616tswinonms, c_draw16x1616tswinonms)
MVS_LEAF(body_16x1616twinonms, c_draw16x1616twinonms)

static u4 pick_16x16(m7regs* const r, int const windowed)
{
    r->bp = (zreg)(uintptr_t)(transpbuf + 32) - r->ax - r->ax;

    if (scaddtype & 0x80u)
        return windowed ? leaf_16x1616tswinonms(r) : leaf_16x1616tsms(r);
    if (!(scaddtype & 0x40u) || (scrnon >> 8) == 0)
        return windowed ? leaf_16x16fulladdwinonms(r) : leaf_16x16fulladdms(r);
    return windowed ? body_16x1616twinonms(r) : body_16x1616tms(r);
}

u4 draw16x1616tms(m7regs* const r)
{
    /* This half's setup carries the same five registers, but hands ebx back as
       curypos instead of the reverse adder. */
    MVAX = r->ax;
    MVBX = r->bx;
    MVCX = r->cx;
    MVDX = r->dx;
    MVSI = r->si;
    c_draw16x16tms_setup();
    r->ax = MVAX;
    r->bx = MVBX;
    r->cx = MVCX;
    r->dx = MVDX;
    r->si = MVSI;

    if (curmosaicsz == 1 && winon != 0)
        return pick_16x16(r, 1);
    return pick_16x16(r, 0);
}
