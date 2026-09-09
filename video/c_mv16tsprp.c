/*
 * The drawsprites16t priority family of video/makev16t.asm: six entry points -
 * half add, full add, full sub, each with a window-masked twin - from one
 * macro with different writers. Only drawsprites16tprio is reached from
 * outside.
 *
 * Like drawsprites16t (video/c_mv16tsprt.c) it consumes transpbuf and writes
 * only the video line, but every sprite on the line is visited on every pass:
 * one of another priority still claims its pixels in sprpriodata, so the pass
 * that owns them leaves those alone. After the last priority the claim mask
 * rotates back to bit 0 and is wiped.
 *
 * ebp carries the priority being drawn and is the one register left alone.
 */
#include <stdint.h>
#include <string.h>

#include "../types.h"
#include "makevid.h"

zreg SPPAX;
zreg SPPBX;
zreg SPPCX;
zreg SPPDX;
zreg SPPSI;
zreg SPPDI;
zreg SPPBP;

extern u1 cwinenabm, winonsp, scaddtype, csprbit, csprprlft;
extern u2 scrnon;
extern u1 sprclprio[4], winspdata[288], sprpriodata[288];
extern u1 transpbuf[];
extern u4 sprsingle, vesa2_clbit;
extern u4 pal16b[256], pal16bcl[256], pal16bxcl[256];
extern u2 fulladdtab[65537]; /* the dword load below reads the last entry */
extern u1* curvidoffset;

typedef struct {
    zreg ax, bx, cx, dx, si, di, bp;
} regs;

/* transpbuf as the assembly indexes it: `transpbuf+32` is word 16, the same
   bias winspdata and sprpriodata carry. */
#define TRANSP ((u2*)(transpbuf + 32))

enum {
    W_PLAIN,
    W_HALF,
    W_ADD,
    W_SUB
};

/* One pixel of sprdrawpr{a,b}[w]16b, with or without a blend. `mask` is the
   a/b split: a-forms skip a pixel a higher-priority sprite already claimed and
   then claim it, b-forms are for lines only one sprite reaches. `idx` is the
   word index, which the byte-wide arrays carry biased by 16.

   The blends leave eax zero, the plain writer the palette index; edx is
   whatever each form last used as scratch. */
static void spr_pixel(regs* const r, u4 const eax, u1 const cl, u1 const ch,
    u4 const idx, u2* const edi, int const mode, int const mask, int const win)
{
    r->ax = eax;
    if (eax == 0) {
        return;
    }
    if (mask && (sprpriodata[idx + 16] & cl)) {
        return;
    }
    if (win && winspdata[idx + 16] != 0) {
        return;
    }
    r->ax = (u1)(eax + ch);
    if (mode == W_PLAIN) {
        r->dx = pal16b[r->ax];
        edi[idx] = (u2)r->dx;
    } else if (mode == W_HALF) {
        u4 col = pal16b[r->ax];

        r->dx = *(u4 const*)(TRANSP + idx);
        /* A transparent pixel underneath means there is nothing to average
           with, so the sprite colour goes down unchanged. */
        if ((u2)r->dx != 0) {
            r->dx &= vesa2_clbit;
            col &= vesa2_clbit;
            col += r->dx;
            col >>= 1;
        }
        edi[idx] = (u2)col;
        r->ax = 0;
    } else {
        /* pal16bcl and pal16bxcl are already clipped, so only the value from
           the transparency buffer needs masking; the pair then goes through
           fulladdtab, read as a dword from a table of words. */
        u4 const* const pal = mode == W_ADD ? pal16bcl : pal16bxcl;
        u4 t = *(u4 const*)(TRANSP + idx);

        t &= vesa2_clbit;
        t += pal[r->ax];
        t >>= 1;
        r->ax = 0;
        memcpy(&t, fulladdtab + t, 4);
        if (mode == W_SUB) {
            t ^= 0xFFFFu;
        }
        r->dx = t;
        edi[idx] = (u2)t;
    }
    if (mask) {
        sprpriodata[idx + 16] |= cl;
    }
}

