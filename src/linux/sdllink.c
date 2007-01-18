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
#include "sw_draw.h"
#include "gl_draw.h"

#include <stdbool.h>
#include <SDL_thread.h>

#include <sys/time.h>
#include <time.h>

#include "audio.h"
#include "safelib.h"
#include "../cfg.h"
#include "../input.h"

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef Uint32 UINT32;
typedef long long _int64;
typedef long long LARGE_INTEGER;
#define QueryPerformanceCounter(x) asm volatile("rdtsc" : "=a"(((unsigned int *)(x))[0]),"=d"(((unsigned int *)x)[1]))


typedef enum { FALSE = 0, TRUE = 1 } BOOL;
typedef enum vidstate_e { vid_null, vid_none, vid_soft, vid_gl } vidstate_t;

/* VIDEO VARIABLES */
SDL_Surface *surface;
int SurfaceLocking = 0;
int SurfaceX, SurfaceY;
static DWORD WindowWidth = 256;
static DWORD WindowHeight = 224;
static DWORD FullScreen = 0;
static vidstate_t sdl_state = vid_null;
static int UseOpenGL = 0;
static const int BitDepth = 16;
DWORD FirstVid = 1;

void SwitchFullScreen (void);
DWORD SMode=0;
DWORD DSMode=0;
DWORD prevHQMode=-1;

extern DWORD converta;
extern DWORD *BitConv32Ptr;
extern DWORD *RGBtoYUVPtr;

extern BYTE GUIWFVID[];
extern BYTE GUISMODE[];
extern BYTE GUIDSMODE[];
extern BYTE GUIHQ2X[];
extern BYTE GUIHQ3X[];
extern BYTE GUIHQ4X[];
extern BYTE GUIRESIZE[];
extern BYTE GUIM7VID[];

/* JOYSTICK AND KEYBOARD INPUT */
SDL_Joystick *JoystickInput[5];
unsigned int AxisOffset[5] = {256 + 128 + 64};  // per joystick offsets in
unsigned int ButtonOffset[5] = {448};           // pressed. We have 128 + 64
unsigned int HatOffset[5] = {448};              // bytes for all joysticks. We
unsigned int BallOffset[5] = {448};             // can control all 5 players.
int shiftptr = 0;
int offset;
DWORD numlockptr;

extern unsigned char pressed[];
extern int CurKeyPos;
extern int CurKeyReadPos;
extern int KeyBuffer[16];

/* MOUSE INPUT */
static float MouseMinX = 0;
static float MouseMaxX = 256;
static float MouseMinY = 0;
static float MouseMaxY = 223;
static int MouseX, MouseY;
static int MouseMove2X, MouseMove2Y;
unsigned char MouseButton;
static float MouseXScale = 1.0;
static float MouseYScale = 1.0;
DWORD LastUsedPos = 0;
DWORD CurMode = -1;

extern BYTE GUIOn;
extern BYTE GUIOn2;
extern BYTE EMUPause;
static BYTE IsActivated = 1;

/* TIMER VARIABLES/MACROS */
// millisecond per world update
#define UPDATE_TICKS_GAME (1000.0/59.948743718592964824120603015060)
#define UPDATE_TICKS_GAMEPAL (20.0)
#define UPDATE_TICKS_GUI (1000.0/36.0)
#define UPDATE_TICKS_UDP (1000.0/60.0)

int T60HZEnabled = 0;
int T36HZEnabled = 0;
float end, end2;
float start, start2;
float update_ticks_pc, update_ticks_pc2;

// Used for semaphore code
static SDL_sem *sem_frames = NULL;
static struct timeval sem_start;
void sem_sleep_rdy(void);
void sem_sleep_die(void);
float sem_GetTicks(void);

extern unsigned char romispal;

/* FUNCTION DECLARATIONS */
void clearwin (void);
void drawscreenwin(void);
void initwinvideo();
unsigned int sdl_keysym_to_pc_scancode(int);
void ProcessKeyBuf(int);
void UpdateSound(void *userdata, Uint8 * stream, int len);

void GUI36hzcall(void);
void Game60hzcall(void);
_int64 copymaskRB = 0x001FF800001FF800LL;
_int64 copymaskG = 0x0000FC000000FC00LL;
_int64 copymagic = 0x0008010000080100LL;
#ifdef __OPENGL__
void gl_clearwin(void);
#endif

static void adjustMouseXScale(void)
{
  MouseXScale = (MouseMaxX - MouseMinX) / ((float) WindowWidth);
}

static void adjustMouseYScale(void)
{
  MouseYScale = (MouseMaxY - MouseMinY) / ((float) WindowHeight);
}

void SetHQx(unsigned int ResX, unsigned int ResY)
{
  int maxHQ;
  if(ResX/256 < ResY/224)
    maxHQ = ResX/256;
  else
    maxHQ = ResY/224;

  if(maxHQ >= 2)
  {
    GUIHQ2X[cvidmode] = 1;
    GUIHQ3X[cvidmode] = 0;
    GUIHQ4X[cvidmode] = 0;
  }

  else
  {
    GUIHQ2X[cvidmode] = 0;
    GUIHQ3X[cvidmode] = 0;
    GUIHQ4X[cvidmode] = 0;
  }
}

