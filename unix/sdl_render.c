// Looks good
/*
Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes
https://zsnes.bountysource.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
 * The accelerated output path, on SDL_Renderer instead of OpenGL 1.x: Metal on
 * macOS, D3D on Windows, Vulkan or GL elsewhere, whichever SDL picks.
 *
 * The GL path drew the frame as a run of quads because a scanline can be
 * hi-res while its neighbour is not, and each run needed its own texture
 * geometry. Composing the frame into one 512x448 image first makes that a
 * per-line decision instead - a lo-res line is doubled, a hi-res one is
 * interleaved from the two field buffers - and leaves a single upload and a
 * single draw. The filters already produce 512x448, so they are copied
 * straight in.
 */

#include "../gblhdr.h"
#include "../link.h"
#include "../video/2xsaiw.h"
#include "../video/copyvwin.h"
#include "cfg.h"
#include "sdllink.h"
#include <math.h>
#include <stdint.h>

void hq2x_16b(void);
void hq3x_16b(void);
void hq4x_16b(void);
#include "../video/ntsc.h"

extern SDL_Window* sdl_window;
extern uint8_t* vidbuffer;
extern uint16_t resolutn;
extern uint8_t curblank;
extern uint8_t GUIRESIZE[];
extern Uint8 GUIOn2;
extern uint32_t NGNoTransp; /* a dword where it is defined (video/c_newgfx16data.c) */
extern uint8_t SpecialLine[256]; /* 0 if lo-res, > 0 if hi-res */
extern uint8_t hqFilterlevel; /* 2, 3 or 4 (cfg.psr) */

char CheckOGLMode(void);

int sr_start(int width, int height, int req_depth, int FullScreen);
void sr_end(void);
void sr_clearwin(void);
void sr_drawwin(void);

/* The composed frame, and what carries it to the GPU. The texture is made at
   the largest picture any mode produces - hq4x over a 239-line overscan frame,
   which is wider and taller than the NTSC filter's 602x446, the 640x480 modes
   and the doubled 512x448 - and each frame uploads and draws only the part it
   filled, so the renderer scales whatever size came out to the window. */
#define SR_W 512
#define SR_H 448
#define SR_MAXW 1024
#define SR_MAXH (239 * 4)
static SDL_Renderer* sr_renderer = NULL;
static SDL_Texture* sr_texture = NULL;
static unsigned short* sr_pixels = NULL;

/* The HDR path, used only when the setting asks for it *and* the renderer says
   the display is really in HDR. sr_hdr holds linear float RGBA, where 1.0 is
   SDR white and the bloom is free to go above it, up to what the display says
   it has room for. */
static float* sr_hdr = NULL;
static SDL_Texture* sr_hdr_texture = NULL;
static float sr_hdr_headroom = 1.0f;
static int sr_hdr_active = 0;

/* The second field sits 75036 pixels on; the line geometry is in copyvwin.h. */
#define SR_SRC_STRIDE VID_STRIDE
#define SR_SRC_SKIP VID_SKIP
#define SR_FIELD2 (75036 * 2)

/* A renderer whose output is linear light, which is what SDL wants before it
   will hand back an HDR surface. Falls back to the ordinary one if the request
   is refused, so asking for HDR on a machine without it costs nothing. */
static SDL_Renderer* sr_create_hdr_renderer(SDL_Window* const win)
{
    SDL_PropertiesID const props = SDL_CreateProperties();
    SDL_Renderer* r;

    if (!props) {
        return NULL;
    }
    SDL_SetPointerProperty(props, SDL_PROP_RENDERER_CREATE_WINDOW_POINTER, win);
    SDL_SetNumberProperty(props, SDL_PROP_RENDERER_CREATE_OUTPUT_COLORSPACE_NUMBER,
        SDL_COLORSPACE_SRGB_LINEAR);
    r = SDL_CreateRendererWithProperties(props);
    SDL_DestroyProperties(props);
    return r;
}

