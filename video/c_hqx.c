/*
 * MaxSt's hq2x, hq3x and hq4x, ported from the MMX assembly that video/hq*.asm
 * held before 5ff6d63d removed it. Each filter is a 256-entry rule table over
 * the neighbourhood pattern; tools/hqxport.py rederives those tables from the
 * original assembly and checks them against this file.
 *
 * Each rule set serves both the 16- and 32-bit entry points: the filtering
 * happens in 565 either way, and the 32-bit writers expand the result through
 * BitConv32Ptr. With the filter off, or under an unfiltered GUI, each entry
 * point falls back to plain NxN pixel replication, as the assembly did.
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
#define SRC_START ((u2 const*)(vidbuffer) + VID_FIRST)
#define SRC_LINE_SKIP 32 // u2 units

/* A neighbourhood of one colour comes out as that colour, whatever the
   filter: every rule interpolates equal pixels back to themselves. Most of a
   SNES frame is flat, so this is where the time goes. */
static int flat9(u2 const w[10])
{
    u2 const c = w[5];

    return w[1] == c && w[2] == c && w[3] == c && w[4] == c && w[6] == c
        && w[7] == c && w[8] == c && w[9] == c;
}

static void fill_flat16(u1* const dst, u4 const pitch, unsigned const n, u2 const c)
{
    for (unsigned r = 0; r < n; r++) {
        u1* const orow = dst + (size_t)pitch * r;

        for (unsigned x = 0; x < n; x++)
            st16u(orow + x * 2, c);
    }
}

static void fill_flat32(u1* const dst, u4 const pitch, unsigned const n, u2 const c)
{
    u4 const px = ((u4 const*)BitConv32Ptr)[c];

    for (unsigned r = 0; r < n; r++) {
        u1* const orow = dst + (size_t)pitch * r;

        for (unsigned x = 0; x < n; x++)
            st32u(orow + x * 4, px);
    }
}

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

/* One source pixel's 2x2 result, row major. w[1..9] is the neighbourhood,
   w[5] the centre. Kept separate from the writers so both output depths run
   the identical rule set. */
static void hq2x_quad(u2 const w[10], u2 out[4])
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

    out[0] = p00;
    out[1] = p01;
    out[2] = p10;
    out[3] = p11;
}

static void hq2x_pixel(u2 const w[10], u1* const dst, u4 const pitch)
{
    u2 q[4];

    hq2x_quad(w, q);
    st16u(dst, q[0]);
    st16u(dst + 2, q[1]);
    st16u(dst + pitch, q[2]);
    st16u(dst + pitch + 2, q[3]);
}

/* The same block through the 16->32bit table: identical filtering, expanded
   on the way out. */
static void hq2x_pixel32(u2 const w[10], u1* const dst, u4 const pitch)
{
    u4 const* const conv = (u4 const*)BitConv32Ptr;
    u2 q[4];

    hq2x_quad(w, q);
    st32u(dst, conv[q[0]]);
    st32u(dst + 4, conv[q[1]]);
    st32u(dst + pitch, conv[q[2]]);
    st32u(dst + pitch + 4, conv[q[3]]);
}

/* Source lines are VID_STRIDE pixels apart: 256 drawn plus the 32-pixel skip. */
#define SRC_LINE VID_STRIDE

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
                if (flat9(w))
                    fill_flat16(dst + x * 4, pitch, 2, w[5]);
                else
                    hq2x_pixel(w, dst + x * 4, pitch);
            }
        }
        /* 256 pixels written as 1024 bytes, then the tail of that row and
           the doubled row below it, exactly as the assembly steps. */
        dst += 256 * 4 + AddEndBytes + pitch;
    }
}

