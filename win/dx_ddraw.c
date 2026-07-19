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

#define DIRECTINPUT_VERSION 0x0800
#define DIRECTSOUND_VERSION 0x0800
#define __STDC_CONSTANT_MACROS
#define COBJMACROS

#include <ddraw.h>
#include <stdio.h>
#include <windows.h>

#include "../c_intrf.h"
#include "../cfg.h"
#include "../intrf.h"
#include "../link.h"
#include "winlink.h"

void zexit(void);
void zexit_error(void);

static LPDIRECTDRAW BasiclpDD = NULL;
static LPDIRECTDRAW7 lpDD = NULL;
static LPDIRECTDRAWSURFACE7 DD_Primary = NULL;
static LPDIRECTDRAWSURFACE7 DD_CFB = NULL;
static LPDIRECTDRAWSURFACE7 DD_CFB16 = NULL;
static LPDIRECTDRAWSURFACE7 DD_BackBuffer = NULL;
static LPDIRECTDRAWCLIPPER lpDDClipper = NULL;

DDSURFACEDESC2 ddsd;

void DDrawError()
{
    char message1[256];

    strcpy(message1,
        "Error drawing to the screen\nMake sure the device is not being used by another process");
    MessageBox(NULL, message1, "DirectDraw Error", MB_ICONERROR);
}

// Letterbox src into the real fullscreen surface (wine ignores SetDisplayMode).
static void resolve_dest_rect(LPDIRECTDRAWSURFACE7 dst_surf,
    const RECT* src_rect, RECT* out)
{
    *out = rcWindow;
    if (FullScreen != 1 || !dst_surf || !src_rect) {
        return;
    }
    DDSURFACEDESC2 desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.dwSize = sizeof(desc);
    if (IDirectDrawSurface7_GetSurfaceDesc(dst_surf, &desc) != DD_OK) {
        return;
    }
    LONG dw = (LONG)desc.dwWidth;
    LONG dh = (LONG)desc.dwHeight;
    if (dw <= 0 || dh <= 0) {
        return;
    }
    LONG cw = out->right - out->left;
    LONG ch = out->bottom - out->top;
    if (cw == dw && ch == dh && out->left == 0 && out->top == 0) {
        return;
    }
    LONG sw = src_rect->right - src_rect->left;
    LONG sh = src_rect->bottom - src_rect->top;
    if (sw <= 0 || sh <= 0) {
        return;
    }
    // Fit preserving aspect.
    LONG fit_w = dw;
    LONG fit_h = (LONG)(((LONGLONG)sh * dw) / sw);
    if (fit_h > dh) {
        fit_h = dh;
        fit_w = (LONG)(((LONGLONG)sw * dh) / sh);
    }
    out->left = (dw - fit_w) / 2;
    out->top = (dh - fit_h) / 2;
    out->right = out->left + fit_w;
    out->bottom = out->top + fit_h;
}

