/*
 * video/c_ngmosaic.c - the mosaic pass of video/newgfx16.asm.
 *
 * domosaicng16b plus the sixty mosdraw* routines it jumps into. A mosaic block
 * is drawn by taking the first pixel of each block out of the scratch line and
 * smearing it across the block, so all sixty differ only in the block size
 * (2..16) and which of four screen combinations they write:
 *
 *   16b     one copy, straight down
 *   16bt    one copy, with the unused bit forced on
 *   16btms  two copies - main with the bit on, sub with it off
 *   16bntms two copies, neither touched
 *
 * The jump tables in cpu/c_table.c still name all sixty entry points, so they
 * stay as one-line thunks that carry the size and the variant.
 *
 * 0xFFFF in the scratch line means "nothing here"; the block is skipped and
 * whatever the earlier passes left in the video line shows through.
 */
#include <stdint.h>

#include "../types.h"
#include "makevid.h"
#include "newgfx16.h"

u4 MOSAX;
u4 MOSBX;
u4 MOSCX;
u4 MOSDX;
u4 MOSSI;
u4 MOSDI;
u4 MOSBP;

extern u1 BGMA[256], BGMS1[], FillSubScr[256], scadtng[256];
extern u4 mosclineval, mostranspval;
extern u1* pesimpng;
extern u2 xtravbuf[288];

enum {
    M_PLAIN,
    M_T,
    M_TMS,
    M_NTMS
};

/* The sub screen sits this far into the same buffer. */
#define MOS_SUB 75036u

/* How many blocks of each size cover a line. Not 256/n: the assembly rounds
   up, so the last block can run past the end of the line. */
static u2 const mos_count[15]
    = { 128, 86, 64, 52, 43, 37, 32, 29, 26, 24, 22, 20, 19, 18, 16 };

/* One mosaic pass. `n` is the block size, 2..16. */
static void mosaic(u4 const n, int const mode)
{
    u2* edi = xtravbuf + 16;
    u2* esi = (u2*)(uintptr_t)MOSSI;
    u4 count = mos_count[n - 2];
    u4 eax = MOSAX;

    do {
        eax = (eax & 0xFFFF0000u) | *edi;
        if ((u2)eax != 0xFFFFu) {
            if (mode == M_T || mode == M_TMS) {
                eax = (eax & 0xFFFF0000u) | (u2)(eax | UnusedBit[0]);
            }
            for (u4 i = 0; i < n; i++) {
                esi[i] = (u2)eax;
            }
            if (mode == M_TMS) {
                eax = (eax & 0xFFFF0000u) | (u2)(eax & UnusedBitXor[0]);
            }
            if (mode == M_TMS || mode == M_NTMS) {
                for (u4 i = 0; i < n; i++) {
                    esi[MOS_SUB + i] = (u2)eax;
                }
            }
        }
        edi += n;
        esi += n;
    } while (--count != 0);
    MOSAX = eax;
    MOSCX = 0;
    MOSDI = (u4)(uintptr_t)edi;
    /* `pop esi` at the end of every mosdraw* undoes both the push and the
       sub-screen bias below, so the caller gets the scratch pointer back. */
    MOSSI = (u4)(uintptr_t)pesimpng;
}

void c_domosaicng16b(void)
{
    u4 const n = curmosaicsz;
    u4 const line = mosclineval;
    u1 const cl = (u1)mostranspval;
    int mode = M_PLAIN;

    MOSAX = n;
    MOSSI = (u4)(uintptr_t)pesimpng;
    MOSDI = (u4)(uintptr_t)(xtravbuf + 16);
    if (n > 16 || n <= 1) {
        return;
    }
    /* Mode 7 always goes down as one plain copy; otherwise which screens this
       line is on, and whether the sub screen is a fill, pick the variant. */
    if (BGMA[line] != 7) {
        if (BGMS1[line * 2] & cl) {
            if (FillSubScr[line] & 1) {
                if (BGMS1[line * 2 + 1] & cl) {
                    mode = (scadtng[line] & cl) ? M_TMS : M_NTMS;
                } else if (scadtng[line] & cl) {
                    mode = M_T;
                }
            }
        } else if (FillSubScr[line] & 1) {
            MOSSI += MOS_SUB * 2u;
        }
    }
    mosaic(n, mode);
}