static void hq2x_double_32b(void)
{
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

static void hq3x_double_16b(void)
{
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

/* ---- hq3x -------------------------------------------------------------- *
 *
 * Same filter one size up, from video/hq3x16.asm. The pattern and the
 * difference test are hq2x's; only the weights and the nine output pixels
 * differ, and the centre is always the source pixel.
 */

/* (a + b) / 2, the plain masked half-mix. */
static u2 interp5(u2 const a, u2 const b)
{
    u4 edx = a, ecx = b;

    if (edx == ecx)
        return (u2)edx;
    edx &= HalfTrans[0];
    ecx &= HalfTrans[0];
    return (u2)((edx + ecx) >> 1);
}

/* (7a + b) / 8 and (2a + 7b + 7c) / 16, through the 32-bit expansion. */
static u2 interp3(u2 a, u2 b) { return interp_w(a, b, b, 7, 1, 0, 5); }
static u2 interp4(u2 a, u2 b, u2 c) { return interp_w(a, b, c, 2, 7, 7, 6); }
/* (5*a + 3*b) / 8, hq4x's Interp8. */
static u2 interp8(u2 a, u2 b) { return interp_w(a, b, b, 5, 3, 0, 5); }

static void hq3x_nine(u2 const w[10], u2 out[9])
{
    u2 const w5 = w[5];
    u2 p00, p01, p02, p10, p11, p12, p20, p21, p22;
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
        p01 = interp1(w5, w[2]);
        p02 = interp2(w5, w[2], w[6]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp2(w5, w[8], w[4]);
        p21 = interp1(w5, w[8]);
        p22 = interp2(w5, w[6], w[8]);
        break;
    case 2:
    case 34:
    case 130:
    case 162:
        p00 = interp1(w5, w[1]);
        p01 = w5;
        p02 = interp1(w5, w[3]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp2(w5, w[8], w[4]);
        p21 = interp1(w5, w[8]);
        p22 = interp2(w5, w[6], w[8]);
        break;
    case 16:
    case 17:
    case 48:
    case 49:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[3]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = w5;
        p20 = interp2(w5, w[8], w[4]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[9]);
        break;
    case 64:
    case 65:
    case 68:
    case 69:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp2(w5, w[2], w[6]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[7]);
        p21 = w5;
        p22 = interp1(w5, w[9]);
        break;
    case 8:
    case 12:
    case 136:
    case 140:
        p00 = interp1(w5, w[1]);
        p01 = interp1(w5, w[2]);
        p02 = interp2(w5, w[2], w[6]);
        p10 = w5;
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[7]);
        p21 = interp1(w5, w[8]);
        p22 = interp2(w5, w[6], w[8]);
        break;
    case 3:
    case 35:
    case 131:
    case 163:
        p00 = interp1(w5, w[4]);
        p01 = w5;
        p02 = interp1(w5, w[3]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp2(w5, w[8], w[4]);
        p21 = interp1(w5, w[8]);
        p22 = interp2(w5, w[6], w[8]);
        break;
    case 6:
    case 38:
    case 134:
    case 166:
        p00 = interp1(w5, w[1]);
        p01 = w5;
        p02 = interp1(w5, w[6]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp2(w5, w[8], w[4]);
        p21 = interp1(w5, w[8]);
        p22 = interp2(w5, w[6], w[8]);
        break;
    case 20:
    case 21:
    case 52:
    case 53:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[2]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = w5;
        p20 = interp2(w5, w[8], w[4]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[9]);
        break;
    case 144:
    case 145:
    case 176:
    case 177:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[3]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = w5;
        p20 = interp2(w5, w[8], w[4]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[8]);
        break;
    case 192:
    case 193:
    case 196:
    case 197:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp2(w5, w[2], w[6]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[7]);
        p21 = w5;
        p22 = interp1(w5, w[6]);
        break;
    case 96:
    case 97:
    case 100:
    case 101:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp2(w5, w[2], w[6]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[4]);
        p21 = w5;
        p22 = interp1(w5, w[9]);
        break;
    case 40:
    case 44:
    case 168:
    case 172:
        p00 = interp1(w5, w[1]);
        p01 = interp1(w5, w[2]);
        p02 = interp2(w5, w[2], w[6]);
        p10 = w5;
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[8]);
        p21 = interp1(w5, w[8]);
        p22 = interp2(w5, w[6], w[8]);
        break;
    case 9:
    case 13:
    case 137:
    case 141:
        p00 = interp1(w5, w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp2(w5, w[2], w[6]);
        p10 = w5;
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[7]);
        p21 = interp1(w5, w[8]);
        p22 = interp2(w5, w[6], w[8]);
        break;
    case 18:
    case 50:
        p00 = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            p01 = w5;
            p02 = interp1(w5, w[3]);
            p12 = w5;
        } else {
            p01 = interp3(w5, w[2]);
            p02 = interp4(w5, w[2], w[6]);
            p12 = interp3(w5, w[6]);
        }
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p20 = interp2(w5, w[8], w[4]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[9]);
        break;
    case 80:
    case 81:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[3]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p20 = interp1(w5, w[7]);
        if (differs(w[6], w[8])) {
            p12 = w5;
            p21 = w5;
            p22 = interp1(w5, w[9]);
        } else {
            p12 = interp3(w5, w[6]);
            p21 = interp3(w5, w[8]);
            p22 = interp4(w5, w[6], w[8]);
        }
        break;
    case 72:
    case 76:
        p00 = interp1(w5, w[1]);
        p01 = interp1(w5, w[2]);
        p02 = interp2(w5, w[2], w[6]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        if (differs(w[8], w[4])) {
            p10 = w5;
            p20 = interp1(w5, w[7]);
            p21 = w5;
        } else {
            p10 = interp3(w5, w[4]);
            p20 = interp4(w5, w[8], w[4]);
            p21 = interp3(w5, w[8]);
        }
        p22 = interp1(w5, w[9]);
        break;
    case 10:
    case 138:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
            p01 = w5;
            p10 = w5;
        } else {
            p00 = interp4(w5, w[4], w[2]);
            p01 = interp3(w5, w[2]);
            p10 = interp3(w5, w[4]);
        }
        p02 = interp1(w5, w[3]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[7]);
        p21 = interp1(w5, w[8]);
        p22 = interp2(w5, w[6], w[8]);
        break;
    case 66:
        p00 = interp1(w5, w[1]);
        p01 = w5;
        p02 = interp1(w5, w[3]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[7]);
        p21 = w5;
        p22 = interp1(w5, w[9]);
        break;
    case 24:
        p00 = interp1(w5, w[1]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[3]);
        p10 = w5;
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[7]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[9]);
        break;
    case 7:
    case 39:
    case 135:
        p00 = interp1(w5, w[4]);
        p01 = w5;
        p02 = interp1(w5, w[6]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp2(w5, w[8], w[4]);
        p21 = interp1(w5, w[8]);
        p22 = interp2(w5, w[6], w[8]);
        break;
    case 148:
    case 149:
    case 180:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[2]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = w5;
        p20 = interp2(w5, w[8], w[4]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[8]);
        break;
    case 224:
    case 225:
    case 228:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp2(w5, w[2], w[6]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[4]);
        p21 = w5;
        p22 = interp1(w5, w[6]);
        break;
    case 41:
    case 45:
    case 169:
        p00 = interp1(w5, w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp2(w5, w[2], w[6]);
        p10 = w5;
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[8]);
        p21 = interp1(w5, w[8]);
        p22 = interp2(w5, w[6], w[8]);
        break;
    case 22:
    case 54:
        p00 = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            p01 = w5;
            p02 = w5;
            p12 = w5;
        } else {
            p01 = interp3(w5, w[2]);
            p02 = interp4(w5, w[2], w[6]);
            p12 = interp3(w5, w[6]);
        }
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p20 = interp2(w5, w[8], w[4]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[9]);
        break;
    case 208:
    case 209:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[3]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p20 = interp1(w5, w[7]);
        if (differs(w[6], w[8])) {
            p12 = w5;
            p21 = w5;
            p22 = w5;
        } else {
            p12 = interp3(w5, w[6]);
            p21 = interp3(w5, w[8]);
            p22 = interp4(w5, w[6], w[8]);
        }
        break;
    case 104:
    case 108:
        p00 = interp1(w5, w[1]);
        p01 = interp1(w5, w[2]);
        p02 = interp2(w5, w[2], w[6]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        if (differs(w[8], w[4])) {
            p10 = w5;
            p20 = w5;
            p21 = w5;
        } else {
            p10 = interp3(w5, w[4]);
            p20 = interp4(w5, w[8], w[4]);
            p21 = interp3(w5, w[8]);
        }
        p22 = interp1(w5, w[9]);
        break;
    case 11:
    case 139:
        if (differs(w[4], w[2])) {
            p00 = w5;
            p01 = w5;
            p10 = w5;
        } else {
            p00 = interp4(w5, w[4], w[2]);
            p01 = interp3(w5, w[2]);
            p10 = interp3(w5, w[4]);
        }
        p02 = interp1(w5, w[3]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[7]);
        p21 = interp1(w5, w[8]);
        p22 = interp2(w5, w[6], w[8]);
        break;
    case 19:
    case 51:
        if (differs(w[2], w[6])) {
            p00 = interp1(w5, w[4]);
            p01 = w5;
            p02 = interp1(w5, w[3]);
            p12 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
            p01 = interp1(w[2], w5);
            p02 = interp5(w[2], w[6]);
            p12 = interp1(w5, w[6]);
        }
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p20 = interp2(w5, w[8], w[4]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[9]);
        break;
    case 146:
    case 178:
        if (differs(w[2], w[6])) {
            p01 = w5;
            p02 = interp1(w5, w[3]);
            p12 = w5;
            p22 = interp1(w5, w[8]);
        } else {
            p01 = interp1(w5, w[2]);
            p02 = interp5(w[2], w[6]);
            p12 = interp1(w[6], w5);
            p22 = interp2(w5, w[6], w[8]);
        }
        p00 = interp1(w5, w[1]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p20 = interp2(w5, w[8], w[4]);
        p21 = interp1(w5, w[8]);
        break;
    case 84:
    case 85:
        if (differs(w[6], w[8])) {
            p02 = interp1(w5, w[2]);
            p12 = w5;
            p21 = w5;
            p22 = interp1(w5, w[9]);
        } else {
            p02 = interp2(w5, w[2], w[6]);
            p12 = interp1(w[6], w5);
            p21 = interp1(w5, w[8]);
            p22 = interp5(w[6], w[8]);
        }
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp1(w5, w[2]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p20 = interp1(w5, w[7]);
        break;
    case 112:
    case 113:
        if (differs(w[6], w[8])) {
            p12 = w5;
            p20 = interp1(w5, w[4]);
            p21 = w5;
            p22 = interp1(w5, w[9]);
        } else {
            p12 = interp1(w5, w[6]);
            p20 = interp2(w5, w[8], w[4]);
            p21 = interp1(w[8], w5);
            p22 = interp5(w[6], w[8]);
        }
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[3]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        break;
    case 200:
    case 204:
        if (differs(w[8], w[4])) {
            p10 = w5;
            p20 = interp1(w5, w[7]);
            p21 = w5;
            p22 = interp1(w5, w[6]);
        } else {
            p10 = interp1(w5, w[4]);
            p20 = interp5(w[8], w[4]);
            p21 = interp1(w[8], w5);
            p22 = interp2(w5, w[6], w[8]);
        }
        p00 = interp1(w5, w[1]);
        p01 = interp1(w5, w[2]);
        p02 = interp2(w5, w[2], w[6]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        break;
    case 73:
    case 77:
        if (differs(w[8], w[4])) {
            p00 = interp1(w5, w[2]);
            p10 = w5;
            p20 = interp1(w5, w[7]);
            p21 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
            p10 = interp1(w[4], w5);
            p20 = interp5(w[8], w[4]);
            p21 = interp1(w5, w[8]);
        }
        p01 = interp1(w5, w[2]);
        p02 = interp2(w5, w[2], w[6]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p22 = interp1(w5, w[9]);
        break;
    case 42:
    case 170:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
            p01 = w5;
            p10 = w5;
            p20 = interp1(w5, w[8]);
        } else {
            p00 = interp5(w[4], w[2]);
            p01 = interp1(w5, w[2]);
            p10 = interp1(w[4], w5);
            p20 = interp2(w5, w[8], w[4]);
        }
        p02 = interp1(w5, w[3]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p21 = interp1(w5, w[8]);
        p22 = interp2(w5, w[6], w[8]);
        break;
    case 14:
    case 142:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
            p01 = w5;
            p02 = interp1(w5, w[6]);
            p10 = w5;
        } else {
            p00 = interp5(w[4], w[2]);
            p01 = interp1(w[2], w5);
            p02 = interp2(w5, w[2], w[6]);
            p10 = interp1(w5, w[4]);
        }
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[7]);
        p21 = interp1(w5, w[8]);
        p22 = interp2(w5, w[6], w[8]);
        break;
    case 67:
        p00 = interp1(w5, w[4]);
        p01 = w5;
        p02 = interp1(w5, w[3]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[7]);
        p21 = w5;
        p22 = interp1(w5, w[9]);
        break;
    case 70:
        p00 = interp1(w5, w[1]);
        p01 = w5;
        p02 = interp1(w5, w[6]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[7]);
        p21 = w5;
        p22 = interp1(w5, w[9]);
        break;
    case 28:
        p00 = interp1(w5, w[1]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[2]);
        p10 = w5;
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[7]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[9]);
        break;
    case 152:
        p00 = interp1(w5, w[1]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[3]);
        p10 = w5;
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[7]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[8]);
        break;
    case 194:
        p00 = interp1(w5, w[1]);
        p01 = w5;
        p02 = interp1(w5, w[3]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[7]);
        p21 = w5;
        p22 = interp1(w5, w[6]);
        break;
    case 98:
        p00 = interp1(w5, w[1]);
        p01 = w5;
        p02 = interp1(w5, w[3]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[4]);
        p21 = w5;
        p22 = interp1(w5, w[9]);
        break;
    case 56:
        p00 = interp1(w5, w[1]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[3]);
        p10 = w5;
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[8]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[9]);
        break;
    case 25:
        p00 = interp1(w5, w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[3]);
        p10 = w5;
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[7]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[9]);
        break;
    case 26:
    case 31:
        if (differs(w[4], w[2])) {
            p00 = w5;
            p10 = w5;
        } else {
            p00 = interp4(w5, w[4], w[2]);
            p10 = interp3(w5, w[4]);
        }
        p01 = w5;
        if (differs(w[2], w[6])) {
            p02 = w5;
            p12 = w5;
        } else {
            p02 = interp4(w5, w[2], w[6]);
            p12 = interp3(w5, w[6]);
        }
        p11 = w5;
        p20 = interp1(w5, w[7]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[9]);
        break;
    case 82:
    case 214:
        p00 = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            p01 = w5;
            p02 = w5;
        } else {
            p01 = interp3(w5, w[2]);
            p02 = interp4(w5, w[2], w[6]);
        }
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[7]);
        if (differs(w[6], w[8])) {
            p21 = w5;
            p22 = w5;
        } else {
            p21 = interp3(w5, w[8]);
            p22 = interp4(w5, w[6], w[8]);
        }
        break;
    case 88:
    case 248:
        p00 = interp1(w5, w[1]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[3]);
        p11 = w5;
        if (differs(w[8], w[4])) {
            p10 = w5;
            p20 = w5;
        } else {
            p10 = interp3(w5, w[4]);
            p20 = interp4(w5, w[8], w[4]);
        }
        p21 = w5;
        if (differs(w[6], w[8])) {
            p12 = w5;
            p22 = w5;
        } else {
            p12 = interp3(w5, w[6]);
            p22 = interp4(w5, w[6], w[8]);
        }
        break;
    case 74:
    case 107:
        if (differs(w[4], w[2])) {
            p00 = w5;
            p01 = w5;
        } else {
            p00 = interp4(w5, w[4], w[2]);
            p01 = interp3(w5, w[2]);
        }
        p02 = interp1(w5, w[3]);
        p10 = w5;
        p11 = w5;
        p12 = interp1(w5, w[6]);
        if (differs(w[8], w[4])) {
            p20 = w5;
            p21 = w5;
        } else {
            p20 = interp4(w5, w[8], w[4]);
            p21 = interp3(w5, w[8]);
        }
        p22 = interp1(w5, w[9]);
        break;
    case 27:
        if (differs(w[4], w[2])) {
            p00 = w5;
            p01 = w5;
            p10 = w5;
        } else {
            p00 = interp4(w5, w[4], w[2]);
            p01 = interp3(w5, w[2]);
            p10 = interp3(w5, w[4]);
        }
        p02 = interp1(w5, w[3]);
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[7]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[9]);
        break;
    case 86:
        p00 = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            p01 = w5;
            p02 = w5;
            p12 = w5;
        } else {
            p01 = interp3(w5, w[2]);
            p02 = interp4(w5, w[2], w[6]);
            p12 = interp3(w5, w[6]);
        }
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p20 = interp1(w5, w[7]);
        p21 = w5;
        p22 = interp1(w5, w[9]);
        break;
    case 216:
        p00 = interp1(w5, w[1]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[3]);
        p10 = w5;
        p11 = w5;
        p20 = interp1(w5, w[7]);
        if (differs(w[6], w[8])) {
            p12 = w5;
            p21 = w5;
            p22 = w5;
        } else {
            p12 = interp3(w5, w[6]);
            p21 = interp3(w5, w[8]);
            p22 = interp4(w5, w[6], w[8]);
        }
        break;
    case 106:
        p00 = interp1(w5, w[1]);
        p01 = w5;
        p02 = interp1(w5, w[3]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        if (differs(w[8], w[4])) {
            p10 = w5;
            p20 = w5;
            p21 = w5;
        } else {
            p10 = interp3(w5, w[4]);
            p20 = interp4(w5, w[8], w[4]);
            p21 = interp3(w5, w[8]);
        }
        p22 = interp1(w5, w[9]);
        break;
    case 30:
        p00 = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            p01 = w5;
            p02 = w5;
            p12 = w5;
        } else {
            p01 = interp3(w5, w[2]);
            p02 = interp4(w5, w[2], w[6]);
            p12 = interp3(w5, w[6]);
        }
        p10 = w5;
        p11 = w5;
        p20 = interp1(w5, w[7]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[9]);
        break;
    case 210:
        p00 = interp1(w5, w[1]);
        p01 = w5;
        p02 = interp1(w5, w[3]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p20 = interp1(w5, w[7]);
        if (differs(w[6], w[8])) {
            p12 = w5;
            p21 = w5;
            p22 = w5;
        } else {
            p12 = interp3(w5, w[6]);
            p21 = interp3(w5, w[8]);
            p22 = interp4(w5, w[6], w[8]);
        }
        break;
    case 120:
        p00 = interp1(w5, w[1]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[3]);
        p11 = w5;
        p12 = w5;
        if (differs(w[8], w[4])) {
            p10 = w5;
            p20 = w5;
            p21 = w5;
        } else {
            p10 = interp3(w5, w[4]);
            p20 = interp4(w5, w[8], w[4]);
            p21 = interp3(w5, w[8]);
        }
        p22 = interp1(w5, w[9]);
        break;
    case 75:
        if (differs(w[4], w[2])) {
            p00 = w5;
            p01 = w5;
            p10 = w5;
        } else {
            p00 = interp4(w5, w[4], w[2]);
            p01 = interp3(w5, w[2]);
            p10 = interp3(w5, w[4]);
        }
        p02 = interp1(w5, w[3]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[7]);
        p21 = w5;
        p22 = interp1(w5, w[9]);
        break;
    case 29:
        p00 = interp1(w5, w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[2]);
        p10 = w5;
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[7]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[9]);
        break;
    case 198:
        p00 = interp1(w5, w[1]);
        p01 = w5;
        p02 = interp1(w5, w[6]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[7]);
        p21 = w5;
        p22 = interp1(w5, w[6]);
        break;
    case 184:
        p00 = interp1(w5, w[1]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[3]);
        p10 = w5;
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[8]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[8]);
        break;
    case 99:
        p00 = interp1(w5, w[4]);
        p01 = w5;
        p02 = interp1(w5, w[3]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[4]);
        p21 = w5;
        p22 = interp1(w5, w[9]);
        break;
    case 57:
        p00 = interp1(w5, w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[3]);
        p10 = w5;
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[8]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[9]);
        break;
    case 71:
        p00 = interp1(w5, w[4]);
        p01 = w5;
        p02 = interp1(w5, w[6]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[7]);
        p21 = w5;
        p22 = interp1(w5, w[9]);
        break;
    case 156:
        p00 = interp1(w5, w[1]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[2]);
        p10 = w5;
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[7]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[8]);
        break;
    case 226:
        p00 = interp1(w5, w[1]);
        p01 = w5;
        p02 = interp1(w5, w[3]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[4]);
        p21 = w5;
        p22 = interp1(w5, w[6]);
        break;
    case 60:
        p00 = interp1(w5, w[1]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[2]);
        p10 = w5;
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[8]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[9]);
        break;
    case 195:
        p00 = interp1(w5, w[4]);
        p01 = w5;
        p02 = interp1(w5, w[3]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[7]);
        p21 = w5;
        p22 = interp1(w5, w[6]);
        break;
    case 102:
        p00 = interp1(w5, w[1]);
        p01 = w5;
        p02 = interp1(w5, w[6]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[4]);
        p21 = w5;
        p22 = interp1(w5, w[9]);
        break;
    case 153:
        p00 = interp1(w5, w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[3]);
        p10 = w5;
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[7]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[8]);
        break;
    case 58:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = w5;
        if (differs(w[2], w[6])) {
            p02 = interp1(w5, w[3]);
        } else {
            p02 = interp2(w5, w[2], w[6]);
        }
        p10 = w5;
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[8]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[9]);
        break;
    case 83:
        p00 = interp1(w5, w[4]);
        p01 = w5;
        if (differs(w[2], w[6])) {
            p02 = interp1(w5, w[3]);
        } else {
            p02 = interp2(w5, w[2], w[6]);
        }
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[7]);
        p21 = w5;
        if (differs(w[6], w[8])) {
            p22 = interp1(w5, w[9]);
        } else {
            p22 = interp2(w5, w[6], w[8]);
        }
        break;
    case 92:
        p00 = interp1(w5, w[1]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[2]);
        p10 = w5;
        p11 = w5;
        p12 = w5;
        if (differs(w[8], w[4])) {
            p20 = interp1(w5, w[7]);
        } else {
            p20 = interp2(w5, w[8], w[4]);
        }
        p21 = w5;
        if (differs(w[6], w[8])) {
            p22 = interp1(w5, w[9]);
        } else {
            p22 = interp2(w5, w[6], w[8]);
        }
        break;
    case 202:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = w5;
        p02 = interp1(w5, w[3]);
        p10 = w5;
        p11 = w5;
        p12 = interp1(w5, w[6]);
        if (differs(w[8], w[4])) {
            p20 = interp1(w5, w[7]);
        } else {
            p20 = interp2(w5, w[8], w[4]);
        }
        p21 = w5;
        p22 = interp1(w5, w[6]);
        break;
    case 78:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = w5;
        p02 = interp1(w5, w[6]);
        p10 = w5;
        p11 = w5;
        p12 = interp1(w5, w[6]);
        if (differs(w[8], w[4])) {
            p20 = interp1(w5, w[7]);
        } else {
            p20 = interp2(w5, w[8], w[4]);
        }
        p21 = w5;
        p22 = interp1(w5, w[9]);
        break;
    case 154:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = w5;
        if (differs(w[2], w[6])) {
            p02 = interp1(w5, w[3]);
        } else {
            p02 = interp2(w5, w[2], w[6]);
        }
        p10 = w5;
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[7]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[8]);
        break;
    case 114:
        p00 = interp1(w5, w[1]);
        p01 = w5;
        if (differs(w[2], w[6])) {
            p02 = interp1(w5, w[3]);
        } else {
            p02 = interp2(w5, w[2], w[6]);
        }
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[4]);
        p21 = w5;
        if (differs(w[6], w[8])) {
            p22 = interp1(w5, w[9]);
        } else {
            p22 = interp2(w5, w[6], w[8]);
        }
        break;
    case 89:
        p00 = interp1(w5, w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[3]);
        p10 = w5;
        p11 = w5;
        p12 = w5;
        if (differs(w[8], w[4])) {
            p20 = interp1(w5, w[7]);
        } else {
            p20 = interp2(w5, w[8], w[4]);
        }
        p21 = w5;
        if (differs(w[6], w[8])) {
            p22 = interp1(w5, w[9]);
        } else {
            p22 = interp2(w5, w[6], w[8]);
        }
        break;
    case 90:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = w5;
        if (differs(w[2], w[6])) {
            p02 = interp1(w5, w[3]);
        } else {
            p02 = interp2(w5, w[2], w[6]);
        }
        p10 = w5;
        p11 = w5;
        p12 = w5;
        if (differs(w[8], w[4])) {
            p20 = interp1(w5, w[7]);
        } else {
            p20 = interp2(w5, w[8], w[4]);
        }
        p21 = w5;
        if (differs(w[6], w[8])) {
            p22 = interp1(w5, w[9]);
        } else {
            p22 = interp2(w5, w[6], w[8]);
        }
        break;
    case 23:
    case 55:
        if (differs(w[2], w[6])) {
            p00 = interp1(w5, w[4]);
            p01 = w5;
            p02 = w5;
            p12 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
            p01 = interp1(w[2], w5);
            p02 = interp5(w[2], w[6]);
            p12 = interp1(w5, w[6]);
        }
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p20 = interp2(w5, w[8], w[4]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[9]);
        break;
    case 150:
    case 182:
        if (differs(w[2], w[6])) {
            p01 = w5;
            p02 = w5;
            p12 = w5;
            p22 = interp1(w5, w[8]);
        } else {
            p01 = interp1(w5, w[2]);
            p02 = interp5(w[2], w[6]);
            p12 = interp1(w[6], w5);
            p22 = interp2(w5, w[6], w[8]);
        }
        p00 = interp1(w5, w[1]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p20 = interp2(w5, w[8], w[4]);
        p21 = interp1(w5, w[8]);
        break;
    case 212:
    case 213:
        if (differs(w[6], w[8])) {
            p02 = interp1(w5, w[2]);
            p12 = w5;
            p21 = w5;
            p22 = w5;
        } else {
            p02 = interp2(w5, w[2], w[6]);
            p12 = interp1(w[6], w5);
            p21 = interp1(w5, w[8]);
            p22 = interp5(w[6], w[8]);
        }
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp1(w5, w[2]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p20 = interp1(w5, w[7]);
        break;
    case 240:
    case 241:
        if (differs(w[6], w[8])) {
            p12 = w5;
            p20 = interp1(w5, w[4]);
            p21 = w5;
            p22 = w5;
        } else {
            p12 = interp1(w5, w[6]);
            p20 = interp2(w5, w[8], w[4]);
            p21 = interp1(w[8], w5);
            p22 = interp5(w[6], w[8]);
        }
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[3]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        break;
    case 232:
    case 236:
        if (differs(w[8], w[4])) {
            p10 = w5;
            p20 = w5;
            p21 = w5;
            p22 = interp1(w5, w[6]);
        } else {
            p10 = interp1(w5, w[4]);
            p20 = interp5(w[8], w[4]);
            p21 = interp1(w[8], w5);
            p22 = interp2(w5, w[6], w[8]);
        }
        p00 = interp1(w5, w[1]);
        p01 = interp1(w5, w[2]);
        p02 = interp2(w5, w[2], w[6]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        break;
    case 105:
    case 109:
        if (differs(w[8], w[4])) {
            p00 = interp1(w5, w[2]);
            p10 = w5;
            p20 = w5;
            p21 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
            p10 = interp1(w[4], w5);
            p20 = interp5(w[8], w[4]);
            p21 = interp1(w5, w[8]);
        }
        p01 = interp1(w5, w[2]);
        p02 = interp2(w5, w[2], w[6]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p22 = interp1(w5, w[9]);
        break;
    case 43:
    case 171:
        if (differs(w[4], w[2])) {
            p00 = w5;
            p01 = w5;
            p10 = w5;
            p20 = interp1(w5, w[8]);
        } else {
            p00 = interp5(w[4], w[2]);
            p01 = interp1(w5, w[2]);
            p10 = interp1(w[4], w5);
            p20 = interp2(w5, w[8], w[4]);
        }
        p02 = interp1(w5, w[3]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p21 = interp1(w5, w[8]);
        p22 = interp2(w5, w[6], w[8]);
        break;
    case 15:
    case 143:
        if (differs(w[4], w[2])) {
            p00 = w5;
            p01 = w5;
            p02 = interp1(w5, w[6]);
            p10 = w5;
        } else {
            p00 = interp5(w[4], w[2]);
            p01 = interp1(w[2], w5);
            p02 = interp2(w5, w[2], w[6]);
            p10 = interp1(w5, w[4]);
        }
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[7]);
        p21 = interp1(w5, w[8]);
        p22 = interp2(w5, w[6], w[8]);
        break;
    case 124:
        p00 = interp1(w5, w[1]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[2]);
        p11 = w5;
        p12 = w5;
        if (differs(w[8], w[4])) {
            p10 = w5;
            p20 = w5;
            p21 = w5;
        } else {
            p10 = interp3(w5, w[4]);
            p20 = interp4(w5, w[8], w[4]);
            p21 = interp3(w5, w[8]);
        }
        p22 = interp1(w5, w[9]);
        break;
    case 203:
        if (differs(w[4], w[2])) {
            p00 = w5;
            p01 = w5;
            p10 = w5;
        } else {
            p00 = interp4(w5, w[4], w[2]);
            p01 = interp3(w5, w[2]);
            p10 = interp3(w5, w[4]);
        }
        p02 = interp1(w5, w[3]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[7]);
        p21 = w5;
        p22 = interp1(w5, w[6]);
        break;
    case 62:
        p00 = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            p01 = w5;
            p02 = w5;
            p12 = w5;
        } else {
            p01 = interp3(w5, w[2]);
            p02 = interp4(w5, w[2], w[6]);
            p12 = interp3(w5, w[6]);
        }
        p10 = w5;
        p11 = w5;
        p20 = interp1(w5, w[8]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[9]);
        break;
    case 211:
        p00 = interp1(w5, w[4]);
        p01 = w5;
        p02 = interp1(w5, w[3]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p20 = interp1(w5, w[7]);
        if (differs(w[6], w[8])) {
            p12 = w5;
            p21 = w5;
            p22 = w5;
        } else {
            p12 = interp3(w5, w[6]);
            p21 = interp3(w5, w[8]);
            p22 = interp4(w5, w[6], w[8]);
        }
        break;
    case 118:
        p00 = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            p01 = w5;
            p02 = w5;
            p12 = w5;
        } else {
            p01 = interp3(w5, w[2]);
            p02 = interp4(w5, w[2], w[6]);
            p12 = interp3(w5, w[6]);
        }
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p20 = interp1(w5, w[4]);
        p21 = w5;
        p22 = interp1(w5, w[9]);
        break;
    case 217:
        p00 = interp1(w5, w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[3]);
        p10 = w5;
        p11 = w5;
        p20 = interp1(w5, w[7]);
        if (differs(w[6], w[8])) {
            p12 = w5;
            p21 = w5;
            p22 = w5;
        } else {
            p12 = interp3(w5, w[6]);
            p21 = interp3(w5, w[8]);
            p22 = interp4(w5, w[6], w[8]);
        }
        break;
    case 110:
        p00 = interp1(w5, w[1]);
        p01 = w5;
        p02 = interp1(w5, w[6]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        if (differs(w[8], w[4])) {
            p10 = w5;
            p20 = w5;
            p21 = w5;
        } else {
            p10 = interp3(w5, w[4]);
            p20 = interp4(w5, w[8], w[4]);
            p21 = interp3(w5, w[8]);
        }
        p22 = interp1(w5, w[9]);
        break;
    case 155:
        if (differs(w[4], w[2])) {
            p00 = w5;
            p01 = w5;
            p10 = w5;
        } else {
            p00 = interp4(w5, w[4], w[2]);
            p01 = interp3(w5, w[2]);
            p10 = interp3(w5, w[4]);
        }
        p02 = interp1(w5, w[3]);
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[7]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[8]);
        break;
    case 188:
        p00 = interp1(w5, w[1]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[2]);
        p10 = w5;
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[8]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[8]);
        break;
    case 185:
        p00 = interp1(w5, w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[3]);
        p10 = w5;
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[8]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[8]);
        break;
    case 61:
        p00 = interp1(w5, w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[2]);
        p10 = w5;
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[8]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[9]);
        break;
    case 157:
        p00 = interp1(w5, w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[2]);
        p10 = w5;
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[7]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[8]);
        break;
    case 103:
        p00 = interp1(w5, w[4]);
        p01 = w5;
        p02 = interp1(w5, w[6]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[4]);
        p21 = w5;
        p22 = interp1(w5, w[9]);
        break;
    case 227:
        p00 = interp1(w5, w[4]);
        p01 = w5;
        p02 = interp1(w5, w[3]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[4]);
        p21 = w5;
        p22 = interp1(w5, w[6]);
        break;
    case 230:
        p00 = interp1(w5, w[1]);
        p01 = w5;
        p02 = interp1(w5, w[6]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[4]);
        p21 = w5;
        p22 = interp1(w5, w[6]);
        break;
    case 199:
        p00 = interp1(w5, w[4]);
        p01 = w5;
        p02 = interp1(w5, w[6]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[7]);
        p21 = w5;
        p22 = interp1(w5, w[6]);
        break;
    case 220:
        p00 = interp1(w5, w[1]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[2]);
        p10 = w5;
        p11 = w5;
        if (differs(w[8], w[4])) {
            p20 = interp1(w5, w[7]);
        } else {
            p20 = interp2(w5, w[8], w[4]);
        }
        if (differs(w[6], w[8])) {
            p12 = w5;
            p21 = w5;
            p22 = w5;
        } else {
            p12 = interp3(w5, w[6]);
            p21 = interp3(w5, w[8]);
            p22 = interp4(w5, w[6], w[8]);
        }
        break;
    case 158:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        if (differs(w[2], w[6])) {
            p01 = w5;
            p02 = w5;
            p12 = w5;
        } else {
            p01 = interp3(w5, w[2]);
            p02 = interp4(w5, w[2], w[6]);
            p12 = interp3(w5, w[6]);
        }
        p10 = w5;
        p11 = w5;
        p20 = interp1(w5, w[7]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[8]);
        break;
    case 234:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = w5;
        p02 = interp1(w5, w[3]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        if (differs(w[8], w[4])) {
            p10 = w5;
            p20 = w5;
            p21 = w5;
        } else {
            p10 = interp3(w5, w[4]);
            p20 = interp4(w5, w[8], w[4]);
            p21 = interp3(w5, w[8]);
        }
        p22 = interp1(w5, w[6]);
        break;
    case 242:
        p00 = interp1(w5, w[1]);
        p01 = w5;
        if (differs(w[2], w[6])) {
            p02 = interp1(w5, w[3]);
        } else {
            p02 = interp2(w5, w[2], w[6]);
        }
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p20 = interp1(w5, w[4]);
        if (differs(w[6], w[8])) {
            p12 = w5;
            p21 = w5;
            p22 = w5;
        } else {
            p12 = interp3(w5, w[6]);
            p21 = interp3(w5, w[8]);
            p22 = interp4(w5, w[6], w[8]);
        }
        break;
    case 59:
        if (differs(w[4], w[2])) {
            p00 = w5;
            p01 = w5;
            p10 = w5;
        } else {
            p00 = interp4(w5, w[4], w[2]);
            p01 = interp3(w5, w[2]);
            p10 = interp3(w5, w[4]);
        }
        if (differs(w[2], w[6])) {
            p02 = interp1(w5, w[3]);
        } else {
            p02 = interp2(w5, w[2], w[6]);
        }
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[8]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[9]);
        break;
    case 121:
        p00 = interp1(w5, w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[3]);
        p11 = w5;
        p12 = w5;
        if (differs(w[8], w[4])) {
            p10 = w5;
            p20 = w5;
            p21 = w5;
        } else {
            p10 = interp3(w5, w[4]);
            p20 = interp4(w5, w[8], w[4]);
            p21 = interp3(w5, w[8]);
        }
        if (differs(w[6], w[8])) {
            p22 = interp1(w5, w[9]);
        } else {
            p22 = interp2(w5, w[6], w[8]);
        }
        break;
    case 87:
        p00 = interp1(w5, w[4]);
        if (differs(w[2], w[6])) {
            p01 = w5;
            p02 = w5;
            p12 = w5;
        } else {
            p01 = interp3(w5, w[2]);
            p02 = interp4(w5, w[2], w[6]);
            p12 = interp3(w5, w[6]);
        }
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p20 = interp1(w5, w[7]);
        p21 = w5;
        if (differs(w[6], w[8])) {
            p22 = interp1(w5, w[9]);
        } else {
            p22 = interp2(w5, w[6], w[8]);
        }
        break;
    case 79:
        if (differs(w[4], w[2])) {
            p00 = w5;
            p01 = w5;
            p10 = w5;
        } else {
            p00 = interp4(w5, w[4], w[2]);
            p01 = interp3(w5, w[2]);
            p10 = interp3(w5, w[4]);
        }
        p02 = interp1(w5, w[6]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        if (differs(w[8], w[4])) {
            p20 = interp1(w5, w[7]);
        } else {
            p20 = interp2(w5, w[8], w[4]);
        }
        p21 = w5;
        p22 = interp1(w5, w[9]);
        break;
    case 122:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = w5;
        if (differs(w[2], w[6])) {
            p02 = interp1(w5, w[3]);
        } else {
            p02 = interp2(w5, w[2], w[6]);
        }
        p11 = w5;
        p12 = w5;
        if (differs(w[8], w[4])) {
            p10 = w5;
            p20 = w5;
            p21 = w5;
        } else {
            p10 = interp3(w5, w[4]);
            p20 = interp4(w5, w[8], w[4]);
            p21 = interp3(w5, w[8]);
        }
        if (differs(w[6], w[8])) {
            p22 = interp1(w5, w[9]);
        } else {
            p22 = interp2(w5, w[6], w[8]);
        }
        break;
    case 94:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        if (differs(w[2], w[6])) {
            p01 = w5;
            p02 = w5;
            p12 = w5;
        } else {
            p01 = interp3(w5, w[2]);
            p02 = interp4(w5, w[2], w[6]);
            p12 = interp3(w5, w[6]);
        }
        p10 = w5;
        p11 = w5;
        if (differs(w[8], w[4])) {
            p20 = interp1(w5, w[7]);
        } else {
            p20 = interp2(w5, w[8], w[4]);
        }
        p21 = w5;
        if (differs(w[6], w[8])) {
            p22 = interp1(w5, w[9]);
        } else {
            p22 = interp2(w5, w[6], w[8]);
        }
        break;
    case 218:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = w5;
        if (differs(w[2], w[6])) {
            p02 = interp1(w5, w[3]);
        } else {
            p02 = interp2(w5, w[2], w[6]);
        }
        p10 = w5;
        p11 = w5;
        if (differs(w[8], w[4])) {
            p20 = interp1(w5, w[7]);
        } else {
            p20 = interp2(w5, w[8], w[4]);
        }
        if (differs(w[6], w[8])) {
            p12 = w5;
            p21 = w5;
            p22 = w5;
        } else {
            p12 = interp3(w5, w[6]);
            p21 = interp3(w5, w[8]);
            p22 = interp4(w5, w[6], w[8]);
        }
        break;
    case 91:
        if (differs(w[4], w[2])) {
            p00 = w5;
            p01 = w5;
            p10 = w5;
        } else {
            p00 = interp4(w5, w[4], w[2]);
            p01 = interp3(w5, w[2]);
            p10 = interp3(w5, w[4]);
        }
        if (differs(w[2], w[6])) {
            p02 = interp1(w5, w[3]);
        } else {
            p02 = interp2(w5, w[2], w[6]);
        }
        p11 = w5;
        p12 = w5;
        if (differs(w[8], w[4])) {
            p20 = interp1(w5, w[7]);
        } else {
            p20 = interp2(w5, w[8], w[4]);
        }
        p21 = w5;
        if (differs(w[6], w[8])) {
            p22 = interp1(w5, w[9]);
        } else {
            p22 = interp2(w5, w[6], w[8]);
        }
        break;
    case 229:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp2(w5, w[2], w[6]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[4]);
        p21 = w5;
        p22 = interp1(w5, w[6]);
        break;
    case 167:
        p00 = interp1(w5, w[4]);
        p01 = w5;
        p02 = interp1(w5, w[6]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp2(w5, w[8], w[4]);
        p21 = interp1(w5, w[8]);
        p22 = interp2(w5, w[6], w[8]);
        break;
    case 173:
        p00 = interp1(w5, w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp2(w5, w[2], w[6]);
        p10 = w5;
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[8]);
        p21 = interp1(w5, w[8]);
        p22 = interp2(w5, w[6], w[8]);
        break;
    case 181:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[2]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = w5;
        p20 = interp2(w5, w[8], w[4]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[8]);
        break;
    case 186:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = w5;
        if (differs(w[2], w[6])) {
            p02 = interp1(w5, w[3]);
        } else {
            p02 = interp2(w5, w[2], w[6]);
        }
        p10 = w5;
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[8]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[8]);
        break;
    case 115:
        p00 = interp1(w5, w[4]);
        p01 = w5;
        if (differs(w[2], w[6])) {
            p02 = interp1(w5, w[3]);
        } else {
            p02 = interp2(w5, w[2], w[6]);
        }
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[4]);
        p21 = w5;
        if (differs(w[6], w[8])) {
            p22 = interp1(w5, w[9]);
        } else {
            p22 = interp2(w5, w[6], w[8]);
        }
        break;
    case 93:
        p00 = interp1(w5, w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[2]);
        p10 = w5;
        p11 = w5;
        p12 = w5;
        if (differs(w[8], w[4])) {
            p20 = interp1(w5, w[7]);
        } else {
            p20 = interp2(w5, w[8], w[4]);
        }
        p21 = w5;
        if (differs(w[6], w[8])) {
            p22 = interp1(w5, w[9]);
        } else {
            p22 = interp2(w5, w[6], w[8]);
        }
        break;
    case 206:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = w5;
        p02 = interp1(w5, w[6]);
        p10 = w5;
        p11 = w5;
        p12 = interp1(w5, w[6]);
        if (differs(w[8], w[4])) {
            p20 = interp1(w5, w[7]);
        } else {
            p20 = interp2(w5, w[8], w[4]);
        }
        p21 = w5;
        p22 = interp1(w5, w[6]);
        break;
    case 201:
    case 205:
        p00 = interp1(w5, w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp2(w5, w[2], w[6]);
        p10 = w5;
        p11 = w5;
        p12 = interp1(w5, w[6]);
        if (differs(w[8], w[4])) {
            p20 = interp1(w5, w[7]);
        } else {
            p20 = interp2(w5, w[8], w[4]);
        }
        p21 = w5;
        p22 = interp1(w5, w[6]);
        break;
    case 46:
    case 174:
        if (differs(w[4], w[2])) {
            p00 = interp1(w5, w[1]);
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = w5;
        p02 = interp1(w5, w[6]);
        p10 = w5;
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[8]);
        p21 = interp1(w5, w[8]);
        p22 = interp2(w5, w[6], w[8]);
        break;
    case 147:
    case 179:
        p00 = interp1(w5, w[4]);
        p01 = w5;
        if (differs(w[2], w[6])) {
            p02 = interp1(w5, w[3]);
        } else {
            p02 = interp2(w5, w[2], w[6]);
        }
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = w5;
        p20 = interp2(w5, w[8], w[4]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[8]);
        break;
    case 116:
    case 117:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[2]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[4]);
        p21 = w5;
        if (differs(w[6], w[8])) {
            p22 = interp1(w5, w[9]);
        } else {
            p22 = interp2(w5, w[6], w[8]);
        }
        break;
    case 189:
        p00 = interp1(w5, w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[2]);
        p10 = w5;
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[8]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[8]);
        break;
    case 231:
        p00 = interp1(w5, w[4]);
        p01 = w5;
        p02 = interp1(w5, w[6]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[4]);
        p21 = w5;
        p22 = interp1(w5, w[6]);
        break;
    case 126:
        p00 = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            p01 = w5;
            p02 = w5;
            p12 = w5;
        } else {
            p01 = interp3(w5, w[2]);
            p02 = interp4(w5, w[2], w[6]);
            p12 = interp3(w5, w[6]);
        }
        p11 = w5;
        if (differs(w[8], w[4])) {
            p10 = w5;
            p20 = w5;
            p21 = w5;
        } else {
            p10 = interp3(w5, w[4]);
            p20 = interp4(w5, w[8], w[4]);
            p21 = interp3(w5, w[8]);
        }
        p22 = interp1(w5, w[9]);
        break;
    case 219:
        if (differs(w[4], w[2])) {
            p00 = w5;
            p01 = w5;
            p10 = w5;
        } else {
            p00 = interp4(w5, w[4], w[2]);
            p01 = interp3(w5, w[2]);
            p10 = interp3(w5, w[4]);
        }
        p02 = interp1(w5, w[3]);
        p11 = w5;
        p20 = interp1(w5, w[7]);
        if (differs(w[6], w[8])) {
            p12 = w5;
            p21 = w5;
            p22 = w5;
        } else {
            p12 = interp3(w5, w[6]);
            p21 = interp3(w5, w[8]);
            p22 = interp4(w5, w[6], w[8]);
        }
        break;
    case 125:
        if (differs(w[8], w[4])) {
            p00 = interp1(w5, w[2]);
            p10 = w5;
            p20 = w5;
            p21 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
            p10 = interp1(w[4], w5);
            p20 = interp5(w[8], w[4]);
            p21 = interp1(w5, w[8]);
        }
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[2]);
        p11 = w5;
        p12 = w5;
        p22 = interp1(w5, w[9]);
        break;
    case 221:
        if (differs(w[6], w[8])) {
            p02 = interp1(w5, w[2]);
            p12 = w5;
            p21 = w5;
            p22 = w5;
        } else {
            p02 = interp2(w5, w[2], w[6]);
            p12 = interp1(w[6], w5);
            p21 = interp1(w5, w[8]);
            p22 = interp5(w[6], w[8]);
        }
        p00 = interp1(w5, w[2]);
        p01 = interp1(w5, w[2]);
        p10 = w5;
        p11 = w5;
        p20 = interp1(w5, w[7]);
        break;
    case 207:
        if (differs(w[4], w[2])) {
            p00 = w5;
            p01 = w5;
            p02 = interp1(w5, w[6]);
            p10 = w5;
        } else {
            p00 = interp5(w[4], w[2]);
            p01 = interp1(w[2], w5);
            p02 = interp2(w5, w[2], w[6]);
            p10 = interp1(w5, w[4]);
        }
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[7]);
        p21 = w5;
        p22 = interp1(w5, w[6]);
        break;
    case 238:
        if (differs(w[8], w[4])) {
            p10 = w5;
            p20 = w5;
            p21 = w5;
            p22 = interp1(w5, w[6]);
        } else {
            p10 = interp1(w5, w[4]);
            p20 = interp5(w[8], w[4]);
            p21 = interp1(w[8], w5);
            p22 = interp2(w5, w[6], w[8]);
        }
        p00 = interp1(w5, w[1]);
        p01 = w5;
        p02 = interp1(w5, w[6]);
        p11 = w5;
        p12 = interp1(w5, w[6]);
        break;
    case 190:
        if (differs(w[2], w[6])) {
            p01 = w5;
            p02 = w5;
            p12 = w5;
            p22 = interp1(w5, w[8]);
        } else {
            p01 = interp1(w5, w[2]);
            p02 = interp5(w[2], w[6]);
            p12 = interp1(w[6], w5);
            p22 = interp2(w5, w[6], w[8]);
        }
        p00 = interp1(w5, w[1]);
        p10 = w5;
        p11 = w5;
        p20 = interp1(w5, w[8]);
        p21 = interp1(w5, w[8]);
        break;
    case 187:
        if (differs(w[4], w[2])) {
            p00 = w5;
            p01 = w5;
            p10 = w5;
            p20 = interp1(w5, w[8]);
        } else {
            p00 = interp5(w[4], w[2]);
            p01 = interp1(w5, w[2]);
            p10 = interp1(w[4], w5);
            p20 = interp2(w5, w[8], w[4]);
        }
        p02 = interp1(w5, w[3]);
        p11 = w5;
        p12 = w5;
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[8]);
        break;
    case 243:
        if (differs(w[6], w[8])) {
            p12 = w5;
            p20 = interp1(w5, w[4]);
            p21 = w5;
            p22 = w5;
        } else {
            p12 = interp1(w5, w[6]);
            p20 = interp2(w5, w[8], w[4]);
            p21 = interp1(w[8], w5);
            p22 = interp5(w[6], w[8]);
        }
        p00 = interp1(w5, w[4]);
        p01 = w5;
        p02 = interp1(w5, w[3]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        break;
    case 119:
        if (differs(w[2], w[6])) {
            p00 = interp1(w5, w[4]);
            p01 = w5;
            p02 = w5;
            p12 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
            p01 = interp1(w[2], w5);
            p02 = interp5(w[2], w[6]);
            p12 = interp1(w5, w[6]);
        }
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p20 = interp1(w5, w[4]);
        p21 = w5;
        p22 = interp1(w5, w[9]);
        break;
    case 233:
    case 237:
        p00 = interp1(w5, w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp2(w5, w[2], w[6]);
        p10 = w5;
        p11 = w5;
        p12 = interp1(w5, w[6]);
        if (differs(w[8], w[4])) {
            p20 = w5;
        } else {
            p20 = interp2(w5, w[8], w[4]);
        }
        p21 = w5;
        p22 = interp1(w5, w[6]);
        break;
    case 47:
    case 175:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = w5;
        p02 = interp1(w5, w[6]);
        p10 = w5;
        p11 = w5;
        p12 = interp1(w5, w[6]);
        p20 = interp1(w5, w[8]);
        p21 = interp1(w5, w[8]);
        p22 = interp2(w5, w[6], w[8]);
        break;
    case 151:
    case 183:
        p00 = interp1(w5, w[4]);
        p01 = w5;
        if (differs(w[2], w[6])) {
            p02 = w5;
        } else {
            p02 = interp2(w5, w[2], w[6]);
        }
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = w5;
        p20 = interp2(w5, w[8], w[4]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[8]);
        break;
    case 244:
    case 245:
        p00 = interp2(w5, w[4], w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[2]);
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[4]);
        p21 = w5;
        if (differs(w[6], w[8])) {
            p22 = w5;
        } else {
            p22 = interp2(w5, w[6], w[8]);
        }
        break;
    case 250:
        p00 = interp1(w5, w[1]);
        p01 = w5;
        p02 = interp1(w5, w[3]);
        p11 = w5;
        if (differs(w[8], w[4])) {
            p10 = w5;
            p20 = w5;
        } else {
            p10 = interp3(w5, w[4]);
            p20 = interp4(w5, w[8], w[4]);
        }
        p21 = w5;
        if (differs(w[6], w[8])) {
            p12 = w5;
            p22 = w5;
        } else {
            p12 = interp3(w5, w[6]);
            p22 = interp4(w5, w[6], w[8]);
        }
        break;
    case 123:
        if (differs(w[4], w[2])) {
            p00 = w5;
            p01 = w5;
        } else {
            p00 = interp4(w5, w[4], w[2]);
            p01 = interp3(w5, w[2]);
        }
        p02 = interp1(w5, w[3]);
        p10 = w5;
        p11 = w5;
        p12 = w5;
        if (differs(w[8], w[4])) {
            p20 = w5;
            p21 = w5;
        } else {
            p20 = interp4(w5, w[8], w[4]);
            p21 = interp3(w5, w[8]);
        }
        p22 = interp1(w5, w[9]);
        break;
    case 95:
        if (differs(w[4], w[2])) {
            p00 = w5;
            p10 = w5;
        } else {
            p00 = interp4(w5, w[4], w[2]);
            p10 = interp3(w5, w[4]);
        }
        p01 = w5;
        if (differs(w[2], w[6])) {
            p02 = w5;
            p12 = w5;
        } else {
            p02 = interp4(w5, w[2], w[6]);
            p12 = interp3(w5, w[6]);
        }
        p11 = w5;
        p20 = interp1(w5, w[7]);
        p21 = w5;
        p22 = interp1(w5, w[9]);
        break;
    case 222:
        p00 = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            p01 = w5;
            p02 = w5;
        } else {
            p01 = interp3(w5, w[2]);
            p02 = interp4(w5, w[2], w[6]);
        }
        p10 = w5;
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[7]);
        if (differs(w[6], w[8])) {
            p21 = w5;
            p22 = w5;
        } else {
            p21 = interp3(w5, w[8]);
            p22 = interp4(w5, w[6], w[8]);
        }
        break;
    case 252:
        p00 = interp1(w5, w[1]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[2]);
        p11 = w5;
        p12 = w5;
        if (differs(w[8], w[4])) {
            p10 = w5;
            p20 = w5;
        } else {
            p10 = interp3(w5, w[4]);
            p20 = interp4(w5, w[8], w[4]);
        }
        p21 = w5;
        if (differs(w[6], w[8])) {
            p22 = w5;
        } else {
            p22 = interp2(w5, w[6], w[8]);
        }
        break;
    case 249:
        p00 = interp1(w5, w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[3]);
        p10 = w5;
        p11 = w5;
        if (differs(w[8], w[4])) {
            p20 = w5;
        } else {
            p20 = interp2(w5, w[8], w[4]);
        }
        p21 = w5;
        if (differs(w[6], w[8])) {
            p12 = w5;
            p22 = w5;
        } else {
            p12 = interp3(w5, w[6]);
            p22 = interp4(w5, w[6], w[8]);
        }
        break;
    case 235:
        if (differs(w[4], w[2])) {
            p00 = w5;
            p01 = w5;
        } else {
            p00 = interp4(w5, w[4], w[2]);
            p01 = interp3(w5, w[2]);
        }
        p02 = interp1(w5, w[3]);
        p10 = w5;
        p11 = w5;
        p12 = interp1(w5, w[6]);
        if (differs(w[8], w[4])) {
            p20 = w5;
        } else {
            p20 = interp2(w5, w[8], w[4]);
        }
        p21 = w5;
        p22 = interp1(w5, w[6]);
        break;
    case 111:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = w5;
        p02 = interp1(w5, w[6]);
        p10 = w5;
        p11 = w5;
        p12 = interp1(w5, w[6]);
        if (differs(w[8], w[4])) {
            p20 = w5;
            p21 = w5;
        } else {
            p20 = interp4(w5, w[8], w[4]);
            p21 = interp3(w5, w[8]);
        }
        p22 = interp1(w5, w[9]);
        break;
    case 63:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = w5;
        if (differs(w[2], w[6])) {
            p02 = w5;
            p12 = w5;
        } else {
            p02 = interp4(w5, w[2], w[6]);
            p12 = interp3(w5, w[6]);
        }
        p10 = w5;
        p11 = w5;
        p20 = interp1(w5, w[8]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[9]);
        break;
    case 159:
        if (differs(w[4], w[2])) {
            p00 = w5;
            p10 = w5;
        } else {
            p00 = interp4(w5, w[4], w[2]);
            p10 = interp3(w5, w[4]);
        }
        p01 = w5;
        if (differs(w[2], w[6])) {
            p02 = w5;
        } else {
            p02 = interp2(w5, w[2], w[6]);
        }
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[7]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[8]);
        break;
    case 215:
        p00 = interp1(w5, w[4]);
        p01 = w5;
        if (differs(w[2], w[6])) {
            p02 = w5;
        } else {
            p02 = interp2(w5, w[2], w[6]);
        }
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[7]);
        if (differs(w[6], w[8])) {
            p21 = w5;
            p22 = w5;
        } else {
            p21 = interp3(w5, w[8]);
            p22 = interp4(w5, w[6], w[8]);
        }
        break;
    case 246:
        p00 = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            p01 = w5;
            p02 = w5;
        } else {
            p01 = interp3(w5, w[2]);
            p02 = interp4(w5, w[2], w[6]);
        }
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[4]);
        p21 = w5;
        if (differs(w[6], w[8])) {
            p22 = w5;
        } else {
            p22 = interp2(w5, w[6], w[8]);
        }
        break;
    case 254:
        p00 = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            p01 = w5;
            p02 = w5;
        } else {
            p01 = interp3(w5, w[2]);
            p02 = interp4(w5, w[2], w[6]);
        }
        p11 = w5;
        if (differs(w[8], w[4])) {
            p10 = w5;
            p20 = w5;
        } else {
            p10 = interp3(w5, w[4]);
            p20 = interp4(w5, w[8], w[4]);
        }
        if (differs(w[6], w[8])) {
            p12 = w5;
            p21 = w5;
            p22 = w5;
        } else {
            p12 = interp3(w5, w[6]);
            p21 = interp3(w5, w[8]);
            p22 = interp2(w5, w[6], w[8]);
        }
        break;
    case 253:
        p00 = interp1(w5, w[2]);
        p01 = interp1(w5, w[2]);
        p02 = interp1(w5, w[2]);
        p10 = w5;
        p11 = w5;
        p12 = w5;
        if (differs(w[8], w[4])) {
            p20 = w5;
        } else {
            p20 = interp2(w5, w[8], w[4]);
        }
        p21 = w5;
        if (differs(w[6], w[8])) {
            p22 = w5;
        } else {
            p22 = interp2(w5, w[6], w[8]);
        }
        break;
    case 251:
        if (differs(w[4], w[2])) {
            p00 = w5;
            p01 = w5;
        } else {
            p00 = interp4(w5, w[4], w[2]);
            p01 = interp3(w5, w[2]);
        }
        p02 = interp1(w5, w[3]);
        p11 = w5;
        if (differs(w[8], w[4])) {
            p10 = w5;
            p20 = w5;
            p21 = w5;
        } else {
            p10 = interp3(w5, w[4]);
            p20 = interp2(w5, w[8], w[4]);
            p21 = interp3(w5, w[8]);
        }
        if (differs(w[6], w[8])) {
            p12 = w5;
            p22 = w5;
        } else {
            p12 = interp3(w5, w[6]);
            p22 = interp4(w5, w[6], w[8]);
        }
        break;
    case 239:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = w5;
        p02 = interp1(w5, w[6]);
        p10 = w5;
        p11 = w5;
        p12 = interp1(w5, w[6]);
        if (differs(w[8], w[4])) {
            p20 = w5;
        } else {
            p20 = interp2(w5, w[8], w[4]);
        }
        p21 = w5;
        p22 = interp1(w5, w[6]);
        break;
    case 127:
        if (differs(w[4], w[2])) {
            p00 = w5;
            p01 = w5;
            p10 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
            p01 = interp3(w5, w[2]);
            p10 = interp3(w5, w[4]);
        }
        if (differs(w[2], w[6])) {
            p02 = w5;
            p12 = w5;
        } else {
            p02 = interp4(w5, w[2], w[6]);
            p12 = interp3(w5, w[6]);
        }
        p11 = w5;
        if (differs(w[8], w[4])) {
            p20 = w5;
            p21 = w5;
        } else {
            p20 = interp4(w5, w[8], w[4]);
            p21 = interp3(w5, w[8]);
        }
        p22 = interp1(w5, w[9]);
        break;
    case 191:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = w5;
        if (differs(w[2], w[6])) {
            p02 = w5;
        } else {
            p02 = interp2(w5, w[2], w[6]);
        }
        p10 = w5;
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[8]);
        p21 = interp1(w5, w[8]);
        p22 = interp1(w5, w[8]);
        break;
    case 223:
        if (differs(w[4], w[2])) {
            p00 = w5;
            p10 = w5;
        } else {
            p00 = interp4(w5, w[4], w[2]);
            p10 = interp3(w5, w[4]);
        }
        if (differs(w[2], w[6])) {
            p01 = w5;
            p02 = w5;
            p12 = w5;
        } else {
            p01 = interp3(w5, w[2]);
            p02 = interp2(w5, w[2], w[6]);
            p12 = interp3(w5, w[6]);
        }
        p11 = w5;
        p20 = interp1(w5, w[7]);
        if (differs(w[6], w[8])) {
            p21 = w5;
            p22 = w5;
        } else {
            p21 = interp3(w5, w[8]);
            p22 = interp4(w5, w[6], w[8]);
        }
        break;
    case 247:
        p00 = interp1(w5, w[4]);
        p01 = w5;
        if (differs(w[2], w[6])) {
            p02 = w5;
        } else {
            p02 = interp2(w5, w[2], w[6]);
        }
        p10 = interp1(w5, w[4]);
        p11 = w5;
        p12 = w5;
        p20 = interp1(w5, w[4]);
        p21 = w5;
        if (differs(w[6], w[8])) {
            p22 = w5;
        } else {
            p22 = interp2(w5, w[6], w[8]);
        }
        break;
    case 255:
        if (differs(w[4], w[2])) {
            p00 = w5;
        } else {
            p00 = interp2(w5, w[4], w[2]);
        }
        p01 = w5;
        if (differs(w[2], w[6])) {
            p02 = w5;
        } else {
            p02 = interp2(w5, w[2], w[6]);
        }
        p10 = w5;
        p11 = w5;
        p12 = w5;
        if (differs(w[8], w[4])) {
            p20 = w5;
        } else {
            p20 = interp2(w5, w[8], w[4]);
        }
        p21 = w5;
        if (differs(w[6], w[8])) {
            p22 = w5;
        } else {
            p22 = interp2(w5, w[6], w[8]);
        }
        break;
    default:
        p00 = p01 = p02 = p10 = p11 = p12 = p20 = p21 = p22 = w5;
        break;
    }

    out[0] = p00;
    out[1] = p01;
    out[2] = p02;
    out[3] = p10;
    out[4] = p11;
    out[5] = p12;
    out[6] = p20;
    out[7] = p21;
    out[8] = p22;
}