void DDDrawScreen()
{
    RECT dst;
    if (FullScreen == 1) {
        if (TripleBufferWin == 1 || KitchenSync == 1 || (KitchenSyncPAL == 1 && totlines == 314)) {
            resolve_dest_rect(DD_BackBuffer, &BlitArea, &dst);
            if (IDirectDrawSurface7_Blt(DD_BackBuffer, &dst, DD_CFB, &BlitArea, DDBLT_WAIT, NULL) == DDERR_SURFACELOST) {
                IDirectDrawSurface7_Restore(DD_Primary);
            }

            if (IDirectDrawSurface7_Flip(DD_Primary, NULL, DDFLIP_WAIT) == DDERR_SURFACELOST) {
                IDirectDrawSurface7_Restore(DD_Primary);
            }

            if (KitchenSync == 1 || (KitchenSyncPAL == 1 && totlines == 314)) {
                resolve_dest_rect(DD_BackBuffer, &BlitArea, &dst);
                if (IDirectDrawSurface7_Blt(DD_BackBuffer, &dst, DD_CFB, &BlitArea, DDBLT_WAIT, NULL) == DDERR_SURFACELOST) {
                    IDirectDrawSurface7_Restore(DD_Primary);
                }

                if (IDirectDrawSurface7_Flip(DD_Primary, NULL, DDFLIP_WAIT) == DDERR_SURFACELOST) {
                    IDirectDrawSurface7_Restore(DD_Primary);
                }
            }
        } else {
            if (vsyncon == 1 && curblank != 0x40) {
                if (IDirectDraw7_WaitForVerticalBlank(lpDD, DDWAITVB_BLOCKBEGIN, NULL) != DD_OK) {
                    DDrawError();
                }
            }
            resolve_dest_rect(DD_Primary, &BlitArea, &dst);
            IDirectDrawSurface7_Blt(DD_Primary, &dst, DD_CFB, &BlitArea, DDBLT_WAIT, NULL);
            IDirectDrawSurface7_Restore(DD_Primary);
        }
    } else {
        if (vsyncon == 1) {
            if (IDirectDraw7_WaitForVerticalBlank(lpDD, DDWAITVB_BLOCKBEGIN, NULL) != DD_OK) {
                DDrawError();
            }
        }
        // Windowed: track the window and clip to the screen; wine rejects
        // dest rects that reach outside the primary surface.
        RECT src = BlitArea;
        GetClientRect(hMainWindow, &rcWindow);
        ClientToScreen(hMainWindow, (LPPOINT)&rcWindow);
        ClientToScreen(hMainWindow, (LPPOINT)&rcWindow + 1);
        dst = rcWindow;
        LONG dw = dst.right - dst.left;
        LONG dh = dst.bottom - dst.top;
        LONG sw = src.right - src.left;
        LONG sh = src.bottom - src.top;
        if (dw > 0 && dh > 0 && sw > 0 && sh > 0) {
            LONG scr_w = GetSystemMetrics(SM_CXSCREEN);
            LONG scr_h = GetSystemMetrics(SM_CYSCREEN);
            if (dst.left < 0) {
                src.left += (-dst.left * sw) / dw;
                dst.left = 0;
            }
            if (dst.top < 0) {
                src.top += (-dst.top * sh) / dh;
                dst.top = 0;
            }
            if (dst.right > scr_w) {
                src.right -= ((dst.right - scr_w) * sw) / dw;
                dst.right = scr_w;
            }
            if (dst.bottom > scr_h) {
                src.bottom -= ((dst.bottom - scr_h) * sh) / dh;
                dst.bottom = scr_h;
            }
            if (dst.left < dst.right && dst.top < dst.bottom && src.left < src.right && src.top < src.bottom) {
                HRESULT br = IDirectDrawSurface7_Blt(DD_Primary, &dst, AltSurface == 0 ? DD_CFB : DD_CFB16, &src, DDBLT_WAIT, NULL);
                static int zn;
                if (++zn % 64 == 1)
                    fprintf(stderr, "ZSDBG blt#%d=%08lx dst=(%ld,%ld,%ld,%ld) src=(%ld,%ld,%ld,%ld)\n", zn, (unsigned long)br, dst.left, dst.top, dst.right, dst.bottom, src.left, src.top, src.right, src.bottom);
            } else {
                static int ze;
                if (++ze % 64 == 1)
                    fprintf(stderr, "ZSDBG empty rect#%d\n", ze);
            }
        }
    }
}

