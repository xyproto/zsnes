#include "crt.h"

#include "cfg.h"
#include <math.h>

/* Vibrancy and scanlines are one pass over the frame, through a table per
   row of the group: every pixel is a single lookup, and the tables are rebuilt
   only when a setting moves.

   The shape matters more than the depth. Dimming whole rows by a flat
   percentage reads as a grille laid over the picture, because the pattern is
   the same whatever is underneath it. A tube instead paints each line with a
   beam that has a soft profile, and the harder it is driven the wider that
   beam spreads, so bright areas bloom across the gap while dark areas keep it
   open. Making the profile depend on the pixel is what stops the scanlines
   looking painted on, and it costs nothing per pixel: the weight is a function
   of (row within the group, pixel), which is exactly what the table holds. */
#define CRT_MAX_VSCALE 4
#define CRT_LUMA_STEPS 256

static unsigned short crt_lut[CRT_MAX_VSCALE][65536];
static int crt_lut_bright = -1;
static int crt_lut_dark = -1;
static int crt_lut_vscale = -1;

/* Beam weight at `d` line pitches from the centre of the beam, for a line
   driven to `luma`. The spread with drive is the whole point: a dark line is a
   thin bright thread with a black gap either side, a bright one swells until
   the gap almost closes. Too narrow a range and the pattern stops responding
   to the picture and reads as a grille painted over it. Over this range the
   gap runs from about half brightness on black to nine tenths on white. */
static double crt_beam(double const d, double const luma)
{
    double const w = 0.15 + 0.35 * luma;
    double const t = d / w;

    return exp(-0.5 * t * t);
}

static void crt_build_luts(int const vscale)
{
    double const depth = sl_intensity / 100.0;
    /* Vibrancy lifts the picture back after the scanlines have taken light out
       of it. A plain multiply cannot: everything above the clipping point
       flattens into white while the rest still scales, which drains the colour
       out of bright areas. A gamma lift leaves white at white and opens up the
       mid-tones instead, and a little saturation with it puts back the punch a
       tube had. */
    double const gamma = 1.0 + sl_vibrancy / 100.0;
    double const sat = 1.0 + 0.5 * sl_vibrancy / 100.0;
    /* The beam weight depends on the pixel only through its luma, so work the
       exponential out once per luma step rather than once per colour. */
    double weight[CRT_MAX_VSCALE][CRT_LUMA_STEPS];
    int p, l;
    unsigned i;

    /* The rows sample the beam profile, so how much of the picture ends up lit
       depends on how many of them there are: at two rows a line one sits on
       the beam and one in the gap, but at three or four the extra rows all
       land in the gap and the picture goes both darker and coarser as the
       filter scale rises. Hold the average across a line to what it is at two
       rows, by working out the depth that gets there, so the slider means the
       same thing whichever filter is on and switching between them does not
       change the brightness. At two rows the two agree and nothing moves. */
    for (l = 0; l < CRT_LUMA_STEPS; l++) {
        double const luma = (double)l / (CRT_LUMA_STEPS - 1);
        double const gap_ref = 1.0 - 0.5 * (crt_beam(0.0, luma) + crt_beam(0.5, luma));
        double gap = 0.0;
        double at_l = depth;

        for (p = 0; p < vscale; p++) {
            double d = (double)p / vscale;

            if (d > 0.5) {
                d = 1.0 - d; /* the next line's beam is nearer than this one's */
            }
            gap += 1.0 - crt_beam(d, luma);
        }
        gap /= vscale;
        if (gap > 1e-6) {
            at_l = depth * gap_ref / gap;
            if (at_l > 1.0) {
                at_l = 1.0;
            }
        }
        for (p = 0; p < vscale; p++) {
            double d = (double)p / vscale;

            if (d > 0.5) {
                d = 1.0 - d;
            }
            weight[p][l] = 1.0 - at_l * (1.0 - crt_beam(d, luma));
        }
    }
    for (i = 0; i < 65536u; i++) {
        double const ch[3] = { ((i >> 11) & 0x1Fu) / 31.0,
            ((i >> 5) & 0x3Fu) / 63.0, (i & 0x1Fu) / 31.0 };
        double const luma = 0.299 * ch[0] + 0.587 * ch[1] + 0.114 * ch[2];
        int const l = (int)(luma * (CRT_LUMA_STEPS - 1) + 0.5);

        for (p = 0; p < vscale; p++) {
            double out[3];
            double lit = 0.0;
            int c;

            for (c = 0; c < 3; c++) {
                double v = ch[c] * weight[p][l];

                out[c] = pow(v, 1.0 / gamma); /* white stays white */
                lit += (c == 0 ? 0.299 : c == 1 ? 0.587
                                                : 0.114)
                    * out[c];
            }
            for (c = 0; c < 3; c++) {
                out[c] = lit + (out[c] - lit) * sat;
                out[c] = out[c] < 0.0 ? 0.0 : out[c] > 1.0 ? 1.0
                                                           : out[c];
            }
            crt_lut[p][i] = (unsigned short)(((unsigned)(out[0] * 31.0 + 0.5) << 11)
                | ((unsigned)(out[1] * 63.0 + 0.5) << 5)
                | (unsigned)(out[2] * 31.0 + 0.5));
        }
    }
    crt_lut_bright = sl_vibrancy;
    crt_lut_dark = sl_intensity;
    crt_lut_vscale = vscale;
}
/* Bloom: bright areas spill light into what surrounds them, the way a phosphor
   does, and it is the one pass here whose output wants more range than it was
   given - everything else is bounded by its input. On an SDR surface the spill
   has to be clipped back into 565; on an HDR one it is allowed to go above
   white, which is what makes a bright sprite read as glowing rather than as
   pale.

   Worked out at quarter resolution. Bloom is low-frequency so nothing is lost,
   and it is most of the saving: measured over a 768x672 frame, a full
   resolution blur costs 7.7ms against 2.3ms here, of which the blur itself is
   the smaller part. */
