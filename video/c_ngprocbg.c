/*
 * video/c_ngprocbg.c - the background passes of StartDrawNewGfx16b in
 * video/newgfx16.asm.
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
#include "makevid.h"

/* calldl16t's register block, defined in video/c_mv16tline.c. */
extern u4 DLR[7];
extern void (*DLFN)(void);
extern void calldl16t(void);

extern u1 BGFB[256], BGMA[256], BG3PRI[256];
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