DWORD LockSurface()
{
    HRESULT hRes;

    if (AltSurface == 0) {
        if (DD_CFB != NULL) {
            memset(&ddsd, 0, sizeof(ddsd));
            ddsd.dwSize = sizeof(ddsd);
            ddsd.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;

            hRes = IDirectDrawSurface7_Lock(DD_CFB, NULL, &ddsd, DDLOCK_WAIT, NULL);

            if (hRes == DD_OK) {
                SurfBuf = (BYTE*)ddsd.lpSurface;
                return (ddsd.lPitch);
            } else {
                if (hRes == DDERR_SURFACELOST) {
                    IDirectDrawSurface7_Restore(DD_Primary);
                    IDirectDrawSurface7_Restore(DD_CFB);
                    Clear2xSaIBuffer();
                }
                return (0);
            }
        } else {
            return (0);
        }
    } else {
        if (DD_CFB16 != NULL) {
            memset(&ddsd, 0, sizeof(ddsd));
            ddsd.dwSize = sizeof(ddsd);
            ddsd.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;

            hRes = IDirectDrawSurface7_Lock(DD_CFB16, NULL, &ddsd, DDLOCK_WAIT, NULL);

            if (hRes == DD_OK) {
                SurfBuf = (BYTE*)ddsd.lpSurface;
                return (ddsd.lPitch);
            } else {
                if (hRes == DDERR_SURFACELOST) {
                    IDirectDrawSurface7_Restore(DD_Primary);
                    IDirectDrawSurface7_Restore(DD_CFB16);
                    Clear2xSaIBuffer();
                }
                return (0);
            }
        } else {
            return (0);
        }
    }
}

void UnlockSurface()
{
    if (AltSurface == 0) {
        IDirectDrawSurface7_Unlock(DD_CFB, (struct tagRECT*)ddsd.lpSurface);
    } else {
        IDirectDrawSurface7_Unlock(DD_CFB16, (struct tagRECT*)ddsd.lpSurface);
    }
}

