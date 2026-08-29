/*
 * The drawsprites16bt family: the 4-bit sprite writer that *produces* the
 * transparency buffer. The same unrolled eight-pixel loop as drawsprites16b,
 * but each pixel is written twice, to the video line and to transpbuf, so the
 * later colour-maths passes see it. Only drawsprites16bt is reached from
 * outside; video/c_mv16tsprt.c consumes the buffer.
 *
 *   plain     every pixel whose low nibble is non-zero
 *   winon     ... unless the sprite window covers it
 *   prio      one priority level per pass, sprpriodata masking the pixels a
 *             higher-priority sprite already claimed
 *
 * ebx means three different things here: the plain form keeps the doubled x as
 * a byte offset, the window form halves it back to a pixel index, and the
 * priority form never doubles it.
 */
#include <stdint.h>
#include <string.h>

#include "../types.h"
#include "makevid.h"

zreg SPBAX;
zreg SPBBX;
zreg SPBCX;
zreg SPBDX;
zreg SPBSI;
zreg SPBDI;
zreg SPBBP;

extern u1 sprprifix, cwinenabm, winonsp, csprbit, csprprlft;
extern u1 sprclprio[4], winspdata[288], sprpriodata[288];
extern u1 transpbuf[];
extern u4 sprsingle;
extern u4 pal16b[256];
extern u1* curvidoffset;

typedef struct {
    zreg ax, bx, cx, dx, si, di, bp;
} regs;

/* The transparency buffer as the assembly indexes it: `transpbuf+32` is word
   16, the same bias winspdata and sprpriodata carry. */
#define TRANSP ((u2*)(transpbuf + 32))

/* drawspr16bt / drawspr16btwo. `pos` is the pixel's place on screen, which for
   a flipped sprite runs the other way from its place in the tile; `win` also
   consults the sprite window. Only al is ever written, so `add al,ch` wraps in
   a byte and pal16b is indexed by that alone. */
static void spr_plain(regs* const r, u1 const* const src, u1 const ch,
    u4 const ebx, u1* const edi, u1* const ebp, u4 const k, u4 const pos,
    int const win)
{
    u4 const eax = src[k];
    u4 const n = 16 - pos * 2;

    r->ax = eax;
    if ((eax & 0x0Fu) == 0) {
        return;
    }
    if (win && winspdata[ebx - 8 + pos + 16] != 0) {
        return;
    }
    r->ax = (u1)(eax + ch);
    r->dx = pal16b[r->ax];
    *(u2*)(edi - n) = (u2)r->dx;
    *(u2*)(ebp - n) = (u2)r->dx;
}

/* drawsprites16bt's own loop and drawsprites16btwinon: one sprite per pass,
   eight pixels each, ending with the object pointer parked past the last. */
static void draw_plain(regs* const r, int const win)
{
    SpriteInfo* esi = currentobjptr;
    u1 cl = (u1)r->cx;
    u1 ch = (u1)(r->cx >> 8);
    u4 ebx = 0;

    r->ax = 0;
    do {
        u1* edi = curvidoffset;
        int const flip = (esi->status & 0x20u) != 0;
        u1 const* const src = esi->obj;
        u1* ebp;

        ch = esi->pal;
        /* mov bx,[esi] then shl bx,1 is a *16-bit* shift, so a sprite past
           0x8000 wraps rather than reaching the second half of the line. */
        ebx = (u2)(esi->x * 2u);
        edi += ebx;
        ebp = (u1*)TRANSP + ebx;
        if (win) {
            ebx >>= 1;
        }
        for (u4 pos = 0; pos < 8; pos++) {
            spr_plain(r, src, ch, ebx, edi, ebp, flip ? 7 - pos : pos, pos,
                win);
        }
        r->di = (zreg)(uintptr_t)edi;
        r->bp = (zreg)(uintptr_t)ebp;
        esi++;
    } while (--cl != 0);
    currentobjptr = esi;
    r->bx = ebx;
    r->cx = (r->cx & 0xFFFF0000u) | (u4)ch << 8;
    r->si = (zreg)(uintptr_t)esi;
}

/* sprdrawpra16bt: draw unless a sprite at or above this priority bit already
   claimed the pixel, then claim it. sprdrawprb16bt is the same without the
   mask, used where only one sprite can reach the line. */
static void spr_pixel(regs* const r, u4 const eax, u1 const cl, u1 const ch,
    s4 const ebx, u2* const edi, s4 const p1, int const prb)
{
    r->ax = eax;
    if (eax == 0) {
        return;
    }
    if (!prb && (sprpriodata[ebx - p1 + 16] & cl)) {
        return;
    }
    r->ax = (u1)(eax + ch);
    r->dx = pal16b[r->ax];
    edi[ebx - p1] = (u2)r->dx;
    TRANSP[ebx - p1] = (u2)r->dx;
    if (!prb) {
        sprpriodata[ebx - p1 + 16] |= cl;
    }
}