/* Whether this renderer is actually showing HDR, and how far above SDR white
   it will go. Both can change while running, so they are read per frame. */
static void sr_hdr_refresh(void)
{
    SDL_PropertiesID props;

    sr_hdr_active = 0;
    sr_hdr_headroom = 1.0f;
    if (!HDROutput || !sr_hdr_texture || !sr_renderer) {
        return;
    }
    props = SDL_GetRendererProperties(sr_renderer);
    if (!props) {
        return;
    }
    if (SDL_GetBooleanProperty(props, SDL_PROP_RENDERER_HDR_ENABLED_BOOLEAN, false)) {
        sr_hdr_active = 1;
        sr_hdr_headroom
            = SDL_GetFloatProperty(props, SDL_PROP_RENDERER_HDR_HEADROOM_FLOAT, 1.0f);
        if (sr_hdr_headroom < 1.0f) {
            sr_hdr_headroom = 1.0f;
        }
    }
}

int sr_start(int width, int height, int req_depth, int FullScreen)
{
    uint32_t flags = 0;

    (void)req_depth;
    flags |= (GUIRESIZE[cvidmode] ? SDL_WINDOW_RESIZABLE : 0);
    flags |= (FullScreen ? SDL_WINDOW_FULLSCREEN : 0);

    if (NTSCFilter) {
        NTSCFilterInit();
    }

    SurfaceX = width;
    SurfaceY = height;

    sr_end();

    sdl_window = SDL_CreateWindow("ZSNES", SurfaceX, SurfaceY, flags);
    if (!sdl_window) {
        fprintf(stderr, "Could not create %dx%d window: %s\n", SurfaceX, SurfaceY,
            SDL_GetError());
        return false;
    }
    PlaceWindowOnMonitor(sdl_window);

    sr_renderer = HDROutput ? sr_create_hdr_renderer(sdl_window) : NULL;
    if (!sr_renderer) {
        sr_renderer = SDL_CreateRenderer(sdl_window, NULL);
    }
    if (!sr_renderer) {
        fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(sdl_window);
        sdl_window = NULL;
        return false;
    }
    SDL_SetRenderVSync(sr_renderer, vsyncon ? 1 : SDL_RENDERER_VSYNC_DISABLED);

    sr_texture = SDL_CreateTexture(sr_renderer, SDL_PIXELFORMAT_RGB565,
        SDL_TEXTUREACCESS_STREAMING, SR_MAXW, SR_MAXH);
    if (!sr_texture) {
        fprintf(stderr, "Could not create texture: %s\n", SDL_GetError());
        SDL_DestroyRenderer(sr_renderer);
        sr_renderer = NULL;
        SDL_DestroyWindow(sdl_window);
        sdl_window = NULL;
        return false;
    }
    if (HDROutput) {
        /* Float rather than 565, so the bloom has somewhere above white to go.
           If the renderer will not take it we simply stay on the ordinary
           surface: HDR is an extra, never a requirement. */
        sr_hdr_texture = SDL_CreateTexture(sr_renderer, SDL_PIXELFORMAT_RGBA128_FLOAT,
            SDL_TEXTUREACCESS_STREAMING, SR_MAXW, SR_MAXH);
        if (sr_hdr_texture) {
            sr_hdr = (float*)malloc((size_t)SR_MAXW * SR_MAXH * 4 * sizeof(float));
        }
        if (!sr_hdr) {
            if (sr_hdr_texture) {
                SDL_DestroyTexture(sr_hdr_texture);
                sr_hdr_texture = NULL;
            }
            fprintf(stderr, "HDR surface unavailable; using the ordinary one\n");
        }
    }
    /* The GUI is drawn from the same buffer, and reads better unfiltered. */
    SDL_SetTextureScaleMode(sr_texture,
        (BilinearFilter && !(GUIOn2 && !FilteredGUI)) ? SDL_SCALEMODE_LINEAR
                                                      : SDL_SCALEMODE_NEAREST);

    if (!sr_pixels) {
        sr_pixels = (unsigned short*)malloc(SR_MAXW * SR_MAXH * sizeof(unsigned short));
        if (!sr_pixels) {
            return false;
        }
    }
    memset(sr_pixels, 0, SR_MAXW * SR_MAXH * sizeof(unsigned short));

    SDL_SetWindowMouseGrab(sdl_window, FullScreen ? true : false);
    SDL_HideCursor(); // the emulator draws its own pointer
    SDL_SetRenderDrawColor(sr_renderer, 0, 0, 0, 255);
    SDL_RenderClear(sr_renderer);
    SDL_RenderPresent(sr_renderer);
    return true;
}

