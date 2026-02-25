/*
 * c_newgfx_asm.c — C replacements for video rendering ASM functions
 *
 * Provides C implementations of:
 *   - preparesprpr (from newgfx.asm)
 *   - domosaic16b (from makev16b.asm)
 *   - dualstartprocess_c (from makevid.asm)
 *   - makedualwinsp_c (from makevid.asm)
 *
 * These are called from c_makevid.c and c_makev16b.c when
 * building with NO_ASM=1.
 *
 * Copyright (C) 1997-2008 ZSNES Team
 * Licensed under GPL v2.
 */

#ifdef NO_ASM

#include "../types.h"
#include <string.h>

/* --- External globals --- */
extern u2  curypos;
extern u1  sprclprio[4];
extern u4  sprsingle;
extern u1  sprleftpr[256];

extern u2  xtravbuf[288];
extern u1* curvidoffset;
extern u1  curmosaicsz;
extern u1  winon;
extern u1* winptrref;

extern u4  winl1;
extern u1  winlogicb;
extern u1  dualwinsp;
extern u1  pwinspenab;
extern u4  pwinsptype;
extern u1  winonsp;
extern u1  winonstype;
extern u1* cwinptr;
extern u1* dwinptrproc;
extern u1  winspdata[288];

/*
 * preparesprpr — check sprite priority configuration
 *
 * Sets sprclprio from the sprite-left-for-priority table, and
 * sets sprsingle=1 if exactly one priority level has sprites.
 */
void preparesprpr_c(void)
{
    u4 val = ((u4*)sprleftpr)[curypos & 0xFF];
    *(u4*)sprclprio = val;
    sprsingle = (val == 0x00000001 || val == 0x00000100 ||
                 val == 0x00010000 || val == 0x01000000) ? 1 : 0;
}

/*
 * domosaic16b — apply mosaic effect to scanline
 *
 * Replicates pixels in xtravbuf to curvidoffset based on mosaic size.
 * Input: mosaic_size = curmosaicsz (passed via edx>>8 in ASM, we use the global)
 */
void domosaic16b_c(void)
{
    u2* src = xtravbuf + 16; /* +16 words = +32 bytes = xtravbuf+32 in ASM */
    u2* dst = (u2*)curvidoffset;
    u1 msize = curmosaicsz;
    u1 counter = 0; /* 8-bit wrap counter: 0 means 256 iterations */
    u1 mcnt = msize;
    u2 pixel = *src;

    if (winon) {
        u1* wp = winptrref;
        do {
            if (!*wp)
                *dst = pixel;
            src++;
            dst++;
            wp++;
            if (--counter == 0)
                break;
            if (--mcnt == 0) {
                pixel = *src;
                mcnt = msize;
            }
        } while (1);
    } else {
        if (pixel == 0)
            goto zeroloop_entry;
        do {
            *dst = pixel;
            src++;
            dst++;
            if (--counter == 0)
                break;
            if (--mcnt == 0) {
                pixel = *src;
                mcnt = msize;
                if (pixel == 0)
                    goto zeroloop;
            }
            continue;
zeroloop:
            src++;
            dst++;
            if (--counter == 0)
                break;
            if (--mcnt == 0) {
                pixel = *src;
                mcnt = msize;
                if (pixel != 0)
                    continue;
            }
zeroloop_entry:
            src++;
            dst++;
            if (--counter == 0)
                break;
            if (--mcnt == 0) {
                pixel = *src;
                mcnt = msize;
                if (pixel != 0) {
                    *dst = pixel;
                    src++;
                    dst++;
                    if (--counter == 0)
                        break;
                    if (--mcnt == 0) {
                        pixel = *src;
                        mcnt = msize;
                    }
                    continue;
                }
            }
            goto zeroloop;
        } while (1);
    }
}

/*
 * fill_window1 — fill 256-byte window mask for window 1
 *
 * al = enable flags (bit 0 = inside/outside for win1)
 * Writes to dwinptrproc buffer.
 * Returns with buffer filled for win1 only.
 */
static void fill_window1(u1 flags, u1* buf, u1 wl, u1 wr)
{
    int i;
    if (!(flags & 0x01)) {
        /* inside window */
        if (wl == 254 || wl >= wr) {
            memset(buf, 0, 256);
            return;
        }
        for (i = 0; i <= wl; i++)
            buf[i] = 0;
        for (; i <= wr; i++)
            buf[i] = 1;
        for (; i < 256; i++)
            buf[i] = 0;
    } else {
        /* outside window */
        if (wl >= wr) {
            memset(buf, 1, 256);
            return;
        }
        if (wl <= 1 && wr >= 254) {
            memset(buf, 0, 256);
            return;
        }
        for (i = 0; i < wl; i++)
            buf[i] = 1;
        for (; i <= wr; i++)
            buf[i] = 0;
        for (i = (u1)(wr + 1); i < 256; i++)
            buf[i] = 1;
    }
}