int InitDirectDraw()
{
    DDSURFACEDESC2 ddsd2;
    DDPIXELFORMAT format;

    unsigned int color32, ScreenPtr2;
    int i, j, k, r, g, b, Y, u, v;

    ScreenPtr2 = BitConv32Ptr;
    for (i = 0; i < 65536; i++) {
        color32 = ((i & 0xF800) << 8) + ((i & 0x07E0) << 5) + ((i & 0x001F) << 3) + 0xFF000000;
        (*(unsigned int*)(ScreenPtr2)) = color32;
        ScreenPtr2 += 4;
    }

    for (i = 0; i < 32; i++) {
        for (j = 0; j < 64; j++) {
            for (k = 0; k < 32; k++) {
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

    if (!hMainWindow) {
        zexit_error();
    }

    ReleaseDirectDraw();

    GetClientRect(hMainWindow, &rcWindow);
    ClientToScreen(hMainWindow, (LPPOINT)&rcWindow);
    ClientToScreen(hMainWindow, (LPPOINT)&rcWindow + 1);

    FullScreen = GUIWFVID[cvidmode];
    DSMode = GUIDSMODE[cvidmode];

    DWORD HQMode = 0;

    if (hqFilter != 0) {
        if ((GUIHQ2X[cvidmode] != 0) && (hqFilterlevel == 2)) {
            HQMode = 2;
        }
        if ((GUIHQ3X[cvidmode] != 0) && (hqFilterlevel == 3)) {
            HQMode = 3;
        }
        if ((GUIHQ4X[cvidmode] != 0) && (hqFilterlevel == 4)) {
            HQMode = 4;
        }
    }

    BlitArea.top = 0;
    BlitArea.left = 0;
    BlitArea.right = SurfaceX;

    if (PrevRes == 0) {
        PrevRes = resolutn;
    }

    if (!FirstVid) {
        /*
    if (X<0)X=0;
    if (X>(int)(GetSystemMetrics(SM_CXSCREEN) - WindowWidth)) X=(GetSystemMetrics(SM_CXSCREEN) - WindowWidth);
    if (Y<0)Y=0;
    if (Y>(int)(GetSystemMetrics(SM_CYSCREEN) - WindowHeight)) Y=(GetSystemMetrics(SM_CYSCREEN) - WindowHeight);
    */

        if (FullScreen == 1) {
            X = 0;
            Y = 0;
        }

        if (FullScreen == 0 && newmode == 1) {
            X = MainWindowX;
            Y = MainWindowY;
        } else if (FullScreen == 0) {
            MainWindowX = X;
            MainWindowY = Y;
        }

        MoveWindow(hMainWindow, X, Y, WindowWidth, WindowHeight, TRUE);

        wndpl.length = sizeof(wndpl);
        GetWindowPlacement(hMainWindow, &wndpl);
        SetRect(&rc1, 0, 0, WindowWidth, WindowHeight);

        AdjustWindowRectEx(&rc1, GetWindowLong(hMainWindow, GWL_STYLE), GetMenu(hMainWindow) != NULL,
            GetWindowLong(hMainWindow, GWL_EXSTYLE));

        GetClientRect(hMainWindow, &rcWindow);
        ClientToScreen(hMainWindow, (LPPOINT)&rcWindow);
        ClientToScreen(hMainWindow, (LPPOINT)&rcWindow + 1);

        if (FullScreen == 1) {
            if (HQMode && !DSMode) {
                int marginx = (rcWindow.right - rcWindow.left - BlitArea.right + BlitArea.left) / 2;
                int marginy = (rcWindow.bottom - rcWindow.top - BlitArea.bottom + BlitArea.top) / 2;

                if (marginx > 0) {
                    rcWindow.left += marginx;
                    rcWindow.right -= marginx;
                }
                if (marginy > 0) {
                    rcWindow.top += marginy;
                    rcWindow.bottom -= marginy;
                }
            }

            if ((DSMode == 1) && (scanlines != 0)) {
                int OldHeight = rcWindow.bottom - rcWindow.top;
                if ((OldHeight % 240) == 0) {
                    int NewHeight = (OldHeight / 240) * resolutn;
                    rcWindow.top += (OldHeight - NewHeight) / 2;
                    rcWindow.bottom = rcWindow.top + NewHeight;
                    clear_display();
                }
            }
        }
        if ((SurfaceX == 602) || (SurfaceX == 640) || (SurfaceX == 320)) {
            BlitArea.bottom = SurfaceY;
        } else if (!NTSCFilter) {
            BlitArea.bottom = (SurfaceY / 240) * resolutn;
        }

        if (CheckTVRatioReq()) {
            KeepTVRatio();
        }
    }
    if (FullScreen == 1) {
        if (HQMode && !DSMode) {
            int marginx = (rcWindow.right - rcWindow.left - BlitArea.right + BlitArea.left) / 2;
            int marginy = (rcWindow.bottom - rcWindow.top - BlitArea.bottom + BlitArea.top) / 2;
            if (marginx > 0) {
                rcWindow.left += marginx;
                rcWindow.right -= marginx;
            }
            if (marginy > 0) {
                rcWindow.top += marginy;
                rcWindow.bottom -= marginy;
            }
        }

        if ((DSMode == 1) && (scanlines != 0)) {
            int OldHeight = rcWindow.bottom - rcWindow.top;
            if ((OldHeight % 240) == 0) {
                int NewHeight = (OldHeight / 240) * resolutn;
                rcWindow.top += (OldHeight - NewHeight) / 2;
                rcWindow.bottom = rcWindow.top + NewHeight;
            }
        }
    }

    fprintf(stderr, "ZSDBG init fs=%d mode=%d win=%dx%d surf=%dx%d res=%d\n", (int)FullScreen, (int)cvidmode, (int)WindowWidth, (int)WindowHeight, (int)SurfaceX, (int)SurfaceY, (int)resolutn);
    if (pDirectDrawCreateEx(NULL, (void**)&lpDD, &IID_IDirectDraw7, NULL) != DD_OK) {
        fprintf(stderr, "ZSDBG CreateEx failed\n");
        MessageBox(NULL, "DirectDrawCreateEx failed.", "DirectDraw Error", MB_ICONERROR);
        return FALSE;
    }

    if (FullScreen == 1) {
        if (IDirectDraw7_SetCooperativeLevel(lpDD, hMainWindow,
                DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE | DDSCL_ALLOWREBOOT)
            != DD_OK) {
            MessageBox(NULL, "IDirectDraw7::SetCooperativeLevel failed.", "DirectDraw Error",
                MB_ICONERROR);
            return FALSE;
        }
        if (IDirectDraw7_SetDisplayMode(lpDD, WindowWidth, WindowHeight, 16, Refresh, 0) != DD_OK) {
            // Mode switching can fail under wine; keep the current display
            // mode and letterbox into it instead of giving up.
            IDirectDraw7_SetDisplayMode(lpDD, WindowWidth, WindowHeight, 16, 0, 0);
            KitchenSync = 0;
            KitchenSyncPAL = 0;
            Refresh = 0;
        }
    } else {
        if (IDirectDraw7_SetCooperativeLevel(lpDD, hMainWindow, DDSCL_NORMAL) != DD_OK) {
            fprintf(stderr, "ZSDBG SetCoop windowed failed\n");
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

    if (FullScreen == 1) {
        ddsd2.dwFlags |= DDSD_BACKBUFFERCOUNT;
        ddsd2.dwBackBufferCount = 2;
        ddsd2.ddsCaps.dwCaps |= DDSCAPS_FLIP | DDSCAPS_COMPLEX;
    }

    HRESULT hRes = IDirectDraw7_CreateSurface(lpDD, &ddsd2, &DD_Primary, NULL);

    if (FullScreen == 1) {
        if ((hRes == DDERR_OUTOFMEMORY) || (hRes == DDERR_OUTOFVIDEOMEMORY)) {
            ddsd2.dwBackBufferCount = 1;
            hRes = IDirectDraw7_CreateSurface(lpDD, &ddsd2, &DD_Primary, NULL);
        }
    }

    if (hRes != DD_OK) {
        MessageBox(NULL, "IDirectDraw7::CreateSurface failed.", "DirectDraw Error", MB_ICONERROR);
        return FALSE;
    }

    if (FullScreen == 1) {
        ddsd2.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;
        if (IDirectDrawSurface7_GetAttachedSurface(DD_Primary, &ddsd2.ddsCaps, &DD_BackBuffer) != DD_OK) {
            MessageBox(NULL, "IDirectDrawSurface7::GetAttachedSurface failed.", "DirectDraw Error",
                MB_ICONERROR);
            return FALSE;
        }
    } else {
        if (IDirectDraw7_CreateClipper(lpDD, 0, &lpDDClipper, NULL) != DD_OK) {
            fprintf(stderr, "ZSDBG CreateClipper failed\n");
            IDirectDraw7_Release(lpDD);
            lpDD = NULL;
            return FALSE;
        }

        if (IDirectDrawClipper_SetHWnd(lpDDClipper, 0, hMainWindow) != DD_OK) {
            fprintf(stderr, "ZSDBG SetHWnd failed\n");
            IDirectDraw7_Release(lpDD);
            lpDD = NULL;
            return FALSE;
        }

        if (IDirectDrawSurface7_SetClipper(DD_Primary, lpDDClipper) != DD_OK) {
            fprintf(stderr, "ZSDBG SetClipper failed\n");
            return FALSE;
        }
    }

    format.dwSize = sizeof(DDPIXELFORMAT);

    if (IDirectDrawSurface7_GetPixelFormat(DD_Primary, &format) != DD_OK) {
        MessageBox(NULL, "IDirectDrawSurface7::GetPixelFormat failed.", "DirectDraw Error",
            MB_ICONERROR);
        return FALSE;
    }

    BitDepth = format.dwRGBBitCount;
    GBitMask = format.dwGBitMask; // 0x07E0 or not

    if (BitDepth == 24) {
        MessageBox(
            NULL,
            "ZSNESw does not support 24bit color.\nPlease change your resolution to either 16bit or 32bit color",
            "Error", MB_OK);
        zexit_error();
    }

    converta = (BitDepth == 16 && GBitMask != 0x07E0);

    ddsd2.dwSize = sizeof(ddsd2);
    ddsd2.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    ddsd2.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    ddsd2.dwWidth = SurfaceX;
    ddsd2.dwHeight = SurfaceY;

    // create drawing surface
    if (IDirectDraw7_CreateSurface(lpDD, &ddsd2, &DD_CFB, NULL) != DD_OK) {
        fprintf(stderr, "ZSDBG CFB CreateSurface failed %ux%u\n", (unsigned)ddsd2.dwWidth, (unsigned)ddsd2.dwHeight);
        MessageBox(NULL, "IDirectDraw7::CreateSurface failed.", "DirectDraw Error", MB_ICONERROR);
        return FALSE;
    }

    AltSurface = 0;

    // create alt. drawing surface
    if (BitDepth == 32) {
        if (DMode == 1 && HQMode == 0) {
            ddsd2.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
        }
        ddsd2.dwFlags |= DDSD_PIXELFORMAT;
        ddsd2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        ddsd2.ddpfPixelFormat.dwFlags = DDPF_RGB;
        ddsd2.ddpfPixelFormat.dwRGBBitCount = 16;
        ddsd2.ddpfPixelFormat.dwRBitMask = 0xF800;
        ddsd2.ddpfPixelFormat.dwGBitMask = 0x07E0;
        ddsd2.ddpfPixelFormat.dwBBitMask = 0x001F;

        if (IDirectDraw7_CreateSurface(lpDD, &ddsd2, &DD_CFB16, NULL) != DD_OK) {
            MessageBox(
                NULL,
                "IDirectDraw7::CreateSurface failed. You should update your video card drivers. Alternatively, you could use a 16-bit desktop or use a non-D mode.",
                "DirectDraw Error", MB_ICONERROR);
            return FALSE;
        }

        if (((SurfaceX == 512) || (SurfaceX == 602) || (SurfaceX == 640)) && (HQMode == 0)) {
            AltSurface = 1;
        }
    }

    fprintf(stderr, "ZSDBG init OK depth=%d alt=%d\n", (int)BitDepth, (int)AltSurface);
    return TRUE;
}

void ReleaseDirectDraw()
{
    if (DD_CFB) {
        IDirectDrawSurface7_Release(DD_CFB);
        DD_CFB = NULL;
    }

    if (DD_CFB16) {
        IDirectDrawSurface7_Release(DD_CFB16);
        DD_CFB16 = NULL;
    }

    if (lpDDClipper) {
        IDirectDrawClipper_Release(lpDDClipper);
        lpDDClipper = NULL;
    }

    if (DD_Primary) {
        IDirectDrawSurface7_Release(DD_Primary);
        DD_Primary = NULL;
    }

    if (lpDD) {
        IDirectDraw7_Release(lpDD);
        lpDD = NULL;
    }
}

void clear_ddraw()
{
    if (FullScreen == 1) {
        DDBLTFX ddbltfx;

        ddbltfx.dwSize = sizeof(ddbltfx);
        ddbltfx.dwFillColor = 0;

        if (TripleBufferWin == 1) {
            if ((DD_Primary != NULL) && (DD_BackBuffer != NULL)) {
                if (IDirectDrawSurface7_Blt(DD_BackBuffer, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx) == DDERR_SURFACELOST) {
                    IDirectDrawSurface7_Restore(DD_Primary);
                }

                if (IDirectDrawSurface7_Flip(DD_Primary, NULL, DDFLIP_WAIT) == DDERR_SURFACELOST) {
                    IDirectDrawSurface7_Restore(DD_Primary);
                }

                if (IDirectDrawSurface7_Blt(DD_BackBuffer, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx) == DDERR_SURFACELOST) {
                    IDirectDrawSurface7_Restore(DD_Primary);
                }

                if (IDirectDrawSurface7_Flip(DD_Primary, NULL, DDFLIP_WAIT) == DDERR_SURFACELOST) {
                    IDirectDrawSurface7_Restore(DD_Primary);
                }

                if (IDirectDrawSurface7_Blt(DD_BackBuffer, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx) == DDERR_SURFACELOST) {
                    IDirectDrawSurface7_Restore(DD_Primary);
                }
            }
        } else {
            if (DD_Primary != NULL) {
                if (vsyncon == 1) {
                    if (IDirectDraw7_WaitForVerticalBlank(lpDD, DDWAITVB_BLOCKBEGIN, NULL) != DD_OK) {
                        DDrawError();
                    }
                }
                if (IDirectDrawSurface7_Blt(DD_Primary, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx) == DDERR_SURFACELOST) {
                    IDirectDrawSurface7_Restore(DD_Primary);
                }
            }
        }
    }
}