#define CRT_BLOOM_DIV 4
#define CRT_BLOOM_W (CRT_MAX_W / CRT_BLOOM_DIV)
#define CRT_BLOOM_H (CRT_MAX_H / CRT_BLOOM_DIV)
#define CRT_BLOOM_TAPS 4 /* each way, so a nine tap blur */
/* Luma above which a pixel starts to spill. Low enough that ordinary bright
   sprites take part: at a high knee only large areas of near-white spilled,
   and the blur then spread what little they gave over so many cells that the
   result was invisible. */
#define CRT_BLOOM_KNEE 0.55f
#define CRT_BLOOM_GAIN 2.5f /* at full slider, per unit over the knee */

static float crt_bloom[CRT_BLOOM_W * CRT_BLOOM_H];
static float crt_bloom_row[CRT_BLOOM_W * CRT_BLOOM_H];
static int crt_bloom_w = 0; /* the size the spill was last worked out at */
static int crt_bloom_h = 0;

/* How much light each quarter-resolution cell spills, blurred. */
void CrtBloomBuild(u2 const* const px, int const w, int const h, int const pitch)
{
    int const bw = w / CRT_BLOOM_DIV;
    int const bh = h / CRT_BLOOM_DIV;
    int x, y, k;

    if (w > CRT_MAX_W || h > CRT_MAX_H || bw <= 0 || bh <= 0) {
        crt_bloom_w = crt_bloom_h = 0;
        return;
    }
    for (y = 0; y < bh; y++) {
        for (x = 0; x < bw; x++) {
            unsigned const p = px[(size_t)(y * CRT_BLOOM_DIV) * (size_t)pitch
                + (size_t)x * CRT_BLOOM_DIV];
            float const luma = 0.299f * ((p >> 11) & 0x1Fu) / 31.0f
                + 0.587f * ((p >> 5) & 0x3Fu) / 63.0f
                + 0.114f * (p & 0x1Fu) / 31.0f;

            crt_bloom_row[y * bw + x]
                = luma > CRT_BLOOM_KNEE ? luma - CRT_BLOOM_KNEE : 0.0f;
        }
    }
    for (y = 0; y < bh; y++) { /* across */
        for (x = 0; x < bw; x++) {
            float sum = 0.0f;

            for (k = -CRT_BLOOM_TAPS; k <= CRT_BLOOM_TAPS; k++) {
                int t = x + k;

                t = t < 0 ? 0 : t >= bw ? bw - 1
                                        : t;
                sum += crt_bloom_row[y * bw + t];
            }
            crt_bloom[y * bw + x] = sum / (2 * CRT_BLOOM_TAPS + 1);
        }
    }
    for (x = 0; x < bw; x++) { /* and down */
        for (y = 0; y < bh; y++) {
            float sum = 0.0f;

            for (k = -CRT_BLOOM_TAPS; k <= CRT_BLOOM_TAPS; k++) {
                int t = y + k;

                t = t < 0 ? 0 : t >= bh ? bh - 1
                                        : t;
                sum += crt_bloom[t * bw + x];
            }
            crt_bloom_row[y * bw + x] = sum / (2 * CRT_BLOOM_TAPS + 1);
        }
    }
    crt_bloom_w = bw;
    crt_bloom_h = bh;
}

