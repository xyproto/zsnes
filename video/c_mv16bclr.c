/*
 * video/c_mv16bclr.c - the clearback16b* cluster of video/makev16t.asm.
 *
 * Fills the sub screen's back area with the fixed colour before anything is
 * drawn over it. Unlike clearback16t this one has no colour maths to do: the
 * work is deciding what the fixed colour is and which pixels the window lets
 * it reach. Nine routines in the assembly, one entry point - clearback16bts -
 * and everything else reached by jump, so the whole cluster is one C function.
 *
 * Three shapes of fill:
 *
 *   flat      512 bytes of one colour, or of zero when the window covers all
 *   run list  windowdata as (column, depth) pairs, colour inside or outside
 *   dual      the per-pixel mask at cwinptr, four pixels at a time
 *
 * winon selects between them as a small enum, 0 to 5, not as a bitmask.
 */
#include <stdint.h>

#include "../types.h"
#include "../vcache.h"

zreg CLBAX;
zreg CLBBX;
zreg CLBCX;
zreg CLBDX;
zreg CLBSI;
zreg CLBDI;

extern u1 DoTransp, winon, scaddset, bgmode, numwin, vidbright;
extern u1 coladdr, coladdg, coladdb;
extern u1 windowdata[];
extern u2 scrnon, prevrgbpal;
extern u4 prevrgbcol;
extern u1 *curvidoffset, *cwinptr;

/* ebp is the one register the cluster never touches, so the seam leaves it
   alone. */
typedef struct {
    u4 ax, bx, cx, dx, si, di;
} regs;

/* ax = al * vidbright, then al = ax / 15 - an 8-bit divide, so the assembly
   traps unless the quotient fits a byte. It does for the values the PPU can
   produce: coladd* is 5 bits and vidbright is 4. The shift count is a byte of
   the position dword and the hardware masks it to five bits. */
static u2 chan(u1 const v, u4 const pos)
{
    u1 const q = (u1)((u2)(v * vidbright) / 15u);
    u4 const n = (u1)pos & 31u;

    return n < 16 ? (u2)(q << n) : 0;
}

/* The key the colour is cached against: the dword at coladdr, which spans
   coladdg, coladdb and colnull, shifted up a byte so colnull falls off the
   top, with vidbright in the low byte. */
static u4 rgbkey(void)
{
    return (u4)coladdr << 8 | (u4)coladdg << 16 | (u4)coladdb << 24 | vidbright;
}

/* Leaves the fixed colour in bx. Returns non-zero if the cache answered, which
   the caller needs because the recompute clears the top half of eax while a
   cache hit leaves the key sitting there. The scratch cl the recompute also
   leaves behind is dead on every path out of the cluster, so it is not kept. */
static int backdrop(regs* const r, u4 const key)
{
    u2 bx;

    r->bx = (r->bx & ~0xFFFFu) | prevrgbpal;
    if (key == prevrgbcol) {
        return 1;
    }
    prevrgbcol = key;
    bx = chan(coladdr, vesa2_rpos);
    bx += chan(coladdg, vesa2_gpos);
    bx += chan(coladdb, vesa2_bpos);
    prevrgbpal = bx;
    r->bx = (r->bx & ~0xFFFFu) | bx;
    return 0;
}

/* clearback16bts0.clearing. Every path reaches it with eax already zero, so
   the fill is a blank line; all three also set edi to curvidoffset first. */
static void clearing(regs* const r)
{
    u4* edi = (u4*)curvidoffset;

    if (!((scrnon >> 8) & 0x10u)) {
        DoTransp = 1;
    }
    for (u4 n = 128; n != 0; n--) {
        *edi++ = 0;
    }
    r->ax = 0;
    r->cx = 0;
    r->di = (zreg)(uintptr_t)edi;
}

/* dowindowback16b and dowindowback16brev: windowdata is a list of (column,
   depth change) byte pairs, and the two forms differ only in which side of the
   window keeps the colour. numwin is counted down to zero on the way through.
   edi is never advanced - every run indexes off the start of the line. */
static void dowindow(regs* const r, int const rev)
{
    u1 const* ebx = windowdata;
    u1* const edi = curvidoffset;
    u2 const ax = (u2)r->ax;
    u4 edx = 0;
    u1 ch = 0;
    u1 cl = 0; /* the caller's cl is dead: the first entry overwrites it */
    int done = 0;

    while (!done) {
        int run;

        cl = *ebx;
        run = (u1)edx != cl;
        for (;;) {
            if (run) {
                int const draw = rev ? ch != 0 : ch == 0;
                u2 const v = draw ? ax : 0;

                /* A run that is exactly zero long spans the whole line: dec
                   wraps cl to 255 and the loop goes round 256 times. */
                cl -= (u1)edx;
                if (draw) {
                    DoTransp = 0;
                }
                do {
                    *(u2*)(edi + edx * 2) = v;
                    edx = (u1)(edx + 1);
                } while (--cl != 0);
            }
            ch += ebx[1];
            ebx += 2;
            if (numwin == 0) {
                done = 1;
                break;
            }
            if (--numwin != 0) {
                break;
            }
            /* Out of entries: one more run, to the end of the line. */
            cl = 0;
            run = 1;
        }
    }
    r->ax = 0;
    r->bx = (zreg)(uintptr_t)ebx;
    r->cx = (r->cx & 0xFFFF0000u) | (u4)ch << 8 | cl;
    r->dx = edx;
    r->di = (zreg)(uintptr_t)edi;
}

