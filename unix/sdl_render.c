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

/* The second field sits 75036 pixels on; the line geometry is in copyvwin.h. */
#define SR_SRC_STRIDE VID_STRIDE
#define SR_SRC_SKIP VID_SKIP
#define SR_FIELD2 (75036 * 2)

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

    sr_renderer = SDL_CreateRenderer(sdl_window, NULL);
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
/* One darkened row per source scanline: the last of each group of `vscale`,
   which for the doubled path is every other row, as it always was. Tying it to
   the source line rather than to the output row keeps the CRT look the same
   whether the picture was scaled 2x, 3x or 4x. */
static void sr_scanlines(int const w, int const h, int const vscale)
{
    unsigned const keep = (unsigned)(100 - sl_intensity);
    int y, x;

    for (y = vscale - 1; y < h; y += vscale) {
        unsigned short* row = sr_pixels + (size_t)y * w;
        for (x = 0; x < w; x++) {
            unsigned const p = row[x];
            unsigned const r = ((p >> 11) & 0x1F) * keep / 100u;
            unsigned const g = ((p >> 5) & 0x3F) * keep / 100u;
            unsigned const b = (p & 0x1F) * keep / 100u;
            row[x] = (unsigned short)((r << 11) | (g << 5) | b);
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

    /* Scanlines go over whatever was composed, so they combine with the hq and
       2xSaI filters instead of only showing on an unfiltered picture. Not over
       the NTSC filter, which darkens alternate rows itself. */
    if (sl_intensity && !ntsc_drawn) {
        sr_scanlines(w, h, vscale);
    }

    {
        SDL_Rect const dirty = { 0, 0, w, h };
        SDL_FRect const src = { 0.0f, 0.0f, (float)w, (float)h };

        SDL_UpdateTexture(sr_texture, &dirty, sr_pixels, dst_pitch);
        SDL_RenderClear(sr_renderer);
        SDL_RenderTexture(sr_renderer, sr_texture, &src, NULL);
        SDL_RenderPresent(sr_renderer);
    }
}
