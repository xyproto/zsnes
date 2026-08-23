/*
 * video/c_ngprocbg.c - the background, sprite and Mode 7 passes of
 * StartDrawNewGfx16b in video/newgfx16.asm.
 *
 * Five macros - Procbgpr016b, Procbg3pr016b, Procbgpr116b, Procbg3pr116b and
 * Procbg3pr1b16b - instantiated sixteen times between them, once per layer per
 * screen. They are one walk down the scanlines that decides, per line, whether
 * the layer can be drawn eight lines at a time or has to go line by line, and
 * they differ only in that decision:
 *
 *   pr0      tile-aligned, and nothing on these lines changed, and the mode is
 *            not 2 or 4-and-up
 *   bg3pr0   the same, but keyed on BG3's priority being steady instead
 *   pr1      whatever the priority-1 flag for this line says
 *   bg3pr1   the same, plus: skip the line in mode 1 when BG3 has priority
 *   bg3pr1b  the same, but draw *only* those lines
 *
 * The line and tile renderers are still assembly and take the scanline in ebx
 * and the video pointer in esi, so they are reached through calldl16t, the
 * register-block shim in video/makev16t.asm.
 */
#include <stdint.h>
#include <string.h>

#include "../types.h"
#include "c_mode716gate.h"
#include "makevid.h"

/* calldl16t's register block, defined in video/c_mv16tline.c. */
extern u4 DLR[7];
extern void (*DLFN)(void);
extern void calldl16t(void);

extern u1 BGFB[256], BGMA[256], BG3PRI[256];
extern u1 sprtlng[256], sprlefttot[256], sprleftpr[], SpecialLine[256];
extern u4 csprival;
extern u1 winon, intrlng[256], scadsng[256], vidbright, prevbrightdc;
extern u1 Mode7HiRes16b, scanlines, mode7set;
extern u2 BG1SXl[256], m7starty;
extern u4 mode7A, mode7C, mode7X0;
extern u4 Mode7BackA, Mode7BackC, Mode7BackX0, Mode7BackSet;
extern u4 mode7ab[256], mode7cd[256], mode7xy[256];
extern u1 mode7st[256];
extern u4 dcolortab[];
extern void Gendcolortable(void);

/* The mode 7 renderers are C now, so they take the register block instead of
   the registers themselves - but they still ride on DLR, which carries the
   scanline loop's edi and ebp from one call to the next the way the assembly
   did. */
static void m7call(void (*const g)(m7regs*))
{
    m7regs r;

    r.ax = DLR[0];
    r.bx = DLR[1];
    r.cx = DLR[2];
    r.dx = DLR[3];
    r.si = DLR[4];
    r.di = DLR[5];
    r.bp = DLR[6];
    g(&r);
    DLR[0] = (u4)r.ax;
    DLR[1] = (u4)r.bx;
    DLR[2] = (u4)r.cx;
    DLR[3] = (u4)r.dx;
    DLR[4] = (u4)r.si;
    DLR[5] = (u4)r.di;
    DLR[6] = (u4)r.bp;
}
extern void drawsprng16b(void);
extern void drawsprng16bhr(void);
extern u1 BGMS1[], FillSubScr[256];
extern u1 bgwinchange[256], bgallchange[256], bg1change[256];
extern u1 winbg1enval[256], mosenng[256], mosszng[256];
extern u2 BG1SYl[256];
extern u4 cpalval[256], CPalPtrng;
extern u4 startlinet, endlinet, reslbyl;
extern u1 moscountdown;
extern u4 mosstart[4];
extern u2 resolutn;
extern u1* vidbuffer;

enum {
    P_PR0,
    P_BG3PR0,
    P_PR1,
    P_BG3PR1,
    P_BG3PR1B
};

/* A dword read at a byte offset into one of the change maps. Several of these
   run off the end of their 256-byte array by up to seven bytes and pick up
   whatever follows; that is what the assembly does. */
static u4 dw(u1 const* const p, u4 const i)
{
    u4 v;

    memcpy(&v, p + i, 4);
    return v;
}

/* mosaic_countdown: the first line of a block latches where it started and how
   long it runs. */
static void countdown(u4 const layer, u4 const y)
{
    if (moscountdown == 0 || y == 1) {
        mosstart[layer] = y;
        moscountdown = (u1)(mosszng[y] + 1);
    }
    moscountdown--;
}

/* ngsub / ngmain: is this layer on the screen being drawn? */
static int on_screen(int const main_, u4 const mask, u4 const y)
{
    if (main_) {
        if (BGMS1[y * 2] & mask) {
            return 1;
        }
        if (!(FillSubScr[y] & 1)) {
            return 0;
        }
        return (BGMS1[y * 2 + 1] & mask) != 0;
    }
    if (!(BGMS1[y * 2 + 1] & mask)) {
        return 0;
    }
    if (BGMS1[y * 2] & mask) {
        return 0;
    }
    return !(FillSubScr[y] & 1);
}

