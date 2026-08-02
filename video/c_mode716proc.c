/*
 * video/c_mode716proc.c - the four Mode7Process* macros, ported from
 * video/mode716.mac.
 *
 * The main Mode 7 scanline walk, in three shapes picked by mode7set:
 *
 *   bit 7 clear      the map repeats: the tile pointer wraps and every pixel
 *                    draws (the %%nextval loop)
 *   bit 7 set        no repetition: pixels outside the 1024x1024 map are
 *                    skipped, and the walk switches to %%nextvalr once it is
 *                    back on the map
 *   bit 7 + bit 6    as above, but off-map pixels repeat the edge tile
 *
 * Reached by falling into it from the Mode7Startup macro with edi = vram,
 * esi = the video pointer, ebp = the palette and the rest zeroed; the macro's
 * tail keeps the finishmode7 exit in assembly, because it is a tail-jump to
 * domosaicng16b with dh live.
 *
 * The positions are 32-bit fixed point with the map coordinate in bytes 1-2,
 * which is why several reads below are unaligned dwords taken at byte 1.
 */
#include <stdint.h>
#include <string.h>

#include "../types.h"
#include "c_mode716write.h"

u4 M7PAX;
u4 M7PBX;
u4 M7PCX;
u4 M7PDX;
u4 M7PSI;
u4 M7PDI;
u4 M7PBP;
u4 M7PWriter;
u4 M7PWriter2; /* Mode7Processngw216b alternates between the two */

extern u4 mtemp; /* video/c_mode716data.c */
extern u4 mm7xaddof, mm7yaddof;
extern u1 mm7xaddof2, mm7yaddof2; /* byte writes into dword slots */
extern u4 mmode7xpos, mmode7ypos, mmode7xrpos, mmode7yrpos;
extern u4 mmode7xadder, mmode7yadder, mmode7xadd2, mmode7yadd2;
extern u4 mmode7ptr;
extern u1 mmode7xinc, mmode7xincc, mmode7yinc;
extern u4 mode7xpos, mode7ypos, mode7xrpos, mode7yrpos;
extern u4 mode7xadder, mode7yadder;
extern u4 m7xaddof, m7yaddof, ngwleft, ngwleftb;
extern u4* ngcwinptr; /* the window run list */
extern u1 m7xaddof2, m7yaddof2, switchtorep3;
extern u1 mode7set; /* cpu/regs.inc */

void c_ProcessMode7ngwin16b(void); /* video/c_mode716win.c */
void c_ProcessMode7ngwinB16b(void);
void c_ProcessMode7ngwinC16b(void);
void c_ProcessMode7ngwinD16b(void);
void c_ProcessMode7ngwinE16b(void);
extern u4 M7WinAX, M7WinBX, M7WinCX, M7WinSI, M7WinDI;
extern u1 mode7tab[65536]; /* ui.h */
extern u1 vrama[65536];
extern u1* vram;

/* A dword read one byte into a position; the map coordinate straddles the
   byte boundary, so this is deliberately unaligned. */
static u4 dword_at1(u4 const* const p)
{
    u4 v;

    memcpy(&v, (u1 const*)p + 1, 4);
    return v;
}

static u1 byte1(u4 const v)
{
    return (u1)(v >> 8);
}

/* Byte 2 is the whole-tile part of the coordinate: over 3 is off the map. */
static u1 byte2(u4 const v)
{
    return (u1)(v >> 16);
}

static u4 set_lo(u4 const r, u1 const v)
{
    return (r & ~0xFFu) | v;
}

static u4 set_hi(u4 const r, u1 const v)
{
    return (r & ~0xFF00u) | ((u4)v << 8);
}

/* Point edi at the tile the current position lands in, and record the tile
   pointer. Shared by the repeating walk and the on-map re-entry. */
static void seek_tile(u4* const eax, u4* const ebx, u4* const ecx,
    u1** const edi)
{
    u4 bx = dword_at1(&mmode7ypos) << 5;
    u4 ax = dword_at1(&mmode7xpos) >> 3;

    bx &= 0x7FF8u;
    ax = set_lo(ax, (u1)((u1)ax << 1)); /* shl al,1 - the byte only */
    bx = set_lo(bx, (u1)ax);
    *eax = ax;
    *edi = vram;
    *ecx = set_hi(*ecx, 0);
    mmode7ptr = bx;
    *ecx = set_lo(*ecx, (*edi)[bx]);
    *ecx <<= 7;
    *edi += *ecx;
    *ebx = bx;
}