static void hq3x_pixel(u2 const w[10], u1* const dst, u4 const pitch)
{
    u2 q[9];

    hq3x_nine(w, q);
    for (unsigned r = 0; r < 3; r++) {
        u1* const row = dst + (size_t)pitch * r;

        st16u(row, q[r * 3]);
        st16u(row + 2, q[r * 3 + 1]);
        st16u(row + 4, q[r * 3 + 2]);
    }
}

static void hq3x_pixel32(u2 const w[10], u1* const dst, u4 const pitch)
{
    u4 const* const conv = (u4 const*)BitConv32Ptr;
    u2 q[9];

    hq3x_nine(w, q);
    for (unsigned r = 0; r < 3; r++) {
        u1* const row = dst + (size_t)pitch * r;

        st32u(row, conv[q[r * 3]]);
        st32u(row + 4, conv[q[r * 3 + 1]]);
        st32u(row + 8, conv[q[r * 3 + 2]]);
    }
}

void hq3x_16b(void)
{
    if (curblank == 0x40)
        return;

    if (!hqFilter || (!FilteredGUI && GUIOn2 == 1)) {
        hq3x_double_16b();
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
            for (u4 x = 0; x < 256; x++) {
                u4 const px = row[x] * 0x00010001u;
                u2 const p = row[x];

                st32u(dst + x * 6, px);
                st16u(dst + x * 6 + 4, p);
                st32u(dst + x * 6 + pitch, px);
                st16u(dst + x * 6 + pitch + 4, p);
                st32u(dst + x * 6 + pitch * 2, px);
                st16u(dst + x * 6 + pitch * 2 + 4, p);
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
                if (flat9(w))
                    fill_flat16(dst + x * 6, pitch, 3, w[5]);
                else
                    hq3x_pixel(w, dst + x * 6, pitch);
            }
        }
        /* 256 pixels as 1536 bytes, then the two rows below them. */
        dst += 256 * 6 + AddEndBytes + pitch * 2;
    }
}

static void hq3x_double_32b(void)
{
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

void hq2x_32b(void)
{
    if (curblank == 0x40)
        return;

    if (!hqFilter || (!FilteredGUI && GUIOn2 == 1)) {
        hq2x_double_32b();
        return;
    }

    u4 const* const conv = (u4 const*)BitConv32Ptr;
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
            /* Already 512 wide: replicated, not filtered, as in 16-bit. */
            for (u4 x = 0; x < 256; x++) {
                u4 const px = conv[row[x]];

                st32u(dst + x * 8, px);
                st32u(dst + x * 8 + 4, px);
                st32u(dst + x * 8 + pitch, px);
                st32u(dst + x * 8 + pitch + 4, px);
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
                if (flat9(w))
                    fill_flat32(dst + x * 8, pitch, 2, w[5]);
                else
                    hq2x_pixel32(w, dst + x * 8, pitch);
            }
        }
        dst += 256 * 8 + AddEndBytes + pitch * 1;
    }
}

void hq3x_32b(void)
{
    if (curblank == 0x40)
        return;

    if (!hqFilter || (!FilteredGUI && GUIOn2 == 1)) {
        hq3x_double_32b();
        return;
    }

    u4 const* const conv = (u4 const*)BitConv32Ptr;
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
            /* Already 512 wide: replicated, not filtered, as in 16-bit. */
            for (u4 x = 0; x < 256; x++) {
                u4 const px = conv[row[x]];

                for (unsigned r = 0; r < 3; r++) {
                    u1* const orow = dst + x * 12 + (size_t)pitch * r;

                    st32u(orow, px);
                    st32u(orow + 4, px);
                    st32u(orow + 8, px);
                }
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
                if (flat9(w))
                    fill_flat32(dst + x * 12, pitch, 3, w[5]);
                else
                    hq3x_pixel32(w, dst + x * 12, pitch);
            }
        }
        dst += 256 * 12 + AddEndBytes + pitch * 2;
    }
}

/* One source pixel's 4x4 result, row major, from video/hq4x16.asm's rule set
   (the MMX original, removed in 5ff6d63d). Every one of the 256 patterns
   assigns all sixteen outputs on every branch, as the assembly did. */
