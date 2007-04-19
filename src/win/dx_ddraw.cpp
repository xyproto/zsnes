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

#define DIRECTINPUT_VERSION 0x0800
#define DIRECTSOUND_VERSION 0x0800
#define __STDC_CONSTANT_MACROS

extern "C"
{
#include <windows.h>
#include <stdio.h>
#include <ctype.h>
#include <ddraw.h>
#include <mmsystem.h>
#include <time.h>
#include "../cfg.h"
}

#include "winlink.h"

static LPDIRECTDRAW BasiclpDD = NULL;
static LPDIRECTDRAW7 lpDD = NULL;
static LPDIRECTDRAWSURFACE7 DD_Primary = NULL;
static LPDIRECTDRAWSURFACE7 DD_CFB = NULL;
static LPDIRECTDRAWSURFACE7 DD_CFB16 = NULL;
static LPDIRECTDRAWSURFACE7 DD_BackBuffer = NULL;
static LPDIRECTDRAWCLIPPER lpDDClipper = NULL;

DWORD LastUsedPos = 0;
DDSURFACEDESC2 ddsd;

void DDrawError()
{
  char message1[256];

  strcpy(message1, "Error drawing to the screen\nMake sure the device is not being used by another process");
  MessageBox(NULL, message1, "DirectDraw Error", MB_ICONERROR);
}

void DrawScreen()
{
  if (FullScreen == 1)
  {
    if (TripleBufferWin == 1 || KitchenSync == 1 || (KitchenSyncPAL == 1 && totlines == 314))
    {
      if (DD_BackBuffer->Blt(&rcWindow, DD_CFB, &BlitArea, DDBLT_WAIT, NULL) == DDERR_SURFACELOST)
      {
        DD_Primary->Restore();
      }

      if (DD_Primary->Flip(NULL, DDFLIP_WAIT) == DDERR_SURFACELOST)
      {
        DD_Primary->Restore();
      }

      if (KitchenSync == 1 || (KitchenSyncPAL == 1 && totlines == 314))
      {
        if (DD_BackBuffer->Blt(&rcWindow, DD_CFB, &BlitArea, DDBLT_WAIT, NULL) == DDERR_SURFACELOST)
        {
          DD_Primary->Restore();
        }

        if (DD_Primary->Flip(NULL, DDFLIP_WAIT) == DDERR_SURFACELOST)
        {
          DD_Primary->Restore();
        }
      }
    }
    else
    {
      if (vsyncon == 1 && curblank != 0x40)
      {
        if (lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL) != DD_OK)
        {
          DDrawError();
        }
      }
      DD_Primary->Blt(&rcWindow, DD_CFB, &BlitArea, DDBLT_WAIT, NULL);
      DD_Primary->Restore();
    }
  }
  else
  {
    if (vsyncon == 1)
    {
      if (lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL) != DD_OK)
      {
        DDrawError();
      }
    }
    DD_Primary->Blt(&rcWindow, AltSurface == 0 ? DD_CFB : DD_CFB16, &BlitArea, DDBLT_WAIT, NULL);
  }
}


DWORD LockSurface()
{
  HRESULT hRes;

  if (AltSurface == 0)
  {
    if (DD_CFB != NULL)
    {
      memset(&ddsd, 0, sizeof(ddsd));
      ddsd.dwSize = sizeof(ddsd);
      ddsd.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;

      hRes = DD_CFB->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);

      if (hRes == DD_OK)
      {
        SurfBuf = (BYTE *)ddsd.lpSurface;
        return(ddsd.lPitch);
      }
      else
      {
        if (hRes == DDERR_SURFACELOST)
        {
          DD_Primary->Restore();
          DD_CFB->Restore();
          Clear2xSaIBuffer();
        }
        return(0);
      }
    }
    else
    {
      return(0);
    }
  }
  else
  {
    if (DD_CFB16 != NULL)
    {
      memset(&ddsd, 0, sizeof(ddsd));
      ddsd.dwSize = sizeof(ddsd);
      ddsd.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;

      hRes = DD_CFB16->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);

      if (hRes == DD_OK)
      {
        SurfBuf = (BYTE *)ddsd.lpSurface;
        return(ddsd.lPitch);
      }
      else
      {
        if (hRes == DDERR_SURFACELOST)
        {
          DD_Primary->Restore();
          DD_CFB16->Restore();
          Clear2xSaIBuffer();
        }
        return(0);
      }
    }
    else
    {
      return(0);
    }
  }
}

