/*
 * video/c_mode716start.c - Mode7Startup16b, ported from video/mode716.mac.
 *
 * Per-scanline setup for every Mode 7 renderer in the file: turn the screen
 * coordinate the caller hands in into a map position and a per-pixel adder for
 * each axis, then decide the direction the walk will run in.
 *
 * A real `call`, reached with ax = current y, dx = current x (left edge) and
 * esi = the video pointer. It hands back eax, esi and edi; see the thunk in
 * video/mode716.mac. The M7Start* block below is its own rather than the
 * M7Seam* one, because this runs *inside* a renderer that the hi-res pass
 * already has spilled into those.
 *
 * Positions are 32-bit fixed point with the map coordinate at byte offset 1,
 * so several of the adds below are 16-bit writes one byte into a dword and
 * must not carry past it.
 */
#include <string.h>

#include <stdint.h>

#include "../types.h"

u4 M7StartAX;
u4 M7StartDX;
u4 M7StartSI;
u4 M7StartDI;

extern u4 M7HROn; /* video/c_mode716data.c */
extern u2 mcxloc, mcyloc;
extern u4 mmode7xpos, mmode7ypos, mmode7xadder, mmode7yadder;
extern u4 mmode7xadd2, mmode7yadd2;
extern u1 mmode7xinc, mmode7xincc, mmode7yinc; /* byte writes into dword slots */
extern u2 m7starty;

extern u2 mode7A, mode7B, mode7C, mode7D, mode7X0, mode7Y0; /* cpu/regs.inc */
extern u1 mode7set;

extern u1 curmosaicsz; /* video/makevid.h */
extern u1* pesimpng; /* video/newgfx.c */
extern u1 xtravbuf[576]; /* endmem.c */

/* 13-bit signed field, sign-extended to 16. */
static u2 convert13(u4 const v)
{
    u2 const b = (u2)(v & 0x1FFFu);

    return (u2)((b & 0x1000u) ? (b | 0xE000u) : b);
}

/* CLIP: keep the low `bits`, or flood the rest with ones when the sign bit
   just above them is set. */
static u2 clip(u2 const v, u2 const sign, u2 const mask)
{
    return (u2)((v & sign) ? (v | (u2)~mask) : (v & mask));
}

/* The map coordinate sits one byte into the dword, and the assembly adds to it
   with a 16-bit add - the top byte must not see the carry. */
static void add_hi16(u4* const dst, u2 const v)
{
    u1* const p = (u1*)dst;
    u2 w;

    memcpy(&w, p + 1, 2);
    w = (u2)(w + v);
    memcpy(p + 1, &w, 2);
}

static s4 scaled(u2 const m, s4 const by, int const halve)
{
    s4 v = (s4)(s2)m * by;

    if (halve) {
        v >>= 1; /* sar */
    }
    return v & ~63;
}

/* Steps 2 and 3 are the same either side of the M7HROn split; only the two
   halvings in step 2 and the width of the y clip differ. */
static s4 calculate(int const hires, u4 const ax, u4 const dx)
{
    s4 eax;

    mcxloc = (u2)(convert13(dx) - convert13(mode7X0));
    mcxloc = clip(mcxloc, 0x2000u, 0x03FFu);

    if (!hires) {
        mcyloc = (u2)(convert13(ax) - convert13(mode7Y0));
        mcyloc = clip(mcyloc, 0x2000u, 0x03FFu);
    } else {
        /* One bit wider on both the field and the clip, and Y0 counts double. */
        u2 const b = (u2)(ax & 0x3FFFu);
        mcyloc = (u2)((b & 0x2000u) ? (b | 0xC000u) : b);
        mcyloc = (u2)(mcyloc - (u2)(convert13(mode7Y0) * 2u));
        mcyloc = clip(mcyloc, 0x4000u, 0x07FFu);
    }

    /* 2.) position at the scaled y, centred x. */
    mmode7xpos = (u4)scaled(mode7B, (s2)mcyloc, hires);
    add_hi16(&mmode7xpos, convert13(mode7X0));
    mmode7xpos += (u4)scaled(mode7B, (s4)m7starty, 0);

    mmode7ypos = (u4)scaled(mode7D, (s2)mcyloc, hires);
    add_hi16(&mmode7ypos, convert13(mode7Y0));
    mmode7ypos += (u4)scaled(mode7D, (s4)m7starty, 0);

    /* 3.) step left to the scaled edge. */
    mmode7xadder = (u4)(s4)(s2)mode7A;
    mmode7xpos += (u4)scaled(mode7A, (s2)mcxloc, 0);

    mmode7yadder = (u4)(-(s4)(s2)mode7C);
    eax = scaled(mode7C, (s2)mcxloc, 0);
    mmode7ypos += (u4)eax;

    if (mode7set & 1u) {
        mmode7xpos += mmode7xadder << 8;
        mmode7xadder = (u4)(-(s4)mmode7xadder);
        eax = (s4)(mmode7yadder << 8);
        mmode7ypos -= (u4)eax;
        mmode7yadder = (u4)(-(s4)mmode7yadder);
    }
    return eax;
}

void c_Mode7Startup16b(void)
{
    /* The assembly compares the low byte of the dword slot, not the dword. */
    s4 const eax = calculate((u1)M7HROn == 1, M7StartAX & 0xFFFFu, M7StartDX & 0xFFFFu);

    pesimpng = (u1*)(uintptr_t)M7StartSI;
    M7StartAX = (u4)eax;
    if (curmosaicsz != 1) {
        /* Mosaic draws into the scratch line instead, so hand the renderer
           that pointer back and leave edi past the block rep stosd cleared. */
        memset(xtravbuf + 32, 0xFF, 512);
        M7StartAX = 0xFFFFFFFFu;
        M7StartSI = (u4)(uintptr_t)(xtravbuf + 32);
        M7StartDI = (u4)(uintptr_t)(xtravbuf + 32 + 512);
    }

    /* Which way the per-pixel walk runs. */
    mmode7xadd2 = 0x800u;
    mmode7xinc = 2;
    mmode7xincc = 0;
    if (mmode7xadder & 0x80000000u) {
        mmode7xadd2 = (u4)-0x800;
        mmode7xinc = (u1)-2;
        mmode7xincc = 0xFE;
    }
    mmode7yadd2 = 0x800u;
    mmode7yinc = 1;
    if (mmode7yadder & 0x80000000u) {
        mmode7yadd2 = (u4)-0x800;
        mmode7yinc = (u1)-1;
    }
}
