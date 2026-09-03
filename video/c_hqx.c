/*
 * Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
 *
 * http://www.zsnes.com
 * http://sourceforge.net/projects/zsnes
 * https://zsnes.bountysource.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * hq2x is MaxSt's filter, ported from the assembly. The other five entry
 * points here are still scalar nearest-neighbour block doublers (NxN pixel
 * replication) rather than the real hq filter.
 */

#include "c_hqx.h"
#include "../c_vcache.h"
#include "../cpu/regs.h"
#include "../ui.h"
#include "../unaligned.h"
#include "cfg.h"
#include "copyvwin.h"

// 16->32bit color lookup table (4096 + 65536*16 bytes), defined in ui.c.
extern u1* BitConv32Ptr;

// Source pixels start past the top/left border; 256 pixels are read per line
// then 32 (64 bytes) are skipped to the next line. Skip drawing when blanked.
#define SRC_START ((u2 const*)(vidbuffer + 16 * 2 + 256 * 2 + 32 * 2))
#define SRC_LINE_SKIP 32 // u2 units

static void hq2x_double_16b(void)
{
    u2 const* src = SRC_START;
    u1* dst = WinVidMemStart;
    u4 const ebx = NumBytesPerLine;
    u1 lines = resolutn;
    do {
        u4 ecx = 256;
        do {
            u4 const px = *src * 0x00010001u; // pixel in both halves
            *(u4*)dst = px;
            *(u4*)(dst + ebx) = px;
            src += 1;
            dst += 4;
        } while (--ecx != 0);
        dst += AddEndBytes + ebx;
        src += SRC_LINE_SKIP;
    } while (--lines != 0);
}

/* ---- hq2x -------------------------------------------------------------- *
 *
 * MaxSt's hq2x, ported from video/hq2x16.asm (deleted in 5ff6d63d). Two
 * things in the assembly are not reproduced: the delta buffer, which skipped
 * pixels whose neighbourhood had not changed since the previous frame, and
 * the "cross" table, a shortcut for the case where no edge neighbour differs
 * perceptually. Both computed the same pixels the general path does.
 */

extern u1* RGBtoYUVPtr; // ui.c, filled by the video backend
extern u4 HalfTrans[4]; // low bit of each channel cleared, per pixel format
extern u1 hirestiledat[256], SpecialLine[256];
extern u1 GUIOn;
extern uint8_t GUIOn2;

/* Perceptually different, the hq2x test: |dY|>0x30, |dU|>7 or |dV|>6, over
   the packed YUV the backend built. */
static int differs(u2 const a, u2 const b)
{
    if (a == b)
        return 0;
    u4 const* const yuv = (u4 const*)RGBtoYUVPtr;
    u4 const ya = yuv[a], yb = yuv[b];

    for (unsigned ch = 0; ch < 3; ch++) {
        u4 const va = (ya >> (ch * 8)) & 0xFFu;
        u4 const vb = (yb >> (ch * 8)) & 0xFFu;
        u4 const d = va > vb ? va - vb : vb - va;
        if (d > ((0x00300706u >> (ch * 8)) & 0xFFu))
            return 1;
    }
    return 0;
}

/* (3*a + b) / 4, as two masked half-mixes. */
static u2 interp1(u2 const a, u2 const b)
{
    u4 edx = a, ecx = b;

    if (edx == ecx)
        return (u2)edx;
    edx &= HalfTrans[0];
    ecx &= HalfTrans[0];
    ecx = ((ecx + edx) >> 1) + 0x0821u;
    ecx &= HalfTrans[0];
    return (u2)((edx + ecx) >> 1);
}

/* (2*a + b + c) / 4. */
static u2 interp2(u2 const a, u2 const b, u2 const c)
{
    u4 edx = b, ecx = c;

    if (edx != ecx) {
        edx &= HalfTrans[0];
        ecx &= HalfTrans[0];
        ecx = ((ecx + edx) >> 1) + 0x0821u;
    }
    edx = a;
    if (edx == ecx)
        return (u2)edx;
    ecx &= HalfTrans[0];
    edx &= HalfTrans[0];
    return (u2)((edx + ecx) >> 1);
}

/* The weighted forms work in the 32-bit expansion, one 16-bit lane per
   channel, then squeeze the four 6-bit results back into 565 the way the
   assembly's shift chain did. */
static u2 interp_w(u2 const a, u2 const b, u2 const c, u4 const ka,
    u4 const kb, u4 const kc, unsigned const sh)
{
    u4 const* const conv = (u4 const*)BitConv32Ptr;
    u4 const ca = conv[a], cb = conv[b], cc = conv[c];
    u4 edx = 0;

    for (unsigned ch = 0; ch < 4; ch++) {
        u4 v = (((ca >> (ch * 8)) & 0xFFu) * ka + ((cb >> (ch * 8)) & 0xFFu) * kb
                   + ((cc >> (ch * 8)) & 0xFFu) * kc)
            >> sh;
        edx |= (v > 255u ? 255u : v) << (ch * 8);
    }
    edx = (edx & 0xFFFFFF00u) | ((edx << 2) & 0xFFu); // shl dl,2
    edx >>= 1;
    edx = (edx & 0xFFFF0000u) | ((edx << 3) & 0xFFFFu); // shl dx,3
    return (u2)(edx >> 5);
}

static u2 interp6(u2 a, u2 b, u2 c) { return interp_w(a, b, c, 5, 2, 1, 5); }
static u2 interp7(u2 a, u2 b, u2 c) { return interp_w(a, b, c, 6, 1, 1, 5); }
static u2 interp9(u2 a, u2 b, u2 c) { return interp_w(a, b, c, 2, 3, 3, 5); }
static u2 interp10(u2 a, u2 b, u2 c) { return interp_w(a, b, c, 14, 1, 1, 6); }

/* One source pixel into its 2x2 block. w[1..9] is the neighbourhood, row
   major, w[5] the centre. */
