/*
Copyright (C) 1997-2006 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes

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


#include "gblhdr.h"

#define BYTE  unsigned char
#define WORD  unsigned short
#define DWORD unsigned long

typedef enum { FALSE = 0, TRUE = !FALSE } BOOL;
extern void CheckFrame();
// VIDEO VARIABLES
extern unsigned char cvidmode;
extern SDL_Surface *surface;
extern int SurfaceX, SurfaceY;
extern int SurfaceLocking;

extern unsigned int vidbuffer;
extern DWORD converta;
extern unsigned char curblank;
extern int frametot;
extern BYTE GUIOn,GUIOn2;
int prevtot = 0;
void UpdateVFrame(void);

BOOL sw_start(int width, int height, int req_depth, int FullScreen)
{
    //unsigned int color32, p;
    //int i;
    Uint32 flags = SDL_DOUBLEBUF | SDL_SWSURFACE;
    DWORD GBitMask;

    flags |= (FullScreen ? SDL_FULLSCREEN : 0);

    SurfaceX = width; SurfaceY = height;
    surface = SDL_SetVideoMode(SurfaceX, SurfaceY, req_depth, flags);
    if (surface == NULL) {
	fprintf (stderr, "Could not set %dx%d video mode: %s\n", SurfaceX, SurfaceY, SDL_GetError ());
	return FALSE;

    }

    SurfaceLocking = SDL_MUSTLOCK(surface);
    SDL_WarpMouse(SurfaceX/4,SurfaceY/4);

    // Grab mouse in fullscreen mode
    FullScreen ? SDL_WM_GrabInput(SDL_GRAB_ON) : SDL_WM_GrabInput(SDL_GRAB_OFF);

    SDL_WM_SetCaption ("ZSNES","ZSNES");
    SDL_ShowCursor(0);

    // Check hardware for 565/555
    GBitMask = surface->format->Gmask;
    if(GBitMask != 0x07E0) {
        converta = 1;
    } else {
        converta = 0;
    }

    return TRUE;
}

void sw_end() {
    // Do nothing
}

static void LockSurface(void)
{
    if (SurfaceLocking) SDL_LockSurface(surface);
}

static void UnlockSurface(void)
{
    if (SurfaceLocking) SDL_UnlockSurface(surface);
    SDL_Flip(surface);
}

extern DWORD AddEndBytes;
extern DWORD NumBytesPerLine;
extern unsigned char *WinVidMemStart;
extern unsigned char NGNoTransp;
extern unsigned char newengen;
extern unsigned short resolutn;
extern void copy640x480x16bwin(void);
extern void hq2x_16b(void);
extern void hq3x_16b(void);
extern void hq4x_16b(void);
extern void ClearWin16 (void);
extern void DrawWin256x224x16(void);
extern void DrawWin320x240x16(void);

extern char hqFilter;

DWORD ScreenPtr;
DWORD SurfBufD;
DWORD pitch;

void sw_clearwin()
{
    pitch = surface->pitch;
    SurfBufD = (DWORD) surface->pixels;

    LockSurface();
    ClearWin16();
    UnlockSurface();
}

void sw_drawwin()
{
    NGNoTransp = 0;             // Set this value to 1 within the appropriate
                                // video mode if you want to add a custom
                                // transparency routine or hardware
                                // transparency.  This only works if
                                // the value of newengen is equal to 1.
                                // (see ProcessTransparencies in newgfx16.asm
                                //  for ZSNES' current transparency code)


    prevtot = frametot;
    CheckFrame();

    UpdateVFrame();

    if (prevtot == frametot && (!GUIOn || !GUIOn2)) { return; }

    if (curblank || cvidmode > 5) return;
    LockSurface();

    ScreenPtr = vidbuffer;
    ScreenPtr += 16*2+32*2+256*2;

    if (resolutn == 239) ScreenPtr+=8*288*2;

    pitch = surface->pitch;
    SurfBufD = (DWORD) surface->pixels;

    if (SurfBufD == 0) {
	UnlockSurface();
	return;
    }

    if (SurfaceX == 256 && SurfaceY == 224) {
	    DrawWin256x224x16();
    } else if (SurfaceX == 320 && SurfaceY == 240) {
	    DrawWin320x240x16();
    } else if(SurfaceX == 512 && SurfaceY == 448) {
	AddEndBytes = pitch-1024;
	NumBytesPerLine = pitch;
	WinVidMemStart = (void*)SurfBufD;
	if (hqFilter) {
		switch (hqFilter)
		{
		case 1:
			hq2x_16b();
			break;
		case 2:
			//hq3x_16b();
			break;
		case 3:
			//hq4x_16b();
			break;
		default:
			break;
		}
	} else {
		copy640x480x16bwin();
	}
    } else if (SurfaceX == 640 && SurfaceY == 480) {
	AddEndBytes = pitch-1024;
	NumBytesPerLine = pitch;
	WinVidMemStart = (void*) (SurfBufD + 16*640*2 + 64*2);
	if (hqFilter) {
		switch (hqFilter)
		{
		case 1:
			hq2x_16b();
			break;
		case 2:
			//hq3x_16b();
			break;
		case 3:
			//hq4x_16b();
			break;
		default:
			break;
		}
	} else {
		copy640x480x16bwin();
	}
    }
    UnlockSurface();
}
