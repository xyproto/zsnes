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

DWORD FullScreen = 0;

DWORD Moving= 0;
DWORD SoundBufferSize=1024*18;
DWORD FirstSound=1;

int AllowDefault=0;
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
LPDIRECTDRAWSURFACE7    DD_BackBuffer = NULL;
LPDIRECTDRAWCLIPPER     lpDDClipper = NULL;
RECT                    rcWindow;

LPDIRECTINPUT8          DInput = NULL;
LPDIRECTINPUTDEVICE8    MouseInput = NULL;
LPDIRECTINPUTDEVICE8    KeyboardInput = NULL;
LPDIRECTINPUTDEVICE8    JoystickInput[4];
DIJOYSTATE js[4];

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

DWORD                   SurfaceX=0;
DWORD                   SurfaceY=0;

BYTE                    IsActivated = 1;
BYTE                    AltTimer = 0;

extern "C" {
DWORD                   MouseButton;
}


#define UPDATE_TICKS_GAME 1000/60      // milliseconds per world update
#define UPDATE_TICKS_GAMEPAL 1000/50   // milliseconds per world update
#define UPDATE_TICKS_GUI 1000/36       // milliseconds per world update
#define UPDATE_TICKS_UDP 1000/60       // milliseconds per world update

_int64 start, end, freq, update_ticks_pc, start2, end2, update_ticks_pc2;

void ReleaseDirectDraw();
void ReleaseDirectSound();
void ReleaseDirectInput();

extern "C"
{
   void drawscreenwin(void);
   DWORD LastUsedPos=0;
   DWORD CurMode=-1;
}

void DDrawError(){
   char message1[256];

   sprintf(message1,"Error drawing to the screen\nMake sure the device is not being used by another process \0");
   MessageBox (NULL, message1, "DirectDraw Error" , MB_ICONERROR );
}

extern "C" BYTE vsyncon;
extern "C" BYTE TripleBufferWin;

void DrawScreen()
{
   if (FullScreen == 1)
   {
      if (TripleBufferWin == 1)
      {
         DD_BackBuffer->Blt(NULL, DD_CFB, NULL, DDBLT_WAIT, NULL);
         DD_Primary->Flip(NULL, DDFLIP_WAIT);
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
         DD_Primary->Blt(&rcWindow, DD_CFB, NULL, DDBLT_WAIT, NULL);
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
         DD_Primary->Blt(&rcWindow, DD_CFB, NULL, DDBLT_WAIT, NULL);
   }
}
 
DWORD InputEn=0;

InputAcquire(void)
{
   if (JoystickInput[0]) JoystickInput[0]->Acquire();
   if (JoystickInput[1]) JoystickInput[1]->Acquire();
   if (JoystickInput[2]) JoystickInput[2]->Acquire();
   if (JoystickInput[3]) JoystickInput[3]->Acquire();

	if (MouseInput) MouseInput->Acquire();
   if (KeyboardInput) KeyboardInput->Acquire();
   InputEn=1;
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
   InputEn=0;
	return TRUE;
}

extern "C" {
unsigned char keyboardhit=0;
void initwinvideo();
void DosExit();
extern BYTE StereoSound;
extern DWORD SoundQuality;
extern BYTE ExclusiveSound;
extern BYTE HighPriority;
extern BYTE AlwaysOnTop;
extern BYTE SaveMainWindowPos;
extern BYTE AlternateTimer;
extern BYTE AllowMultipleInst;
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

void ExitFunction()
{
   ReleaseDirectInput();
   ReleaseDirectSound();
   ReleaseDirectDraw();
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
         IsActivated = 1;
         if (LOWORD(wParam) != WA_INACTIVE)
            if (!FirstActivate) initwinvideo(); 
         InputAcquire();
         if (FirstActivate == 1) FirstActivate = 0;
         break;
      case WM_SETFOCUS:
         if (FullScreen == 0) ShowWindow(hMainWindow, SW_SHOWNORMAL);
         InputAcquire();
         break;
      case WM_KILLFOCUS:
         InputDeAcquire();
         IsActivated = 0;
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
         SetForegroundWindow(hFindWindow);
         DosExit();
      }
   }

   WNDCLASS wcl;

   wcl.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_NOCLOSE ;
   wcl.cbClsExtra	= 0;
   wcl.cbWndExtra = 0;
   wcl.hIcon = LoadIcon(NULL,IDI_APPLICATION);
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

   if (!SoundEnabled) return FALSE;

   PrevSoundQuality=SoundQuality;
   PrevStereoSound=StereoSound;

      if (DS_OK == DirectSoundCreate8(NULL, &lpDirectSound,NULL))
      {
          if (ExclusiveSound == 0)
          {
             if (DS_OK != lpDirectSound->SetCooperativeLevel(hMainWindow, DSSCL_NORMAL))
             {
                if (DS_OK != lpDirectSound->SetCooperativeLevel(hMainWindow, DSSCL_EXCLUSIVE))
                {
                   SoundEnabled=0;
                   return FALSE;
                }
             }
          }
          else
          {
             if (DS_OK != lpDirectSound->SetCooperativeLevel(hMainWindow, DSSCL_EXCLUSIVE))
             {
                SoundEnabled=0;
                return FALSE;
             }
          }
      }
      else 
      {
          SoundEnabled=0; return FALSE;
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
            SoundEnabled=0; return FALSE;
         }      
         FirstSound=0;
         return TRUE;
      }
      else
      {
         SoundEnabled=0; return FALSE;
      }
   } 
   else 
   {
      SoundEnabled=0; return FALSE;
   }

}

