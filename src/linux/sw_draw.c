/*
Copyright (C) 1997-2007 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

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


#include "../gblhdr.h"
#include "../cfg.h"
#include "../asm_call.h"


#define BYTE  unsigned char
#define WORD  unsigned short
#define DWORD unsigned long

//C++ style code in C
#define bool unsigned char
#define true 1
#define false 0

void CheckFrame();
// VIDEO VARIABLES
extern SDL_Surface *surface;
extern int SurfaceX, SurfaceY;
extern int SurfaceLocking;

extern unsigned int vidbuffer;
extern DWORD converta;
extern unsigned char curblank;
extern int frametot;
extern BYTE GUIOn,GUIOn2;

void UpdateVFrame(void);
void NTSCFilterInit();
void NTSCFilterDraw(int SurfaceX, int SurfaceY, int pitch, unsigned char * buffer);

bool OGLModeCheck();
void initwinvideo();

bool sw_start(int width, int height, int req_depth, int FullScreen)
{
  //unsigned int color32, p;
  //int i;
#ifndef __MACOSX__
  Uint32 flags = SDL_DOUBLEBUF | SDL_HWSURFACE;
#else
  Uint32 flags = SDL_SWSURFACE;
#endif
  DWORD GBitMask;

  flags |= (FullScreen ? SDL_FULLSCREEN : 0);

  if (NTSCFilter) NTSCFilterInit();

  SurfaceX = width; SurfaceY = height;
  surface = SDL_SetVideoMode(SurfaceX, SurfaceY, req_depth, flags);
  if (surface == NULL) {
    fprintf (stderr, "Could not set %dx%d video mode: %s\n", SurfaceX, SurfaceY, SDL_GetError ());
    return false;
  }

  SurfaceLocking = SDL_MUSTLOCK(surface);
  SDL_WarpMouse(SurfaceX/4,SurfaceY/4);

  // Grab mouse in fullscreen mode
  FullScreen ? SDL_WM_GrabInput(SDL_GRAB_ON) : SDL_WM_GrabInput(SDL_GRAB_OFF);

  SDL_WM_SetCaption ("ZSNES","ZSNES");
  SDL_ShowCursor(0);

  // Check hardware for 565/555
  GBitMask = surface->format->Gmask;
  if(GBitMask != 0x07E0) converta = 1;
  else converta = 0;

  return true;
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
extern unsigned short resolutn;
void copy640x480x16bwin(void);
void hq2x_16b(void);
void hq3x_16b(void);
void hq4x_16b(void);
void ClearWin16 (void);
void DrawWin256x224x16(void);
void DrawWin320x240x16(void);
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

extern unsigned char prevNTSCMode;
extern unsigned char changeRes;
extern unsigned char prevKeep4_3Ratio;

void sw_drawwin()
{
  NGNoTransp = 0;             // Set this value to 1 within the appropriate
                              // video mode if you want to add a custom
                              // transparency routine or hardware
                              // transparency.  This only works if
                              // the value of newengen is equal to 1.
                              // (see ProcessTransparencies in newgfx16.asm
                              //  for ZSNES' current transparency code)


  UpdateVFrame();

  if (curblank || OGLModeCheck()) return;
  LockSurface();

  if (NTSCFilter != prevNTSCMode) initwinvideo();

  if (changeRes) initwinvideo();

  if (prevKeep4_3Ratio != Keep4_3Ratio) initwinvideo();

  ScreenPtr = vidbuffer;
  ScreenPtr += 16*2+32*2+256*2;

  if (resolutn == 239) ScreenPtr+=8*288*2;

  pitch = surface->pitch;
  SurfBufD = (DWORD) surface->pixels;

  if (SurfBufD == 0) {
    UnlockSurface();
    return;
  }

  if (SurfaceX == 256 && SurfaceY == 224) DrawWin256x224x16();
  else if (SurfaceX == 320 && SurfaceY == 240) DrawWin320x240x16();
  else if((SurfaceX == 512 && SurfaceY == 448))
  {
    AddEndBytes = pitch-1024;
    NumBytesPerLine = pitch;
    WinVidMemStart = (void*)SurfBufD;

    if (hqFilter)
    {
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
    }
    else asm_call(copy640x480x16bwin);
  }
  else if ((SurfaceX == 602) && NTSCFilter)
  {
    AddEndBytes = pitch-1024;
    NumBytesPerLine = pitch;
    WinVidMemStart = (void*)SurfBufD;

    NTSCFilterDraw(SurfaceX, SurfaceY, pitch, WinVidMemStart);
  }

  else if (SurfaceX == 640 && SurfaceY == 480)
  {
    AddEndBytes = pitch-1024;
    NumBytesPerLine = pitch;
    WinVidMemStart = (void*) (SurfBufD + 16*640*2 + 64*2);
    if (hqFilter)
    {
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
    }
    else if (NTSCFilter) NTSCFilterDraw(SurfaceX, SurfaceY, pitch, WinVidMemStart-16*640*2-64*2); else asm_call(copy640x480x16bwin);
  }

  UnlockSurface();
}