static void hq2x_pixel(u2 const w[10], u1* const dst, u4 const pitch)
{
    u2 const w5 = w[5];
    u2 p00, p01, p10, p11;
    unsigned pattern = 0;

    if (differs(w5, w[1]))
        pattern |= 1;
    if (differs(w5, w[2]))
        pattern |= 2;
    if (differs(w5, w[3]))
        pattern |= 4;
    if (differs(w5, w[4]))
        pattern |= 8;
    if (differs(w5, w[6]))
        pattern |= 16;
    if (differs(w5, w[7]))
        pattern |= 32;
    if (differs(w5, w[8]))
        pattern |= 64;
    if (differs(w5, w[9]))
        pattern |= 128;

    switch (pattern) {
    case 0:
    case 1:
    case 4:
    case 5:
    case 32:
    case 33:
    case 36:
    case 37:
    case 128:
    case 129:
    case 132:
    case 133:
    case 160:
    case 161:
    case 164:
    case 165:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp2(w5, w[2], w[6]);
        p10 = interp2(w5, w[8], w[4]);
        p11 = interp2(w5, w[6], w[8]);
        break;
    case 2:
    case 34:
    case 130:
    case 162:
        p00 = interp2(w5, w[1], w[4]);
        p01 = interp2(w5, w[3], w[6]);
        p10 = interp2(w5, w[8], w[4]);
        p11 = interp2(w5, w[6], w[8]);
        break;
    case 16:
    case 17:
    case 48:
    case 49:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp2(w5, w[3], w[2]);
        p10 = interp2(w5, w[8], w[4]);
        p11 = interp2(w5, w[9], w[8]);
        break;
    case 64:
    case 65:
    case 68:
    case 69:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp2(w5, w[2], w[6]);
        p10 = interp2(w5, w[7], w[4]);
        p11 = interp2(w5, w[9], w[6]);
        break;
    case 8:
    case 12:
    case 136:
    case 140:
        p00 = interp2(w5, w[1], w[2]);
        p01 = interp2(w5, w[2], w[6]);
        p10 = interp2(w5, w[7], w[8]);
        p11 = interp2(w5, w[6], w[8]);
        break;
    case 3:
    case 35:
    case 131:
    case 163:
        p00 = interp1(w5, w[4]);
        p01 = interp2(w5, w[3], w[6]);
        p10 = interp2(w5, w[8], w[4]);
        p11 = interp2(w5, w[6], w[8]);
        break;
    case 6:
    case 38:
    case 134:
    case 166:
        p00 = interp2(w5, w[1], w[4]);
        p01 = interp1(w5, w[6]);
        p10 = interp2(w5, w[8], w[4]);
        p11 = interp2(w5, w[6], w[8]);
        break;
    case 20:
    case 21:
    case 52:
    case 53:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp1(w5, w[2]);
        p10 = interp2(w5, w[8], w[4]);
        p11 = interp2(w5, w[9], w[8]);
        break;
    case 144:
    case 145:
    case 176:
    case 177:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp2(w5, w[3], w[2]);
        p10 = interp2(w5, w[8], w[4]);
        p11 = interp1(w5, w[8]);
        break;
    case 192:
    case 193:
    case 196:
    case 197:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp2(w5, w[2], w[6]);
        p10 = interp2(w5, w[7], w[4]);
        p11 = interp1(w5, w[6]);
        break;
    case 96:
    case 97:
    case 100:
    case 101:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp2(w5, w[2], w[6]);
        p10 = interp1(w5, w[4]);
        p11 = interp2(w5, w[9], w[6]);
        break;
    case 40:
    case 44:
    case 168:
    case 172:
        p00 = interp2(w5, w[1], w[2]);
        p01 = interp2(w5, w[2], w[6]);
        p10 = interp1(w5, w[8]);
        p11 = interp2(w5, w[6], w[8]);
        break;
    case 9:
    case 13:
    case 137:
    case 141:
        p00 = interp1(w5, w[2]);
        p01 = interp2(w5, w[2], w[6]);
        p10 = interp2(w5, w[7], w[8]);
        p11 = interp2(w5, w[6], w[8]);
        break;
    case 18:
    case 50:
        p00 = interp2(w5, w[1], w[4]);
        if (differs(w[2], w[6])) {
            p01 = interp1(w5, w[3]);
        } else {
            p01 = interp2(w5, w[2], w[6]);
        }
        p10 = interp2(w5, w[8], w[4]);
        p11 = interp2(w5, w[9], w[8]);
        break;
    case 80:
    case 81:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp2(w5, w[3], w[2]);
        p10 = interp2(w5, w[7], w[4]);
        if (differs(w[6], w[8])) {
            p11 = interp1(w5, w[9]);
        } else {
            p11 = interp2(w5, w[6], w[8]);
        }
        break;
    case 72:
    case 76:
        p00 = interp2(w5, w[1], w[2]);
        p01 = interp2(w5, w[2], w[6]);
        if (differs(w[8], w[4])) {
            p10 = interp1(w5, w[7]);
        } else {
            p10 = interp2(w5, w[8], w[4]);
        }
        p11 = interp2(w5, w[9], w[6]);
        break;
    case 10:
    case 138:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = interp2(w5, w[3], w[6]);
        p10 = interp2(w5, w[7], w[8]);
        p11 = interp2(w5, w[6], w[8]);
        break;
    case 66:
        p00 = interp2(w5, w[1], w[4]);
        p01 = interp2(w5, w[3], w[6]);
        p10 = interp2(w5, w[7], w[4]);
        p11 = interp2(w5, w[9], w[6]);
        break;
    case 24:
        p00 = interp2(w5, w[1], w[2]);
        p01 = interp2(w5, w[3], w[2]);
        p10 = interp2(w5, w[7], w[8]);
        p11 = interp2(w5, w[9], w[8]);
        break;
    case 7:
    case 39:
    case 135:
        p00 = interp1(w5, w[4]);
        p01 = interp1(w5, w[6]);
        p10 = interp2(w5, w[8], w[4]);
        p11 = interp2(w5, w[6], w[8]);
        break;
    case 148:
    case 149:
    case 180:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp1(w5, w[2]);
        p10 = interp2(w5, w[8], w[4]);
        p11 = interp1(w5, w[8]);
        break;
    case 224:
    case 225:
    case 228:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp2(w5, w[2], w[6]);
        p10 = interp1(w5, w[4]);
        p11 = interp1(w5, w[6]);
        break;
    case 41:
    case 45:
    case 169:
        p00 = interp1(w5, w[2]);
        p01 = interp2(w5, w[2], w[6]);
        p10 = interp1(w5, w[8]);
        p11 = interp2(w5, w[6], w[8]);
        break;
    case 22:
    case 54:
        p00 = interp2(w5, w[1], w[4]);
        if (differs(w[2], w[6])) {
            p01 = w5;
        } else {
            p01 = interp2(w5, w[2], w[6]);
        }
        p10 = interp2(w5, w[8], w[4]);
        p11 = interp2(w5, w[9], w[8]);
        break;
    case 208:
    case 209:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp2(w5, w[3], w[2]);
        p10 = interp2(w5, w[7], w[4]);
        if (differs(w[6], w[8])) {
            p11 = w5;
        } else {
            p11 = interp2(w5, w[6], w[8]);
        }
        break;
    case 104:
    case 108:
        p00 = interp2(w5, w[1], w[2]);
        p01 = interp2(w5, w[2], w[6]);
        if (differs(w[8], w[4])) {
            p10 = w5;
        } else {
            p10 = interp2(w5, w[8], w[4]);
        }
        p11 = interp2(w5, w[9], w[6]);
        break;
    case 11:
    case 139:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = interp2(w5, w[3], w[6]);
        p10 = interp2(w5, w[7], w[8]);
        p11 = interp2(w5, w[6], w[8]);
        break;
    case 19:
    case 51:
        if (differs(w[2], w[6])) {
            p00 = interp1(w5, w[4]);
            p01 = interp1(w5, w[3]);
        } else {
            p00 = interp6(w5, w[2], w[4]);
            p01 = interp9(w5, w[2], w[6]);
        }
        p10 = interp2(w5, w[8], w[4]);
        p11 = interp2(w5, w[9], w[8]);
        break;
    case 146:
    case 178:
        p00 = interp2(w5, w[1], w[4]);
        if (differs(w[2], w[6])) {
            p01 = interp1(w5, w[3]);
            p11 = interp1(w5, w[8]);
        } else {
            p01 = interp9(w5, w[2], w[6]);
            p11 = interp6(w5, w[6], w[8]);
        }
        p10 = interp2(w5, w[8], w[4]);
        break;
    case 84:
    case 85:
        p00 = interp2(w5, w[4], w[2]);
        if (differs(w[6], w[8])) {
            p01 = interp1(w5, w[2]);
            p11 = interp1(w5, w[9]);
        } else {
            p01 = interp6(w5, w[6], w[2]);
            p11 = interp9(w5, w[6], w[8]);
        }
        p10 = interp2(w5, w[7], w[4]);
        break;
    case 112:
    case 113:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp2(w5, w[3], w[2]);
        if (differs(w[6], w[8])) {
            p10 = interp1(w5, w[4]);
            p11 = interp1(w5, w[9]);
        } else {
            p10 = interp6(w5, w[8], w[4]);
            p11 = interp9(w5, w[6], w[8]);
        }
        break;
    case 200:
    case 204:
        p00 = interp2(w5, w[1], w[2]);
        p01 = interp2(w5, w[2], w[6]);
        if (differs(w[8], w[4])) {
            p10 = interp1(w5, w[7]);
            p11 = interp1(w5, w[6]);
        } else {
            p10 = interp9(w5, w[8], w[4]);
            p11 = interp6(w5, w[8], w[6]);
        }
        break;
    case 73:
    case 77:
        if (differs(w[8], w[4])) {
            p00 = interp1(w5, w[2]);
            p10 = interp1(w5, w[7]);
        } else {
            p00 = interp6(w5, w[4], w[2]);
            p10 = interp9(w5, w[8], w[4]);
        }
        p01 = interp2(w5, w[2], w[6]);
        p11 = interp2(w5, w[9], w[6]);
        break;
    case 42:
    case 170:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
            p10 = interp1(w5, w[8]);
        } else {
            p00 = interp9(w5, w[4], w[2]);
            p10 = interp6(w5, w[4], w[8]);
        }
        p01 = interp2(w5, w[3], w[6]);
        p11 = interp2(w5, w[6], w[8]);
        break;
    case 14:
    case 142:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
            p01 = interp1(w5, w[6]);
        } else {
            p00 = interp9(w5, w[4], w[2]);
            p01 = interp6(w5, w[2], w[6]);
        }
        p10 = interp2(w5, w[7], w[8]);
        p11 = interp2(w5, w[6], w[8]);
        break;
    case 67:
        p00 = interp1(w5, w[4]);
        p01 = interp2(w5, w[3], w[6]);
        p10 = interp2(w5, w[7], w[4]);
        p11 = interp2(w5, w[9], w[6]);
        break;
    case 70:
        p00 = interp2(w5, w[1], w[4]);
        p01 = interp1(w5, w[6]);
        p10 = interp2(w5, w[7], w[4]);
        p11 = interp2(w5, w[9], w[6]);
        break;
    case 28:
        p00 = interp2(w5, w[1], w[2]);
        p01 = interp1(w5, w[2]);
        p10 = interp2(w5, w[7], w[8]);
        p11 = interp2(w5, w[9], w[8]);
        break;
    case 152:
        p00 = interp2(w5, w[1], w[2]);
        p01 = interp2(w5, w[3], w[2]);
        p10 = interp2(w5, w[7], w[8]);
        p11 = interp1(w5, w[8]);
        break;
    case 194:
        p00 = interp2(w5, w[1], w[4]);
        p01 = interp2(w5, w[3], w[6]);
        p10 = interp2(w5, w[7], w[4]);
        p11 = interp1(w5, w[6]);
        break;
    case 98:
        p00 = interp2(w5, w[1], w[4]);
        p01 = interp2(w5, w[3], w[6]);
        p10 = interp1(w5, w[4]);
        p11 = interp2(w5, w[9], w[6]);
        break;
    case 56:
        p00 = interp2(w5, w[1], w[2]);
        p01 = interp2(w5, w[3], w[2]);
        p10 = interp1(w5, w[8]);
        p11 = interp2(w5, w[9], w[8]);
        break;
    case 25:
        p00 = interp1(w5, w[2]);
        p01 = interp2(w5, w[3], w[2]);
        p10 = interp2(w5, w[7], w[8]);
        p11 = interp2(w5, w[9], w[8]);
        break;
    case 26:
    case 31:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        if (differs(w[2], w[6])) {
            p01 = w5;
        } else {
            p01 = interp2(w5, w[2], w[6]);
        }
        p10 = interp2(w5, w[7], w[8]);
        p11 = interp2(w5, w[9], w[8]);
        break;
    case 82:
    case 214:
        p00 = interp2(w5, w[1], w[4]);
        if (differs(w[2], w[6])) {
            p01 = w5;
        } else {
            p01 = interp2(w5, w[2], w[6]);
        }
        p10 = interp2(w5, w[7], w[4]);
        if (differs(w[6], w[8])) {
            p11 = w5;
        } else {
            p11 = interp2(w5, w[6], w[8]);
        }
        break;
    case 88:
    case 248:
        p00 = interp2(w5, w[1], w[2]);
        p01 = interp2(w5, w[3], w[2]);
        if (differs(w[8], w[4])) {
            p10 = w5;
        } else {
            p10 = interp2(w5, w[8], w[4]);
        }
        if (differs(w[6], w[8])) {
            p11 = w5;
        } else {
            p11 = interp2(w5, w[6], w[8]);
        }
        break;
    case 74:
    case 107:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = interp2(w5, w[3], w[6]);
        if (differs(w[8], w[4])) {
            p10 = w5;
        } else {
            p10 = interp2(w5, w[8], w[4]);
        }
        p11 = interp2(w5, w[9], w[6]);
        break;
    case 27:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = interp1(w5, w[3]);
        p10 = interp2(w5, w[7], w[8]);
        p11 = interp2(w5, w[9], w[8]);
        break;
    case 86:
        p00 = interp2(w5, w[1], w[4]);
        if (differs(w[2], w[6])) {
            p01 = w5;
        } else {
            p01 = interp2(w5, w[2], w[6]);
        }
        p10 = interp2(w5, w[7], w[4]);
        p11 = interp1(w5, w[9]);
        break;
    case 216:
        p00 = interp2(w5, w[1], w[2]);
        p01 = interp2(w5, w[3], w[2]);
        p10 = interp1(w5, w[7]);
        if (differs(w[6], w[8])) {
            p11 = w5;
        } else {
            p11 = interp2(w5, w[6], w[8]);
        }
        break;
    case 106:
        p00 = interp1(w5, w[1]);
        p01 = interp2(w5, w[3], w[6]);
        if (differs(w[8], w[4])) {
            p10 = w5;
        } else {
            p10 = interp2(w5, w[8], w[4]);
        }
        p11 = interp2(w5, w[9], w[6]);
        break;
    case 30:
        p00 = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            p01 = w5;
        } else {
            p01 = interp2(w5, w[2], w[6]);
        }
        p10 = interp2(w5, w[7], w[8]);
        p11 = interp2(w5, w[9], w[8]);
        break;
    case 210:
        p00 = interp2(w5, w[1], w[4]);
        p01 = interp1(w5, w[3]);
        p10 = interp2(w5, w[7], w[4]);
        if (differs(w[6], w[8])) {
            p11 = w5;
        } else {
            p11 = interp2(w5, w[6], w[8]);
        }
        break;
    case 120:
        p00 = interp2(w5, w[1], w[2]);
        p01 = interp2(w5, w[3], w[2]);
        if (differs(w[8], w[4])) {
            p10 = w5;
        } else {
            p10 = interp2(w5, w[8], w[4]);
        }
        p11 = interp1(w5, w[9]);
        break;
    case 75:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = interp2(w5, w[3], w[6]);
        p10 = interp1(w5, w[7]);
        p11 = interp2(w5, w[9], w[6]);
        break;
    case 29:
        p00 = interp1(w5, w[2]);
        p01 = interp1(w5, w[2]);
        p10 = interp2(w5, w[7], w[8]);
        p11 = interp2(w5, w[9], w[8]);
        break;
    case 198:
        p00 = interp2(w5, w[1], w[4]);
        p01 = interp1(w5, w[6]);
        p10 = interp2(w5, w[7], w[4]);
        p11 = interp1(w5, w[6]);
        break;
    case 184:
        p00 = interp2(w5, w[1], w[2]);
        p01 = interp2(w5, w[3], w[2]);
        p10 = interp1(w5, w[8]);
        p11 = interp1(w5, w[8]);
        break;
    case 99:
        p00 = interp1(w5, w[4]);
        p01 = interp2(w5, w[3], w[6]);
        p10 = interp1(w5, w[4]);
        p11 = interp2(w5, w[9], w[6]);
        break;
    case 57:
        p00 = interp1(w5, w[2]);
        p01 = interp2(w5, w[3], w[2]);
        p10 = interp1(w5, w[8]);
        p11 = interp2(w5, w[9], w[8]);
        break;
    case 71:
        p00 = interp1(w5, w[4]);
        p01 = interp1(w5, w[6]);
        p10 = interp2(w5, w[7], w[4]);
        p11 = interp2(w5, w[9], w[6]);
        break;
    case 156:
        p00 = interp2(w5, w[1], w[2]);
        p01 = interp1(w5, w[2]);
        p10 = interp2(w5, w[7], w[8]);
        p11 = interp1(w5, w[8]);
        break;
    case 226:
        p00 = interp2(w5, w[1], w[4]);
        p01 = interp2(w5, w[3], w[6]);
        p10 = interp1(w5, w[4]);
        p11 = interp1(w5, w[6]);
        break;
    case 60:
        p00 = interp2(w5, w[1], w[2]);
        p01 = interp1(w5, w[2]);
        p10 = interp1(w5, w[8]);
        p11 = interp2(w5, w[9], w[8]);
        break;
    case 195:
        p00 = interp1(w5, w[4]);
        p01 = interp2(w5, w[3], w[6]);
        p10 = interp2(w5, w[7], w[4]);
        p11 = interp1(w5, w[6]);
        break;
    case 102:
        p00 = interp2(w5, w[1], w[4]);
        p01 = interp1(w5, w[6]);
        p10 = interp1(w5, w[4]);
        p11 = interp2(w5, w[9], w[6]);
        break;
    case 153:
        p00 = interp1(w5, w[2]);
        p01 = interp2(w5, w[3], w[2]);
        p10 = interp2(w5, w[7], w[8]);
        p11 = interp1(w5, w[8]);
        break;
    case 58:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
        } else {
            p00 = interp7(w5, w[4], w[2]);
        }
        if (differs(w[2], w[6])) {
            p01 = interp1(w5, w[3]);
        } else {
            p01 = interp7(w5, w[2], w[6]);
        }
        p10 = interp1(w5, w[8]);
        p11 = interp2(w5, w[9], w[8]);
        break;
    case 83:
        p00 = interp1(w5, w[4]);
        if (differs(w[2], w[6])) {
            p01 = interp1(w5, w[3]);
        } else {
            p01 = interp7(w5, w[2], w[6]);
        }
        p10 = interp2(w5, w[7], w[4]);
        if (differs(w[6], w[8])) {
            p11 = interp1(w5, w[9]);
        } else {
            p11 = interp7(w5, w[6], w[8]);
        }
        break;
    case 92:
        p00 = interp2(w5, w[1], w[2]);
        p01 = interp1(w5, w[2]);
        if (differs(w[8], w[4])) {
            p10 = interp1(w5, w[7]);
        } else {
            p10 = interp7(w5, w[8], w[4]);
        }
        if (differs(w[6], w[8])) {
            p11 = interp1(w5, w[9]);
        } else {
            p11 = interp7(w5, w[6], w[8]);
        }
        break;
    case 202:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
        } else {
            p00 = interp7(w5, w[4], w[2]);
        }
        p01 = interp2(w5, w[3], w[6]);
        if (differs(w[8], w[4])) {
            p10 = interp1(w5, w[7]);
        } else {
            p10 = interp7(w5, w[8], w[4]);
        }
        p11 = interp1(w5, w[6]);
        break;
    case 78:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
        } else {
            p00 = interp7(w5, w[4], w[2]);
        }
        p01 = interp1(w5, w[6]);
        if (differs(w[8], w[4])) {
            p10 = interp1(w5, w[7]);
        } else {
            p10 = interp7(w5, w[8], w[4]);
        }
        p11 = interp2(w5, w[9], w[6]);
        break;
    case 154:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
        } else {
            p00 = interp7(w5, w[4], w[2]);
        }
        if (differs(w[2], w[6])) {
            p01 = interp1(w5, w[3]);
        } else {
            p01 = interp7(w5, w[2], w[6]);
        }
        p10 = interp2(w5, w[7], w[8]);
        p11 = interp1(w5, w[8]);
        break;
    case 114:
        p00 = interp2(w5, w[1], w[4]);
        if (differs(w[2], w[6])) {
            p01 = interp1(w5, w[3]);
        } else {
            p01 = interp7(w5, w[2], w[6]);
        }
        p10 = interp1(w5, w[4]);
        if (differs(w[6], w[8])) {
            p11 = interp1(w5, w[9]);
        } else {
            p11 = interp7(w5, w[6], w[8]);
        }
        break;
    case 89:
        p00 = interp1(w5, w[2]);
        p01 = interp2(w5, w[3], w[2]);
        if (differs(w[8], w[4])) {
            p10 = interp1(w5, w[7]);
        } else {
            p10 = interp7(w5, w[8], w[4]);
        }
        if (differs(w[6], w[8])) {
            p11 = interp1(w5, w[9]);
        } else {
            p11 = interp7(w5, w[6], w[8]);
        }
        break;
    case 90:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
        } else {
            p00 = interp7(w5, w[4], w[2]);
        }
        if (differs(w[2], w[6])) {
            p01 = interp1(w5, w[3]);
        } else {
            p01 = interp7(w5, w[2], w[6]);
        }
        if (differs(w[8], w[4])) {
            p10 = interp1(w5, w[7]);
        } else {
            p10 = interp7(w5, w[8], w[4]);
        }
        if (differs(w[6], w[8])) {
            p11 = interp1(w5, w[9]);
        } else {
            p11 = interp7(w5, w[6], w[8]);
        }
        break;
    case 23:
    case 55:
        if (differs(w[2], w[6])) {
            p00 = interp1(w5, w[4]);
            p01 = w5;
        } else {
            p00 = interp6(w5, w[2], w[4]);
            p01 = interp9(w5, w[2], w[6]);
        }
        p10 = interp2(w5, w[8], w[4]);
        p11 = interp2(w5, w[9], w[8]);
        break;
    case 150:
    case 182:
        p00 = interp2(w5, w[1], w[4]);
        if (differs(w[2], w[6])) {
            p01 = w5;
            p11 = interp1(w5, w[8]);
        } else {
            p01 = interp9(w5, w[2], w[6]);
            p11 = interp6(w5, w[6], w[8]);
        }
        p10 = interp2(w5, w[8], w[4]);
        break;
    case 212:
    case 213:
        p00 = interp2(w5, w[4], w[2]);
        if (differs(w[6], w[8])) {
            p01 = interp1(w5, w[2]);
            p11 = w5;
        } else {
            p01 = interp6(w5, w[6], w[2]);
            p11 = interp9(w5, w[6], w[8]);
        }
        p10 = interp2(w5, w[7], w[4]);
        break;
    case 240:
    case 241:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp2(w5, w[3], w[2]);
        if (differs(w[6], w[8])) {
            p10 = interp1(w5, w[4]);
            p11 = w5;
        } else {
            p10 = interp6(w5, w[8], w[4]);
            p11 = interp9(w5, w[6], w[8]);
        }
        break;
    case 232:
    case 236:
        p00 = interp2(w5, w[1], w[2]);
        p01 = interp2(w5, w[2], w[6]);
        if (differs(w[8], w[4])) {
            p10 = w5;
            p11 = interp1(w5, w[6]);
        } else {
            p10 = interp9(w5, w[8], w[4]);
            p11 = interp6(w5, w[8], w[6]);
        }
        break;
    case 105:
    case 109:
        if (differs(w[8], w[4])) {
            p00 = interp1(w5, w[2]);
            p10 = w5;
        } else {
            p00 = interp6(w5, w[4], w[2]);
            p10 = interp9(w5, w[8], w[4]);
        }
        p01 = interp2(w5, w[2], w[6]);
        p11 = interp2(w5, w[9], w[6]);
        break;
    case 43:
    case 171:
        if (differs(w[4], w[2])) {
            p00 = w5;
            p10 = interp1(w5, w[8]);
        } else {
            p00 = interp9(w5, w[4], w[2]);
            p10 = interp6(w5, w[4], w[8]);
        }
        p01 = interp2(w5, w[3], w[6]);
        p11 = interp2(w5, w[6], w[8]);
        break;
    case 15:
    case 143:
        if (differs(w[4], w[2])) {
            p00 = w5;
            p01 = interp1(w5, w[6]);
        } else {
            p00 = interp9(w5, w[4], w[2]);
            p01 = interp6(w5, w[2], w[6]);
        }
        p10 = interp2(w5, w[7], w[8]);
        p11 = interp2(w5, w[6], w[8]);
        break;
    case 124:
        p00 = interp2(w5, w[1], w[2]);
        p01 = interp1(w5, w[2]);
        if (differs(w[8], w[4])) {
            p10 = w5;
        } else {
            p10 = interp2(w5, w[8], w[4]);
        }
        p11 = interp1(w5, w[9]);
        break;
    case 203:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = interp2(w5, w[3], w[6]);
        p10 = interp1(w5, w[7]);
        p11 = interp1(w5, w[6]);
        break;
    case 62:
        p00 = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            p01 = w5;
        } else {
            p01 = interp2(w5, w[2], w[6]);
        }
        p10 = interp1(w5, w[8]);
        p11 = interp2(w5, w[9], w[8]);
        break;
    case 211:
        p00 = interp1(w5, w[4]);
        p01 = interp1(w5, w[3]);
        p10 = interp2(w5, w[7], w[4]);
        if (differs(w[6], w[8])) {
            p11 = w5;
        } else {
            p11 = interp2(w5, w[6], w[8]);
        }
        break;
    case 118:
        p00 = interp2(w5, w[1], w[4]);
        if (differs(w[2], w[6])) {
            p01 = w5;
        } else {
            p01 = interp2(w5, w[2], w[6]);
        }
        p10 = interp1(w5, w[4]);
        p11 = interp1(w5, w[9]);
        break;
    case 217:
        p00 = interp1(w5, w[2]);
        p01 = interp2(w5, w[3], w[2]);
        p10 = interp1(w5, w[7]);
        if (differs(w[6], w[8])) {
            p11 = w5;
        } else {
            p11 = interp2(w5, w[6], w[8]);
        }
        break;
    case 110:
        p00 = interp1(w5, w[1]);
        p01 = interp1(w5, w[6]);
        if (differs(w[8], w[4])) {
            p10 = w5;
        } else {
            p10 = interp2(w5, w[8], w[4]);
        }
        p11 = interp2(w5, w[9], w[6]);
        break;
    case 155:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = interp1(w5, w[3]);
        p10 = interp2(w5, w[7], w[8]);
        p11 = interp1(w5, w[8]);
        break;
    case 188:
        p00 = interp2(w5, w[1], w[2]);
        p01 = interp1(w5, w[2]);
        p10 = interp1(w5, w[8]);
        p11 = interp1(w5, w[8]);
        break;
    case 185:
        p00 = interp1(w5, w[2]);
        p01 = interp2(w5, w[3], w[2]);
        p10 = interp1(w5, w[8]);
        p11 = interp1(w5, w[8]);
        break;
    case 61:
        p00 = interp1(w5, w[2]);
        p01 = interp1(w5, w[2]);
        p10 = interp1(w5, w[8]);
        p11 = interp2(w5, w[9], w[8]);
        break;
    case 157:
        p00 = interp1(w5, w[2]);
        p01 = interp1(w5, w[2]);
        p10 = interp2(w5, w[7], w[8]);
        p11 = interp1(w5, w[8]);
        break;
    case 103:
        p00 = interp1(w5, w[4]);
        p01 = interp1(w5, w[6]);
        p10 = interp1(w5, w[4]);
        p11 = interp2(w5, w[9], w[6]);
        break;
    case 227:
        p00 = interp1(w5, w[4]);
        p01 = interp2(w5, w[3], w[6]);
        p10 = interp1(w5, w[4]);
        p11 = interp1(w5, w[6]);
        break;
    case 230:
        p00 = interp2(w5, w[1], w[4]);
        p01 = interp1(w5, w[6]);
        p10 = interp1(w5, w[4]);
        p11 = interp1(w5, w[6]);
        break;
    case 199:
        p00 = interp1(w5, w[4]);
        p01 = interp1(w5, w[6]);
        p10 = interp2(w5, w[7], w[4]);
        p11 = interp1(w5, w[6]);
        break;
    case 220:
        p00 = interp2(w5, w[1], w[2]);
        p01 = interp1(w5, w[2]);
        if (differs(w[8], w[4])) {
            p10 = interp1(w5, w[7]);
        } else {
            p10 = interp7(w5, w[8], w[4]);
        }
        if (differs(w[6], w[8])) {
            p11 = w5;
        } else {
            p11 = interp2(w5, w[6], w[8]);
        }
        break;
    case 158:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
        } else {
            p00 = interp7(w5, w[4], w[2]);
        }
        if (differs(w[2], w[6])) {
            p01 = w5;
        } else {
            p01 = interp2(w5, w[2], w[6]);
        }
        p10 = interp2(w5, w[7], w[8]);
        p11 = interp1(w5, w[8]);
        break;
    case 234:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
        } else {
            p00 = interp7(w5, w[4], w[2]);
        }
        p01 = interp2(w5, w[3], w[6]);
        if (differs(w[8], w[4])) {
            p10 = w5;
        } else {
            p10 = interp2(w5, w[8], w[4]);
        }
        p11 = interp1(w5, w[6]);
        break;
    case 242:
        p00 = interp2(w5, w[1], w[4]);
        if (differs(w[2], w[6])) {
            p01 = interp1(w5, w[3]);
        } else {
            p01 = interp7(w5, w[2], w[6]);
        }
        p10 = interp1(w5, w[4]);
        if (differs(w[6], w[8])) {
            p11 = w5;
        } else {
            p11 = interp2(w5, w[6], w[8]);
        }
        break;
    case 59:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        if (differs(w[2], w[6])) {
            p01 = interp1(w5, w[3]);
        } else {
            p01 = interp7(w5, w[2], w[6]);
        }
        p10 = interp1(w5, w[8]);
        p11 = interp2(w5, w[9], w[8]);
        break;
    case 121:
        p00 = interp1(w5, w[2]);
        p01 = interp2(w5, w[3], w[2]);
        if (differs(w[8], w[4])) {
            p10 = w5;
        } else {
            p10 = interp2(w5, w[8], w[4]);
        }
        if (differs(w[6], w[8])) {
            p11 = interp1(w5, w[9]);
        } else {
            p11 = interp7(w5, w[6], w[8]);
        }
        break;
    case 87:
        p00 = interp1(w5, w[4]);
        if (differs(w[2], w[6])) {
            p01 = w5;
        } else {
            p01 = interp2(w5, w[2], w[6]);
        }
        p10 = interp2(w5, w[7], w[4]);
        if (differs(w[6], w[8])) {
            p11 = interp1(w5, w[9]);
        } else {
            p11 = interp7(w5, w[6], w[8]);
        }
        break;
    case 79:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = interp1(w5, w[6]);
        if (differs(w[8], w[4])) {
            p10 = interp1(w5, w[7]);
        } else {
            p10 = interp7(w5, w[8], w[4]);
        }
        p11 = interp2(w5, w[9], w[6]);
        break;
    case 122:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
        } else {
            p00 = interp7(w5, w[4], w[2]);
        }
        if (differs(w[2], w[6])) {
            p01 = interp1(w5, w[3]);
        } else {
            p01 = interp7(w5, w[2], w[6]);
        }
        if (differs(w[8], w[4])) {
            p10 = w5;
        } else {
            p10 = interp2(w5, w[8], w[4]);
        }
        if (differs(w[6], w[8])) {
            p11 = interp1(w5, w[9]);
        } else {
            p11 = interp7(w5, w[6], w[8]);
        }
        break;
    case 94:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
        } else {
            p00 = interp7(w5, w[4], w[2]);
        }
        if (differs(w[2], w[6])) {
            p01 = w5;
        } else {
            p01 = interp2(w5, w[2], w[6]);
        }
        if (differs(w[8], w[4])) {
            p10 = interp1(w5, w[7]);
        } else {
            p10 = interp7(w5, w[8], w[4]);
        }
        if (differs(w[6], w[8])) {
            p11 = interp1(w5, w[9]);
        } else {
            p11 = interp7(w5, w[6], w[8]);
        }
        break;
    case 218:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
        } else {
            p00 = interp7(w5, w[4], w[2]);
        }
        if (differs(w[2], w[6])) {
            p01 = interp1(w5, w[3]);
        } else {
            p01 = interp7(w5, w[2], w[6]);
        }
        if (differs(w[8], w[4])) {
            p10 = interp1(w5, w[7]);
        } else {
            p10 = interp7(w5, w[8], w[4]);
        }
        if (differs(w[6], w[8])) {
            p11 = w5;
        } else {
            p11 = interp2(w5, w[6], w[8]);
        }
        break;
    case 91:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        if (differs(w[2], w[6])) {
            p01 = interp1(w5, w[3]);
        } else {
            p01 = interp7(w5, w[2], w[6]);
        }
        if (differs(w[8], w[4])) {
            p10 = interp1(w5, w[7]);
        } else {
            p10 = interp7(w5, w[8], w[4]);
        }
        if (differs(w[6], w[8])) {
            p11 = interp1(w5, w[9]);
        } else {
            p11 = interp7(w5, w[6], w[8]);
        }
        break;
    case 229:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp2(w5, w[2], w[6]);
        p10 = interp1(w5, w[4]);
        p11 = interp1(w5, w[6]);
        break;
    case 167:
        p00 = interp1(w5, w[4]);
        p01 = interp1(w5, w[6]);
        p10 = interp2(w5, w[8], w[4]);
        p11 = interp2(w5, w[6], w[8]);
        break;
    case 173:
        p00 = interp1(w5, w[2]);
        p01 = interp2(w5, w[2], w[6]);
        p10 = interp1(w5, w[8]);
        p11 = interp2(w5, w[6], w[8]);
        break;
    case 181:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp1(w5, w[2]);
        p10 = interp2(w5, w[8], w[4]);
        p11 = interp1(w5, w[8]);
        break;
    case 186:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
        } else {
            p00 = interp7(w5, w[4], w[2]);
        }
        if (differs(w[2], w[6])) {
            p01 = interp1(w5, w[3]);
        } else {
            p01 = interp7(w5, w[2], w[6]);
        }
        p10 = interp1(w5, w[8]);
        p11 = interp1(w5, w[8]);
        break;
    case 115:
        p00 = interp1(w5, w[4]);
        if (differs(w[2], w[6])) {
            p01 = interp1(w5, w[3]);
        } else {
            p01 = interp7(w5, w[2], w[6]);
        }
        p10 = interp1(w5, w[4]);
        if (differs(w[6], w[8])) {
            p11 = interp1(w5, w[9]);
        } else {
            p11 = interp7(w5, w[6], w[8]);
        }
        break;
    case 93:
        p00 = interp1(w5, w[2]);
        p01 = interp1(w5, w[2]);
        if (differs(w[8], w[4])) {
            p10 = interp1(w5, w[7]);
        } else {
            p10 = interp7(w5, w[8], w[4]);
        }
        if (differs(w[6], w[8])) {
            p11 = interp1(w5, w[9]);
        } else {
            p11 = interp7(w5, w[6], w[8]);
        }
        break;
    case 206:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
        } else {
            p00 = interp7(w5, w[4], w[2]);
        }
        p01 = interp1(w5, w[6]);
        if (differs(w[8], w[4])) {
            p10 = interp1(w5, w[7]);
        } else {
            p10 = interp7(w5, w[8], w[4]);
        }
        p11 = interp1(w5, w[6]);
        break;
    case 201:
    case 205:
        p00 = interp1(w5, w[2]);
        p01 = interp2(w5, w[2], w[6]);
        if (differs(w[8], w[4])) {
            p10 = interp1(w5, w[7]);
        } else {
            p10 = interp7(w5, w[8], w[4]);
        }
        p11 = interp1(w5, w[6]);
        break;
    case 46:
    case 174:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
        } else {
            p00 = interp7(w5, w[4], w[2]);
        }
        p01 = interp1(w5, w[6]);
        p10 = interp1(w5, w[8]);
        p11 = interp2(w5, w[6], w[8]);
        break;
    case 147:
    case 179:
        p00 = interp1(w5, w[4]);
        if (differs(w[2], w[6])) {
            p01 = interp1(w5, w[3]);
        } else {
            p01 = interp7(w5, w[2], w[6]);
        }
        p10 = interp2(w5, w[8], w[4]);
        p11 = interp1(w5, w[8]);
        break;
    case 116:
    case 117:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp1(w5, w[2]);
        p10 = interp1(w5, w[4]);
        if (differs(w[6], w[8])) {
            p11 = interp1(w5, w[9]);
        } else {
            p11 = interp7(w5, w[6], w[8]);
        }
        break;
    case 189:
        p00 = interp1(w5, w[2]);
        p01 = interp1(w5, w[2]);
        p10 = interp1(w5, w[8]);
        p11 = interp1(w5, w[8]);
        break;
    case 231:
        p00 = interp1(w5, w[4]);
        p01 = interp1(w5, w[6]);
        p10 = interp1(w5, w[4]);
        p11 = interp1(w5, w[6]);
        break;
    case 126:
        p00 = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            p01 = w5;
        } else {
            p01 = interp2(w5, w[2], w[6]);
        }
        if (differs(w[8], w[4])) {
            p10 = w5;
        } else {
            p10 = interp2(w5, w[8], w[4]);
        }
        p11 = interp1(w5, w[9]);
        break;
    case 219:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = interp1(w5, w[3]);
        p10 = interp1(w5, w[7]);
        if (differs(w[6], w[8])) {
            p11 = w5;
        } else {
            p11 = interp2(w5, w[6], w[8]);
        }
        break;
    case 125:
        if (differs(w[8], w[4])) {
            p00 = interp1(w5, w[2]);
            p10 = w5;
        } else {
            p00 = interp6(w5, w[4], w[2]);
            p10 = interp9(w5, w[8], w[4]);
        }
        p01 = interp1(w5, w[2]);
        p11 = interp1(w5, w[9]);
        break;
    case 221:
        p00 = interp1(w5, w[2]);
        if (differs(w[6], w[8])) {
            p01 = interp1(w5, w[2]);
            p11 = w5;
        } else {
            p01 = interp6(w5, w[6], w[2]);
            p11 = interp9(w5, w[6], w[8]);
        }
        p10 = interp1(w5, w[7]);
        break;
    case 207:
        if (differs(w[4], w[2])) {
            p00 = w5;
            p01 = interp1(w5, w[6]);
        } else {
            p00 = interp9(w5, w[4], w[2]);
            p01 = interp6(w5, w[2], w[6]);
        }
        p10 = interp1(w5, w[7]);
        p11 = interp1(w5, w[6]);
        break;
    case 238:
        p00 = interp1(w5, w[1]);
        p01 = interp1(w5, w[6]);
        if (differs(w[8], w[4])) {
            p10 = w5;
            p11 = interp1(w5, w[6]);
        } else {
            p10 = interp9(w5, w[8], w[4]);
            p11 = interp6(w5, w[8], w[6]);
        }
        break;
    case 190:
        p00 = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            p01 = w5;
            p11 = interp1(w5, w[8]);
        } else {
            p01 = interp9(w5, w[2], w[6]);
            p11 = interp6(w5, w[6], w[8]);
        }
        p10 = interp1(w5, w[8]);
        break;
    case 187:
        if (differs(w[4], w[2])) {
            p00 = w5;
            p10 = interp1(w5, w[8]);
        } else {
            p00 = interp9(w5, w[4], w[2]);
            p10 = interp6(w5, w[4], w[8]);
        }
        p01 = interp1(w5, w[3]);
        p11 = interp1(w5, w[8]);
        break;
    case 243:
        p00 = interp1(w5, w[4]);
        p01 = interp1(w5, w[3]);
        if (differs(w[6], w[8])) {
            p10 = interp1(w5, w[4]);
            p11 = w5;
        } else {
            p10 = interp6(w5, w[8], w[4]);
            p11 = interp9(w5, w[6], w[8]);
        }
        break;
    case 119:
        if (differs(w[2], w[6])) {
            p00 = interp1(w5, w[4]);
            p01 = w5;
        } else {
            p00 = interp6(w5, w[2], w[4]);
            p01 = interp9(w5, w[2], w[6]);
        }
        p10 = interp1(w5, w[4]);
        p11 = interp1(w5, w[9]);
        break;
    case 233:
    case 237:
        p00 = interp1(w5, w[2]);
        p01 = interp2(w5, w[2], w[6]);
        if (differs(w[8], w[4])) {
            p10 = w5;
        } else {
            p10 = interp10(w5, w[8], w[4]);
        }
        p11 = interp1(w5, w[6]);
        break;
    case 47:
    case 175:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp10(w5, w[4], w[2]);
        }
        p01 = interp1(w5, w[6]);
        p10 = interp1(w5, w[8]);
        p11 = interp2(w5, w[6], w[8]);
        break;
    case 151:
    case 183:
        p00 = interp1(w5, w[4]);
        if (differs(w[2], w[6])) {
            p01 = w5;
        } else {
            p01 = interp10(w5, w[2], w[6]);
        }
        p10 = interp2(w5, w[8], w[4]);
        p11 = interp1(w5, w[8]);
        break;
    case 244:
    case 245:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp1(w5, w[2]);
        p10 = interp1(w5, w[4]);
        if (differs(w[6], w[8])) {
            p11 = w5;
        } else {
            p11 = interp10(w5, w[6], w[8]);
        }
        break;
    case 250:
        p00 = interp1(w5, w[1]);
        p01 = interp1(w5, w[3]);
        if (differs(w[8], w[4])) {
            p10 = w5;
        } else {
            p10 = interp2(w5, w[8], w[4]);
        }
        if (differs(w[6], w[8])) {
            p11 = w5;
        } else {
            p11 = interp2(w5, w[6], w[8]);
        }
        break;
    case 123:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = interp1(w5, w[3]);
        if (differs(w[8], w[4])) {
            p10 = w5;
        } else {
            p10 = interp2(w5, w[8], w[4]);
        }
        p11 = interp1(w5, w[9]);
        break;
    case 95:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        if (differs(w[2], w[6])) {
            p01 = w5;
        } else {
            p01 = interp2(w5, w[2], w[6]);
        }
        p10 = interp1(w5, w[7]);
        p11 = interp1(w5, w[9]);
        break;
    case 222:
        p00 = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            p01 = w5;
        } else {
            p01 = interp2(w5, w[2], w[6]);
        }
        p10 = interp1(w5, w[7]);
        if (differs(w[6], w[8])) {
            p11 = w5;
        } else {
            p11 = interp2(w5, w[6], w[8]);
        }
        break;
    case 252:
        p00 = interp2(w5, w[1], w[2]);
        p01 = interp1(w5, w[2]);
        if (differs(w[8], w[4])) {
            p10 = w5;
        } else {
            p10 = interp2(w5, w[8], w[4]);
        }
        if (differs(w[6], w[8])) {
            p11 = w5;
        } else {
            p11 = interp10(w5, w[6], w[8]);
        }
        break;
    case 249:
        p00 = interp1(w5, w[2]);
        p01 = interp2(w5, w[3], w[2]);
        if (differs(w[8], w[4])) {
            p10 = w5;
        } else {
            p10 = interp10(w5, w[8], w[4]);
        }
        if (differs(w[6], w[8])) {
            p11 = w5;
        } else {
            p11 = interp2(w5, w[6], w[8]);
        }
        break;
    case 235:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = interp2(w5, w[3], w[6]);
        if (differs(w[8], w[4])) {
            p10 = w5;
        } else {
            p10 = interp10(w5, w[8], w[4]);
        }
        p11 = interp1(w5, w[6]);
        break;
    case 111:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp10(w5, w[4], w[2]);
        }
        p01 = interp1(w5, w[6]);
        if (differs(w[8], w[4])) {
            p10 = w5;
        } else {
            p10 = interp2(w5, w[8], w[4]);
        }
        p11 = interp2(w5, w[9], w[6]);
        break;
    case 63:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp10(w5, w[4], w[2]);
        }
        if (differs(w[2], w[6])) {
            p01 = w5;
        } else {
            p01 = interp2(w5, w[2], w[6]);
        }
        p10 = interp1(w5, w[8]);
        p11 = interp2(w5, w[9], w[8]);
        break;
    case 159:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        if (differs(w[2], w[6])) {
            p01 = w5;
        } else {
            p01 = interp10(w5, w[2], w[6]);
        }
        p10 = interp2(w5, w[7], w[8]);
        p11 = interp1(w5, w[8]);
        break;
    case 215:
        p00 = interp1(w5, w[4]);
        if (differs(w[2], w[6])) {
            p01 = w5;
        } else {
            p01 = interp10(w5, w[2], w[6]);
        }
        p10 = interp2(w5, w[7], w[4]);
        if (differs(w[6], w[8])) {
            p11 = w5;
        } else {
            p11 = interp2(w5, w[6], w[8]);
        }
        break;
    case 246:
        p00 = interp2(w5, w[1], w[4]);
        if (differs(w[2], w[6])) {
            p01 = w5;
        } else {
            p01 = interp2(w5, w[2], w[6]);
        }
        p10 = interp1(w5, w[4]);
        if (differs(w[6], w[8])) {
            p11 = w5;
        } else {
            p11 = interp10(w5, w[6], w[8]);
        }
        break;
    case 254:
        p00 = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            p01 = w5;
        } else {
            p01 = interp2(w5, w[2], w[6]);
        }
        if (differs(w[8], w[4])) {
            p10 = w5;
        } else {
            p10 = interp2(w5, w[8], w[4]);
        }
        if (differs(w[6], w[8])) {
            p11 = w5;
        } else {
            p11 = interp10(w5, w[6], w[8]);
        }
        break;
    case 253:
        p00 = interp1(w5, w[2]);
        p01 = interp1(w5, w[2]);
        if (differs(w[8], w[4])) {
            p10 = w5;
        } else {
            p10 = interp10(w5, w[8], w[4]);
        }
        if (differs(w[6], w[8])) {
            p11 = w5;
        } else {
            p11 = interp10(w5, w[6], w[8]);
        }
        break;
    case 251:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = interp1(w5, w[3]);
        if (differs(w[8], w[4])) {
            p10 = w5;
        } else {
            p10 = interp10(w5, w[8], w[4]);
        }
        if (differs(w[6], w[8])) {
            p11 = w5;
        } else {
            p11 = interp2(w5, w[6], w[8]);
        }
        break;
    case 239:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp10(w5, w[4], w[2]);
        }
        p01 = interp1(w5, w[6]);
        if (differs(w[8], w[4])) {
            p10 = w5;
        } else {
            p10 = interp10(w5, w[8], w[4]);
        }
        p11 = interp1(w5, w[6]);
        break;
    case 127:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp10(w5, w[4], w[2]);
        }
        if (differs(w[2], w[6])) {
            p01 = w5;
        } else {
            p01 = interp2(w5, w[2], w[6]);
        }
        if (differs(w[8], w[4])) {
            p10 = w5;
        } else {
            p10 = interp2(w5, w[8], w[4]);
        }
        p11 = interp1(w5, w[9]);
        break;
    case 191:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp10(w5, w[4], w[2]);
        }
        if (differs(w[2], w[6])) {
            p01 = w5;
        } else {
            p01 = interp10(w5, w[2], w[6]);
        }
        p10 = interp1(w5, w[8]);
        p11 = interp1(w5, w[8]);
        break;
    case 223:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        if (differs(w[2], w[6])) {
            p01 = w5;
        } else {
            p01 = interp10(w5, w[2], w[6]);
        }
        p10 = interp1(w5, w[7]);
        if (differs(w[6], w[8])) {
            p11 = w5;
        } else {
            p11 = interp2(w5, w[6], w[8]);
        }
        break;
    case 247:
        p00 = interp1(w5, w[4]);
        if (differs(w[2], w[6])) {
            p01 = w5;
        } else {
            p01 = interp10(w5, w[2], w[6]);
        }
        p10 = interp1(w5, w[4]);
        if (differs(w[6], w[8])) {
            p11 = w5;
        } else {
            p11 = interp10(w5, w[6], w[8]);
        }
        break;
    case 255:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp10(w5, w[4], w[2]);
        }
        if (differs(w[2], w[6])) {
            p01 = w5;
        } else {
            p01 = interp10(w5, w[2], w[6]);
        }
        if (differs(w[8], w[4])) {
            p10 = w5;
        } else {
            p10 = interp10(w5, w[8], w[4]);
        }
        if (differs(w[6], w[8])) {
            p11 = w5;
        } else {
            p11 = interp10(w5, w[6], w[8]);
        }
        break;
    default:
        p00 = p01 = p10 = p11 = w5;
        break;
    }

    st16u(dst, p00);
    st16u(dst + 2, p01);
    st16u(dst + pitch, p10);
    st16u(dst + pitch + 2, p11);
}

