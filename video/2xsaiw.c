/*
 * Kreed's 2xSaI, Super2xSaI and SuperEagle, ported from video/2xsaiw.asm
 * (the MMX version, replaced by a scale2x stub in 5ff6d63d). One source line
 * in, two output lines out.
 *
 * The assembly's delta buffer, which skipped a group of four pixels whose
 * neighbourhood had not changed since the last frame, is not reproduced; it
 * computed the same pixels this does. The parameter stays for the ABI.
 *
 * Neighbourhood, with the current pixel at color5 / colorA:
 *
 *      colorB0 colorB1 colorB2 colorB3          I E F J
 *      color4  color5  color6  colorS2          G A B K
 *      color1  color2  color3  colorS1          H C D L
 *      colorA0 colorA1 colorA2 colorA3          M N O P
 */

#include <stdint.h>
#include <string.h>

#include "../types.h"
#include "../unaligned.h"

// Pixel masks written by Init_2xSaI(); defaults match the 565 format.
u4 colorMask[2] = { 0xF7DEF7DE, 0xF7DEF7DE };
u4 lowPixelMask[2] = { 0x08210821, 0x08210821 };
u4 qcolorMask[2] = { 0xE79CE79C, 0xE79CE79C };
u4 qlowpixelMask[2] = { 0x18631863, 0x18631863 };

static u2 interpolate(u2 const a, u2 const b)
{
    return (u2)(((a & colorMask[0]) >> 1) + ((b & colorMask[0]) >> 1)
        + (a & b & lowPixelMask[0]));
}

static u2 q_interpolate(u2 const a, u2 const b, u2 const c, u2 const d)
{
    u4 const x = ((a & qcolorMask[0]) >> 2) + ((b & qcolorMask[0]) >> 2)
        + ((c & qcolorMask[0]) >> 2) + ((d & qcolorMask[0]) >> 2);
    u4 const y = (((a & qlowpixelMask[0]) + (b & qlowpixelMask[0])
                      + (c & qlowpixelMask[0]) + (d & qlowpixelMask[0]))
                     >> 2)
        & qlowpixelMask[0];

    return (u2)(x + y);
}

static int get_result(u2 const a, u2 const b, u2 const c, u2 const d)
{
    return (a != c || a != d) - (b != c || b != d);
}

/* Row n of the neighbourhood, relative to the row above the current one. */
static u2 px(u1 const* const base, u4 const pitch, u4 const row, int const col)
{
    return ld16u(base + row * (size_t)pitch + col * 2);
}

void _2xSaISuper2xSaILine(
    u2* src, u1* delta, u4 srcPitch, u4 width, u1* dst, u4 dstPitch)
{
    u1 const* const base = (u1 const*)src - srcPitch;

    (void)delta;
    for (u4 i = 0; i < width; i++) {
        u2 const colorB0 = px(base, srcPitch, 0, (int)i - 1);
        u2 const colorB1 = px(base, srcPitch, 0, (int)i);
        u2 const colorB2 = px(base, srcPitch, 0, (int)i + 1);
        u2 const colorB3 = px(base, srcPitch, 0, (int)i + 2);
        u2 const color4 = px(base, srcPitch, 1, (int)i - 1);
        u2 const color5 = px(base, srcPitch, 1, (int)i);
        u2 const color6 = px(base, srcPitch, 1, (int)i + 1);
        u2 const colorS2 = px(base, srcPitch, 1, (int)i + 2);
        u2 const color1 = px(base, srcPitch, 2, (int)i - 1);
        u2 const color2 = px(base, srcPitch, 2, (int)i);
        u2 const color3 = px(base, srcPitch, 2, (int)i + 1);
        u2 const colorS1 = px(base, srcPitch, 2, (int)i + 2);
        u2 const colorA0 = px(base, srcPitch, 3, (int)i - 1);
        u2 const colorA1 = px(base, srcPitch, 3, (int)i);
        u2 const colorA2 = px(base, srcPitch, 3, (int)i + 1);
        u2 const colorA3 = px(base, srcPitch, 3, (int)i + 2);
        u2 product1a, product1b, product2a, product2b;
        int m35, m26;

        /* Which of the two colours the right-hand pair leans to. The vote
           only runs when both diagonals match and the two colours differ. */
        m35 = (color5 == color3 && color6 != color2)
            || (color5 == color3 && color6 == color2 && color5 == color6);
        m26 = (color5 != color3 && color6 == color2);
        if (color5 == color3 && color6 == color2 && color5 != color6) {
            int r = 0;

            r += get_result(color5, color6, color1, colorA1);
            r += get_result(color5, color6, color4, colorB1);
            r += get_result(color5, color6, colorA2, colorS1);
            r += get_result(color5, color6, colorB2, colorS2);
            if (r > 0) {
                m35 = 1;
            } else if (r < 0) {
                m26 = 1;
            }
        }

        /* The three-to-one blends outrank the lean, which is why they are
           tested first; the assembly clears their bits out of both masks. */
        if (color3 == color6 && colorB1 == color6 && color5 != colorB2
            && colorB0 != color6) {
            product1b = q_interpolate(color6, color6, color6, color5);
        } else if (color5 == color2 && color5 == colorB2 && colorB1 != color6
            && color5 != colorB3) {
            product1b = q_interpolate(color6, color5, color5, color5);
        } else if (m35) {
            product1b = color5;
        } else if (m26) {
            product1b = color6;
        } else {
            product1b = interpolate(color5, color6);
        }

        if (color6 == color3 && color3 == colorA1 && color2 != colorA2
            && color3 != colorA0) {
            product2b = q_interpolate(color3, color3, color3, color2);
        } else if (color5 == color2 && color2 == colorA2 && colorA1 != color3
            && color2 != colorA3) {
            product2b = q_interpolate(color2, color2, color2, color3);
        } else if (m35) {
            product2b = color3;
        } else if (m26) {
            product2b = color2;
        } else {
            product2b = interpolate(color2, color3);
        }

        /* The first test of each pair is the lean itself, vote included -
           the assembly reads the mask here, not the plain comparison its
           comment shows. */
        if (m35 && color4 == color5 && color5 != colorA2) {
            product2a = interpolate(color2, color5);
        } else if (color5 == color1 && color6 == color5 && color4 != color2
            && color5 != colorA0) {
            product2a = interpolate(color2, color5);
        } else {
            product2a = color2;
        }

        if (m26 && color1 == color2 && colorB2 != color2) {
            product1a = interpolate(color2, color5);
        } else if (color3 == color2 && color4 == color2 && color5 != color1
            && colorB0 != color2) {
            product1a = interpolate(color2, color5);
        } else {
            product1a = color5;
        }

        st16u(dst + i * 4, product1a);
        st16u(dst + i * 4 + 2, product1b);
        st16u(dst + dstPitch + i * 4, product2a);
        st16u(dst + dstPitch + i * 4 + 2, product2b);
    }
}