void sr_end(void)
{
    if (sr_texture) {
        SDL_DestroyTexture(sr_texture);
        sr_texture = NULL;
    }
    if (sr_hdr_texture) {
        SDL_DestroyTexture(sr_hdr_texture);
        sr_hdr_texture = NULL;
    }
    if (sr_hdr) {
        free(sr_hdr);
        sr_hdr = NULL;
    }
    sr_hdr_active = 0;
    if (sr_renderer) {
        SDL_DestroyRenderer(sr_renderer);
        sr_renderer = NULL;
    }
    if (sr_pixels) {
        free(sr_pixels);
        sr_pixels = NULL;
    }
    if (sdl_window) {
        SDL_PumpEvents();
        SDL_DestroyWindow(sdl_window);
        sdl_window = NULL;
    }
}

void sr_clearwin(void)
{
    if (sr_renderer) {
        SDL_SetRenderDrawColor(sr_renderer, 0, 0, 0, 255);
        SDL_RenderClear(sr_renderer);
        SDL_RenderPresent(sr_renderer);
    }
}

/* One source line into the two output rows it occupies. */
static void sr_line(unsigned short* dst, int const line)
{
    unsigned short const* src1
        = (unsigned short*)vidbuffer + VID_FIRST + line * VID_STRIDE;
    unsigned short const* src2 = src1 + SR_FIELD2;
    /* SpecialLine is indexed by scanline, and output row `line` is scanline
       line+1 - the same line VID_FIRST starts src1 on. */
    int const hires = SpecialLine[line + 1];
    int i;

    if (!hires) {
        for (i = 0; i < 256; i++) {
            dst[i * 2] = dst[i * 2 + 1] = src1[i];
        }
        memcpy(dst + SR_W, dst, SR_W * sizeof(unsigned short));
        return;
    }
    if (hires > 1) {
        /* Mode 7: the two fields are separate 256-wide lines, one above the
           other, so each is doubled across and kept on its own row. */
        for (i = 0; i < 256; i++) {
            dst[i * 2] = dst[i * 2 + 1] = src1[i];
            dst[SR_W + i * 2] = dst[SR_W + i * 2 + 1] = src2[i];
        }
        return;
    }
    /* 512 across: the fields interleave column by column. */
    for (i = 0; i < 256; i++) {
        dst[i * 2] = src1[i];
        dst[i * 2 + 1] = src2[i];
    }
    memcpy(dst + SR_W, dst, SR_W * sizeof(unsigned short));
}

/* Halve every second row, which is what the GL path's blended 1D texture did. */
/* Scale a 565 pixel by a percentage, saturating. 100 leaves it alone. */
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
#define SR_MAX_VSCALE 4
#define SR_LUMA_STEPS 256

static unsigned short sr_lut[SR_MAX_VSCALE][65536];
static int sr_lut_bright = -1;
static int sr_lut_dark = -1;
static int sr_lut_vscale = -1;

/* Beam weight at `d` line pitches from the centre of the beam, for a line
   driven to `luma`. Half-width runs from a fifth of the pitch when black to
   two fifths when full, which is roughly where a tube's spot sits. */
static double sr_beam(double const d, double const luma)
{
    double const w = 0.20 + 0.20 * luma;
    double const t = d / w;

    return exp(-0.5 * t * t);
}

