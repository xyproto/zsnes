/*
 * The sprite drawers of video/newgfx16.asm: drawsprng16b, drawsprng16bhr and
 * the fourteen arms they dispatch into. The assembly's 58 writer macros are
 * four skeletons plus flags:
 *
 *   A  no window          palette, transparency, store main, store sub
 *   B  window gates both  the window test first, then A
 *   C  window gates sub   store main, window test, store sub
 *   D  window gates main  store sub, window test, store main
 *
 * and independently: which palette half, whether the unused bit is cleared for
 * a low palette entry, whether the sub screen is written, whether the main
 * store ORs the bit back in, and whether the priority bit is claimed before
 * the window test or after. The hi-res forms just pair every store with one
 * 75036 words on.
 */
#include <stdint.h>
#include <string.h>

#include "../types.h"

zreg NGSAX;
zreg NGSBX;
zreg NGSCX;
zreg NGSDX;
zreg NGSSI;
zreg NGSDI;
zreg NGSBP;

typedef struct SpriteInfo {
    u2 x;
    u1* obj __attribute__((packed, aligned(2)));
    u1 pal;
    u1 status;
} SpriteInfo;

extern u1 sprpriodata[288], csprbit, NGNumSpr;
extern u1 BGMS1[], FillSubScr[256], scadtng[256];
extern u1 winbg1enval[];
/* sprtbng holds host pointers into the sprite table, so it is pointer-wide. */
extern u4 csprival;
extern u4 sprleftpr_b[256]; /* sprleftpr..sprleftpr3 as dwords */
extern zreg cpalval[256];
extern zreg sprtbng[256];
/* ngwinptr is a real pointer in ui.c; objclineptr holds byte offsets into it. */
extern u1 *CMainWinScr, *CSubWinScr;
extern u4 objclineptr[256];
extern u1* ngwinptr;
extern u4 UnusedBit[2], UnusedBitXor[2];

/* The second field of a hi-res frame, in 16-bit pixels. */
#define HR 75036u
#define SUB 75036u

enum {
    SK_A,
    SK_B,
    SK_C,
    SK_D
};

typedef struct {
    u1 sk;
    u1 pal512;
    u1 transp;
    u1 sub;
    u1 orbit;
} sform;

typedef struct {
    zreg ax, bx, cx, dx, si, di, bp;
} regs;

/* One pixel. `mask` is the a-form/b-form split: the a-forms skip a pixel a
   higher-priority sprite already claimed and then claim it themselves. */
static void spr_px(regs* const r, u1 const* const src, u4 const k,
    u4 const n, u2* const edi, u2 const* const pal, u1 const* const win,
    u1 const adder, u1 const dl, sform const f, int const mask,
    int const hires)
{
    /* A sprite hanging off the left edge makes this negative, and the
       assembly then stepped back off the line - keep it signed. */
    s4 const i = (s4)((u4)r->bx - n);
    u4 eax = src[k];
    u4 col;

    r->ax = eax;
    if (eax == 0) {
        return;
    }
    eax = (u1)(eax + adder);
    r->ax = eax;
    if (mask && (sprpriodata[i + 16] & dl)) {
        /* C and D put their `xor eax,eax` where every skip falls through it;
           A and B skip past it. Only the routine's exit eax can see this. */
        if (f.sk == SK_C || f.sk == SK_D) {
            r->ax = 0;
        }
        return;
    }
    if (f.sk == SK_B && win[i] == 1) {
        return;
    }
    /* The claim happens as soon as nothing left can skip the whole pixel:
       after the window test for B, straight away for C and D, and only after
       the stores for A. */
    if (mask && f.sk != SK_A) {
        sprpriodata[i + 16] |= dl;
    }
    col = pal[eax + (f.pal512 ? 256u : 0u)];

    if (f.sk == SK_D) {
        /* The sub screen goes down first and unconditionally. */
        edi[i + SUB] = (u2)col;
        if (hires) {
            edi[i + SUB + HR] = (u2)col;
        }
        if (win[i] == 1) {
            r->ax = 0;
            return;
        }
        if (f.transp && adder < 0xC0u) {
            col &= UnusedBitXor[0];
        }
        if (f.orbit) {
            col = (col & 0xFFFF0000u) | (u2)(col | UnusedBit[0]);
        }
        edi[i] = (u2)col;
        if (hires) {
            edi[i + HR] = (u2)col;
        }
        r->ax = 0;
        return;
    }

    if (f.transp && adder < 0xC0u) {
        col &= UnusedBitXor[0];
    }
    edi[i] = (u2)col;
    if (hires) {
        edi[i + HR] = (u2)col;
    }
    if (f.sub) {
        if (f.sk == SK_C && win[i] == 1) {
            r->ax = 0;
            return;
        }
        if (f.transp) {
            col &= UnusedBitXor[0];
        }
        edi[i + SUB] = (u2)col;
        if (hires) {
            edi[i + SUB + HR] = (u2)col;
        }
    }
    r->ax = 0;
    if (mask && f.sk == SK_A) {
        sprpriodata[i + 16] |= dl;
    }
}