/* Source lines are 288 pixels apart: 256 drawn plus the 32-pixel skip. */
#define SRC_LINE 288

void hq2x_16b(void)
{
    if (curblank == 0x40)
        return;

    /* The assembly ran the doubler when the filter was off or when an
       unfiltered GUI was up; keep both. */
    if (!hqFilter || (!FilteredGUI && GUIOn2 == 1)) {
        hq2x_double_16b();
        return;
    }

    u2 const* const base = SRC_START;
    u1* dst = WinVidMemStart;
    u4 const pitch = NumBytesPerLine;
    u1 const* const linetype = (GUIOn != 1 && newengen != 0) ? SpecialLine + 1
                                                             : hirestiledat + 1;
    u4 const lines = resolutn;

    for (u4 y = 0; y < lines; y++) {
        u2 const* const row = base + (size_t)y * SRC_LINE;
        long const up = y == 0 ? 0 : -SRC_LINE;
        long const dn = y + 1 == lines ? 0 : SRC_LINE;

        if (linetype[y] > 1) {
            /* A hi-res line is already 512 wide; the assembly doubled it
               rather than filtering, and so does this. */
            for (u4 x = 0; x < 256; x++) {
                u4 const px = row[x] * 0x00010001u;
                st32u(dst + x * 4, px);
                st32u(dst + x * 4 + pitch, px);
            }
        } else {
            u2 w[10];

            for (u4 x = 0; x < 256; x++) {
                long const l = x == 0 ? 0 : -1;
                long const r = x == 255 ? 0 : 1;

                w[1] = row[up + (long)x + l];
                w[2] = row[up + (long)x];
                w[3] = row[up + (long)x + r];
                w[4] = row[(long)x + l];
                w[5] = row[x];
                w[6] = row[(long)x + r];
                w[7] = row[dn + (long)x + l];
                w[8] = row[dn + (long)x];
                w[9] = row[dn + (long)x + r];
                hq2x_pixel(w, dst + x * 4, pitch);
            }
        }
        /* 256 pixels written as 1024 bytes, then the tail of that row and
           the doubled row below it, exactly as the assembly steps. */
        dst += 256 * 4 + AddEndBytes + pitch;
    }
}