/* The registers the walk carries. Keeping them in one place is what lets the
   two macros share their second half. */
struct m7regs {
    u4 eax, ebx, ecx, edx;
    u1 *esi, *edi;
};

/* The no-repetition half, identical in both macros apart from two things:
   `mask` is the off-tile test (0x08 in Mode7Process, 0xF8 in Mode7ProcessB),
   and `retry` says whether the off-tile handlers jump back above their own
   test - ProcessB's do (%%roffxretb -> %%nextvalr), Mode7Process's jump
   forward past it instead. */
static void m7_norepeat(struct m7regs* const r, enum m7_writer const w,
    u2 const* const pal, u1 const mask, int const retry)
{
    if (!(mode7set & 0x40u)) {
        while (byte2(mmode7ypos) > 3u || byte2(mmode7xpos) > 3u) {
            r->eax = mmode7xadder;
            r->ebx = mmode7yadder;
            mmode7xpos += mmode7xadder;
            mmode7ypos -= mmode7yadder;
            r->esi += 2;
            if (--mtemp == 0) {
                return;
            }
        }
    } else {
        /* Tile repeat: off-map pixels draw the edge tile instead of nothing. */
        while (byte2(mmode7ypos) > 3u || byte2(mmode7xpos) > 3u) {
            r->ecx = set_hi(r->ecx, byte1(mmode7xpos));
            r->eax = mmode7xadder;
            r->ecx = set_lo(r->ecx, byte1(mmode7ypos));
            r->ebx = mmode7yadder;
            r->edx = set_lo(r->edx, mode7tab[r->ecx]);
            mmode7xpos += mmode7xadder;
            r->edx = set_lo(r->edx, vrama[r->edx]);
            mmode7ypos -= mmode7yadder;
            r->edx = m7_write(w, &r->esi, pal, r->edx);
            if (--mtemp == 0) {
                return;
            }
        }
    }

    mmode7xrpos = mmode7xpos & 0x7FFu;
    mmode7yrpos = mmode7ypos & 0x7FFu;
    seek_tile(&r->eax, &r->ebx, &r->ecx, &r->edi);

    for (;;) {
        for (;;) {
            if (byte1(mmode7xrpos) & mask) {
                r->eax = set_lo(r->eax, mmode7xinc);
                r->edi = vram;
                mmode7ptr = set_lo(mmode7ptr, (u1)((u1)mmode7ptr + mmode7xinc));
                r->ecx = set_lo(r->ecx, mmode7xincc);
                if ((u1)mmode7ptr == mmode7xincc) {
                    goto wrapped;
                }
                r->ebx = mmode7ptr;
                r->ecx = (u4)r->edi[r->ebx] << 7;
                r->eax = mmode7xadd2;
                mmode7xrpos -= mmode7xadd2;
                r->edi += r->ecx;
                if (retry) {
                    continue;
                }
            }
            if (!(byte1(mmode7yrpos) & mask)) {
                break;
            }
            r->eax = set_lo(r->eax, mmode7yinc);
            r->edi = vram;
            mmode7ptr = set_hi(mmode7ptr, (u1)((u1)(mmode7ptr >> 8) - mmode7yinc));
            if ((u1)(mmode7ptr >> 8) & 0x80u) { /* js */
                goto wrapped;
            }
            r->ebx = mmode7ptr;
            r->ecx = (u4)r->edi[r->ebx] << 7;
            r->eax = mmode7yadd2;
            mmode7yrpos += mmode7yadd2;
            r->edi += r->ecx;
            if (!retry) {
                break;
            }
        }
        r->ecx = set_lo(r->ecx, byte1(mmode7yrpos));
        r->ecx = set_hi(r->ecx, byte1(mmode7xrpos));
        mmode7xrpos += mmode7xadder;
        mmode7yrpos -= mmode7yadder;
        r->edx = mode7tab[r->ecx];
        r->edx = set_lo(r->edx, r->edi[r->edx]);
        r->edx = m7_write(w, &r->esi, pal, r->edx);
        if (--mtemp == 0) {
            return;
        }
    }

wrapped:
    if (!(mode7set & 0x40u)) {
        return;
    }
    do {
        mmode7yrpos &= 0xFFFF07FFu;
        mmode7xrpos &= 0xFFFF07FFu;
        r->ecx = set_lo(r->ecx, byte1(mmode7yrpos));
        r->eax = mmode7xadder;
        r->ecx = set_hi(r->ecx, byte1(mmode7xrpos));
        mmode7xrpos += mmode7xadder;
        r->edx = set_lo(r->edx, mode7tab[r->ecx]);
        r->eax = mmode7yadder;
        mmode7yrpos -= mmode7yadder;
        r->edx = set_lo(r->edx, vrama[r->edx]);
        r->edx = m7_write(w, &r->esi, pal, r->edx);
    } while (--mtemp != 0);
}

