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
extern SDL_Surface* surface;
extern int SurfaceLocking;

extern unsigned char curblank;
extern int frametot;
extern uint8_t GUIOn, GUIOn2;

void NTSCFilterInit();
void NTSCFilterDraw(int SurfaceX, int SurfaceY, int pitch, unsigned char* buffer);
char CheckOGLMode();

bool sw_start(int width, int height, int req_depth, int FullScreen)
{
    // unsigned int color32, p;
    // int i;
#ifndef __MACOSX__
    uint32_t flags = SDL_DOUBLEBUF | SDL_HWSURFACE;
#else
    uint32_t flags = SDL_SWSURFACE;
#endif
    uint32_t GBitMask;

    flags |= (FullScreen ? SDL_FULLSCREEN : 0);

    if (NTSCFilter) {
        NTSCFilterInit();
    }

    SurfaceX = width;
    SurfaceY = height;
    surface = SDL_SetVideoMode(SurfaceX, SurfaceY, req_depth, flags);
    if (surface == NULL) {
        fprintf(stderr, "Could not set %dx%d video mode: %s\n", SurfaceX, SurfaceY, SDL_GetError());
        return false;
    }

    SurfaceLocking = SDL_MUSTLOCK(surface);

    // Grab mouse in fullscreen mode
    FullScreen ? SDL_WM_GrabInput(SDL_GRAB_ON) : SDL_WM_GrabInput(SDL_GRAB_OFF);

    SDL_WM_SetCaption("ZSNES", "ZSNES");
    SDL_ShowCursor(0);

    // Check hardware for 565/555
    GBitMask = surface->format->Gmask;
    if (GBitMask != 0x07E0) {
        converta = 1;
    } else {
        converta = 0;
    }

    return true;
}

void sw_end()
{
    // Do nothing
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
    SDL_Flip(surface);
}

extern unsigned char NGNoTransp;
extern unsigned short resolutn;
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

extern unsigned char prevNTSCMode;
extern unsigned char changeRes;
extern unsigned char prevKeep4_3Ratio;

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