void hq2x_32b(void)
{
    if (curblank == 0x40)
        return;
    u4 const* conv = (u4 const*)BitConv32Ptr;
    u2 const* src = SRC_START;
    u1* dst = WinVidMemStart;
    u4 const ebx = NumBytesPerLine;
    u1 lines = resolutn;
    do {
        u4 ecx = 256;
        do {
            u4 const px = conv[*src];
            *(u4*)dst = px;
            *(u4*)(dst + 4) = px;
            *(u4*)(dst + ebx) = px;
            *(u4*)(dst + ebx + 4) = px;
            src += 1;
            dst += 8;
        } while (--ecx != 0);
        dst += AddEndBytes + ebx;
        src += SRC_LINE_SKIP;
    } while (--lines != 0);
}

void hq3x_16b(void)
{
    if (curblank == 0x40)
        return;
    u2 const* src = SRC_START;
    u1* dst = WinVidMemStart;
    u4 const ebx = NumBytesPerLine;
    u1 lines = resolutn;
    do {
        u4 ecx = 256;
        do {
            u4 const px = *src * 0x00010001u;
            u2 const p = *src;
            *(u4*)dst = px;
            *(u4*)(dst + ebx) = px;
            *(u4*)(dst + ebx * 2) = px;
            *(u2*)(dst + 4) = p;
            *(u2*)(dst + ebx + 4) = p;
            *(u2*)(dst + ebx * 2 + 4) = p;
            src += 1;
            dst += 6;
        } while (--ecx != 0);
        dst += AddEndBytes + ebx * 2;
        src += SRC_LINE_SKIP;
    } while (--lines != 0);
}

