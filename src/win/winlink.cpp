//Copyright (C) 1997-2001 ZSNES Team ( zsknight@zsnes.com / _demo_@zsnes.com )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later
//version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#define DIRECTINPUT_VERSION 0x0800
#define DIRECTSOUND_VERSION 0x0800

extern "C" {
   #include <windows.h>
   #include <stdio.h>
   #include <ddraw.h> 
   #include <initguid.h>
   #include <mmsystem.h>
   #include <time.h>
}
#include <math.h>
#include <dsound.h>
#include <dinput.h>
#include <winuser.h>
#include "resource.h"
#include <fstream.h>

DWORD WindowWidth = 256;
DWORD WindowHeight = 224;

DWORD FullScreen=0;
DWORD Moving=0;
DWORD SoundBufferSize=1024*18;
DWORD FirstSound=1;

int SoundEnabled=1;

DWORD FirstActivate = 1;

#define BYTE   unsigned char
#define WORD   unsigned short
#define DWORD  unsigned long

HWND hMainWindow;

extern "C"
{
HINSTANCE hInst;
}

LPDIRECTSOUND8          lpDirectSound = NULL;
LPDIRECTSOUNDBUFFER8    lpSoundBuffer = NULL;
LPDIRECTSOUNDBUFFER     lpPrimaryBuffer = NULL;
DSBUFFERDESC dsbd;

LPVOID lpvPtr1;
DWORD dwBytes1;
LPVOID lpvPtr2;
DWORD dwBytes2;

LPDIRECTDRAW            BasiclpDD = NULL;

LPDIRECTDRAW7           lpDD = NULL;
LPDIRECTDRAWSURFACE7    DD_Primary = NULL;
LPDIRECTDRAWSURFACE7    DD_CFB = NULL;
LPDIRECTDRAWSURFACE7    DD_CFB16 = NULL;
LPDIRECTDRAWSURFACE7    DD_BackBuffer = NULL;
LPDIRECTDRAWCLIPPER     lpDDClipper = NULL;
RECT                    rcWindow;

LPDIRECTINPUT8          DInput = NULL;
LPDIRECTINPUTDEVICE8    MouseInput = NULL;
LPDIRECTINPUTDEVICE8    KeyboardInput = NULL;
LPDIRECTINPUTDEVICE8    JoystickInput[5];
DIJOYSTATE js[5];