/* sprdrawa16b / sprdrawaf16b. The flipped form walks the tile forwards and the
   screen backwards, which is the same eight writes in the other order. */
static void spr_row(regs* const r, u1 const cl, u1 const ch, u4 const ebx,
    u1 const* const src, u2* const edi, int const flip, int const prb)
{
    r->ax = 0;
    for (u4 k = 0; k < 8; k++) {
        spr_pixel(r, src[k], cl, ch, ebx, edi, flip ? k + 1 : 8 - k, prb);
    }
}

/* sprdrawa / sprdrawaf with sprdrawpra2: a sprite of the wrong priority still
   claims its pixels, so the pass at that priority skips them. It reads two
   pixels per 16-bit load, which is where al and ah come from. */
static void spr_mark(regs* const r, u1 const dl, u4 const ebx,
    u1 const* const src, int const flip)
{
    for (u4 k = 0; k < 8; k++) {
        if (k % 2 == 0) {
            r->ax = src[k] | (u4)src[k + 1] << 8;
        }
        if (src[k] != 0) {
            sprpriodata[ebx - (flip ? k + 1 : 8 - k) + 16] |= dl;
        }
    }
}

/* sprprioritydrawbt's .drawsingle arm: the line holds one sprite's worth of
   pixels, walked backwards from the last entry, with no priority mask. */
static void draw_single(regs* const r)
{
    SpriteInfo const* esi = currentobjptr;
    u2* const edi = (u2*)curvidoffset;
    u1 cl = (u1)r->cx;
    u1 ch = (u1)(r->cx >> 8);
    u4 ebx = 0;

    r->di = (zreg)(uintptr_t)edi;
    /* edx is the walk's starting address, and stays that unless a pixel draws
       over it with a palette entry. */
    r->dx = (zreg)(uintptr_t)(esi + cl - 1);
    esi = (SpriteInfo const*)(uintptr_t)r->dx;
    r->ax = 0;
    do {
        int const flip = (esi->status & 0x20u) != 0;

        ebx = esi->x;
        ch = esi->pal;
        spr_row(r, cl, ch, ebx, esi->obj, edi, flip, 1);
        esi--;
    } while (--cl != 0);
    r->bx = ebx;
    r->cx = (r->cx & 0xFFFF0000u) | (u4)ch << 8;
    r->si = (zreg)(uintptr_t)esi;
}

/* sprprioritydrawbt proper: every sprite is visited, and one at another
   priority only claims its pixels. */
static void draw_prio(regs* const r)
{
    SpriteInfo const* esi = currentobjptr;
    u2* const edi = (u2*)curvidoffset;
    u4 const ebp = r->bp;
    u1 cl = (u1)r->cx;
    u1 ch = (u1)(r->cx >> 8);
    u4 ebx = 0;

    csprprlft = cl;
    r->di = (zreg)(uintptr_t)edi;
    for (;;) {
        int const flip = (esi->status & 0x20u) != 0;

        ebx = esi->x;
        ch = esi->pal;
        /* mov dl,[esi+7] then and edx,3 - the mask leaves nothing of whatever
           edx held, which is why the `mov edx,esi` above the loop is dead. */
        r->dx = esi->status & 0x03u;
        r->ax = 0;
        if (r->dx == ebp) {
            cl = csprbit;
            /* The pixel writer leaves a palette entry in edx behind it. */
            spr_row(r, cl, ch, ebx, esi->obj, edi, flip, 0);
        } else {
            r->dx = csprbit;
            spr_mark(r, csprbit, ebx, esi->obj, flip);
        }
        esi++;
        if (--csprprlft == 0) {
            break;
        }
    }
    r->bx = ebx;
    r->cx = (r->cx & 0xFFFF0000u) | (u4)ch << 8 | cl;
    r->si = (zreg)(uintptr_t)esi;
    csprbit = (u1)(csprbit << 1 | csprbit >> 7);
    if (csprbit == 1) {
        memset(sprpriodata + 16, 0, 256);
        r->ax = 0;
        r->cx = 0;
        r->di = (zreg)(uintptr_t)(sprpriodata + 16 + 256);
    }
}

void c_drawsprites16bt(void)
{
    regs r = { SPBAX, SPBBX, SPBCX, SPBDX, SPBSI, SPBDI, SPBBP };

    if (sprprifix == 1) {
        /* drawsprites16btprio. Its window twin drawsprites16btpriow is the
           same body - the window only decides whether the guard below runs,
           and both instantiate the macro with the same writers. */
        if (sprclprio[r.bp] != 0) {
            if (sprsingle == 1) {
                draw_single(&r);
            } else {
                draw_prio(&r);
            }
        }
    } else if ((cwinenabm & 0x10u) && winonsp != 0) {
        draw_plain(&r, 1);
    } else {
        draw_plain(&r, 0);
    }
    SPBAX = r.ax;
    SPBBX = r.bx;
    SPBCX = r.cx;
    SPBDX = r.dx;
    SPBSI = r.si;
    SPBDI = r.di;
    SPBBP = r.bp;
}