/* Can this line and the seven after it be drawn as whole tiles? Only the two
   priority-0 forms ask; the priority-1 ones read a flag instead. */
static int tile_ok(u4 const layer, u4 const y, int const kind)
{
    if (startlinet >= y || endlinet <= y) {
        return 0;
    }
    if ((dw(bgwinchange, y) & 0xFFFFFF00u) || dw(bgwinchange, y + 4)) {
        /* The window changed somewhere in these eight lines, so it is only
           safe if this layer's window is off for all of them. */
        if (dw(winbg1enval, y + layer * 256u) & 0x0A0A0A0Au) {
            return 0;
        }
        if (dw(winbg1enval, y + layer * 256u + 4u) & 0x0A0A0A0Au) {
            return 0;
        }
    }
    if (y >= reslbyl) {
        return 0;
    }
    if ((dw(bgallchange, y) & 0xFFFFFF00u) || dw(bgallchange, y + 4)) {
        return 0;
    }
    if ((dw(bg1change, y + layer * 256u) & 0xFFFFFF00u)
        || dw(bg1change, y + layer * 256u + 4u)) {
        return 0;
    }
    if (kind == P_BG3PR0) {
        if (dw(BG3PRI, y) != dw(BG3PRI, y + 4)) {
            return 0;
        }
    } else if (BGMA[y] >= 4 || BGMA[y] == 2) {
        return 0;
    }
    if ((mosenng[y] & (1u << layer)) && mosszng[y] != 0) {
        return 0;
    }
    /* The last eight lines never go tile by tile. */
    return (u2)y < (u2)(resolutn - 8u);
}

/* The two bg3 priority-1 forms gate the whole line on BG3's priority in mode
   1 - one draws every line except those, the other only those. */
static int line_wanted(u4 const y, int const kind)
{
    int const bg3hi = BGMA[y] == 1 && BG3PRI[y] != 0;

    if (kind == P_BG3PR1) {
        return !bg3hi;
    }
    if (kind == P_BG3PR1B) {
        return bg3hi;
    }
    return 1;
}

/* The renderers take the scanline in ebx and the video pointer in esi, and the
   priority-0 pair also read ecx - it still holds the scroll-plus-line value the
   tile-alignment test computed. The priority-1 pair never look at it. */
static void call_proc(void (*fn)(void), u1 const* const esi, u4 const y,
    u4 const ecx)
{
    DLR[1] = y;
    DLR[2] = ecx;
    DLR[4] = (u4)(uintptr_t)esi;
    DLFN = fn;
    calldl16t();
}

void c_procbg16b(u4 layer, void (*lineproc)(void), void (*tileproc)(void),
    u1 const* prdat, int main_, u4 mask, int kind);

void c_procbg16b(u4 const layer, void (*const lineproc)(void),
    void (*const tileproc)(void), u1 const* const prdat, int const main_,
    u4 const mask, int const kind)
{
    u1 const* esi = vidbuffer + 32;
    u4 y = 0;

    for (;;) {
        u4 adv = 1;

        countdown(layer, y);
        if (BGFB[y] == 0 && on_screen(main_, mask, y) && line_wanted(y, kind)) {
            u4 ecx = DLR[2];
            int tile;

            if (kind == P_PR0 || kind == P_BG3PR0) {
                /* Only a tile-aligned line can start a tile row, and a line
                   that cannot goes down one at a time rather than being
                   skipped. */
                ecx = (BG1SYl[y + layer * 256u] & 0xFFFFu) + y;
                tile = (ecx & 7u) == 0 && tile_ok(layer, y, kind);
            } else {
                tile = prdat[y] == 1;
            }
            if (tile) {
                call_proc(tileproc, esi, y, ecx);
                adv = 8;
            } else {
                CPalPtrng = cpalval[y];
                call_proc(lineproc, esi, y, ecx);
            }
        }
        y += adv;
        esi += 576u * adv;
        if (resolutn < (u2)y) {
            return;
        }
    }
}

/* Procsprng0116b, Procsprng23456716b and Procsprng16b: one sprite priority
   pass down the scanlines, from line 1. The three are the same walk and differ
   only in which background modes they run on.

   csprival and the sprtlng bump happen for every line that passes the screen
   test, whether or not a sprite is then drawn. The renderer takes the scanline
   in ebx, the video pointer in esi and the sprite count in cl. */
enum {
    S_MODE01,
    S_MODE27,
    S_ALL
};

void c_procspr16b(int main_, u4 mask, int modes);