/* sprdrawa16b / sprdrawaf16b: eight pixels, the flipped form walking the tile
   forwards and the screen backwards. */
static void spr_row(regs* const r, u1 const* const src, u2* const edi,
    u2 const* const pal, u1 const* const win, u1 const adder, u1 const dl,
    sform const f, int const mask, int const hires, int const flip)
{
    r->ax = 0;
    for (u4 k = 0; k < 8; k++) {
        spr_px(r, src, k, flip ? k + 1 : 8 - k, edi, pal, win, adder, dl, f,
            mask, hires);
    }
}

/* sprdrawa / sprdrawaf with sprdrawpra2: a sprite of another priority claims
   its pixels and draws nothing. Two pixels come out of each 16-bit load. */
static void spr_mark(regs* const r, u1 const* const src, u1 const dl,
    int const flip)
{
    for (u4 k = 0; k < 8; k++) {
        if (k % 2 == 0) {
            r->ax = src[k] | (u4)src[k + 1] << 8;
        }
        if (src[k] != 0) {
            sprpriodata[(s4)((u4)r->bx - (flip ? k + 1u : 8u - k)) + 16] |= dl;
        }
    }
}

static void clear_prio(regs* const r)
{
    csprbit = (u1)(csprbit << 1 | csprbit >> 7);
    if (csprbit == 1) {
        memset(sprpriodata + 16, 0, 256);
        r->ax = 0;
        r->cx = 0;
        r->di = (zreg)(uintptr_t)(sprpriodata + 16 + 256);
    }
}

/* normalsprng16b / normalwsprng16b. `win` non-null is the windowed form, which
   counts the sprites in memory because ecx holds the mask pointer, and carries
   the palette in dh rather than ch. */
/* esi and ebx are pushed on entry and popped before every return, so the
   caller gets them back unchanged; only eax, ecx and edi are outputs. */
static void spr_loop(regs* const r, u2* edi, u2 const* const pal,
    u1 const* const win, sform const f, int const hires, u4 const y)
{
    SpriteInfo const* esi;
    u4 const bx_in = r->bx;
    u1 count = (u1)r->cx;
    u1 ch = (u1)(r->cx >> 8);

    if (win != 0) {
        NGNumSpr = count;
    }
    if (sprleftpr_b[y] & 0x80000000u) {
        /* .drawsingle: one sprite's worth of pixels, walked backwards, with no
           priority mask to consult or set. */
        zreg edx = sprtbng[y] + (zreg)count * sizeof(SpriteInfo)
            - sizeof(SpriteInfo);

        esi = (SpriteInfo const*)(uintptr_t)edx;
        r->bx = 0;
        do {
            int const flip = (esi->status & 0x20u) != 0;

            r->bx = esi->x;
            ch = esi->pal;
            spr_row(r, esi->obj, edi, pal, win, ch, 0, f, 0, hires, flip);
            edx -= sizeof(SpriteInfo);
            esi = (SpriteInfo const*)(uintptr_t)edx;
        } while (--count != 0);
        r->cx = (r->cx & 0xFFFF0000u) | (u4)ch << 8;
        r->bx = bx_in;
        return;
    }

    {
        zreg edx = sprtbng[y];

        esi = (SpriteInfo const*)(uintptr_t)edx;
        r->bx = 0;
        for (;;) {
            int const flip = (esi->status & 0x20u) != 0;
            u1 adder;

            r->bx = esi->x;
            if (win == 0) {
                ch = esi->pal;
            }
            if ((u4)(esi->status & 0x03u) == csprival) {
                if (win != 0) {
                    ch = esi->pal;
                }
                adder = ch;
                spr_row(r, esi->obj, edi, pal, win, adder, csprbit, f, 1,
                    hires, flip);
            } else {
                spr_mark(r, esi->obj, csprbit, flip);
            }
            edx += sizeof(SpriteInfo);
            esi = (SpriteInfo const*)(uintptr_t)edx;
            if (win != 0) {
                if (--NGNumSpr == 0) {
                    break;
                }
            } else if (--count == 0) {
                break;
            }
        }
        r->cx = win != 0 ? 0u : ((r->cx & 0xFFFF0000u) | (u4)ch << 8);
        clear_prio(r);
        r->bx = bx_in;
    }
}

/* The dispatch. Which of the fourteen arms runs is decided by whether this
   layer is on the sub screen, whether the sub screen is a fill, whether colour
   maths applies, and which of the two window masks is active. */
