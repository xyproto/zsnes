/*
 * Mode 7 16-bit background renderers, ported from mode716b.asm,
 * mode716d.asm, mode716t.asm, and makev16b.asm.
 *
 * The drawmode7 entry points keep the legacy register ABI (y scroll
 * in AX, x scroll in DX) through i386 trampolines; the asm renderers
 * tail-jump to domosaic16b with DH holding curmosaicsz, so the C
 * version reads the global instead.
 *
 * Positions are 24.8 fixed point kept in 32-bit words.  The asm mixed
 * byte, word, and dword accesses into those words; the masked helpers
 * below reproduce its dropped carries exactly.
 */

#include <string.h>

#include "../cpu/regs.h"
#include "../endmem.h"
#include "../types.h"
#include "../ui.h"
#include "../vcache.h"
#include "c_newgfx16.h"
#include "makev16b.h"
#include "makev16t.h"
#include "makevid.h"
#include "mode716.h"
#include "mode716b.h"
#include "mode716d.h"
#include "mode716t.h"
#include "newgfx16.h"

u1 tileleft16b;

/* how plot() colors a pixel; the transp modes blend with the sub
   screen previously rendered into transpbuf */
enum m7mode {
    M7_PAL,
    M7_DCOLOR,
    M7_HALFADD,
    M7_FULLADD,
    M7_FULLSUB,
    M7_MAINSUB,
};

/* mode 7 line state, mirrors the asm scratch words */
static u1 m7mode;
static u2* m7transp; // sub screen pixel cursor, in step with dest
static u4 m7xpos;
static u4 m7ypos;
static s4 m7xadder;
static s4 m7yadder;
static s4 m7xadd2;
static s4 m7yadd2;
static u1 m7xinc;
static u1 m7xincc;
static u1 m7yinc;

/* 13-bit signed register value -> 16-bit */
static u2 conv13(u2 v)
{
    v &= 0x1FFF;
    if (v & 0x1000)
        v |= 0xE000;
    return v;
}

/* clip a screen delta to 10 bits, keeping far negatives negative */
static u2 clip10(u2 v)
{
    return v & 0x2000 ? v | 0xFC00 : v & 0x3FF;
}

/* word add at byte offset 1: bytes 0 and 3 untouched, carry dropped */
static u4 addw1(u4 v, u2 w)
{
    return (v & 0xFF0000FFu) | ((((v >> 8) + w) & 0xFFFFu) << 8);
}

static void mode7calculate(u2 y, u2 x)
{
    u2 const cx = clip10((u2)(conv13(x) - conv13(mode7X0)));
    u2 const cy = clip10((u2)(conv13(y) - conv13(mode7Y0)));

    m7xpos = (u4)((s4)(s2)mode7B * (s2)cy) & ~63u;
    m7xpos = addw1(m7xpos, conv13(mode7X0));
    m7xpos += (u4)((s4)(s2)mode7B * m7starty) & ~63u;

    m7ypos = (u4)((s4)(s2)mode7D * (s2)cy) & ~63u;
    m7ypos = addw1(m7ypos, conv13(mode7Y0));
    m7ypos += (u4)((s4)(s2)mode7D * m7starty) & ~63u;

    m7xadder = (s2)mode7A;
    m7xpos += (u4)((s4)(s2)mode7A * (s2)cx) & ~63u;

    m7yadder = -(s4)(s2)mode7C;
    m7ypos += (u4)((s4)(s2)mode7C * (s2)cx) & ~63u;

    if (mode7set & 0x01) { // horizontal flip
        m7xpos += (u4)m7xadder << 8;
        m7xadder = -m7xadder;
        m7ypos -= (u4)m7yadder << 8;
        m7yadder = -m7yadder;
    }
}

/* win and transp move only when a pixel slot is consumed here,
   matching the asm */
static void plot(u2** dest, u1 const** win, u1 pix)
{
    if (pix != 0 && (*win == NULL || **win == 0)) {
        switch (m7mode) {
        case M7_PAL:
            **dest = (u2)pal16b[pix];
            break;
        case M7_DCOLOR:
            **dest = dcolortab[0][pix];
            break;
        case M7_HALFADD: {
            u4 c = pal16bcl[pix];
            if (*m7transp != 0)
                c = ((c & vesa2_clbit) + (*m7transp & vesa2_clbit)) >> 1;
            **dest = (u2)c;
            break;
        }
        case M7_FULLADD:
            **dest
                = fulladdtab[(pal16bcl[pix] + (*m7transp & vesa2_clbit)) >> 1];
            break;
        case M7_FULLSUB:
            **dest = (u2)(fulladdtab[(pal16bxcl[pix] + (*m7transp & vesa2_clbit)) >> 1]
                ^ 0xFFFF);
            break;
        case M7_MAINSUB:
            *m7transp = **dest = (u2)pal16b[pix];
            break;
        }
    }
    ++*dest;
    ++m7transp;
    if (*win != NULL)
        ++*win;
}