void UnlockSurface()
{
  if (AltSurface == 0)
  {
    DD_CFB->Unlock((struct tagRECT *)ddsd.lpSurface);
  }
  else
  {
    DD_CFB16->Unlock((struct tagRECT *)ddsd.lpSurface);
  }
}

int InitDirectDraw()
{
  DDSURFACEDESC2 ddsd2;
  DDPIXELFORMAT format;

  unsigned int color32, ScreenPtr2;
  int i, j, k, r, g, b, Y, u, v;

  ScreenPtr2 = BitConv32Ptr;
  for (i = 0; i < 65536; i++)
  {
    color32 = ((i & 0xF800) << 8) + ((i & 0x07E0) << 5) + ((i & 0x001F) << 3) + 0xFF000000;
    (*(unsigned int*)(ScreenPtr2)) = color32;
    ScreenPtr2 += 4;
  }

  for (i = 0; i < 32; i++)
  {
    for (j = 0; j < 64; j++)
    {
      for (k = 0; k < 32; k++)
      {
        r = i << 3;
        g = j << 2;
        b = k << 3;
        Y = (r + g + b) >> 2;
        u = 128 + ((r - b) >> 2);
        v = 128 + ((-r + 2 * g - b) >> 3);
        *(((unsigned int*)RGBtoYUVPtr) + (i << 11) + (j << 5) + k) = (Y << 16) + (u << 8) + v;
      }
    }
  }

  if (!hMainWindow)
  {
    exit(1);
  }

  ReleaseDirectDraw();

  GetClientRect(hMainWindow, &rcWindow);
  ClientToScreen(hMainWindow, (LPPOINT)&rcWindow);
  ClientToScreen(hMainWindow, (LPPOINT)&rcWindow + 1);

  FullScreen = GUIWFVID[cvidmode];
  DSMode = GUIDSMODE[cvidmode];

  DWORD HQMode = 0;

  if (hqFilter != 0)
  {
    if ((GUIHQ2X[cvidmode] != 0) && (hqFilterlevel == 2))
    {
      HQMode = 2;
    }
    if ((GUIHQ3X[cvidmode] != 0) && (hqFilterlevel == 3))
    {
      HQMode = 3;
    }
    if ((GUIHQ4X[cvidmode] != 0) && (hqFilterlevel == 4))
    {
      HQMode = 4;
    }
  }

  if (FullScreen == 1)
  {
    if (HQMode && !DSMode)
    {
      int marginx = (rcWindow.right - rcWindow.left - BlitArea.right + BlitArea.left) / 2;
      int marginy = (rcWindow.bottom - rcWindow.top - BlitArea.bottom + BlitArea.top) / 2;
      if (marginx > 0)
      {
        rcWindow.left += marginx;
        rcWindow.right -= marginx;
      }
      if (marginy > 0)
      {
        rcWindow.top += marginy;
        rcWindow.bottom -= marginy;
      }
    }

    if ((DSMode == 1) && (scanlines != 0))
    {
      int OldHeight = rcWindow.bottom - rcWindow.top;
      if ((OldHeight % 240) == 0)
      {
        int NewHeight = (OldHeight / 240) * resolutn;
        rcWindow.top += (OldHeight - NewHeight) / 2;
        rcWindow.bottom = rcWindow.top + NewHeight;
      }
    }
  }

  if (pDirectDrawCreateEx(NULL, (void**)&lpDD, IID_IDirectDraw7, NULL) != DD_OK)
  {
    MessageBox(NULL, "DirectDrawCreateEx failed.", "DirectDraw Error", MB_ICONERROR);
    return FALSE;
  }

  if (FullScreen == 1)
  {
    if (lpDD->SetCooperativeLevel(hMainWindow,
                                  DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE | DDSCL_ALLOWREBOOT) != DD_OK)
    {
      MessageBox(NULL, "IDirectDraw7::SetCooperativeLevel failed.", "DirectDraw Error",
                 MB_ICONERROR);
      return FALSE;
    }
    if (lpDD->SetDisplayMode(WindowWidth, WindowHeight, 16, Refresh, 0) != DD_OK)
    {
      if (lpDD->SetDisplayMode(WindowWidth, WindowHeight, 16, 0, 0) != DD_OK)
      {
        MessageBox(NULL,
                   "IDirectDraw7::SetDisplayMode failed.\nMake sure your video card supports this mode.",
                   "DirectDraw Error", MB_ICONERROR);
        return FALSE;
      }
      else
      {
        KitchenSync = 0;
        KitchenSyncPAL = 0;
        Refresh = 0;
      }
    }
  }
  else
  {
    if (lpDD->SetCooperativeLevel(hMainWindow, DDSCL_NORMAL) != DD_OK)
    {
      MessageBox(NULL, "IDirectDraw7::SetCooperativeLevel failed.", "DirectDraw Error",
                 MB_ICONERROR);
      return FALSE;
    }
    CheckAlwaysOnTop();
  }

  ZeroMemory(&ddsd2, sizeof(DDSURFACEDESC2));
  ddsd2.dwSize = sizeof(DDSURFACEDESC2);
  ddsd2.dwFlags = DDSD_CAPS;
  ddsd2.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

  if (FullScreen == 1)
  {
    ddsd2.dwFlags |= DDSD_BACKBUFFERCOUNT;
    ddsd2.dwBackBufferCount = 2;
    ddsd2.ddsCaps.dwCaps |= DDSCAPS_FLIP | DDSCAPS_COMPLEX;
  }

  HRESULT hRes = lpDD->CreateSurface(&ddsd2, &DD_Primary, NULL);

  if (FullScreen == 1)
  {
    if ((hRes == DDERR_OUTOFMEMORY) || (hRes == DDERR_OUTOFVIDEOMEMORY))
    {
      ddsd2.dwBackBufferCount = 1;
      hRes = lpDD->CreateSurface(&ddsd2, &DD_Primary, NULL);
    }
  }

  if (hRes != DD_OK)
  {
    MessageBox(NULL, "IDirectDraw7::CreateSurface failed.", "DirectDraw Error", MB_ICONERROR);
    return FALSE;
  }

  if (FullScreen == 1)
  {
    ddsd2.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;
    if (DD_Primary->GetAttachedSurface(&ddsd2.ddsCaps, &DD_BackBuffer) != DD_OK)
    {
      MessageBox(NULL, "IDirectDrawSurface7::GetAttachedSurface failed.", "DirectDraw Error",
                 MB_ICONERROR);
      return FALSE;
    }
  }
  else
  {
    if (lpDD->CreateClipper(0, &lpDDClipper, NULL) != DD_OK)
    {
      lpDD->Release();
      lpDD = NULL;
      return FALSE;
    }

    if (lpDDClipper->SetHWnd(0, hMainWindow) != DD_OK)
    {
      lpDD->Release();
      lpDD = NULL;
      return FALSE;
    }

    if (DD_Primary->SetClipper(lpDDClipper) != DD_OK)
    {
      return FALSE;
    }
  }

  format.dwSize = sizeof(DDPIXELFORMAT);

  if (DD_Primary->GetPixelFormat(&format) != DD_OK)
  {
    MessageBox(NULL, "IDirectDrawSurface7::GetPixelFormat failed.", "DirectDraw Error",
               MB_ICONERROR);
    return FALSE;
  }

  BitDepth = format.dwRGBBitCount;
  GBitMask = format.dwGBitMask; // 0x07E0 or not

  if (BitDepth == 24)
  {
    MessageBox(NULL,
               "ZSNESw does not support 24bit color.\nPlease change your resolution to either 16bit or 32bit color",
               "Error", MB_OK);
    exit(0);
  }

  converta = (BitDepth == 16 && GBitMask != 0x07E0);

  ddsd2.dwSize = sizeof(ddsd2);
  ddsd2.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
  ddsd2.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
  ddsd2.dwWidth = SurfaceX;
  ddsd2.dwHeight = SurfaceY;

  // create drawing surface
  if (lpDD->CreateSurface(&ddsd2, &DD_CFB, NULL) != DD_OK)
  {
    MessageBox(NULL, "IDirectDraw7::CreateSurface failed.", "DirectDraw Error", MB_ICONERROR);
    return FALSE;
  }

  AltSurface = 0;

  // create alt. drawing surface
  if (BitDepth == 32)
  {
    if (DMode == 1 && HQMode == 0)
    {
      ddsd2.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
    }
    ddsd2.dwFlags |= DDSD_PIXELFORMAT;
    ddsd2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    ddsd2.ddpfPixelFormat.dwFlags = DDPF_RGB;
    ddsd2.ddpfPixelFormat.dwRGBBitCount = 16;
    ddsd2.ddpfPixelFormat.dwRBitMask = 0xF800;
    ddsd2.ddpfPixelFormat.dwGBitMask = 0x07E0;
    ddsd2.ddpfPixelFormat.dwBBitMask = 0x001F;

    if (lpDD->CreateSurface(&ddsd2, &DD_CFB16, NULL) != DD_OK)
    {
      MessageBox(NULL,
                 "IDirectDraw7::CreateSurface failed. You should update your video card drivers. Alternatively, you could use a 16-bit desktop or use a non-D mode.",
                 "DirectDraw Error", MB_ICONERROR);
      return FALSE;
    }

    if (((SurfaceX == 512) || (SurfaceX == 602) || (SurfaceX == 640)) && (HQMode == 0))
    {
      AltSurface = 1;
    }
  }

  return TRUE;
}