void _2xSaISuperEagleLine(
    u2* src, u1* delta, u4 srcPitch, u4 width, u1* dst, u4 dstPitch)
{
    u1 const* const base = (u1 const*)src - srcPitch;

    (void)delta;
    for (u4 i = 0; i < width; i++) {
        u2 const colorB1 = px(base, srcPitch, 0, (int)i);
        u2 const colorB2 = px(base, srcPitch, 0, (int)i + 1);
        u2 const color4 = px(base, srcPitch, 1, (int)i - 1);
        u2 const color5 = px(base, srcPitch, 1, (int)i);
        u2 const color6 = px(base, srcPitch, 1, (int)i + 1);
        u2 const colorS2 = px(base, srcPitch, 1, (int)i + 2);
        u2 const color1 = px(base, srcPitch, 2, (int)i - 1);
        u2 const color2 = px(base, srcPitch, 2, (int)i);
        u2 const color3 = px(base, srcPitch, 2, (int)i + 1);
        u2 const colorS1 = px(base, srcPitch, 2, (int)i + 2);
        u2 const colorA1 = px(base, srcPitch, 3, (int)i);
        u2 const colorA2 = px(base, srcPitch, 3, (int)i + 1);
        u2 const i56 = interpolate(color5, color6);
        u2 const i23 = interpolate(color2, color3);
        u2 const p1a = interpolate(i56, color5); /* 5,5,5,6 */
        u2 const p1b = interpolate(i56, color6); /* 6,6,6,5 */
        u2 const p2a = interpolate(i23, color2); /* 2,2,2,3 */
        u2 const p2b = interpolate(i23, color3); /* 3,3,3,2 */
        int const m35p = color5 == color3 && color6 != color2;
        int const m26p = color5 != color3 && color6 == color2;
        /* The "b" forms ask whether the lean is backed up further out. They
           are taken before the vote widens the masks. */
        int const m35b = m35p
            && ((colorS1 == color5 && color4 == color5)
                || (colorA2 == color5 && colorB1 == color5));
        int const m26b = m26p
            && ((color1 == color6 && colorS2 == color6)
                || (colorA1 == color6 && colorB2 == color6));
        int m35 = m35p, m26 = m26p, vmatch;
        u2 product1a, product1b, product2a, product2b;

        if (color5 == color3 && color6 == color2) {
            int r = 0;

            r += get_result(color5, color6, color1, colorA1);
            r += get_result(color5, color6, color4, colorB1);
            r += get_result(color5, color6, colorA2, colorS1);
            r += get_result(color5, color6, colorB2, colorS2);
            if (r > 0) {
                m35 = 1;
            } else if (r < 0) {
                m26 = 1;
            }
        }

        /* A vertical pair matching with neither lean is its own case. */
        vmatch = (color5 == color2 || color6 == color3) && !(m35 || m26);

        product1a = (vmatch || m35) ? color5
            : (m26 && !m26b)        ? i56
            : m26b                  ? p1b
                                    : p1a;
        product1b = (vmatch || m26) ? color6
            : (m35 && !m35b)        ? i56
            : m35b                  ? p1a
                                    : p1b;
        product2a = (vmatch || m26) ? color2
            : (m35 && !m35b)        ? i23
            : m35b                  ? p2b
                                    : p2a;
        product2b = (vmatch || m35) ? color3
            : (m26 && !m26b)        ? i23
            : m26b                  ? p2a
                                    : p2b;

        st16u(dst + i * 4, product1a);
        st16u(dst + i * 4 + 2, product1b);
        st16u(dst + dstPitch + i * 4, product2a);
        st16u(dst + dstPitch + i * 4 + 2, product2b);
    }
}