static void sr_build_luts(int const vscale)
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
    double weight[SR_MAX_VSCALE][SR_LUMA_STEPS];
    int p, l;
    unsigned i;

    for (p = 0; p < vscale; p++) {
        double d = (double)p / vscale;

        if (d > 0.5) {
            d = 1.0 - d; /* the next line's beam is nearer than this one's */
        }
        for (l = 0; l < SR_LUMA_STEPS; l++) {
            double const luma = (double)l / (SR_LUMA_STEPS - 1);

            weight[p][l] = 1.0 - depth * (1.0 - sr_beam(d, luma));
        }
    }
    for (i = 0; i < 65536u; i++) {
        double const ch[3] = { ((i >> 11) & 0x1Fu) / 31.0,
            ((i >> 5) & 0x3Fu) / 63.0, (i & 0x1Fu) / 31.0 };
        double const luma = 0.299 * ch[0] + 0.587 * ch[1] + 0.114 * ch[2];
        int const l = (int)(luma * (SR_LUMA_STEPS - 1) + 0.5);

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
            sr_lut[p][i] = (unsigned short)(((unsigned)(out[0] * 31.0 + 0.5) << 11)
                | ((unsigned)(out[1] * 63.0 + 0.5) << 5)
                | (unsigned)(out[2] * 31.0 + 0.5));
        }
    }
    sr_lut_bright = sl_vibrancy;
    sr_lut_dark = sl_intensity;
    sr_lut_vscale = vscale;
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
#define SR_BLOOM_DIV 4
#define SR_BLOOM_W (SR_MAXW / SR_BLOOM_DIV)
#define SR_BLOOM_H (SR_MAXH / SR_BLOOM_DIV)
#define SR_BLOOM_TAPS 4 /* each way, so a nine tap blur */
#define SR_BLOOM_KNEE 0.72f /* luma above which a pixel starts to spill */

static float sr_bloom[SR_BLOOM_W * SR_BLOOM_H];
static float sr_bloom_row[SR_BLOOM_W * SR_BLOOM_H];

/* How much light each quarter-resolution cell spills, blurred. */
static void sr_bloom_build(int const w, int const h)
{
    int const bw = w / SR_BLOOM_DIV;
    int const bh = h / SR_BLOOM_DIV;
    int x, y, k;

    for (y = 0; y < bh; y++) {
        for (x = 0; x < bw; x++) {
            unsigned const p = sr_pixels[(size_t)(y * SR_BLOOM_DIV) * w
                + (size_t)x * SR_BLOOM_DIV];
            float const luma = 0.299f * ((p >> 11) & 0x1Fu) / 31.0f
                + 0.587f * ((p >> 5) & 0x3Fu) / 63.0f
                + 0.114f * (p & 0x1Fu) / 31.0f;

            sr_bloom_row[y * bw + x]
                = luma > SR_BLOOM_KNEE ? luma - SR_BLOOM_KNEE : 0.0f;
        }
    }
    for (y = 0; y < bh; y++) { /* across */
        for (x = 0; x < bw; x++) {
            float sum = 0.0f;

            for (k = -SR_BLOOM_TAPS; k <= SR_BLOOM_TAPS; k++) {
                int t = x + k;

                t = t < 0 ? 0 : t >= bw ? bw - 1
                                        : t;
                sum += sr_bloom_row[y * bw + t];
            }
            sr_bloom[y * bw + x] = sum / (2 * SR_BLOOM_TAPS + 1);
        }
    }
    for (x = 0; x < bw; x++) { /* and down */
        for (y = 0; y < bh; y++) {
            float sum = 0.0f;

            for (k = -SR_BLOOM_TAPS; k <= SR_BLOOM_TAPS; k++) {
                int t = y + k;

                t = t < 0 ? 0 : t >= bh ? bh - 1
                                        : t;
                sum += sr_bloom[t * bw + x];
            }
            sr_bloom_row[y * bw + x] = sum / (2 * SR_BLOOM_TAPS + 1);
        }
    }
}