/* tile map offset and tile data pointer for the current position */
static u4 tileptr(void)
{
    u4 ptr = ((m7ypos >> 8) << 5) & 0x7FF8;
    return (ptr & ~0xFFu) | (u1)((m7xpos >> 11) << 1);
}

static void finishline(void)
{
    if (curmosaicsz != 1)
        domosaic16b();
}

/* per-pixel steps stay within one tile: adders are within +/-0x7F0 */
static void process(u2* dest, u1 const* win)
{
    u4 count = 256;

    if (!(mode7set & 0x80)) { // repeating map
        u4 xr = m7xpos & 0x7FF;
        u4 yr = m7ypos & 0x7FF;
        u4 ptr = tileptr();
        u1 const* tiled = vram + vram[ptr] * 128;

        for (;;) {
            if ((xr >> 8) & 0x08) {
                ptr = (ptr & ~0xFFu) | ((ptr + m7xinc) & 0xFF);
                tiled = vrama + vrama[ptr] * 128;
                xr -= (u4)m7xadd2;
            }
            if ((yr >> 8) & 0x08) {
                ptr = (ptr & ~0xFF00u) | ((((ptr >> 8) - m7yinc) & 0xFF) << 8);
                ptr &= 0x7FFF;
                tiled = vrama + vrama[ptr] * 128;
                yr += (u4)m7yadd2;
            }
            u1 const pix
                = tiled[mode7tab[(((xr >> 8) & 0xFF) << 8) | ((yr >> 8) & 0xFF)]];
            yr -= (u4)m7yadder;
            xr += (u4)m7xadder;
            plot(&dest, &win, pix);
            if (--count == 0)
                break;
        }
        finishline();
        return;
    }

    // non-repeating map: skip or tile-0-fill until the map is entered
    for (;;) {
        if (((m7ypos >> 16) & 0xFF) <= 3 && ((m7xpos >> 16) & 0xFF) <= 3)
            break;
        if (mode7set & 0x40) {
            u4 const idx
                = (((m7xpos >> 8) & 0xFF) << 8) | ((m7ypos >> 8) & 0xFF);
            u1 const pix = vrama[mode7tab[idx]];
            m7xpos += (u4)m7xadder;
            m7ypos -= (u4)m7yadder;
            plot(&dest, &win, pix);
        } else {
            m7xpos += (u4)m7xadder;
            m7ypos -= (u4)m7yadder;
            ++dest;
        }
        if (--count == 0) {
            finishline();
            return;
        }
    }

    u4 xr = m7xpos & 0x7FF;
    u4 yr = m7ypos & 0x7FF;
    u4 ptr = tileptr();
    u1 const* tiled = vram + vram[ptr] * 128;

    for (;;) {
        if ((xr >> 8) & 0x08) {
            ptr = (ptr & ~0xFFu) | ((ptr + m7xinc) & 0xFF);
            if ((ptr & 0xFF) == m7xincc)
                goto offmap;
            tiled = vram + vram[ptr] * 128;
            xr -= (u4)m7xadd2;
        }
        if ((yr >> 8) & 0x08) {
            u1 const row = (u1)((ptr >> 8) - m7yinc);
            ptr = (ptr & ~0xFF00u) | ((u4)row << 8);
            if (row & 0x80)
                goto offmap;
            tiled = vram + vram[ptr] * 128;
            yr += (u4)m7yadd2;
        }
        u4 const idx = (((xr >> 8) & 0xFF) << 8) | ((yr >> 8) & 0xFF);
        xr += (u4)m7xadder;
        yr -= (u4)m7yadder;
        plot(&dest, &win, tiled[mode7tab[idx]]);
        if (--count == 0) {
            finishline();
            return;
        }
    }

offmap:
    if (mode7set & 0x40) {
        do { // tile 0 repeats outside the map
            xr &= 0xFFFF07FFu;
            yr &= 0xFFFF07FFu;
            u4 const idx = (((xr >> 8) & 0xFF) << 8) | ((yr >> 8) & 0xFF);
            xr += (u4)m7xadder;
            yr -= (u4)m7yadder;
            plot(&dest, &win, vrama[mode7tab[idx]]);
        } while (--count != 0);
    }
    finishline();
}

