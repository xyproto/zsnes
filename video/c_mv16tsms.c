/*
 * video/c_mv16tsms.c - the eleven tile-row routines of video/mv16tms.asm.
 *
 * Colour-maths rows: each pixel is looked up in a palette table, averaged with
 * what the transparency buffer already holds and run through fulladdtab. Three
 * pixel writers appear, named a/b/c after the assembly macros - a averages into
 * pal16b and only when the transparency buffer is non-zero, b and c use
 * pal16bcl and pal16bxcl, and c inverts the result (the subtractive form).
 * Each writer has a plain and a window-masked variant, and 8x8 and 16x16 row
 * walkers use them in the seven combinations the assembly had.
 *
 * All of them are reached by jump from the dispatch that stays in the assembly,
 * with esi = the video pointer, ebp = the transparency buffer, edi = the tile
 * map and either dl or temp holding the starting tile column; esi and ebp are
 * biased by the horizontal offset already. See video/c_mv16tms.c.
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

/* Non-zero when the caller must tail-jump to domosaic16b, which the assembly
   does with dh carrying curmosaicsz. */
u4 MVSMosaic;

extern u1 tileleft16b, drawn, temp, bshifter, curbgpr, bgcoloradder;
extern u1 curmosaicsz;
extern u1 coadder16; /* the palette adder, in memory for the winon writers */
extern u1* winptrref;
extern u4 tempcach, temptile, bgofwptr, bgsubby, yadder, yrevadder;
extern u4 pal16bcl[256], pal16bxcl[256];
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
    u1* const ebp, u1 const dh, int const sub)
{
    u1 const al = src[n];

    r->eax = (r->eax & ~0xFFu) | al;
    if (al == 0 || (win != 0 && win[wn] != 0)) {
        return;
    }
    r->eax = (r->eax & ~0xFFu) | (u1)(al + (win ? coadder16 : dh));
    r->ebx = *(u4*)(ebp + off);
    r->ecx = (sub ? pal16bxcl : pal16bcl)[(u1)r->eax];
    *(u2*)(ebp + off) = (u2)r->ecx;
    r->ebx &= CLBIT;
    r->ecx += r->ebx;
    r->ecx >>= 1;
    /* A dword load from a table of words: it picks up the next entry in the
       top half too, and only the low half is stored. The index is the whole
       of ecx, which stays inside the table because a palette entry is 16 bits
       and the average of two of them cannot carry out. Reproduced, not fixed. */
    memcpy(&r->ecx, fulladdtab + r->ecx, 4);
    if (sub) {
        r->ecx ^= 0xFFFFu;
    }
    *(u2*)(esi + off) = (u2)r->ecx;
}

/* drawtilegrpfull / drawtilegrpfullf: two groups of four, each skipped when
   its source dword is zero. `flip` walks the tile backwards - but the window
   mask is still read forwards, which is what the 7-n in the flipped writer
   does. */
static void draw_group(struct mvregs* const r, u1 const* const src,
    u1 const* const win, u1* const esi, u1* const ebp, u1 const dh,
    int const flip, int const sub)
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
                i * 2, esi, ebp, dh, sub);
        }
    }
}

/* The four routines differ only in the mask, where the palette adder lives,
   which counter tracks the column, whether the writer subtracts, and whether
   there is a mosaic tail - so they share one body. */
static void draw_row(int const winon, int const sub, int const mosaic)
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
                (flags & 0x40u) != 0, sub);
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

    MVSMosaic = 0;
    if (mosaic && drawn != 0) {
        dh = curmosaicsz;
        MVSMosaic = dh != 1 ? 1u : 0u;
    }
    MVSAX = r.eax;
    MVSBX = r.ebx;
    MVSCX = r.ecx;
    MVSDX = winon ? (u4)(uintptr_t)edx
                  : ((MVSDX & ~0xFFFFu) | ((u4)dh << 8) | dl);
    MVSSI = (u4)(uintptr_t)esi;
    MVSBP = (u4)(uintptr_t)ebp;
    MVSDI = (u4)(uintptr_t)edi;
}

void c_draw8x816tsms(void) { draw_row(0, 1, 0); }
void c_draw8x816tswinonms(void) { draw_row(1, 1, 0); }
void c_draw8x8fulladdms(void) { draw_row(0, 0, 1); }
void c_draw8x8fulladdwinonms(void) { draw_row(1, 0, 0); }

