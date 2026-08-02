/*
 * video/c_mv16tsms.c - draw8x816tsms and draw8x816tswinonms, ported from
 * video/mv16tms.asm.
 *
 * Draws a row of 8x8 tiles with subtractive colour maths: each pixel is looked
 * up in pal16bxcl, averaged with what the transparency buffer already holds,
 * run through fulladdtab and inverted.
 *
 * Reached by jump from draw8x816tms once the shared prologue has run, with
 * esi = the video pointer, ebp = the transparency buffer, edi = the tile map
 * and dl = the starting tile column. Both are biased by the horizontal offset
 * already; see video/c_mv16tms.c.
 */
#include <stdint.h>
#include <string.h>

#include "../types.h"

u4 MVSAX;
u4 MVSBX;
u4 MVSCX;
u4 MVSDX;
u4 MVSSI;
u4 MVSDI;
u4 MVSBP;

extern u1 tileleft16b, drawn, temp, bshifter, curbgpr, bgcoloradder;
extern u1 coadder16; /* the palette adder, in memory for the winon writers */
extern u1* winptrref;
extern u4 tempcach, temptile, bgofwptr, bgsubby, yadder, yrevadder;
extern u4 pal16bxcl[256];
extern u2 fulladdtab[65536]; /* the dword load above reads one past */

#define CLBIT 0xF7DEu /* 1111011111011110b - clears the low bit of each 5-6-5 channel */

/* The scratch registers the pixel writer leaves behind. The routine returns
   with whatever the last executed pixel put there, and its caller is still
   assembly, so they are modelled rather than assumed dead. */
struct mvregs {
    u4 eax, ebx, ecx;
};

/* One pixel. `win` is NULL for the unmasked form; for the masked one a
   non-zero mask byte drops the pixel and the palette adder comes from the
   coadder16 global instead of dh. */
static void write_c(struct mvregs* const r, u1 const* const src, u4 const n,
    u1 const* const win, u4 const wn, u4 const off, u1* const esi,
    u1* const ebp, u1 const dh)
{
    u1 const al = src[n];

    r->eax = (r->eax & ~0xFFu) | al;
    if (al == 0 || (win != 0 && win[wn] != 0)) {
        return;
    }
    r->eax = (r->eax & ~0xFFu) | (u1)(al + (win ? coadder16 : dh));
    r->ebx = *(u4*)(ebp + off);
    r->ecx = pal16bxcl[(u1)r->eax];
    *(u2*)(ebp + off) = (u2)r->ecx;
    r->ebx &= CLBIT;
    r->ecx += r->ebx;
    r->ecx >>= 1;
    /* A dword load from a table of words: it picks up the next entry in the
       top half too, and only the low half is stored. The index is the whole
       of ecx, which stays inside the table because a palette entry is 16 bits
       and the average of two of them cannot carry out. Reproduced, not fixed. */
    memcpy(&r->ecx, fulladdtab + r->ecx, 4);
    r->ecx ^= 0xFFFFu;
    *(u2*)(esi + off) = (u2)r->ecx;
}

/* drawtilegrpfull / drawtilegrpfullf: two groups of four, each skipped when
   its source dword is zero. `flip` walks the tile backwards - but the window
   mask is still read forwards, which is what the 7-n in the flipped writer
   does. */
static void draw_group(struct mvregs* const r, u1 const* const src,
    u1 const* const win, u1* const esi, u1* const ebp, u1 const dh,
    int const flip)
{
    static u1 const fwd[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    static u1 const rev[8] = { 7, 6, 5, 4, 3, 2, 1, 0 };
    u1 const* const ix = flip ? rev : fwd;

    r->eax = 0;
    r->ecx = 0;
    for (u4 g = 0; g < 2; g++) {
        u4 const test = g == 0 ? (flip ? 4u : 0u) : (flip ? 0u : 4u);
        if (*(u4 const*)(src + test) == 0) {
            continue;
        }
        for (u4 i = g * 4u; i < g * 4u + 4u; i++) {
            write_c(r, src, ix[i], win, flip ? (u4)(7u - ix[i]) : ix[i],
                i * 2, esi, ebp, dh);
        }
    }
}

/* The two routines differ only in the mask, where the palette adder lives and
   which counter tracks the column, so they share one body. */
static void draw_row(int const winon)
{
    struct mvregs r = { MVSAX, MVSBX, MVSCX };
    u1* esi = (u1*)(uintptr_t)MVSSI;
    u1* ebp = (u1*)(uintptr_t)MVSBP;
    u1 const* edi = (u1 const*)(uintptr_t)MVSDI;
    u1 const* edx = winon ? winptrref : 0;
    u1 dl = temp;
    u1 dh = (u1)(MVSDX >> 8);

    tileleft16b = 33;
    drawn = 0;

    do {
        u2 const entry = *(u2 const*)edi;
        /* bit 5 = priority, 6 = flip x, 7 = flip y, 10-12 = palette. */
        u1 const flags = (u1)((entry >> 8) ^ curbgpr);

        /* mov ax,[edi] writes only ax; the flags go to dh, or to cl in the
           masked form. */
        r.eax = (r.eax & ~0xFFFFu) | entry;
        if (winon) {
            r.ecx = (r.ecx & ~0xFFu) | flags;
        } else {
            dh = flags;
        }
        edi += 2;

        if (!(flags & 0x20u)) {
            u4 tile;

            drawn++;
            r.eax &= 0x03FFu;
            tile = tempcach + (r.eax << 6);
            if (tile >= bgofwptr) {
                tile -= bgsubby;
            }
            tile += (flags & 0x80u) ? yrevadder : yadder;
            /* An 8-bit shift, so a large shifter leaves nothing. */
            if (winon) {
                r.eax = (r.eax & ~0xFFu) | flags;
                r.ecx = (r.ecx & ~0xFFu) | bshifter;
                coadder16 = (u1)((u1)((flags & 0x1Cu) << (bshifter & 31u))
                    + bgcoloradder);
            } else {
                r.ecx = (r.ecx & ~0xFFu) | bshifter;
                dh = (u1)((u1)((flags & 0x1Cu) << (bshifter & 31u))
                    + bgcoloradder);
            }
            draw_group(&r, (u1 const*)(uintptr_t)tile, edx, esi, ebp, dh,
                (flags & 0x40u) != 0);
        }
        esi += 16;
        ebp += 16;
        if (winon) {
            edx += 8;
            if (++temp == 0x20u) {
                edi = (u1 const*)(uintptr_t)temptile;
            }
        } else if (++dl == 0x20u) {
            edi = (u1 const*)(uintptr_t)temptile;
        }
    } while (--tileleft16b != 0);

    MVSAX = r.eax;
    MVSBX = r.ebx;
    MVSCX = r.ecx;
    MVSDX = winon ? (u4)(uintptr_t)edx
                  : ((MVSDX & ~0xFFFFu) | ((u4)dh << 8) | dl);
    MVSSI = (u4)(uintptr_t)esi;
    MVSBP = (u4)(uintptr_t)ebp;
    MVSDI = (u4)(uintptr_t)edi;
}

void c_draw8x816tsms(void) { draw_row(0); }
void c_draw8x816tswinonms(void) { draw_row(1); }