ReInitSound()
{
   WAVEFORMATEX wfx;

   if (!SoundEnabled) return FALSE;

   lpSoundBuffer->Stop();
   lpSoundBuffer->Release();

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
//   fprintf(tempf,"Cur :%d %X\n",CurrentJoy,pdinst->guidInstance);
   GUID DeviceGuid = pdinst->guidInstance;

   if (CurrentJoy>3)
      return DIENUM_CONTINUE;

   // Create the DirectInput joystick device.
   if (pdi->CreateDevice(DeviceGuid,&JoystickInput[CurrentJoy], NULL) != DI_OK)
   {
//      fprintf(tempf,"IDirectInput7::CreateDeviceEx FAILED\n");
      return DIENUM_CONTINUE;
   }

   if (JoystickInput[CurrentJoy]->SetDataFormat(&c_dfDIJoystick) != DI_OK)
   {
//      fprintf(tempf,"IDirectInputDevice7::SetDataFormat FAILED\n");
      JoystickInput[CurrentJoy]->Release();
      return DIENUM_CONTINUE;
   }

   if (JoystickInput[CurrentJoy]->SetCooperativeLevel(hMainWindow, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND) != DI_OK)
   {
//      fprintf(tempf,"IDirectInputDevice7::SetCooperativeLevel FAILED\n");
      JoystickInput[CurrentJoy]->Release();
      return DIENUM_CONTINUE;
   }

   DIPROPRANGE diprg;

   diprg.diph.dwSize       = sizeof(diprg);
   diprg.diph.dwHeaderSize = sizeof(diprg.diph);
   diprg.diph.dwObj        = DIJOFS_X;
   diprg.diph.dwHow        = DIPH_BYOFFSET;
   diprg.lMin              = -1000;
   diprg.lMax              = +1000;

   if FAILED(JoystickInput[CurrentJoy]->SetProperty(DIPROP_RANGE, &diprg.diph))
   {
//      fprintf(tempf,"IDirectInputDevice7::SetProperty(DIPH_RANGE) FAILED\n");
//      JoystickInput[CurrentJoy]->Release();
//      return FALSE;
      X1Disable[CurrentJoy]=1;
      X2Disable[CurrentJoy]=1;
   }

   diprg.diph.dwObj        = DIJOFS_Y;

   if FAILED(JoystickInput[CurrentJoy]->SetProperty(DIPROP_RANGE, &diprg.diph))
   {
//      fprintf(tempf,"IDirectInputDevice7::SetProperty(DIPH_RANGE) FAILED\n");
//      JoystickInput[CurrentJoy]->Release();
//      return FALSE;
      Y1Disable[CurrentJoy]=1;
      Y2Disable[CurrentJoy]=1;
   }

   diprg.diph.dwObj        = DIJOFS_Z;
   if (FAILED(JoystickInput[CurrentJoy]->SetProperty(DIPROP_RANGE, &diprg.diph)))
   {
      Z1Disable[CurrentJoy]=1;
      Z2Disable[CurrentJoy]=1;
   }

   diprg.diph.dwObj        = DIJOFS_RX;
   if (FAILED(JoystickInput[CurrentJoy]->SetProperty(DIPROP_RANGE, &diprg.diph)))
   {
      RX1Disable[CurrentJoy]=1;
      RX2Disable[CurrentJoy]=1;
   }

   diprg.diph.dwObj        = DIJOFS_RY;
   if (FAILED(JoystickInput[CurrentJoy]->SetProperty(DIPROP_RANGE, &diprg.diph)))
   {
      RY1Disable[CurrentJoy]=1;
      RY2Disable[CurrentJoy]=1;
   }

   diprg.diph.dwObj        = DIJOFS_RZ;
   if (FAILED(JoystickInput[CurrentJoy]->SetProperty(DIPROP_RANGE, &diprg.diph)))
   {
      RZ1Disable[CurrentJoy]=1;
      RZ2Disable[CurrentJoy]=1;
   }

   diprg.diph.dwObj        = DIJOFS_SLIDER(0);
   if (FAILED(JoystickInput[CurrentJoy]->SetProperty(DIPROP_RANGE, &diprg.diph)))
   {
      S01Disable[CurrentJoy]=1;
      S02Disable[CurrentJoy]=1;
   }

   diprg.diph.dwObj        = DIJOFS_SLIDER(1);
   if (FAILED(JoystickInput[CurrentJoy]->SetProperty(DIPROP_RANGE, &diprg.diph)))
   {
      S11Disable[CurrentJoy]=1;
      S12Disable[CurrentJoy]=1;
   }

   DIPROPDWORD dipdw;

   dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
   dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
   dipdw.diph.dwHow        = DIPH_BYOFFSET;
   dipdw.dwData            = 2500;
   dipdw.diph.dwObj         = DIJOFS_X;
   JoystickInput[CurrentJoy]->SetProperty(DIPROP_DEADZONE, &dipdw.diph);

   dipdw.diph.dwObj         = DIJOFS_Y;
   JoystickInput[CurrentJoy]->SetProperty(DIPROP_DEADZONE, &dipdw.diph);

   dipdw.diph.dwObj         = DIJOFS_Z;
   JoystickInput[CurrentJoy]->SetProperty(DIPROP_DEADZONE, &dipdw.diph);

   dipdw.diph.dwObj         = DIJOFS_RX;
   JoystickInput[CurrentJoy]->SetProperty(DIPROP_DEADZONE, &dipdw.diph);

   dipdw.diph.dwObj         = DIJOFS_RY;
   JoystickInput[CurrentJoy]->SetProperty(DIPROP_DEADZONE, &dipdw.diph);

   dipdw.diph.dwObj         = DIJOFS_RZ;
   JoystickInput[CurrentJoy]->SetProperty(DIPROP_DEADZONE, &dipdw.diph);

   dipdw.diph.dwObj         = DIJOFS_SLIDER(0);
   JoystickInput[CurrentJoy]->SetProperty(DIPROP_DEADZONE, &dipdw.diph);

   dipdw.diph.dwObj         = DIJOFS_SLIDER(1);
   JoystickInput[CurrentJoy]->SetProperty(DIPROP_DEADZONE, &dipdw.diph);

   dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
   dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
   dipdw.diph.dwHow        = DIPH_DEVICE;
   dipdw.dwData            = DIPROPAXISMODE_ABS;
   dipdw.diph.dwObj        = 0;

   JoystickInput[CurrentJoy]->SetProperty(DIPROP_AXISMODE, &dipdw.diph);

   CurrentJoy+=1;
//   fprintf(tempf,"joystick initialized!\n");

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

   for(int i = 0; i < 4; i++)
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

   if (FAILED(hr=DirectInput8Create(hInst,DIRECTINPUT_VERSION,IID_IDirectInput8A,(void **) &DInput,NULL)))
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
//         memset(&js[i], 0, sizeof(DIJOYSTATE));

         if (IDirectInputDevice8_GetDeviceState(JoystickInput[i],sizeof(DIJOYSTATE), &js[i])==DIERR_INPUTLOST)
         {
            if (JoystickInput[i]) JoystickInput[i]->Acquire();
            if (FAILED(IDirectInputDevice8_GetDeviceState(JoystickInput[i],sizeof(DIJOYSTATE), &js[i]))) return;
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

int InitDirectDraw()
{
   DDSURFACEDESC2       ddsd2;
   DDPIXELFORMAT        format;
   HRESULT hr;
   char message1[256];
   unsigned int color32,ScreenPtr2;
   int i;

   ScreenPtr2=BitConv32Ptr;
   for(i=0;i<65536;i++)
   {
      color32=((i&0xF800)<<8)+
              ((i&0x07E0)<<5)+
              ((i&0x001F)<<3)+0xFF000000;
              (*(unsigned int *)(ScreenPtr2))=color32;
      ScreenPtr2+=4;
   }

   if (!hMainWindow)
   {
      exit(1);
   }

   ReleaseDirectDraw();

   GetClientRect(hMainWindow, &rcWindow);
   ClientToScreen(hMainWindow, ( LPPOINT )&rcWindow);
   ClientToScreen(hMainWindow, ( LPPOINT )&rcWindow + 1);

   if (DirectDrawCreateEx(NULL, (void **)&lpDD, IID_IDirectDraw7, NULL) != DD_OK)
   {
      MessageBox(NULL, "DirectDrawCreateEx failed.", "DirectDraw Error", MB_ICONERROR);
   }

   if (FullScreen == 1)
   {
      if (lpDD->SetCooperativeLevel(hMainWindow, DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE | DDSCL_ALLOWREBOOT) != DD_OK)
      {
         MessageBox(NULL, "IDirectDraw7::SetCooperativeLevel failed.", "DirectDraw Error", MB_ICONERROR);
      }
      if (lpDD->SetDisplayMode(WindowWidth, WindowHeight, 16, 0, 0) != DD_OK)
      {
         MessageBox(NULL, "IDirectDraw7::SetDisplayMode failed.", "DirectDraw Error", MB_ICONERROR);
      }      
   }
   else
   {
      if (lpDD->SetCooperativeLevel(hMainWindow, DDSCL_NORMAL) != DD_OK)
      {
         MessageBox(NULL, "IDirectDraw7::SetCooperativeLevel failed.", "DirectDraw Error", MB_ICONERROR);
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

   if (lpDD->CreateSurface( &ddsd2, &DD_Primary, NULL) != DD_OK)
   {
      MessageBox(NULL, "IDirectDraw7::CreateSurface failed.", "DirectDraw Error", MB_ICONERROR);
   }

   if (FullScreen == 1)
   {
      ddsd2.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;
      if (DD_Primary->GetAttachedSurface(&ddsd2.ddsCaps, &DD_BackBuffer) != DD_OK)
      {
         MessageBox(NULL, "IDirectDrawSurface7::GetAttachedSurface failed.", "DirectDraw Error", MB_ICONERROR);
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
   }

   return TRUE;
}

BYTE* SurfBuf;
DDSURFACEDESC2       ddsd;

DWORD LockSurface()
{

   if (DD_CFB == NULL) return(0);
    
   memset(&ddsd,0,sizeof(ddsd));
   ddsd.dwSize = sizeof( ddsd );
   ddsd.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
   if (DD_CFB->Lock(NULL,&ddsd,DDLOCK_WAIT,NULL) != DD_OK)
   {
      return(0);
   }

   SurfBuf = (BYTE*)ddsd.lpSurface;
   return(ddsd.lPitch);
}

void UnlockSurface()
{
   DD_CFB->Unlock((struct tagRECT *)ddsd.lpSurface);
   DrawScreen();
}

extern "C" {

void WinUpdateDevices();

DWORD NeedBuffer=1;
short Buffer[1800*2];
int Running=0;

unsigned char Noise[]={ 27,232,234,138,187,246,176,81,25,241,1,127,154,190,195,103,231,165,220,238,
232,189,57,201,123,75,63,143,145,159,13,236,191,142,56,164,222,80,88,13,
148,118,162,212,157,146,176,0,241,88,244,238,51,235,149,50,77,212,186,241,
88,32,23,206,1,24,48,244,248,210,253,77,19,100,83,222,108,68,11,58,
152,161,223,245,4,105,3,82,15,130,171,242,141,2,172,218,152,97,223,157,
93,75,83,238,104,238,131,70,22,252,180,82,110,123,106,133,183,209,48,230,
157,205,27,21,107,63,85,164};

   int X, Y;
   DWORD Temp1;
	MSG msg;
   DWORD SurfBufD;
   int count, x,count2;
   HRESULT hr;
	int i;
	short *Sound;
	DWORD CurrentPos;
	DWORD WritePos;
   DWORD SoundBufD;
   DWORD SoundBufD2;

DWORD T60HZEnabled=0;
DWORD T36HZEnabled=0;

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

   if (AltTimer == 0)
   {
      QueryPerformanceCounter((LARGE_INTEGER*)&start);
      QueryPerformanceCounter((LARGE_INTEGER*)&start2);
   }
   else
   {
      start = timeGetTime();
      start2 = timeGetTime();
   }

   T36HZEnabled=0;
   T60HZEnabled=1;
}

void Stop60HZ(void)
{
   T60HZEnabled=0;
}

void Start36HZ(void)
{
   update_ticks_pc2 = UPDATE_TICKS_UDP * freq / 1000;
   update_ticks_pc = UPDATE_TICKS_GUI * freq / 1000;

   if (AltTimer == 0)
   {
      QueryPerformanceCounter((LARGE_INTEGER*)&start);
      QueryPerformanceCounter((LARGE_INTEGER*)&start2);
   }
   else
   {
      start = timeGetTime();
      start2 = timeGetTime();
   }

   T60HZEnabled=0;
   T36HZEnabled=1;
}

void Stop36HZ(void)
{
   T36HZEnabled=0;
}

char WinMessage[256];
extern unsigned char cvidmode;
extern BYTE BlackAndWhite;
extern BYTE V8Mode;
DWORD FirstVid=1;
DWORD FirstFull=1;
extern BYTE GUIWFVID[];
void clearwin();

char WinName[]={"ZSNESW\0"};

void initwinvideo(void)
{
   RECT zwindowrect;
   WINDOWPLACEMENT wndpl;
   RECT rc1, swrect;
   DWORD newmode=0;

   V8Mode = (BlackAndWhite == 1);

   if (CurMode!=cvidmode)
   {
      CurMode=cvidmode;
      newmode=1;
      SurfaceX=256;
      SurfaceY=224;
      X=0;
      Y=0;
      FullScreen=GUIWFVID[cvidmode];

      switch (cvidmode)
      {
      case 0:
         WindowWidth=64;
         WindowHeight=56;
         break;
      case 1:
         WindowWidth=128;
         WindowHeight=112;
         break;
      case 2:
         WindowWidth=256;
         WindowHeight=224;
         break;
      case 3:
         WindowWidth=640;
         WindowHeight=480;
         SurfaceX=320;
         SurfaceY=240;
         break;
      case 4:
         WindowWidth=512;
         WindowHeight=448;
         break;
      case 5:
         WindowWidth=512;
         WindowHeight=448;
         SurfaceX=512;
         SurfaceY=448;
         break;
      case 6:
         WindowWidth=640;
         WindowHeight=480;
         break;
      case 7:
         WindowWidth=640;
         WindowHeight=480;
         SurfaceX=640;
         SurfaceY=480;
         break;
      case 8:
         WindowWidth=640;
         WindowHeight=480;
         SurfaceX=512;
         SurfaceY=448;
         break;
      case 9:
         WindowWidth=640;
         WindowHeight=480;
         break;
      case 10:
         WindowWidth=800;
         WindowHeight=600;
         break;
      case 11:
         WindowWidth=800;
         WindowHeight=600;
         SurfaceX=512;
         SurfaceY=448;
         break;
      case 12:
         WindowWidth=800;
         WindowHeight=600;
         break;
      case 13:
         WindowWidth=800;
         WindowHeight=600;
         SurfaceX=512;
         SurfaceY=448;
         break;
      case 14:
         WindowWidth=1024;
         WindowHeight=768;
         break;
      case 15:
         WindowWidth=1024;
         WindowHeight=768;
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
         WindowWidth=768;
         WindowHeight=672;
         break;
      case 19:
         WindowWidth=768;
         WindowHeight=672;
         SurfaceX=512;
         SurfaceY=448;
         break;
      case 20:
         WindowWidth=1024;
         WindowHeight=896;
         break;
      case 21:
         WindowWidth=1024;
         WindowHeight=896;
         SurfaceX=512;
         SurfaceY=448;
         break;
      case 22:
         WindowWidth=1600;
         WindowHeight=1200;
         SurfaceX=512;
         SurfaceY=448;
         break;
      case 23:
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
   }

   if (((PrevStereoSound!=StereoSound)||(PrevSoundQuality!=SoundQuality))&&FirstSound!=1)
      ReInitSound();

   if (!FirstVid)
   {
//      sprintf(WinMessage,"FirstVid!=1 start\n\0");
//      MessageBox (NULL, WinMessage, "Init", MB_ICONERROR );
   
      if (X<0)X=0;
      if (X>(GetSystemMetrics( SM_CXSCREEN )-WindowWidth)) X=(GetSystemMetrics( SM_CXSCREEN )-WindowWidth);
      if (Y<0)Y=0;
      if (Y>(GetSystemMetrics( SM_CYSCREEN )-WindowHeight)) Y=(GetSystemMetrics( SM_CYSCREEN )-WindowHeight);
      if (FullScreen==1) {X=0; Y=0;}

      MainWindowX = X; MainWindowY = Y;

      MoveWindow( hMainWindow, X, Y,
                  WindowWidth, WindowHeight, TRUE );

      wndpl.length = sizeof(wndpl);
      GetWindowPlacement( hMainWindow, &wndpl);
      SetRect( &rc1, 0, 0, WindowWidth, WindowHeight );

      AdjustWindowRectEx( &rc1,GetWindowLong( hMainWindow, GWL_STYLE ),
      GetMenu( hMainWindow ) != NULL, GetWindowLong( hMainWindow, GWL_EXSTYLE ) ); 

      GetClientRect( hMainWindow, &rc1 );
      ClientToScreen( hMainWindow, ( LPPOINT )&rc1 );
      ClientToScreen( hMainWindow, ( LPPOINT )&rc1 + 1 );
//      return;
//      sprintf(WinMessage,"FirstVid!=1 end\n\0");
//      MessageBox (NULL, WinMessage, "Init", MB_ICONERROR );

//      MoveScreen(wndpl.rcNormalPosition.left, wndpl.rcNormalPosition.top);
   }
   else
   {
      FirstVid=0;
      atexit(ExitFunction);

      AltTimer = AlternateTimer;

      if (AltTimer == 0)
      {
         if (!QueryPerformanceFrequency((LARGE_INTEGER*)&freq)) return;
      }
      else
      {
         freq = CLOCKS_PER_SEC;
      }

//      hInst=GetModuleHandle(0);
      if (!RegisterWinClass())
      { 
          exit(1);
      }
      X=(GetSystemMetrics( SM_CXSCREEN ) - WindowWidth) / 2;
      Y=(GetSystemMetrics( SM_CYSCREEN ) - WindowHeight) / 2;
      if (FullScreen==1) {X=0; Y=0;}
      if (hMainWindow) 
      {
         CloseWindow(hMainWindow);
      }

      if (SaveMainWindowPos == 1 && MainWindowX != -1) { X = MainWindowX; Y = MainWindowY; }

      hMainWindow = CreateWindow( "ZSNESWIN", WinName, WS_VISIBLE|WS_POPUP,X,Y,  //WS_OVERLAPPED "ZSNESWIN"
                                 WindowWidth,WindowHeight,NULL,NULL,hInst,NULL);
      
      CheckPriority();
      CheckAlwaysOnTop();

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
   
   if (Moving == 1) return;
   
   if (FullScreen == 0 || newmode == 1)
   {
      if (InitDirectDraw() != TRUE)
      {
         exit(1);
      }
      if (newmode) clearwin();
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
   if (AltTimer == 0) QueryPerformanceCounter((LARGE_INTEGER*)&end2);
      else end2 = timeGetTime();

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

   if (T60HZEnabled)
   {
      if (AltTimer == 0) QueryPerformanceCounter((LARGE_INTEGER*)&end);
         else end = timeGetTime();

   while ((end - start) >= update_ticks_pc)
      {
         _asm{
         pushad
         call Game60hzcall
         popad
         }
         start += update_ticks_pc;
      }                                     
   }

   if (T36HZEnabled)
   {
      if (AltTimer == 0) QueryPerformanceCounter((LARGE_INTEGER*)&end);
         else end = timeGetTime();

   while ((end - start) >= update_ticks_pc)
      {
         _asm{
         pushad
         call GUI36hzcall
         popad
         }
         start += update_ticks_pc;
      }                                     
   }
}

extern BYTE GUIOn2;

void UpdateVFrame(void)
{

   if (GUIOn2 == 1 && IsActivated == 0) WaitMessage();

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

      for(i=0;i<SPCSize;i++)
      {
         Buffer[i]=DSPBuffer1[i];
         if (DSPBuffer1[i]>32767)Buffer[i]=32767;
         if (DSPBuffer1[i]<-32767)Buffer[i]=-32767;
         if (T36HZEnabled)Buffer[i]=0;
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
extern unsigned char FPUCopy;
extern unsigned char NGNoTransp;
extern unsigned char newengen;

void clearwin()
{
   DWORD i,j,color32;
   DWORD *SURFDW;

   Temp1=LockSurface();
   if (Temp1==0) { return; }

   SurfBufD=(DWORD) &SurfBuf[0];
   SURFDW=(DWORD *) &SurfBuf[0];

   switch (BitDepth)
   {
      case 16:
         _asm {
            push es
            mov ax,ds
            mov es,ax
            xor eax,eax
            mov edi,SurfBufD
            xor ebx,ebx
         Blank2:
            xor eax,eax
            mov ecx,SurfaceX
            rep stosw
            add edi,Temp1
            sub edi,SurfaceX
            sub edi,SurfaceX
            add ebx,1
            cmp ebx,SurfaceY
            jne Blank2
            pop es  // BUGFIX
         }
         break;
      case 32:
         _asm {
            push es
            mov ax,ds
            mov es,ax
            xor eax,eax
            mov edi,SurfBufD
            xor ebx,ebx
         Blank3:
            xor eax,eax
            mov ecx,SurfaceX
            rep stosd
            add edi,Temp1
            sub edi,SurfaceX
            sub edi,SurfaceX
            sub edi,SurfaceX
            sub edi,SurfaceX
            add ebx,1
            cmp ebx,SurfaceY
            jne Blank3
            pop es  // BUGFIX
         }
         break;
   }
   UnlockSurface();
}

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

   if (!(Temp1 = LockSurface()))
   { 
      DD_Primary->Restore();
      DD_CFB->Restore();
      return;
   }

   ScreenPtr=vidbuffer;
   ScreenPtr+=16*2+32*2+256*2; 
   SurfBufD=(DWORD) &SurfBuf[0];
   SURFDW=(DWORD *) &SurfBuf[0];

   if (SurfaceX==256&&SurfaceY==224)
   {
      switch (BitDepth)
      {
         case 16:
             if (FPUCopy){
                  _asm {
                     push es
                     mov ax,ds
                     mov es,ax
                     xor eax,eax
                     mov esi,ScreenPtr
                     mov edi,SurfBufD
                  Copying3:
                     mov ecx,32
                  CopyLoop:
                     movq mm0,[esi]
                     movq mm1,[esi+8]
                     movq [edi],mm0
                     movq [edi+8],mm1
                     add esi,16
                     add edi,16
                     dec ecx
                     jnz CopyLoop
                     inc eax            
                     add edi,Temp1
                     sub edi,512
                     sub esi,512
                     add esi,576
                     cmp eax,223
                     jne Copying3
                     xor eax,eax
                     mov ecx,128
                     rep stosd
                     pop es
                     emms
                  }
             } else {
                  _asm {
                     push es
                     mov ax,ds
                     mov es,ax
                     xor eax,eax
                     mov esi,ScreenPtr
                     mov edi,SurfBufD
                  Copying:
                     mov ecx,128
                     rep movsd
                     inc eax            
                     add edi,Temp1
                     sub edi,512
                     sub esi,512
                     add esi,576
                     cmp eax,223
                     jne Copying
                     xor eax,eax
                     mov ecx,128
                     rep stosd
                     pop es
                  }
            }
            break;
      case 32:
                  _asm {
                     push es
                     mov ax,ds
                     mov es,ax
                     xor eax,eax
                     mov ebx,BitConv32Ptr
                     mov esi,ScreenPtr
                     mov edi,SurfBufD
                  Copying32b:
                     mov ecx,256
                     push eax
                     xor eax,eax
                  CopyLoop32b:
                     mov ax,[esi]
                     add esi,2
                     mov edx,[ebx+eax*4]
                     mov [edi],edx
                     add edi,4
                     loop CopyLoop32b
                     pop eax
                     inc eax
                     add edi,Temp1
                     sub edi,1024
                     sub esi,512
                     add esi,576
                     cmp eax,223
                     jne Copying32b
                     pop es
                  }
   
            SURFDW=(DWORD *) &SurfBuf[222*Temp1];
            color32=0x7F000000;
            
               for(i=0;i<256;i++)
               {
                  SURFDW[i]=color32;
               }

            SURFDW=(DWORD *) &SurfBuf[223*Temp1];
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
//            exit(0);
            break;
      }
   }

   if (SurfaceX==320&&SurfaceY==240)
   {
      switch (BitDepth)
      {
         case 16:
             if (FPUCopy){
                  _asm {
                     push es
                     mov ax,ds
                     mov es,ax
                     xor eax,eax
                     xor ebx,ebx
                     mov esi,ScreenPtr
                     mov edi,SurfBufD
                  Blank1MMX:
                     xor eax,eax
                     mov ecx,160
                     rep stosd
                     sub edi,640
                     add edi,Temp1
                     add ebx,1
                     cmp ebx,8
                     jne Blank1MMX
                     xor ebx,ebx
                     pxor mm0,mm0
                  Copying2MMX:
                     mov ecx,4
                  MMXLoopA:
                     movq [edi],mm0
                     movq [edi+8],mm0
                     add edi,16
                     dec ecx
                     jnz MMXLoopA
                     mov ecx,32
                  MMXLoopB:
                     movq mm1,[esi]
                     movq mm2,[esi+8]
                     movq [edi],mm1
                     movq [edi+8],mm2
                     add esi,16
                     add edi,16
                     dec ecx
                     jnz MMXLoopB
                     mov ecx,4
                  MMXLoopC:
                     movq [edi],mm0
                     movq [edi+8],mm0
                     add edi,16
                     dec ecx
                     jnz MMXLoopC
                     inc ebx
                     add edi,Temp1
                     sub edi,640
                     sub esi,512
                     add esi,576
                     cmp ebx,223
                     jne Copying2MMX
                     xor eax,eax
                     mov ecx,128
                     rep stosd
                     pop es
                     emms
                  }
             } else {
                  _asm {
                     push es
                     mov ax,ds
                     mov es,ax
                     xor eax,eax
                     xor ebx,ebx
                     mov esi,ScreenPtr
                     mov edi,SurfBufD
                  Blank1:
                     xor eax,eax
                     mov ecx,160
                     rep stosd
                     sub edi,640
                     add edi,Temp1
                     add ebx,1
                     cmp ebx,8
                     jne Blank1
                     xor ebx,ebx
                  Copying2:
                     xor eax,eax
                     mov ecx,16
                     rep stosd
                     mov ecx,128
                     rep movsd
                     xor eax,eax
                     mov ecx,16
                     rep stosd
                     inc ebx
                     add edi,Temp1
                     sub edi,640
                     sub esi,512
                     add esi,576
                     cmp ebx,223
                     jne Copying2
                     xor eax,eax
                     mov ecx,128
                     rep stosd
                     pop es
                  }
             }
            break;
      case 32:
            for(j=0;j<8;j++)
            {
               SURFDW=(DWORD *) &SurfBuf[j*Temp1];
               color32=0x7F000000;
            
               for(i=0;i<320;i++)
               {
                  SURFDW[i]=color32;
               }
            }

            for(j=8;j<223+8;j++)
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
               SURFDW=(DWORD *) &SurfBuf[(j)*Temp1];
            }
   
            for(j=(223+8);j<240;j++)
            {
               SURFDW=(DWORD *) &SurfBuf[j*Temp1];

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
//            exit(0);
            break;
      }
   }

   if (SurfaceX==512&&SurfaceY==448)
   {
      switch (BitDepth)
      {
         case 16:
            AddEndBytes=Temp1-1024;
            NumBytesPerLine=Temp1;
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
            MessageBox (NULL, "Mode only available in 16 bit color", "DDRAW Error" , MB_ICONERROR );
            cvidmode=2;
            initwinvideo();
            Sleep(1000);
            drawscreenwin();
//            exit(0);
         }
   }

   if (SurfaceX==640&&SurfaceY==480)
   {
      switch (BitDepth)
      {
         case 16:
            AddEndBytes=Temp1-1024;
            NumBytesPerLine=Temp1;
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
            MessageBox (NULL, "Mode only available in 16 bit color", "DDRAW Error" , MB_ICONERROR );
            cvidmode=2;
            initwinvideo();
            Sleep(1000);
            drawscreenwin();
//            exit(0);
         }
   }


   UnlockSurface();
}

extern char fulladdtab[65536*2];
extern WORD vesa2_usbit;
extern WORD vesa2_clbit;
extern WORD vesa2_rpos;
extern WORD vesa2_rfull;
extern WORD vesa2_rtrcl;
extern WORD vesa2_rtrcla;
extern WORD vesa2_gpos;
extern WORD vesa2_gfull;
extern WORD vesa2_gtrcl;
extern WORD vesa2_gtrcla;
extern WORD vesa2_bpos;
extern WORD vesa2_bfull;
extern WORD vesa2_btrcl;
extern WORD vesa2_btrcla;
extern WORD nojoystickpoll;

extern void SwitchFullScreen(void);

void WinUpdateDevices()
{
   int i,j;
   unsigned char * keys;
   unsigned char keys2[256];
   HRESULT hRes;

   for (i=0;i<256;i++)
      keys2[i]=0;
   keys=(unsigned char *)&pressed;

   if (KeyboardInput&&InputEn==1)
   {
      KeyboardInput->GetDeviceState(256, keys2);
   }
   else
   {
      return;
   }
   if (keys2[0x38]!=0&&keys2[0x3E]!=0) exit(0);
   if (keys2[0x38]!=0&&keys2[0x1c]!=0)
   {
    _asm{
      pushad
      call SwitchFullScreen
      popad
      }
      return;
   }
   for(i=0;i<256;i++)
   {
      if (keys2[i]==0) keys[i]=0;
      if (keys2[i]!=0&&keys[i]==0) keys[i]=1;
   }
//   keys[1]=keys[16];
   keys[0]=0;

//   if (nojoystickpoll) return;

   for(i=0;i<4;i++)
   {
      if (JoystickInput[i])
      {
         for(j=0;j<32;j++)
         {
            keys[0x100+i*32+j]=0;
         }

//         memset(&js[i], 0, sizeof(DIJOYSTATE));
         JoystickInput[i]->Poll();
         if (IDirectInputDevice7_GetDeviceState(JoystickInput[i],sizeof(DIJOYSTATE), &js[i])==DIERR_INPUTLOST)
         {
            if (JoystickInput[i]) JoystickInput[i]->Acquire();
            if (FAILED(IDirectInputDevice7_GetDeviceState(JoystickInput[i],sizeof(DIJOYSTATE), &js[i]))) return;
         }

         if (!X1Disable[i])
         {
            if (js[i].lX>0) keys[0x100+i*32+0]=1;
         }

         if (!X2Disable[i])
         {
            if (js[i].lX<0) keys[0x100+i*32+1]=1;
         }


         if (!Y1Disable[i])
         {
            if (js[i].lY>0) keys[0x100+i*32+2]=1;
         }

         if (!Y2Disable[i])
         {
            if (js[i].lY<0) keys[0x100+i*32+3]=1;
         }

         if (!Z1Disable[i])
         {
            if (js[i].lZ>0) keys[0x100+i*32+4]=1;
         }

         if (!Z2Disable[i])
         {
            if (js[i].lZ<0) keys[0x100+i*32+5]=1;
         }
         if (!RY1Disable[i])
         {
            if (js[i].lRy>0) keys[0x100+i*32+6]=1;
         }
         if (!RY2Disable[i])
         {
            if (js[i].lRy<0) keys[0x100+i*32+7]=1;
         }
         if (!RZ1Disable[i])
         {
            if (js[i].lRz>0) keys[0x100+i*32+8]=1;
         }
         if (!RZ2Disable[i])
         {
            if (js[i].lRz<0) keys[0x100+i*32+9]=1;
         }
         if (!S01Disable[i])
         {
            if (js[i].rglSlider[0]>0) keys[0x100+i*32+10]=1;
         }
         if (!S02Disable[i])
         {
            if (js[i].rglSlider[0]<0) keys[0x100+i*32+11]=1;
         }
         if (!S11Disable[i])
         {
            if (js[i].rglSlider[1]>0) keys[0x100+i*32+12]=1;
         }
         if (!S12Disable[i])
         {
            if (js[i].rglSlider[1]<0) keys[0x100+i*32+13]=1;
         }
         if (js[i].rgbButtons[0]) keys[0x100+i*32+16]=1;
         if (js[i].rgbButtons[1]) keys[0x100+i*32+17]=1;
         if (js[i].rgbButtons[2]) keys[0x100+i*32+18]=1;
         if (js[i].rgbButtons[3]) keys[0x100+i*32+19]=1;
         if (js[i].rgbButtons[4]) keys[0x100+i*32+20]=1;
         if (js[i].rgbButtons[5]) keys[0x100+i*32+21]=1;
         if (js[i].rgbButtons[6]) keys[0x100+i*32+22]=1;
         if (js[i].rgbButtons[7]) keys[0x100+i*32+23]=1;
         if (js[i].rgbButtons[8]) keys[0x100+i*32+24]=1;
         if (js[i].rgbButtons[9]) keys[0x100+i*32+25]=1;
         if (js[i].rgbButtons[10]) keys[0x100+i*32+26]=1;
         if (js[i].rgbButtons[11]) keys[0x100+i*32+27]=1;
         if (js[i].rgbButtons[12]) keys[0x100+i*32+28]=1;
         if (js[i].rgbButtons[13]) keys[0x100+i*32+29]=1;
         if (js[i].rgbButtons[14]) keys[0x100+i*32+30]=1;
         if (js[i].rgbButtons[15]) keys[0x100+i*32+31]=1;
      }
      else
      {
         for(j=0;j<32;j++)
         {
            keys[0x100+i*32+j]=0;
         }
      }
   } 

}

int GetMouseX(void)
{
   InputRead();
   MouseX+=MouseMoveX;
   if (MouseX>MouseMaxX)
   {
      if (abs(MouseMoveX)>10&&T36HZEnabled&&(FullScreen==0))
      {
         MouseInput->Unacquire();
         SetCursorPos(X+WindowWidth+32,Y+(MouseY*WindowHeight/224));
      }
      MouseX=MouseMaxX;
   }

   if (MouseX<MouseMinX)
   {
      if (abs(MouseMoveX)>10&&T36HZEnabled&&(FullScreen==0))
      {
         MouseInput->Unacquire();
         SetCursorPos(X-32,Y+(MouseY*WindowHeight/224));
      }
      MouseX=MouseMinX;
   }
   return((int)MouseX);
}

int GetMouseY(void)
{
   MouseY+=MouseMoveY;
   if (MouseY>MouseMaxY)
   {
      MouseY=MouseMaxY;
      if (abs(MouseMoveY)>10&&T36HZEnabled&&(FullScreen==0))
      {
         MouseInput->Unacquire();
         SetCursorPos(X+(MouseX*WindowWidth/256), Y+WindowHeight+32);
      }
   }
   if (MouseY<MouseMinY)
   {
      MouseY=MouseMinY;
      if (abs(MouseMoveY)>10&&T36HZEnabled&&(FullScreen==0))
      {
         MouseInput->Unacquire();
         SetCursorPos(X+(MouseX*WindowWidth/256), Y-32);
      }
   }
   return((int)MouseY);
}

int GetMouseMoveX(void)
{
//   InputRead();
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
   if (MouseButton&2)
   {
   while ((MouseButton!=0)&&T36HZEnabled&&(FullScreen==0))
   {
         Moving=1;
         X+=MouseMoveX;
         Y+=MouseMoveY;
         if (X<0)X=0;
         if (X>(GetSystemMetrics( SM_CXSCREEN )-WindowWidth)) X=(GetSystemMetrics( SM_CXSCREEN )-WindowWidth);
         if (Y<0)Y=0;
         if (Y>(GetSystemMetrics( SM_CYSCREEN )-WindowHeight)) Y=(GetSystemMetrics( SM_CYSCREEN )-WindowHeight);
         InputRead();
         initwinvideo();
      }
   }
   if (Moving==1)
   {
      Moving=0;
      initwinvideo();
   }
   return((int)MouseButton);
}

void SetMouseMinX(int MinX)
{
}

void SetMouseMaxX(int MaxX)
{
}

void SetMouseMinY(int MinY)
{
}

void SetMouseMaxY(int MaxY)
{
}

void SetMouseX(int X)
{
}

void SetMouseY(int Y)
{
}

void ZsnesPage()
{
     ShellExecute(NULL, NULL, "http://www.zsnes.com/", NULL, NULL, 0);
}

}