static void dowindowback(regs* const r)
{
    if (!((scrnon >> 8) & 0x10u)) {
        DoTransp = 1;
    }
    dowindow(r, (scaddset & 0x30u) == 0x20u);
}

enum {
    P_NONE,
    P_ALL,
    P_PART
};

/* clearback16bdual and its rev/b2/rev2 variants. The mask at cwinptr is read
   four bytes at a time and a group is either drawn whole, skipped whole, or
   resolved pixel by pixel. Which comparison comes first depends on what the
   previous group was - the assembly has three loop heads rather than one -
   and the choice shows up in ebx and in DoTransp even where the pixels come
   out the same, so the state is carried rather than folded away. */
static void dual(regs* const r, int const rev, int const b2)
{
    u4 const eax = r->ax;
    u4 const vall = rev ? 0x01010101u : 0u;
    u4 const vnone = rev ? 0u : 0x01010101u;
    u4 ebx = r->bx;
    u1* edi = curvidoffset;
    u1 const* esi = cwinptr;
    int state = P_PART; /* the entry tests are the per-pixel head's tests */

    if (b2 && *(u4 const*)esi != vnone) {
        DoTransp = 0;
    }
    for (u4 ecx = 64; ecx != 0; ecx--) {
        u4 const w = *(u4 const*)esi;
        int kind;

        switch (state) {
        case P_ALL:
            kind = w != vall ? P_PART : P_ALL;
            break;
        case P_NONE:
            kind = w != vnone ? P_PART : P_NONE;
            break;
        default:
            if (w == vnone) {
                kind = P_NONE;
            } else if (w == vall) {
                kind = P_ALL;
            } else {
                kind = P_PART;
            }
            break;
        }
        switch (kind) {
        case P_ALL:
            *(u4*)edi = eax;
            *(u4*)(edi + 4) = eax;
            break;
        case P_NONE:
            *(u4*)edi = 0;
            *(u4*)(edi + 4) = 0;
            break;
        default:
            if (b2) {
                DoTransp = 0;
            }
            for (u4 k = 0; k < 4; k++) {
                ebx = (esi[k] == 1) == (rev != 0) ? eax : 0;
                *(u2*)(edi + k * 2) = (u2)ebx;
            }
            break;
        }
        edi += 8;
        esi += 4;
        state = kind;
    }
    r->bx = ebx;
    r->cx = 0;
    r->si = (zreg)(uintptr_t)esi;
    r->di = (zreg)(uintptr_t)edi;
}

static void dualback(regs* const r)
{
    int const b2 = bgmode == 7 && !((scrnon >> 8) & 0x10u);

    if (b2) {
        DoTransp = 1;
    }
    r->bx = (r->bx & ~0xFFu) | (scaddset & 0x30u);
    dual(r, (scaddset & 0x30u) == 0x10u, b2);
}

/* clearback16bts0b: the colour in both halves of eax, then a flat fill - or
   the per-pixel mask when two windows are in play. */
static void ts0b(regs* const r)
{
    u4* edi;

    backdrop(r, rgbkey());
    r->ax = (u4)(u2)r->bx * 0x00010001u;
    if (winon == 3) {
        dualback(r);
        return;
    }
    if (r->ax == 0) {
        clearing(r);
        return;
    }
    edi = (u4*)curvidoffset;
    for (u4 n = 128; n != 0; n--) {
        *edi++ = r->ax;
    }
    r->ax = 0;
    r->cx = 0;
    r->di = (zreg)(uintptr_t)edi;
}

static void ts0(regs* const r)
{
    r->bx = (r->bx & ~0xFFu) | (scaddset & 0x30u);
    if ((scaddset & 0x30u) == 0x20u) {
        ts0b(r);
        return;
    }
    clearing(r);
}

void c_clearback16bts(void)
{
    regs r = { CLBAX, CLBBX, CLBCX, CLBDX, CLBSI, CLBDI };

    DoTransp = 0;
    if (vesa2_rpos == 0) {
        clearing(&r);
    } else if (winon == 0) {
        ts0b(&r);
    } else if (winon == 2) {
        ts0(&r);
    } else if (winon == 4) {
        clearing(&r);
    } else {
        r.bx = (r.bx & ~0xFFu) | (scaddset & 0x30u);
        if ((scaddset & 0x30u) == 0x20u && winon == 5) {
            clearing(&r);
        } else if (winon == 5 || winon == 3) {
            ts0b(&r);
        } else {
            u4 const key = rgbkey();
            int const hit = backdrop(&r, key);

            /* mov ax,bx then cmp eax,0 tests the whole dword. On a cache hit
               the top half is still coladdg and coladdb, so a black backdrop
               goes to the window walker anyway; only a freshly computed
               colour can be seen as zero here. */
            r.ax = (hit ? key & 0xFFFF0000u : 0u) | (u2)r.bx;
            if (r.ax == 0) {
                clearing(&r);
            } else {
                dowindowback(&r);
            }
        }
    }
    CLBAX = r.ax;
    CLBBX = r.bx;
    CLBCX = r.cx;
    CLBDX = r.dx;
    CLBSI = r.si;
    CLBDI = r.di;
}
