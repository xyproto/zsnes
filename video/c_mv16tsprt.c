/*
 * video/c_mv16tsprt.c - the drawsprites16t family of video/makev16t.asm.
 *
 * The colour-maths sprite writer. Where drawsprites16bt (video/c_mv16tspr.c)
 * *produces* the transparency buffer, this one *consumes* it: a sprite in one
 * of the top four palettes is blended with whatever transpbuf already holds,
 * and only the video line is written. Four routines behind one entry point -
 * plain and window-masked, each with a full-add twin - plus a priority family
 * that is still assembly and keeps its own dispatch at the top of the thunk.
 *
 *   plain       palette 0 to 11: the colour goes down as it is
 *   half add    average with what is underneath, unless that is transparent
 *   full add    both sides clipped, then through fulladdtab
 *   full sub    the same with the palette pre-inverted and the result flipped
 */
#include <stdint.h>
#include <string.h>

#include "../types.h"
#include "makevid.h"

u4 SPTAX;
u4 SPTBX;
u4 SPTCX;
u4 SPTDX;
u4 SPTSI;
u4 SPTDI;
u4 SPTBP;

extern u1 cwinenabm, winonsp, scaddtype;
extern u2 scrnon;
extern u1 winspdata[288];
extern u1 transpbuf[];
extern u4 vesa2_clbit;
extern u4 pal16b[256], pal16bcl[256], pal16bxcl[256];
extern u2 fulladdtab[65537]; /* the dword load below reads one past */
extern u1* curvidoffset;

typedef struct {
    u4 ax, bx, cx, dx, si, di, bp;
} regs;

/* transpbuf as the assembly indexes it: `transpbuf+32` is word 16, the same
   bias winspdata carries. */
#define TRANSP ((u2*)(transpbuf + 32))

enum {
    W_PLAIN,
    W_HALF,
    W_ADD,
    W_SUB
};

/* One pixel of drawspr16t{a,b,c,d} and their winon twins. `dst` already
   carries the sprite's x, so every form writes at dst - n.

   Which register each form uses as scratch differs, but the arm that calls
   them zeroes ebx and edx straight afterwards, so eax is the only one that
   survives - the windowed writers clear it, the others leave the palette
   index in it. The arithmetic is 32-bit because the sum of two 16-bit
   colours needs 17 bits before the shift, not because a register demands
   it. */
static void spr_t(regs* const r, u1 const* const src, u1 const ch,
    u4 const ebx, u1* const dst, u1 const* const ebp, u4 const k,
    u4 const pos, int const win, int const mode)
{
    u4 const eax = src[k];
    u4 const n = 16 - pos * 2;

    r->ax = eax;
    if (eax == 0) {
        return;
    }
    if (win && winspdata[ebx - 8 + pos + 16] != 0) {
        return;
    }
    r->ax = (u1)(eax + ch);
    if (mode == W_PLAIN) {
        r->dx = pal16b[r->ax];
        *(u2*)(dst - n) = (u2)r->dx;
        return;
    }
    if (mode == W_HALF) {
        u4 t = *(u4 const*)(ebp - n);
        u4 col = pal16b[r->ax];

        /* A transparent pixel underneath means there is nothing to average
           with, so the sprite colour goes down unchanged. */
        if ((u2)t != 0) {
            col &= vesa2_clbit;
            t &= vesa2_clbit;
            col += t;
            col >>= 1;
        }
        *(u2*)(dst - n) = (u2)col;
        if (win) {
            r->ax = 0;
        }
        return;
    }
    {
        /* pal16bcl and pal16bxcl are already clipped, so only the value from
           the transparency buffer needs masking; the pair then goes through
           fulladdtab, read as a dword from a table of words. */
        u4 const* const pal = mode == W_ADD ? pal16bcl : pal16bxcl;
        u4 t = *(u4 const*)(ebp - n);
        u4 col = pal[r->ax];

        t &= vesa2_clbit;
        col += t;
        col >>= 1;
        memcpy(&col, fulladdtab + col, 4);
        if (mode == W_SUB) {
            col ^= 0xFFFFu;
        }
        *(u2*)(dst - n) = (u2)col;
        if (win) {
            r->ax = 0;
        }
    }
}

static int sub_mode(void)
{
    return (scaddtype & 0x80u) ? W_SUB : W_ADD;
}

/* drawsprites16t, drawspritesfulladd and their two winon twins: one body, with
   `win` choosing the window mask and `fulladd` choosing what a palette-12-and-
   up sprite does. The assembly's transparent arm biases edi and puts it back;
   here the bias lives in `dst` and edi never moves. */
static void sprites_t(regs* const r, int const win, int const fulladd)
{
    SpriteInfo* esi = currentobjptr;
    u1* const cvo = curvidoffset;
    int const tmode = !fulladd ? W_HALF : sub_mode();
    u1 cl = (u1)r->cx;
    u1 ch = (u1)(r->cx >> 8);
    u4 ebx = 0;

    r->ax = 0;
    r->di = (u4)(uintptr_t)cvo;
    do {
        int const flip = (esi->status & 0x20u) != 0;
        u1 const* const src = esi->obj;
        int const clear = fulladd;
        u1* dst;

        ch = esi->pal;
        /* The plain forms double x with a 16-bit shift, so a sprite past
           0x8000 wraps; the windowed ones scale by 2 in the addressing mode
           instead and do not. */
        ebx = win ? esi->x : (u2)(esi->x * 2u);
        dst = cvo + (win ? ebx * 2u : ebx);
        if (ch >= 12 * 16) {
            /* transpbuf is biased by ebx, which is the *doubled* x only in
               the non-windowed forms - the windowed ones add x where they add
               2x to edi. Reproduced; it is what the assembly does. */
            u1 const* const ebp = (u1*)TRANSP + ebx;

            r->dx = 0;
            r->bp = (u4)(uintptr_t)ebp;
            for (u4 pos = 0; pos < 8; pos++) {
                spr_t(r, src, ch, ebx, dst, ebp, flip ? 7 - pos : pos, pos,
                    win, tmode);
            }
            r->dx = 0;
            ebx = 0;
        } else {
            for (u4 pos = 0; pos < 8; pos++) {
                spr_t(r, src, ch, ebx, dst, 0, flip ? 7 - pos : pos, pos, win,
                    W_PLAIN);
            }
            if (clear) {
                r->dx = 0;
                ebx = 0;
            }
        }
        esi++;
    } while (--cl != 0);
    currentobjptr = esi;
    r->bx = ebx;
    r->cx = (r->cx & 0xFFFF0000u) | (u4)ch << 8;
    r->si = (u4)(uintptr_t)esi;
}

/* Entered past the sprprifix test, which stays in the assembly because the
   priority forms of this family have not been ported. */
void c_drawsprites16t(void)
{
    regs r = { SPTAX, SPTBX, SPTCX, SPTDX, SPTSI, SPTDI, SPTBP };
    int const win = (cwinenabm & 0x10u) && winonsp != 0;
    /* Half add wants colour maths on, something on the sub screen and not the
       subtractive mode; every other combination is the full add/sub path. */
    int const half = (scaddtype & 0x40u) && (scrnon >> 8) != 0
        && !(scaddtype & 0x80u);

    sprites_t(&r, win, !half);
    SPTAX = r.ax;
    SPTBX = r.bx;
    SPTCX = r.cx;
    SPTDX = r.dx;
    SPTSI = r.si;
    SPTDI = r.di;
    SPTBP = r.bp;
}