static void hq4x_sixteen(u2 const w[10], u2 out[16])
{
    u2 const w5 = w[5];
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
        out[0] = interp2(w5, w[2], w[4]);
        out[1] = interp6(w5, w[2], w[4]);
        out[2] = interp6(w5, w[2], w[6]);
        out[3] = interp2(w5, w[2], w[6]);
        out[4] = interp6(w5, w[4], w[2]);
        out[5] = interp7(w5, w[4], w[2]);
        out[6] = interp7(w5, w[6], w[2]);
        out[7] = interp6(w5, w[6], w[2]);
        out[8] = interp6(w5, w[4], w[8]);
        out[9] = interp7(w5, w[4], w[8]);
        out[10] = interp7(w5, w[6], w[8]);
        out[11] = interp6(w5, w[6], w[8]);
        out[12] = interp2(w5, w[8], w[4]);
        out[13] = interp6(w5, w[8], w[4]);
        out[14] = interp6(w5, w[8], w[6]);
        out[15] = interp2(w5, w[8], w[6]);
        break;
    case 2:
    case 34:
    case 130:
    case 162:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp6(w5, w[4], w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp6(w5, w[6], w[3]);
        out[8] = interp6(w5, w[4], w[8]);
        out[9] = interp7(w5, w[4], w[8]);
        out[10] = interp7(w5, w[6], w[8]);
        out[11] = interp6(w5, w[6], w[8]);
        out[12] = interp2(w5, w[8], w[4]);
        out[13] = interp6(w5, w[8], w[4]);
        out[14] = interp6(w5, w[8], w[6]);
        out[15] = interp2(w5, w[8], w[6]);
        break;
    case 3:
    case 35:
    case 131:
    case 163:
        out[0] = interp8(w5, w[4]);
        out[1] = interp3(w5, w[4]);
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp8(w5, w[4]);
        out[5] = interp3(w5, w[4]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp6(w5, w[6], w[3]);
        out[8] = interp6(w5, w[4], w[8]);
        out[9] = interp7(w5, w[4], w[8]);
        out[10] = interp7(w5, w[6], w[8]);
        out[11] = interp6(w5, w[6], w[8]);
        out[12] = interp2(w5, w[8], w[4]);
        out[13] = interp6(w5, w[8], w[4]);
        out[14] = interp6(w5, w[8], w[6]);
        out[15] = interp2(w5, w[8], w[6]);
        break;
    case 6:
    case 38:
    case 134:
    case 166:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        out[2] = interp3(w5, w[6]);
        out[3] = interp8(w5, w[6]);
        out[4] = interp6(w5, w[4], w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[6]);
        out[7] = interp8(w5, w[6]);
        out[8] = interp6(w5, w[4], w[8]);
        out[9] = interp7(w5, w[4], w[8]);
        out[10] = interp7(w5, w[6], w[8]);
        out[11] = interp6(w5, w[6], w[8]);
        out[12] = interp2(w5, w[8], w[4]);
        out[13] = interp6(w5, w[8], w[4]);
        out[14] = interp6(w5, w[8], w[6]);
        out[15] = interp2(w5, w[8], w[6]);
        break;
    case 7:
    case 39:
    case 135:
    case 167:
        out[0] = interp8(w5, w[4]);
        out[1] = interp3(w5, w[4]);
        out[2] = interp3(w5, w[6]);
        out[3] = interp8(w5, w[6]);
        out[4] = interp8(w5, w[4]);
        out[5] = interp3(w5, w[4]);
        out[6] = interp3(w5, w[6]);
        out[7] = interp8(w5, w[6]);
        out[8] = interp6(w5, w[4], w[8]);
        out[9] = interp7(w5, w[4], w[8]);
        out[10] = interp7(w5, w[6], w[8]);
        out[11] = interp6(w5, w[6], w[8]);
        out[12] = interp2(w5, w[8], w[4]);
        out[13] = interp6(w5, w[8], w[4]);
        out[14] = interp6(w5, w[8], w[6]);
        out[15] = interp2(w5, w[8], w[6]);
        break;
    case 8:
    case 12:
    case 136:
    case 140:
        out[0] = interp8(w5, w[1]);
        out[1] = interp6(w5, w[2], w[1]);
        out[2] = interp6(w5, w[2], w[6]);
        out[3] = interp2(w5, w[2], w[6]);
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp7(w5, w[6], w[2]);
        out[7] = interp6(w5, w[6], w[2]);
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp7(w5, w[6], w[8]);
        out[11] = interp6(w5, w[6], w[8]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp6(w5, w[8], w[7]);
        out[14] = interp6(w5, w[8], w[6]);
        out[15] = interp2(w5, w[8], w[6]);
        break;
    case 9:
    case 13:
    case 137:
    case 141:
        out[0] = interp8(w5, w[2]);
        out[1] = interp8(w5, w[2]);
        out[2] = interp6(w5, w[2], w[6]);
        out[3] = interp2(w5, w[2], w[6]);
        out[4] = interp3(w5, w[2]);
        out[5] = interp3(w5, w[2]);
        out[6] = interp7(w5, w[6], w[2]);
        out[7] = interp6(w5, w[6], w[2]);
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp7(w5, w[6], w[8]);
        out[11] = interp6(w5, w[6], w[8]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp6(w5, w[8], w[7]);
        out[14] = interp6(w5, w[8], w[6]);
        out[15] = interp2(w5, w[8], w[6]);
        break;
    case 10:
    case 138:
        if (differs(w[4], w[2])) {
            out[0] = interp8(w5, w[1]);
            out[1] = interp1(w5, w[1]);
            out[4] = interp1(w5, w[1]);
            out[5] = interp3(w5, w[1]);
        } else {
            out[0] = interp5(w[2], w[4]);
            out[1] = interp5(w[2], w5);
            out[4] = interp5(w[4], w5);
            out[5] = w5;
        }
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp6(w5, w[6], w[3]);
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp7(w5, w[6], w[8]);
        out[11] = interp6(w5, w[6], w[8]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp6(w5, w[8], w[7]);
        out[14] = interp6(w5, w[8], w[6]);
        out[15] = interp2(w5, w[8], w[6]);
        break;
    case 11:
    case 139:
        if (differs(w[4], w[2])) {
            out[0] = w5;
            out[1] = w5;
            out[4] = w5;
        } else {
            out[0] = interp5(w[2], w[4]);
            out[1] = interp5(w[2], w5);
            out[4] = interp5(w[4], w5);
        }
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[5] = w5;
        out[6] = interp3(w5, w[3]);
        out[7] = interp6(w5, w[6], w[3]);
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp7(w5, w[6], w[8]);
        out[11] = interp6(w5, w[6], w[8]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp6(w5, w[8], w[7]);
        out[14] = interp6(w5, w[8], w[6]);
        out[15] = interp2(w5, w[8], w[6]);
        break;
    case 14:
    case 142:
        if (differs(w[4], w[2])) {
            out[0] = interp8(w5, w[1]);
            out[1] = interp1(w5, w[1]);
            out[2] = interp3(w5, w[6]);
            out[3] = interp8(w5, w[6]);
            out[4] = interp1(w5, w[1]);
            out[5] = interp3(w5, w[1]);
        } else {
            out[0] = interp5(w[2], w[4]);
            out[1] = interp8(w[2], w[4]);
            out[2] = interp1(w[2], w5);
            out[3] = interp1(w5, w[2]);
            out[4] = interp2(w[4], w5, w[2]);
            out[5] = interp7(w5, w[4], w[2]);
        }
        out[6] = interp3(w5, w[6]);
        out[7] = interp8(w5, w[6]);
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp7(w5, w[6], w[8]);
        out[11] = interp6(w5, w[6], w[8]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp6(w5, w[8], w[7]);
        out[14] = interp6(w5, w[8], w[6]);
        out[15] = interp2(w5, w[8], w[6]);
        break;
    case 15:
    case 143:
        if (differs(w[4], w[2])) {
            out[0] = w5;
            out[1] = w5;
            out[2] = interp3(w5, w[6]);
            out[3] = interp8(w5, w[6]);
            out[4] = w5;
            out[5] = w5;
        } else {
            out[0] = interp5(w[2], w[4]);
            out[1] = interp8(w[2], w[4]);
            out[2] = interp1(w[2], w5);
            out[3] = interp1(w5, w[2]);
            out[4] = interp2(w[4], w5, w[2]);
            out[5] = interp7(w5, w[4], w[2]);
        }
        out[6] = interp3(w5, w[6]);
        out[7] = interp8(w5, w[6]);
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp7(w5, w[6], w[8]);
        out[11] = interp6(w5, w[6], w[8]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp6(w5, w[8], w[7]);
        out[14] = interp6(w5, w[8], w[6]);
        out[15] = interp2(w5, w[8], w[6]);
        break;
    case 16:
    case 17:
    case 48:
    case 49:
        out[0] = interp2(w5, w[2], w[4]);
        out[1] = interp6(w5, w[2], w[4]);
        out[2] = interp6(w5, w[2], w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp6(w5, w[4], w[2]);
        out[5] = interp7(w5, w[4], w[2]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        out[8] = interp6(w5, w[4], w[8]);
        out[9] = interp7(w5, w[4], w[8]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[12] = interp2(w5, w[8], w[4]);
        out[13] = interp6(w5, w[8], w[4]);
        out[14] = interp6(w5, w[8], w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 18:
    case 50:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            out[2] = interp1(w5, w[3]);
            out[3] = interp8(w5, w[3]);
            out[6] = interp3(w5, w[3]);
            out[7] = interp1(w5, w[3]);
        } else {
            out[2] = interp5(w[2], w5);
            out[3] = interp5(w[2], w[6]);
            out[6] = w5;
            out[7] = interp5(w[6], w5);
        }
        out[4] = interp6(w5, w[4], w[1]);
        out[5] = interp3(w5, w[1]);
        out[8] = interp6(w5, w[4], w[8]);
        out[9] = interp7(w5, w[4], w[8]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[12] = interp2(w5, w[8], w[4]);
        out[13] = interp6(w5, w[8], w[4]);
        out[14] = interp6(w5, w[8], w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 19:
    case 51:
        if (differs(w[2], w[6])) {
            out[0] = interp8(w5, w[4]);
            out[1] = interp3(w5, w[4]);
            out[2] = interp1(w5, w[3]);
            out[3] = interp8(w5, w[3]);
            out[6] = interp3(w5, w[3]);
            out[7] = interp1(w5, w[3]);
        } else {
            out[0] = interp1(w5, w[2]);
            out[1] = interp1(w[2], w5);
            out[2] = interp8(w[2], w[6]);
            out[3] = interp5(w[2], w[6]);
            out[6] = interp7(w5, w[6], w[2]);
            out[7] = interp2(w[6], w5, w[2]);
        }
        out[4] = interp8(w5, w[4]);
        out[5] = interp3(w5, w[4]);
        out[8] = interp6(w5, w[4], w[8]);
        out[9] = interp7(w5, w[4], w[8]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[12] = interp2(w5, w[8], w[4]);
        out[13] = interp6(w5, w[8], w[4]);
        out[14] = interp6(w5, w[8], w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 20:
    case 21:
    case 52:
    case 53:
        out[0] = interp2(w5, w[2], w[4]);
        out[1] = interp6(w5, w[2], w[4]);
        out[2] = interp8(w5, w[2]);
        out[3] = interp8(w5, w[2]);
        out[4] = interp6(w5, w[4], w[2]);
        out[5] = interp7(w5, w[4], w[2]);
        out[6] = interp3(w5, w[2]);
        out[7] = interp3(w5, w[2]);
        out[8] = interp6(w5, w[4], w[8]);
        out[9] = interp7(w5, w[4], w[8]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[12] = interp2(w5, w[8], w[4]);
        out[13] = interp6(w5, w[8], w[4]);
        out[14] = interp6(w5, w[8], w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 22:
    case 54:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            out[2] = w5;
            out[3] = w5;
            out[7] = w5;
        } else {
            out[2] = interp5(w[2], w5);
            out[3] = interp5(w[2], w[6]);
            out[7] = interp5(w[6], w5);
        }
        out[4] = interp6(w5, w[4], w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = w5;
        out[8] = interp6(w5, w[4], w[8]);
        out[9] = interp7(w5, w[4], w[8]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[12] = interp2(w5, w[8], w[4]);
        out[13] = interp6(w5, w[8], w[4]);
        out[14] = interp6(w5, w[8], w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 23:
    case 55:
        if (differs(w[2], w[6])) {
            out[0] = interp8(w5, w[4]);
            out[1] = interp3(w5, w[4]);
            out[2] = w5;
            out[3] = w5;
            out[6] = w5;
            out[7] = w5;
        } else {
            out[0] = interp1(w5, w[2]);
            out[1] = interp1(w[2], w5);
            out[2] = interp8(w[2], w[6]);
            out[3] = interp5(w[2], w[6]);
            out[6] = interp7(w5, w[6], w[2]);
            out[7] = interp2(w[6], w5, w[2]);
        }
        out[4] = interp8(w5, w[4]);
        out[5] = interp3(w5, w[4]);
        out[8] = interp6(w5, w[4], w[8]);
        out[9] = interp7(w5, w[4], w[8]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[12] = interp2(w5, w[8], w[4]);
        out[13] = interp6(w5, w[8], w[4]);
        out[14] = interp6(w5, w[8], w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 24:
        out[0] = interp8(w5, w[1]);
        out[1] = interp6(w5, w[2], w[1]);
        out[2] = interp6(w5, w[2], w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp6(w5, w[8], w[7]);
        out[14] = interp6(w5, w[8], w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 25:
        out[0] = interp8(w5, w[2]);
        out[1] = interp8(w5, w[2]);
        out[2] = interp6(w5, w[2], w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp3(w5, w[2]);
        out[5] = interp3(w5, w[2]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp6(w5, w[8], w[7]);
        out[14] = interp6(w5, w[8], w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 26:
    case 31:
        if (differs(w[4], w[2])) {
            out[0] = w5;
            out[1] = w5;
            out[4] = w5;
        } else {
            out[0] = interp5(w[2], w[4]);
            out[1] = interp5(w[2], w5);
            out[4] = interp5(w[4], w5);
        }
        if (differs(w[2], w[6])) {
            out[2] = w5;
            out[3] = w5;
            out[7] = w5;
        } else {
            out[2] = interp5(w[2], w5);
            out[3] = interp5(w[2], w[6]);
            out[7] = interp5(w[6], w5);
        }
        out[5] = w5;
        out[6] = w5;
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp6(w5, w[8], w[7]);
        out[14] = interp6(w5, w[8], w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 27:
        if (differs(w[4], w[2])) {
            out[0] = w5;
            out[1] = w5;
            out[4] = w5;
        } else {
            out[0] = interp5(w[2], w[4]);
            out[1] = interp5(w[2], w5);
            out[4] = interp5(w[4], w5);
        }
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[5] = w5;
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp6(w5, w[8], w[7]);
        out[14] = interp6(w5, w[8], w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 28:
        out[0] = interp8(w5, w[1]);
        out[1] = interp6(w5, w[2], w[1]);
        out[2] = interp8(w5, w[2]);
        out[3] = interp8(w5, w[2]);
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[2]);
        out[7] = interp3(w5, w[2]);
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp6(w5, w[8], w[7]);
        out[14] = interp6(w5, w[8], w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 29:
        out[0] = interp8(w5, w[2]);
        out[1] = interp8(w5, w[2]);
        out[2] = interp8(w5, w[2]);
        out[3] = interp8(w5, w[2]);
        out[4] = interp3(w5, w[2]);
        out[5] = interp3(w5, w[2]);
        out[6] = interp3(w5, w[2]);
        out[7] = interp3(w5, w[2]);
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp6(w5, w[8], w[7]);
        out[14] = interp6(w5, w[8], w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 30:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            out[2] = w5;
            out[3] = w5;
            out[7] = w5;
        } else {
            out[2] = interp5(w[2], w5);
            out[3] = interp5(w[2], w[6]);
            out[7] = interp5(w[6], w5);
        }
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = w5;
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp6(w5, w[8], w[7]);
        out[14] = interp6(w5, w[8], w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 40:
    case 44:
    case 168:
    case 172:
        out[0] = interp8(w5, w[1]);
        out[1] = interp6(w5, w[2], w[1]);
        out[2] = interp6(w5, w[2], w[6]);
        out[3] = interp2(w5, w[2], w[6]);
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp7(w5, w[6], w[2]);
        out[7] = interp6(w5, w[6], w[2]);
        out[8] = interp3(w5, w[8]);
        out[9] = interp3(w5, w[8]);
        out[10] = interp7(w5, w[6], w[8]);
        out[11] = interp6(w5, w[6], w[8]);
        out[12] = interp8(w5, w[8]);
        out[13] = interp8(w5, w[8]);
        out[14] = interp6(w5, w[8], w[6]);
        out[15] = interp2(w5, w[8], w[6]);
        break;
    case 41:
    case 45:
    case 169:
    case 173:
        out[0] = interp8(w5, w[2]);
        out[1] = interp8(w5, w[2]);
        out[2] = interp6(w5, w[2], w[6]);
        out[3] = interp2(w5, w[2], w[6]);
        out[4] = interp3(w5, w[2]);
        out[5] = interp3(w5, w[2]);
        out[6] = interp7(w5, w[6], w[2]);
        out[7] = interp6(w5, w[6], w[2]);
        out[8] = interp3(w5, w[8]);
        out[9] = interp3(w5, w[8]);
        out[10] = interp7(w5, w[6], w[8]);
        out[11] = interp6(w5, w[6], w[8]);
        out[12] = interp8(w5, w[8]);
        out[13] = interp8(w5, w[8]);
        out[14] = interp6(w5, w[8], w[6]);
        out[15] = interp2(w5, w[8], w[6]);
        break;
    case 42:
    case 170:
        if (differs(w[4], w[2])) {
            out[0] = interp8(w5, w[1]);
            out[1] = interp1(w5, w[1]);
            out[4] = interp1(w5, w[1]);
            out[5] = interp3(w5, w[1]);
            out[8] = interp3(w5, w[8]);
            out[12] = interp8(w5, w[8]);
        } else {
            out[0] = interp5(w[2], w[4]);
            out[1] = interp2(w[2], w5, w[4]);
            out[4] = interp8(w[4], w[2]);
            out[5] = interp7(w5, w[4], w[2]);
            out[8] = interp1(w[4], w5);
            out[12] = interp1(w5, w[4]);
        }
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp6(w5, w[6], w[3]);
        out[9] = interp3(w5, w[8]);
        out[10] = interp7(w5, w[6], w[8]);
        out[11] = interp6(w5, w[6], w[8]);
        out[13] = interp8(w5, w[8]);
        out[14] = interp6(w5, w[8], w[6]);
        out[15] = interp2(w5, w[8], w[6]);
        break;
    case 43:
    case 171:
        if (differs(w[4], w[2])) {
            out[0] = w5;
            out[1] = w5;
            out[4] = w5;
            out[5] = w5;
            out[8] = interp3(w5, w[8]);
            out[12] = interp8(w5, w[8]);
        } else {
            out[0] = interp5(w[2], w[4]);
            out[1] = interp2(w[2], w5, w[4]);
            out[4] = interp8(w[4], w[2]);
            out[5] = interp7(w5, w[4], w[2]);
            out[8] = interp1(w[4], w5);
            out[12] = interp1(w5, w[4]);
        }
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp6(w5, w[6], w[3]);
        out[9] = interp3(w5, w[8]);
        out[10] = interp7(w5, w[6], w[8]);
        out[11] = interp6(w5, w[6], w[8]);
        out[13] = interp8(w5, w[8]);
        out[14] = interp6(w5, w[8], w[6]);
        out[15] = interp2(w5, w[8], w[6]);
        break;
    case 46:
    case 174:
        if (differs(w[4], w[2])) {
            out[0] = interp8(w5, w[1]);
            out[1] = interp1(w5, w[1]);
            out[4] = interp1(w5, w[1]);
            out[5] = interp3(w5, w[1]);
        } else {
            out[0] = interp2(w5, w[2], w[4]);
            out[1] = interp1(w5, w[2]);
            out[4] = interp1(w5, w[4]);
            out[5] = w5;
        }
        out[2] = interp3(w5, w[6]);
        out[3] = interp8(w5, w[6]);
        out[6] = interp3(w5, w[6]);
        out[7] = interp8(w5, w[6]);
        out[8] = interp3(w5, w[8]);
        out[9] = interp3(w5, w[8]);
        out[10] = interp7(w5, w[6], w[8]);
        out[11] = interp6(w5, w[6], w[8]);
        out[12] = interp8(w5, w[8]);
        out[13] = interp8(w5, w[8]);
        out[14] = interp6(w5, w[8], w[6]);
        out[15] = interp2(w5, w[8], w[6]);
        break;
    case 47:
    case 175:
        if (differs(w[4], w[2])) {
            out[0] = w5;
        } else {
            out[0] = interp2(w5, w[2], w[4]);
        }
        out[1] = w5;
        out[2] = interp3(w5, w[6]);
        out[3] = interp8(w5, w[6]);
        out[4] = w5;
        out[5] = w5;
        out[6] = interp3(w5, w[6]);
        out[7] = interp8(w5, w[6]);
        out[8] = interp3(w5, w[8]);
        out[9] = interp3(w5, w[8]);
        out[10] = interp7(w5, w[6], w[8]);
        out[11] = interp6(w5, w[6], w[8]);
        out[12] = interp8(w5, w[8]);
        out[13] = interp8(w5, w[8]);
        out[14] = interp6(w5, w[8], w[6]);
        out[15] = interp2(w5, w[8], w[6]);
        break;
    case 56:
        out[0] = interp8(w5, w[1]);
        out[1] = interp6(w5, w[2], w[1]);
        out[2] = interp6(w5, w[2], w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        out[8] = interp3(w5, w[8]);
        out[9] = interp3(w5, w[8]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[12] = interp8(w5, w[8]);
        out[13] = interp8(w5, w[8]);
        out[14] = interp6(w5, w[8], w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 57:
        out[0] = interp8(w5, w[2]);
        out[1] = interp8(w5, w[2]);
        out[2] = interp6(w5, w[2], w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp3(w5, w[2]);
        out[5] = interp3(w5, w[2]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        out[8] = interp3(w5, w[8]);
        out[9] = interp3(w5, w[8]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[12] = interp8(w5, w[8]);
        out[13] = interp8(w5, w[8]);
        out[14] = interp6(w5, w[8], w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 58:
        if (differs(w[4], w[2])) {
            out[0] = interp8(w5, w[1]);
            out[1] = interp1(w5, w[1]);
            out[4] = interp1(w5, w[1]);
            out[5] = interp3(w5, w[1]);
        } else {
            out[0] = interp2(w5, w[2], w[4]);
            out[1] = interp1(w5, w[2]);
            out[4] = interp1(w5, w[4]);
            out[5] = w5;
        }
        if (differs(w[2], w[6])) {
            out[2] = interp1(w5, w[3]);
            out[3] = interp8(w5, w[3]);
            out[6] = interp3(w5, w[3]);
            out[7] = interp1(w5, w[3]);
        } else {
            out[2] = interp1(w5, w[2]);
            out[3] = interp2(w5, w[2], w[6]);
            out[6] = w5;
            out[7] = interp1(w5, w[6]);
        }
        out[8] = interp3(w5, w[8]);
        out[9] = interp3(w5, w[8]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[12] = interp8(w5, w[8]);
        out[13] = interp8(w5, w[8]);
        out[14] = interp6(w5, w[8], w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 59:
        if (differs(w[4], w[2])) {
            out[0] = w5;
            out[1] = w5;
            out[4] = w5;
        } else {
            out[0] = interp5(w[2], w[4]);
            out[1] = interp5(w[2], w5);
            out[4] = interp5(w[4], w5);
        }
        if (differs(w[2], w[6])) {
            out[2] = interp1(w5, w[3]);
            out[3] = interp8(w5, w[3]);
            out[6] = interp3(w5, w[3]);
            out[7] = interp1(w5, w[3]);
        } else {
            out[2] = interp1(w5, w[2]);
            out[3] = interp2(w5, w[2], w[6]);
            out[6] = w5;
            out[7] = interp1(w5, w[6]);
        }
        out[5] = w5;
        out[8] = interp3(w5, w[8]);
        out[9] = interp3(w5, w[8]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[12] = interp8(w5, w[8]);
        out[13] = interp8(w5, w[8]);
        out[14] = interp6(w5, w[8], w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 60:
        out[0] = interp8(w5, w[1]);
        out[1] = interp6(w5, w[2], w[1]);
        out[2] = interp8(w5, w[2]);
        out[3] = interp8(w5, w[2]);
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[2]);
        out[7] = interp3(w5, w[2]);
        out[8] = interp3(w5, w[8]);
        out[9] = interp3(w5, w[8]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[12] = interp8(w5, w[8]);
        out[13] = interp8(w5, w[8]);
        out[14] = interp6(w5, w[8], w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 61:
        out[0] = interp8(w5, w[2]);
        out[1] = interp8(w5, w[2]);
        out[2] = interp8(w5, w[2]);
        out[3] = interp8(w5, w[2]);
        out[4] = interp3(w5, w[2]);
        out[5] = interp3(w5, w[2]);
        out[6] = interp3(w5, w[2]);
        out[7] = interp3(w5, w[2]);
        out[8] = interp3(w5, w[8]);
        out[9] = interp3(w5, w[8]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[12] = interp8(w5, w[8]);
        out[13] = interp8(w5, w[8]);
        out[14] = interp6(w5, w[8], w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 62:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            out[2] = w5;
            out[3] = w5;
            out[7] = w5;
        } else {
            out[2] = interp5(w[2], w5);
            out[3] = interp5(w[2], w[6]);
            out[7] = interp5(w[6], w5);
        }
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = w5;
        out[8] = interp3(w5, w[8]);
        out[9] = interp3(w5, w[8]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[12] = interp8(w5, w[8]);
        out[13] = interp8(w5, w[8]);
        out[14] = interp6(w5, w[8], w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 63:
        if (differs(w[4], w[2])) {
            out[0] = w5;
        } else {
            out[0] = interp2(w5, w[2], w[4]);
        }
        out[1] = w5;
        if (differs(w[2], w[6])) {
            out[2] = w5;
            out[3] = w5;
            out[7] = w5;
        } else {
            out[2] = interp5(w[2], w5);
            out[3] = interp5(w[2], w[6]);
            out[7] = interp5(w[6], w5);
        }
        out[4] = w5;
        out[5] = w5;
        out[6] = w5;
        out[8] = interp3(w5, w[8]);
        out[9] = interp3(w5, w[8]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[12] = interp8(w5, w[8]);
        out[13] = interp8(w5, w[8]);
        out[14] = interp6(w5, w[8], w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 64:
    case 65:
    case 68:
    case 69:
        out[0] = interp2(w5, w[2], w[4]);
        out[1] = interp6(w5, w[2], w[4]);
        out[2] = interp6(w5, w[2], w[6]);
        out[3] = interp2(w5, w[2], w[6]);
        out[4] = interp6(w5, w[4], w[2]);
        out[5] = interp7(w5, w[4], w[2]);
        out[6] = interp7(w5, w[6], w[2]);
        out[7] = interp6(w5, w[6], w[2]);
        out[8] = interp6(w5, w[4], w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp6(w5, w[6], w[9]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 66:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp6(w5, w[4], w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp6(w5, w[6], w[3]);
        out[8] = interp6(w5, w[4], w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp6(w5, w[6], w[9]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 67:
        out[0] = interp8(w5, w[4]);
        out[1] = interp3(w5, w[4]);
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp8(w5, w[4]);
        out[5] = interp3(w5, w[4]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp6(w5, w[6], w[3]);
        out[8] = interp6(w5, w[4], w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp6(w5, w[6], w[9]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 70:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        out[2] = interp3(w5, w[6]);
        out[3] = interp8(w5, w[6]);
        out[4] = interp6(w5, w[4], w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[6]);
        out[7] = interp8(w5, w[6]);
        out[8] = interp6(w5, w[4], w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp6(w5, w[6], w[9]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 71:
        out[0] = interp8(w5, w[4]);
        out[1] = interp3(w5, w[4]);
        out[2] = interp3(w5, w[6]);
        out[3] = interp8(w5, w[6]);
        out[4] = interp8(w5, w[4]);
        out[5] = interp3(w5, w[4]);
        out[6] = interp3(w5, w[6]);
        out[7] = interp8(w5, w[6]);
        out[8] = interp6(w5, w[4], w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp6(w5, w[6], w[9]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 72:
    case 76:
        out[0] = interp8(w5, w[1]);
        out[1] = interp6(w5, w[2], w[1]);
        out[2] = interp6(w5, w[2], w[6]);
        out[3] = interp2(w5, w[2], w[6]);
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp7(w5, w[6], w[2]);
        out[7] = interp6(w5, w[6], w[2]);
        if (differs(w[8], w[4])) {
            out[8] = interp1(w5, w[7]);
            out[9] = interp3(w5, w[7]);
            out[12] = interp8(w5, w[7]);
            out[13] = interp1(w5, w[7]);
        } else {
            out[8] = interp5(w[4], w5);
            out[9] = w5;
            out[12] = interp5(w[8], w[4]);
            out[13] = interp5(w[8], w5);
        }
        out[10] = interp3(w5, w[9]);
        out[11] = interp6(w5, w[6], w[9]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 73:
    case 77:
        if (differs(w[8], w[4])) {
            out[0] = interp8(w5, w[2]);
            out[4] = interp3(w5, w[2]);
            out[8] = interp1(w5, w[7]);
            out[9] = interp3(w5, w[7]);
            out[12] = interp8(w5, w[7]);
            out[13] = interp1(w5, w[7]);
        } else {
            out[0] = interp1(w5, w[4]);
            out[4] = interp1(w[4], w5);
            out[8] = interp8(w[4], w[8]);
            out[9] = interp7(w5, w[4], w[8]);
            out[12] = interp5(w[8], w[4]);
            out[13] = interp2(w[8], w5, w[4]);
        }
        out[1] = interp8(w5, w[2]);
        out[2] = interp6(w5, w[2], w[6]);
        out[3] = interp2(w5, w[2], w[6]);
        out[5] = interp3(w5, w[2]);
        out[6] = interp7(w5, w[6], w[2]);
        out[7] = interp6(w5, w[6], w[2]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp6(w5, w[6], w[9]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 74:
    case 107:
        if (differs(w[4], w[2])) {
            out[0] = w5;
            out[1] = w5;
            out[4] = w5;
        } else {
            out[0] = interp5(w[2], w[4]);
            out[1] = interp5(w[2], w5);
            out[4] = interp5(w[4], w5);
        }
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[5] = w5;
        out[6] = interp3(w5, w[3]);
        out[7] = interp6(w5, w[6], w[3]);
        if (differs(w[8], w[4])) {
            out[8] = w5;
            out[12] = w5;
            out[13] = w5;
        } else {
            out[8] = interp5(w[4], w5);
            out[12] = interp5(w[8], w[4]);
            out[13] = interp5(w[8], w5);
        }
        out[9] = w5;
        out[10] = interp3(w5, w[9]);
        out[11] = interp6(w5, w[6], w[9]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 75:
        if (differs(w[4], w[2])) {
            out[0] = w5;
            out[1] = w5;
            out[4] = w5;
        } else {
            out[0] = interp5(w[2], w[4]);
            out[1] = interp5(w[2], w5);
            out[4] = interp5(w[4], w5);
        }
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[5] = w5;
        out[6] = interp3(w5, w[3]);
        out[7] = interp6(w5, w[6], w[3]);
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp6(w5, w[6], w[9]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 78:
        if (differs(w[4], w[2])) {
            out[0] = interp8(w5, w[1]);
            out[1] = interp1(w5, w[1]);
            out[4] = interp1(w5, w[1]);
            out[5] = interp3(w5, w[1]);
        } else {
            out[0] = interp2(w5, w[2], w[4]);
            out[1] = interp1(w5, w[2]);
            out[4] = interp1(w5, w[4]);
            out[5] = w5;
        }
        out[2] = interp3(w5, w[6]);
        out[3] = interp8(w5, w[6]);
        out[6] = interp3(w5, w[6]);
        out[7] = interp8(w5, w[6]);
        if (differs(w[8], w[4])) {
            out[8] = interp1(w5, w[7]);
            out[9] = interp3(w5, w[7]);
            out[12] = interp8(w5, w[7]);
            out[13] = interp1(w5, w[7]);
        } else {
            out[8] = interp1(w5, w[4]);
            out[9] = w5;
            out[12] = interp2(w5, w[8], w[4]);
            out[13] = interp1(w5, w[8]);
        }
        out[10] = interp3(w5, w[9]);
        out[11] = interp6(w5, w[6], w[9]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 79:
        if (differs(w[4], w[2])) {
            out[0] = w5;
            out[1] = w5;
            out[4] = w5;
        } else {
            out[0] = interp5(w[2], w[4]);
            out[1] = interp5(w[2], w5);
            out[4] = interp5(w[4], w5);
        }
        out[2] = interp3(w5, w[6]);
        out[3] = interp8(w5, w[6]);
        out[5] = w5;
        out[6] = interp3(w5, w[6]);
        out[7] = interp8(w5, w[6]);
        if (differs(w[8], w[4])) {
            out[8] = interp1(w5, w[7]);
            out[9] = interp3(w5, w[7]);
            out[12] = interp8(w5, w[7]);
            out[13] = interp1(w5, w[7]);
        } else {
            out[8] = interp1(w5, w[4]);
            out[9] = w5;
            out[12] = interp2(w5, w[8], w[4]);
            out[13] = interp1(w5, w[8]);
        }
        out[10] = interp3(w5, w[9]);
        out[11] = interp6(w5, w[6], w[9]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 80:
    case 81:
        out[0] = interp2(w5, w[2], w[4]);
        out[1] = interp6(w5, w[2], w[4]);
        out[2] = interp6(w5, w[2], w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp6(w5, w[4], w[2]);
        out[5] = interp7(w5, w[4], w[2]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        out[8] = interp6(w5, w[4], w[7]);
        out[9] = interp3(w5, w[7]);
        if (differs(w[6], w[8])) {
            out[10] = interp3(w5, w[9]);
            out[11] = interp1(w5, w[9]);
            out[14] = interp1(w5, w[9]);
            out[15] = interp8(w5, w[9]);
        } else {
            out[10] = w5;
            out[11] = interp5(w[6], w5);
            out[14] = interp5(w[8], w5);
            out[15] = interp5(w[8], w[6]);
        }
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        break;
    case 82:
    case 214:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            out[2] = w5;
            out[3] = w5;
            out[7] = w5;
        } else {
            out[2] = interp5(w[2], w5);
            out[3] = interp5(w[2], w[6]);
            out[7] = interp5(w[6], w5);
        }
        out[4] = interp6(w5, w[4], w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = w5;
        out[8] = interp6(w5, w[4], w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = w5;
        if (differs(w[6], w[8])) {
            out[11] = w5;
            out[14] = w5;
            out[15] = w5;
        } else {
            out[11] = interp5(w[6], w5);
            out[14] = interp5(w[8], w5);
            out[15] = interp5(w[8], w[6]);
        }
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        break;
    case 83:
        out[0] = interp8(w5, w[4]);
        out[1] = interp3(w5, w[4]);
        if (differs(w[2], w[6])) {
            out[2] = interp1(w5, w[3]);
            out[3] = interp8(w5, w[3]);
            out[6] = interp3(w5, w[3]);
            out[7] = interp1(w5, w[3]);
        } else {
            out[2] = interp1(w5, w[2]);
            out[3] = interp2(w5, w[2], w[6]);
            out[6] = w5;
            out[7] = interp1(w5, w[6]);
        }
        out[4] = interp8(w5, w[4]);
        out[5] = interp3(w5, w[4]);
        out[8] = interp6(w5, w[4], w[7]);
        out[9] = interp3(w5, w[7]);
        if (differs(w[6], w[8])) {
            out[10] = interp3(w5, w[9]);
            out[11] = interp1(w5, w[9]);
            out[14] = interp1(w5, w[9]);
            out[15] = interp8(w5, w[9]);
        } else {
            out[10] = w5;
            out[11] = interp1(w5, w[6]);
            out[14] = interp1(w5, w[8]);
            out[15] = interp2(w5, w[8], w[6]);
        }
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        break;
    case 84:
    case 85:
        out[0] = interp2(w5, w[2], w[4]);
        out[1] = interp6(w5, w[2], w[4]);
        out[2] = interp8(w5, w[2]);
        if (differs(w[6], w[8])) {
            out[3] = interp8(w5, w[2]);
            out[7] = interp3(w5, w[2]);
            out[10] = interp3(w5, w[9]);
            out[11] = interp1(w5, w[9]);
            out[14] = interp1(w5, w[9]);
            out[15] = interp8(w5, w[9]);
        } else {
            out[3] = interp1(w5, w[6]);
            out[7] = interp1(w[6], w5);
            out[10] = interp7(w5, w[6], w[8]);
            out[11] = interp8(w[6], w[8]);
            out[14] = interp2(w[8], w5, w[6]);
            out[15] = interp5(w[8], w[6]);
        }
        out[4] = interp6(w5, w[4], w[2]);
        out[5] = interp7(w5, w[4], w[2]);
        out[6] = interp3(w5, w[2]);
        out[8] = interp6(w5, w[4], w[7]);
        out[9] = interp3(w5, w[7]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        break;
    case 86:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            out[2] = w5;
            out[3] = w5;
            out[7] = w5;
        } else {
            out[2] = interp5(w[2], w5);
            out[3] = interp5(w[2], w[6]);
            out[7] = interp5(w[6], w5);
        }
        out[4] = interp6(w5, w[4], w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = w5;
        out[8] = interp6(w5, w[4], w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 87:
        out[0] = interp8(w5, w[4]);
        out[1] = interp3(w5, w[4]);
        if (differs(w[2], w[6])) {
            out[2] = w5;
            out[3] = w5;
            out[7] = w5;
        } else {
            out[2] = interp5(w[2], w5);
            out[3] = interp5(w[2], w[6]);
            out[7] = interp5(w[6], w5);
        }
        out[4] = interp8(w5, w[4]);
        out[5] = interp3(w5, w[4]);
        out[6] = w5;
        out[8] = interp6(w5, w[4], w[7]);
        out[9] = interp3(w5, w[7]);
        if (differs(w[6], w[8])) {
            out[10] = interp3(w5, w[9]);
            out[11] = interp1(w5, w[9]);
            out[14] = interp1(w5, w[9]);
            out[15] = interp8(w5, w[9]);
        } else {
            out[10] = w5;
            out[11] = interp1(w5, w[6]);
            out[14] = interp1(w5, w[8]);
            out[15] = interp2(w5, w[8], w[6]);
        }
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        break;
    case 88:
    case 248:
        out[0] = interp8(w5, w[1]);
        out[1] = interp6(w5, w[2], w[1]);
        out[2] = interp6(w5, w[2], w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        if (differs(w[8], w[4])) {
            out[8] = w5;
            out[12] = w5;
            out[13] = w5;
        } else {
            out[8] = interp5(w[4], w5);
            out[12] = interp5(w[8], w[4]);
            out[13] = interp5(w[8], w5);
        }
        out[9] = w5;
        out[10] = w5;
        if (differs(w[6], w[8])) {
            out[11] = w5;
            out[14] = w5;
            out[15] = w5;
        } else {
            out[11] = interp5(w[6], w5);
            out[14] = interp5(w[8], w5);
            out[15] = interp5(w[8], w[6]);
        }
        break;
    case 89:
        out[0] = interp8(w5, w[2]);
        out[1] = interp8(w5, w[2]);
        out[2] = interp6(w5, w[2], w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp3(w5, w[2]);
        out[5] = interp3(w5, w[2]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        if (differs(w[8], w[4])) {
            out[8] = interp1(w5, w[7]);
            out[9] = interp3(w5, w[7]);
            out[12] = interp8(w5, w[7]);
            out[13] = interp1(w5, w[7]);
        } else {
            out[8] = interp1(w5, w[4]);
            out[9] = w5;
            out[12] = interp2(w5, w[8], w[4]);
            out[13] = interp1(w5, w[8]);
        }
        if (differs(w[6], w[8])) {
            out[10] = interp3(w5, w[9]);
            out[11] = interp1(w5, w[9]);
            out[14] = interp1(w5, w[9]);
            out[15] = interp8(w5, w[9]);
        } else {
            out[10] = w5;
            out[11] = interp1(w5, w[6]);
            out[14] = interp1(w5, w[8]);
            out[15] = interp2(w5, w[8], w[6]);
        }
        break;
    case 90:
        if (differs(w[4], w[2])) {
            out[0] = interp8(w5, w[1]);
            out[1] = interp1(w5, w[1]);
            out[4] = interp1(w5, w[1]);
            out[5] = interp3(w5, w[1]);
        } else {
            out[0] = interp2(w5, w[2], w[4]);
            out[1] = interp1(w5, w[2]);
            out[4] = interp1(w5, w[4]);
            out[5] = w5;
        }
        if (differs(w[2], w[6])) {
            out[2] = interp1(w5, w[3]);
            out[3] = interp8(w5, w[3]);
            out[6] = interp3(w5, w[3]);
            out[7] = interp1(w5, w[3]);
        } else {
            out[2] = interp1(w5, w[2]);
            out[3] = interp2(w5, w[2], w[6]);
            out[6] = w5;
            out[7] = interp1(w5, w[6]);
        }
        if (differs(w[8], w[4])) {
            out[8] = interp1(w5, w[7]);
            out[9] = interp3(w5, w[7]);
            out[12] = interp8(w5, w[7]);
            out[13] = interp1(w5, w[7]);
        } else {
            out[8] = interp1(w5, w[4]);
            out[9] = w5;
            out[12] = interp2(w5, w[8], w[4]);
            out[13] = interp1(w5, w[8]);
        }
        if (differs(w[6], w[8])) {
            out[10] = interp3(w5, w[9]);
            out[11] = interp1(w5, w[9]);
            out[14] = interp1(w5, w[9]);
            out[15] = interp8(w5, w[9]);
        } else {
            out[10] = w5;
            out[11] = interp1(w5, w[6]);
            out[14] = interp1(w5, w[8]);
            out[15] = interp2(w5, w[8], w[6]);
        }
        break;
    case 91:
        if (differs(w[4], w[2])) {
            out[0] = w5;
            out[1] = w5;
            out[4] = w5;
        } else {
            out[0] = interp5(w[2], w[4]);
            out[1] = interp5(w[2], w5);
            out[4] = interp5(w[4], w5);
        }
        if (differs(w[2], w[6])) {
            out[2] = interp1(w5, w[3]);
            out[3] = interp8(w5, w[3]);
            out[6] = interp3(w5, w[3]);
            out[7] = interp1(w5, w[3]);
        } else {
            out[2] = interp1(w5, w[2]);
            out[3] = interp2(w5, w[2], w[6]);
            out[6] = w5;
            out[7] = interp1(w5, w[6]);
        }
        out[5] = w5;
        if (differs(w[8], w[4])) {
            out[8] = interp1(w5, w[7]);
            out[9] = interp3(w5, w[7]);
            out[12] = interp8(w5, w[7]);
            out[13] = interp1(w5, w[7]);
        } else {
            out[8] = interp1(w5, w[4]);
            out[9] = w5;
            out[12] = interp2(w5, w[8], w[4]);
            out[13] = interp1(w5, w[8]);
        }
        if (differs(w[6], w[8])) {
            out[10] = interp3(w5, w[9]);
            out[11] = interp1(w5, w[9]);
            out[14] = interp1(w5, w[9]);
            out[15] = interp8(w5, w[9]);
        } else {
            out[10] = w5;
            out[11] = interp1(w5, w[6]);
            out[14] = interp1(w5, w[8]);
            out[15] = interp2(w5, w[8], w[6]);
        }
        break;
    case 92:
        out[0] = interp8(w5, w[1]);
        out[1] = interp6(w5, w[2], w[1]);
        out[2] = interp8(w5, w[2]);
        out[3] = interp8(w5, w[2]);
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[2]);
        out[7] = interp3(w5, w[2]);
        if (differs(w[8], w[4])) {
            out[8] = interp1(w5, w[7]);
            out[9] = interp3(w5, w[7]);
            out[12] = interp8(w5, w[7]);
            out[13] = interp1(w5, w[7]);
        } else {
            out[8] = interp1(w5, w[4]);
            out[9] = w5;
            out[12] = interp2(w5, w[8], w[4]);
            out[13] = interp1(w5, w[8]);
        }
        if (differs(w[6], w[8])) {
            out[10] = interp3(w5, w[9]);
            out[11] = interp1(w5, w[9]);
            out[14] = interp1(w5, w[9]);
            out[15] = interp8(w5, w[9]);
        } else {
            out[10] = w5;
            out[11] = interp1(w5, w[6]);
            out[14] = interp1(w5, w[8]);
            out[15] = interp2(w5, w[8], w[6]);
        }
        break;
    case 93:
        out[0] = interp8(w5, w[2]);
        out[1] = interp8(w5, w[2]);
        out[2] = interp8(w5, w[2]);
        out[3] = interp8(w5, w[2]);
        out[4] = interp3(w5, w[2]);
        out[5] = interp3(w5, w[2]);
        out[6] = interp3(w5, w[2]);
        out[7] = interp3(w5, w[2]);
        if (differs(w[8], w[4])) {
            out[8] = interp1(w5, w[7]);
            out[9] = interp3(w5, w[7]);
            out[12] = interp8(w5, w[7]);
            out[13] = interp1(w5, w[7]);
        } else {
            out[8] = interp1(w5, w[4]);
            out[9] = w5;
            out[12] = interp2(w5, w[8], w[4]);
            out[13] = interp1(w5, w[8]);
        }
        if (differs(w[6], w[8])) {
            out[10] = interp3(w5, w[9]);
            out[11] = interp1(w5, w[9]);
            out[14] = interp1(w5, w[9]);
            out[15] = interp8(w5, w[9]);
        } else {
            out[10] = w5;
            out[11] = interp1(w5, w[6]);
            out[14] = interp1(w5, w[8]);
            out[15] = interp2(w5, w[8], w[6]);
        }
        break;
    case 94:
        if (differs(w[4], w[2])) {
            out[0] = interp8(w5, w[1]);
            out[1] = interp1(w5, w[1]);
            out[4] = interp1(w5, w[1]);
            out[5] = interp3(w5, w[1]);
        } else {
            out[0] = interp2(w5, w[2], w[4]);
            out[1] = interp1(w5, w[2]);
            out[4] = interp1(w5, w[4]);
            out[5] = w5;
        }
        if (differs(w[2], w[6])) {
            out[2] = w5;
            out[3] = w5;
            out[7] = w5;
        } else {
            out[2] = interp5(w[2], w5);
            out[3] = interp5(w[2], w[6]);
            out[7] = interp5(w[6], w5);
        }
        out[6] = w5;
        if (differs(w[8], w[4])) {
            out[8] = interp1(w5, w[7]);
            out[9] = interp3(w5, w[7]);
            out[12] = interp8(w5, w[7]);
            out[13] = interp1(w5, w[7]);
        } else {
            out[8] = interp1(w5, w[4]);
            out[9] = w5;
            out[12] = interp2(w5, w[8], w[4]);
            out[13] = interp1(w5, w[8]);
        }
        if (differs(w[6], w[8])) {
            out[10] = interp3(w5, w[9]);
            out[11] = interp1(w5, w[9]);
            out[14] = interp1(w5, w[9]);
            out[15] = interp8(w5, w[9]);
        } else {
            out[10] = w5;
            out[11] = interp1(w5, w[6]);
            out[14] = interp1(w5, w[8]);
            out[15] = interp2(w5, w[8], w[6]);
        }
        break;
    case 95:
        if (differs(w[4], w[2])) {
            out[0] = w5;
            out[1] = w5;
            out[4] = w5;
        } else {
            out[0] = interp5(w[2], w[4]);
            out[1] = interp5(w[2], w5);
            out[4] = interp5(w[4], w5);
        }
        if (differs(w[2], w[6])) {
            out[2] = w5;
            out[3] = w5;
            out[7] = w5;
        } else {
            out[2] = interp5(w[2], w5);
            out[3] = interp5(w[2], w[6]);
            out[7] = interp5(w[6], w5);
        }
        out[5] = w5;
        out[6] = w5;
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 96:
    case 97:
    case 100:
    case 101:
        out[0] = interp2(w5, w[2], w[4]);
        out[1] = interp6(w5, w[2], w[4]);
        out[2] = interp6(w5, w[2], w[6]);
        out[3] = interp2(w5, w[2], w[6]);
        out[4] = interp6(w5, w[4], w[2]);
        out[5] = interp7(w5, w[4], w[2]);
        out[6] = interp7(w5, w[6], w[2]);
        out[7] = interp6(w5, w[6], w[2]);
        out[8] = interp8(w5, w[4]);
        out[9] = interp3(w5, w[4]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp6(w5, w[6], w[9]);
        out[12] = interp8(w5, w[4]);
        out[13] = interp3(w5, w[4]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 98:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp6(w5, w[4], w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp6(w5, w[6], w[3]);
        out[8] = interp8(w5, w[4]);
        out[9] = interp3(w5, w[4]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp6(w5, w[6], w[9]);
        out[12] = interp8(w5, w[4]);
        out[13] = interp3(w5, w[4]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 99:
        out[0] = interp8(w5, w[4]);
        out[1] = interp3(w5, w[4]);
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp8(w5, w[4]);
        out[5] = interp3(w5, w[4]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp6(w5, w[6], w[3]);
        out[8] = interp8(w5, w[4]);
        out[9] = interp3(w5, w[4]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp6(w5, w[6], w[9]);
        out[12] = interp8(w5, w[4]);
        out[13] = interp3(w5, w[4]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 102:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        out[2] = interp3(w5, w[6]);
        out[3] = interp8(w5, w[6]);
        out[4] = interp6(w5, w[4], w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[6]);
        out[7] = interp8(w5, w[6]);
        out[8] = interp8(w5, w[4]);
        out[9] = interp3(w5, w[4]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp6(w5, w[6], w[9]);
        out[12] = interp8(w5, w[4]);
        out[13] = interp3(w5, w[4]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 103:
        out[0] = interp8(w5, w[4]);
        out[1] = interp3(w5, w[4]);
        out[2] = interp3(w5, w[6]);
        out[3] = interp8(w5, w[6]);
        out[4] = interp8(w5, w[4]);
        out[5] = interp3(w5, w[4]);
        out[6] = interp3(w5, w[6]);
        out[7] = interp8(w5, w[6]);
        out[8] = interp8(w5, w[4]);
        out[9] = interp3(w5, w[4]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp6(w5, w[6], w[9]);
        out[12] = interp8(w5, w[4]);
        out[13] = interp3(w5, w[4]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 104:
    case 108:
        out[0] = interp8(w5, w[1]);
        out[1] = interp6(w5, w[2], w[1]);
        out[2] = interp6(w5, w[2], w[6]);
        out[3] = interp2(w5, w[2], w[6]);
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp7(w5, w[6], w[2]);
        out[7] = interp6(w5, w[6], w[2]);
        if (differs(w[8], w[4])) {
            out[8] = w5;
            out[12] = w5;
            out[13] = w5;
        } else {
            out[8] = interp5(w[4], w5);
            out[12] = interp5(w[8], w[4]);
            out[13] = interp5(w[8], w5);
        }
        out[9] = w5;
        out[10] = interp3(w5, w[9]);
        out[11] = interp6(w5, w[6], w[9]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 105:
    case 109:
        if (differs(w[8], w[4])) {
            out[0] = interp8(w5, w[2]);
            out[4] = interp3(w5, w[2]);
            out[8] = w5;
            out[9] = w5;
            out[12] = w5;
            out[13] = w5;
        } else {
            out[0] = interp1(w5, w[4]);
            out[4] = interp1(w[4], w5);
            out[8] = interp8(w[4], w[8]);
            out[9] = interp7(w5, w[4], w[8]);
            out[12] = interp5(w[8], w[4]);
            out[13] = interp2(w[8], w5, w[4]);
        }
        out[1] = interp8(w5, w[2]);
        out[2] = interp6(w5, w[2], w[6]);
        out[3] = interp2(w5, w[2], w[6]);
        out[5] = interp3(w5, w[2]);
        out[6] = interp7(w5, w[6], w[2]);
        out[7] = interp6(w5, w[6], w[2]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp6(w5, w[6], w[9]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 106:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp6(w5, w[6], w[3]);
        if (differs(w[8], w[4])) {
            out[8] = w5;
            out[12] = w5;
            out[13] = w5;
        } else {
            out[8] = interp5(w[4], w5);
            out[12] = interp5(w[8], w[4]);
            out[13] = interp5(w[8], w5);
        }
        out[9] = w5;
        out[10] = interp3(w5, w[9]);
        out[11] = interp6(w5, w[6], w[9]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 110:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        out[2] = interp3(w5, w[6]);
        out[3] = interp8(w5, w[6]);
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[6]);
        out[7] = interp8(w5, w[6]);
        if (differs(w[8], w[4])) {
            out[8] = w5;
            out[12] = w5;
            out[13] = w5;
        } else {
            out[8] = interp5(w[4], w5);
            out[12] = interp5(w[8], w[4]);
            out[13] = interp5(w[8], w5);
        }
        out[9] = w5;
        out[10] = interp3(w5, w[9]);
        out[11] = interp6(w5, w[6], w[9]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 111:
        if (differs(w[4], w[2])) {
            out[0] = w5;
        } else {
            out[0] = interp2(w5, w[2], w[4]);
        }
        out[1] = w5;
        out[2] = interp3(w5, w[6]);
        out[3] = interp8(w5, w[6]);
        out[4] = w5;
        out[5] = w5;
        out[6] = interp3(w5, w[6]);
        out[7] = interp8(w5, w[6]);
        if (differs(w[8], w[4])) {
            out[8] = w5;
            out[12] = w5;
            out[13] = w5;
        } else {
            out[8] = interp5(w[4], w5);
            out[12] = interp5(w[8], w[4]);
            out[13] = interp5(w[8], w5);
        }
        out[9] = w5;
        out[10] = interp3(w5, w[9]);
        out[11] = interp6(w5, w[6], w[9]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 112:
    case 113:
        out[0] = interp2(w5, w[2], w[4]);
        out[1] = interp6(w5, w[2], w[4]);
        out[2] = interp6(w5, w[2], w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp6(w5, w[4], w[2]);
        out[5] = interp7(w5, w[4], w[2]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        out[8] = interp8(w5, w[4]);
        out[9] = interp3(w5, w[4]);
        if (differs(w[6], w[8])) {
            out[10] = interp3(w5, w[9]);
            out[11] = interp1(w5, w[9]);
            out[12] = interp8(w5, w[4]);
            out[13] = interp3(w5, w[4]);
            out[14] = interp1(w5, w[9]);
            out[15] = interp8(w5, w[9]);
        } else {
            out[10] = interp7(w5, w[6], w[8]);
            out[11] = interp2(w[6], w5, w[8]);
            out[12] = interp1(w5, w[8]);
            out[13] = interp1(w[8], w5);
            out[14] = interp8(w[8], w[6]);
            out[15] = interp5(w[8], w[6]);
        }
        break;
    case 114:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            out[2] = interp1(w5, w[3]);
            out[3] = interp8(w5, w[3]);
            out[6] = interp3(w5, w[3]);
            out[7] = interp1(w5, w[3]);
        } else {
            out[2] = interp1(w5, w[2]);
            out[3] = interp2(w5, w[2], w[6]);
            out[6] = w5;
            out[7] = interp1(w5, w[6]);
        }
        out[4] = interp6(w5, w[4], w[1]);
        out[5] = interp3(w5, w[1]);
        out[8] = interp8(w5, w[4]);
        out[9] = interp3(w5, w[4]);
        if (differs(w[6], w[8])) {
            out[10] = interp3(w5, w[9]);
            out[11] = interp1(w5, w[9]);
            out[14] = interp1(w5, w[9]);
            out[15] = interp8(w5, w[9]);
        } else {
            out[10] = w5;
            out[11] = interp1(w5, w[6]);
            out[14] = interp1(w5, w[8]);
            out[15] = interp2(w5, w[8], w[6]);
        }
        out[12] = interp8(w5, w[4]);
        out[13] = interp3(w5, w[4]);
        break;
    case 115:
        out[0] = interp8(w5, w[4]);
        out[1] = interp3(w5, w[4]);
        if (differs(w[2], w[6])) {
            out[2] = interp1(w5, w[3]);
            out[3] = interp8(w5, w[3]);
            out[6] = interp3(w5, w[3]);
            out[7] = interp1(w5, w[3]);
        } else {
            out[2] = interp1(w5, w[2]);
            out[3] = interp2(w5, w[2], w[6]);
            out[6] = w5;
            out[7] = interp1(w5, w[6]);
        }
        out[4] = interp8(w5, w[4]);
        out[5] = interp3(w5, w[4]);
        out[8] = interp8(w5, w[4]);
        out[9] = interp3(w5, w[4]);
        if (differs(w[6], w[8])) {
            out[10] = interp3(w5, w[9]);
            out[11] = interp1(w5, w[9]);
            out[14] = interp1(w5, w[9]);
            out[15] = interp8(w5, w[9]);
        } else {
            out[10] = w5;
            out[11] = interp1(w5, w[6]);
            out[14] = interp1(w5, w[8]);
            out[15] = interp2(w5, w[8], w[6]);
        }
        out[12] = interp8(w5, w[4]);
        out[13] = interp3(w5, w[4]);
        break;
    case 116:
    case 117:
        out[0] = interp2(w5, w[2], w[4]);
        out[1] = interp6(w5, w[2], w[4]);
        out[2] = interp8(w5, w[2]);
        out[3] = interp8(w5, w[2]);
        out[4] = interp6(w5, w[4], w[2]);
        out[5] = interp7(w5, w[4], w[2]);
        out[6] = interp3(w5, w[2]);
        out[7] = interp3(w5, w[2]);
        out[8] = interp8(w5, w[4]);
        out[9] = interp3(w5, w[4]);
        if (differs(w[6], w[8])) {
            out[10] = interp3(w5, w[9]);
            out[11] = interp1(w5, w[9]);
            out[14] = interp1(w5, w[9]);
            out[15] = interp8(w5, w[9]);
        } else {
            out[10] = w5;
            out[11] = interp1(w5, w[6]);
            out[14] = interp1(w5, w[8]);
            out[15] = interp2(w5, w[8], w[6]);
        }
        out[12] = interp8(w5, w[4]);
        out[13] = interp3(w5, w[4]);
        break;
    case 118:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            out[2] = w5;
            out[3] = w5;
            out[7] = w5;
        } else {
            out[2] = interp5(w[2], w5);
            out[3] = interp5(w[2], w[6]);
            out[7] = interp5(w[6], w5);
        }
        out[4] = interp6(w5, w[4], w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = w5;
        out[8] = interp8(w5, w[4]);
        out[9] = interp3(w5, w[4]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[12] = interp8(w5, w[4]);
        out[13] = interp3(w5, w[4]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 119:
        if (differs(w[2], w[6])) {
            out[0] = interp8(w5, w[4]);
            out[1] = interp3(w5, w[4]);
            out[2] = w5;
            out[3] = w5;
            out[6] = w5;
            out[7] = w5;
        } else {
            out[0] = interp1(w5, w[2]);
            out[1] = interp1(w[2], w5);
            out[2] = interp8(w[2], w[6]);
            out[3] = interp5(w[2], w[6]);
            out[6] = interp7(w5, w[6], w[2]);
            out[7] = interp2(w[6], w5, w[2]);
        }
        out[4] = interp8(w5, w[4]);
        out[5] = interp3(w5, w[4]);
        out[8] = interp8(w5, w[4]);
        out[9] = interp3(w5, w[4]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[12] = interp8(w5, w[4]);
        out[13] = interp3(w5, w[4]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 120:
        out[0] = interp8(w5, w[1]);
        out[1] = interp6(w5, w[2], w[1]);
        out[2] = interp6(w5, w[2], w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        if (differs(w[8], w[4])) {
            out[8] = w5;
            out[12] = w5;
            out[13] = w5;
        } else {
            out[8] = interp5(w[4], w5);
            out[12] = interp5(w[8], w[4]);
            out[13] = interp5(w[8], w5);
        }
        out[9] = w5;
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 121:
        out[0] = interp8(w5, w[2]);
        out[1] = interp8(w5, w[2]);
        out[2] = interp6(w5, w[2], w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp3(w5, w[2]);
        out[5] = interp3(w5, w[2]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        if (differs(w[8], w[4])) {
            out[8] = w5;
            out[12] = w5;
            out[13] = w5;
        } else {
            out[8] = interp5(w[4], w5);
            out[12] = interp5(w[8], w[4]);
            out[13] = interp5(w[8], w5);
        }
        out[9] = w5;
        if (differs(w[6], w[8])) {
            out[10] = interp3(w5, w[9]);
            out[11] = interp1(w5, w[9]);
            out[14] = interp1(w5, w[9]);
            out[15] = interp8(w5, w[9]);
        } else {
            out[10] = w5;
            out[11] = interp1(w5, w[6]);
            out[14] = interp1(w5, w[8]);
            out[15] = interp2(w5, w[8], w[6]);
        }
        break;
    case 122:
        if (differs(w[4], w[2])) {
            out[0] = interp8(w5, w[1]);
            out[1] = interp1(w5, w[1]);
            out[4] = interp1(w5, w[1]);
            out[5] = interp3(w5, w[1]);
        } else {
            out[0] = interp2(w5, w[2], w[4]);
            out[1] = interp1(w5, w[2]);
            out[4] = interp1(w5, w[4]);
            out[5] = w5;
        }
        if (differs(w[2], w[6])) {
            out[2] = interp1(w5, w[3]);
            out[3] = interp8(w5, w[3]);
            out[6] = interp3(w5, w[3]);
            out[7] = interp1(w5, w[3]);
        } else {
            out[2] = interp1(w5, w[2]);
            out[3] = interp2(w5, w[2], w[6]);
            out[6] = w5;
            out[7] = interp1(w5, w[6]);
        }
        if (differs(w[8], w[4])) {
            out[8] = w5;
            out[12] = w5;
            out[13] = w5;
        } else {
            out[8] = interp5(w[4], w5);
            out[12] = interp5(w[8], w[4]);
            out[13] = interp5(w[8], w5);
        }
        out[9] = w5;
        if (differs(w[6], w[8])) {
            out[10] = interp3(w5, w[9]);
            out[11] = interp1(w5, w[9]);
            out[14] = interp1(w5, w[9]);
            out[15] = interp8(w5, w[9]);
        } else {
            out[10] = w5;
            out[11] = interp1(w5, w[6]);
            out[14] = interp1(w5, w[8]);
            out[15] = interp2(w5, w[8], w[6]);
        }
        break;
    case 123:
        if (differs(w[4], w[2])) {
            out[0] = w5;
            out[1] = w5;
            out[4] = w5;
        } else {
            out[0] = interp5(w[2], w[4]);
            out[1] = interp5(w[2], w5);
            out[4] = interp5(w[4], w5);
        }
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[5] = w5;
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        if (differs(w[8], w[4])) {
            out[8] = w5;
            out[12] = w5;
            out[13] = w5;
        } else {
            out[8] = interp5(w[4], w5);
            out[12] = interp5(w[8], w[4]);
            out[13] = interp5(w[8], w5);
        }
        out[9] = w5;
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 124:
        out[0] = interp8(w5, w[1]);
        out[1] = interp6(w5, w[2], w[1]);
        out[2] = interp8(w5, w[2]);
        out[3] = interp8(w5, w[2]);
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[2]);
        out[7] = interp3(w5, w[2]);
        if (differs(w[8], w[4])) {
            out[8] = w5;
            out[12] = w5;
            out[13] = w5;
        } else {
            out[8] = interp5(w[4], w5);
            out[12] = interp5(w[8], w[4]);
            out[13] = interp5(w[8], w5);
        }
        out[9] = w5;
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 125:
        if (differs(w[8], w[4])) {
            out[0] = interp8(w5, w[2]);
            out[4] = interp3(w5, w[2]);
            out[8] = w5;
            out[9] = w5;
            out[12] = w5;
            out[13] = w5;
        } else {
            out[0] = interp1(w5, w[4]);
            out[4] = interp1(w[4], w5);
            out[8] = interp8(w[4], w[8]);
            out[9] = interp7(w5, w[4], w[8]);
            out[12] = interp5(w[8], w[4]);
            out[13] = interp2(w[8], w5, w[4]);
        }
        out[1] = interp8(w5, w[2]);
        out[2] = interp8(w5, w[2]);
        out[3] = interp8(w5, w[2]);
        out[5] = interp3(w5, w[2]);
        out[6] = interp3(w5, w[2]);
        out[7] = interp3(w5, w[2]);
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 126:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            out[2] = w5;
            out[3] = w5;
            out[7] = w5;
        } else {
            out[2] = interp5(w[2], w5);
            out[3] = interp5(w[2], w[6]);
            out[7] = interp5(w[6], w5);
        }
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = w5;
        if (differs(w[8], w[4])) {
            out[8] = w5;
            out[12] = w5;
            out[13] = w5;
        } else {
            out[8] = interp5(w[4], w5);
            out[12] = interp5(w[8], w[4]);
            out[13] = interp5(w[8], w5);
        }
        out[9] = w5;
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 127:
        if (differs(w[4], w[2])) {
            out[0] = w5;
        } else {
            out[0] = interp2(w5, w[2], w[4]);
        }
        out[1] = w5;
        if (differs(w[2], w[6])) {
            out[2] = w5;
            out[3] = w5;
            out[7] = w5;
        } else {
            out[2] = interp5(w[2], w5);
            out[3] = interp5(w[2], w[6]);
            out[7] = interp5(w[6], w5);
        }
        out[4] = w5;
        out[5] = w5;
        out[6] = w5;
        if (differs(w[8], w[4])) {
            out[8] = w5;
            out[12] = w5;
            out[13] = w5;
        } else {
            out[8] = interp5(w[4], w5);
            out[12] = interp5(w[8], w[4]);
            out[13] = interp5(w[8], w5);
        }
        out[9] = w5;
        out[10] = interp3(w5, w[9]);
        out[11] = interp1(w5, w[9]);
        out[14] = interp1(w5, w[9]);
        out[15] = interp8(w5, w[9]);
        break;
    case 144:
    case 145:
    case 176:
    case 177:
        out[0] = interp2(w5, w[2], w[4]);
        out[1] = interp6(w5, w[2], w[4]);
        out[2] = interp6(w5, w[2], w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp6(w5, w[4], w[2]);
        out[5] = interp7(w5, w[4], w[2]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        out[8] = interp6(w5, w[4], w[8]);
        out[9] = interp7(w5, w[4], w[8]);
        out[10] = interp3(w5, w[8]);
        out[11] = interp3(w5, w[8]);
        out[12] = interp2(w5, w[8], w[4]);
        out[13] = interp6(w5, w[8], w[4]);
        out[14] = interp8(w5, w[8]);
        out[15] = interp8(w5, w[8]);
        break;
    case 146:
    case 178:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            out[2] = interp1(w5, w[3]);
            out[3] = interp8(w5, w[3]);
            out[6] = interp3(w5, w[3]);
            out[7] = interp1(w5, w[3]);
            out[11] = interp3(w5, w[8]);
            out[15] = interp8(w5, w[8]);
        } else {
            out[2] = interp2(w[2], w5, w[6]);
            out[3] = interp5(w[2], w[6]);
            out[6] = interp7(w5, w[6], w[2]);
            out[7] = interp8(w[6], w[2]);
            out[11] = interp1(w[6], w5);
            out[15] = interp1(w5, w[6]);
        }
        out[4] = interp6(w5, w[4], w[1]);
        out[5] = interp3(w5, w[1]);
        out[8] = interp6(w5, w[4], w[8]);
        out[9] = interp7(w5, w[4], w[8]);
        out[10] = interp3(w5, w[8]);
        out[12] = interp2(w5, w[8], w[4]);
        out[13] = interp6(w5, w[8], w[4]);
        out[14] = interp8(w5, w[8]);
        break;
    case 147:
    case 179:
        out[0] = interp8(w5, w[4]);
        out[1] = interp3(w5, w[4]);
        if (differs(w[2], w[6])) {
            out[2] = interp1(w5, w[3]);
            out[3] = interp8(w5, w[3]);
            out[6] = interp3(w5, w[3]);
            out[7] = interp1(w5, w[3]);
        } else {
            out[2] = interp1(w5, w[2]);
            out[3] = interp2(w5, w[2], w[6]);
            out[6] = w5;
            out[7] = interp1(w5, w[6]);
        }
        out[4] = interp8(w5, w[4]);
        out[5] = interp3(w5, w[4]);
        out[8] = interp6(w5, w[4], w[8]);
        out[9] = interp7(w5, w[4], w[8]);
        out[10] = interp3(w5, w[8]);
        out[11] = interp3(w5, w[8]);
        out[12] = interp2(w5, w[8], w[4]);
        out[13] = interp6(w5, w[8], w[4]);
        out[14] = interp8(w5, w[8]);
        out[15] = interp8(w5, w[8]);
        break;
    case 148:
    case 149:
    case 180:
    case 181:
        out[0] = interp2(w5, w[2], w[4]);
        out[1] = interp6(w5, w[2], w[4]);
        out[2] = interp8(w5, w[2]);
        out[3] = interp8(w5, w[2]);
        out[4] = interp6(w5, w[4], w[2]);
        out[5] = interp7(w5, w[4], w[2]);
        out[6] = interp3(w5, w[2]);
        out[7] = interp3(w5, w[2]);
        out[8] = interp6(w5, w[4], w[8]);
        out[9] = interp7(w5, w[4], w[8]);
        out[10] = interp3(w5, w[8]);
        out[11] = interp3(w5, w[8]);
        out[12] = interp2(w5, w[8], w[4]);
        out[13] = interp6(w5, w[8], w[4]);
        out[14] = interp8(w5, w[8]);
        out[15] = interp8(w5, w[8]);
        break;
    case 150:
    case 182:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            out[2] = w5;
            out[3] = w5;
            out[6] = w5;
            out[7] = w5;
            out[11] = interp3(w5, w[8]);
            out[15] = interp8(w5, w[8]);
        } else {
            out[2] = interp2(w[2], w5, w[6]);
            out[3] = interp5(w[2], w[6]);
            out[6] = interp7(w5, w[6], w[2]);
            out[7] = interp8(w[6], w[2]);
            out[11] = interp1(w[6], w5);
            out[15] = interp1(w5, w[6]);
        }
        out[4] = interp6(w5, w[4], w[1]);
        out[5] = interp3(w5, w[1]);
        out[8] = interp6(w5, w[4], w[8]);
        out[9] = interp7(w5, w[4], w[8]);
        out[10] = interp3(w5, w[8]);
        out[12] = interp2(w5, w[8], w[4]);
        out[13] = interp6(w5, w[8], w[4]);
        out[14] = interp8(w5, w[8]);
        break;
    case 151:
    case 183:
        out[0] = interp8(w5, w[4]);
        out[1] = interp3(w5, w[4]);
        out[2] = w5;
        if (differs(w[2], w[6])) {
            out[3] = w5;
        } else {
            out[3] = interp2(w5, w[2], w[6]);
        }
        out[4] = interp8(w5, w[4]);
        out[5] = interp3(w5, w[4]);
        out[6] = w5;
        out[7] = w5;
        out[8] = interp6(w5, w[4], w[8]);
        out[9] = interp7(w5, w[4], w[8]);
        out[10] = interp3(w5, w[8]);
        out[11] = interp3(w5, w[8]);
        out[12] = interp2(w5, w[8], w[4]);
        out[13] = interp6(w5, w[8], w[4]);
        out[14] = interp8(w5, w[8]);
        out[15] = interp8(w5, w[8]);
        break;
    case 152:
        out[0] = interp8(w5, w[1]);
        out[1] = interp6(w5, w[2], w[1]);
        out[2] = interp6(w5, w[2], w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[8]);
        out[11] = interp3(w5, w[8]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp6(w5, w[8], w[7]);
        out[14] = interp8(w5, w[8]);
        out[15] = interp8(w5, w[8]);
        break;
    case 153:
        out[0] = interp8(w5, w[2]);
        out[1] = interp8(w5, w[2]);
        out[2] = interp6(w5, w[2], w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp3(w5, w[2]);
        out[5] = interp3(w5, w[2]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[8]);
        out[11] = interp3(w5, w[8]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp6(w5, w[8], w[7]);
        out[14] = interp8(w5, w[8]);
        out[15] = interp8(w5, w[8]);
        break;
    case 154:
        if (differs(w[4], w[2])) {
            out[0] = interp8(w5, w[1]);
            out[1] = interp1(w5, w[1]);
            out[4] = interp1(w5, w[1]);
            out[5] = interp3(w5, w[1]);
        } else {
            out[0] = interp2(w5, w[2], w[4]);
            out[1] = interp1(w5, w[2]);
            out[4] = interp1(w5, w[4]);
            out[5] = w5;
        }
        if (differs(w[2], w[6])) {
            out[2] = interp1(w5, w[3]);
            out[3] = interp8(w5, w[3]);
            out[6] = interp3(w5, w[3]);
            out[7] = interp1(w5, w[3]);
        } else {
            out[2] = interp1(w5, w[2]);
            out[3] = interp2(w5, w[2], w[6]);
            out[6] = w5;
            out[7] = interp1(w5, w[6]);
        }
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[8]);
        out[11] = interp3(w5, w[8]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp6(w5, w[8], w[7]);
        out[14] = interp8(w5, w[8]);
        out[15] = interp8(w5, w[8]);
        break;
    case 155:
        if (differs(w[4], w[2])) {
            out[0] = w5;
            out[1] = w5;
            out[4] = w5;
        } else {
            out[0] = interp5(w[2], w[4]);
            out[1] = interp5(w[2], w5);
            out[4] = interp5(w[4], w5);
        }
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[5] = w5;
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[8]);
        out[11] = interp3(w5, w[8]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp6(w5, w[8], w[7]);
        out[14] = interp8(w5, w[8]);
        out[15] = interp8(w5, w[8]);
        break;
    case 156:
        out[0] = interp8(w5, w[1]);
        out[1] = interp6(w5, w[2], w[1]);
        out[2] = interp8(w5, w[2]);
        out[3] = interp8(w5, w[2]);
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[2]);
        out[7] = interp3(w5, w[2]);
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[8]);
        out[11] = interp3(w5, w[8]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp6(w5, w[8], w[7]);
        out[14] = interp8(w5, w[8]);
        out[15] = interp8(w5, w[8]);
        break;
    case 157:
        out[0] = interp8(w5, w[2]);
        out[1] = interp8(w5, w[2]);
        out[2] = interp8(w5, w[2]);
        out[3] = interp8(w5, w[2]);
        out[4] = interp3(w5, w[2]);
        out[5] = interp3(w5, w[2]);
        out[6] = interp3(w5, w[2]);
        out[7] = interp3(w5, w[2]);
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[8]);
        out[11] = interp3(w5, w[8]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp6(w5, w[8], w[7]);
        out[14] = interp8(w5, w[8]);
        out[15] = interp8(w5, w[8]);
        break;
    case 158:
        if (differs(w[4], w[2])) {
            out[0] = interp8(w5, w[1]);
            out[1] = interp1(w5, w[1]);
            out[4] = interp1(w5, w[1]);
            out[5] = interp3(w5, w[1]);
        } else {
            out[0] = interp2(w5, w[2], w[4]);
            out[1] = interp1(w5, w[2]);
            out[4] = interp1(w5, w[4]);
            out[5] = w5;
        }
        if (differs(w[2], w[6])) {
            out[2] = w5;
            out[3] = w5;
            out[7] = w5;
        } else {
            out[2] = interp5(w[2], w5);
            out[3] = interp5(w[2], w[6]);
            out[7] = interp5(w[6], w5);
        }
        out[6] = w5;
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[8]);
        out[11] = interp3(w5, w[8]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp6(w5, w[8], w[7]);
        out[14] = interp8(w5, w[8]);
        out[15] = interp8(w5, w[8]);
        break;
    case 159:
        if (differs(w[4], w[2])) {
            out[0] = w5;
            out[1] = w5;
            out[4] = w5;
        } else {
            out[0] = interp5(w[2], w[4]);
            out[1] = interp5(w[2], w5);
            out[4] = interp5(w[4], w5);
        }
        out[2] = w5;
        if (differs(w[2], w[6])) {
            out[3] = w5;
        } else {
            out[3] = interp2(w5, w[2], w[6]);
        }
        out[5] = w5;
        out[6] = w5;
        out[7] = w5;
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[8]);
        out[11] = interp3(w5, w[8]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp6(w5, w[8], w[7]);
        out[14] = interp8(w5, w[8]);
        out[15] = interp8(w5, w[8]);
        break;
    case 184:
        out[0] = interp8(w5, w[1]);
        out[1] = interp6(w5, w[2], w[1]);
        out[2] = interp6(w5, w[2], w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        out[8] = interp3(w5, w[8]);
        out[9] = interp3(w5, w[8]);
        out[10] = interp3(w5, w[8]);
        out[11] = interp3(w5, w[8]);
        out[12] = interp8(w5, w[8]);
        out[13] = interp8(w5, w[8]);
        out[14] = interp8(w5, w[8]);
        out[15] = interp8(w5, w[8]);
        break;
    case 185:
        out[0] = interp8(w5, w[2]);
        out[1] = interp8(w5, w[2]);
        out[2] = interp6(w5, w[2], w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp3(w5, w[2]);
        out[5] = interp3(w5, w[2]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        out[8] = interp3(w5, w[8]);
        out[9] = interp3(w5, w[8]);
        out[10] = interp3(w5, w[8]);
        out[11] = interp3(w5, w[8]);
        out[12] = interp8(w5, w[8]);
        out[13] = interp8(w5, w[8]);
        out[14] = interp8(w5, w[8]);
        out[15] = interp8(w5, w[8]);
        break;
    case 186:
        if (differs(w[4], w[2])) {
            out[0] = interp8(w5, w[1]);
            out[1] = interp1(w5, w[1]);
            out[4] = interp1(w5, w[1]);
            out[5] = interp3(w5, w[1]);
        } else {
            out[0] = interp2(w5, w[2], w[4]);
            out[1] = interp1(w5, w[2]);
            out[4] = interp1(w5, w[4]);
            out[5] = w5;
        }
        if (differs(w[2], w[6])) {
            out[2] = interp1(w5, w[3]);
            out[3] = interp8(w5, w[3]);
            out[6] = interp3(w5, w[3]);
            out[7] = interp1(w5, w[3]);
        } else {
            out[2] = interp1(w5, w[2]);
            out[3] = interp2(w5, w[2], w[6]);
            out[6] = w5;
            out[7] = interp1(w5, w[6]);
        }
        out[8] = interp3(w5, w[8]);
        out[9] = interp3(w5, w[8]);
        out[10] = interp3(w5, w[8]);
        out[11] = interp3(w5, w[8]);
        out[12] = interp8(w5, w[8]);
        out[13] = interp8(w5, w[8]);
        out[14] = interp8(w5, w[8]);
        out[15] = interp8(w5, w[8]);
        break;
    case 187:
        if (differs(w[4], w[2])) {
            out[0] = w5;
            out[1] = w5;
            out[4] = w5;
            out[5] = w5;
            out[8] = interp3(w5, w[8]);
            out[12] = interp8(w5, w[8]);
        } else {
            out[0] = interp5(w[2], w[4]);
            out[1] = interp2(w[2], w5, w[4]);
            out[4] = interp8(w[4], w[2]);
            out[5] = interp7(w5, w[4], w[2]);
            out[8] = interp1(w[4], w5);
            out[12] = interp1(w5, w[4]);
        }
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        out[9] = interp3(w5, w[8]);
        out[10] = interp3(w5, w[8]);
        out[11] = interp3(w5, w[8]);
        out[13] = interp8(w5, w[8]);
        out[14] = interp8(w5, w[8]);
        out[15] = interp8(w5, w[8]);
        break;
    case 188:
        out[0] = interp8(w5, w[1]);
        out[1] = interp6(w5, w[2], w[1]);
        out[2] = interp8(w5, w[2]);
        out[3] = interp8(w5, w[2]);
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[2]);
        out[7] = interp3(w5, w[2]);
        out[8] = interp3(w5, w[8]);
        out[9] = interp3(w5, w[8]);
        out[10] = interp3(w5, w[8]);
        out[11] = interp3(w5, w[8]);
        out[12] = interp8(w5, w[8]);
        out[13] = interp8(w5, w[8]);
        out[14] = interp8(w5, w[8]);
        out[15] = interp8(w5, w[8]);
        break;
    case 189:
        out[0] = interp8(w5, w[2]);
        out[1] = interp8(w5, w[2]);
        out[2] = interp8(w5, w[2]);
        out[3] = interp8(w5, w[2]);
        out[4] = interp3(w5, w[2]);
        out[5] = interp3(w5, w[2]);
        out[6] = interp3(w5, w[2]);
        out[7] = interp3(w5, w[2]);
        out[8] = interp3(w5, w[8]);
        out[9] = interp3(w5, w[8]);
        out[10] = interp3(w5, w[8]);
        out[11] = interp3(w5, w[8]);
        out[12] = interp8(w5, w[8]);
        out[13] = interp8(w5, w[8]);
        out[14] = interp8(w5, w[8]);
        out[15] = interp8(w5, w[8]);
        break;
    case 190:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            out[2] = w5;
            out[3] = w5;
            out[6] = w5;
            out[7] = w5;
            out[11] = interp3(w5, w[8]);
            out[15] = interp8(w5, w[8]);
        } else {
            out[2] = interp2(w[2], w5, w[6]);
            out[3] = interp5(w[2], w[6]);
            out[6] = interp7(w5, w[6], w[2]);
            out[7] = interp8(w[6], w[2]);
            out[11] = interp1(w[6], w5);
            out[15] = interp1(w5, w[6]);
        }
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[8] = interp3(w5, w[8]);
        out[9] = interp3(w5, w[8]);
        out[10] = interp3(w5, w[8]);
        out[12] = interp8(w5, w[8]);
        out[13] = interp8(w5, w[8]);
        out[14] = interp8(w5, w[8]);
        break;
    case 191:
        if (differs(w[4], w[2])) {
            out[0] = w5;
        } else {
            out[0] = interp2(w5, w[2], w[4]);
        }
        out[1] = w5;
        out[2] = w5;
        if (differs(w[2], w[6])) {
            out[3] = w5;
        } else {
            out[3] = interp2(w5, w[2], w[6]);
        }
        out[4] = w5;
        out[5] = w5;
        out[6] = w5;
        out[7] = w5;
        out[8] = interp3(w5, w[8]);
        out[9] = interp3(w5, w[8]);
        out[10] = interp3(w5, w[8]);
        out[11] = interp3(w5, w[8]);
        out[12] = interp8(w5, w[8]);
        out[13] = interp8(w5, w[8]);
        out[14] = interp8(w5, w[8]);
        out[15] = interp8(w5, w[8]);
        break;
    case 192:
    case 193:
    case 196:
    case 197:
        out[0] = interp2(w5, w[2], w[4]);
        out[1] = interp6(w5, w[2], w[4]);
        out[2] = interp6(w5, w[2], w[6]);
        out[3] = interp2(w5, w[2], w[6]);
        out[4] = interp6(w5, w[4], w[2]);
        out[5] = interp7(w5, w[4], w[2]);
        out[6] = interp7(w5, w[6], w[2]);
        out[7] = interp6(w5, w[6], w[2]);
        out[8] = interp6(w5, w[4], w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[6]);
        out[11] = interp8(w5, w[6]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        out[14] = interp3(w5, w[6]);
        out[15] = interp8(w5, w[6]);
        break;
    case 194:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp6(w5, w[4], w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp6(w5, w[6], w[3]);
        out[8] = interp6(w5, w[4], w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[6]);
        out[11] = interp8(w5, w[6]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        out[14] = interp3(w5, w[6]);
        out[15] = interp8(w5, w[6]);
        break;
    case 195:
        out[0] = interp8(w5, w[4]);
        out[1] = interp3(w5, w[4]);
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp8(w5, w[4]);
        out[5] = interp3(w5, w[4]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp6(w5, w[6], w[3]);
        out[8] = interp6(w5, w[4], w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[6]);
        out[11] = interp8(w5, w[6]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        out[14] = interp3(w5, w[6]);
        out[15] = interp8(w5, w[6]);
        break;
    case 198:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        out[2] = interp3(w5, w[6]);
        out[3] = interp8(w5, w[6]);
        out[4] = interp6(w5, w[4], w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[6]);
        out[7] = interp8(w5, w[6]);
        out[8] = interp6(w5, w[4], w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[6]);
        out[11] = interp8(w5, w[6]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        out[14] = interp3(w5, w[6]);
        out[15] = interp8(w5, w[6]);
        break;
    case 199:
        out[0] = interp8(w5, w[4]);
        out[1] = interp3(w5, w[4]);
        out[2] = interp3(w5, w[6]);
        out[3] = interp8(w5, w[6]);
        out[4] = interp8(w5, w[4]);
        out[5] = interp3(w5, w[4]);
        out[6] = interp3(w5, w[6]);
        out[7] = interp8(w5, w[6]);
        out[8] = interp6(w5, w[4], w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[6]);
        out[11] = interp8(w5, w[6]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        out[14] = interp3(w5, w[6]);
        out[15] = interp8(w5, w[6]);
        break;
    case 200:
    case 204:
        out[0] = interp8(w5, w[1]);
        out[1] = interp6(w5, w[2], w[1]);
        out[2] = interp6(w5, w[2], w[6]);
        out[3] = interp2(w5, w[2], w[6]);
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp7(w5, w[6], w[2]);
        out[7] = interp6(w5, w[6], w[2]);
        if (differs(w[8], w[4])) {
            out[8] = interp1(w5, w[7]);
            out[9] = interp3(w5, w[7]);
            out[12] = interp8(w5, w[7]);
            out[13] = interp1(w5, w[7]);
            out[14] = interp3(w5, w[6]);
            out[15] = interp8(w5, w[6]);
        } else {
            out[8] = interp2(w[4], w5, w[8]);
            out[9] = interp7(w5, w[4], w[8]);
            out[12] = interp5(w[8], w[4]);
            out[13] = interp8(w[8], w[4]);
            out[14] = interp1(w[8], w5);
            out[15] = interp1(w5, w[8]);
        }
        out[10] = interp3(w5, w[6]);
        out[11] = interp8(w5, w[6]);
        break;
    case 201:
    case 205:
        out[0] = interp8(w5, w[2]);
        out[1] = interp8(w5, w[2]);
        out[2] = interp6(w5, w[2], w[6]);
        out[3] = interp2(w5, w[2], w[6]);
        out[4] = interp3(w5, w[2]);
        out[5] = interp3(w5, w[2]);
        out[6] = interp7(w5, w[6], w[2]);
        out[7] = interp6(w5, w[6], w[2]);
        if (differs(w[8], w[4])) {
            out[8] = interp1(w5, w[7]);
            out[9] = interp3(w5, w[7]);
            out[12] = interp8(w5, w[7]);
            out[13] = interp1(w5, w[7]);
        } else {
            out[8] = interp1(w5, w[4]);
            out[9] = w5;
            out[12] = interp2(w5, w[8], w[4]);
            out[13] = interp1(w5, w[8]);
        }
        out[10] = interp3(w5, w[6]);
        out[11] = interp8(w5, w[6]);
        out[14] = interp3(w5, w[6]);
        out[15] = interp8(w5, w[6]);
        break;
    case 202:
        if (differs(w[4], w[2])) {
            out[0] = interp8(w5, w[1]);
            out[1] = interp1(w5, w[1]);
            out[4] = interp1(w5, w[1]);
            out[5] = interp3(w5, w[1]);
        } else {
            out[0] = interp2(w5, w[2], w[4]);
            out[1] = interp1(w5, w[2]);
            out[4] = interp1(w5, w[4]);
            out[5] = w5;
        }
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp6(w5, w[6], w[3]);
        if (differs(w[8], w[4])) {
            out[8] = interp1(w5, w[7]);
            out[9] = interp3(w5, w[7]);
            out[12] = interp8(w5, w[7]);
            out[13] = interp1(w5, w[7]);
        } else {
            out[8] = interp1(w5, w[4]);
            out[9] = w5;
            out[12] = interp2(w5, w[8], w[4]);
            out[13] = interp1(w5, w[8]);
        }
        out[10] = interp3(w5, w[6]);
        out[11] = interp8(w5, w[6]);
        out[14] = interp3(w5, w[6]);
        out[15] = interp8(w5, w[6]);
        break;
    case 203:
        if (differs(w[4], w[2])) {
            out[0] = w5;
            out[1] = w5;
            out[4] = w5;
        } else {
            out[0] = interp5(w[2], w[4]);
            out[1] = interp5(w[2], w5);
            out[4] = interp5(w[4], w5);
        }
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[5] = w5;
        out[6] = interp3(w5, w[3]);
        out[7] = interp6(w5, w[6], w[3]);
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[6]);
        out[11] = interp8(w5, w[6]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        out[14] = interp3(w5, w[6]);
        out[15] = interp8(w5, w[6]);
        break;
    case 206:
        if (differs(w[4], w[2])) {
            out[0] = interp8(w5, w[1]);
            out[1] = interp1(w5, w[1]);
            out[4] = interp1(w5, w[1]);
            out[5] = interp3(w5, w[1]);
        } else {
            out[0] = interp2(w5, w[2], w[4]);
            out[1] = interp1(w5, w[2]);
            out[4] = interp1(w5, w[4]);
            out[5] = w5;
        }
        out[2] = interp3(w5, w[6]);
        out[3] = interp8(w5, w[6]);
        out[6] = interp3(w5, w[6]);
        out[7] = interp8(w5, w[6]);
        if (differs(w[8], w[4])) {
            out[8] = interp1(w5, w[7]);
            out[9] = interp3(w5, w[7]);
            out[12] = interp8(w5, w[7]);
            out[13] = interp1(w5, w[7]);
        } else {
            out[8] = interp1(w5, w[4]);
            out[9] = w5;
            out[12] = interp2(w5, w[8], w[4]);
            out[13] = interp1(w5, w[8]);
        }
        out[10] = interp3(w5, w[6]);
        out[11] = interp8(w5, w[6]);
        out[14] = interp3(w5, w[6]);
        out[15] = interp8(w5, w[6]);
        break;
    case 207:
        if (differs(w[4], w[2])) {
            out[0] = w5;
            out[1] = w5;
            out[2] = interp3(w5, w[6]);
            out[3] = interp8(w5, w[6]);
            out[4] = w5;
            out[5] = w5;
        } else {
            out[0] = interp5(w[2], w[4]);
            out[1] = interp8(w[2], w[4]);
            out[2] = interp1(w[2], w5);
            out[3] = interp1(w5, w[2]);
            out[4] = interp2(w[4], w5, w[2]);
            out[5] = interp7(w5, w[4], w[2]);
        }
        out[6] = interp3(w5, w[6]);
        out[7] = interp8(w5, w[6]);
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = interp3(w5, w[6]);
        out[11] = interp8(w5, w[6]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        out[14] = interp3(w5, w[6]);
        out[15] = interp8(w5, w[6]);
        break;
    case 208:
    case 209:
        out[0] = interp2(w5, w[2], w[4]);
        out[1] = interp6(w5, w[2], w[4]);
        out[2] = interp6(w5, w[2], w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp6(w5, w[4], w[2]);
        out[5] = interp7(w5, w[4], w[2]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        out[8] = interp6(w5, w[4], w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = w5;
        if (differs(w[6], w[8])) {
            out[11] = w5;
            out[14] = w5;
            out[15] = w5;
        } else {
            out[11] = interp5(w[6], w5);
            out[14] = interp5(w[8], w5);
            out[15] = interp5(w[8], w[6]);
        }
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        break;
    case 210:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp6(w5, w[4], w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        out[8] = interp6(w5, w[4], w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = w5;
        if (differs(w[6], w[8])) {
            out[11] = w5;
            out[14] = w5;
            out[15] = w5;
        } else {
            out[11] = interp5(w[6], w5);
            out[14] = interp5(w[8], w5);
            out[15] = interp5(w[8], w[6]);
        }
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        break;
    case 211:
        out[0] = interp8(w5, w[4]);
        out[1] = interp3(w5, w[4]);
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp8(w5, w[4]);
        out[5] = interp3(w5, w[4]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        out[8] = interp6(w5, w[4], w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = w5;
        if (differs(w[6], w[8])) {
            out[11] = w5;
            out[14] = w5;
            out[15] = w5;
        } else {
            out[11] = interp5(w[6], w5);
            out[14] = interp5(w[8], w5);
            out[15] = interp5(w[8], w[6]);
        }
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        break;
    case 212:
    case 213:
        out[0] = interp2(w5, w[2], w[4]);
        out[1] = interp6(w5, w[2], w[4]);
        out[2] = interp8(w5, w[2]);
        if (differs(w[6], w[8])) {
            out[3] = interp8(w5, w[2]);
            out[7] = interp3(w5, w[2]);
            out[10] = w5;
            out[11] = w5;
            out[14] = w5;
            out[15] = w5;
        } else {
            out[3] = interp1(w5, w[6]);
            out[7] = interp1(w[6], w5);
            out[10] = interp7(w5, w[6], w[8]);
            out[11] = interp8(w[6], w[8]);
            out[14] = interp2(w[8], w5, w[6]);
            out[15] = interp5(w[8], w[6]);
        }
        out[4] = interp6(w5, w[4], w[2]);
        out[5] = interp7(w5, w[4], w[2]);
        out[6] = interp3(w5, w[2]);
        out[8] = interp6(w5, w[4], w[7]);
        out[9] = interp3(w5, w[7]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        break;
    case 215:
        out[0] = interp8(w5, w[4]);
        out[1] = interp3(w5, w[4]);
        out[2] = w5;
        if (differs(w[2], w[6])) {
            out[3] = w5;
        } else {
            out[3] = interp2(w5, w[2], w[6]);
        }
        out[4] = interp8(w5, w[4]);
        out[5] = interp3(w5, w[4]);
        out[6] = w5;
        out[7] = w5;
        out[8] = interp6(w5, w[4], w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = w5;
        if (differs(w[6], w[8])) {
            out[11] = w5;
            out[14] = w5;
            out[15] = w5;
        } else {
            out[11] = interp5(w[6], w5);
            out[14] = interp5(w[8], w5);
            out[15] = interp5(w[8], w[6]);
        }
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        break;
    case 216:
        out[0] = interp8(w5, w[1]);
        out[1] = interp6(w5, w[2], w[1]);
        out[2] = interp6(w5, w[2], w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = w5;
        if (differs(w[6], w[8])) {
            out[11] = w5;
            out[14] = w5;
            out[15] = w5;
        } else {
            out[11] = interp5(w[6], w5);
            out[14] = interp5(w[8], w5);
            out[15] = interp5(w[8], w[6]);
        }
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        break;
    case 217:
        out[0] = interp8(w5, w[2]);
        out[1] = interp8(w5, w[2]);
        out[2] = interp6(w5, w[2], w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp3(w5, w[2]);
        out[5] = interp3(w5, w[2]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = w5;
        if (differs(w[6], w[8])) {
            out[11] = w5;
            out[14] = w5;
            out[15] = w5;
        } else {
            out[11] = interp5(w[6], w5);
            out[14] = interp5(w[8], w5);
            out[15] = interp5(w[8], w[6]);
        }
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        break;
    case 218:
        if (differs(w[4], w[2])) {
            out[0] = interp8(w5, w[1]);
            out[1] = interp1(w5, w[1]);
            out[4] = interp1(w5, w[1]);
            out[5] = interp3(w5, w[1]);
        } else {
            out[0] = interp2(w5, w[2], w[4]);
            out[1] = interp1(w5, w[2]);
            out[4] = interp1(w5, w[4]);
            out[5] = w5;
        }
        if (differs(w[2], w[6])) {
            out[2] = interp1(w5, w[3]);
            out[3] = interp8(w5, w[3]);
            out[6] = interp3(w5, w[3]);
            out[7] = interp1(w5, w[3]);
        } else {
            out[2] = interp1(w5, w[2]);
            out[3] = interp2(w5, w[2], w[6]);
            out[6] = w5;
            out[7] = interp1(w5, w[6]);
        }
        if (differs(w[8], w[4])) {
            out[8] = interp1(w5, w[7]);
            out[9] = interp3(w5, w[7]);
            out[12] = interp8(w5, w[7]);
            out[13] = interp1(w5, w[7]);
        } else {
            out[8] = interp1(w5, w[4]);
            out[9] = w5;
            out[12] = interp2(w5, w[8], w[4]);
            out[13] = interp1(w5, w[8]);
        }
        out[10] = w5;
        if (differs(w[6], w[8])) {
            out[11] = w5;
            out[14] = w5;
            out[15] = w5;
        } else {
            out[11] = interp5(w[6], w5);
            out[14] = interp5(w[8], w5);
            out[15] = interp5(w[8], w[6]);
        }
        break;
    case 219:
        if (differs(w[4], w[2])) {
            out[0] = w5;
            out[1] = w5;
            out[4] = w5;
        } else {
            out[0] = interp5(w[2], w[4]);
            out[1] = interp5(w[2], w5);
            out[4] = interp5(w[4], w5);
        }
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[5] = w5;
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = w5;
        if (differs(w[6], w[8])) {
            out[11] = w5;
            out[14] = w5;
            out[15] = w5;
        } else {
            out[11] = interp5(w[6], w5);
            out[14] = interp5(w[8], w5);
            out[15] = interp5(w[8], w[6]);
        }
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        break;
    case 220:
        out[0] = interp8(w5, w[1]);
        out[1] = interp6(w5, w[2], w[1]);
        out[2] = interp8(w5, w[2]);
        out[3] = interp8(w5, w[2]);
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[2]);
        out[7] = interp3(w5, w[2]);
        if (differs(w[8], w[4])) {
            out[8] = interp1(w5, w[7]);
            out[9] = interp3(w5, w[7]);
            out[12] = interp8(w5, w[7]);
            out[13] = interp1(w5, w[7]);
        } else {
            out[8] = interp1(w5, w[4]);
            out[9] = w5;
            out[12] = interp2(w5, w[8], w[4]);
            out[13] = interp1(w5, w[8]);
        }
        out[10] = w5;
        if (differs(w[6], w[8])) {
            out[11] = w5;
            out[14] = w5;
            out[15] = w5;
        } else {
            out[11] = interp5(w[6], w5);
            out[14] = interp5(w[8], w5);
            out[15] = interp5(w[8], w[6]);
        }
        break;
    case 221:
        out[0] = interp8(w5, w[2]);
        out[1] = interp8(w5, w[2]);
        out[2] = interp8(w5, w[2]);
        if (differs(w[6], w[8])) {
            out[3] = interp8(w5, w[2]);
            out[7] = interp3(w5, w[2]);
            out[10] = w5;
            out[11] = w5;
            out[14] = w5;
            out[15] = w5;
        } else {
            out[3] = interp1(w5, w[6]);
            out[7] = interp1(w[6], w5);
            out[10] = interp7(w5, w[6], w[8]);
            out[11] = interp8(w[6], w[8]);
            out[14] = interp2(w[8], w5, w[6]);
            out[15] = interp5(w[8], w[6]);
        }
        out[4] = interp3(w5, w[2]);
        out[5] = interp3(w5, w[2]);
        out[6] = interp3(w5, w[2]);
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        break;
    case 222:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            out[2] = w5;
            out[3] = w5;
            out[7] = w5;
        } else {
            out[2] = interp5(w[2], w5);
            out[3] = interp5(w[2], w[6]);
            out[7] = interp5(w[6], w5);
        }
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = w5;
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = w5;
        if (differs(w[6], w[8])) {
            out[11] = w5;
            out[14] = w5;
            out[15] = w5;
        } else {
            out[11] = interp5(w[6], w5);
            out[14] = interp5(w[8], w5);
            out[15] = interp5(w[8], w[6]);
        }
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        break;
    case 223:
        if (differs(w[4], w[2])) {
            out[0] = w5;
            out[1] = w5;
            out[4] = w5;
        } else {
            out[0] = interp5(w[2], w[4]);
            out[1] = interp5(w[2], w5);
            out[4] = interp5(w[4], w5);
        }
        out[2] = w5;
        if (differs(w[2], w[6])) {
            out[3] = w5;
        } else {
            out[3] = interp2(w5, w[2], w[6]);
        }
        out[5] = w5;
        out[6] = w5;
        out[7] = w5;
        out[8] = interp1(w5, w[7]);
        out[9] = interp3(w5, w[7]);
        out[10] = w5;
        if (differs(w[6], w[8])) {
            out[11] = w5;
            out[14] = w5;
            out[15] = w5;
        } else {
            out[11] = interp5(w[6], w5);
            out[14] = interp5(w[8], w5);
            out[15] = interp5(w[8], w[6]);
        }
        out[12] = interp8(w5, w[7]);
        out[13] = interp1(w5, w[7]);
        break;
    case 224:
    case 225:
    case 228:
    case 229:
        out[0] = interp2(w5, w[2], w[4]);
        out[1] = interp6(w5, w[2], w[4]);
        out[2] = interp6(w5, w[2], w[6]);
        out[3] = interp2(w5, w[2], w[6]);
        out[4] = interp6(w5, w[4], w[2]);
        out[5] = interp7(w5, w[4], w[2]);
        out[6] = interp7(w5, w[6], w[2]);
        out[7] = interp6(w5, w[6], w[2]);
        out[8] = interp8(w5, w[4]);
        out[9] = interp3(w5, w[4]);
        out[10] = interp3(w5, w[6]);
        out[11] = interp8(w5, w[6]);
        out[12] = interp8(w5, w[4]);
        out[13] = interp3(w5, w[4]);
        out[14] = interp3(w5, w[6]);
        out[15] = interp8(w5, w[6]);
        break;
    case 226:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp6(w5, w[4], w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp6(w5, w[6], w[3]);
        out[8] = interp8(w5, w[4]);
        out[9] = interp3(w5, w[4]);
        out[10] = interp3(w5, w[6]);
        out[11] = interp8(w5, w[6]);
        out[12] = interp8(w5, w[4]);
        out[13] = interp3(w5, w[4]);
        out[14] = interp3(w5, w[6]);
        out[15] = interp8(w5, w[6]);
        break;
    case 227:
        out[0] = interp8(w5, w[4]);
        out[1] = interp3(w5, w[4]);
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp8(w5, w[4]);
        out[5] = interp3(w5, w[4]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp6(w5, w[6], w[3]);
        out[8] = interp8(w5, w[4]);
        out[9] = interp3(w5, w[4]);
        out[10] = interp3(w5, w[6]);
        out[11] = interp8(w5, w[6]);
        out[12] = interp8(w5, w[4]);
        out[13] = interp3(w5, w[4]);
        out[14] = interp3(w5, w[6]);
        out[15] = interp8(w5, w[6]);
        break;
    case 230:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        out[2] = interp3(w5, w[6]);
        out[3] = interp8(w5, w[6]);
        out[4] = interp6(w5, w[4], w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[6]);
        out[7] = interp8(w5, w[6]);
        out[8] = interp8(w5, w[4]);
        out[9] = interp3(w5, w[4]);
        out[10] = interp3(w5, w[6]);
        out[11] = interp8(w5, w[6]);
        out[12] = interp8(w5, w[4]);
        out[13] = interp3(w5, w[4]);
        out[14] = interp3(w5, w[6]);
        out[15] = interp8(w5, w[6]);
        break;
    case 231:
        out[0] = interp8(w5, w[4]);
        out[1] = interp3(w5, w[4]);
        out[2] = interp3(w5, w[6]);
        out[3] = interp8(w5, w[6]);
        out[4] = interp8(w5, w[4]);
        out[5] = interp3(w5, w[4]);
        out[6] = interp3(w5, w[6]);
        out[7] = interp8(w5, w[6]);
        out[8] = interp8(w5, w[4]);
        out[9] = interp3(w5, w[4]);
        out[10] = interp3(w5, w[6]);
        out[11] = interp8(w5, w[6]);
        out[12] = interp8(w5, w[4]);
        out[13] = interp3(w5, w[4]);
        out[14] = interp3(w5, w[6]);
        out[15] = interp8(w5, w[6]);
        break;
    case 232:
    case 236:
        out[0] = interp8(w5, w[1]);
        out[1] = interp6(w5, w[2], w[1]);
        out[2] = interp6(w5, w[2], w[6]);
        out[3] = interp2(w5, w[2], w[6]);
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp7(w5, w[6], w[2]);
        out[7] = interp6(w5, w[6], w[2]);
        if (differs(w[8], w[4])) {
            out[8] = w5;
            out[9] = w5;
            out[12] = w5;
            out[13] = w5;
            out[14] = interp3(w5, w[6]);
            out[15] = interp8(w5, w[6]);
        } else {
            out[8] = interp2(w[4], w5, w[8]);
            out[9] = interp7(w5, w[4], w[8]);
            out[12] = interp5(w[8], w[4]);
            out[13] = interp8(w[8], w[4]);
            out[14] = interp1(w[8], w5);
            out[15] = interp1(w5, w[8]);
        }
        out[10] = interp3(w5, w[6]);
        out[11] = interp8(w5, w[6]);
        break;
    case 233:
    case 237:
        out[0] = interp8(w5, w[2]);
        out[1] = interp8(w5, w[2]);
        out[2] = interp6(w5, w[2], w[6]);
        out[3] = interp2(w5, w[2], w[6]);
        out[4] = interp3(w5, w[2]);
        out[5] = interp3(w5, w[2]);
        out[6] = interp7(w5, w[6], w[2]);
        out[7] = interp6(w5, w[6], w[2]);
        out[8] = w5;
        out[9] = w5;
        out[10] = interp3(w5, w[6]);
        out[11] = interp8(w5, w[6]);
        if (differs(w[8], w[4])) {
            out[12] = w5;
        } else {
            out[12] = interp2(w5, w[8], w[4]);
        }
        out[13] = w5;
        out[14] = interp3(w5, w[6]);
        out[15] = interp8(w5, w[6]);
        break;
    case 234:
        if (differs(w[4], w[2])) {
            out[0] = interp8(w5, w[1]);
            out[1] = interp1(w5, w[1]);
            out[4] = interp1(w5, w[1]);
            out[5] = interp3(w5, w[1]);
        } else {
            out[0] = interp2(w5, w[2], w[4]);
            out[1] = interp1(w5, w[2]);
            out[4] = interp1(w5, w[4]);
            out[5] = w5;
        }
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp6(w5, w[6], w[3]);
        if (differs(w[8], w[4])) {
            out[8] = w5;
            out[12] = w5;
            out[13] = w5;
        } else {
            out[8] = interp5(w[4], w5);
            out[12] = interp5(w[8], w[4]);
            out[13] = interp5(w[8], w5);
        }
        out[9] = w5;
        out[10] = interp3(w5, w[6]);
        out[11] = interp8(w5, w[6]);
        out[14] = interp3(w5, w[6]);
        out[15] = interp8(w5, w[6]);
        break;
    case 235:
        if (differs(w[4], w[2])) {
            out[0] = w5;
            out[1] = w5;
            out[4] = w5;
        } else {
            out[0] = interp5(w[2], w[4]);
            out[1] = interp5(w[2], w5);
            out[4] = interp5(w[4], w5);
        }
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[5] = w5;
        out[6] = interp3(w5, w[3]);
        out[7] = interp6(w5, w[6], w[3]);
        out[8] = w5;
        out[9] = w5;
        out[10] = interp3(w5, w[6]);
        out[11] = interp8(w5, w[6]);
        if (differs(w[8], w[4])) {
            out[12] = w5;
        } else {
            out[12] = interp2(w5, w[8], w[4]);
        }
        out[13] = w5;
        out[14] = interp3(w5, w[6]);
        out[15] = interp8(w5, w[6]);
        break;
    case 238:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        out[2] = interp3(w5, w[6]);
        out[3] = interp8(w5, w[6]);
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[6]);
        out[7] = interp8(w5, w[6]);
        if (differs(w[8], w[4])) {
            out[8] = w5;
            out[9] = w5;
            out[12] = w5;
            out[13] = w5;
            out[14] = interp3(w5, w[6]);
            out[15] = interp8(w5, w[6]);
        } else {
            out[8] = interp2(w[4], w5, w[8]);
            out[9] = interp7(w5, w[4], w[8]);
            out[12] = interp5(w[8], w[4]);
            out[13] = interp8(w[8], w[4]);
            out[14] = interp1(w[8], w5);
            out[15] = interp1(w5, w[8]);
        }
        out[10] = interp3(w5, w[6]);
        out[11] = interp8(w5, w[6]);
        break;
    case 239:
        if (differs(w[4], w[2])) {
            out[0] = w5;
        } else {
            out[0] = interp2(w5, w[2], w[4]);
        }
        out[1] = w5;
        out[2] = interp3(w5, w[6]);
        out[3] = interp8(w5, w[6]);
        out[4] = w5;
        out[5] = w5;
        out[6] = interp3(w5, w[6]);
        out[7] = interp8(w5, w[6]);
        out[8] = w5;
        out[9] = w5;
        out[10] = interp3(w5, w[6]);
        out[11] = interp8(w5, w[6]);
        if (differs(w[8], w[4])) {
            out[12] = w5;
        } else {
            out[12] = interp2(w5, w[8], w[4]);
        }
        out[13] = w5;
        out[14] = interp3(w5, w[6]);
        out[15] = interp8(w5, w[6]);
        break;
    case 240:
    case 241:
        out[0] = interp2(w5, w[2], w[4]);
        out[1] = interp6(w5, w[2], w[4]);
        out[2] = interp6(w5, w[2], w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp6(w5, w[4], w[2]);
        out[5] = interp7(w5, w[4], w[2]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        out[8] = interp8(w5, w[4]);
        out[9] = interp3(w5, w[4]);
        if (differs(w[6], w[8])) {
            out[10] = w5;
            out[11] = w5;
            out[12] = interp8(w5, w[4]);
            out[13] = interp3(w5, w[4]);
            out[14] = w5;
            out[15] = w5;
        } else {
            out[10] = interp7(w5, w[6], w[8]);
            out[11] = interp2(w[6], w5, w[8]);
            out[12] = interp1(w5, w[8]);
            out[13] = interp1(w[8], w5);
            out[14] = interp8(w[8], w[6]);
            out[15] = interp5(w[8], w[6]);
        }
        break;
    case 242:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            out[2] = interp1(w5, w[3]);
            out[3] = interp8(w5, w[3]);
            out[6] = interp3(w5, w[3]);
            out[7] = interp1(w5, w[3]);
        } else {
            out[2] = interp1(w5, w[2]);
            out[3] = interp2(w5, w[2], w[6]);
            out[6] = w5;
            out[7] = interp1(w5, w[6]);
        }
        out[4] = interp6(w5, w[4], w[1]);
        out[5] = interp3(w5, w[1]);
        out[8] = interp8(w5, w[4]);
        out[9] = interp3(w5, w[4]);
        out[10] = w5;
        if (differs(w[6], w[8])) {
            out[11] = w5;
            out[14] = w5;
            out[15] = w5;
        } else {
            out[11] = interp5(w[6], w5);
            out[14] = interp5(w[8], w5);
            out[15] = interp5(w[8], w[6]);
        }
        out[12] = interp8(w5, w[4]);
        out[13] = interp3(w5, w[4]);
        break;
    case 243:
        out[0] = interp8(w5, w[4]);
        out[1] = interp3(w5, w[4]);
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp8(w5, w[4]);
        out[5] = interp3(w5, w[4]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        out[8] = interp8(w5, w[4]);
        out[9] = interp3(w5, w[4]);
        if (differs(w[6], w[8])) {
            out[10] = w5;
            out[11] = w5;
            out[12] = interp8(w5, w[4]);
            out[13] = interp3(w5, w[4]);
            out[14] = w5;
            out[15] = w5;
        } else {
            out[10] = interp7(w5, w[6], w[8]);
            out[11] = interp2(w[6], w5, w[8]);
            out[12] = interp1(w5, w[8]);
            out[13] = interp1(w[8], w5);
            out[14] = interp8(w[8], w[6]);
            out[15] = interp5(w[8], w[6]);
        }
        break;
    case 244:
    case 245:
        out[0] = interp2(w5, w[2], w[4]);
        out[1] = interp6(w5, w[2], w[4]);
        out[2] = interp8(w5, w[2]);
        out[3] = interp8(w5, w[2]);
        out[4] = interp6(w5, w[4], w[2]);
        out[5] = interp7(w5, w[4], w[2]);
        out[6] = interp3(w5, w[2]);
        out[7] = interp3(w5, w[2]);
        out[8] = interp8(w5, w[4]);
        out[9] = interp3(w5, w[4]);
        out[10] = w5;
        out[11] = w5;
        out[12] = interp8(w5, w[4]);
        out[13] = interp3(w5, w[4]);
        out[14] = w5;
        if (differs(w[6], w[8])) {
            out[15] = w5;
        } else {
            out[15] = interp2(w5, w[8], w[6]);
        }
        break;
    case 246:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            out[2] = w5;
            out[3] = w5;
            out[7] = w5;
        } else {
            out[2] = interp5(w[2], w5);
            out[3] = interp5(w[2], w[6]);
            out[7] = interp5(w[6], w5);
        }
        out[4] = interp6(w5, w[4], w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = w5;
        out[8] = interp8(w5, w[4]);
        out[9] = interp3(w5, w[4]);
        out[10] = w5;
        out[11] = w5;
        out[12] = interp8(w5, w[4]);
        out[13] = interp3(w5, w[4]);
        out[14] = w5;
        if (differs(w[6], w[8])) {
            out[15] = w5;
        } else {
            out[15] = interp2(w5, w[8], w[6]);
        }
        break;
    case 247:
        out[0] = interp8(w5, w[4]);
        out[1] = interp3(w5, w[4]);
        out[2] = w5;
        if (differs(w[2], w[6])) {
            out[3] = w5;
        } else {
            out[3] = interp2(w5, w[2], w[6]);
        }
        out[4] = interp8(w5, w[4]);
        out[5] = interp3(w5, w[4]);
        out[6] = w5;
        out[7] = w5;
        out[8] = interp8(w5, w[4]);
        out[9] = interp3(w5, w[4]);
        out[10] = w5;
        out[11] = w5;
        out[12] = interp8(w5, w[4]);
        out[13] = interp3(w5, w[4]);
        out[14] = w5;
        if (differs(w[6], w[8])) {
            out[15] = w5;
        } else {
            out[15] = interp2(w5, w[8], w[6]);
        }
        break;
    case 249:
        out[0] = interp8(w5, w[2]);
        out[1] = interp8(w5, w[2]);
        out[2] = interp6(w5, w[2], w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp3(w5, w[2]);
        out[5] = interp3(w5, w[2]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        out[8] = w5;
        out[9] = w5;
        out[10] = w5;
        if (differs(w[6], w[8])) {
            out[11] = w5;
            out[14] = w5;
            out[15] = w5;
        } else {
            out[11] = interp5(w[6], w5);
            out[14] = interp5(w[8], w5);
            out[15] = interp5(w[8], w[6]);
        }
        if (differs(w[8], w[4])) {
            out[12] = w5;
        } else {
            out[12] = interp2(w5, w[8], w[4]);
        }
        out[13] = w5;
        break;
    case 250:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        if (differs(w[8], w[4])) {
            out[8] = w5;
            out[12] = w5;
            out[13] = w5;
        } else {
            out[8] = interp5(w[4], w5);
            out[12] = interp5(w[8], w[4]);
            out[13] = interp5(w[8], w5);
        }
        out[9] = w5;
        out[10] = w5;
        if (differs(w[6], w[8])) {
            out[11] = w5;
            out[14] = w5;
            out[15] = w5;
        } else {
            out[11] = interp5(w[6], w5);
            out[14] = interp5(w[8], w5);
            out[15] = interp5(w[8], w[6]);
        }
        break;
    case 251:
        if (differs(w[4], w[2])) {
            out[0] = w5;
            out[1] = w5;
            out[4] = w5;
        } else {
            out[0] = interp5(w[2], w[4]);
            out[1] = interp5(w[2], w5);
            out[4] = interp5(w[4], w5);
        }
        out[2] = interp1(w5, w[3]);
        out[3] = interp8(w5, w[3]);
        out[5] = w5;
        out[6] = interp3(w5, w[3]);
        out[7] = interp1(w5, w[3]);
        out[8] = w5;
        out[9] = w5;
        out[10] = w5;
        if (differs(w[6], w[8])) {
            out[11] = w5;
            out[14] = w5;
            out[15] = w5;
        } else {
            out[11] = interp5(w[6], w5);
            out[14] = interp5(w[8], w5);
            out[15] = interp5(w[8], w[6]);
        }
        if (differs(w[8], w[4])) {
            out[12] = w5;
        } else {
            out[12] = interp2(w5, w[8], w[4]);
        }
        out[13] = w5;
        break;
    case 252:
        out[0] = interp8(w5, w[1]);
        out[1] = interp6(w5, w[2], w[1]);
        out[2] = interp8(w5, w[2]);
        out[3] = interp8(w5, w[2]);
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = interp3(w5, w[2]);
        out[7] = interp3(w5, w[2]);
        if (differs(w[8], w[4])) {
            out[8] = w5;
            out[12] = w5;
            out[13] = w5;
        } else {
            out[8] = interp5(w[4], w5);
            out[12] = interp5(w[8], w[4]);
            out[13] = interp5(w[8], w5);
        }
        out[9] = w5;
        out[10] = w5;
        out[11] = w5;
        out[14] = w5;
        if (differs(w[6], w[8])) {
            out[15] = w5;
        } else {
            out[15] = interp2(w5, w[8], w[6]);
        }
        break;
    case 253:
        out[0] = interp8(w5, w[2]);
        out[1] = interp8(w5, w[2]);
        out[2] = interp8(w5, w[2]);
        out[3] = interp8(w5, w[2]);
        out[4] = interp3(w5, w[2]);
        out[5] = interp3(w5, w[2]);
        out[6] = interp3(w5, w[2]);
        out[7] = interp3(w5, w[2]);
        out[8] = w5;
        out[9] = w5;
        out[10] = w5;
        out[11] = w5;
        if (differs(w[8], w[4])) {
            out[12] = w5;
        } else {
            out[12] = interp2(w5, w[8], w[4]);
        }
        out[13] = w5;
        out[14] = w5;
        if (differs(w[6], w[8])) {
            out[15] = w5;
        } else {
            out[15] = interp2(w5, w[8], w[6]);
        }
        break;
    case 254:
        out[0] = interp8(w5, w[1]);
        out[1] = interp1(w5, w[1]);
        if (differs(w[2], w[6])) {
            out[2] = w5;
            out[3] = w5;
            out[7] = w5;
        } else {
            out[2] = interp5(w[2], w5);
            out[3] = interp5(w[2], w[6]);
            out[7] = interp5(w[6], w5);
        }
        out[4] = interp1(w5, w[1]);
        out[5] = interp3(w5, w[1]);
        out[6] = w5;
        if (differs(w[8], w[4])) {
            out[8] = w5;
            out[12] = w5;
            out[13] = w5;
        } else {
            out[8] = interp5(w[4], w5);
            out[12] = interp5(w[8], w[4]);
            out[13] = interp5(w[8], w5);
        }
        out[9] = w5;
        out[10] = w5;
        out[11] = w5;
        out[14] = w5;
        if (differs(w[6], w[8])) {
            out[15] = w5;
        } else {
            out[15] = interp2(w5, w[8], w[6]);
        }
        break;
    case 255:
        if (differs(w[4], w[2])) {
            out[0] = w5;
        } else {
            out[0] = interp2(w5, w[2], w[4]);
        }
        out[1] = w5;
        out[2] = w5;
        if (differs(w[2], w[6])) {
            out[3] = w5;
        } else {
            out[3] = interp2(w5, w[2], w[6]);
        }
        out[4] = w5;
        out[5] = w5;
        out[6] = w5;
        out[7] = w5;
        out[8] = w5;
        out[9] = w5;
        out[10] = w5;
        out[11] = w5;
        if (differs(w[8], w[4])) {
            out[12] = w5;
        } else {
            out[12] = interp2(w5, w[8], w[4]);
        }
        out[13] = w5;
        out[14] = w5;
        if (differs(w[6], w[8])) {
            out[15] = w5;
        } else {
            out[15] = interp2(w5, w[8], w[6]);
        }
        break;
    default:
        for (unsigned i = 0; i < 16; i++)
            out[i] = w5;
        break;
    }
}

static void hq4x_pixel(u2 const w[10], u1* const dst, u4 const pitch)
{
    u2 q[16];

    hq4x_sixteen(w, q);
    for (unsigned r = 0; r < 4; r++) {
        u1* const row = dst + (size_t)pitch * r;

        for (unsigned c = 0; c < 4; c++)
            st16u(row + c * 2, q[r * 4 + c]);
    }
}

static void hq4x_pixel32(u2 const w[10], u1* const dst, u4 const pitch)
{
    u4 const* const conv = (u4 const*)BitConv32Ptr;
    u2 q[16];

    hq4x_sixteen(w, q);
    for (unsigned r = 0; r < 4; r++) {
        u1* const row = dst + (size_t)pitch * r;

        for (unsigned c = 0; c < 4; c++)
            st32u(row + c * 4, conv[q[r * 4 + c]]);
    }
}

static void hq4x_double_16b(void)
{
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

static void hq4x_double_32b(void)
{
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

void hq4x_16b(void)
{
    if (curblank == 0x40)
        return;

    if (!hqFilter || (!FilteredGUI && GUIOn2 == 1)) {
        hq4x_double_16b();
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
            /* Already 512 wide: replicated, not filtered, which is what the
               assembly's HighResProc did and what hq2x/hq3x do here. */
            for (u4 x = 0; x < 256; x++) {
                u2 const px = row[x];

                for (unsigned ry = 0; ry < 4; ry++) {
                    u1* const orow = dst + x * 8 + (size_t)pitch * ry;

                    for (unsigned rx = 0; rx < 4; rx++)
                        st16u(orow + rx * 2, px);
                }
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
                if (flat9(w))
                    fill_flat16(dst + x * 8, pitch, 4, w[5]);
                else
                    hq4x_pixel(w, dst + x * 8, pitch);
            }
        }
        dst += 256 * 8 + AddEndBytes + pitch * 3;
    }
}

void hq4x_32b(void)
{
    if (curblank == 0x40)
        return;

    if (!hqFilter || (!FilteredGUI && GUIOn2 == 1)) {
        hq4x_double_32b();
        return;
    }

    u4 const* const conv = (u4 const*)BitConv32Ptr;
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
            /* Already 512 wide: replicated, not filtered, which is what the
               assembly's HighResProc did and what hq2x/hq3x do here. */
            for (u4 x = 0; x < 256; x++) {
                u4 const px = conv[row[x]];

                for (unsigned ry = 0; ry < 4; ry++) {
                    u1* const orow = dst + x * 16 + (size_t)pitch * ry;

                    for (unsigned rx = 0; rx < 4; rx++)
                        st32u(orow + rx * 4, px);
                }
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
                if (flat9(w))
                    fill_flat32(dst + x * 16, pitch, 4, w[5]);
                else
                    hq4x_pixel32(w, dst + x * 16, pitch);
            }
        }
        dst += 256 * 16 + AddEndBytes + pitch * 3;
    }
}