static void m7_load(struct m7regs* const r)
{
    r->eax = M7PAX;
    r->ebx = M7PBX;
    r->ecx = M7PCX;
    r->edx = M7PDX;
    r->esi = (u1*)(uintptr_t)M7PSI;
    r->edi = (u1*)(uintptr_t)M7PDI;
}

static void m7_store(struct m7regs const* const r)
{
    M7PAX = r->eax;
    M7PBX = r->ebx;
    M7PCX = r->ecx;
    M7PDX = r->edx;
    M7PSI = (u4)(uintptr_t)r->esi;
    M7PDI = (u4)(uintptr_t)r->edi;
}

void c_Mode7Process(void)
{
    enum m7_writer const w = (enum m7_writer)M7PWriter;
    u2 const* const pal = (u2 const*)(uintptr_t)M7PBP;
    struct m7regs r;

    m7_load(&r);
    mtemp = 256;

    if (mode7set & 0x80u) {
        m7_norepeat(&r, w, pal, 0x08u, 0);
        m7_store(&r);
        return;
    }

    mmode7xrpos = mmode7xpos & 0x7FFu;
    mmode7yrpos = mmode7ypos & 0x7FFu;
    seek_tile(&r.eax, &r.ebx, &r.ecx, &r.edi);
    r.eax = mmode7xrpos;
    r.ebx = mmode7ptr;

    do {
        if (byte1(r.eax) & 0x08u) {
            r.ebx = set_lo(r.ebx, (u1)((u1)r.ebx + mmode7xinc));
            r.ecx = (u4)vrama[r.ebx] << 7;
            r.eax -= mmode7xadd2;
            r.edi = vrama + r.ecx;
        }
        if (byte1(mmode7yrpos) & 0x08u) {
            r.ebx = set_hi(r.ebx, (u1)((u1)(r.ebx >> 8) - mmode7yinc));
            r.ebx &= 0x7FFFu;
            r.ecx = (u4)vrama[r.ebx] << 7;
            mmode7yrpos += mmode7yadd2;
            r.edi = vrama + r.ecx;
        }
        r.ecx = set_hi(r.ecx, byte1(r.eax));
        r.ecx = set_lo(r.ecx, byte1(mmode7yrpos));
        mmode7yrpos -= mmode7yadder;
        r.edx = mode7tab[r.ecx];
        r.eax += mmode7xadder;
        r.edx = set_lo(r.edx, r.edi[r.edx]);
        r.edx = m7_write(w, &r.esi, pal, r.edx);
    } while (--mtemp != 0);

    m7_store(&r);
}

/* --- Mode7ProcessB -------------------------------------------------------- *
 *
 * The same walk for adders of a whole tile or more: one pixel step can cross
 * several tiles, so a prologue works out how far the tile pointer moves per
 * step (mm7xaddof/mm7xaddof2) and the off-tile handlers apply that in one go.
 * Reached from the .nextval3 arm of Mode7NonMainSub.
 */

/* How much of one step is whole tiles. The compare is signed, as in the
   assembly - a negated 0x80000000 stays negative and falls straight through. */
static void step_offsets(u4 const adder, u4 const add2, u1 const inc,
    u4* const off, u1* const off2, u4* const ecx, u4* const edx)
{
    s4 bx = (s4)adder;
    u4 cx = 0;
    u1 dl = 0;

    if (adder & 0x80000000u) {
        bx = -bx;
    }
    while (bx >= 0x800) {
        bx -= 0x800;
        cx += add2;
        dl = (u1)(dl + inc);
    }
    *off = cx;
    *off2 = dl;
    *ecx = cx;
    *edx = set_lo(*edx, dl);
}

