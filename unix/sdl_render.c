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

#include "../cfg.h"
#include "../gblhdr.h"
#include "../link.h"
#include "../video/copyvwin.h"
#include <stdint.h>

void hq2x_16b();
void NTSCFilterDraw(int out_width, int out_height, int out_pitch, unsigned char* rgb16_out);
void NTSCFilterInit(void);

extern SDL_Window* sdl_window;
extern uint8_t* vidbuffer;
extern uint8_t curblank;
extern uint8_t GUIRESIZE[];
extern Uint8 GUIOn2;
extern uint32_t NGNoTransp; /* a dword where it is defined (video/c_newgfx16data.c) */
extern uint8_t SpecialLine[256]; /* 0 if lo-res, > 0 if hi-res */

char CheckOGLMode();

int sr_start(int width, int height, int req_depth, int FullScreen);
void sr_end(void);
void sr_clearwin(void);
void sr_drawwin(void);

/* The composed frame, and what carries it to the GPU. The texture is made at
   the largest size any mode produces - the NTSC filter's 602x446 and the
   640x480 modes are both wider than the doubled 512x448 - and each frame
   uploads and draws only the part it filled. */
#define SR_W 512
#define SR_H 448
#define SR_MAXW 640
#define SR_MAXH 512
static SDL_Renderer* sr_renderer = NULL;
static SDL_Texture* sr_texture = NULL;
static unsigned short* sr_pixels = NULL;

/* vidbuffer is 288 pixels to the line with the picture starting at 16, and the
   second field sits 75036 pixels on. Both are what the GL path used. */
#define SR_SRC_STRIDE 288
#define SR_SRC_SKIP 16
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
    unsigned short const* src1 = (unsigned short*)vidbuffer + SR_SRC_SKIP + line * SR_SRC_STRIDE;
    unsigned short const* src2 = src1 + SR_FIELD2;
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
static void sr_scanlines(void)
{
    unsigned const keep = (unsigned)(100 - sl_intensity);
    int y, x;

    for (y = 1; y < SR_H; y += 2) {
        unsigned short* row = sr_pixels + y * SR_W;
        for (x = 0; x < SR_W; x++) {
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
    int w = SR_W, h = SR_H, pitch = SR_W * 2;

    NGNoTransp = 0; // Set this value to 1 within the appropriate
    // Where a custom or hardware transparency routine would go. Only reachable
    // with newengen == 1; see ProcessTransparencies (video/c_ngtransp.c).
    UpdateVFrame();
    if (curblank || !CheckOGLMode() || !sr_renderer) {
        return;
    }

    if (NTSCFilter && SurfaceX == 602 && SurfaceY <= SR_MAXH) {
        /* The NTSC filter produces its own picture, and only at the 602-wide
           size the blitter's chunking is built around - a narrower width just
           crops. It used to be reachable only from the software path, so an
           accelerated mode resized the window for it and then stretched an
           unfiltered frame over it. */
        w = SurfaceX;
        h = SurfaceY;
        pitch = w * 2;
        NTSCFilterDraw(w, h, pitch, (unsigned char*)sr_pixels);
    } else if (SurfaceX >= 512 && (hqFilter || En2xSaI)) {
        /* The filters write a finished 512-wide picture themselves. */
        AddEndBytes = 0;
        NumBytesPerLine = pitch;
        WinVidMemStart = (void*)sr_pixels;
        if (hqFilter) {
            hq2x_16b();
        } else {
            copy640x480x16bwin();
        }
    } else {
        for (line = 0; line < 224; line++) {
            sr_line(sr_pixels + line * 2 * SR_W, line);
        }
        if (sl_intensity) {
            sr_scanlines();
        }
    }

    {
        SDL_Rect const dirty = { 0, 0, w, h };
        SDL_FRect const src = { 0.0f, 0.0f, (float)w, (float)h };

        SDL_UpdateTexture(sr_texture, &dirty, sr_pixels, pitch);
        SDL_RenderClear(sr_renderer);
        SDL_RenderTexture(sr_renderer, sr_texture, &src, NULL);
        SDL_RenderPresent(sr_renderer);
    }
}