void SetHiresOpt(unsigned int ResX, unsigned int ResY)
{
  if(ResX >= 512 && ResY >= 448)
    GUIM7VID[cvidmode] = 1;
  else
    GUIM7VID[cvidmode] = 0;
}

void Clear2xSaIBuffer();

int Main_Proc(void)
{
  SDL_Event event;
  unsigned int key;

  while (SDL_PollEvent(&event))
  {
    switch (event.type)
    {
      case SDL_ACTIVEEVENT:
        IsActivated = event.active.gain;
        break;
      case SDL_KEYDOWN:
        if ((event.key.keysym.sym == SDLK_RETURN) &&
            (event.key.keysym.mod & KMOD_ALT)) {
          SwitchFullScreen();
          break;
        }
        if (event.key.keysym.sym == SDLK_LSHIFT ||
            event.key.keysym.sym == SDLK_RSHIFT)
          shiftptr = 1;
        if (event.key.keysym.mod & KMOD_NUM)
          numlockptr = 1;
        else
          numlockptr = 0;

        key = sdl_keysym_to_pc_scancode(event.key.keysym.sym);
        if (key < 448)
        {
          pressed[key] = 1;
          ProcessKeyBuf(event.key.keysym.sym);
        }
        break;

      case SDL_KEYUP:
        if (event.key.keysym.sym == SDLK_LSHIFT ||
            event.key.keysym.sym == SDLK_RSHIFT)
          shiftptr = 0;
        key = sdl_keysym_to_pc_scancode(event.key.keysym.sym);
        if (key < 448)
        {
          pressed[key] = 0;
        }
        break;

      case SDL_MOUSEMOTION:
        if (FullScreen)
        {
          MouseX += event.motion.xrel;
          MouseY += event.motion.yrel;
        }
        else
        {
          MouseX = ((int) ((float) event.motion.x) * MouseXScale);
          MouseY = ((int) ((float) event.motion.y) * MouseYScale);
        }

        if (MouseX < MouseMinX) MouseX = MouseMinX;
        if (MouseX > MouseMaxX) MouseX = MouseMaxX;
        if (MouseY < MouseMinY) MouseY = MouseMinY;
        if (MouseY > MouseMaxY) MouseY = MouseMaxY;
        break;

      case SDL_MOUSEBUTTONDOWN:
        /*
           button 2 = enter (i.e. select)
           button 4 = mouse wheel up (treat as "up" key)
           button 5 = mouse wheel down (treat as "down" key)
         */
        switch (event.button.button)
        {
          case 4:
            ProcessKeyBuf(SDLK_UP);
            break;
          case 5:
            ProcessKeyBuf(SDLK_DOWN);
            break;
          case 3:
            MouseButton |= 2;
            break;
          case 2:
            ProcessKeyBuf(SDLK_RETURN);
            // Yes, this is intentional - DDOI
          case 1:
            MouseButton |= event.button.button;
            break;
        }
        break;

      case SDL_MOUSEBUTTONUP:
        switch (event.button.button)
        {
          case 1: case 2:
            MouseButton &= ~event.button.button;
            break;

          case 3:
            MouseButton &= ~2;
            break;
        }
        break;

      case SDL_JOYHATMOTION: // POV hats act as direction pad
        offset = HatOffset[event.jhat.which];
        if (offset >= (256 + 128 + 64)) break;
        switch (event.jhat.value)
        {
          case SDL_HAT_CENTERED:
            pressed[offset] = 0;
            pressed[offset + 1] = 0;
            pressed[offset + 2] = 0;
            pressed[offset + 3] = 0;
            break;
          case SDL_HAT_UP:
            pressed[offset + 3] = 1;
            pressed[offset + 2] = 0;
            break;
          case SDL_HAT_RIGHTUP:
            pressed[offset] = 1;
            pressed[offset + 3] = 1;
            pressed[offset + 1] = 0;
            pressed[offset + 2] = 0;
            break;
          case SDL_HAT_RIGHT:
            pressed[offset] = 1;
            pressed[offset + 1] = 0;
            break;
          case SDL_HAT_RIGHTDOWN:
            pressed[offset] = 1;
            pressed[offset + 2] = 1;
            pressed[offset + 1] = 0;
            pressed[offset + 3] = 0;
            break;
          case SDL_HAT_DOWN:
            pressed[offset + 2] = 1;
            pressed[offset + 3] = 0;
            break;
          case SDL_HAT_LEFTDOWN:
            pressed[offset + 1] = 1;
            pressed[offset + 2] = 1;
            pressed[offset] = 0;
            pressed[offset + 3] = 0;
            break;
          case SDL_HAT_LEFT:
            pressed[offset + 1] = 1;
            pressed[offset] = 0;
            break;
          case SDL_HAT_LEFTUP:
            pressed[offset + 1] = 1;
            pressed[offset + 3] = 1;
            pressed[offset] = 0;
            pressed[offset + 2] = 0;
            break;
        }
        break;

      /*
         joystick trackball code untested; change the test
         values if the motion is too sensitive (or not
         sensitive enough)
       */
      case SDL_JOYBALLMOTION:
        offset = BallOffset[event.jball.which];
        offset += event.jball.ball;
        if (offset >= (256 + 128 + 64)) break;
        if (event.jball.xrel < -100)
        {
          pressed[offset] = 0;
          pressed[offset + 1] = 1;
        }
        if (event.jball.xrel > 100)
        {
          pressed[offset] = 1;
          pressed[offset + 1] = 0;
        }
        if (event.jball.yrel < -100)
        {
          pressed[offset + 2] = 0;
          pressed[offset + 3] = 1;
        }
        if (event.jball.yrel > 100)
        {
          pressed[offset + 2] = 1;
          pressed[offset + 3] = 0;
        }
        break;

      case SDL_JOYAXISMOTION:
        offset = AxisOffset[event.jaxis.which];
        offset += event.jaxis.axis * 2;
        if (offset >= (256 + 128 + 64)) break;
        //printf("DEBUG axis offset: %d\n", offset);
        if (event.jaxis.value < -(joy_sensitivity))
        {
          pressed[offset + 1] = 1;
          pressed[offset + 0] = 0;
        }
        else if (event.jaxis.value > joy_sensitivity)
        {
          pressed[offset + 0] = 1;
          pressed[offset + 1] = 0;
        }
        else
        {
          pressed[offset + 0] = 0;
          pressed[offset + 1] = 0;
        }
        break;

      case SDL_JOYBUTTONDOWN:
        offset = ButtonOffset[event.jbutton.which];
        offset += event.jbutton.button;
        //printf("DEBUG button offset: %d\n", offset);
        if (offset >= (256 + 128 + 64)) break;
        pressed[offset] = 1;
        break;

      case SDL_JOYBUTTONUP:
        offset = ButtonOffset[event.jbutton.which];
        offset += event.jbutton.button;
        //printf("DEBUG button offset: %d\n", offset);
        if (offset >= (256 + 128 + 64)) break;
        pressed[offset] = 0;
        break;
      case SDL_QUIT:
        exit(0);
        break;
#ifdef __OPENGL__
      case SDL_VIDEORESIZE:
        if(!GUIRESIZE[cvidmode])
        {
          surface = SDL_SetVideoMode(WindowWidth, WindowHeight, BitDepth, surface->flags & ~SDL_RESIZABLE);
          adjustMouseXScale();
          adjustMouseYScale();
          break;
        }
        WindowWidth = SurfaceX = event.resize.w;
        WindowHeight = SurfaceY = event.resize.h;
        SetHQx(SurfaceX,SurfaceY);
        SetHiresOpt(SurfaceX,SurfaceY);
        surface = SDL_SetVideoMode(WindowWidth, WindowHeight, BitDepth, surface->flags);
        adjustMouseXScale();
        adjustMouseYScale();
        glViewport(0,0, WindowWidth, WindowHeight);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        if (cvidmode == 20)
        {
          if (224*WindowWidth > 256*WindowHeight && WindowHeight)
          {
            glOrtho (- ((float) 224*WindowWidth)/((float) 256*WindowHeight),
                     ((float) 224*WindowWidth)/((float) 256*WindowHeight), -1, 1, -1, 1);
          }
          else if (224*WindowWidth < 256*WindowHeight && WindowWidth)
          {
            glOrtho (-1, 1,- ((float) 256*WindowHeight)/((float) 224*WindowWidth),
                   ((float) 256*WindowHeight)/((float) 224*WindowWidth), -1, 1);
          }
          else
          {
            glOrtho (-1, 1, -1, 1, -1, 1);
          }
        }

        if (Keep4_3Ratio && (cvidmode == 21))
        {
          if (3*WindowWidth > 4*WindowHeight && WindowHeight)
          {
            glOrtho (- ((float) 3*WindowWidth)/((float) 4*WindowHeight),
                     ((float) 3*WindowWidth)/((float) 4*WindowHeight), -1, 1, -1, 1);
          }
          else if (3*WindowWidth < 4*WindowHeight && WindowWidth)
          {
            glOrtho (-1, 1,- ((float) 4*WindowHeight)/((float) 3*WindowWidth),
                   ((float) 4*WindowHeight)/((float) 3*WindowWidth), -1, 1);
          }
          else
          {
            glOrtho (-1, 1, -1, 1, -1, 1);
          }
        }

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glDisable(GL_DEPTH_TEST);
        glFlush();
        gl_clearwin();
        Clear2xSaIBuffer();
        break;
#endif
      default:
        break;
    }
  }

  return TRUE;
}