/* per-pixel steps may cross several tiles: adders beyond +/-0x7F0 */
static void process_b(u2* dest, u1 const* win)
{
    u4 count = 256;

    if (!(mode7set & 0x80)) { // repeating map
        u4 xr = m7xpos & 0x7FF;
        u4 yr = m7ypos & 0x7FF;
        u4 ptr = tileptr();
        u1 const* tiled = vram + vram[ptr] * 128;

        // whole-tile part of one step, applied in a single correction
        u4 xaddofa = 0;
        u1 xaddof2a = 0;
        for (s4 b = m7xadder < 0 ? -m7xadder : m7xadder; b >= 0x800;
            b -= 0x800) {
            xaddofa += (u4)m7xadd2;
            xaddof2a += m7xinc;
        }
        u4 yaddofa = 0;
        u1 yaddof2a = 0;
        for (s4 b = m7yadder < 0 ? -m7yadder : m7yadder; b >= 0x800;
            b -= 0x800) {
            yaddofa += (u4)m7yadd2;
            yaddof2a += m7yinc;
        }

        for (;;) {
            if ((xr >> 8) & 0xF8) {
                xr -= xaddofa;
                ptr = (ptr & ~0xFFu) | ((ptr + xaddof2a) & 0xFF);
                if ((xr >> 8) & 0xF8) {
                    ptr = (ptr & ~0xFFu) | ((ptr + m7xinc) & 0xFF);
                    xr -= (u4)m7xadd2;
                }
                tiled = vrama + vrama[ptr] * 128;
            }
            if ((yr >> 8) & 0xF8) {
                ptr = (ptr & ~0xFF00u)
                    | ((((ptr >> 8) - yaddof2a) & 0xFF) << 8);
                yr += yaddofa;
                if ((yr >> 8) & 0xF8) {
                    ptr = (ptr & ~0xFF00u)
                        | ((((ptr >> 8) - m7yinc) & 0xFF) << 8);
                    yr += (u4)m7yadd2;
                }
                ptr &= 0x7FFF;
                tiled = vrama + vrama[ptr] * 128;
            }
            u1 const pix
                = tiled[mode7tab[(((xr >> 8) & 0xFF) << 8) | ((yr >> 8) & 0xFF)]];
            yr -= (u4)m7yadder;
            xr += (u4)m7xadder;
            plot(&dest, &win, pix);
            if (--count == 0)
                break;
        }
        finishline();
        return;
    }

    // non-repeating map
    for (;;) {
        if (((m7ypos >> 16) & 0xFF) <= 3 && ((m7xpos >> 16) & 0xFF) <= 3)
            break;
        if (mode7set & 0x40) {
            u4 const idx
                = (((m7xpos >> 8) & 0xFF) << 8) | ((m7ypos >> 8) & 0xFF);
            u1 const pix = vrama[mode7tab[idx]];
            m7xpos += (u4)m7xadder;
            m7ypos -= (u4)m7yadder;
            plot(&dest, &win, pix);
        } else {
            m7xpos += (u4)m7xadder;
            m7ypos -= (u4)m7yadder;
            ++dest;
        }
        if (--count == 0) {
            finishline();
            return;
        }
    }

    u4 xr = m7xpos & 0x7FF;
    u4 yr = m7ypos & 0x7FF;
    u4 ptr = tileptr();
    u1 const* tiled = vram + vram[ptr] * 128;

    for (;;) {
        while ((xr >> 8) & 0xF8) {
            ptr = (ptr & ~0xFFu) | ((ptr + m7xinc) & 0xFF);
            if ((ptr & 0xFF) == m7xincc)
                goto offmap;
            tiled = vram + vram[ptr] * 128;
            xr -= (u4)m7xadd2;
        }
        while ((yr >> 8) & 0xF8) {
            u1 const row = (u1)((ptr >> 8) - m7yinc);
            ptr = (ptr & ~0xFF00u) | ((u4)row << 8);
            if (row & 0x80)
                goto offmap;
            tiled = vram + vram[ptr] * 128;
            yr += (u4)m7yadd2;
        }
        u4 const idx = (((xr >> 8) & 0xFF) << 8) | ((yr >> 8) & 0xFF);
        xr += (u4)m7xadder;
        yr -= (u4)m7yadder;
        plot(&dest, &win, tiled[mode7tab[idx]]);
        if (--count == 0) {
            finishline();
            return;
        }
    }

offmap:
    if (mode7set & 0x40) {
        do {
            xr &= 0xFFFF07FFu;
            yr &= 0xFFFF07FFu;
            u4 const idx = (((xr >> 8) & 0xFF) << 8) | ((yr >> 8) & 0xFF);
            xr += (u4)m7xadder;
            yr -= (u4)m7yadder;
            plot(&dest, &win, vrama[mode7tab[idx]]);
        } while (--count != 0);
    }
    finishline();
}