void c_Mode7ProcessB(void)
{
    enum m7_writer const w = (enum m7_writer)M7PWriter;
    u2 const* const pal = (u2 const*)(uintptr_t)M7PBP;
    struct m7regs r;

    m7_load(&r);
    mtemp = 256;

    if (mode7set & 0x80u) {
        m7_norepeat(&r, w, pal, 0xF8u, 1);
        m7_store(&r);
        return;
    }

    mmode7xrpos = mmode7xpos & 0x7FFu;
    mmode7yrpos = mmode7ypos & 0x7FFu;
    seek_tile(&r.eax, &r.ebx, &r.ecx, &r.edi);

    step_offsets(mmode7xadder, mmode7xadd2, mmode7xinc, &mm7xaddof, &mm7xaddof2,
        &r.ecx, &r.edx);
    step_offsets(mmode7yadder, mmode7yadd2, mmode7yinc, &mm7yaddof, &mm7yaddof2,
        &r.ecx, &r.edx);
    r.ecx = 0;
    r.eax = mmode7xrpos;
    r.ebx = mmode7ptr;

    do {
        if (byte1(r.eax) & 0xF8u) {
            r.eax -= mm7xaddof;
            r.ebx = set_lo(r.ebx, (u1)((u1)r.ebx + mm7xaddof2));
            if (byte1(r.eax) & 0xF8u) {
                r.ebx = set_lo(r.ebx, (u1)((u1)r.ebx + mmode7xinc));
                r.eax -= mmode7xadd2;
            }
            r.ecx = (u4)vrama[r.ebx] << 7;
            r.edi = vrama + r.ecx;
        }
        if (byte1(mmode7yrpos) & 0xF8u) {
            r.edx = mm7yaddof;
            mmode7yrpos += r.edx;
            r.ebx = set_hi(r.ebx, (u1)((u1)(r.ebx >> 8) - mm7yaddof2));
            if (byte1(mmode7yrpos) & 0xF8u) {
                r.edx = mmode7yadd2;
                r.ebx = set_hi(r.ebx, (u1)((u1)(r.ebx >> 8) - mmode7yinc));
                mmode7yrpos += r.edx;
            }
            r.ebx &= 0x7FFFu;
            r.ecx = (u4)vrama[r.ebx] << 7;
            r.edi = vrama + r.ecx;
        }
        r.ecx = set_hi(r.ecx, byte1(r.eax));
        r.ecx = set_lo(r.ecx, byte1(mmode7yrpos));
        mmode7yrpos -= mmode7yadder;
        r.edx = mode7tab[r.ecx];
        r.eax += mmode7xadder;
        r.edx = set_lo(r.edx, r.edi[r.edx]);
        r.edx = m7_write(w, &r.esi, pal, r.edx);
    } while (--mtemp != 0);

    m7_store(&r);
}

/* --- Mode7Processngw16b --------------------------------------------------- *
 *
 * Mode7ProcessB again, but windowed: the run lengths in ngwleft/ngwleftb come
 * from the ngwin cluster, which is called back into whenever a run ends to
 * skip the masked pixels and start the next one. The walk itself uses the
 * unprefixed mode7* positions, while the prologue still reads the mmode7*
 * ones - that asymmetry is in the original, not a slip.
 */

/* The cluster is C too, but it is reached through its own register seam. */
static void call_ngwin(void (*const fn)(void), struct m7regs* const r)
{
    M7WinAX = r->eax;
    M7WinBX = r->ebx;
    M7WinCX = r->ecx;
    M7WinSI = (u4)(uintptr_t)r->esi;
    M7WinDI = (u4)(uintptr_t)r->edi;
    fn();
    r->eax = M7WinAX;
    r->ebx = M7WinBX;
    r->ecx = M7WinCX;
    r->esi = (u1*)(uintptr_t)M7WinSI;
    r->edi = (u1*)(uintptr_t)M7WinDI;
}