unsigned int sdl_keysym_to_pc_scancode(int sym)
{
  switch (sym)
  {
    case SDLK_ESCAPE:       return 0x01;
    case SDLK_1:            return 0x02;
    case SDLK_2:            return 0x03;
    case SDLK_3:            return 0x04;
    case SDLK_4:            return 0x05;
    case SDLK_5:            return 0x06;
    case SDLK_6:            return 0x07;
    case SDLK_7:            return 0x08;
    case SDLK_8:            return 0x09;
    case SDLK_9:            return 0x0a;
    case SDLK_0:            return 0x0b;
    case SDLK_MINUS:        return 0x0c;
    case SDLK_EQUALS:       return 0x0d;
    case SDLK_BACKSPACE:    return 0x0e;
    case SDLK_TAB:          return 0x0f;
    case SDLK_q:            return 0x10;
    case SDLK_w:            return 0x11;
    case SDLK_e:            return 0x12;
    case SDLK_r:            return 0x13;
    case SDLK_t:            return 0x14;
    case SDLK_y:            return 0x15;
    case SDLK_u:            return 0x16;
    case SDLK_i:            return 0x17;
    case SDLK_o:            return 0x18;
    case SDLK_p:            return 0x19;
    case SDLK_LEFTBRACKET:  return 0x1a;
    case SDLK_RIGHTBRACKET: return 0x1b;
    case SDLK_RETURN:       return 0x1c;
    case SDLK_LCTRL:        return 0x1d;
    case SDLK_a:            return 0x1e;
    case SDLK_s:            return 0x1f;
    case SDLK_d:            return 0x20;
    case SDLK_f:            return 0x21;
    case SDLK_g:            return 0x22;
    case SDLK_h:            return 0x23;
    case SDLK_j:            return 0x24;
    case SDLK_k:            return 0x25;
    case SDLK_l:            return 0x26;
    case SDLK_SEMICOLON:    return 0x27;
    case SDLK_QUOTE:        return 0x28;
    case SDLK_BACKQUOTE:
    case SDLK_HASH:         return 0x29;
    case SDLK_LSHIFT:       return 0x2a;
    case SDLK_BACKSLASH:    return 0x2b;
    case SDLK_z:            return 0x2c;
    case SDLK_x:            return 0x2d;
    case SDLK_c:            return 0x2e;
    case SDLK_v:            return 0x2f;
    case SDLK_b:            return 0x30;
    case SDLK_n:            return 0x31;
    case SDLK_m:            return 0x32;
    case SDLK_COMMA:        return 0x33;
    case SDLK_PERIOD:       return 0x34;
    case SDLK_SLASH:        return 0x35;
    case SDLK_RSHIFT:       return 0x36;
    case SDLK_KP_MULTIPLY:  return 0x37;
    case SDLK_LALT:         return 0x38;
    case SDLK_SPACE:        return 0x39;
    case SDLK_CAPSLOCK:     return 0x3a;
    case SDLK_F1:           return 0x3b;
    case SDLK_F2:           return 0x3c;
    case SDLK_F3:           return 0x3d;
    case SDLK_F4:           return 0x3e;
    case SDLK_F5:           return 0x3f;
    case SDLK_F6:           return 0x40;
    case SDLK_F7:           return 0x41;
    case SDLK_F8:           return 0x42;
    case SDLK_F9:           return 0x43;
    case SDLK_F10:          return 0x44;
    case SDLK_NUMLOCK:      return 0x45;
    case SDLK_SCROLLOCK:    return 0x46;
    case SDLK_KP7:          return 0x47;
    case SDLK_KP8:          return 0x48;
    case SDLK_KP9:          return 0x49;
    case SDLK_KP_MINUS:     return 0x4a;
    case SDLK_KP4:          return 0x4b;
    case SDLK_KP5:          return 0x4c;
    case SDLK_KP6:          return 0x4d;
    case SDLK_KP_PLUS:      return 0x4e;
    case SDLK_KP1:          return 0x4f;
    case SDLK_KP2:          return 0x50;
    case SDLK_KP3:          return 0x51;
    case SDLK_KP0:          return 0x52;
    case SDLK_KP_PERIOD:    return 0x53;
    case SDLK_F11:          return 0x57;
    case SDLK_F12:          return 0x58;
    case SDLK_HOME:         return 0x59;
    case SDLK_UP:           return 0x5a;
    case SDLK_PAGEUP:       return 0x5b;
    case SDLK_LEFT:         return 0x5c;
    case SDLK_RIGHT:        return 0x5e;
    case SDLK_END:          return 0x5f;
    case SDLK_DOWN:         return 0x60;
    case SDLK_PAGEDOWN:     return 0x61;
    case SDLK_INSERT:       return 0x62;
    case SDLK_DELETE:       return 0x63;

  }
  return(0x64+sym);
}

