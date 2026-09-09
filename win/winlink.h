#ifndef WINLINK_H
#define WINLINK_H

#include <windows.h>

typedef HRESULT(WINAPI* lpDirectDrawCreateEx)(GUID FAR* lpGuid, LPVOID* lplpDD, REFIID iid,
    IUnknown FAR* pUnkOuter);

#ifdef __cplusplus
extern "C" {
#endif
extern BYTE changeRes;
extern unsigned char* BitConv32Ptr;
extern unsigned char* RGBtoYUVPtr;
extern unsigned short resolutn;
extern BYTE PrevRes;
extern BYTE hqFilterlevel;
extern WORD totlines;
extern DWORD CurMode;
extern DWORD WindowWidth;
extern DWORD WindowHeight;
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
extern BYTE* SurfBuf;
extern int X;
extern DWORD newmode;
extern WINDOWPLACEMENT wndpl;
extern RECT rc1;

void Clear2xSaIBuffer(void);
void FrameSemaphore(void);
void clear_display(void);
char CheckTVRatioReq(void);
void KeepTVRatio(void);

void CheckAlwaysOnTop(void);
void CheckPriority(void);
void CheckScreenSaver(void);
void DoSleep(void);
void MinimizeWindow(void);
void PasteClipBoard(void);
void SetMouseMaxX(int MaxX);
void SetMouseMaxY(int MaxY);
void SetMouseMinX(int MinX);
void SetMouseMinY(int MinY);
void SetMouseX(int X);
void SetMouseY(int Y);
void WinUpdateDevices(void);
void initDirectDraw(void);
void reInitSound(void);

extern BOOL ctrlptr;
extern char* CBBuffer;
extern u4 CBLength;

BOOL ReInitSound(void);
void ReleaseDirectDraw(void);
void DDDrawScreen(void);

#ifdef __cplusplus
}
#endif

#endif