/* sprdrawa16b / sprdrawaf16b: eight pixels, the flipped form walking the tile
   forwards and the screen backwards. */
static void spr_row(regs* const r, u1 const cl, u1 const ch, u4 const ebx,
    u1 const* const src, u2* const edi, int const flip, int const mode,
    int const mask, int const win)
{
    for (u4 k = 0; k < 8; k++) {
        spr_pixel(r, src[k], cl, ch, ebx - (flip ? k + 1 : 8 - k), edi, mode,
            mask, win);
    }
}

/* sprdrawa / sprdrawaf with sprdrawpra2: a sprite of another priority claims
   its pixels and draws nothing. It reads two pixels per 16-bit load, which is
   where al and ah come from. */
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

/* sprprioritydrawt16b proper. Palette 12 and up picks the blend, anything
   below it goes down as a plain colour, and a sprite of another priority only
   marks. csprprlft, not cl, is the counter, so the loop cannot exit early. */
static void draw_prio(regs* const r, int const mode, int const win)
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
        if (r->dx == ebp) {
            cl = csprbit;
            spr_row(r, cl, ch, ebx, esi->obj, edi, flip,
                ch >= 12 * 16 ? mode : W_PLAIN, 1, win);
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

/* The .drawsingle arm: the line holds one sprite's worth of pixels, walked
   backwards from the last entry, with no priority mask to consult or set. */
static void draw_single(regs* const r, int const mode, int const win)
{
    SpriteInfo const* esi = currentobjptr;
    u2* const edi = (u2*)curvidoffset;
    u1 cl = (u1)r->cx;
    u1 ch = (u1)(r->cx >> 8);
    u4 ebx = 0;

    r->di = (zreg)(uintptr_t)edi;
    /* edx is the walk's starting address, and stays that unless a pixel draws
       over it with something. */
    r->dx = (zreg)(uintptr_t)(esi + cl - 1);
    esi = (SpriteInfo const*)(uintptr_t)r->dx;
    do {
        int const flip = (esi->status & 0x20u) != 0;

        ebx = esi->x;
        ch = esi->pal;
        spr_row(r, cl, ch, ebx, esi->obj, edi, flip,
            ch >= 12 * 16 ? mode : W_PLAIN, 0, win);
        esi--;
    } while (--cl != 0);
    r->bx = ebx;
    r->cx = (r->cx & 0xFFFF0000u) | (u4)ch << 8;
    r->si = (zreg)(uintptr_t)esi;
}

/* Entered from the sprprifix test at the top of drawsprites16t. */
void c_drawsprites16tprio(void)
{
    regs r = { SPPAX, SPPBX, SPPCX, SPPDX, SPPSI, SPPDI, SPPBP };
    /* Half add wants colour maths on, something on the sub screen and not the
       subtractive mode; every other combination is the full add/sub path. */
    int const half = (scaddtype & 0x40u) && (scrnon >> 8) != 0
        && !(scaddtype & 0x80u);
    int mode;

    if (half) {
        mode = W_HALF;
    } else if (scaddtype & 0x80u) {
        mode = W_SUB;
    } else {
        mode = W_ADD;
    }
    /* sprpriorityinit: nothing to draw at this priority, and the window test
       that picks the masked twin. All three modes run the same pair. */
    if (sprclprio[r.bp] != 0) {
        int const win = (cwinenabm & 0x10u) && winonsp != 0;

        if (sprsingle == 1) {
            draw_single(&r, mode, win);
        } else {
            draw_prio(&r, mode, win);
        }
    }
    SPPAX = r.ax;
    SPPBX = r.bx;
    SPPCX = r.cx;
    SPPDX = r.dx;
    SPPSI = r.si;
    SPPDI = r.di;
    SPPBP = r.bp;
}