/* seek_tile off the unprefixed positions, for the on-map re-entry. */
static void seek_tile_ng(struct m7regs* const r)
{
    u4 bx = dword_at1(&mode7ypos) << 5;
    u4 ax = dword_at1(&mode7xpos) >> 3;

    bx &= 0x7FF8u;
    ax = set_lo(ax, (u1)((u1)ax << 1));
    bx = set_lo(bx, (u1)ax);
    r->eax = ax;
    r->edi = vram;
    r->ecx = set_hi(r->ecx, 0);
    mmode7ptr = bx;
    r->ecx = set_lo(r->ecx, r->edi[bx]);
    r->ecx <<= 7;
    r->edi += r->ecx;
    r->ebx = bx;
}

void c_Mode7Processngw16b(void)
{
    enum m7_writer const w = (enum m7_writer)M7PWriter;
    u2 const* const pal = (u2 const*)(uintptr_t)M7PBP;
    struct m7regs r;

    m7_load(&r);
    ngwleftb = 256;
    ngwleft = 256;

    if (!(mode7set & 0x80u)) {
        mode7xrpos = mmode7xpos & 0x7FFu;
        mode7yrpos = mmode7ypos & 0x7FFu;
        seek_tile(&r.eax, &r.ebx, &r.ecx, &r.edi);

        /* Both copies: the walk below reads mm7*, the cluster reads m7*. */
        step_offsets(mmode7xadder, mmode7xadd2, mmode7xinc, &mm7xaddof,
            &mm7xaddof2, &r.ecx, &r.edx);
        m7xaddof = mm7xaddof;
        m7xaddof2 = mm7xaddof2;
        step_offsets(mmode7yadder, mmode7yadd2, mmode7yinc, &mm7yaddof,
            &mm7yaddof2, &r.ecx, &r.edx);
        m7yaddof = mm7yaddof;
        m7yaddof2 = mm7yaddof2;
        r.ecx = 0;

        r.eax = mode7xrpos;
        r.ebx = mmode7ptr;
        call_ngwin(c_ProcessMode7ngwin16b, &r);

        while (ngwleftb != 0) {
            do {
                if (byte1(r.eax) & 0xF8u) {
                    r.eax -= mm7xaddof;
                    r.ebx = set_lo(r.ebx, (u1)((u1)r.ebx + mm7xaddof2));
                    if (byte1(r.eax) & 0xF8u) {
                        r.ebx = set_lo(r.ebx, (u1)((u1)r.ebx + mmode7xinc));
                        r.eax -= mmode7xadd2;
                    }
                    r.ecx = (u4)vrama[r.ebx] << 7;
                    r.edi = vrama + r.ecx;
                }
                if (byte1(mode7yrpos) & 0xF8u) {
                    r.edx = mm7yaddof;
                    mode7yrpos += r.edx;
                    r.ebx = set_hi(r.ebx, (u1)((u1)(r.ebx >> 8) - mm7yaddof2));
                    if (byte1(mode7yrpos) & 0xF8u) {
                        r.edx = mmode7yadd2;
                        r.ebx = set_hi(r.ebx, (u1)((u1)(r.ebx >> 8) - mmode7yinc));
                        mode7yrpos += r.edx;
                    }
                    r.ebx &= 0x7FFFu;
                    r.ecx = (u4)vrama[r.ebx] << 7;
                    r.edi = vrama + r.ecx;
                }
                r.ecx = set_hi(r.ecx, byte1(r.eax));
                r.ecx = set_lo(r.ecx, byte1(mode7yrpos));
                mode7yrpos -= mode7yadder;
                r.edx = mode7tab[r.ecx];
                r.eax += mode7xadder;
                r.edx = set_lo(r.edx, r.edi[r.edx]);
                r.edx = m7_write(w, &r.esi, pal, r.edx);
            } while (--ngwleftb != 0);
            mode7xrpos = r.eax;
            if (ngwleft == 0) {
                break;
            }
            call_ngwin(c_ProcessMode7ngwinB16b, &r);
        }
        m7_store(&r);
        return;
    }

    switchtorep3 = 0;
    call_ngwin(c_ProcessMode7ngwinC16b, &r);
    if (ngwleftb == 0) {
        m7_store(&r);
        return;
    }

    if (!(mode7set & 0x40u)) {
        for (;;) {
            while (byte2(mode7ypos) > 3u || byte2(mode7xpos) > 3u) {
                r.eax = mode7xadder;
                r.ebx = mode7yadder;
                mode7xpos += mode7xadder;
                mode7ypos -= mode7yadder;
                r.esi += 2;
                if (--ngwleftb == 0) {
                    goto refill_b2;
                }
            }
            goto onmap_ng;
        refill_b2:
            if (ngwleft == 0) {
                break;
            }
            call_ngwin(c_ProcessMode7ngwinD16b, &r);
            if (ngwleftb == 0) {
                break;
            }
        }
        m7_store(&r);
        return;
    }

    for (;;) {
        while (byte2(mode7ypos) > 3u || byte2(mode7xpos) > 3u) {
            r.ecx = set_hi(r.ecx, byte1(mode7xpos));
            r.eax = mode7xadder;
            r.ecx = set_lo(r.ecx, byte1(mode7ypos));
            r.ebx = mode7yadder;
            r.edx = set_lo(r.edx, mode7tab[r.ecx]);
            mode7xpos += mode7xadder;
            r.edx = set_lo(r.edx, vrama[r.edx]);
            mode7ypos -= mode7yadder;
            r.edx = m7_write(w, &r.esi, pal, r.edx);
            if (--ngwleftb == 0) {
                goto refill_b3;
            }
        }
        goto onmap_ng;
    refill_b3:
        if (ngwleft == 0) {
            break;
        }
        call_ngwin(c_ProcessMode7ngwinD16b, &r);
        if (ngwleftb == 0) {
            break;
        }
    }
    m7_store(&r);
    return;

onmap_ng:
    mode7xrpos = mode7xpos & 0x7FFu;
    mode7yrpos = mode7ypos & 0x7FFu;
    seek_tile_ng(&r);

    for (;;) {
        for (;;) {
            if (byte1(mode7xrpos) & 0xF8u) {
                r.eax = set_lo(r.eax, mmode7xinc);
                r.edi = vram;
                mmode7ptr = set_lo(mmode7ptr, (u1)((u1)mmode7ptr + mmode7xinc));
                r.ecx = set_lo(r.ecx, mmode7xincc);
                if ((u1)mmode7ptr == mmode7xincc) {
                    goto wrapped_ng;
                }
                r.ebx = mmode7ptr;
                r.ecx = (u4)r.edi[r.ebx] << 7;
                r.eax = mmode7xadd2;
                mode7xrpos -= mmode7xadd2;
                r.edi += r.ecx;
                continue; /* %%roffxretb jumps back above the x test */
            }
            if (!(byte1(mode7yrpos) & 0xF8u)) {
                break;
            }
            r.eax = set_lo(r.eax, mmode7yinc);
            r.edi = vram;
            mmode7ptr = set_hi(mmode7ptr, (u1)((u1)(mmode7ptr >> 8) - mmode7yinc));
            if ((u1)(mmode7ptr >> 8) & 0x80u) { /* js */
                goto wrapped_ng;
            }
            r.ebx = mmode7ptr;
            r.ecx = (u4)r.edi[r.ebx] << 7;
            r.eax = mmode7yadd2;
            mode7yrpos += mmode7yadd2;
            r.edi += r.ecx;
        }
        r.ecx = set_lo(r.ecx, byte1(mode7yrpos));
        r.ecx = set_hi(r.ecx, byte1(mode7xrpos));
        mode7xrpos += mode7xadder;
        mode7yrpos -= mode7yadder;
        r.edx = mode7tab[r.ecx];
        r.edx = set_lo(r.edx, r.edi[r.edx]);
        r.edx = m7_write(w, &r.esi, pal, r.edx);
        if (--ngwleftb == 0) {
            if (ngwleft == 0) {
                break;
            }
            call_ngwin(c_ProcessMode7ngwinE16b, &r);
            /* Unreachable in practice: the only writes of switchtorep3 are
               in ProcessMode7ngwinD16b's off-tile handlers, which the
               commented-out tests above them make dead code. Kept because the
               assembly has it. */
            if (switchtorep3 != 0) {
                goto tilerep_ng;
            }
            if (ngwleftb == 0) {
                break;
            }
        }
    }
    m7_store(&r);
    return;

wrapped_ng:
    if (!(mode7set & 0x40u)) {
        m7_store(&r);
        return;
    }
tilerep_ng:
    for (;;) {
        do {
            mode7yrpos &= 0xFFFF07FFu;
            mode7xrpos &= 0xFFFF07FFu;
            r.ecx = set_lo(r.ecx, byte1(mode7yrpos));
            r.eax = mode7xadder;
            r.ecx = set_hi(r.ecx, byte1(mode7xrpos));
            mode7xrpos += mode7xadder;
            r.edx = set_lo(r.edx, mode7tab[r.ecx]);
            r.eax = mode7yadder;
            mode7yrpos -= mode7yadder;
            r.edx = set_lo(r.edx, vrama[r.edx]);
            r.edx = m7_write(w, &r.esi, pal, r.edx);
        } while (--ngwleftb != 0);
        /* Writes eax, which the loop left holding mode7yadder. */
        mode7xrpos = r.eax;
        if (ngwleft == 0) {
            break;
        }
        call_ngwin(c_ProcessMode7ngwinB16b, &r);
        if (ngwleftb == 0) {
            break;
        }
    }
    m7_store(&r);
}