void ProcessKeyBuf(int scancode)
{
  int accept = 0;
  int vkeyval = 0;

  if (((scancode >= 'A') && (scancode <= 'Z')) ||
      ((scancode >= 'a') && (scancode <= 'z')) ||
      (scancode == SDLK_ESCAPE) || (scancode == SDLK_SPACE) ||
      (scancode == SDLK_BACKSPACE) || (scancode == SDLK_RETURN) ||
      (scancode == SDLK_TAB))
  {
    accept = 1;
    vkeyval = scancode;
  }
  if ((scancode >= '0') && (scancode <= '9'))
  {
    accept = 1;
    vkeyval = scancode;
    if (shiftptr)
    {
      switch (scancode)
      {
        case '1': vkeyval = '!'; break;
        case '2': vkeyval = '@'; break;
        case '3': vkeyval = '#'; break;
        case '4': vkeyval = '$'; break;
        case '5': vkeyval = '%'; break;
        case '6': vkeyval = '^'; break;
        case '7': vkeyval = '&'; break;
        case '8': vkeyval = '*'; break;
        case '9': vkeyval = '('; break;
        case '0': vkeyval = ')'; break;
      }
    }
  }
  if ((scancode >= SDLK_KP0) && (scancode <= SDLK_KP9))
  {
    if (numlockptr)
    {
      accept = 1;
      vkeyval = scancode - SDLK_KP0 + '0';
    }
    else
    {

      switch (scancode)
      {
        case SDLK_KP9: vkeyval = 256 + 73; accept = 1; break;
        case SDLK_KP8: vkeyval = 256 + 72; accept = 1; break;
        case SDLK_KP7: vkeyval = 256 + 71; accept = 1; break;
        case SDLK_KP6: vkeyval = 256 + 77; accept = 1; break;
        case SDLK_KP5: vkeyval = 256 + 76; accept = 1; break;
        case SDLK_KP4: vkeyval = 256 + 75; accept = 1; break;
        case SDLK_KP3: vkeyval = 256 + 81; accept = 1; break;
        case SDLK_KP2: vkeyval = 256 + 80; accept = 1; break;
        case SDLK_KP1: vkeyval = 256 + 79; accept = 1; break;
      }
    }    // end no-numlock
  }      // end testing of keypad
  if (!shiftptr)
  {
    switch (scancode)
    {
      case SDLK_MINUS:        vkeyval = '-'; accept = 1; break;
      case SDLK_EQUALS:       vkeyval = '='; accept = 1; break;
      case SDLK_LEFTBRACKET:  vkeyval = '['; accept = 1; break;
      case SDLK_RIGHTBRACKET: vkeyval = ']'; accept = 1; break;
      case SDLK_SEMICOLON:    vkeyval = ';'; accept = 1; break;
      case SDLK_COMMA:        vkeyval = ','; accept = 1; break;
      case SDLK_PERIOD:       vkeyval = '.'; accept = 1; break;
      case SDLK_SLASH:        vkeyval = '/'; accept = 1; break;
      case SDLK_QUOTE:        vkeyval = '`'; accept = 1; break;
    }
  }
  else
  {
    switch (scancode)
    {
      case SDLK_MINUS:        vkeyval = '_'; accept = 1; break;
      case SDLK_EQUALS:       vkeyval = '+'; accept = 1; break;
      case SDLK_LEFTBRACKET:  vkeyval = '{'; accept = 1; break;
      case SDLK_RIGHTBRACKET: vkeyval = '}'; accept = 1; break;
      case SDLK_SEMICOLON:    vkeyval = ':'; accept = 1; break;
      case SDLK_QUOTE:        vkeyval = '"'; accept = 1; break;
      case SDLK_COMMA:        vkeyval = '<'; accept = 1; break;
      case SDLK_PERIOD:       vkeyval = '>'; accept = 1; break;
      case SDLK_SLASH:        vkeyval = '?'; accept = 1; break;
      case SDLK_BACKQUOTE:    vkeyval = '~'; accept = 1; break;
      case SDLK_BACKSLASH:    vkeyval = '|'; accept = 1; break;
    }
  }
  switch (scancode)
  {
    case SDLK_PAGEUP:      vkeyval = 256 + 73; accept = 1; break;
    case SDLK_UP:          vkeyval = 256 + 72; accept = 1; break;
    case SDLK_HOME:        vkeyval = 256 + 71; accept = 1; break;
    case SDLK_RIGHT:       vkeyval = 256 + 77; accept = 1; break;
    case SDLK_LEFT:        vkeyval = 256 + 75; accept = 1; break;
    case SDLK_PAGEDOWN:    vkeyval = 256 + 81; accept = 1; break;
    case SDLK_DOWN:        vkeyval = 256 + 80; accept = 1; break;
    case SDLK_END:         vkeyval = 256 + 79; accept = 1; break;
    case SDLK_KP_PLUS:     vkeyval = '+'; accept = 1; break;
    case SDLK_KP_MINUS:    vkeyval = '-'; accept = 1; break;
    case SDLK_KP_MULTIPLY: vkeyval = '*'; accept = 1; break;
    case SDLK_KP_DIVIDE:   vkeyval = '/'; accept = 1; break;
    case SDLK_KP_PERIOD:   vkeyval = '.'; accept = 1; break;
  }

  if (accept)
  {
    KeyBuffer[CurKeyPos] = vkeyval;
    CurKeyPos++;
    if (CurKeyPos == 16)
      CurKeyPos = 0;
  }
}