static void dispatch(regs* const r, u4 const y, int const hires)
{
    u2* edi = (u2*)(uintptr_t)r->si;
    u2 const* const pal = (u2 const*)(uintptr_t)cpalval[y];
    u1 const* const wmain = CMainWinScr;
    u1 const* const wsub = CSubWinScr;
    int const submain = (BGMS1[y * 2] & 0x10u) != 0;
    int const fill = (FillSubScr[y] & 1u) != 0;
    /* 0xFFFFFFFF is the "no window" sentinel, and the assembly let it wrap. */
    u1 const* const line = ngwinptr + (s4)objclineptr[y];
    sform f;

    memset(&f, 0, sizeof f);
    r->bp = cpalval[y];
    r->ax = 0;

    if (!(submain && fill)) {
        /* No colour maths on this layer: one plain copy. */
        u1 const* const w = (fill && !submain) ? wsub : wmain;

        if (w[y + 4u * 256u] != 0) {
            f.sk = SK_B;
            if (fill && !submain) {
                edi += SUB;
            }
            spr_loop(r, edi, pal, line, f, hires, y);
            return;
        }
        if (fill && !submain) {
            edi += SUB;
        }
        f.sk = SK_A;
        spr_loop(r, edi, pal, 0, f, hires, y);
        return;
    }

    if (scadtng[y] & 0x10u) {
        f.transp = 1;
        f.pal512 = 1;
        if (!(BGMS1[y * 2 + 1] & 0x10u)) {
            /* Main screen only. */
            f.sk = wmain[y + 4u * 256u] != 0 ? SK_B : SK_A;
            spr_loop(r, edi, pal, f.sk == SK_B ? line : 0, f, hires, y);
            return;
        }
        f.sub = 1;
        if (wmain[y + 4u * 256u] != 0) {
            if (wsub[y + 4u * 256u] != 0) {
                f.sk = SK_B;
            } else {
                /* The window covers the main screen only, and that arm reads
                   the low palette and ORs the unused bit back in. */
                f.sk = SK_D;
                f.pal512 = 0;
                f.orbit = 1;
            }
        } else if (wsub[y + 4u * 256u] != 0) {
            f.sk = SK_C;
        } else {
            f.sk = SK_A;
        }
        spr_loop(r, edi, pal, f.sk == SK_A ? 0 : line, f, hires, y);
        return;
    }

    /* Colour maths off for this layer: the same shapes without the palette
       offset or the transparency step. */
    if (!(BGMS1[y * 2 + 1] & 0x10u)) {
        if (wmain[y + 4u * 256u] != 0) {
            f.sk = SK_B;
            spr_loop(r, edi, pal, line, f, hires, y);
            return;
        }
        f.sk = SK_A;
        spr_loop(r, edi, pal, 0, f, hires, y);
        return;
    }
    f.sub = 1;
    if (wmain[y + 4u * 256u] != 0) {
        if (wsub[y + 4u * 256u] != 0) {
            f.sk = SK_B;
        } else {
            f.sk = SK_D;
        }
    } else if (wsub[y + 4u * 256u] != 0) {
        f.sk = SK_C;
    } else {
        f.sk = SK_A;
    }
    spr_loop(r, edi, pal, f.sk == SK_A ? 0 : line, f, hires, y);
}

void c_drawsprng16b(void);
void c_drawsprng16bhr(void);

void c_drawsprng16b(void)
{
    regs r = { NGSAX, NGSBX, NGSCX, NGSDX, NGSSI, NGSDI, NGSBP };

    dispatch(&r, r.bx, 0);
    NGSAX = r.ax;
    NGSBX = r.bx;
    NGSCX = r.cx;
    NGSDX = r.dx;
    NGSSI = r.si;
    NGSDI = r.di;
    NGSBP = r.bp;
}

void c_drawsprng16bhr(void)
{
    regs r = { NGSAX, NGSBX, NGSCX, NGSDX, NGSSI, NGSDI, NGSBP };

    dispatch(&r, r.bx, 1);
    NGSAX = r.ax;
    NGSBX = r.bx;
    NGSCX = r.cx;
    NGSDX = r.dx;
    NGSSI = r.si;
    NGSDI = r.di;
    NGSBP = r.bp;
}

/* Test entry point: one pixel through one writer shape. The windowed
   skeletons are not reached by any local ROM, so test/difftest_ngspr.c
   compares them against verbatim transcriptions of the macros. */
void ng_spr_test(u1 pixel, u4 x, u4 n, u2* edi, u2 const* pal, u1 const* win,
    u1 adder, u1 dl, int sk, int pal512, int transp, int sub, int orbit,
    int mask, int hires, u4* eax_out);

void ng_spr_test(u1 const pixel, u4 const x, u4 const n, u2* const edi,
    u2 const* const pal, u1 const* const win, u1 const adder, u1 const dl,
    int const sk, int const pal512, int const transp, int const sub,
    int const orbit, int const mask, int const hires, u4* const eax_out)
{
    regs r;
    sform f;
    u1 src[8];

    memset(&r, 0, sizeof r);
    memset(&f, 0, sizeof f);
    memset(src, 0, sizeof src);
    src[0] = pixel;
    r.bx = x;
    f.sk = (u1)sk;
    f.pal512 = (u1)pal512;
    f.transp = (u1)transp;
    f.sub = (u1)sub;
    f.orbit = (u1)orbit;
    spr_px(&r, src, 0, n, edi, pal, win, adder, dl, f, mask, hires);
    *eax_out = r.ax;
}