/*
 * apply_window2_and — apply window 2 with AND logic
 */
static void apply_window2_and(u1 flags, u1* buf, u1 wl, u1 wr)
{
    int i;
    if (!(flags & 0x04)) {
        /* inside */
        if (wl == 254 || wl >= wr) {
            memset(buf, 0, 256);
            return;
        }
        for (i = 0; i <= wl; i++)
            buf[i] = 0;
        for (; i <= wr; i++)
            buf[i] &= 1;
        for (; i < 256; i++)
            buf[i] = 0;
    } else {
        /* outside */
        if (wl >= wr)
            return; /* AND with all-1 = no change */
        if (wl <= 1 && wr >= 254) {
            memset(buf, 0, 256);
            return;
        }
        for (i = 0; i < wl; i++)
            buf[i] &= 1;
        for (; i <= wr; i++)
            buf[i] = 0;
        for (i = (u1)(wr + 1); i < 256; i++)
            buf[i] &= 1;
    }
}

/*
 * apply_window2_or — apply window 2 with OR logic
 */
static void apply_window2_or(u1 flags, u1* buf, u1 wl, u1 wr)
{
    int i;
    if (!(flags & 0x04)) {
        /* inside */
        if (wl == 254 || wl >= wr)
            return; /* OR with all-0 = no change */
        for (i = wl + 1; i <= wr; i++)
            buf[i] = 1;
    } else {
        /* outside */
        if (wl >= wr) {
            memset(buf, 1, 256);
            return;
        }
        if (wl <= 1 && wr >= 254)
            return; /* OR with all-0 = no change */
        for (i = 0; i < wl; i++)
            buf[i] = 1;
        for (i = (u1)(wr + 1); i < 256; i++)
            buf[i] = 1;
    }
}

/*
 * apply_window2_xor — apply window 2 with XOR logic
 */
static void apply_window2_xor(u1 flags, u1* buf, u1 wl, u1 wr)
{
    int i;
    if (!(flags & 0x04)) {
        /* inside */
        if (wl == 254 || wl >= wr)
            return;
        for (i = wl + 1; i <= wr; i++)
            buf[i] ^= 1;
    } else {
        /* outside */
        if (wl >= wr) {
            for (i = 0; i < 256; i++)
                buf[i] ^= 1;
            return;
        }
        if (wl <= 1 && wr >= 254)
            return;
        for (i = 0; i < wl; i++)
            buf[i] ^= 1;
        for (i = (u1)(wr + 1); i < 256; i++)
            buf[i] ^= 1;
    }
}

/*
 * apply_window2_xnor — apply window 2 with XNOR logic (XOR + NOT)
 */
static void apply_window2_xnor(u1 flags, u1* buf, u1 wl, u1 wr)
{
    apply_window2_xor(flags, buf, wl, wr);
    int i;
    for (i = 0; i < 256; i++)
        buf[i] ^= 1;
}

/*
 * dualstartprocess_c — compute dual-window mask
 *
 * Fills the 256-byte dwinptrproc buffer with a per-pixel window mask.
 * Window 1 is applied first, then window 2 is combined using the
 * selected logic operation (AND, OR, XOR, XNOR).
 */
void dualstartprocess_c(u1 flags, u1 logic)
{
    u1* buf = dwinptrproc;
    u1 wl1 = (u1)winl1;
    u1 wr1 = (u1)(winl1 >> 8);
    u1 wl2 = (u1)(winl1 >> 16);
    u1 wr2 = (u1)(winl1 >> 24);

    fill_window1(flags, buf, wl1, wr1);

    switch (logic) {
    case 0: apply_window2_and(flags, buf, wl2, wr2);  break;
    case 1: apply_window2_or(flags, buf, wl2, wr2);   break;
    case 2: apply_window2_xor(flags, buf, wl2, wr2);  break;
    case 3: apply_window2_xnor(flags, buf, wl2, wr2); break;
    }
}

/*
 * makedualwinsp_c — compute dual-window mask for sprites
 *
 * Caches previous window settings to avoid redundant recomputation.
 */
void makedualwinsp_c(u1 flags)
{
    u1 logic = winlogicb & 0x03;

    if (logic == dualwinsp && flags == pwinspenab && winl1 == pwinsptype) {
        cwinptr = winspdata + 16;
        winonsp = winonstype;
        return;
    }

    dualwinsp = logic;
    pwinspenab = flags;
    pwinsptype = winl1;
    dwinptrproc = winspdata + 16;
    cwinptr = winspdata + 16;
    winonsp = 1;
    winonstype = 1;

    dualstartprocess_c(flags, logic);
}

#endif /* NO_ASM */