BOOL InitJoystickInput(void)
{
  int i, max_num_joysticks;
  int num_axes, num_buttons, num_hats, num_balls;
  int next_offset = 256;

  for (i = 0; i < 5; i++)
    JoystickInput[i] = NULL;

  // If it is possible to use SDL_NumJoysticks
  // before initialising SDL_INIT_JOYSTICK then
  // this call can be replaced with SDL_InitSubSystem
  SDL_InitSubSystem (SDL_INIT_JOYSTICK);
  max_num_joysticks = SDL_NumJoysticks();
  if (!max_num_joysticks)
  {
    printf("ZSNES could not find any joysticks.\n");
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
    return FALSE;
  }
  SDL_JoystickEventState(SDL_ENABLE);

  if (max_num_joysticks > 5) max_num_joysticks = 5;

  for (i = 0; i < max_num_joysticks; i++)
  {
    JoystickInput[i] = SDL_JoystickOpen(i);
    num_axes = SDL_JoystickNumAxes(JoystickInput[i]);
    num_buttons = SDL_JoystickNumButtons(JoystickInput[i]);
    num_hats = SDL_JoystickNumHats(JoystickInput[i]);
    num_balls = SDL_JoystickNumBalls(JoystickInput[i]);
    printf("Device %i %s\n", i, SDL_JoystickName(i));
    printf("  %i axis, %i buttons, %i hats, %i balls\n",
      num_axes, num_buttons, num_hats, num_balls);

    if (next_offset >= 448)
    {
      printf("Warning: Joystick won't work.\n");
      continue;
    }

    AxisOffset[i] = next_offset;
    ButtonOffset[i] = AxisOffset[i] + num_axes * 2;
    HatOffset[i] = ButtonOffset[i] + num_buttons;
    BallOffset[i] = HatOffset[i] + num_hats * 4;
    next_offset = BallOffset[i] + num_balls * 4;

    if (next_offset > (256 + 128 + 64))
    {
      printf("Warning: Too many buttons, axes, hats and/or Balls!\n");
      printf("Warning: Joystick won't work fully.\n");
    }
  }

  return TRUE;
}

