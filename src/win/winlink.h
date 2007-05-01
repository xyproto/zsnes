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

#ifndef WINLINK_H
#define WINLINK_H

typedef HRESULT (WINAPI* lpDirectDrawCreateEx)(GUID FAR *lpGuid, LPVOID *lplpDD, REFIID  iid,
                                               IUnknown FAR *pUnkOuter);

#ifdef __cplusplus
extern "C" {
#endif
  extern BYTE changeRes;
  extern DWORD converta;
  extern unsigned int BitConv32Ptr;
  extern unsigned int RGBtoYUVPtr;
  extern unsigned short resolutn;
  extern BYTE PrevRes;
  extern BYTE GUIWFVID[];
  extern BYTE GUIDSIZE[];
  extern BYTE GUISMODE[];
  extern BYTE GUIDSMODE[];
  extern BYTE GUIHQ2X[];
  extern BYTE GUIHQ3X[];
  extern BYTE GUIHQ4X[];
  extern BYTE GUINTVID[];
  extern BYTE hqFilterlevel;
  extern WORD totlines;
  extern DWORD CurMode;
  extern DWORD WindowWidth;
  extern DWORD WindowHeight;
  extern DWORD SurfaceX;
  extern DWORD SurfaceY;
  extern BYTE BitDepth;
  extern DWORD GBitMask;
  extern WORD Refresh;
  extern DWORD FirstVid;
  extern DWORD FirstFull;
  extern DWORD DMode;
  extern DWORD SMode;
  extern DWORD DSMode;
  extern DWORD NTSCMode;
  extern DWORD prevHQMode;
  extern DWORD prevNTSCMode;
  extern DWORD prevScanlines;
  extern HWND hMainWindow;
  extern BYTE curblank;
  extern WORD totlines;
  extern DWORD FullScreen;
  extern RECT rcWindow;
  extern RECT BlitArea;
  extern BYTE AltSurface;
  extern lpDirectDrawCreateEx pDirectDrawCreateEx;
  extern BYTE *SurfBuf;
  extern int X;
  extern DWORD newmode;
  extern WINDOWPLACEMENT wndpl;
  extern RECT rc1;

  void Clear2xSaIBuffer();
  void drawscreenwin();
  void clear_display();
  char CheckTVRatioReq();
  void KeepTVRatio();

  void CheckAlwaysOnTop();

#ifdef __cplusplus
}

BOOL ReInitSound();
void ReleaseDirectDraw();
#endif

#endif