/* --- the 'a' writer: main screen with a conditional average ---------------- *
 *
 * draw8x816tams and its two windowed forms. Unlike the b/c writers this one
 * keeps the colour in eax and the transparency value in ecx, reads the tile
 * through ebx rather than edi, and only averages when the transparency buffer
 * already holds something.
 */
extern u4 pal16b[256];

static void write_a(struct mvregs* const r, u1 const* const src, u4 const n,
    u1 const* const win, u4 const wn, u4 const off, u1* const esi,
    u1* const ebp, u1 const dh)
{
    u1 const al = src[n];

    r->eax = (r->eax & ~0xFFu) | al;
    if (al == 0 || (win != 0 && win[wn] != 0)) {
        return;
    }
    r->eax = (r->eax & ~0xFFu) | (u1)(al + (win ? coadder16 : dh));
    r->ecx = *(u4*)(ebp + off);
    r->eax = pal16b[(u1)r->eax];
    *(u2*)(ebp + off) = (u2)r->eax;
    if ((r->ecx & 0xFFFFu) != 0) {
        r->eax &= CLBIT;
        r->ecx &= CLBIT;
        r->eax += r->ecx;
        r->eax >>= 1;
    }
    *(u2*)(esi + off) = (u2)r->eax;
    r->eax = 0;
}

/* drawtilegrp / drawtilegrpf: as the full forms, but the zero test is on the
   tile pointer in ebx. */
static void draw_group_a(struct mvregs* const r, u1 const* const src,
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
            write_a(r, src, ix[i], win, flip ? (u4)(7u - ix[i]) : ix[i],
                i * 2, esi, ebp, dh);
        }
    }
}

static void draw_row_a(int const winon)
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
        u1 const flags = (u1)((entry >> 8) ^ curbgpr);

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
            r.ebx = tile;
            r.ecx = (r.ecx & ~0xFFu) | bshifter;
            if (winon) {
                r.eax = (r.eax & ~0xFFu) | flags;
                coadder16 = (u1)((u1)((flags & 0x1Cu) << (bshifter & 31u))
                    + bgcoloradder);
            } else {
                dh = (u1)((u1)((flags & 0x1Cu) << (bshifter & 31u))
                    + bgcoloradder);
            }
            draw_group_a(&r, (u1 const*)(uintptr_t)tile, edx, esi, ebp, dh,
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

    /* dh takes curmosaicsz whenever anything drew, whether or not the mosaic
       tail is then taken - the fall-through path returns it. */
    MVSMosaic = 0;
    if (drawn != 0) {
        dh = curmosaicsz;
        MVSMosaic = dh != 1 ? 1u : 0u;
    }
    MVSAX = r.eax;
    MVSBX = r.ebx;
    MVSCX = r.ecx;
    MVSDX = winon ? (u4)(uintptr_t)edx
                  : ((MVSDX & ~0xFFFFu) | ((u4)dh << 8) | dl);
    MVSSI = (u4)(uintptr_t)esi;
    MVSBP = (u4)(uintptr_t)ebp;
    MVSDI = (u4)(uintptr_t)edi;
}

void c_draw8x816tms_body(void) { draw_row_a(0); }
void c_draw8x816twinonms_body(void) { draw_row_a(1); }

/* --- the 16x16 tile rows -------------------------------------------------- *
 *
 * Same writers, but each map entry covers a 16x16 tile, so the row is walked
 * half a tile at a time: a16x16xinc toggles every column and decides whether
 * this step advances the map pointer or just moves to the tile's other half.
 * The x flip swaps which of the two that means, the y flip which row.
 *
 * The arithmetic on the tile number is 16-bit (`and ax,3FFh`, `shl ax,6`) but
 * the result reaches edi as a full dword, so the top half of eax takes part.
 * It is zero after any tile that drew, and whatever the caller left after one
 * skipped on priority.
 */
extern u1 a16x16xinc; /* video/makevid.h */
extern u2 yadd, yflipadd; /* video/c_makev16tdata.c */

static u4 set_lo16(u4 const r, u2 const v)
{
    return (r & ~0xFFFFu) | v;
}

/* The four forms differ only in which pixel writer they use and whether a
   window mask is in play. The windowed ones take the palette adder from
   coadder16, walk a mask pointer in edx and count the column in memory; the
   plain one keeps all three in registers and has the mosaic tail. */