void c_procspr16b(int const main_, u4 const mask, int const modes)
{
    u1 const* esi = vidbuffer + 32 + 576;
    u4 y = 1;

    for (;;) {
        int mode_ok = modes == S_ALL || (modes == S_MODE01 ? BGMA[y] <= 1 : BGMA[y] > 1);

        if (BGFB[y] == 0 && on_screen(main_, mask, y) && mode_ok) {
            u4 const pri = sprtlng[y];
            u1 const count = sprlefttot[y];

            csprival = pri;
            sprtlng[y]++;
            if ((sprleftpr[y * 4 + pri] & 1) && count != 0) {
                DLR[1] = y;
                DLR[2] = (DLR[2] & ~0xFFu) | count;
                DLR[4] = (u4)(uintptr_t)esi;
                DLFN = (SpecialLine[y] & 2) ? drawsprng16bhr : drawsprng16b;
                calldl16t();
            }
        }
        y++;
        esi += 576u;
        if (resolutn < (u2)y) {
            return;
        }
    }
}

/* ProcMode7ng16b, ProcMode7ngextbg16b and ProcMode7ngextbg216b: the Mode 7
   line pass. All three save the matrix, walk the lines feeding it the
   per-line values, and put it back; they differ in which renderer they call,
   whether they want the interlace bit set or clear, and one flag byte.

   Note the renderers are handed ebx = the *unsnapped* line for the plain form
   but the mosaic-snapped one for the two extbg forms - the plain form pops it
   back before the call and they do not. */
enum {
    M7_PLAIN,
    M7_EXTBG,
    M7_EXTBG2
};

void c_procmode7ng16b(int main_, u4 mask, int kind);

void c_procmode7ng16b(int const main_, u4 const mask, int const kind)
{
    u1* esi = vidbuffer + 32 + 576;
    u4 y = 1;

    winon = 0;
    Mode7BackA = mode7A;
    Mode7BackC = mode7C;
    Mode7BackX0 = mode7X0;
    Mode7BackSet = mode7set;
    for (;;) {
        if (kind == M7_EXTBG) {
            /* A per-line flag parked in the line's left margin. Only the
               dead half of ProcMode7ngextbg216b ever read it. */
            esi[-1] = 0;
        }
        if (BGFB[y] == 0 && BGMA[y] == 7 && on_screen(main_, mask, y)
            && (kind == M7_PLAIN ? !(intrlng[y] & 0x40u)
                                 : (intrlng[y] & 0x40u) != 0)) {
            u4 line = y;
            u4 eax, edx;

            if (kind == M7_EXTBG) {
                esi[-1] = 1;
            }
            DLR[6] = cpalval[y];
            mode7A = mode7ab[y];
            mode7C = mode7cd[y];
            mode7X0 = mode7xy[y];
            mode7set = mode7st[y];
            curmosaicsz = 1;
            if (mosenng[y] & 1u) {
                u1 const sz = mosszng[y];

                if (sz != 0) {
                    u1 const n = (u1)(sz + 1u);

                    curmosaicsz = n;
                    /* 8-bit divide then multiply: the start of this mosaic
                       block, and never line 0. */
                    line = (u1)((u1)(y / n) * n);
                    if (line == 0) {
                        line = 1;
                    }
                }
            }
            eax = (mode7set & 0x02u) ? 255u - line : line;
            edx = BG1SXl[line];
            m7starty = (u2)eax;
            eax = (eax & 0xFFFF0000u) | BG1SYl[line];
            curvidoffset = esi;

            DLR[0] = eax;
            DLR[3] = edx;
            DLR[4] = (u4)(uintptr_t)esi;
            DLR[1] = kind == M7_PLAIN ? y : line;
            if (kind == M7_PLAIN) {
                if (scadsng[y] & 1u) {
                    if (vidbright != prevbrightdc) {
                        prevbrightdc = vidbright;
                        Gendcolortable();
                    }
                    DLR[6] = (u4)(uintptr_t)dcolortab;
                }
                m7call(drawmode7win16b);
            } else if (kind == M7_EXTBG) {
                m7call(drawmode7ngextbg16b);
            } else {
                m7call(drawmode7ngextbg216b);
            }

            if (kind == M7_PLAIN && Mode7HiRes16b != 0 && scanlines == 0) {
                DLR[1] = y;
                DLR[4] = (u4)(uintptr_t)esi;
                m7call(processmode7hires16b);
            }
        }
        y++;
        esi += 576u;
        if (resolutn < (u2)y) {
            break;
        }
    }
    mode7A = Mode7BackA;
    mode7C = Mode7BackC;
    mode7X0 = Mode7BackX0;
    mode7set = (u1)Mode7BackSet;
}