/* Where each output pixel sits between the bloom cells. The mapping is the
   same for every frame of a given size, so it is worked out once into a pair
   of tables rather than with a floor and a divide per pixel - doing it per
   pixel cost more than the blur it was sampling. */
static short crt_bx0[CRT_MAX_W], crt_by0[CRT_MAX_H];
static float crt_btx[CRT_MAX_W], crt_bty[CRT_MAX_H];
static int crt_bmap_w = 0, crt_bmap_h = 0;

static void crt_bloom_map(short* const idx, float* const frac, int const n,
    int const cells)
{
    int i;

    for (i = 0; i < n; i++) {
        /* Cell centres sit half a cell in, so the sample point is offset. */
        float const f = ((float)i + 0.5f) / CRT_BLOOM_DIV - 0.5f;
        int c = (int)f;

        if (f < 0.0f) {
            c = 0;
        }
        if (c > cells - 1) {
            c = cells - 1;
        }
        idx[i] = (short)c;
        frac[i] = f - (float)c;
        if (frac[i] < 0.0f) {
            frac[i] = 0.0f;
        }
    }
}

/* Add the spill back into the 565 frame, where it has nowhere to go but up
   against white. Sampled between the cells, not out of the nearest one: the
   spill is worked out at quarter resolution, and taking the nearest cell
   paints it as 4x4 blocks, which is what makes bloom look pixelated even
   where the glow underneath is smooth. */
/* One row: the shade table if there is one, then the spill if there is
   one, in a single read-modify-write of the row. */
static void crt_row(u2* const row, int const w, u2 const* const lut,
    int const* const rowspill, u1 const* const add5, u1 const* const add6)
{
    int x;

    if (!rowspill) {
        for (x = 0; x < w; x++) {
            row[x] = lut[row[x]];
        }
        return;
    }
    for (x = 0; x < w; x++) {
        int const x0 = crt_bx0[x];
        int const q = (rowspill[x0] + (int)((rowspill[x0 + 1] - rowspill[x0]) * crt_btx[x])) >> 8;
        unsigned p = lut ? lut[row[x]] : row[x];
        unsigned r, g, bl;

        if (q <= 0) {
            row[x] = (u2)p;
            continue;
        }
        r = ((p >> 11) & 0x1Fu) + add5[q];
        g = ((p >> 5) & 0x3Fu) + add6[q];
        bl = (p & 0x1Fu) + add5[q];
        r = r > 0x1Fu ? 0x1Fu : r;
        g = g > 0x3Fu ? 0x3Fu : g;
        bl = bl > 0x1Fu ? 0x1Fu : bl;
        row[x] = (u2)((r << 11) | (g << 5) | bl);
    }
}

/* Shade and bloom together, one pass over the picture. `groups` is 1 with
   the beam off; `shade` 0 leaves the brightness alone. */