/* --- Mode7Processngw216b -------------------------------------------------- *
 *
 * The main/sub window variant: it walks the whole 256 pixels but alternates
 * between two pixel writers, switching every time a window run ends. The
 * assembly writes that out as two mirrored copies of the entire body (the
 * %%...w labels), which here is one body and a `side` index.
 *
 * Two counters run at once: mtemp is the pixels left on the line and ends the
 * routine, ngwleft is the pixels left in the current run and only switches
 * sides. The window list is read directly rather than through the ngwin
 * cluster.
 */

/* Step to the next run. The assembly clears edx here every time. */
static void next_run(struct m7regs* const r)
{
    ngcwinptr++;
    ngwleft = *ngcwinptr;
    r->edx = 0;
}

void c_Mode7Processngw216b(void)
{
    enum m7_writer const wr[2] = { (enum m7_writer)M7PWriter,
        (enum m7_writer)M7PWriter2 };
    u2 const* const pal = (u2 const*)(uintptr_t)M7PBP;
    struct m7regs r;
    int side;

    m7_load(&r);
    ngwleft = *ngcwinptr;
    mtemp = 256;

    if (!(mode7set & 0x80u)) {
        mmode7xrpos = mmode7xpos & 0x7FFu;
        mmode7yrpos = mmode7ypos & 0x7FFu;
        seek_tile(&r.eax, &r.ebx, &r.ecx, &r.edi);
        r.eax = mmode7xrpos;
        r.ebx = mmode7ptr;

        if (ngwleft != 0) {
            side = 0;
        } else {
            next_run(&r);
            side = 1;
        }
        for (;;) {
            if (byte1(r.eax) & 0x08u) {
                r.ebx = set_lo(r.ebx, (u1)((u1)r.ebx + mmode7xinc));
                r.ecx = (u4)vrama[r.ebx] << 7;
                r.eax -= mmode7xadd2;
                r.edi = vrama + r.ecx;
            }
            if (byte1(mmode7yrpos) & 0x08u) {
                r.ebx = set_hi(r.ebx, (u1)((u1)(r.ebx >> 8) - mmode7yinc));
                r.ebx &= 0x7FFFu;
                r.ecx = (u4)vrama[r.ebx] << 7;
                mmode7yrpos += mmode7yadd2;
                r.edi = vrama + r.ecx;
            }
            r.ecx = set_hi(r.ecx, byte1(r.eax));
            r.ecx = set_lo(r.ecx, byte1(mmode7yrpos));
            mmode7yrpos -= mmode7yadder;
            r.edx = mode7tab[r.ecx];
            r.eax += mmode7xadder;
            r.edx = set_lo(r.edx, r.edi[r.edx]);
            r.edx = m7_write(wr[side], &r.esi, pal, r.edx);
            if (--mtemp == 0) {
                goto done2;
            }
            if (--ngwleft == 0) {
                next_run(&r);
                side ^= 1;
            }
        }
    }

    if (ngwleft != 0) {
        side = 0;
    } else {
        next_run(&r);
        side = 1;
    }

norep:
    if (!(mode7set & 0x40u)) {
        while (byte2(mmode7ypos) > 3u || byte2(mmode7xpos) > 3u) {
            r.eax = mmode7xadder;
            r.ebx = mmode7yadder;
            mmode7xpos += mmode7xadder;
            mmode7ypos -= mmode7yadder;
            r.esi += 2;
            if (--mtemp == 0) {
                goto done2;
            }
            if (--ngwleft == 0) {
                next_run(&r);
                side ^= 1;
                goto norep;
            }
        }
    } else {
        while (byte2(mmode7ypos) > 3u || byte2(mmode7xpos) > 3u) {
            r.ecx = set_hi(r.ecx, byte1(mmode7xpos));
            r.eax = mmode7xadder;
            r.ecx = set_lo(r.ecx, byte1(mmode7ypos));
            r.ebx = mmode7yadder;
            r.edx = set_lo(r.edx, mode7tab[r.ecx]);
            mmode7xpos += mmode7xadder;
            r.edx = set_lo(r.edx, vrama[r.edx]);
            mmode7ypos -= mmode7yadder;
            r.edx = m7_write(wr[side], &r.esi, pal, r.edx);
            if (--mtemp == 0) {
                goto done2;
            }
            if (--ngwleft == 0) {
                /* Both mirrors of this arm jump to %%firstsetw, so unlike
                   every other run boundary this one always lands on side 0
                   rather than toggling. */
                next_run(&r);
                side = 0;
                goto norep;
            }
        }
    }

    mmode7xrpos = mmode7xpos & 0x7FFu;
    mmode7yrpos = mmode7ypos & 0x7FFu;
    seek_tile(&r.eax, &r.ebx, &r.ecx, &r.edi);

    for (;;) {
        if (byte1(mmode7xrpos) & 0x08u) {
            r.eax = set_lo(r.eax, mmode7xinc);
            r.edi = vram;
            mmode7ptr = set_lo(mmode7ptr, (u1)((u1)mmode7ptr + mmode7xinc));
            r.ecx = set_lo(r.ecx, mmode7xincc);
            if ((u1)mmode7ptr == mmode7xincc) {
                goto wrapped2;
            }
            r.ebx = mmode7ptr;
            r.ecx = (u4)r.edi[r.ebx] << 7;
            r.eax = mmode7xadd2;
            mmode7xrpos -= mmode7xadd2;
            r.edi += r.ecx;
        }
        if (byte1(mmode7yrpos) & 0x08u) {
            r.eax = set_lo(r.eax, mmode7yinc);
            r.edi = vram;
            mmode7ptr = set_hi(mmode7ptr, (u1)((u1)(mmode7ptr >> 8) - mmode7yinc));
            if ((u1)(mmode7ptr >> 8) & 0x80u) { /* js */
                goto wrapped2;
            }
            r.ebx = mmode7ptr;
            r.ecx = (u4)r.edi[r.ebx] << 7;
            r.eax = mmode7yadd2;
            mmode7yrpos += mmode7yadd2;
            r.edi += r.ecx;
        }
        r.ecx = set_lo(r.ecx, byte1(mmode7yrpos));
        r.ecx = set_hi(r.ecx, byte1(mmode7xrpos));
        mmode7xrpos += mmode7xadder;
        mmode7yrpos -= mmode7yadder;
        r.edx = mode7tab[r.ecx];
        r.edx = set_lo(r.edx, r.edi[r.edx]);
        r.edx = m7_write(wr[side], &r.esi, pal, r.edx);
        if (--mtemp == 0) {
            goto done2;
        }
        if (--ngwleft == 0) {
            next_run(&r);
            side ^= 1;
        }
    }

wrapped2:
    if (!(mode7set & 0x40u)) {
        goto done2;
    }
    for (;;) {
        mmode7yrpos &= 0xFFFF07FFu;
        mmode7xrpos &= 0xFFFF07FFu;
        r.ecx = set_lo(r.ecx, byte1(mmode7yrpos));
        r.eax = mmode7xadder;
        r.ecx = set_hi(r.ecx, byte1(mmode7xrpos));
        mmode7xrpos += mmode7xadder;
        r.edx = set_lo(r.edx, mode7tab[r.ecx]);
        r.eax = mmode7yadder;
        mmode7yrpos -= mmode7yadder;
        r.edx = set_lo(r.edx, vrama[r.edx]);
        r.edx = m7_write(wr[side], &r.esi, pal, r.edx);
        if (--mtemp == 0) {
            goto done2;
        }
        if (--ngwleft == 0) {
            next_run(&r);
            side ^= 1;
        }
    }

done2:
    m7_store(&r);
}