static void renderline(u2 ypos, u2 xpos)
{
    winptrref = cwinptr;
    mode7calculate(ypos, xpos);
    m7transp = (u2*)(transpbuf + 32);

    u2* dest = (u2*)curvidoffset;
    if (curmosaicsz != 1) {
        memset((u1*)xtravbuf + 32, 0, 512);
        dest = xtravbuf + 16;
    }

    m7xadd2 = 0x800;
    m7xinc = 2;
    m7xincc = 0;
    if (m7xadder < 0) {
        m7xadd2 = -0x800;
        m7xinc = 0xFE;
        m7xincc = 0xFE;
    }
    m7yadd2 = 0x800;
    m7yinc = 1;
    if (m7yadder < 0) {
        m7yadd2 = -0x800;
        m7yinc = 0xFF;
    }

    if (m7xadder > 0x7F0 || m7xadder < -0x7F0 || m7yadder > 0x7F0
        || m7yadder < -0x7F0) {
        process_b(dest, NULL); // windowing skipped here, as in the asm
    } else if (curmosaicsz == 1 && winon != 0) {
        process(dest, cwinptr);
    } else {
        process(dest, NULL);
    }
}

void c_drawmode7dcolor(u4 ypos, u4 xpos)
{
    if (scrndis & 0x01)
        return;
    if (vidbright != prevbrightdc) {
        prevbrightdc = vidbright;
        Gendcolortable();
    }
    m7mode = M7_DCOLOR;
    renderline((u2)ypos, (u2)xpos);
}

void c_drawmode716b(u4 ypos, u4 xpos)
{
    if (scrndis & 0x01)
        return;
    if (scaddset & 0x01) {
        c_drawmode7dcolor(ypos, xpos);
        return;
    }
    m7mode = M7_PAL;
    renderline((u2)ypos, (u2)xpos);
}

void c_drawmode716t(u4 ypos, u4 xpos)
{
    if (scrndis & 0x01)
        return;
    if (scaddset & 0x01) {
        c_drawmode7dcolor(ypos, xpos);
        return;
    }
    if (DoTransp == 1) {
        c_drawmode716b(ypos, xpos);
        return;
    }
    if (scaddtype & 0x80)
        m7mode = M7_FULLSUB;
    else if ((scaddtype & 0x40) && (u1)(scrnon >> 8) != 0
        && (coladdr | coladdg | coladdb | colnull) == 0)
        m7mode = M7_HALFADD; // the asm checks coladdr..colnull as one dword
    else
        m7mode = M7_FULLADD;
    renderline((u2)ypos, (u2)xpos);
}

void c_drawmode716tb(u4 ypos, u4 xpos)
{
    if (scrndis & 0x01)
        return;
    m7mode = M7_MAINSUB;
    renderline((u2)ypos, (u2)xpos);
}

/* expand the first pixel of each block; the first 0x200 bytes of the
   line were rendered into xtravbuf */
void domosaic16b(void)
{
    u2 const* src = xtravbuf + 16;
    u2* dst = (u2*)curvidoffset;
    u1 const* win = winptrref;
    u1 const size = curmosaicsz;
    u1 blockleft = size;
    u2 c = *src;

    for (u4 left = 256;;) {
        if (c != 0 && (winon == 0 || *win == 0))
            *dst = c;
        ++src;
        ++dst;
        ++win;
        if (--left == 0)
            break;
        if (--blockleft == 0) {
            c = *src;
            blockleft = size;
        }
    }
}

#if defined(__GNUC__) && defined(__i386__)

#if defined(__APPLE__) || defined(__MINGW32__)
#define CSYM(x) "_" #x
#else
#define CSYM(x) #x
#endif

__asm__(
    ".globl " CSYM(drawmode716b) "\n" CSYM(drawmode716b) ":\n"
                                                         "pushl %edx\n"
                                                         "pushl %eax\n"
                                                         "call " CSYM(c_drawmode716b) "\n"
                                                                                      "addl $8, %esp\n"
                                                                                      "ret\n");

__asm__(
    ".globl " CSYM(drawmode7dcolor) "\n" CSYM(drawmode7dcolor) ":\n"
                                                               "pushl %edx\n"
                                                               "pushl %eax\n"
                                                               "call " CSYM(c_drawmode7dcolor) "\n"
                                                                                               "addl $8, %esp\n"
                                                                                               "ret\n");

__asm__(
    ".globl " CSYM(drawmode716t) "\n" CSYM(drawmode716t) ":\n"
                                                         "pushl %edx\n"
                                                         "pushl %eax\n"
                                                         "call " CSYM(c_drawmode716t) "\n"
                                                                                      "addl $8, %esp\n"
                                                                                      "ret\n");

__asm__(
    ".globl " CSYM(drawmode716tb) "\n" CSYM(drawmode716tb) ":\n"
                                                           "pushl %edx\n"
                                                           "pushl %eax\n"
                                                           "call " CSYM(c_drawmode716tb) "\n"
                                                                                         "addl $8, %esp\n"
                                                                                         "ret\n");

#endif