static void crt_shade_bloom(u2* const px, int const w, int const h, int const pitch,
    int const groups, int const shade, int const bloom)
{
    float const scale = (float)BloomLevel / 100.0f * CRT_BLOOM_GAIN;
    int const bw = crt_bloom_w;
    int const bh = crt_bloom_h;
    /* Spill is a smooth low-frequency field, so it is walked in 8.8 fixed
       point along the row and quantised to 8 bits, and the per-channel adds
       come from a table rather than from float maths on every pixel. */
    static int rowspill[CRT_BLOOM_W + 1];
    static u1 add5[256], add6[256];
    static int tables;
    int const spill = bloom && bw > 0 && bh > 0;
    int x, y;

    if (w > CRT_MAX_W || h > CRT_MAX_H || (!shade && !spill)) {
        return;
    }
    if (!tables) {
        for (x = 0; x < 256; x++) {
            add5[x] = (u1)(x * 31 / 255);
            add6[x] = (u1)(x * 63 / 255);
        }
        tables = 1;
    }
    if (spill && (crt_bmap_w != w || crt_bmap_h != h)) {
        crt_bloom_map(crt_bx0, crt_btx, w, bw);
        crt_bloom_map(crt_by0, crt_bty, h, bh);
        crt_bmap_w = w;
        crt_bmap_h = h;
    }
    for (y = 0; y < h; y++) {
        u2* const row = px + (size_t)y * (size_t)pitch;
        u2 const* const lut = shade ? crt_lut[y % groups] : NULL;
        int peak = 0;

        if (spill) {
            int const y0 = crt_by0[y];
            int const y1 = y0 + 1 > bh - 1 ? bh - 1 : y0 + 1;
            float const ty = crt_bty[y];
            float const* const r0 = crt_bloom_row + (size_t)y0 * bw;
            float const* const r1 = crt_bloom_row + (size_t)y1 * bw;

            for (x = 0; x < bw; x++) {
                float v = (r0[x] + (r1[x] - r0[x]) * ty) * scale;

                v = v > 1.0f ? 1.0f : v;
                rowspill[x] = (int)(v * 65280.0f); /* 255 << 8 */
                peak = rowspill[x] > peak ? rowspill[x] : peak;
            }
            rowspill[bw] = rowspill[bw - 1];
        }
        if (peak < 256) { /* nothing on this row spills a visible amount */
            if (lut) {
                crt_row(row, w, lut, NULL, add5, add6);
            }
            continue;
        }
        crt_row(row, w, lut, rowspill, add5, add6);
    }
}

void CrtBloomApply(u2* const px, int const w, int const h, int const pitch)
{
    crt_shade_bloom(px, w, h, pitch, 1, 0, 1);
}

/* The spill at one pixel, for the HDR path, which adds it in linear light. */
float CrtBloomAt(int const x, int const y, int const w)
{
    int const bw = crt_bloom_w;
    int const bh = crt_bloom_h;
    int x0, x1, y0, y1;
    float a, b;

    if (bw <= 0 || bh <= 0 || crt_bmap_w != w) {
        return 0.0f;
    }
    x0 = crt_bx0[x];
    y0 = crt_by0[y];
    x1 = x0 + 1 > bw - 1 ? bw - 1 : x0 + 1;
    y1 = y0 + 1 > bh - 1 ? bh - 1 : y0 + 1;
    a = crt_bloom_row[(size_t)y0 * bw + x0]
        + (crt_bloom_row[(size_t)y0 * bw + x1] - crt_bloom_row[(size_t)y0 * bw + x0])
            * crt_btx[x];
    b = crt_bloom_row[(size_t)y1 * bw + x0]
        + (crt_bloom_row[(size_t)y1 * bw + x1] - crt_bloom_row[(size_t)y1 * bw + x0])
            * crt_btx[x];
    return (a + (b - a) * crt_bty[y]) * ((float)BloomLevel / 100.0f) * CRT_BLOOM_GAIN;
}
/* The beam sits on the first row of each group, so the rows after it fall away
   and pick up again at the next line's beam. With scanlines off every row uses
   the beam row's table, which is brightness alone. */
static int crt_groups(int const vscale, int const scanlines)
{
    int const groups
        = (scanlines && vscale >= 1 && vscale <= CRT_MAX_VSCALE) ? vscale : 1;

    if (crt_lut_bright != (int)sl_vibrancy || crt_lut_dark != (int)sl_intensity
        || crt_lut_vscale != groups) {
        crt_build_luts(groups);
    }
    return groups;
}

void CrtShade(u2* const px, int const w, int const h, int const pitch,
    int const vscale, int const scanlines)
{
    crt_shade_bloom(px, w, h, pitch, crt_groups(vscale, scanlines), 1, 0);
}

void CrtShadeBloom(u2* const px, int const w, int const h, int const pitch,
    int const vscale, int const scanlines)
{
    int const shade = scanlines || sl_vibrancy;

    crt_shade_bloom(px, w, h, pitch, shade ? crt_groups(vscale, scanlines) : 1,
        shade, BloomLevel != 0);
}

void CrtPass(u2* const px, int const w, int const h, int const pitch,
    int const vscale, int const scanlines)
{
    if (BloomLevel) {
        CrtBloomBuild(px, w, h, pitch);
    }
    CrtShadeBloom(px, w, h, pitch, vscale, scanlines);
}