void ReleaseDirectDraw()
{
  if (DD_CFB)
  {
    DD_CFB->Release();
    DD_CFB = NULL;
  }

  if (DD_CFB16)
  {
    DD_CFB16->Release();
    DD_CFB16 = NULL;
  }

  if (lpDDClipper)
  {
    lpDDClipper->Release();
    lpDDClipper = NULL;
  }

  if (DD_Primary)
  {
    DD_Primary->Release();
    DD_Primary = NULL;
  }

  if (lpDD)
  {
    lpDD->Release();
    lpDD = NULL;
  }
}

void clear_ddraw()
{
    if (FullScreen == 1)
    {
      DDBLTFX ddbltfx;

      ddbltfx.dwSize = sizeof(ddbltfx);
      ddbltfx.dwFillColor = 0;

      if (TripleBufferWin == 1)
      {
        if ((DD_Primary != NULL) && (DD_BackBuffer != NULL))
        {
          if (DD_BackBuffer->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx) ==
              DDERR_SURFACELOST)
          {
            DD_Primary->Restore();
          }

          if (DD_Primary->Flip(NULL, DDFLIP_WAIT) == DDERR_SURFACELOST)
          {
            DD_Primary->Restore();
          }

          if (DD_BackBuffer->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx) ==
              DDERR_SURFACELOST)
          {
            DD_Primary->Restore();
          }

          if (DD_Primary->Flip(NULL, DDFLIP_WAIT) == DDERR_SURFACELOST)
          {
            DD_Primary->Restore();
          }

          if (DD_BackBuffer->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx) ==
              DDERR_SURFACELOST)
          {
            DD_Primary->Restore();
          }
        }
      }
      else
      {
        if (DD_Primary != NULL)
        {
          if (vsyncon == 1)
          {
            if (lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL) != DD_OK)
            {
              DDrawError();
            }
          }
          if (DD_Primary->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx) ==
              DDERR_SURFACELOST)
          {
            DD_Primary->Restore();
          }
        }
      }
    }
}
