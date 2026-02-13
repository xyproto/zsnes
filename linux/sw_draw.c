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

#include "../video/sw_draw.h"
#include "../cfg.h"
#include "../gblhdr.h"
#include "../intrf.h"
#include "../link.h"
#include "../ui.h"
#include "../video/copyvwin.h"
#include <stdint.h>

void CheckFrame();
// VIDEO VARIABLES
extern SDL_Window* sdl_window;
extern SDL_Surface* surface;
extern int SurfaceLocking;
static SDL_Surface* render_surface = NULL; // 16-bit RGB565 surface for emulator output

extern uint8_t curblank;
extern int frametot;
extern uint8_t GUIOn, GUIOn2;

void NTSCFilterInit();
void NTSCFilterDraw(int SurfaceX, int SurfaceY, int pitch, unsigned char* buffer);
char CheckOGLMode();

bool sw_start(int width, int height, int req_depth, int FullScreen)
{
    uint32_t flags = 0;

    flags |= (FullScreen ? SDL_WINDOW_FULLSCREEN : 0);

    if (NTSCFilter) {
        NTSCFilterInit();
    }

    SurfaceX = width;
    SurfaceY = height;

    if (render_surface) {
        SDL_FreeSurface(render_surface);
        render_surface = NULL;
    }
    if (sdl_window) {
        SDL_DestroyWindow(sdl_window);
    }
    sdl_window = SDL_CreateWindow("ZSNES", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SurfaceX, SurfaceY, flags);
    if (sdl_window == NULL) {
        fprintf(stderr, "Could not create %dx%d window: %s\n", SurfaceX, SurfaceY, SDL_GetError());
        return false;
    }

    // Create a 16-bit RGB565 surface for the emulator to render into
    render_surface = SDL_CreateRGBSurface(0, SurfaceX, SurfaceY, 16,
        0xF800, 0x07E0, 0x001F, 0);
    if (render_surface == NULL) {
        fprintf(stderr, "Could not create render surface: %s\n", SDL_GetError());
        return false;
    }

    surface = render_surface;
    SurfaceLocking = SDL_MUSTLOCK(surface);

    // Grab mouse in fullscreen mode
    SDL_SetWindowGrab(sdl_window, FullScreen ? SDL_TRUE : SDL_FALSE);

    SDL_ShowCursor(SDL_DISABLE);

    // Always RGB565
    converta = 0;

    return true;
}

void sw_end()
{
    if (render_surface) {
        SDL_FreeSurface(render_surface);
        render_surface = NULL;
        surface = NULL;
    }
    if (sdl_window) {
        SDL_DestroyWindow(sdl_window);
        sdl_window = NULL;
    }
}

static void LockSurface()
{
    if (SurfaceLocking) {
        SDL_LockSurface(surface);
    }
}

static void UnlockSurface()
{
    if (SurfaceLocking) {
        SDL_UnlockSurface(surface);
    }
    // Blit the 16-bit render surface to the window surface (with format conversion)
    SDL_Surface* win_surface = SDL_GetWindowSurface(sdl_window);
    if (win_surface) {
        SDL_BlitSurface(render_surface, NULL, win_surface, NULL);
        SDL_UpdateWindowSurface(sdl_window);
    }
}

extern uint8_t NGNoTransp;
extern uint16_t resolutn;
void hq2x_16b();
void hq3x_16b();
void hq4x_16b();
uint32_t pitch;

void sw_clearwin()
{
    pitch = surface->pitch;
    SurfBufD = surface->pixels;

    LockSurface();
    ClearWin16();
    UnlockSurface();
}

extern uint8_t prevNTSCMode;
extern uint8_t changeRes;
extern uint8_t prevKeep4_3Ratio;

void sw_drawwin()
{
    NGNoTransp = 0; // Set this value to 1 within the appropriate
    // video mode if you want to add a custom
    // transparency routine or hardware
    // transparency.  This only works if
    // the value of newengen is equal to 1.
    // (see ProcessTransparencies in newgfx16.asm
    //  for ZSNES' current transparency code)

    UpdateVFrame();

    if (curblank || CheckOGLMode()) {
        return;
    }
    LockSurface();

    if (NTSCFilter != prevNTSCMode) {
        initwinvideo();
    }

    if (changeRes) {
        initwinvideo();
    }

    if (prevKeep4_3Ratio != Keep4_3Ratio) {
        initwinvideo();
    }

    ScreenPtr = vidbuffer;
    ScreenPtr += 16 * 2 + 32 * 2 + 256 * 2;

    if (resolutn == 239) {
        ScreenPtr += 8 * 288 * 2;
    }

    pitch = surface->pitch;
    SurfBufD = surface->pixels;

    if (SurfBufD == 0) {
        UnlockSurface();
        return;
    }

    if (SurfaceX == 256 && SurfaceY == 224) {
        DrawWin256x224x16();
    } else if (SurfaceX == 320 && SurfaceY == 240) {
        DrawWin320x240x16();
    } else if ((SurfaceX == 512 && SurfaceY == 448)) {
        AddEndBytes = pitch - 1024;
        NumBytesPerLine = pitch;
        WinVidMemStart = SurfBufD;

        if (hqFilter) {
            switch (hqFilter) {
            case 1:
                hq2x_16b();
                break;
            case 2:
                // hq3x_16b();
                break;
            case 3:
                // hq4x_16b();
                break;
            default:
                break;
            }
        } else {
            copy640x480x16bwin();
        }
    } else if ((SurfaceX == 602) && NTSCFilter) {
        AddEndBytes = pitch - 1024;
        NumBytesPerLine = pitch;
        WinVidMemStart = SurfBufD;

        NTSCFilterDraw(SurfaceX, SurfaceY, pitch, WinVidMemStart);
    } else if (SurfaceX == 640 && SurfaceY == 480) {
        AddEndBytes = pitch - 1024;
        NumBytesPerLine = pitch;
        WinVidMemStart = SurfBufD + 16 * 640 * 2 + 64 * 2;
        if (hqFilter) {
            switch (hqFilter) {
            case 1:
                hq2x_16b();
                break;
            case 2:
                // hq3x_16b();
                break;
            case 3:
                // hq4x_16b();
                break;
            default:
                break;
            }
        } else if (NTSCFilter) {
            NTSCFilterDraw(SurfaceX, SurfaceY, pitch, WinVidMemStart - 16 * 640 * 2 - 64 * 2);
        } else {
            copy640x480x16bwin();
        }
    }

    UnlockSurface();
}
