/*
 * video/c_mode716win.c - the ProcessMode7ngwin*16b cluster, ported from
 * video/mode716.mac.
 *
 * These five subroutines walk the Mode 7 position over the pixels a window
 * masks out: no drawing, just advance the accumulators (and the VRAM tile
 * pointer, when a position crosses a tile edge) so the renderer resumes in the
 * right place. ngcwinptr walks a list of run lengths built by
 * ProcessBuildWindow - a "draw this many" entry alternating with a "skip this
 * many" one, which is why the routines tail-jump back into each other.
 *
 * Reached by call from the Mode7Processngw*16b macros with eax = the position
 * accumulator, ebx = mmode7ptr, esi = the video pointer and edi = the VRAM
 * tile base; all four come back, so the seam carries whole registers rather
 * than values. The M7Win* block is its own for the same reason M7Start* is:
 * this runs inside a renderer that has already spilled into M7Seam*.
 *
 * Positions are 32-bit fixed point with the tile coordinate in byte 1, so the
 * off-tile tests and the wrap masks below all work on that byte alone.
 */
#include <stdint.h>

#include "../types.h"

u4 M7WinAX;
u4 M7WinBX;
u4 M7WinCX;
u4 M7WinSI;
u4 M7WinDI;

extern u4 *ngcwinptr, ngwleft, ngwleftb, pixelsleft; /* video/c_mode716data.c */
extern u4 mode7xpos, mode7ypos, mode7xrpos, mode7yrpos;
extern u4 mode7xadder, mode7yadder;
extern u4 m7xaddof, m7yaddof;
extern u1 m7xaddof2, m7yaddof2; /* byte reads out of dword slots */
extern u4 mmode7ptr, mmode7xadd2, mmode7yadd2;
extern u1 mmode7xinc, mmode7xincc, mmode7yinc;
extern u1 mode7set; /* cpu/regs.inc */
extern u1 vrama[65536]; /* video buffer, by symbol */
extern u1* vram; /* the same buffer, by pointer */

/* Byte 1 holds the tile coordinate; 0xF8 set means the walk left the tile. */
static int off_tile(u4 const pos)
{
    return ((pos >> 8) & 0xF8u) != 0;
}

static u4 set_lo(u4 const r, u1 const v)
{
    return (r & ~0xFFu) | v;
}

static u4 set_hi(u4 const r, u1 const v)
{
    return (r & ~0xFF00u) | ((u4)v << 8);
}

enum entry { NGWIN_A, NGWIN_B, NGWIN_C, NGWIN_D, NGWIN_E };

/* Shared head of ngwin16b and ngwinC16b: claim the next run. Returns 0 when
   the run is empty, which drops into the matching skip routine. */
static int take_run(u4 const pos)
{
    u4 n = *ngcwinptr;

    if (n == 0) {
        M7WinCX = 0;
        return 0;
    }
    if (n >= ngwleft) {
        n = ngwleft;
        ngwleft = 0;
    } else {
        ngwleft -= n;
    }
    ngwleftb = n;
    M7WinCX = 0;
    M7WinAX = pos;
    return 1;
}

/* Head of the three skip routines: step to the next run length. Returns -1
   when nothing is left and the caller must stop. */
static s4 next_skip(void)
{
    u4 n;

    ngcwinptr++;
    n = *ngcwinptr;
    M7WinCX = n;
    if (n >= ngwleft) {
        ngwleft = 0;
        ngwleftb = 0;
        return -1;
    }
    ngwleft -= n;
    return (s4)n;
}

/* ProcessMode7ngwinB16b's skip loop: the tile pointer lives in ebx and is
   indexed straight off the vrama symbol. */
static void skip_b(void)
{
    do {
        while (off_tile(mode7xrpos)) {
            mode7xrpos -= m7xaddof;
            M7WinBX = set_lo(M7WinBX, (u1)((u1)M7WinBX + m7xaddof2));
            if (off_tile(mode7xrpos)) {
                M7WinBX = set_lo(M7WinBX, (u1)((u1)M7WinBX + mmode7xinc));
                mode7xrpos -= mmode7xadd2;
            }
            M7WinCX = (u4)vrama[M7WinBX] << 7;
            M7WinDI = (u4)(uintptr_t)(vrama + M7WinCX);
            break; /* .rposoffx falls through to the y test, it does not retry */
        }
        while (off_tile(mode7yrpos)) {
            mode7yrpos += m7yaddof;
            M7WinBX = set_hi(M7WinBX, (u1)((u1)(M7WinBX >> 8) - m7yaddof2));
            if (off_tile(mode7yrpos)) {
                M7WinBX = set_hi(M7WinBX, (u1)((u1)(M7WinBX >> 8) - mmode7yinc));
                mode7yrpos += mmode7yadd2;
            }
            M7WinBX &= 0x7FFFu;
            M7WinCX = (u4)vrama[M7WinBX] << 7;
            M7WinDI = (u4)(uintptr_t)(vrama + M7WinCX);
            break;
        }
        M7WinAX = mode7xadder;
        mode7xrpos += mode7xadder;
        M7WinAX = mode7yadder;
        mode7yrpos -= mode7yadder;
        M7WinSI += 2;
    } while (--pixelsleft != 0);
}