enum mv16var { MV16_A,
    MV16_B,
    MV16_C };

static void draw_row16(enum mv16var const var, int const winon)
{
    struct mvregs r = { MVSAX, MVSBX, MVSCX };
    u1* esi = (u1*)(uintptr_t)MVSSI;
    u1* ebp = (u1*)(uintptr_t)MVSBP;
    u1 const* edi = (u1 const*)(uintptr_t)MVSDI;
    u1 const* edx = winon ? winptrref : 0;
    u1 dl = temp;
    u1 dh = (u1)(MVSDX >> 8);

    tileleft16b = 33;

    do {
        u2 const entry = *(u2 const*)edi;
        u1 const raw = (u1)(entry >> 8);
        u1 const flags = (u1)(raw ^ curbgpr);

        r.eax = set_lo16(r.eax, entry);
        if (winon) {
            r.ecx = (r.ecx & ~0xFFu) | raw;
        } else {
            dh = raw;
        }
        a16x16xinc ^= 1u;

        /* Half of the tile, so either the map pointer moves or the tile
           number does; the flip decides which happens on which column. */
        if (!(raw & 0x40u)) {
            if (!(a16x16xinc & 1u)) {
                r.eax = set_lo16(r.eax, (u2)((u2)r.eax + 1u));
                edi += 2;
            }
        } else if (!(a16x16xinc & 1u)) {
            edi += 2;
        } else {
            r.eax = set_lo16(r.eax, (u2)((u2)r.eax + 1u));
        }

        if (winon) {
            r.ecx = (r.ecx & ~0xFFu) | flags;
        } else {
            dh = flags;
        }
        if (!(flags & 0x20u)) {
            u4 tile;

            drawn++;
            r.eax = set_lo16(
                r.eax, (u2)((u2)r.eax + ((flags & 0x80u) ? yflipadd : yadd)));
            r.eax = set_lo16(r.eax, (u2)((u2)r.eax & 0x03FFu));
            r.eax = set_lo16(r.eax, (u2)((u2)r.eax << 6));
            tile = tempcach + r.eax;
            if (tile >= bgofwptr) {
                tile -= bgsubby;
            }
            tile += (flags & 0x80u) ? yrevadder : yadder;

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
            if (var == MV16_A) {
                r.ebx = tile; /* the a writer reads the tile through ebx */
                draw_group_a(&r, (u1 const*)(uintptr_t)tile, edx, esi, ebp,
                    winon ? 0 : dh, (flags & 0x40u) != 0);
            } else {
                draw_group(&r, (u1 const*)(uintptr_t)tile, edx, esi, ebp,
                    winon ? 0 : dh, (flags & 0x40u) != 0, var == MV16_C);
            }
        }
        esi += 16;
        ebp += 16;
        if (winon) {
            edx += 8;
            if (!(a16x16xinc & 1u)) {
                temp++;
            }
            if (temp == 0x20u) {
                temp = 0;
                edi = (u1 const*)(uintptr_t)temptile;
            }
        } else {
            if (!(a16x16xinc & 1u)) {
                dl++;
            }
            if (dl == 0x20u) {
                dl = 0;
                edi = (u1 const*)(uintptr_t)temptile;
            }
        }
    } while (--tileleft16b != 0);

    if (!winon) {
        dh = curmosaicsz;
        MVSMosaic = dh != 1 ? 1u : 0u;
    }
    MVSAX = r.eax;
    MVSBX = r.ebx;
    MVSCX = r.ecx;
    MVSDX = winon ? (u4)(uintptr_t)edx
                  : ((MVSDX & ~0xFFFFu) | ((u4)dh << 8) | dl);
    MVSSI = (u4)(uintptr_t)esi;
    MVSBP = (u4)(uintptr_t)ebp;
    MVSDI = (u4)(uintptr_t)edi;
}

void c_draw16x1616tms_body(void) { draw_row16(MV16_A, 0); }
void c_draw16x16fulladdms(void) { draw_row16(MV16_B, 0); }
void c_draw16x1616tsms(void) { draw_row16(MV16_C, 0); }
void c_draw16x1616twinonms(void) { draw_row16(MV16_A, 1); }
void c_draw16x16fulladdwinonms(void) { draw_row16(MV16_B, 1); }
void c_draw16x1616tswinonms(void) { draw_row16(MV16_C, 1); }