/* The spill at a full-resolution pixel, scaled by the slider. */
static float sr_bloom_at(int const x, int const y, int const w)
{
    int const bw = w / SR_BLOOM_DIV;

    return sr_bloom_row[(size_t)(y / SR_BLOOM_DIV) * bw + (size_t)(x / SR_BLOOM_DIV)]
        * ((float)BloomLevel / 100.0f) * 2.0f;
}

/* Add the spill back into the 565 frame, where it has nowhere to go but up
   against white. */
static void sr_bloom_apply(int const w, int const h)
{
    int x, y;

    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            unsigned const p = sr_pixels[(size_t)y * w + x];
            float const b = sr_bloom_at(x, y, w);
            unsigned r = (unsigned)(((p >> 11) & 0x1Fu) + b * 31.0f);
            unsigned g = (unsigned)(((p >> 5) & 0x3Fu) + b * 63.0f);
            unsigned bl = (unsigned)((p & 0x1Fu) + b * 31.0f);

            r = r > 0x1Fu ? 0x1Fu : r;
            g = g > 0x3Fu ? 0x3Fu : g;
            bl = bl > 0x1Fu ? 0x1Fu : bl;
            sr_pixels[(size_t)y * w + x]
                = (unsigned short)((r << 11) | (g << 5) | bl);
        }
    }
}

/* Widen the composed frame into linear float, adding the bloom without
   clipping it. 565 is only 32 levels a channel, so the conversion is a plain
   scale - HDR here buys headroom above white, not tonal resolution, and
   stretching 15-bit source would only make its banding easier to see. The
   spill is what uses the headroom. */
static void sr_to_hdr(int const w, int const h)
{
    float const room = sr_hdr_headroom;
    int x, y;

    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            unsigned const p = sr_pixels[(size_t)y * w + x];
            float const b = BloomLevel ? sr_bloom_at(x, y, w) : 0.0f;
            float* const o = sr_hdr + ((size_t)y * w + x) * 4;
            float c[3];
            int i;

            c[0] = ((p >> 11) & 0x1Fu) / 31.0f;
            c[1] = ((p >> 5) & 0x3Fu) / 63.0f;
            c[2] = (p & 0x1Fu) / 31.0f;
            for (i = 0; i < 3; i++) {
                float v = c[i] + b;

                o[i] = v > room ? room : v;
            }
            o[3] = 1.0f;
        }
    }
}

/* The beam sits on the first row of each group, so the rows after it fall away
   and pick up again at the next line's beam. With scanlines off every row uses
   the beam row's table, which is brightness alone. */
static void sr_shade_frame(int const w, int const h, int const vscale,
    int const scanlines)
{
    int const groups = (scanlines && vscale <= SR_MAX_VSCALE) ? vscale : 1;
    int y, x;

    if (sr_lut_bright != (int)sl_vibrancy || sr_lut_dark != (int)sl_intensity
        || sr_lut_vscale != groups) {
        sr_build_luts(groups);
    }
    for (y = 0; y < h; y++) {
        unsigned short* const row = sr_pixels + (size_t)y * w;
        unsigned short const* const lut = sr_lut[y % groups];

        for (x = 0; x < w; x++) {
            row[x] = lut[row[x]];
        }
    }
}