BOOL InitInput()
{
  InitJoystickInput();
  return TRUE;
}

int startgame()
{
  static bool ranonce = false;
  int status;

  if (!ranonce)
  {
    ranonce = true;

    // Start semaphore code so ZSNES multitasks nicely :)
    sem_sleep_rdy();
  }

  if (sdl_state != vid_null)
  {
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_VIDEO) < 0)
    {
      fprintf(stderr, "Could not initialize SDL: %s", SDL_GetError());
      return FALSE;
    }
    sdl_state = vid_none;
  }

  if (sdl_state == vid_soft) { sw_end(); }
#ifdef __OPENGL__
  else if (sdl_state == vid_gl) { gl_end(); }

  SDL_Init(SDL_INIT_VIDEO);

  if (UseOpenGL)
  {
    status = gl_start(WindowWidth, WindowHeight, BitDepth, FullScreen);
  }
  else
#endif
  {
    status = sw_start(WindowWidth, WindowHeight, BitDepth, FullScreen);
  }

  if (!status) return FALSE;
  sdl_state = (UseOpenGL ? vid_gl : vid_soft);

  return TRUE;
}

void Start60HZ(void)
{
  update_ticks_pc2 = UPDATE_TICKS_UDP;
  if (romispal == 1)
  {
    update_ticks_pc = UPDATE_TICKS_GAMEPAL;
  }
  else
  {
    update_ticks_pc = UPDATE_TICKS_GAME;
  }

  // Restore timer data from semaphore data
  start = sem_GetTicks();
  start2 = sem_GetTicks();
  T36HZEnabled = 0;
  T60HZEnabled = 1;
}

void Stop60HZ(void)
{
  T60HZEnabled = 0;
}

void Start36HZ(void)
{
  update_ticks_pc2 = UPDATE_TICKS_UDP;
  update_ticks_pc = UPDATE_TICKS_GUI;

  // Restore timer data from semaphore data
  start = sem_GetTicks();
  start2 = sem_GetTicks();
  T60HZEnabled = 0;
  T36HZEnabled = 1;
}

void Stop36HZ(void)
{
  T36HZEnabled = 0;
}

void init_hqNx(void)
{
  DWORD color32;
  DWORD *p;
  int i, j, k, r, g, b, Y, u, v;

  for(i = 0, p = BitConv32Ptr; i < 65536; i++, p++) {
    color32=((i&0xF800)<<8)+
            ((i&0x07E0)<<5)+
            ((i&0x001F)<<3)+0xFF000000;

    *p = color32;
  }

  for (i=0; i<32; i++)
  for (j=0; j<64; j++)
  for (k=0; k<32; k++) {
    r = i << 3;
    g = j << 2;
    b = k << 3;
    Y = (r + g + b) >> 2;
    u = 128 + ((r - b) >> 2);
    v = 128 + ((-r + 2*g -b)>>3);
    RGBtoYUVPtr[(i << 11) + (j << 5) + k] = (Y<<16) + (u<<8) + v;
  }
}

unsigned char prevNTSCMode = 0;
unsigned char changeRes = 1;
unsigned char prevKeep4_3Ratio = 0;

bool OGLModeCheck()
{
   return(cvidmode > 4);
}