/* ProcessMode7ngwinE16b's skip loop. Unlike B, both off-tile handlers jump
   back above their own test, so each retries until the position is inside a
   tile again; and either can run the pointer off the map, which is what the
   0 return means. */
static int skip_e(void)
{
    do {
        for (;;) {
            if (off_tile(mode7xrpos)) {
                M7WinAX = set_lo(M7WinAX, mmode7xinc);
                M7WinDI = (u4)(uintptr_t)vram;
                mmode7ptr = set_lo(mmode7ptr, (u1)((u1)mmode7ptr + mmode7xinc));
                M7WinCX = set_lo(M7WinCX, mmode7xincc);
                if ((u1)mmode7ptr == mmode7xincc) {
                    return 0;
                }
                M7WinBX = mmode7ptr;
                M7WinCX = (u4)((u1 const*)(uintptr_t)M7WinDI)[M7WinBX] << 7;
                M7WinAX = mmode7xadd2;
                mode7xrpos -= mmode7xadd2;
                M7WinDI += M7WinCX;
                continue; /* .rposoffxr jumps back above the x test */
            }
            if (!off_tile(mode7yrpos)) {
                break;
            }
            M7WinAX = set_lo(M7WinAX, mmode7yinc);
            M7WinDI = (u4)(uintptr_t)vram;
            mmode7ptr = set_hi(mmode7ptr, (u1)((u1)(mmode7ptr >> 8) - mmode7yinc));
            if ((u1)(mmode7ptr >> 8) & 0x80u) { /* js */
                return 0;
            }
            M7WinBX = mmode7ptr;
            M7WinCX = (u4)((u1 const*)(uintptr_t)M7WinDI)[M7WinBX] << 7;
            M7WinAX = mmode7yadd2;
            mode7yrpos += mmode7yadd2;
            M7WinDI += M7WinCX;
            /* .rposoffyr jumps back above the y test, not the x one. */
        }
        M7WinAX = mode7xadder;
        mode7xrpos += mode7xadder;
        M7WinAX = mode7yadder;
        mode7yrpos -= mode7yadder;
        M7WinSI += 2;
    } while (--pixelsleft != 0);
    return 1;
}

static void run(enum entry e)
{
    for (;;) {
        switch (e) {
        case NGWIN_A:
            if (take_run(mode7xrpos)) {
                return;
            }
            e = NGWIN_B;
            break;

        case NGWIN_C:
            if (take_run(mode7xpos)) {
                return;
            }
            e = NGWIN_D;
            break;

        case NGWIN_B: {
            s4 const n = next_skip();
            if (n < 0) {
                return;
            }
            if (n != 0) {
                pixelsleft = (u4)n;
                skip_b();
            }
            ngcwinptr++;
            e = NGWIN_A;
            break;
        }

        case NGWIN_D: {
            /* Both off-tile tests are commented out in the assembly, which
               leaves its .rposoff* blocks unreachable - so this is a plain
               advance, and the counter stays in ecx rather than pixelsleft. */
            s4 n = next_skip();
            if (n < 0) {
                return;
            }
            while (n != 0) {
                M7WinAX = mode7xadder;
                mode7xpos += mode7xadder;
                M7WinAX = mode7yadder;
                mode7ypos -= mode7yadder;
                M7WinSI += 2;
                n--;
            }
            M7WinCX = 0;
            ngcwinptr++;
            e = NGWIN_A;
            break;
        }

        case NGWIN_E: {
            s4 const n = next_skip();
            if (n < 0) {
                return;
            }
            if (n != 0) {
                pixelsleft = (u4)n;
                if (!skip_e()) {
                    if (!(mode7set & 0x40u)) {
                        ngwleft = 0;
                        ngwleftb = 0;
                        return;
                    }
                    /* Tile repeat: keep only the in-tile part of each
                       coordinate and run out the rest of the span. */
                    do {
                        mode7yrpos &= 0xFFFF07FFu;
                        mode7xrpos &= 0xFFFF07FFu;
                        M7WinAX = mode7xadder;
                        mode7xrpos += mode7xadder;
                        M7WinAX = mode7yadder;
                        mode7yrpos -= mode7yadder;
                        M7WinSI += 2;
                    } while (--pixelsleft != 0);
                    /* Writes eax, which the loop left holding mode7yadder
                       rather than a position - kept as the assembly has it. */
                    mode7xrpos = M7WinAX;
                    return;
                }
            }
            ngcwinptr++;
            e = NGWIN_A;
            break;
        }
        }
    }
}

void c_ProcessMode7ngwin16b(void) { run(NGWIN_A); }
void c_ProcessMode7ngwinB16b(void) { run(NGWIN_B); }
void c_ProcessMode7ngwinC16b(void) { run(NGWIN_C); }
void c_ProcessMode7ngwinD16b(void) { run(NGWIN_D); }
void c_ProcessMode7ngwinE16b(void) { run(NGWIN_E); }