void _2xSaILine(u2* src, u1* delta, u4 srcPitch, u4 width, u1* dst, u4 dstPitch)
{
    u1 const* const base = (u1 const*)src - srcPitch;

    (void)delta;
    for (u4 i = 0; i < width; i++) {
        u2 const colorI = px(base, srcPitch, 0, (int)i - 1);
        u2 const colorE = px(base, srcPitch, 0, (int)i);
        u2 const colorF = px(base, srcPitch, 0, (int)i + 1);
        u2 const colorJ = px(base, srcPitch, 0, (int)i + 2);
        u2 const colorG = px(base, srcPitch, 1, (int)i - 1);
        u2 const colorA = px(base, srcPitch, 1, (int)i);
        u2 const colorB = px(base, srcPitch, 1, (int)i + 1);
        u2 const colorK = px(base, srcPitch, 1, (int)i + 2);
        u2 const colorH = px(base, srcPitch, 2, (int)i - 1);
        u2 const colorC = px(base, srcPitch, 2, (int)i);
        u2 const colorD = px(base, srcPitch, 2, (int)i + 1);
        u2 const colorL = px(base, srcPitch, 2, (int)i + 2);
        u2 const colorM = px(base, srcPitch, 3, (int)i - 1);
        u2 const colorN = px(base, srcPitch, 3, (int)i);
        u2 const colorO = px(base, srcPitch, 3, (int)i + 1);
        u2 product, product1, product2;

        if (colorA == colorD && colorB != colorC) {
            if ((colorA == colorE && colorB == colorL)
                || (colorA == colorC && colorA == colorF && colorB != colorE
                    && colorB == colorJ)) {
                product = colorA;
            } else {
                product = interpolate(colorA, colorB);
            }

            if ((colorA == colorG && colorC == colorO)
                || (colorA == colorB && colorA == colorH && colorG != colorC
                    && colorC == colorM)) {
                product1 = colorA;
            } else {
                product1 = interpolate(colorA, colorC);
            }
            product2 = colorA;
        } else if (colorB == colorC && colorA != colorD) {
            if ((colorB == colorF && colorA == colorH)
                || (colorB == colorE && colorB == colorD && colorA != colorF
                    && colorA == colorI)) {
                product = colorB;
            } else {
                product = interpolate(colorA, colorB);
            }

            if ((colorC == colorH && colorA == colorF)
                || (colorC == colorG && colorC == colorD && colorA != colorH
                    && colorA == colorI)) {
                product1 = colorC;
            } else {
                product1 = interpolate(colorA, colorC);
            }
            product2 = colorB;
        } else if (colorA == colorD && colorB == colorC) {
            if (colorA == colorB) {
                product = product1 = product2 = colorA;
            } else {
                int r = 0;

                product1 = interpolate(colorA, colorC);
                product = interpolate(colorA, colorB);
                r += get_result(colorA, colorB, colorE, colorG);
                r += get_result(colorA, colorB, colorF, colorK);
                r += get_result(colorA, colorB, colorH, colorN);
                r += get_result(colorA, colorB, colorL, colorO);
                if (r > 0) {
                    product2 = colorA;
                } else if (r < 0) {
                    product2 = colorB;
                } else {
                    product2 = q_interpolate(colorA, colorB, colorC, colorD);
                }
            }
        } else {
            product2 = q_interpolate(colorA, colorB, colorC, colorD);

            if (colorA == colorC && colorA == colorF && colorB != colorE
                && colorB == colorJ) {
                product = colorA;
            } else if (colorB == colorE && colorB == colorD && colorA != colorF
                && colorA == colorI) {
                product = colorB;
            } else {
                product = interpolate(colorA, colorB);
            }

            if (colorA == colorB && colorA == colorH && colorG != colorC
                && colorC == colorM) {
                product1 = colorA;
            } else if (colorC == colorG && colorC == colorD && colorA != colorH
                && colorA == colorI) {
                product1 = colorC;
            } else {
                product1 = interpolate(colorA, colorC);
            }
        }

        st16u(dst + i * 4, colorA);
        st16u(dst + i * 4 + 2, product);
        st16u(dst + dstPitch + i * 4, product1);
        st16u(dst + dstPitch + i * 4 + 2, product2);
    }
}
