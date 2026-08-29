#ifndef C_MODE716WRITE_H
#define C_MODE716WRITE_H

/*
 * The Mode 7 pixel writers from video/mode716.asm as one table. The assembly
 * has fourteen Mode7Normal/Mode7ExtBG macros but only ten distinct writes -
 * the nt suffix marks a caller. Each takes the palette index in edx's low byte
 * and leaves edx as the assembly does: zeroed when it drew, untouched
 * otherwise.
 */
#include "../types.h"

#define M7_BUF 75036u /* one video buffer, in 16-bit pixels */

enum m7_writer {
    M7W_NORMAL, /* Mode7Normal, Mode7Normalnt        */
    M7W_NORMAL_T, /* Mode7Normalt                      */
    M7W_NORMAL_MS, /* Mode7Normalmsnt                   */
    M7W_NORMAL_MST, /* Mode7Normalmst                    */
    M7W_NORMAL_S, /* Mode7Normalsnt, Mode7Normalst     */
    M7W_EXTBG, /* Mode7ExtBG, Mode7ExtBGnt          */
    M7W_EXTBG_T, /* Mode7ExtBGt                       */
    M7W_EXTBG_MS, /* Mode7ExtBGmsnt                    */
    M7W_EXTBG_MST, /* Mode7ExtBGmst                     */
    M7W_EXTBG_S /* Mode7ExtBGsnt, Mode7ExtBGst       */
};

extern u4 UnusedBit[2], UnusedBitXor[2]; /* video/newgfx16.asm */

/* Writes one pixel at *p, advances it by 2, and returns the new edx. */
static inline u4 m7_write(enum m7_writer const w, u1** const p, u2 const* const pal,
    u4 edx)
{
    u1* const s = *p;
    u1 const d = (u1)edx;
    u2 dx;

    *p = s + 2;

    if (w >= M7W_EXTBG) {
        /* The EXTBG writers stash the raw index a whole buffer on, then skip
           any pixel whose high bit marks it for the second pass. */
        s[M7_BUF * 8] = d;
        if (d == 0 || (d & 0x80u)) {
            return edx;
        }
    } else if (d == 0) {
        return edx;
    }

    switch (w) {
    case M7W_NORMAL:
    case M7W_EXTBG:
        dx = pal[edx];
        *(u2*)s = dx;
        break;
    case M7W_NORMAL_T:
    case M7W_EXTBG_T:
        dx = pal[edx + 256];
        *(u2*)s = dx;
        break;
    case M7W_NORMAL_MS:
    case M7W_EXTBG_MS:
        dx = pal[edx];
        *(u2*)s = dx;
        *(u2*)(s + M7_BUF * 2) = dx;
        break;
    case M7W_NORMAL_MST:
        dx = pal[edx + 256];
        *(u2*)s = dx;
        dx = (u2)(dx & UnusedBitXor[0]);
        *(u2*)(s + M7_BUF * 2) = dx;
        break;
    case M7W_EXTBG_MST:
        /* The one that writes the sub screen first and the main screen from
           the OR-ed value, not the other way round. */
        dx = pal[edx + 256];
        *(u2*)(s + M7_BUF * 2) = dx;
        dx = (u2)(dx | UnusedBit[0]);
        *(u2*)s = dx;
        break;
    case M7W_NORMAL_S:
    case M7W_EXTBG_S:
        dx = pal[edx];
        *(u2*)(s + M7_BUF * 2) = dx;
        break;
    }
    return 0;
}

#endif