void hq3x_32b(void)
{
    if (curblank == 0x40)
        return;
    u4 const* conv = (u4 const*)BitConv32Ptr;
    u2 const* src = SRC_START;
    u1* dst = WinVidMemStart;
    u4 const ebx = NumBytesPerLine;
    u1 lines = resolutn;
    do {
        u4 ecx = 256;
        do {
            u4 const px = conv[*src];
            *(u4*)dst = px;
            *(u4*)(dst + 4) = px;
            *(u4*)(dst + 8) = px;
            *(u4*)(dst + ebx) = px;
            *(u4*)(dst + ebx + 4) = px;
            *(u4*)(dst + ebx + 8) = px;
            *(u4*)(dst + ebx * 2) = px;
            *(u4*)(dst + ebx * 2 + 4) = px;
            *(u4*)(dst + ebx * 2 + 8) = px;
            src += 1;
            dst += 12;
        } while (--ecx != 0);
        dst += AddEndBytes + ebx * 2;
        src += SRC_LINE_SKIP;
    } while (--lines != 0);
}

void hq4x_16b(void)
{
    if (curblank == 0x40)
        return;
    u2 const* src = SRC_START;
    u1* dst = WinVidMemStart;
    u4 const ebx = NumBytesPerLine;
    u1 lines = resolutn;
    do {
        u4 ecx = 256;
        do {
            u4 const px = *src * 0x00010001u;
            *(u4*)dst = px;
            *(u4*)(dst + 4) = px;
            *(u4*)(dst + ebx) = px;
            *(u4*)(dst + ebx + 4) = px;
            *(u4*)(dst + ebx * 2) = px;
            *(u4*)(dst + ebx * 2 + 4) = px;
            *(u4*)(dst + ebx * 3) = px;
            *(u4*)(dst + ebx * 3 + 4) = px;
            src += 1;
            dst += 8;
        } while (--ecx != 0);
        dst += AddEndBytes + ebx * 3;
        src += SRC_LINE_SKIP;
    } while (--lines != 0);
}