void initwinvideo(void)
{
  DWORD newmode = 0;

  init_hqNx();

  if ((CurMode != cvidmode) || (prevNTSCMode != NTSCFilter) ||
      (changeRes) || (prevKeep4_3Ratio != Keep4_3Ratio))
  {
    CurMode = cvidmode;
    newmode = 1;
    WindowWidth = 256;
    WindowHeight = 224;
    prevNTSCMode = NTSCFilter;
    changeRes = 0;
    prevKeep4_3Ratio = Keep4_3Ratio;

    FullScreen = GUIWFVID[cvidmode];
#ifdef __OPENGL__
    UseOpenGL = 0;
    if (OGLModeCheck())
       UseOpenGL = 1;

    if ((cvidmode == 20) || (cvidmode == 21) || (cvidmode == 22))
    {
       SetHQx(CustomResX,CustomResY);
       SetHiresOpt(CustomResX,CustomResY);
    }
#else
    if (OGLModeCheck())
      cvidmode = 2; // set it to the default 512x448 W
#endif

    switch (cvidmode)
    {
      default:
      case 0:
      case 1:
        WindowWidth = 256;
        WindowHeight = 224;
        break;
      case 2:
      case 3:
      case 6:
        if (NTSCFilter)
        {
           WindowWidth = 602;
           WindowHeight = 446;
        }
        else
        {
           WindowWidth = 512;
           WindowHeight = 448;
        }
        break;
      case 4:
      case 7:
      case 8:
        WindowWidth = 640;
        WindowHeight = 480;
        break;
      case 9:
        WindowWidth = 640;
        WindowHeight = 560;
        break;
      case 10:
        WindowWidth = 768;
        WindowHeight = 672;
        break;
      case 11:
      case 12:
        WindowWidth = 800;
        WindowHeight = 600;
        break;
      case 13:
        WindowWidth = 896;
        WindowHeight = 784;
        break;
      case 14:
      case 15:
        WindowWidth = 1024;
        WindowHeight = 768;
        break;
      case 16:
        WindowWidth = 1024;
        WindowHeight = 896;
        break;
      case 17:
        WindowWidth = 1280;
        WindowHeight = 960;
        break;
      case 18:
        WindowWidth = 1280;
        WindowHeight = 1024;
        break;
      case 19:
        WindowWidth = 1600;
        WindowHeight = 1200;
        break;
      case 20: // Variable ODR
      case 21: // Variable ODS
      case 22: // Custom Res
        WindowWidth = CustomResX;
        WindowHeight = CustomResY;
        break;
    }
    adjustMouseXScale();
    adjustMouseYScale();
  }

  if (startgame() != TRUE)
  {
    /* Exit zsnes if SDL could not be initialized */
    if (sdl_state == vid_null)
      exit(0);
    else
      return;
  }

  if (newmode == 1)
  {
    #ifdef __OPENGL__
    if(OGLModeCheck())
    {
      surface = SDL_SetVideoMode(WindowWidth, WindowHeight, BitDepth, surface->flags);
      adjustMouseXScale();
      adjustMouseYScale();
      glViewport(0,0, WindowWidth, WindowHeight);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();

      if (cvidmode == 20)
      {
        if (224*WindowWidth > 256*WindowHeight && WindowHeight)
        {
          glOrtho (- ((float) 224*WindowWidth)/((float) 256*WindowHeight),
            ((float) 224*WindowWidth)/((float) 256*WindowHeight), -1, 1, -1, 1);
        }
        else if (224*WindowWidth < 256*WindowHeight && WindowWidth)
        {
          glOrtho (-1, 1,- ((float) 256*WindowHeight)/((float) 224*WindowWidth),
            ((float) 256*WindowHeight)/((float) 224*WindowWidth), -1, 1);
        }
        else
        {
          glOrtho (-1, 1, -1, 1, -1, 1);
        }
      }

      if (Keep4_3Ratio && ((cvidmode == 21)||(cvidmode == 22)))
      {
        if (3*WindowWidth > 4*WindowHeight && WindowHeight)
        {
          glOrtho (- ((float) 3*WindowWidth)/((float) 4*WindowHeight),
            ((float) 3*WindowWidth)/((float) 4*WindowHeight), -1, 1, -1, 1);
        }
        else if (3*WindowWidth < 4*WindowHeight && WindowWidth)
        {
          glOrtho (-1, 1,- ((float) 4*WindowHeight)/((float) 3*WindowWidth),
            ((float) 4*WindowHeight)/((float) 3*WindowWidth), -1, 1);
        }
        else
        {
          glOrtho (-1, 1, -1, 1, -1, 1);
        }
      }

      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      glDisable(GL_DEPTH_TEST);
      glFlush();
    }
    #endif
    clearwin();
  }

  if (FirstVid == 1)
  {
    FirstVid = 0;

    InitSound();
    InitInput();
  }

  if (((PrevStereoSound != StereoSound) || (PrevSoundQuality != SoundQuality)))
  {
    InitSound();
  }
}

void CheckTimers(void)
{
  //QueryPerformanceCounter((LARGE_INTEGER*)&end2);
  end2 = sem_GetTicks();

  while ((end2 - start2) >= update_ticks_pc2)
  {
    start2 += update_ticks_pc2;
  }

  if (T60HZEnabled)
  {
    //QueryPerformanceCounter((LARGE_INTEGER*)&end);
    end = sem_GetTicks();

    while ((end - start) >= update_ticks_pc)
    {
      Game60hzcall();
      SDL_SemPost(sem_frames);
      start += update_ticks_pc;
    }
  }

  if (T36HZEnabled)
  {
    //QueryPerformanceCounter((LARGE_INTEGER*)&end);
    end = sem_GetTicks();

    while ((end - start) >= update_ticks_pc)
    {
      GUI36hzcall();
      start += update_ticks_pc;
    }
  }
}

void sem_sleep(void)
{
  end = update_ticks_pc - (sem_GetTicks() - start) - .2f;
  if (end>0.f) SDL_SemWaitTimeout(sem_frames, (int)end);
}