DWORD                   X1Disable[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
DWORD                   X2Disable[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
DWORD                   Y1Disable[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
DWORD                   Y2Disable[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
DWORD                   Z1Disable[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
DWORD                   Z2Disable[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
DWORD                   RX1Disable[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
DWORD                   RX2Disable[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
DWORD                   RY1Disable[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
DWORD                   RY2Disable[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
DWORD                   RZ1Disable[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
DWORD                   RZ2Disable[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
DWORD                   S01Disable[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
DWORD                   S02Disable[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
DWORD                   S11Disable[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
DWORD                   S12Disable[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
DWORD                   POVDisable[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
DWORD                   NumPOV[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
DWORD                   NumBTN[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

DWORD                   CurrentJoy=0;

DWORD                   BitDepth;
DWORD                   GBitMask;
BYTE                    BackColor=0;
DEVMODE                 mode;

float                   MouseMinX=0;
float                   MouseMaxX=256;
float                   MouseMinY=0;
float                   MouseMaxY=223;
float                   MouseX;
float                   MouseY;
float                   MouseMoveX;
float                   MouseMoveY;
int                     MouseMove2X;
int                     MouseMove2Y;
BYTE                    MouseButtonPressed;

BYTE                    IsActivated=1;

WORD                    PrevRes=0;
RECT                    BlitArea;
BYTE                    AltSurface=0;

extern "C" {
DWORD                   MouseButton;
DWORD                   SurfaceX=0;
DWORD                   SurfaceY=0;
VOID *blur_temp=0;
VOID *blur_buffer=0;
}

HANDLE hLock, hThread;
DWORD dwThreadId, dwThreadParam, semaphore_run;

extern "C" int SemaphoreMax = 5;

extern "C" void InitSemaphore();
extern "C" void ShutdownSemaphore();

static char dinput8_dll[] = {"dinput8.dll\0"};
static char dinput8_imp[] = {"DirectInput8Create\0"};

static char ddraw_dll[] = {"ddraw.dll\0"};
static char ddraw_imp[] = {"DirectDrawCreateEx\0"};

static char dsound_dll[] = {"dsound.dll\0"};
static char dsound_imp[] = {"DirectSoundCreate8\0"};

static HMODULE hM_ddraw = NULL, hM_dsound = NULL,hM_dinput8 = NULL;

typedef HRESULT (WINAPI* lpDirectInput8Create)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter);
static lpDirectInput8Create pDirectInput8Create;

typedef HRESULT (WINAPI* lpDirectDrawCreateEx)( GUID FAR * lpGuid, LPVOID  *lplpDD, REFIID  iid,IUnknown FAR *pUnkOuter );
static lpDirectDrawCreateEx pDirectDrawCreateEx;

typedef HRESULT (WINAPI* lpDirectSoundCreate8)(LPCGUID pcGuidDevice, LPDIRECTSOUND8 *ppDS8, LPUNKNOWN pUnkOuter);
static lpDirectSoundCreate8 pDirectSoundCreate8;

extern "C" void FreeDirectX()
{
   FreeLibrary(hM_dsound);
   FreeLibrary(hM_ddraw);
   FreeLibrary(hM_dinput8);
   exit(0);
}


extern "C" void ImportDirectX()
{
   hM_dinput8 = LoadLibrary(dinput8_dll);

   if (hM_dinput8 == NULL)
   {
      if (MessageBox(NULL, "Sorry, you need DirectX v8.0 or higher to use\nZSNESW. Would you like to go to the DirectX homepage?", "Error", MB_ICONINFORMATION | MB_YESNO) == IDYES)
         ShellExecute(NULL, NULL, "http://www.microsoft.com/directx/", NULL, NULL, 0);
      FreeDirectX();
   }

   pDirectInput8Create = (lpDirectInput8Create) GetProcAddress(hM_dinput8, dinput8_imp);

   if (pDirectInput8Create == NULL)
   {
      char err[256];
      wsprintf(err,"Failed to import %s:%s", dinput8_dll, dinput8_imp);
      MessageBox(NULL, err, "Error", MB_ICONERROR);
      FreeDirectX();
   }

   hM_ddraw = LoadLibrary(ddraw_dll);

   if (hM_ddraw == NULL)
   {
      char err[256];
      wsprintf(err,"Failed to import %s",ddraw_dll);
      MessageBox(NULL, err,"Error",MB_ICONERROR);
      FreeDirectX();
   }

   pDirectDrawCreateEx = (lpDirectDrawCreateEx) GetProcAddress(hM_ddraw, ddraw_imp);

   if (pDirectDrawCreateEx == NULL)
   {
      char err[256];
      wsprintf(err,"Failed to import %s:%s", ddraw_dll, ddraw_imp);
      MessageBox(NULL, err, "Error", MB_ICONERROR);
      FreeDirectX();
   }

   hM_dsound = LoadLibrary(dsound_dll);

   if (hM_dsound == NULL)
   {
      char err[256];
      wsprintf(err,"Failed to import %s",dsound_dll);
      MessageBox(NULL, err,"Error",MB_ICONERROR);
      FreeDirectX();
   }

   pDirectSoundCreate8 = (lpDirectSoundCreate8) GetProcAddress(hM_dsound, dsound_imp);

   if (pDirectSoundCreate8 == NULL)
   {
      char err[256];
      wsprintf(err,"Failed to import %s:%s", dsound_dll, dsound_imp);
      MessageBox(NULL, err, "Error", MB_ICONERROR);
      FreeDirectX();
   }
}

#define UPDATE_TICKS_GAME 1000.855001760297741789468390082/60      // milliseconds per world update
#define UPDATE_TICKS_GAMEPAL 1000/50   // milliseconds per world update
#define UPDATE_TICKS_GUI 1000/36       // milliseconds per world update
#define UPDATE_TICKS_UDP 1000/60       // milliseconds per world update

_int64 start, end, freq, update_ticks_pc, start2, end2, update_ticks_pc2;

void ReleaseDirectDraw();
void ReleaseDirectSound();
void ReleaseDirectInput();
int InitDirectDraw();
int ReInitSound();

extern "C"
{
   void drawscreenwin(void);
   DWORD LastUsedPos=0;
   DWORD CurMode=-1;
   void initDirectDraw()
   {
      InitDirectDraw();
   }
   void reInitSound()
   {
      ReInitSound();
   }
}

void DDrawError(){
   char message1[256];

   sprintf(message1,"Error drawing to the screen\nMake sure the device is not being used by another process \0");
   MessageBox (NULL, message1, "DirectDraw Error" , MB_ICONERROR );
}

extern "C" BYTE vsyncon;
extern "C" BYTE KitchenSync;

void DrawScreen()
{
  if (FullScreen == 1)
  {
    if ((DD_Primary != NULL) && (DD_BackBuffer != NULL))
    {
      if (DD_BackBuffer->Blt(&rcWindow, DD_CFB, &BlitArea, DDBLT_WAIT, NULL) == DDERR_SURFACELOST)
        DD_Primary->Restore();

      if (DD_Primary->Flip(NULL, DDFLIP_WAIT) == DDERR_SURFACELOST)
        DD_Primary->Restore();

      if (KitchenSync == 1)
      {
         if (DD_BackBuffer->Blt(&rcWindow, DD_CFB, &BlitArea, DDBLT_WAIT, NULL) == DDERR_SURFACELOST)
           DD_Primary->Restore();

         if (DD_Primary->Flip(NULL, DDFLIP_WAIT) == DDERR_SURFACELOST)
           DD_Primary->Restore();
      }

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
 
DWORD InputEn=0;

InputAcquire(void)
{
   if (JoystickInput[0]) JoystickInput[0]->Acquire();
   if (JoystickInput[1]) JoystickInput[1]->Acquire();
   if (JoystickInput[2]) JoystickInput[2]->Acquire();
   if (JoystickInput[3]) JoystickInput[3]->Acquire();
   if (JoystickInput[4]) JoystickInput[4]->Acquire();
   if (MouseInput) MouseInput->Acquire();
   if (KeyboardInput) KeyboardInput->Acquire();
   InputEn = 1;
   return TRUE;
}

BOOL InputDeAcquire(void)
{
   if (MouseInput) { MouseInput->Unacquire(); }
   if (KeyboardInput) KeyboardInput->Unacquire();
   if (JoystickInput[0]) JoystickInput[0]->Unacquire();
   if (JoystickInput[1]) JoystickInput[1]->Unacquire();
   if (JoystickInput[2]) JoystickInput[2]->Unacquire();
   if (JoystickInput[3]) JoystickInput[3]->Unacquire();
   if (JoystickInput[4]) JoystickInput[4]->Unacquire();
   InputEn = 0;
   return TRUE;
}

extern "C" {
void initwinvideo();
void DosExit(void);
extern BYTE GUIOn2;
extern BYTE cfgsoundon;
extern BYTE StereoSound;
extern DWORD SoundQuality;
extern BYTE HighPriority;
extern BYTE AlwaysOnTop;
extern BYTE SaveMainWindowPos;
extern BYTE AllowMultipleInst;
extern BYTE DisableScreenSaver;
extern BYTE TrapMouseCursor;
extern signed short int MainWindowX;
extern signed short int MainWindowY;
extern int CurKeyPos;
extern int CurKeyReadPos;
extern int KeyBuffer[16];
}

extern "C" void CheckPriority()
{
   if (HighPriority == 1) SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
      else SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
}

extern "C" void CheckAlwaysOnTop()
{
   if (AlwaysOnTop == 1) SetWindowPos(hMainWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
      else SetWindowPos(hMainWindow, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

extern "C" void CheckScreenSaver()
{
   if (DisableScreenSaver == 1 && IsActivated == 1) SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, FALSE, 0, SPIF_SENDCHANGE);
      else SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, TRUE, 0, SPIF_SENDCHANGE);
}

extern "C" void MinimizeWindow()
{
   ShowWindow(hMainWindow, SW_MINIMIZE);
   IsActivated = 0;
}

extern "C" BYTE MouseWheel;

BOOL InputRead(void)
{
   static PrevZ=0;
   MouseMoveX=0;
   MouseMoveY=0;
   if (MouseInput&&InputEn==1)
   {
      DIMOUSESTATE dims;
      HRESULT hr;
aquireagain:;
      hr=MouseInput->GetDeviceState(sizeof(DIMOUSESTATE),&dims);
		
      if (hr==DIERR_INPUTLOST)
      {
         hr=MouseInput->Acquire();
         if (SUCCEEDED(hr))
         {
            goto aquireagain;
         }
      }

      if (SUCCEEDED(hr))
      {
         MouseMoveX=dims.lX;
         MouseMoveY=dims.lY;

         if (MouseWheel == 1)
         {
            long zDelta = dims.lZ-PrevZ;
            if (!dims.lZ) zDelta=0;
            while (zDelta>0){
              zDelta-=40;
              if (!((CurKeyPos+1==CurKeyReadPos) || ((CurKeyPos+1==16)
                 && (CurKeyReadPos==0)))){
                 KeyBuffer[CurKeyPos]=72+256;
                 CurKeyPos++;
                 if (CurKeyPos==16) CurKeyPos=0;
              }
            }
            while (zDelta<0){
              zDelta+=40;
              if (!((CurKeyPos+1==CurKeyReadPos) || ((CurKeyPos+1==16)
                 && (CurKeyReadPos==0)))){
                 KeyBuffer[CurKeyPos]=80+256;
                 CurKeyPos++;
                 if (CurKeyPos==16) CurKeyPos=0;
              }
            }
            PrevZ=dims.lZ;
         }

         MouseButton=(dims.rgbButtons[0]>>7)|(dims.rgbButtons[1]>>6)|(dims.rgbButtons[2]>>5)|(dims.rgbButtons[3]>>4);
   }
   else
   {
      return FALSE;
   }

   }
	return TRUE;
}

extern "C" void SaveSramData(void);

void ExitFunction()
{
   if (GUIOn2 == 0)
   {
      _asm
      {
         pushad
         call SaveSramData
         popad
      }
   }
   IsActivated = 0;
   CheckScreenSaver();
   ReleaseDirectInput();
   ReleaseDirectSound();
   ReleaseDirectDraw();
   if (blur_temp) free(blur_temp);
   if (blur_buffer) free(blur_buffer);
   FreeLibrary(hM_dsound);
   FreeLibrary(hM_ddraw);
   FreeLibrary(hM_dinput8);
   DestroyWindow(hMainWindow);
}

LRESULT CALLBACK Main_Proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   static bool shiftpr;
   bool accept;
   int vkeyval;
   short zDelta;

	switch (uMsg)
   {
      case WM_KEYDOWN:        // sent when user presses a key
         if (!((CurKeyPos+1==CurKeyReadPos) || ((CurKeyPos+1==16)
            && (CurKeyReadPos==0)))){
            accept=false;

            if (wParam==16)
              shiftpr=true;
            if (((wParam>='A') && (wParam<='Z')) ||
                ((wParam>='a') && (wParam<='z')) || (wParam==27) ||
                (wParam==32) || (wParam==8) || (wParam==13) || (wParam==9)) {
              accept=true; vkeyval=wParam;
            }
            if ((wParam>='0') && (wParam<='9')) {
              accept=true; vkeyval=wParam;
              if (shiftpr) {
                switch (wParam) {
                  case '1': vkeyval='!'; break;
                  case '2': vkeyval='@'; break;
                  case '3': vkeyval='#'; break;
                  case '4': vkeyval='$'; break;
                  case '5': vkeyval='%'; break;
                  case '6': vkeyval='^'; break;
                  case '7': vkeyval='&'; break;
                  case '8': vkeyval='*'; break;
                  case '9': vkeyval='('; break;
                  case '0': vkeyval=')'; break;
                }
              }
            }
            if ((wParam>=VK_NUMPAD0) && (wParam<=VK_NUMPAD9)) {
              accept=true; vkeyval=wParam-VK_NUMPAD0+'0';
            }
            if (!shiftpr){
              switch (wParam) {
                case 189: vkeyval='-'; accept=true; break;
                case 187: vkeyval='='; accept=true; break;
                case 219: vkeyval='['; accept=true; break;
                case 221: vkeyval=']'; accept=true; break;
                case 186: vkeyval=';'; accept=true; break;
                case 222: vkeyval=39; accept=true; break;
                case 188: vkeyval=','; accept=true; break;
                case 190: vkeyval='.'; accept=true; break;
                case 191: vkeyval='/'; accept=true; break;
                case 192: vkeyval='`'; accept=true; break;
                case 220: vkeyval=92; accept=true; break;
              }
            } else {
              switch (wParam) {
                case 189: vkeyval='_'; accept=true; break;
                case 187: vkeyval='+'; accept=true; break;
                case 219: vkeyval='{'; accept=true; break;
                case 221: vkeyval='}'; accept=true; break;
                case 186: vkeyval=':'; accept=true; break;
                case 222: vkeyval='"'; accept=true; break;
                case 188: vkeyval='<'; accept=true; break;
                case 190: vkeyval='>'; accept=true; break;
                case 191: vkeyval='?'; accept=true; break;
                case 192: vkeyval='~'; accept=true; break;
                case 220: vkeyval='|'; accept=true; break;
              }
            }
            switch (wParam) {
              case 33: vkeyval=256+73; accept=true; break;
              case 38: vkeyval=256+72; accept=true; break;
              case 36: vkeyval=256+71; accept=true; break;
              case 39: vkeyval=256+77; accept=true; break;
              case 12: vkeyval=256+76; accept=true; break;
              case 37: vkeyval=256+75; accept=true; break;
              case 34: vkeyval=256+81; accept=true; break;
              case 40: vkeyval=256+80; accept=true; break;
              case 35: vkeyval=256+79; accept=true; break;
              case 107: vkeyval='+'; accept=true; break;
              case 109: vkeyval='-'; accept=true; break;
              case 106: vkeyval='*'; accept=true; break;
              case 111: vkeyval='/'; accept=true; break;
              case 110: vkeyval='.'; accept=true; break;
            }
            if (accept){
              KeyBuffer[CurKeyPos]=vkeyval;
              CurKeyPos++;
              if (CurKeyPos==16) CurKeyPos=0;
            }
         }
         break;
      case WM_KEYUP:          // sent when user releases a key
         if (wParam==16)
            shiftpr=false;
         break;
      case WM_MOUSEMOVE:
         if (MouseInput) MouseInput->Acquire();
         break;
      case WM_MOVE:
         break;
      case WM_PAINT:
         ValidateRect(hWnd,NULL);
         break;
      case WM_ACTIVATE: 
         if (LOWORD(wParam) != WA_INACTIVE)
         {
            IsActivated = 1;
            if (FirstActivate == 0) initwinvideo(); 
            InputAcquire();
            if (FirstActivate == 1) FirstActivate = 0;
            CheckPriority();
            CheckScreenSaver();
         }
         if (LOWORD(wParam) == WA_INACTIVE)
         {
            IsActivated = 0;
            InputDeAcquire();
            if (GUIOn2 == 1) SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
            CheckScreenSaver();
         }
         break;
      case WM_SETFOCUS:
         if (FullScreen == 0) ShowWindow(hMainWindow, SW_SHOWNORMAL);
         CheckPriority();
         CheckScreenSaver();
         InputAcquire();
         break;
      case WM_KILLFOCUS:
         InputDeAcquire();
         IsActivated = 0;
         if (GUIOn2 == 1) SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
         CheckScreenSaver();
         break;
      case WM_DESTROY:
         break;
      case WM_CLOSE:
         break;
      default:
         return DefWindowProc(hWnd,uMsg,wParam,lParam);
   }
	return 0;
}

int RegisterWinClass(void)
{
   if (AllowMultipleInst == 0)
   {
      HWND hFindWindow;
      hFindWindow = FindWindow("ZSNESWIN", NULL);

      if (hFindWindow != NULL)
      {
         ShowWindow(hFindWindow, SW_SHOWNORMAL);
         SetForegroundWindow(hFindWindow);
         DosExit();
      }
   }

   WNDCLASS wcl;

   wcl.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_NOCLOSE;
   wcl.cbClsExtra	= 0;
   wcl.cbWndExtra = 0;
   wcl.hIcon = LoadIcon(NULL,"ZSNESW.ICO");
   wcl.hCursor = NULL;
   wcl.hInstance = hInst;
   wcl.lpfnWndProc = (WNDPROC)Main_Proc;
   wcl.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
   wcl.lpszMenuName = NULL;
   wcl.lpszClassName = "ZSNESWIN";

   if (RegisterClass(&wcl) == 0) return FALSE;

   return TRUE;
}

BYTE PrevStereoSound;
DWORD PrevSoundQuality;

InitSound()
{
   WAVEFORMATEX wfx;

   if (cfgsoundon == 0) return FALSE;

   SoundEnabled = 0;

   PrevSoundQuality=SoundQuality;
   PrevStereoSound=StereoSound;

      if (DS_OK == pDirectSoundCreate8(NULL, &lpDirectSound,NULL))
	{
		lpDirectSound->Initialize(NULL);
		if (DS_OK != lpDirectSound->SetCooperativeLevel(hMainWindow, DSSCL_NORMAL))
		{
			if (DS_OK != lpDirectSound->SetCooperativeLevel(hMainWindow, DSSCL_EXCLUSIVE))
				return FALSE;
		}
	}
	else 
	{
		return FALSE;
	}

	wfx.wFormatTag = WAVE_FORMAT_PCM;

   switch (SoundQuality)
   {
      case 0:
         wfx.nSamplesPerSec = 8000;
         SoundBufferSize=1024*2;
         break;
      case 1:
         wfx.nSamplesPerSec = 11025;
         SoundBufferSize=1024*2;
         break;
      case 2:
         wfx.nSamplesPerSec = 22050;
         SoundBufferSize=1024*4;
         break;
      case 3:
         wfx.nSamplesPerSec = 44100;
         SoundBufferSize=1024*8;
         break;
      case 4:
         wfx.nSamplesPerSec = 16000;
         SoundBufferSize=1024*4;
         break;
      case 5:
         wfx.nSamplesPerSec = 32000;
         SoundBufferSize=1024*8;
         break;
      case 6:
         wfx.nSamplesPerSec = 48000;
         SoundBufferSize=1024*8;
         break;
      default:
         wfx.nSamplesPerSec = 11025;
         SoundBufferSize=1024*2;
   }

   if (StereoSound==1)
   {
      wfx.nChannels = 2;
      wfx.nBlockAlign = 4;
      SoundBufferSize*=2;
   }
   else
   {
      wfx.nChannels = 1;
      wfx.nBlockAlign = 2;
   }

   wfx.wBitsPerSample = 16;
   wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
   wfx.cbSize=0;
    
   memset(&dsbd, 0, sizeof(DSBUFFERDESC));
   dsbd.dwSize = sizeof(DSBUFFERDESC);
   dsbd.dwFlags = DSBCAPS_STICKYFOCUS; // | DSBCAPS_PRIMARYBUFFER;
   dsbd.dwBufferBytes = SoundBufferSize;
   dsbd.lpwfxFormat = &wfx;

   if (DS_OK == lpDirectSound->CreateSoundBuffer(&dsbd, &lpPrimaryBuffer, NULL))
	{
      if (DS_OK == lpPrimaryBuffer->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID *) &lpSoundBuffer))
      {
         if (DS_OK != lpSoundBuffer->Play(0,0,DSBPLAY_LOOPING))
         {
            return FALSE;
         }
         SoundEnabled=1;
         FirstSound=0;
         return TRUE;
      }
      else
      {
         return FALSE;
      }
   } 
   else 
   {
      return FALSE;
   }

}

ReInitSound()
{
   WAVEFORMATEX wfx;

   if (lpSoundBuffer)
   {
      lpSoundBuffer->Stop();
      lpSoundBuffer->Release();
      lpSoundBuffer = NULL;
   }

   if (cfgsoundon == 0)
   {
      SoundEnabled = 0;
      ReleaseDirectSound();
      return FALSE;
   }
   else if (SoundEnabled == 0)
      return InitSound();

   SoundEnabled = 0;

   PrevSoundQuality=SoundQuality;
   PrevStereoSound=StereoSound;

   wfx.wFormatTag = WAVE_FORMAT_PCM;

   switch (SoundQuality)
   {
      case 0:
         wfx.nSamplesPerSec = 8000;
         SoundBufferSize=1024*2;
         break;
      case 1:
         wfx.nSamplesPerSec = 11025;
         SoundBufferSize=1024*2;
         break;
      case 2:
         wfx.nSamplesPerSec = 22050;
         SoundBufferSize=1024*4;
         break;
      case 3:
         wfx.nSamplesPerSec = 44100;
         SoundBufferSize=1024*8;
         break;
      case 4:
         wfx.nSamplesPerSec = 16000;
         SoundBufferSize=1024*4;
         break;
      case 5:
         wfx.nSamplesPerSec = 32000;
         SoundBufferSize=1024*8;
         break;
      case 6:
         wfx.nSamplesPerSec = 48000;
         SoundBufferSize=1024*8;
         break;
      default:
         wfx.nSamplesPerSec = 11025;
         SoundBufferSize=1024*2;
  }

   if (StereoSound==1)
   {
      wfx.nChannels = 2;
      wfx.nBlockAlign = 4;
      SoundBufferSize*=2;
   }
   else
   {
      wfx.nChannels = 1;
      wfx.nBlockAlign = 2;
   }

   wfx.wBitsPerSample = 16;
   wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
   wfx.cbSize=0;
    
   memset(&dsbd, 0, sizeof(DSBUFFERDESC));
   dsbd.dwSize = sizeof(DSBUFFERDESC);
   dsbd.dwFlags = DSBCAPS_STICKYFOCUS;
   dsbd.dwBufferBytes = SoundBufferSize;
   dsbd.lpwfxFormat = &wfx;

   if (DS_OK == lpDirectSound->CreateSoundBuffer(&dsbd, &lpPrimaryBuffer, NULL))
	{
      if (DS_OK == lpPrimaryBuffer->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID *) &lpSoundBuffer))
      {
         if (DS_OK != lpSoundBuffer->Play(0,0,DSBPLAY_LOOPING ))
         {
            return FALSE;
         }
         SoundEnabled=1;
         LastUsedPos=0;
         return TRUE;
      }
      else
      {
         return FALSE;
      }
   } 
   else 
   {
      return FALSE;
   }

}

BOOL FAR PASCAL InitJoystickInput(LPCDIDEVICEINSTANCE pdinst, LPVOID pvRef)
{
   LPDIRECTINPUT8 pdi = (LPDIRECTINPUT8)pvRef;
   GUID DeviceGuid = pdinst->guidInstance;

   if (CurrentJoy>3)
      return DIENUM_CONTINUE;

   // Create the DirectInput joystick device.
   if (pdi->CreateDevice(DeviceGuid,&JoystickInput[CurrentJoy], NULL) != DI_OK)
   {
      return DIENUM_CONTINUE;
   }

   if (JoystickInput[CurrentJoy]->SetDataFormat(&c_dfDIJoystick) != DI_OK)
   {
      JoystickInput[CurrentJoy]->Release();
      return DIENUM_CONTINUE;
   }

   if (JoystickInput[CurrentJoy]->SetCooperativeLevel(hMainWindow, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND) != DI_OK)
   {
      JoystickInput[CurrentJoy]->Release();
      return DIENUM_CONTINUE;
   }

   DIPROPRANGE diprg;

   diprg.diph.dwSize = sizeof(diprg);
   diprg.diph.dwHeaderSize = sizeof(diprg.diph);
   diprg.diph.dwObj = DIJOFS_X;
   diprg.diph.dwHow = DIPH_BYOFFSET;
   diprg.lMin = -1000;
   diprg.lMax = +1000;

   if FAILED(JoystickInput[CurrentJoy]->SetProperty(DIPROP_RANGE, &diprg.diph))
   {
      X1Disable[CurrentJoy]=1;
      X2Disable[CurrentJoy]=1;
   }

   diprg.diph.dwObj = DIJOFS_Y;

   if FAILED(JoystickInput[CurrentJoy]->SetProperty(DIPROP_RANGE, &diprg.diph))
   {
      Y1Disable[CurrentJoy]=1;
      Y2Disable[CurrentJoy]=1;
   }

   diprg.diph.dwObj = DIJOFS_Z;
   if (FAILED(JoystickInput[CurrentJoy]->SetProperty(DIPROP_RANGE, &diprg.diph)))
   {
      Z1Disable[CurrentJoy]=1;
      Z2Disable[CurrentJoy]=1;
   }

   diprg.diph.dwObj = DIJOFS_RX;
   if (FAILED(JoystickInput[CurrentJoy]->SetProperty(DIPROP_RANGE, &diprg.diph)))
   {
      RX1Disable[CurrentJoy]=1;
      RX2Disable[CurrentJoy]=1;
   }

   diprg.diph.dwObj = DIJOFS_RY;
   if (FAILED(JoystickInput[CurrentJoy]->SetProperty(DIPROP_RANGE, &diprg.diph)))
   {
      RY1Disable[CurrentJoy]=1;
      RY2Disable[CurrentJoy]=1;
   }

   diprg.diph.dwObj = DIJOFS_RZ;
   if (FAILED(JoystickInput[CurrentJoy]->SetProperty(DIPROP_RANGE, &diprg.diph)))
   {
      RZ1Disable[CurrentJoy]=1;
      RZ2Disable[CurrentJoy]=1;
   }

   diprg.diph.dwObj = DIJOFS_SLIDER(0);
   if (FAILED(JoystickInput[CurrentJoy]->SetProperty(DIPROP_RANGE, &diprg.diph)))
   {
      S01Disable[CurrentJoy]=1;
      S02Disable[CurrentJoy]=1;
   }

   diprg.diph.dwObj = DIJOFS_SLIDER(1);
   if (FAILED(JoystickInput[CurrentJoy]->SetProperty(DIPROP_RANGE, &diprg.diph)))
   {
      S11Disable[CurrentJoy]=1;
      S12Disable[CurrentJoy]=1;
   }

   DIDEVCAPS didc;

   didc.dwSize = sizeof(DIDEVCAPS);

   if (JoystickInput[CurrentJoy]->GetCapabilities(&didc) != DI_OK)
   {
      JoystickInput[CurrentJoy]->Release();
      return DIENUM_CONTINUE;
   }

   if (didc.dwButtons <= 16)   
      NumBTN[CurrentJoy] = didc.dwButtons;
   else
      NumBTN[CurrentJoy] = 16;

   if (didc.dwPOVs)
      NumPOV[CurrentJoy] = didc.dwPOVs;
   else
      POVDisable[CurrentJoy] = 1;

   DIPROPDWORD dipdw;

   dipdw.diph.dwSize = sizeof(DIPROPDWORD);
   dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
   dipdw.diph.dwHow = DIPH_BYOFFSET;
   dipdw.dwData = 2500;
   dipdw.diph.dwObj = DIJOFS_X;
   JoystickInput[CurrentJoy]->SetProperty(DIPROP_DEADZONE, &dipdw.diph);

   dipdw.diph.dwObj = DIJOFS_Y;
   JoystickInput[CurrentJoy]->SetProperty(DIPROP_DEADZONE, &dipdw.diph);

   dipdw.diph.dwObj = DIJOFS_Z;
   JoystickInput[CurrentJoy]->SetProperty(DIPROP_DEADZONE, &dipdw.diph);

   dipdw.diph.dwObj = DIJOFS_RX;
   JoystickInput[CurrentJoy]->SetProperty(DIPROP_DEADZONE, &dipdw.diph);

   dipdw.diph.dwObj = DIJOFS_RY;
   JoystickInput[CurrentJoy]->SetProperty(DIPROP_DEADZONE, &dipdw.diph);

   dipdw.diph.dwObj = DIJOFS_RZ;
   JoystickInput[CurrentJoy]->SetProperty(DIPROP_DEADZONE, &dipdw.diph);

   dipdw.diph.dwObj = DIJOFS_SLIDER(0);
   JoystickInput[CurrentJoy]->SetProperty(DIPROP_DEADZONE, &dipdw.diph);

   dipdw.diph.dwObj = DIJOFS_SLIDER(1);
   JoystickInput[CurrentJoy]->SetProperty(DIPROP_DEADZONE, &dipdw.diph);

   dipdw.diph.dwSize = sizeof(DIPROPDWORD);
   dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
   dipdw.diph.dwHow = DIPH_DEVICE;
   dipdw.dwData = DIPROPAXISMODE_ABS;
   dipdw.diph.dwObj = 0;

   JoystickInput[CurrentJoy]->SetProperty(DIPROP_AXISMODE, &dipdw.diph);

   CurrentJoy+=1;

   return DIENUM_CONTINUE;
}

void ReleaseDirectInput()
{
   if (MouseInput)
   {
      MouseInput->Release();
      MouseInput = NULL;
   }

   if (KeyboardInput)
   {
      KeyboardInput->Release();
      KeyboardInput = NULL;
   }

   for (int i=0; i<5; i++)
      if (JoystickInput[i])
      {
         JoystickInput[i]->Release();
         JoystickInput[i] = NULL;
      }

   if (DInput)
   {
      DInput->Release();
      DInput = NULL;
   }

}

void ReleaseDirectSound()
{
   if (lpSoundBuffer)
   {
      lpSoundBuffer->Release();
      lpSoundBuffer = NULL;
   }

   if (lpPrimaryBuffer)
   {
      lpPrimaryBuffer->Release();
      lpPrimaryBuffer = NULL;
   }

   if (lpDirectSound)
   {
      lpDirectSound->Release();
      lpDirectSound = NULL;
   }
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

void DInputError(){
   char message1[256];

   sprintf(message1,"Error initializing DirectInput\nYou may need to install DirectX 8.0a or higher located at www.microsoft.com/directx \0");
   MessageBox (NULL, message1, "DirectInput Error" , MB_ICONERROR );
}

bool InitInput()
{
   char message1[256];
   HRESULT hr;

   if (FAILED(hr=pDirectInput8Create(hInst,DIRECTINPUT_VERSION,IID_IDirectInput8A,(void **) &DInput,NULL)))
   {
      sprintf(message1,"Error initializing DirectInput\nYou may need to install DirectX 8.0a or higher located at www.microsoft.com/directx \0");
      MessageBox (NULL, message1, "DirectInput Error" , MB_ICONERROR );

      switch (hr)
      {
      case DIERR_BETADIRECTINPUTVERSION:
         sprintf(message1,"Beta %X\n\0",hr);
         MessageBox (NULL, message1, "Init", MB_ICONERROR );
         break;
      case DIERR_INVALIDPARAM:
         sprintf(message1,"Invalid %X\n\0",hr);
         MessageBox (NULL, message1, "Init", MB_ICONERROR );
         break;
      case DIERR_OLDDIRECTINPUTVERSION:
         sprintf(message1,"OLDDIRECTINPUTVERSION %X\n\0",hr);
         MessageBox (NULL, message1, "Init", MB_ICONERROR );
         break;
      case DIERR_OUTOFMEMORY:
         sprintf(message1,"OUTOFMEMORY %X\n\0",hr);
         MessageBox (NULL, message1, "Init", MB_ICONERROR );
         break;
      default:
         sprintf(message1,"UNKNOWN %X\n\0",hr);
         MessageBox (NULL, message1, "Init", MB_ICONERROR );
         break;
      }
      return FALSE;
   }

   hr=DInput->CreateDevice(GUID_SysKeyboard, &KeyboardInput,NULL);
   if (FAILED(hr)) {DInputError();return FALSE;}

   hr=KeyboardInput->SetDataFormat(&c_dfDIKeyboard);
   if (FAILED(hr)) {DInputError();return FALSE;}
	
   hr=KeyboardInput->SetCooperativeLevel(hMainWindow,DISCL_NONEXCLUSIVE | DISCL_FOREGROUND );

   hr=DInput->CreateDevice(GUID_SysMouse, &MouseInput,NULL);
   if (FAILED(hr)) {DInputError();return FALSE;}

   hr=MouseInput->SetDataFormat(&c_dfDIMouse);
   if (FAILED(hr)) {DInputError();return FALSE;}
	
   hr=MouseInput->SetCooperativeLevel(hMainWindow,DISCL_EXCLUSIVE|DISCL_FOREGROUND);
   if (FAILED(hr)) {DInputError();return FALSE;}

   JoystickInput[0]=NULL;JoystickInput[1]=NULL;JoystickInput[2]=NULL;JoystickInput[3]=NULL;

   hr=DInput->EnumDevices(DI8DEVCLASS_GAMECTRL, InitJoystickInput,
                       DInput, DIEDFL_ATTACHEDONLY);

   if (FAILED(hr)) {DInputError(); return FALSE;}

   InputAcquire();

   return TRUE;

}

void TestJoy()
{
   int i;

   for(i=0;i<4;i++)
   {
      if (JoystickInput[i])
      {
         JoystickInput[i]->Poll();

         if (JoystickInput[i]->GetDeviceState(sizeof(DIJOYSTATE), &js[i])==DIERR_INPUTLOST)
         {
            if (JoystickInput[i]) JoystickInput[i]->Acquire();
            if (FAILED(JoystickInput[i]->GetDeviceState(sizeof(DIJOYSTATE), &js[i]))) return;
         }

         if (!X1Disable[i])
         {
            if (js[i].lX>0) X1Disable[i]=1;
         }

         if (!X2Disable[i])
         {
            if (js[i].lX<0) X2Disable[i]=1;
         }

         if (!Y1Disable[i])
         {
            if (js[i].lY>0) Y1Disable[i]=1;
         }

         if (!Y2Disable[i])
         {
            if (js[i].lY<0) Y2Disable[i]=1;
         }

         if (!Z1Disable[i])
         {
            if (js[i].lZ>0) Z1Disable[i]=1;
         }

         if (!Z2Disable[i])
         {
            if (js[i].lZ<0) Z2Disable[i]=1;
         }

         if (!RY1Disable[i])
         {
            if (js[i].lRy>0) RY1Disable[i]=1;
         }

         if (!RY2Disable[i])
         {
            if (js[i].lRy<0) RY2Disable[i]=1;
         }

         if (!RZ1Disable[i])
         {
            if (js[i].lRz>0) RZ1Disable[i]=1;
         }

         if (!RZ2Disable[i])
         {
            if (js[i].lRz<0) RZ2Disable[i]=1;
         }

         if (!S01Disable[i])
         {
            if (js[i].rglSlider[0]>0) S01Disable[i]=1;
         }

         if (!S02Disable[i])
         {
            if (js[i].rglSlider[0]<0) S02Disable[i]=1;
         }

         if (!S11Disable[i])
         {
            if (js[i].rglSlider[1]>0) S11Disable[i]=1;
         }

         if (!S12Disable[i])
         {
            if (js[i].rglSlider[1]<0) S12Disable[i]=1;
         }

      }
   } 

}

extern "C" DWORD converta;
extern "C" unsigned int BitConv32Ptr;
extern "C" unsigned int RGBtoYUVPtr;
extern "C" unsigned char cvidmode;
extern "C" unsigned char hq3xFilter;
DWORD FirstVid=1;
DWORD FirstFull=1;
DWORD SMode=0;
DWORD DSMode=0;
DWORD prevHQ3XMode=-1;
extern "C" BYTE GUIWFVID[];
extern "C" BYTE GUISMODE[];
extern "C" BYTE GUIDSMODE[];
extern "C" BYTE GUIHQ3X[];
int Refresh = 0;

int InitDirectDraw()
{
   DDSURFACEDESC2 ddsd2;
   DDPIXELFORMAT format;
   HRESULT hr;
   char message1[256];
   unsigned int color32,ScreenPtr2;
   int i, j, k, r, g, b, Y, u, v;

   ScreenPtr2=BitConv32Ptr;
   for(i=0;i<65536;i++)
   {
      color32=((i&0xF800)<<8)+
              ((i&0x07E0)<<5)+
              ((i&0x001F)<<3)+0xFF000000;
              (*(unsigned int *)(ScreenPtr2))=color32;
      ScreenPtr2+=4;
   }

   for (i=0; i<32; i++)
   for (j=0; j<64; j++)
   for (k=0; k<32; k++)
   {
     r = i << 3;
     g = j << 2;
     b = k << 3;
     Y = (r + g + b) >> 2;
     u = 128 + ((r - b) >> 2);
     v = 128 + ((-r + 2*g -b)>>3);
     *(((unsigned int *)RGBtoYUVPtr) + (i << 11) + (j << 5) + k) = (Y<<16) + (u<<8) + v;
   }

   if (!hMainWindow)
   {
      exit(1);
   }

   ReleaseDirectDraw();
   
   GetClientRect(hMainWindow, &rcWindow);
   ClientToScreen(hMainWindow, ( LPPOINT )&rcWindow);
   ClientToScreen(hMainWindow, ( LPPOINT )&rcWindow + 1);

   FullScreen=GUIWFVID[cvidmode];
   DSMode=GUIDSMODE[cvidmode];

   if (FullScreen == 1 && DSMode == 0)
   {
     if (SurfaceX == 768 && SurfaceY == 720)
     {
       int marginx = (rcWindow.right - rcWindow.left - BlitArea.right + BlitArea.left)/2;
       int marginy = (rcWindow.bottom - rcWindow.top - BlitArea.bottom + BlitArea.top)/2;
       if (marginx>0)
       {
         rcWindow.left += marginx;
         rcWindow.right -= marginx;
       }
       if (marginy>0)
       {
         rcWindow.top += marginy;
         rcWindow.bottom -= marginy;
       }
     }
   }
   
   if (pDirectDrawCreateEx(NULL, (void **)&lpDD, IID_IDirectDraw7, NULL) != DD_OK)
   {
      MessageBox(NULL, "DirectDrawCreateEx failed.", "DirectDraw Error", MB_ICONERROR);
      return FALSE;
   }

   if (FullScreen == 1)
   {
      if (lpDD->SetCooperativeLevel(hMainWindow, DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE | DDSCL_ALLOWREBOOT) != DD_OK)
      {
         MessageBox(NULL, "IDirectDraw7::SetCooperativeLevel failed.", "DirectDraw Error", MB_ICONERROR);
         return FALSE;
      }
      if (lpDD->SetDisplayMode(WindowWidth, WindowHeight, 16, Refresh, 0) != DD_OK)
      {
         MessageBox(NULL, "IDirectDraw7::SetDisplayMode failed.\nMake sure your video card supports this mode.", "DirectDraw Error", MB_ICONERROR);
         return FALSE;
      }
   }
   else
   {
      if (lpDD->SetCooperativeLevel(hMainWindow, DDSCL_NORMAL) != DD_OK)
      {
         MessageBox(NULL, "IDirectDraw7::SetCooperativeLevel failed.", "DirectDraw Error", MB_ICONERROR);
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
         MessageBox(NULL, "IDirectDrawSurface7::GetAttachedSurface failed.", "DirectDraw Error", MB_ICONERROR);
         return FALSE;
      }
   }
   else
   {
      if (lpDD->CreateClipper(0,&lpDDClipper,NULL) != DD_OK)
      {
         lpDD->Release();
         lpDD=NULL;
         return FALSE;
      }
      
      if (lpDDClipper->SetHWnd(0,hMainWindow) != DD_OK)
      {
         lpDD->Release();
         lpDD=NULL;
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
      MessageBox(NULL, "IDirectDrawSurface7::GetPixelFormat failed.", "DirectDraw Error", MB_ICONERROR);
      return FALSE;
   }
	
   BitDepth=format.dwRGBBitCount;
   GBitMask=format.dwGBitMask; // 0x07E0 or not

   if (BitDepth==24)
   {
      MessageBox(NULL,"ZSNESw does not support 24bit color.\nPlease change your resolution to either 16bit or 32bit color","Error",MB_OK);
      exit(0);
   }

   converta = (BitDepth==16 && GBitMask!=0x07E0);

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

   // create alt. drawing surface
   if ( BitDepth == 32 )
   {
     ddsd2.dwFlags |= DDSD_PIXELFORMAT;
     ddsd2.ddpfPixelFormat.dwSize        = sizeof(DDPIXELFORMAT);
     ddsd2.ddpfPixelFormat.dwFlags       = DDPF_RGB;
     ddsd2.ddpfPixelFormat.dwRGBBitCount = 16;
     ddsd2.ddpfPixelFormat.dwRBitMask    = 0xF800;
     ddsd2.ddpfPixelFormat.dwGBitMask    = 0x07E0;
     ddsd2.ddpfPixelFormat.dwBBitMask    = 0x001F;

     if (lpDD->CreateSurface(&ddsd2, &DD_CFB16, NULL) != DD_OK)
     {
        MessageBox(NULL, "IDirectDraw7::CreateSurface failed.", "DirectDraw Error", MB_ICONERROR);
        return FALSE;
     }
   }

      if (!blur_buffer) blur_buffer = malloc(SurfaceX * SurfaceY * (BitDepth == 16 ? 2 : 4));
	  else blur_buffer = realloc(blur_buffer, SurfaceX * SurfaceY * (BitDepth == 16 ? 2 : 4));
	  if (!blur_temp) blur_temp = malloc(SurfaceX * SurfaceY * (BitDepth == 16 ? 2 : 4));
	  else blur_temp = realloc(blur_temp, SurfaceX * SurfaceY * (BitDepth == 16 ? 2 : 4));

   return TRUE;
}

BYTE* SurfBuf;
DDSURFACEDESC2 ddsd;

DWORD LockSurface()
{
  HRESULT hRes;

  if (AltSurface == 0)
  {
    if (DD_CFB != NULL)
    {
      memset(&ddsd,0,sizeof(ddsd));
      ddsd.dwSize = sizeof( ddsd );
      ddsd.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;

      hRes = DD_CFB->Lock(NULL,&ddsd,DDLOCK_WAIT,NULL);

      if (hRes == DD_OK)
      {
        SurfBuf = (BYTE*)ddsd.lpSurface;
        return(ddsd.lPitch);
      }
      else
      {
        if (hRes == DDERR_SURFACELOST)
          DD_CFB->Restore();
        return(0);
      }
    }
    else
      return(0);
  }
  else
  {
    if (DD_CFB16 != NULL)
    {
      memset(&ddsd,0,sizeof(ddsd));
      ddsd.dwSize = sizeof( ddsd );
      ddsd.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;

      hRes = DD_CFB16->Lock(NULL,&ddsd,DDLOCK_WAIT,NULL);

      if (hRes == DD_OK)
      {
        SurfBuf = (BYTE*)ddsd.lpSurface;
        return(ddsd.lPitch);
      }
      else
      {
        if (hRes == DDERR_SURFACELOST)
          DD_CFB16->Restore();
        return(0);
      }
    }
    else
      return(0);
  }  
}

void UnlockSurface()
{
  if (AltSurface == 0)
    DD_CFB->Unlock((struct tagRECT *)ddsd.lpSurface);
  else
    DD_CFB16->Unlock((struct tagRECT *)ddsd.lpSurface);
}

extern "C" {

void WinUpdateDevices();

short Buffer[1800*2];

int X, Y;
DWORD pitch;
MSG msg;
DWORD SurfBufD;
int count, x,count2;
HRESULT hr;
int i;
short *Sound;
DWORD CurrentPos;
DWORD WritePos;
DWORD T60HZEnabled=0;
DWORD T36HZEnabled=0;

DWORD WINAPI SemaphoreThread( LPVOID lpParam )
{
   while(semaphore_run)
   {
      if (T60HZEnabled)
      {
         ReleaseSemaphore(hLock, 1, NULL);
         Sleep(1);
      }
      else
         Sleep(20);
   }
   return 0;
}

void InitSemaphore()
{
   if (hLock) return;

   hLock = CreateSemaphore(NULL, 1, SemaphoreMax, NULL);

   semaphore_run = 1;

   hThread = CreateThread(NULL, 0, SemaphoreThread, &dwThreadParam, 0, &dwThreadId);
}

void ShutdownSemaphore()
{
   if (!hLock) return;

   semaphore_run = 0;

   WaitForSingleObject(hThread, INFINITE);
   CloseHandle(hThread);

   CloseHandle(hLock);

   hLock = NULL;
}

extern unsigned int pressed;
extern unsigned char romispal;

void Start60HZ(void)
{
   update_ticks_pc2 = UPDATE_TICKS_UDP * freq / 1000;

   if (romispal==1)
   {
      update_ticks_pc = UPDATE_TICKS_GAMEPAL * freq / 1000;
   }
   else
   {
      update_ticks_pc = UPDATE_TICKS_GAME * freq / 1000;
   }

   QueryPerformanceCounter((LARGE_INTEGER*)&start);
   QueryPerformanceCounter((LARGE_INTEGER*)&start2);

   T36HZEnabled=0;
   T60HZEnabled=1;

   InitSemaphore();

}

void Stop60HZ(void)
{
   T60HZEnabled=0;
   ShutdownSemaphore();
}

void Start36HZ(void)
{
   update_ticks_pc2 = UPDATE_TICKS_UDP * freq / 1000;
   update_ticks_pc = UPDATE_TICKS_GUI * freq / 1000;

   QueryPerformanceCounter((LARGE_INTEGER*)&start);
   QueryPerformanceCounter((LARGE_INTEGER*)&start2);

   T60HZEnabled=0;
   T36HZEnabled=1;
}

void Stop36HZ(void)
{
   T36HZEnabled=0;
}

char WinMessage[256];
extern unsigned short resolutn;
void clearwin();
void clear_display();

char WinName[]={"ZSNESW\0"};

void initwinvideo(void)
{
   RECT zwindowrect;
   WINDOWPLACEMENT wndpl;
   RECT rc1, swrect;
   DWORD newmode=0;
   DWORD HQ3XMode=0;

   if ((GUIHQ3X[cvidmode]!=0) && (hq3xFilter!=0))
     HQ3XMode=1;

   if ((CurMode!=cvidmode) || (prevHQ3XMode!=HQ3XMode))
   {
      CurMode=cvidmode;
      prevHQ3XMode=HQ3XMode;
      newmode=1;
      SurfaceX=256;
      SurfaceY=240;
      X=0;
      Y=0;
      FullScreen=GUIWFVID[cvidmode];
      SMode=GUISMODE[cvidmode];
      DSMode=GUIDSMODE[cvidmode];

      switch (cvidmode)
      {
      case 0:
         WindowWidth=256;
         WindowHeight=224;
         break;
      case 1:
         WindowWidth=640;
         WindowHeight=480;
         SurfaceX=320;
         SurfaceY=240;
         break;
      case 2:
         WindowWidth=512;
         WindowHeight=448;
         break;
      case 3:
         WindowWidth=512;
         WindowHeight=448;
         SurfaceX=512;
         SurfaceY=480;
         break;
      case 4:
         WindowWidth=640;
         WindowHeight=480;
         break;
      case 5:
         WindowWidth=640;
         WindowHeight=480;
         SurfaceX=512;
         SurfaceY=448;
         break;
      case 6:
         WindowWidth=640;
         WindowHeight=480;
         SurfaceX=640;
         SurfaceY=480;
         break;
      case 7:
         WindowWidth=640;
         WindowHeight=480;
         SurfaceX=512;
         SurfaceY=448;
         break;
      case 8:
         WindowWidth=640;
         WindowHeight=480;
         break;
      case 9:
         WindowWidth=768;
         WindowHeight=672;
         break;
      case 10:
         WindowWidth=768;
         WindowHeight=672;
         SurfaceX=512;
         SurfaceY=480;
         break;
      case 11:
         WindowWidth=800;
         WindowHeight=600;
         break;
      case 12:
         WindowWidth=800;
         WindowHeight=600;
         SurfaceX=512;
         SurfaceY=448;
         break;
      case 13:
         WindowWidth=800;
         WindowHeight=600;
         break;
      case 14:
         WindowWidth=800;
         WindowHeight=600;
         SurfaceX=640;
         SurfaceY=480;
         break;
      case 15:
         WindowWidth=800;
         WindowHeight=600;
         SurfaceX=512;
         SurfaceY=448;
         break;
      case 16:
         WindowWidth=1024;
         WindowHeight=768;
         break;
      case 17:
         WindowWidth=1024;
         WindowHeight=768;
         SurfaceX=512;
         SurfaceY=448;
         break;
      case 18:
         WindowWidth=1024;
         WindowHeight=768;
         break;
      case 19:
         WindowWidth=1024;
         WindowHeight=768;
         SurfaceX=640;
         SurfaceY=480;
         break;
      case 20:
         WindowWidth=1024;
         WindowHeight=768;
         SurfaceX=512;
         SurfaceY=448;
         break;
      case 21:
         WindowWidth=1024;
         WindowHeight=896;
         break;
      case 22:
         WindowWidth=1024;
         WindowHeight=896;
         SurfaceX=512;
         SurfaceY=480;
         break;
      case 23:
         WindowWidth=1280;
         WindowHeight=960;
         break;
      case 24:
         WindowWidth=1280;
         WindowHeight=960;
         SurfaceX=512;
         SurfaceY=448;
         break;
      case 25:
         WindowWidth=1280;
         WindowHeight=960;
         break;
      case 26:
         WindowWidth=1280;
         WindowHeight=960;
         SurfaceX=640;
         SurfaceY=480;
         break;
      case 27:
         WindowWidth=1280;
         WindowHeight=960;
         SurfaceX=512;
         SurfaceY=448;
         break;
      case 28:
         WindowWidth=1280;
         WindowHeight=1024;
         break;
      case 29:
         WindowWidth=1280;
         WindowHeight=1024;
         SurfaceX=512;
         SurfaceY=448;
         break;
      case 30:
         WindowWidth=1280;
         WindowHeight=1024;
         break;
      case 31:
         WindowWidth=1280;
         WindowHeight=1024;
         SurfaceX=640;
         SurfaceY=480;
         break;
      case 32:
         WindowWidth=1280;
         WindowHeight=1024;
         SurfaceX=512;
         SurfaceY=448;
         break;
      default:
         WindowWidth=256;
         WindowHeight=224;
         break;
      }

      if (HQ3XMode!=0)
      {
        SurfaceX=768;
        SurfaceY=720;
      }

      BlitArea.top = 0;
      BlitArea.left = 0;
      BlitArea.right = SurfaceX;


     if (FullScreen == 0)
     {
        if (SurfaceX == 256) BlitArea.bottom = (SurfaceY/240)*resolutn;
        if (SurfaceX == 512)
        {
           BlitArea.bottom = (SurfaceY/240)*resolutn;
           if (DSMode == 1) BlitArea.bottom = SurfaceY;
        }
     }
     else
     {
        if (SMode == 1)
           BlitArea.bottom = (SurfaceY/240)*resolutn;
        else
           BlitArea.bottom = SurfaceY;
     }

     if (SurfaceX == 768) BlitArea.bottom = (SurfaceY/240)*resolutn;

     if (PrevRes == 0) PrevRes = resolutn;

   }

   if (((PrevStereoSound!=StereoSound)||(PrevSoundQuality!=SoundQuality))&&FirstSound!=1)
      ReInitSound();

   if (!FirstVid)
   {   
      if (X<0)X=0;
      if (X>(GetSystemMetrics(SM_CXSCREEN) - WindowWidth)) X=(GetSystemMetrics(SM_CXSCREEN) - WindowWidth);
      if (Y<0)Y=0;
      if (Y>(GetSystemMetrics(SM_CYSCREEN) - WindowHeight)) Y=(GetSystemMetrics(SM_CYSCREEN) - WindowHeight);

      if (FullScreen==1) {X=0; Y=0;}

      if (FullScreen==0 && newmode == 1) { X = MainWindowX; Y = MainWindowY; }
         else if (FullScreen==0) { MainWindowX = X; MainWindowY = Y; }

      MoveWindow(hMainWindow, X, Y, WindowWidth, WindowHeight, TRUE);

      wndpl.length = sizeof(wndpl);
      GetWindowPlacement(hMainWindow, &wndpl);
      SetRect(&rc1, 0, 0, WindowWidth, WindowHeight);

      AdjustWindowRectEx(&rc1,GetWindowLong(hMainWindow, GWL_STYLE),
      GetMenu(hMainWindow) != NULL, GetWindowLong(hMainWindow, GWL_EXSTYLE)); 
      
      GetClientRect(hMainWindow, &rcWindow);
      ClientToScreen(hMainWindow, (LPPOINT) &rcWindow);
      ClientToScreen(hMainWindow, (LPPOINT) &rcWindow + 1);

      if (FullScreen == 1 && DSMode == 0)
      {
        if (SurfaceX == 768 && SurfaceY == 720)
        {
          int marginx = (rcWindow.right - rcWindow.left - BlitArea.right + BlitArea.left)/2;
          int marginy = (rcWindow.bottom - rcWindow.top - BlitArea.bottom + BlitArea.top)/2;
          if (marginx>0)
          {
            rcWindow.left += marginx;
            rcWindow.right -= marginx;
          }
          if (marginy>0)
          {
            rcWindow.top += marginy;
            rcWindow.bottom -= marginy;
          }
        }
      }
   }
   else
   {
      atexit(ExitFunction);

      if (!QueryPerformanceFrequency((LARGE_INTEGER*)&freq)) return;

      if (!RegisterWinClass())
      { 
          exit(1);
      }
      X=(GetSystemMetrics(SM_CXSCREEN) - WindowWidth) / 2;
      Y=(GetSystemMetrics(SM_CYSCREEN) - WindowHeight) / 2;

      if (FullScreen==1) {X=0; Y=0;}

      if (hMainWindow) 
      {
         CloseWindow(hMainWindow);
      }

      if (SaveMainWindowPos == 1 && MainWindowX != -1 && FullScreen == 0) { X = MainWindowX; Y = MainWindowY; }

      hMainWindow = CreateWindow( "ZSNESWIN", WinName, WS_VISIBLE|WS_POPUP,X,Y,  //WS_OVERLAPPED "ZSNESWIN"
                                 WindowWidth,WindowHeight,NULL,NULL,hInst,NULL);

      CheckPriority();
      CheckAlwaysOnTop();
      CheckScreenSaver();

      if (!hMainWindow)
      { 
         return;
      }

      ShowWindow(hMainWindow, SW_SHOWNORMAL);
      SetWindowText(hMainWindow,"ZSNESWIN");
      InitInput();
      InitSound();
      TestJoy();
   }

   if (FirstVid == 1)
   {
      FirstVid = 0;
      InitDirectDraw();
      clearwin();
      clear_display();
      return;
   }

   if (Moving == 1) return;

   if (newmode == 1)
   {
      ReleaseDirectDraw();
      InitDirectDraw();
      clearwin();
      clear_display();
      return;
   }

}

extern unsigned int vidbuffer;
extern void SoundProcess();
extern int DSPBuffer;
int * DSPBuffer1;
DWORD ScreenPtr;
DWORD ScreenPtr2;
extern GUI36hzcall(void);
extern Game60hzcall(void);
extern int packettimeleft[256];
extern int PacketCounter;
extern int CounterA;
extern int CounterB;

void CheckTimers(void)
{
   QueryPerformanceCounter((LARGE_INTEGER*)&end2);

   while ((end2 - start2) >= update_ticks_pc2)
      {
         if (CounterA>0) CounterA--;
         if (CounterB>0) CounterB--;
         if (PacketCounter){
           for (int i=0;i<256;i++){
             if (packettimeleft[i]>0)
               packettimeleft[i]--;
           }
         }
         start2 += update_ticks_pc2;
      }                                     

   if (T60HZEnabled == 1)
   {
      QueryPerformanceCounter((LARGE_INTEGER*)&end);

   while ((end - start) >= update_ticks_pc)
      {
         Game60hzcall();
         start += update_ticks_pc;
      }                                     
   }

   if (T36HZEnabled == 1)
   {
      QueryPerformanceCounter((LARGE_INTEGER*)&end);

   while ((end - start) >= update_ticks_pc)
      {
         GUI36hzcall();
         start += update_ticks_pc;
      }                                     
   }
}

extern unsigned char MMXSupport;

void UpdateVFrame(void)
{
   int DataNeeded;
   int SPCSize=256;

   if (StereoSound==1)SPCSize=256;

   while (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
   {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }

   WinUpdateDevices();
   CheckTimers();

   if (SoundEnabled == 0) return;

   lpSoundBuffer->GetCurrentPosition(&CurrentPos,&WritePos);

   if (LastUsedPos <= CurrentPos)
   {
      DataNeeded=CurrentPos-LastUsedPos;
   }
   else
   {
      DataNeeded=SoundBufferSize - LastUsedPos + CurrentPos;
   }

   DataNeeded/=(SPCSize*2);
   DataNeeded*=(SPCSize*2);

   while (DataNeeded>0)
   {
      _asm
      {
         pushad
         call SoundProcess
         popad
      }

      DSPBuffer1=(int *)&DSPBuffer;

      int buffer_ptr = (int)&Buffer[0];

      if (T36HZEnabled == 1)
         if (MMXSupport == 1)
            _asm
            {
               mov edi,buffer_ptr
               mov ecx,SPCSize
               shr ecx,2
               pxor mm0,mm0
_blank_top_fpu:
               movq [edi],mm0
               add edi,8
               dec ecx
               jne _blank_top_fpu
               emms
            }
         else
            _asm
            {
               mov edi,buffer_ptr
               mov ecx,SPCSize
               shr ecx,1
               xor eax,eax
_blank_top:
               mov [edi],eax
               add edi,4
               dec ecx
               jne _blank_top
            }
      else
         if (MMXSupport == 1)
            _asm
            {
               mov esi,DSPBuffer1
               mov edi,buffer_ptr
               mov ecx,SPCSize
               shr ecx,2
_top_mmx:
               movq mm0,[esi]
               packssdw mm0,[esi+8]
               movq [edi],mm0
               add esi,16
               add edi,8
               dec ecx
               jne _top_mmx
               emms
            }
         else
            for(i=0;i<SPCSize;i++)
            {
               Buffer[i]=DSPBuffer1[i];
               if (DSPBuffer1[i]>32767)Buffer[i]=32767;
               if (DSPBuffer1[i]<-32767)Buffer[i]=-32767;
            }

      if (DS_OK!=lpSoundBuffer->Lock(LastUsedPos,
                                  SPCSize*2, &lpvPtr1,
                                  &dwBytes1, &lpvPtr2,
                                  &dwBytes2, 0))
      {
         return;
      }

      Sound=(short *)lpvPtr1;

      CopyMemory(lpvPtr1, &Buffer[0], dwBytes1);

      if (NULL != lpvPtr2)
      {
         CopyMemory(lpvPtr2, &Buffer[0]+dwBytes1, dwBytes2);
      }   

      if (DS_OK != lpSoundBuffer->Unlock(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2))
      {
         return;
      }

      LastUsedPos+=SPCSize*2;
      if (LastUsedPos==SoundBufferSize) LastUsedPos=0;
      DataNeeded-=(SPCSize*2);
   }

}

extern unsigned char curblank;
extern DWORD AddEndBytes;
extern DWORD NumBytesPerLine;
extern unsigned char * WinVidMemStart;
extern void copy640x480x16bwin(void);
extern void copy768x720x16bwin(void);
extern void copy768x720x32bwin(void);
extern unsigned char NGNoTransp;
extern unsigned char newengen;
extern void ClearWin16();
extern void ClearWin32();

void clearwin()
{
  HRESULT hRes;

  if (DD_CFB != NULL)
  {
    memset(&ddsd,0,sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);

    hRes = DD_CFB->Lock(NULL,&ddsd,DDLOCK_WAIT,NULL);

    if (hRes == DD_OK)
    {
      SurfBufD=(DWORD)ddsd.lpSurface;
      pitch = ddsd.lPitch;

      switch (BitDepth)
      {
        case 16:
          ClearWin16();
          break;
        case 32:
          ClearWin32();
          break;
      }
      DD_CFB->Unlock((struct tagRECT *)ddsd.lpSurface);
    }
    else
      if (hRes == DDERR_SURFACELOST)
        DD_CFB->Restore();
  }

  if (DD_CFB16 != NULL)
  {
    memset(&ddsd,0,sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);

    hRes = DD_CFB16->Lock(NULL,&ddsd,DDLOCK_WAIT,NULL);

    if (hRes == DD_OK)
    {
      SurfBufD=(DWORD)ddsd.lpSurface;
      pitch = ddsd.lPitch;
      ClearWin16();
      DD_CFB16->Unlock((struct tagRECT *)ddsd.lpSurface);
    }
    else
      if (hRes == DDERR_SURFACELOST)
        DD_CFB16->Restore();
  }
}

void clear_display()
{
  if (FullScreen == 1)
  {
    if ((DD_Primary != NULL) && (DD_BackBuffer != NULL))
    {
      DDBLTFX ddbltfx;

      ddbltfx.dwSize = sizeof(ddbltfx);
      ddbltfx.dwFillColor = 0;

      if (DD_BackBuffer->Blt( NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx ) == DDERR_SURFACELOST)
        DD_Primary->Restore();

      if (DD_Primary->Flip(NULL, DDFLIP_WAIT) == DDERR_SURFACELOST)
        DD_Primary->Restore();

      if (DD_BackBuffer->Blt( NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx ) == DDERR_SURFACELOST)
        DD_Primary->Restore();

      if (DD_Primary->Flip(NULL, DDFLIP_WAIT) == DDERR_SURFACELOST)
        DD_Primary->Restore();

      if (DD_BackBuffer->Blt( NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx ) == DDERR_SURFACELOST)
        DD_Primary->Restore();
    }
  }
}

extern void DrawWin256x224x16();
extern void DrawWin256x224x16MB();
extern void DrawWin256x224x32();
extern void DrawWin256x224x32MB();
extern void DrawWin320x240x16();

extern _int64 copymaskRB = 0x001FF800001FF800;
extern _int64 copymaskG = 0x0000FC000000FC00;
extern _int64 copymagic = 0x0008010000080100;
extern _int64 coef = 0x0066009a0066009a;

extern BYTE MotionBlur;
extern WORD totlines;

void drawscreenwin(void)
{
   DWORD i,j,color32;
   DWORD *SURFDW;

   NGNoTransp = 0;              // Set this value to 1 within the appropriate
                                // video mode if you want to add a custom
                                // transparency routine or hardware
                                // transparency.  This only works if
                                // the value of newengen is equal to 1.
                                // (see ProcessTransparencies in newgfx16.asm
                                //  for ZSNES' current transparency code)

   UpdateVFrame();
   if (curblank!=0) return;

   AltSurface = 0;

   if ( ((SurfaceX==512) || (SurfaceX==640)) && (BitDepth == 32) )
     AltSurface = 1;

   if (!(pitch = LockSurface()))
      return;

   ScreenPtr=vidbuffer;
   ScreenPtr+=16*2+32*2+256*2;

   if (resolutn == 224 && FullScreen == 0 && PrevRes != resolutn)
   {
      BlitArea.bottom = (SurfaceY/240)*224;
      if (SurfaceX == 512 && DSMode == 1) BlitArea.bottom = SurfaceY;
      if ((SurfaceX == 256 || SurfaceX == 512 || SurfaceX == 768) && (SMode == 0 && DSMode == 0)) WindowHeight = (WindowHeight/239)*224;
      initwinvideo();
      PrevRes = resolutn;
   }

   if (resolutn == 239 && FullScreen == 0 && PrevRes != resolutn)
   {
      BlitArea.bottom = (SurfaceY/240)*239;
      if (SurfaceX == 512 && DSMode == 1) BlitArea.bottom = SurfaceY;
      if ((SurfaceX == 256 || SurfaceX == 512 || SurfaceX == 768) && (SMode == 0 && DSMode == 0)) WindowHeight = (WindowHeight/224)*239;
      initwinvideo();
      PrevRes = resolutn;
   }

   DWORD HQ3XMode=0;

   if (MMXSupport == 0)
     hq3xFilter=0;
   else
   {
     if ((GUIHQ3X[cvidmode]!=0) && (hq3xFilter!=0))
       HQ3XMode=1;
   }

   if (prevHQ3XMode!=HQ3XMode)
     initwinvideo();

   SurfBufD=(DWORD) &SurfBuf[0];
   SURFDW=(DWORD *) &SurfBuf[0];

   if (KitchenSync == 1 && Refresh == 0)
   {
      Refresh = 60;
      InitDirectDraw();
   }

   if (KitchenSync == 0 && Refresh != 0)
   {
      Refresh = 0;
      InitDirectDraw();
   }

   if (KitchenSync == 1 && Refresh != 120 && totlines == 263)
   {
      Refresh = 120;
      InitDirectDraw();
   }

   if (KitchenSync == 1 && Refresh != 100 && totlines == 314)
   {
      Refresh = 100;
      InitDirectDraw();
   }

   if (SurfaceX == 256 && SurfaceY == 240)
   {
      switch (BitDepth)
      {
        case 16:
        {
          if (MotionBlur == 1) DrawWin256x224x16MB();
            else DrawWin256x224x16();
          break;
        }
        case 32:
        {
          if (MotionBlur == 1) DrawWin256x224x32MB();
            else DrawWin256x224x32();
          break;
        }

            SURFDW=(DWORD *) &SurfBuf[(resolutn-1)*pitch];
            color32=0x7F000000;
            
               for(i=0;i<256;i++)
               {
                  SURFDW[i]=color32;
               }

            SURFDW=(DWORD *) &SurfBuf[resolutn*pitch];
            color32=0x7F000000;
         
               for(i=0;i<256;i++)
               {
                  SURFDW[i]=color32;
               }         
            break;
        case 24:
            MessageBox (NULL, "Sorry.  ZSNESw does not work in windowed 24 bit color modes. \nClick 'OK' to switch to a full screen mode.", "DDRAW Error" , MB_ICONERROR );
            cvidmode=3;
            initwinvideo();
            Sleep(1000);
            drawscreenwin();
            break;
        default:
            UnlockSurface();
            MessageBox (NULL, "Mode only available in 16 and 32 bit color", "DDRAW Error" , MB_ICONERROR );
            cvidmode=2;
            initwinvideo();
            Sleep(1000);
            drawscreenwin();
            break;
      }
   }

   if (SurfaceX == 320 && SurfaceY == 240)
   {
      switch (BitDepth)
      {
        case 16:
        {
          DrawWin320x240x16();
          break;
        }
        case 32:
            for(j=0;j<8;j++)
            {
               SURFDW=(DWORD *) &SurfBuf[j*pitch];
               color32=0x7F000000;
            
               for(i=0;i<320;i++)
               {
                  SURFDW[i]=color32;
               }
            }

            for(j=8;j<(resolutn-1)+8;j++)
            {
               color32=0x7F000000;
               for(i=0;i<32;i++)
               {
                  SURFDW[i]=color32;
               }

               for(i=32;i<(256+32);i++)
               {
                  color32=(((*(WORD *)(ScreenPtr))&0xF800)<<8)+
                          (((*(WORD *)(ScreenPtr))&0x07E0)<<5)+
                          (((*(WORD *)(ScreenPtr))&0x001F)<<3)+0xFF000000;
//                  SURFDW[i]=color32;
                  ScreenPtr+=2;
               }

               color32=0x7F000000;
               for(i=(256+32);i<320;i++)
               {
                  SURFDW[i]=color32;
               }

               ScreenPtr=ScreenPtr+576-512;
               SURFDW=(DWORD *) &SurfBuf[(j)*pitch];
            }
   
            for(j=((resolutn-1)+8);j<240;j++)
            {
               SURFDW=(DWORD *) &SurfBuf[j*pitch];

               color32=0x7F000000;
               for(i=0;i<320;i++)
               {
                  SURFDW[i]=color32;
               }
            }
            break;
        default:
            UnlockSurface();
            MessageBox (NULL, "Mode only available in 16 and 32 bit color", "DDRAW Error" , MB_ICONERROR );
            cvidmode=2;
            initwinvideo();
            Sleep(1000);
            drawscreenwin();
            break;
      }
   }

   if (SurfaceX==512 && (SurfaceY==448 || SurfaceY==480))
   {
      switch (BitDepth)
      {
         case 16:
         case 32: // using 16bpp AltSurface
            AddEndBytes=pitch-1024;
            NumBytesPerLine=pitch;
            WinVidMemStart=&SurfBuf[0];
            _asm
            {
               pushad
               call copy640x480x16bwin
               popad
            }
            break;
         default:
            UnlockSurface();
            MessageBox (NULL, "Mode only available in 16 and 32 bit color", "DDRAW Error" , MB_ICONERROR );
            cvidmode=2;
            initwinvideo();
            Sleep(1000);
            drawscreenwin();
         }
   }

   if (SurfaceX == 640 && SurfaceY == 480)
   {
      switch (BitDepth)
      {
         case 16:
         case 32: // using 16bpp AltSurface
            AddEndBytes=pitch-1024;
            NumBytesPerLine=pitch;
            WinVidMemStart=&SurfBuf[16*640*2+64*2];
            _asm
            {
               pushad
               call copy640x480x16bwin
               popad
            }
            break;
         default:
            UnlockSurface();
            MessageBox (NULL, "Mode only available in 16 and 32 bit color", "DDRAW Error" , MB_ICONERROR );
            cvidmode=2;
            initwinvideo();
            Sleep(1000);
            drawscreenwin();
      }
   }
   if (SurfaceX == 768 && SurfaceY == 720)
   {
      switch (BitDepth)
      {
         case 16:
            AddEndBytes=pitch-768*2;
            NumBytesPerLine=pitch;
            WinVidMemStart=&SurfBuf[0];
            _asm
            {
               pushad
               call copy768x720x16bwin
               popad
            }
            break;
         case 32:
            AddEndBytes=pitch-768*4;
            NumBytesPerLine=pitch;
            WinVidMemStart=&SurfBuf[0];
            _asm
            {
               pushad
               call copy768x720x32bwin
               popad
            }
            break;
         default:
            UnlockSurface();
            MessageBox (NULL, "Mode only available in 16 and 32 bit color", "DDRAW Error" , MB_ICONERROR );
            cvidmode=2;
            initwinvideo();
            Sleep(1000);
            drawscreenwin();
      }
   }

   UnlockSurface();
   DrawScreen();
}

extern void SwitchFullScreen(void);

void WinUpdateDevices()
{
   int i,j;
   unsigned char * keys;
   unsigned char keys2[256];
   HRESULT hRes;

   for (i = 0; i<256; i++)
   keys2[i] = 0;
   keys = (unsigned char *)&pressed;

   if (KeyboardInput&&InputEn==1)
   {
      KeyboardInput->GetDeviceState(256, keys2);
   }
   else
   {
      return;
   }
   if (keys2[0x38] != 0 && keys2[0x3E] != 0) exit(0);
   if (keys2[0xB8] != 0 && keys2[0x1C] != 0 || keys2[0x38] != 0 && keys2[0x1C] != 0)
   {
      _asm
      {
         pushad
         call SwitchFullScreen
         popad
      }
      return;
   }

   for(i=0; i<256; i++)
   {
      if (keys2[i] == 0) keys[i] = 0;
      if (keys2[i] != 0 && keys[i] == 0) keys[i] = 1;
   }

   keys[0] = 0;

   for(i=0; i<5; i++)
   {
      if (JoystickInput[i])
      {
         for(j=0; j<32; j++)
         {
            keys[0x100 + i * 32 + j] = 0;
         }

         JoystickInput[i]->Poll();

         if (JoystickInput[i]->GetDeviceState(sizeof(DIJOYSTATE), &js[i])==DIERR_INPUTLOST)
         {
            if (JoystickInput[i]) JoystickInput[i]->Acquire();
            if (FAILED(JoystickInput[i]->GetDeviceState(sizeof(DIJOYSTATE), &js[i]))) return;
         }

         if (!X1Disable[i])
         {
            if (js[i].lX>0) keys[0x100 + i * 32 + 0] = 1;
         }

         if (!X2Disable[i])
         {
            if (js[i].lX<0) keys[0x100 + i * 32 + 1] = 1;
         }

         if (!Y1Disable[i])
         {
            if (js[i].lY>0) keys[0x100 + i * 32 + 2] = 1;
         }

         if (!Y2Disable[i])
         {
            if (js[i].lY<0) keys[0x100 + i * 32 + 3] = 1;
         }

         if (!Z1Disable[i])
         {
            if (js[i].lZ>0) keys[0x100 + i * 32 + 4] = 1;
         }

         if (!Z2Disable[i])
         {
            if (js[i].lZ<0) keys[0x100 + i * 32 + 5] = 1;
         }

         if (!RY1Disable[i])
         {
            if (js[i].lRy>0) keys[0x100 + i * 32 + 6] = 1;
         }

         if (!RY2Disable[i])
         {
            if (js[i].lRy<0) keys[0x100 + i * 32 + 7] = 1;
         }

         if (!RZ1Disable[i])
         {
            if (js[i].lRz>0) keys[0x100 + i * 32 + 8] = 1;
         }

         if (!RZ2Disable[i])
         {
            if (js[i].lRz<0) keys[0x100 + i * 32 + 9] = 1;
         }

         if (!S01Disable[i])
         {
            if (js[i].rglSlider[0]>0) keys[0x100 + i * 32 + 10] = 1;
         }

         if (!S02Disable[i])
         {
            if (js[i].rglSlider[0]<0) keys[0x100 + i * 32 + 11] = 1;
         }

         if (!S11Disable[i])
         {
            if (js[i].rglSlider[1]>0) keys[0x100 + i * 32 + 12] = 1;
         }

         if (!S12Disable[i])
         {
            if (js[i].rglSlider[1]<0) keys[0x100 + i * 32 + 13] = 1;
         }

         if (!POVDisable[i])
         {
            for (int p=0; p<NumPOV[i]; p++)
            {
               switch (js[i].rgdwPOV[p])
               {
               case 0:
                  keys[0x100 + i * 32 + 3] = 1;
                  break;
               case 4500:
                  keys[0x100 + i * 32 + 0] = 1;
                  keys[0x100 + i * 32 + 3] = 1;
                  break; 
               case 9000:
                  keys[0x100 + i * 32 + 0] = 1;
                  break;
               case 13500:
                  keys[0x100 + i * 32 + 0] = 1;
                  keys[0x100 + i * 32 + 2] = 1;
                  break;
               case 18000:
                  keys[0x100 + i * 32 + 2] = 1;
                  break;
               case 22500:
                  keys[0x100 + i * 32 + 1] = 1;
                  keys[0x100 + i * 32 + 2] = 1;
                  break;
               case 27000:
                  keys[0x100 + i * 32 + 1] = 1;
                  break;
               case 31500:
                  keys[0x100 + i * 32 + 1] = 1;
                  keys[0x100 + i * 32 + 3] = 1;
                  break;
               }
            }
         }

         if (NumBTN[i])
            for (j=0; j<NumBTN[i]; j++)
               if (js[i].rgbButtons[j]) keys[0x100 + i * 32 + 16 + j] = 1;
      }
      else
      {
         for(j=0; j<32; j++)
         {
            keys[0x100 + i * 32 + j] = 0;
         }
      }
   } 

}

extern BYTE snesmouse;

int GetMouseX(void)
{
   InputRead();
   MouseX += MouseMoveX;

   if (MouseX > MouseMaxX)
   {
      MouseX = MouseMaxX;

      if (TrapMouseCursor == 1)
      {
         if (abs(MouseMoveX) > 10 && T36HZEnabled == 1 && FullScreen == 0 && MouseButtonPressed == 0)
         {
            MouseInput->Unacquire();
            SetCursorPos(X + WindowWidth + 32, Y + (MouseY * WindowHeight / 224));
         }
      }
      else if (FullScreen == 0 && snesmouse == 0 && MouseButtonPressed == 0 && GUIOn2 == 1)
      {
         MouseInput->Unacquire();
         SetCursorPos(X + WindowWidth + 1, Y + (MouseY * WindowHeight / 224));
      }
   }

   if (MouseX < MouseMinX)
   {
      MouseX = MouseMinX;

      if (TrapMouseCursor == 1)
      {
         if (abs(MouseMoveX) > 10 && T36HZEnabled == 1 && FullScreen == 0 && MouseButtonPressed == 0)
         {
            MouseInput->Unacquire();
            SetCursorPos(X - 32, Y + (MouseY * WindowHeight / 224));
         }
      }
      else if (FullScreen == 0 && snesmouse == 0 && MouseButtonPressed == 0 && GUIOn2 == 1)
      {
         MouseInput->Unacquire();
         SetCursorPos(X - 1, Y + (MouseY * WindowHeight / 224));
      }
   }
   return((int) MouseX);
}

int GetMouseY(void)
{
   MouseY += MouseMoveY;

   if (MouseY > MouseMaxY)
   {
      MouseY = MouseMaxY;

      if (TrapMouseCursor == 1)
      {
         if (abs(MouseMoveY) > 10 && T36HZEnabled == 1 && FullScreen == 0 && MouseButtonPressed == 0)
         {
            MouseInput->Unacquire();
            SetCursorPos(X+(MouseX * WindowWidth / 256), Y + WindowHeight + 32);
         }
      }
      else if (FullScreen == 0 && snesmouse == 0 && MouseButtonPressed == 0 && GUIOn2 == 1)
      {
         MouseInput->Unacquire();
         SetCursorPos(X+(MouseX * WindowWidth / 256), Y + WindowHeight + 1);
      }
   }

   if (MouseY < MouseMinY)
   {
      MouseY = MouseMinY;

      if (TrapMouseCursor == 1)
      {
         if (abs(MouseMoveY) > 10 && T36HZEnabled == 1 && FullScreen == 0 && MouseButtonPressed == 0)
         {
            MouseInput->Unacquire();
            SetCursorPos(X + (MouseX * WindowWidth / 256), Y - 32);
         }
      }
      else if (FullScreen == 0 && snesmouse == 0 && MouseButtonPressed == 0 && GUIOn2 == 1)
      {
         MouseInput->Unacquire();
         SetCursorPos(X + (MouseX * WindowWidth / 256), Y - 1);
      }
   }

   return((int) MouseY);
}

int GetMouseMoveX(void)
{
   MouseMove2X=MouseMoveX;
   return(MouseMove2X);
}

int GetMouseMoveY(void)
{
   MouseMove2Y=MouseMoveY;
   return(MouseMove2Y);
}

int GetMouseButton(void)
{
   RECT rc1;
   if (MouseButton == 1) MouseButtonPressed = 1;
      else MouseButtonPressed = 0;
   if (MouseButton&2)
   {
   while (MouseButton != 0 && T36HZEnabled && FullScreen == 0)
   {
         Moving = 1;
         X += MouseMoveX;
         Y += MouseMoveY;
         if (X < 0)X = 0;
         if (X > (GetSystemMetrics(SM_CXSCREEN) - WindowWidth)) X = (GetSystemMetrics(SM_CXSCREEN) - WindowWidth);
         if (Y < 0)Y=0;
         if (Y > (GetSystemMetrics(SM_CYSCREEN) - WindowHeight)) Y = (GetSystemMetrics(SM_CYSCREEN) - WindowHeight);
         InputRead();
         initwinvideo();
      }
   }
   if (Moving == 1)
   {
      Moving = 0;
      initwinvideo();
   }
   return((int) MouseButton);
}

void SetMouseMinX(int MinX)
{
   MouseMinX = MinX;
}

void SetMouseMaxX(int MaxX)
{
   MouseMaxX = MaxX;
}

void SetMouseMinY(int MinY)
{
   MouseMinY = MinY;
}

void SetMouseMaxY(int MaxY)
{
   MouseMaxY = MaxY;
}

void SetMouseX(int X)
{
   MouseX = X;
}

void SetMouseY(int Y)
{
   MouseY = Y;
}

void FrameSemaphore()
{
   if (T60HZEnabled == 1)
   {
      int delay;
      QueryPerformanceCounter((LARGE_INTEGER*)&end);

      delay = ((update_ticks_pc - (end - start)) * 1000 / freq) - 3;
   
      if (delay>0) WaitForSingleObject(hLock, delay);

   }
}

void ZsnesPage()
{
   ShellExecute(NULL, NULL, "http://www.zsnes.com/", NULL, NULL, 0);
   MouseX = 0;
   MouseY = 0;
}

}