void hq4x_32b(void)
{
    if (curblank == 0x40)
        return;
    u4 const* conv = (u4 const*)BitConv32Ptr;
    u2 const* src = SRC_START;
    u1* dst = WinVidMemStart;
    u4 const ebx = NumBytesPerLine;
    u1 lines = resolutn;
    do {
        u4 ecx = 256;
        do {
            u4 const px = conv[*src];
            *(u4*)dst = px;
            *(u4*)(dst + 4) = px;
            *(u4*)(dst + 8) = px;
            *(u4*)(dst + 12) = px;
            *(u4*)(dst + ebx) = px;
            *(u4*)(dst + ebx + 4) = px;
            *(u4*)(dst + ebx + 8) = px;
            *(u4*)(dst + ebx + 12) = px;
            *(u4*)(dst + ebx * 2) = px;
            *(u4*)(dst + ebx * 2 + 4) = px;
            *(u4*)(dst + ebx * 2 + 8) = px;
            *(u4*)(dst + ebx * 2 + 12) = px;
            *(u4*)(dst + ebx * 3) = px;
            *(u4*)(dst + ebx * 3 + 4) = px;
            *(u4*)(dst + ebx * 3 + 8) = px;
            *(u4*)(dst + ebx * 3 + 12) = px;
            src += 1;
            dst += 16;
        } while (--ecx != 0);
        dst += AddEndBytes + ebx * 3;
        src += SRC_LINE_SKIP;
    } while (--lines != 0);
}