void sr_drawwin(void)
{
    int line;
    /* What this frame ends up filling, which the upload and the draw follow. */
    int w = SR_W, h = SR_H, dst_pitch = SR_W * 2;
    /* Output rows per source scanline, which is what the scanline pass steps
       by. The unfiltered path below doubles, whatever `resolutn` is. */
    int vscale = 2;
    int ntsc_drawn = 0;

    NGNoTransp = 0; // Set this value to 1 within the appropriate
    // Where a custom or hardware transparency routine would go. Only reachable
    // with newengen == 1; see ProcessTransparencies (video/c_ngtransp.c).
    UpdateVFrame();
    if (curblank || !CheckOGLMode() || !sr_renderer) {
        return;
    }

    if (NTSCFilter) {
        ntsc_drawn = 1;
        /* Always at the filter's own size, whatever the window is: a narrower
           request crops the picture and a wider one reads past the end of the
           source line. The renderer scales it out, so this no longer needs a
           602-wide video mode to be selected first. */
        w = NTSC_OUT_WIDTH;
        h = (int)resolutn * 2;
        dst_pitch = w * 2;
        NTSCFilterDraw(w, h, dst_pitch, (unsigned char*)sr_pixels);
    } else if (SurfaceX >= 512 && (hqFilter || En2xSaI)) {
        /* The filters write a finished picture themselves, at their own scale:
           hq2x 512 wide, hq3x 768, hq4x 1024. The renderer scales it to the
           window, so the filter picked is the filter drawn. */
        int const scale = hqFilter ? (hqFilterlevel < 2          ? 2
                                             : hqFilterlevel > 4 ? 4
                                                                 : hqFilterlevel)
                                   : 2;

        w = 256 * scale;
        h = (int)resolutn * scale;
        dst_pitch = w * 2;
        vscale = scale;
        AddEndBytes = 0;
        NumBytesPerLine = dst_pitch;
        WinVidMemStart = (void*)sr_pixels;
        if (hqFilter) {
            if (scale == 4) {
                hq4x_16b();
            } else if (scale == 3) {
                hq3x_16b();
            } else {
                hq2x_16b();
            }
        } else {
            /* En2xSaI: 1 = 2xSaI, 2 = Super Eagle, 3 = Super 2xSaI (cfg.psr).
               The filters take one line at a time and read a row above and two
               below, which the vidbuffer border already provides. */
            LineFilter* const f = En2xSaI == 2 ? _2xSaISuperEagleLine
                : En2xSaI == 3                 ? _2xSaISuper2xSaILine
                                               : _2xSaILine;
            unsigned short* const base = (unsigned short*)vidbuffer + VID_FIRST;

            for (unsigned y = 0; y < resolutn; y++) {
                f(base + (size_t)y * SR_SRC_STRIDE, NULL, SR_SRC_STRIDE * 2, 256,
                    (unsigned char*)sr_pixels + (size_t)y * 2 * dst_pitch, dst_pitch);
            }
        }
    } else {
        for (line = 0; line < 224; line++) {
            sr_line(sr_pixels + line * 2 * SR_W, line);
        }
    }

    /* Brightness and scanlines go over whatever was composed, so they combine
       with the hq and 2xSaI filters instead of only showing on an unfiltered
       picture. Scanlines are skipped over the NTSC filter, which dims alternate
       rows itself, but brightness still applies. */
    {
        int const scanlines = (sl_intensity != 0 && !ntsc_drawn);

        if (scanlines || sl_vibrancy) {
            sr_shade_frame(w, h, vscale, scanlines);
        }
    }

    sr_hdr_refresh();
    if (BloomLevel) {
        sr_bloom_build(w, h);
        if (!sr_hdr_active) {
            sr_bloom_apply(w, h); /* clipped into 565 */
        }
    }

    {
        SDL_Rect const dirty = { 0, 0, w, h };
        SDL_FRect const src = { 0.0f, 0.0f, (float)w, (float)h };
        SDL_Texture* const tex = sr_hdr_active ? sr_hdr_texture : sr_texture;

        if (sr_hdr_active) {
            sr_to_hdr(w, h);
            SDL_UpdateTexture(tex, &dirty, sr_hdr, w * 4 * (int)sizeof(float));
        } else {
            SDL_UpdateTexture(tex, &dirty, sr_pixels, dst_pitch);
        }
        SDL_RenderClear(sr_renderer);
        SDL_RenderTexture(sr_renderer, tex, &src, NULL);
        SDL_RenderPresent(sr_renderer);
    }
}