static SDL_Thread *sem_threadid = NULL;
static int sem_threadrun;

int sem_thread(void *param)
{
  while (sem_threadrun)
  {
    if (T60HZEnabled)
    {
      SDL_SemPost(sem_frames);
      usleep(romispal ? 2000 : 1000);
    }
    else
      usleep(20000);
  }
  return(0);
}

void sem_sleep_rdy(void)
{
  if (sem_frames) return;
  sem_frames = SDL_CreateSemaphore(0);
  sem_threadrun = 1;
  sem_threadid = SDL_CreateThread(sem_thread, 0);
}

void sem_sleep_die(void)
{
  if (sem_threadid)
  {
    sem_threadrun = 0;
    SDL_WaitThread(sem_threadid, NULL);
    sem_threadid = NULL;
  }
  if (sem_frames)
  {
    SDL_DestroySemaphore(sem_frames);
    sem_frames = NULL;
  }
}

void UpdateVFrame(void)
{
  //Quick fix for GUI CPU usage
  if (GUIOn || GUIOn2 || EMUPause) { usleep(6000); }

  CheckTimers();
  Main_Proc();

  if (sound_sdl)
  {
    SoundWrite_sdl();
  }
}

void clearwin()
{
  /* If we're vid_null and we get here, there's a problem */
  /* elsewhere - DDOI */
  if (sdl_state == vid_none) return;

#ifdef __OPENGL__
  if (UseOpenGL)
    gl_clearwin();
  else
#endif
    sw_clearwin();
}

void drawscreenwin(void)
{
#ifdef __LIBAO__
  extern bool RawDumpInProgress;
  if (!sound_sdl && !GUIOn2 && !GUIOn && !EMUPause && !RawDumpInProgress)
  {
    SoundWrite_ao();
  }
#endif

  /* Just in case - DDOI */
  if (sdl_state == vid_none) return;

#ifdef __OPENGL__
  if (UseOpenGL)
    gl_drawwin();
  else
#endif
    sw_drawwin();
}

void UnloadSDL()
{
  DeinitSound();
  sem_sleep_die(); // Shutdown semaphore
  if (sdl_state == vid_soft) { sw_end(); }
#ifdef __OPENGL__
  else if (sdl_state == vid_gl) { gl_end(); }
#endif
  if (sdl_state != vid_null)
  {
    SDL_WM_GrabInput(SDL_GRAB_OFF); // probably redundant
    SDL_FreeSurface(surface);
  }
  SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
  SDL_QuitSubSystem(SDL_INIT_VIDEO);
  SDL_Quit();
}

int GetMouseX(void)
{
  return ((int) MouseX);
}
int GetMouseY(void)
{
  return ((int) MouseY);
}

int GetMouseMoveX(void)
{
  //   InputRead();
  //SDL_GetRelativeMouseState(&MouseMove2X, NULL);
  SDL_GetRelativeMouseState(&MouseMove2X, &MouseMove2Y);
  return (MouseMove2X);
}

int GetMouseMoveY(void)
{
  return (MouseMove2Y);
}
int GetMouseButton(void)
{
  return ((int) MouseButton);
}

void SetMouseMinX(int MinX)
{
  MouseMinX = MinX;
  adjustMouseXScale();
}
void SetMouseMaxX(int MaxX)
{
  MouseMaxX = MaxX;
  adjustMouseXScale();
}
void SetMouseMinY(int MinY)
{
  MouseMinY = MinY;
  adjustMouseYScale();
}
void SetMouseMaxY(int MaxY)
{
  MouseMaxY = MaxY;
  adjustMouseYScale();
}
void SetMouseX(int X)
{
  MouseX = X;
}
void SetMouseY(int Y)
{
  MouseY = Y;
}

/* evul, maybe should use something other than constructor method */
void __attribute__ ((stdcall, constructor)) sem_StartTicks()
{
  gettimeofday(&sem_start, NULL);
}

float sem_GetTicks()
{
  struct timeval now;
  float ticks;

  gettimeofday(&now, NULL);
  ticks=((float)(now.tv_sec-sem_start.tv_sec))*1000.f+((float)(now.tv_usec-sem_start.tv_usec))*.001f;
  return(ticks);
}



void LaunchBrowser(char *browser, char *url)
{
  char *const arglist[] = { browser, url, 0 };
  execvp(browser, arglist);
}

void LaunchURL(char *url)
{
  if (safe_fork(0, 0)) //If fork failed, or we are the parent
  {
    MouseX = 0;
    MouseY = 0;
    return;
  }

  //We are now the child proccess

  //If any of these LaunchBrowser() calls return that means it failed and we should try the next one
  LaunchBrowser("mozilla", url);
  LaunchBrowser("mozilla-firefox", url);
  LaunchBrowser("firefox", url);
  LaunchBrowser("konqueror", url);
  LaunchBrowser("opera", url);
  LaunchBrowser("lynx", url);
  LaunchBrowser("links", url);

  _exit(0); //All browser launches failed, oh well
}

void ZsnesPage()
{
  LaunchURL("http://www.zsnes.com/");
}

void DocsPage()
{
  LaunchURL("http://zsnes-docs.sourceforge.net/");
}

