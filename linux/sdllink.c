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

#include "../gblhdr.h"
#include "gl_draw.h"
#include "sw_draw.h"

#include <stdbool.h>
#include <sys/time.h>
#include <time.h>

#include "../c_intrf.h"
#include "../cfg.h"
#include "../cpu/execute.h"
#include "../gui/c_gui.h"
#include "../gui/gui.h"
#include "../gui/guimouse.h"
#include "../initc.h"
#include "../input.h"
#include "../intrf.h"
#include "../link.h"
#include "../ui.h"
#include "../video/procvidc.h"
#include "../zip/zpng.h"
#include "audio.h"
#include "safelib.h"
#include "sdllink.h"

#ifdef __OPENGL__
#include "gl_draw.h"
#endif

#ifdef QT_DEBUGGER
#include "debugger/load.h"
#endif

_Noreturn void zexit_error(void);

typedef enum {
    FALSE = 0,
    TRUE = 1
} BOOL;

typedef enum vidstate_e {
    vid_null,
    vid_none,
    vid_soft,
    vid_gl
} vidstate_t;

/* VIDEO VARIABLES */
SDL_Window* sdl_window = NULL;
SDL_Surface* surface;
int SurfaceLocking = 0;
int SurfaceX, SurfaceY;
static uint32_t WindowWidth = 256;
static uint32_t WindowHeight = 224;
static uint32_t FullScreen = 0;
static vidstate_t sdl_state = vid_null;
static int UseOpenGL = 0;
static const int BitDepth = 16;
static uint32_t FirstVid = 1;
#ifdef __OPENGL__
SDL_GLContext gl_context = NULL;
#endif

extern unsigned char* BitConv32Ptr;
extern unsigned char* RGBtoYUVPtr;

/* JOYSTICK AND KEYBOARD INPUT */
static SDL_Joystick* JoystickInput[5];
static SDL_JoystickID JoystickID[5];
static unsigned int AxisOffset[5] = { 256 + 128 + 64 }; // per joystick offsets in
static unsigned int ButtonOffset[5] = { 448 }; // pressed. We have 128 + 64
static unsigned int HatOffset[5] = { 448 }; // bytes for all joysticks. We
// joystick balls are gone in SDL3
static int shiftptr = 0;
static int offset;
uint32_t numlockptr;

static int joystick_index_from_id(SDL_JoystickID id)
{
    int i;
    for (i = 0; i < 5; i++) {
        if (JoystickInput[i] && JoystickID[i] == id) {
            return i;
        }
    }
    return -1;
}

/* MOUSE INPUT */
static float MouseMinX = 0;
static float MouseMaxX = 256;
static float MouseMinY = 0;
static float MouseMaxY = 223;
static int MouseX, MouseY;
static int MouseMove2X, MouseMove2Y;
u1 MouseButton;
static float MouseXScale = 1.0;
static float MouseYScale = 1.0;
static uint32_t CurMode = -1;

static uint8_t IsActivated = 1;

/* TIMER VARIABLES/MACROS */
// millisecond per world update
#define UPDATE_TICKS_GAME (1000.0 / 59.948743718592964824120603015060)
#define UPDATE_TICKS_GAMEPAL (20.0)
#define UPDATE_TICKS_GUI (1000.0 / 36.0)
#define UPDATE_TICKS_UDP (1000.0 / 60.0)

static int T60HZEnabled = 0;
u1 T36HZEnabled = 0;
static float end;
static float end2;
static float start;
static float start2;
static float update_ticks_pc;
static float update_ticks_pc2;

// Used for semaphore code
static SDL_Semaphore* sem_frames = NULL;
static struct timeval sem_start;

void Game60hzcall();
u8 copymaskRB = UINT64_C(0x001FF800001FF800);
u8 copymaskG = UINT64_C(0x0000FC000000FC00);
u8 copymagic = UINT64_C(0x0008010000080100);

static void adjustMouseXScale()
{
    MouseXScale = (MouseMaxX - MouseMinX) / ((float)WindowWidth);
}

static void adjustMouseYScale()
{
    MouseYScale = (MouseMaxY - MouseMinY) / ((float)WindowHeight);
}

#ifdef __OPENGL__
// Point the GL viewport at a w*h drawable and set the projection, correcting
// the aspect ratio for the variable (20) and custom (21/22) video modes. Pass
// the real drawable size in pixels: in fullscreen it differs from
// WindowWidth/Height and has to come from SDL_GetWindowSizeInPixels once the
// window change has settled.
static void SetGLViewport(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (cvidmode == 20) {
        if (224 * w > 256 * h && h) {
            glOrtho(-((float)224 * w) / ((float)256 * h),
                ((float)224 * w) / ((float)256 * h), -1, 1, -1, 1);
        } else if (224 * w < 256 * h && w) {
            glOrtho(-1, 1, -((float)256 * h) / ((float)224 * w),
                ((float)256 * h) / ((float)224 * w), -1, 1);
        } else {
            glOrtho(-1, 1, -1, 1, -1, 1);
        }
    }

    if (Keep4_3Ratio && ((cvidmode == 21) || (cvidmode == 22))) {
        if (3 * w > 4 * h && h) {
            glOrtho(-((float)3 * w) / ((float)4 * h),
                ((float)3 * w) / ((float)4 * h), -1, 1, -1, 1);
        } else if (3 * w < 4 * h && w) {
            glOrtho(-1, 1, -((float)4 * h) / ((float)3 * w),
                ((float)4 * h) / ((float)3 * w), -1, 1);
        } else {
            glOrtho(-1, 1, -1, 1, -1, 1);
        }
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glFlush();
}
#endif

void SetHQx(unsigned int ResX, unsigned int ResY)
{
    int maxHQ;
    if (ResX / 256 < ResY / 224) {
        maxHQ = ResX / 256;
    } else {
        maxHQ = ResY / 224;
    }

    if (maxHQ >= 2) {
        GUIHQ2X[cvidmode] = 1;
        GUIHQ3X[cvidmode] = 0;
        GUIHQ4X[cvidmode] = 0;
    } else {
        GUIHQ2X[cvidmode] = 0;
        GUIHQ3X[cvidmode] = 0;
        GUIHQ4X[cvidmode] = 0;
    }
}

void SetHiresOpt(unsigned int ResX, unsigned int ResY)
{
    if (ResX >= 512 && ResY >= 448) {
        GUIM7VID[cvidmode] = 1;
    } else {
        GUIM7VID[cvidmode] = 0;
    }
}

static unsigned int sdl_keysym_to_pc_scancode(int sym);
static void ProcessKeyBuf(int scancode);

int Main_Proc()
{
    SDL_Event event;
    unsigned int key;

#ifdef QT_DEBUGGER
    if (debugger_quit) {
        debug_exit(0);
    }
#endif

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
            IsActivated = 1;
            break;
        case SDL_EVENT_WINDOW_FOCUS_LOST:
            IsActivated = 0;
            // Drop held key state, otherwise a key released while the window
            // is unfocused never gets a KEY_UP event and stays "pressed",
            // hanging the wait-for-release loop in guipresstestb (Set Keys).
            memset(pressed, 0, sizeof(pressed));
            shiftptr = 0;
            break;
#ifdef __OPENGL__
        case SDL_EVENT_WINDOW_RESIZED:
            if (UseOpenGL && GUIRESIZE[cvidmode]) {
                WindowWidth = SurfaceX = event.window.data1;
                WindowHeight = SurfaceY = event.window.data2;
                SetHQx(SurfaceX, SurfaceY);
                SetHiresOpt(SurfaceX, SurfaceY);
                adjustMouseXScale();
                adjustMouseYScale();
                SetGLViewport(WindowWidth, WindowHeight);
                gl_clearwin();
                Clear2xSaIBuffer();
            }
            break;
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            // Wayland applies the fullscreen drawable size asynchronously, so
            // re-fit the viewport whenever the pixel size actually changes
            // rather than trusting the size right after the toggle. Modes 1, 3
            // and 4 are fullscreen *software* modes, where FullScreen alone
            // would call gl_clearwin() with no context.
            if (UseOpenGL && FullScreen) {
                SetGLViewport(event.window.data1, event.window.data2);
                gl_clearwin();
                Clear2xSaIBuffer();
            }
            break;
#endif
        case SDL_EVENT_KEY_DOWN:
            if ((event.key.key == SDLK_RETURN) && (event.key.mod & SDL_KMOD_ALT) && !event.key.repeat) {
                SwitchFullScreen();
                break;
            }
            if (event.key.key == SDLK_LSHIFT || event.key.key == SDLK_RSHIFT) {
                shiftptr = 1;
            }
            if (event.key.mod & SDL_KMOD_NUM) {
                numlockptr = 1;
            } else {
                numlockptr = 0;
            }

            key = sdl_keysym_to_pc_scancode(event.key.key);
            if (key < 448) {
                pressed[key] = 1;
                ProcessKeyBuf(event.key.key);
            }
            break;

        case SDL_EVENT_KEY_UP:
            if (event.key.key == SDLK_LSHIFT || event.key.key == SDLK_RSHIFT) {
                shiftptr = 0;
            }
            key = sdl_keysym_to_pc_scancode(event.key.key);
            if (key < 448) {
                pressed[key] = 0;
            }
            break;

        case SDL_EVENT_MOUSE_MOTION:
            if (FullScreen) {
                MouseX += (int)event.motion.xrel;
                MouseY += (int)event.motion.yrel;
            } else {
                MouseX = ((int)event.motion.x * MouseXScale);
                MouseY = ((int)event.motion.y * MouseYScale);
            }

            if (MouseX < MouseMinX) {
                MouseX = MouseMinX;
            }
            if (MouseX > MouseMaxX) {
                MouseX = MouseMaxX;
            }
            if (MouseY < MouseMinY) {
                MouseY = MouseMinY;
            }
            if (MouseY > MouseMaxY) {
                MouseY = MouseMaxY;
            }
            break;

        case SDL_EVENT_MOUSE_WHEEL:
            if (event.wheel.y > 0)
                ProcessKeyBuf(SDLK_UP);
            else if (event.wheel.y < 0)
                ProcessKeyBuf(SDLK_DOWN);
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            switch (event.button.button) {
            case SDL_BUTTON_RIGHT:
                MouseButton |= 2;
                break;
            case SDL_BUTTON_MIDDLE:
                ProcessKeyBuf(SDLK_RETURN);
                // Yes, this is intentional - DDOI
            case SDL_BUTTON_LEFT:
                MouseButton |= event.button.button;
                break;
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_UP:
            switch (event.button.button) {
            case SDL_BUTTON_LEFT:
            case SDL_BUTTON_MIDDLE:
                MouseButton &= ~event.button.button;
                break;

            case SDL_BUTTON_RIGHT:
                MouseButton &= ~2;
                break;
            }
            break;

        case SDL_EVENT_JOYSTICK_HAT_MOTION: {
            int idx = joystick_index_from_id(event.jhat.which);
            if (idx < 0) {
                break;
            }
            // POV hats act as direction pad
            offset = HatOffset[idx];
            if (offset >= (256 + 128 + 64)) {
                break;
            }
            switch (event.jhat.value) {
            case SDL_HAT_CENTERED:
                pressed[offset] = 0;
                pressed[offset + 1] = 0;
                pressed[offset + 2] = 0;
                pressed[offset + 3] = 0;
                break;
            case SDL_HAT_UP:
                pressed[offset + 3] = 1;
                pressed[offset + 2] = 0;
                pressed[offset + 1] = 0;
                pressed[offset + 0] = 0;
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
                pressed[offset + 2] = 0;
                pressed[offset + 3] = 0;
                break;
            case SDL_HAT_RIGHTDOWN:
                pressed[offset] = 1;
                pressed[offset + 2] = 1;
                pressed[offset + 1] = 0;
                pressed[offset + 3] = 0;
                break;
            case SDL_HAT_DOWN:
                pressed[offset + 2] = 1;
                pressed[offset + 0] = 0;
                pressed[offset + 1] = 0;
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
                pressed[offset + 2] = 0;
                pressed[offset + 3] = 0;
                break;
            case SDL_HAT_LEFTUP:
                pressed[offset + 1] = 1;
                pressed[offset + 3] = 1;
                pressed[offset] = 0;
                pressed[offset + 2] = 0;
                break;
            }
            break;
        }

        case SDL_EVENT_JOYSTICK_AXIS_MOTION: {
            int idx = joystick_index_from_id(event.jaxis.which);
            if (idx < 0) {
                break;
            }
            offset = AxisOffset[idx];
            offset += event.jaxis.axis * 2;
            if (offset >= (256 + 128 + 64)) {
                break;
            }
            if (event.jaxis.value < -(joy_sensitivity)) {
                pressed[offset + 1] = 1;
                pressed[offset + 0] = 0;
            } else if (event.jaxis.value > joy_sensitivity) {
                pressed[offset + 0] = 1;
                pressed[offset + 1] = 0;
            } else {
                pressed[offset + 0] = 0;
                pressed[offset + 1] = 0;
            }
            break;
        }

        case SDL_EVENT_JOYSTICK_BUTTON_DOWN: {
            int idx = joystick_index_from_id(event.jbutton.which);
            if (idx < 0) {
                break;
            }
            offset = ButtonOffset[idx];
            offset += event.jbutton.button;
            if (offset >= (256 + 128 + 64)) {
                break;
            }
            pressed[offset] = 1;
            break;
        }

        case SDL_EVENT_JOYSTICK_BUTTON_UP: {
            int idx = joystick_index_from_id(event.jbutton.which);
            if (idx < 0) {
                break;
            }
            offset = ButtonOffset[idx];
            offset += event.jbutton.button;
            if (offset >= (256 + 128 + 64)) {
                break;
            }
            pressed[offset] = 0;
            break;
        }
        case SDL_EVENT_QUIT:
            zexit();
            break;
        default:
            break;
        }
    }

    return TRUE;
}

static unsigned int sdl_keysym_to_pc_scancode(int sym)
{
    switch (sym) {
    case SDLK_ESCAPE:
        return 0x01;
    case SDLK_1:
        return 0x02;
    case SDLK_2:
        return 0x03;
    case SDLK_3:
        return 0x04;
    case SDLK_4:
        return 0x05;
    case SDLK_5:
        return 0x06;
    case SDLK_6:
        return 0x07;
    case SDLK_7:
        return 0x08;
    case SDLK_8:
        return 0x09;
    case SDLK_9:
        return 0x0a;
    case SDLK_0:
        return 0x0b;
    case SDLK_MINUS:
        return 0x0c;
    case SDLK_EQUALS:
        return 0x0d;
    case SDLK_BACKSPACE:
        return 0x0e;
    case SDLK_TAB:
        return 0x0f;
    case SDLK_Q:
        return 0x10;
    case SDLK_W:
        return 0x11;
    case SDLK_E:
        return 0x12;
    case SDLK_R:
        return 0x13;
    case SDLK_T:
        return 0x14;
    case SDLK_Y:
        return 0x15;
    case SDLK_U:
        return 0x16;
    case SDLK_I:
        return 0x17;
    case SDLK_O:
        return 0x18;
    case SDLK_P:
        return 0x19;
    case SDLK_LEFTBRACKET:
        return 0x1a;
    case SDLK_RIGHTBRACKET:
        return 0x1b;
    case SDLK_RETURN:
        return 0x1c;
    case SDLK_LCTRL:
        return 0x1d;
    case SDLK_A:
        return 0x1e;
    case SDLK_S:
        return 0x1f;
    case SDLK_D:
        return 0x20;
    case SDLK_F:
        return 0x21;
    case SDLK_G:
        return 0x22;
    case SDLK_H:
        return 0x23;
    case SDLK_J:
        return 0x24;
    case SDLK_K:
        return 0x25;
    case SDLK_L:
        return 0x26;
    case SDLK_SEMICOLON:
        return 0x27;
    case SDLK_APOSTROPHE:
        return 0x28;
    case SDLK_GRAVE:
    case SDLK_HASH:
        return 0x29;
    case SDLK_LSHIFT:
        return 0x2a;
    case SDLK_BACKSLASH:
        return 0x2b;
    case SDLK_Z:
        return 0x2c;
    case SDLK_X:
        return 0x2d;
    case SDLK_C:
        return 0x2e;
    case SDLK_V:
        return 0x2f;
    case SDLK_B:
        return 0x30;
    case SDLK_N:
        return 0x31;
    case SDLK_M:
        return 0x32;
    case SDLK_COMMA:
        return 0x33;
    case SDLK_PERIOD:
        return 0x34;
    case SDLK_SLASH:
        return 0x35;
    case SDLK_RSHIFT:
        return 0x36;
    case SDLK_KP_MULTIPLY:
        return 0x37;
    case SDLK_LALT:
        return 0x38;
    case SDLK_SPACE:
        return 0x39;
    case SDLK_CAPSLOCK:
        return 0x3a;
    case SDLK_F1:
        return 0x3b;
    case SDLK_F2:
        return 0x3c;
    case SDLK_F3:
        return 0x3d;
    case SDLK_F4:
        return 0x3e;
    case SDLK_F5:
        return 0x3f;
    case SDLK_F6:
        return 0x40;
    case SDLK_F7:
        return 0x41;
    case SDLK_F8:
        return 0x42;
    case SDLK_F9:
        return 0x43;
    case SDLK_F10:
        return 0x44;
    case SDLK_NUMLOCKCLEAR:
        return 0x45;
    case SDLK_SCROLLLOCK:
        return 0x46;
    case SDLK_KP_7:
        return 0x47;
    case SDLK_KP_8:
        return 0x48;
    case SDLK_KP_9:
        return 0x49;
    case SDLK_KP_MINUS:
        return 0x4a;
    case SDLK_KP_4:
        return 0x4b;
    case SDLK_KP_5:
        return 0x4c;
    case SDLK_KP_6:
        return 0x4d;
    case SDLK_KP_PLUS:
        return 0x4e;
    case SDLK_KP_1:
        return 0x4f;
    case SDLK_KP_2:
        return 0x50;
    case SDLK_KP_3:
        return 0x51;
    case SDLK_KP_0:
        return 0x52;
    case SDLK_KP_PERIOD:
        return 0x53;
    case SDLK_F11:
        return 0x57;
    case SDLK_F12:
        return 0x58;
    case SDLK_HOME:
        return 0x59;
    case SDLK_UP:
        return 0x5a;
    case SDLK_PAGEUP:
        return 0x5b;
    case SDLK_LEFT:
        return 0x5c;
    case SDLK_RIGHT:
        return 0x5e;
    case SDLK_END:
        return 0x5f;
    case SDLK_DOWN:
        return 0x60;
    case SDLK_PAGEDOWN:
        return 0x61;
    case SDLK_INSERT:
        return 0x62;
    case SDLK_DELETE:
        return 0x63;
    case SDLK_RCTRL:
        return 0x54;
    case SDLK_RALT:
        return 0x55;
    case SDLK_KP_ENTER:
        return 0x5D;
    case SDLK_KP_DIVIDE:
        return 0x56;
    case SDLK_KP_EQUALS:
        return 0x64;
    }
    return (0x64 + sym);
}

static void ProcessKeyBuf(int scancode)
{
    int accept = 0;
    int vkeyval = 0;

    if (((scancode >= 'A') && (scancode <= 'Z')) || ((scancode >= 'a') && (scancode <= 'z')) || (scancode == SDLK_ESCAPE) || (scancode == SDLK_SPACE) || (scancode == SDLK_BACKSPACE) || (scancode == SDLK_RETURN) || (scancode == SDLK_TAB)) {
        accept = 1;
        vkeyval = scancode;
    }
    if (scancode == SDLK_KP_ENTER) {
        accept = 1;
        vkeyval = SDLK_RETURN;
    }
    if ((scancode >= '0') && (scancode <= '9')) {
        accept = 1;
        vkeyval = scancode;
        if (shiftptr) {
            switch (scancode) {
            case '1':
                vkeyval = '!';
                break;
            case '2':
                vkeyval = '@';
                break;
            case '3':
                vkeyval = '#';
                break;
            case '4':
                vkeyval = '$';
                break;
            case '5':
                vkeyval = '%';
                break;
            case '6':
                vkeyval = '^';
                break;
            case '7':
                vkeyval = '&';
                break;
            case '8':
                vkeyval = '*';
                break;
            case '9':
                vkeyval = '(';
                break;
            case '0':
                vkeyval = ')';
                break;
            }
        }
    }
    if ((scancode >= SDLK_KP_0) && (scancode <= SDLK_KP_9)) {
        if (numlockptr) {
            accept = 1;
            vkeyval = scancode - SDLK_KP_0 + '0';
        } else {
            switch (scancode) {
            case SDLK_KP_9:
                vkeyval = 256 + 73;
                accept = 1;
                break;
            case SDLK_KP_8:
                vkeyval = 256 + 72;
                accept = 1;
                break;
            case SDLK_KP_7:
                vkeyval = 256 + 71;
                accept = 1;
                break;
            case SDLK_KP_6:
                vkeyval = 256 + 77;
                accept = 1;
                break;
            case SDLK_KP_5:
                vkeyval = 256 + 76;
                accept = 1;
                break;
            case SDLK_KP_4:
                vkeyval = 256 + 75;
                accept = 1;
                break;
            case SDLK_KP_3:
                vkeyval = 256 + 81;
                accept = 1;
                break;
            case SDLK_KP_2:
                vkeyval = 256 + 80;
                accept = 1;
                break;
            case SDLK_KP_1:
                vkeyval = 256 + 79;
                accept = 1;
                break;
            }
        } // end no-numlock
    } // end testing of keypad
    if (!shiftptr) {
        switch (scancode) {
        case SDLK_MINUS:
            vkeyval = '-';
            accept = 1;
            break;
        case SDLK_EQUALS:
            vkeyval = '=';
            accept = 1;
            break;
        case SDLK_LEFTBRACKET:
            vkeyval = '[';
            accept = 1;
            break;
        case SDLK_RIGHTBRACKET:
            vkeyval = ']';
            accept = 1;
            break;
        case SDLK_SEMICOLON:
            vkeyval = ';';
            accept = 1;
            break;
        case SDLK_COMMA:
            vkeyval = ',';
            accept = 1;
            break;
        case SDLK_PERIOD:
            vkeyval = '.';
            accept = 1;
            break;
        case SDLK_SLASH:
            vkeyval = '/';
            accept = 1;
            break;
        case SDLK_APOSTROPHE:
            vkeyval = '`';
            accept = 1;
            break;
        }
    } else {
        switch (scancode) {
        case SDLK_MINUS:
            vkeyval = '_';
            accept = 1;
            break;
        case SDLK_EQUALS:
            vkeyval = '+';
            accept = 1;
            break;
        case SDLK_LEFTBRACKET:
            vkeyval = '{';
            accept = 1;
            break;
        case SDLK_RIGHTBRACKET:
            vkeyval = '}';
            accept = 1;
            break;
        case SDLK_SEMICOLON:
            vkeyval = ':';
            accept = 1;
            break;
        case SDLK_APOSTROPHE:
            vkeyval = '"';
            accept = 1;
            break;
        case SDLK_COMMA:
            vkeyval = '<';
            accept = 1;
            break;
        case SDLK_PERIOD:
            vkeyval = '>';
            accept = 1;
            break;
        case SDLK_SLASH:
            vkeyval = '?';
            accept = 1;
            break;
        case SDLK_GRAVE:
            vkeyval = '~';
            accept = 1;
            break;
        case SDLK_BACKSLASH:
            vkeyval = '|';
            accept = 1;
            break;
        }
    }
    switch (scancode) {
    case SDLK_PAGEUP:
        vkeyval = 256 + 73;
        accept = 1;
        break;
    case SDLK_UP:
        vkeyval = 256 + 72;
        accept = 1;
        break;
    case SDLK_HOME:
        vkeyval = 256 + 71;
        accept = 1;
        break;
    case SDLK_RIGHT:
        vkeyval = 256 + 77;
        accept = 1;
        break;
    case SDLK_LEFT:
        vkeyval = 256 + 75;
        accept = 1;
        break;
    case SDLK_PAGEDOWN:
        vkeyval = 256 + 81;
        accept = 1;
        break;
    case SDLK_DOWN:
        vkeyval = 256 + 80;
        accept = 1;
        break;
    case SDLK_END:
        vkeyval = 256 + 79;
        accept = 1;
        break;
    case SDLK_KP_PLUS:
        vkeyval = '+';
        accept = 1;
        break;
    case SDLK_KP_MINUS:
        vkeyval = '-';
        accept = 1;
        break;
    case SDLK_KP_MULTIPLY:
        vkeyval = '*';
        accept = 1;
        break;
    case SDLK_KP_DIVIDE:
        vkeyval = '/';
        accept = 1;
        break;
    case SDLK_KP_EQUALS:
        vkeyval = '=';
        accept = 1;
        break;
    case SDLK_KP_PERIOD:
        vkeyval = '.';
        accept = 1;
        break;
    }

    if (accept) {
        // Drop the event if the ring buffer is full, to avoid corrupting
        // CurKeyPos/CurKeyReadPos which would freeze Get_Key.
        unsigned int const next = (CurKeyPos + 1) % 16;
        if (next == CurKeyReadPos)
            return;
        KeyBuffer[CurKeyPos] = vkeyval;
        CurKeyPos++;
        if (CurKeyPos == 16) {
            CurKeyPos = 0;
        }
    }
}

BOOL InitJoystickInput()
{
    int i, max_num_joysticks, num_joysticks = 0;
    int num_axes, num_buttons, num_hats;
    int next_offset = 256;
    SDL_JoystickID* ids;

    for (i = 0; i < 5; i++) {
        JoystickInput[i] = NULL;
        JoystickID[i] = 0;
    }

    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    ids = SDL_GetJoysticks(&num_joysticks);
    if (!ids || num_joysticks <= 0) {
        printf("No joysticks found.\n");
        SDL_free(ids);
        SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
        return FALSE;
    }
    SDL_SetJoystickEventsEnabled(true);

    max_num_joysticks = num_joysticks > 5 ? 5 : num_joysticks;

    for (i = 0; i < max_num_joysticks; i++) {
        JoystickInput[i] = SDL_OpenJoystick(ids[i]);
        if (!JoystickInput[i]) {
            printf("Could not open joystick %d: %s\n", i, SDL_GetError());
            continue;
        }
        JoystickID[i] = ids[i];
        num_axes = SDL_GetNumJoystickAxes(JoystickInput[i]);
        num_buttons = SDL_GetNumJoystickButtons(JoystickInput[i]);
        num_hats = SDL_GetNumJoystickHats(JoystickInput[i]);
        printf("Device %i %s\n", i, SDL_GetJoystickName(JoystickInput[i]));
        printf("  %i axis, %i buttons, %i hats\n", num_axes, num_buttons, num_hats);

        if (next_offset >= 448) {
            printf("Warning: Joystick won't work.\n");
            continue;
        }

        AxisOffset[i] = next_offset;
        ButtonOffset[i] = AxisOffset[i] + num_axes * 2;
        HatOffset[i] = ButtonOffset[i] + num_buttons;
        next_offset = HatOffset[i] + num_hats * 4;

        if (next_offset > (256 + 128 + 64)) {
            printf("Warning: Too many buttons, axes and/or hats!\n");
            printf("Warning: Joystick won't work fully.\n");
        }
    }
    SDL_free(ids);

    return TRUE;
}

BOOL InitInput()
{
    InitJoystickInput();
    return TRUE;
}

static void sem_sleep_rdy(void);

int startgame()
{
    static bool ranonce = false;
    int status;

    if (!ranonce) {
        ranonce = true;

        gettimeofday(&sem_start, NULL);

        // Start semaphore code so ZSNES multitasks nicely :)
        sem_sleep_rdy();
    }

    if (sdl_state == vid_null) {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            fprintf(stderr, "Could not initialize SDL: %s", SDL_GetError());
            return FALSE;
        }
        sdl_state = vid_none;
    }

    if (sdl_state == vid_soft) {
#ifdef __OPENGL__
        if (UseOpenGL) {
            sw_end(); // switching software -> GL: the software window must go
        }
#endif
        // Otherwise keep the window alive so sw_start() can resize it in place
        // instead of destroying/recreating it (see sw_start in sw_draw.c).
    }
#ifdef __OPENGL__
    else if (sdl_state == vid_gl) {
        gl_end();
    }

    SDL_Init(SDL_INIT_VIDEO);

    if (UseOpenGL) {
        status = gl_start(WindowWidth, WindowHeight, BitDepth, FullScreen);
    } else
#endif
    {
        status = sw_start(WindowWidth, WindowHeight, BitDepth, FullScreen);
    }

    if (!status) {
        // The old window was torn down above; do not claim it is still there.
        sdl_state = vid_none;
        return FALSE;
    }
    sdl_state = (UseOpenGL ? vid_gl : vid_soft);

    return TRUE;
}

static float sem_GetTicks(void);

void Start60HZ(void)
{
    update_ticks_pc2 = UPDATE_TICKS_UDP;
    if (romispal == 1) {
        update_ticks_pc = UPDATE_TICKS_GAMEPAL;
    } else {
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

void Stop36HZ()
{
    T36HZEnabled = 0;
}

void init_hqNx()
{
    uint32_t color32;
    uint32_t* p;
    int i, j, k, r, g, b, Y, u, v;

    for (i = 0, p = (uint32_t*)BitConv32Ptr; i < 65536; i++, p++) {
        color32 = ((i & 0xF800) << 8) + ((i & 0x07E0) << 5) + ((i & 0x001F) << 3) + 0xFF000000;

        *p = color32;
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
                ((uint32_t*)RGBtoYUVPtr)[(i << 11) + (j << 5) + k] = (Y << 16) + (u << 8) + v;
            }
        }
    }
}

unsigned char prevNTSCMode = 0;
unsigned char changeRes = 1;
unsigned char prevKeep4_3Ratio = 0;
static unsigned char prevsync = 0;
char CheckOGLMode();

void initwinvideo(void)
{
    // A failed mode change leaves no window at all, so keep somewhere to
    // fall back to.
    static uint32_t lastGoodMode = ~0u;
    uint32_t newmode = 0;

    init_hqNx();

    if ((CurMode != cvidmode) || (prevNTSCMode != NTSCFilter) || (changeRes) || (prevKeep4_3Ratio != Keep4_3Ratio) || (prevsync != vsyncon)) {
        CurMode = cvidmode;
        newmode = 1;
        WindowWidth = 256;
        WindowHeight = 224;
        prevNTSCMode = NTSCFilter;
        changeRes = 0;
        prevKeep4_3Ratio = Keep4_3Ratio;
        prevsync = vsyncon;

        FullScreen = GUIWFVID[cvidmode];
#ifdef __OPENGL__
        UseOpenGL = 0;
        if (CheckOGLMode()) {
            UseOpenGL = 1;
        }

        if ((cvidmode == 20) || (cvidmode == 21) || (cvidmode == 22)) {
            SetHQx(CustomResX, CustomResY);
            SetHiresOpt(CustomResX, CustomResY);
        }
#else
        if (CheckOGLMode()) {
            cvidmode = 2;
        } // set it to the default 512x448 W
#endif

        switch (cvidmode) {
        default:
        case 0:
        case 1:
            WindowWidth = 256;
            WindowHeight = 224;
            break;
        case 2:
        case 3:
        case 6:
            if (NTSCFilter) {
                WindowWidth = 602;
                WindowHeight = 446;
            } else {
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
        case 20:
            // Variable ODR
        case 21:
            // Variable ODS
        case 22:
            // Custom Res
            WindowWidth = CustomResX;
            WindowHeight = CustomResY;
            break;
        }
        adjustMouseXScale();
        adjustMouseYScale();
    }

    if (startgame() != TRUE) {
        /* Exit zsnes if SDL could not be initialized */
        if (sdl_state == vid_null) {
            zexit_error();
        }
        /* Returning here would resume drawing into the destroyed window.
           The retry cannot loop: cvidmode is then already lastGoodMode. */
        if (lastGoodMode != ~0u && cvidmode != lastGoodMode) {
            fprintf(stderr, "Video mode %u failed to start, reverting to %u\n",
                (unsigned)cvidmode, (unsigned)lastGoodMode);
            cvidmode = lastGoodMode;
            CurMode = ~0u; /* force the size recompute above to run again */
            initwinvideo();
            return;
        }
        fprintf(stderr, "Could not start any video mode: %s\n", SDL_GetError());
        zexit_error();
    }
    lastGoodMode = cvidmode;

    if (newmode == 1) {
#ifdef __OPENGL__
        if (CheckOGLMode()) {
            SetGLAttributes();
            if (sdl_window) {
                SDL_SetWindowSize(sdl_window, WindowWidth, WindowHeight);
                SDL_SyncWindow(sdl_window); // settle the new size before querying it
            }
            adjustMouseXScale();
            adjustMouseYScale();

            int vp_w = (int)WindowWidth;
            int vp_h = (int)WindowHeight;
            if (FullScreen && sdl_window) {
                SDL_GetWindowSizeInPixels(sdl_window, &vp_w, &vp_h);
            }
            SetGLViewport(vp_w, vp_h);
        }
#endif
        clearwin();
        Clear2xSaIBuffer();
    }

    if (FirstVid == 1) {
        FirstVid = 0;

        InitSound();
        InitInput();

        // SDL_DisableScreenSaver();
    }

    if (((PrevStereoSound != StereoSound) || (PrevSoundQuality != SoundQuality))) {
        InitSound();
    }
}

int TryToggleFullScreen(void)
{
    if (!sdl_window) {
        return 0;
    }

    FullScreen = GUIWFVID[cvidmode];

    // Wayland resizes the GL surface rather than recreating it, so resizing in
    // place across a fullscreen transition flickers and draws the frame twice.
    // Reinit the window and context whenever the fullscreen state changes; the
    // cheap in-place path stays for mode changes that do not.
    bool const wasFullScreen = (SDL_GetWindowFlags(sdl_window) & SDL_WINDOW_FULLSCREEN) != 0;
    if (wasFullScreen != (FullScreen != 0)) {
        return 0;
    }

#ifdef __OPENGL__
    // Fall back to full reinit if the backend would change
    int newUseOpenGL = CheckOGLMode() ? 1 : 0;
    if (newUseOpenGL != UseOpenGL) {
        return 0;
    }
#endif

    CurMode = cvidmode;

    // Compute WindowWidth/WindowHeight for the new mode
    WindowWidth = 256;
    WindowHeight = 224;
    switch (cvidmode) {
    default:
    case 0:
    case 1:
        break;
    case 2:
    case 3:
    case 6:
        if (NTSCFilter) {
            WindowWidth = 602;
            WindowHeight = 446;
        } else {
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
    case 20:
    case 21:
    case 22:
        WindowWidth = CustomResX;
        WindowHeight = CustomResY;
        break;
    }

    SDL_SetWindowFullscreen(sdl_window, FullScreen ? true : false);

    if (!FullScreen) {
        SDL_SetWindowSize(sdl_window, WindowWidth, WindowHeight);
    }

    // The fullscreen/size change is asynchronous in SDL3; settle it before
    // reading the drawable size, otherwise the viewport gets sized from the
    // stale (pre-toggle) dimensions and fullscreen scaling comes out wrong.
    SDL_SyncWindow(sdl_window);

    SDL_SetWindowMouseGrab(sdl_window, FullScreen ? true : false);

    adjustMouseXScale();
    adjustMouseYScale();

#ifdef __OPENGL__
    if (CheckOGLMode()) {
        int vp_w = (int)WindowWidth;
        int vp_h = (int)WindowHeight;
        if (FullScreen) {
            SDL_GetWindowSizeInPixels(sdl_window, &vp_w, &vp_h);
        }
        SetGLViewport(vp_w, vp_h);
    }
#endif

    clearwin();
    Clear2xSaIBuffer();

    return 1;
}

void CheckTimers(void)
{
    end2 = sem_GetTicks();

    while ((end2 - start2) >= update_ticks_pc2) {
        start2 += update_ticks_pc2;
    }

    if (T60HZEnabled) {
        end = sem_GetTicks();

        while ((end - start) >= update_ticks_pc) {
            Game60hzcall();
            SDL_SignalSemaphore(sem_frames);
            start += update_ticks_pc;
        }
    }

    if (T36HZEnabled) {
        end = sem_GetTicks();

        while ((end - start) >= update_ticks_pc) {
            GUI36hzcall();
            start += update_ticks_pc;
        }
    }
}

void sem_sleep(void)
{
    end = update_ticks_pc - (sem_GetTicks() - start) - .2f;
    if (end > 0.f) {
        SDL_WaitSemaphoreTimeout(sem_frames, (Sint32)end);
    }
}

static SDL_Thread* sem_threadid = NULL;
static int sem_threadrun;

int sem_thread(void* param)
{
    while (sem_threadrun) {
        if (T60HZEnabled) {
            SDL_SignalSemaphore(sem_frames);
            usleep(romispal ? 2000 : 1000);
        } else {
            usleep(20000);
        }
    }
    return (0);
}

static void sem_sleep_rdy(void)
{
    if (sem_frames) {
        return;
    }
    sem_frames = SDL_CreateSemaphore(0);
    sem_threadrun = 1;
    sem_threadid = SDL_CreateThread(sem_thread, "sem_thread", 0);
}

static void sem_sleep_die()
{
    if (sem_threadid) {
        sem_threadrun = 0;
        SDL_WaitThread(sem_threadid, NULL);
        sem_threadid = NULL;
    }
    if (sem_frames) {
        SDL_DestroySemaphore(sem_frames);
        sem_frames = NULL;
    }
}

void DoRumble(void)
{
    extern u2 RumbleData;

    if (RumbleData == 0xFFFF) {
        RumbleData = 0;
    }

    if ((RumbleData & 0xFF00) == 0x7200) {
        u2 RumbleLeft = ((RumbleData & 0x000F) * 4369);
        u2 RumbleRight = (((RumbleData & 0x00F0) >> 4) * 4369);
        SDL_RumbleJoystick(JoystickInput[0], RumbleLeft, RumbleRight, 600);
        RumbleData = 0;
    }
}

void UpdateVFrame(void)
{
    extern u1 MultiTap;

    // Quick fix for GUI CPU usage
    if (GUIOn || GUIOn2 || EMUPause) {
        usleep(6000);
    }

    CheckTimers();
    Main_Proc();

    // Debug: ASCII_SCREENSHOT_EVERY_FIVE=1 writes a burst of consecutive
    // frames (ASCII_SCREENSHOT_BURST, default 10) to /tmp/zsnes_<seq>.txt
    // every 5s, and per-frame hashes to /tmp/zsnes_hashes.txt
    {
        static int sshot_checked = 0;
        static int sshot_enabled = 0;
        static int sshot_burst = 0;
        static int sshot_burst_len = 10;
        static Uint64 sshot_next_ms = 0;
        static unsigned int sshot_seq = 0;
        if (!sshot_checked) {
            const char* e = getenv("ASCII_SCREENSHOT_EVERY_FIVE");
            const char* b = getenv("ASCII_SCREENSHOT_BURST");
            sshot_enabled = (e && *e == '1');
            if (b && atoi(b) > 0) {
                sshot_burst_len = atoi(b);
            }
            sshot_next_ms = SDL_GetTicks() + 5000;
            sshot_checked = 1;
        }
        if (sshot_enabled) {
            Uint64 now = SDL_GetTicks();
            if (sshot_burst == 0 && now >= sshot_next_ms) {
                sshot_burst = sshot_burst_len;
                sshot_next_ms = now + 5000;
            }
            if (sshot_burst > 0) {
                sshot_burst--;
                char path[64];
                snprintf(path, sizeof(path), "/tmp/zsnes_%05u.txt", sshot_seq++);
                Grab_ASCII_Data_Path(path);
            }
            Grab_Frame_Hash_Path("/tmp/zsnes_hashes.txt");
        }
    }

    // Debug: PNG_SCREENSHOT_EVERY_N=<n> writes a full-resolution PNG
    // /tmp/zsnes_<frame>.png every N emulated frames (filmstrip for headless
    // debugging, e.g. driving DEBUG_INPUT_SCRIPT to reproduce a bug).
    // zip/zpng.h only declares the writer when the build has libpng.
#ifndef NO_PNG
    {
        static int png_checked = 0;
        static int png_every = 0;
        static unsigned int png_frame = 0;
        if (!png_checked) {
            const char* e = getenv("PNG_SCREENSHOT_EVERY_N");
            if (e && atoi(e) > 0)
                png_every = atoi(e);
            png_checked = 1;
        }
        if (png_every) {
            if (png_frame % (unsigned)png_every == 0) {
                char path[64];
                snprintf(path, sizeof(path), "/tmp/zsnes_%06u.png", png_frame);
                Grab_PNG_Data_Path(path);
            }
            png_frame++;
        }
    }
#endif

    // Debug: PPU_STATE_LOG=1 appends per-frame PPU brightness/blank/layer state
    // to /tmp/zsnes_ppu.txt (correlate with the PNG filmstrip to explain a black
    // screen: force-blank set? brightness 0? no layers enabled?).
    // Built only with WITH_DEBUG_HOOKS=1; a release build has no frame hook.
#ifdef ZSNES_DEBUG_HOOKS
    {
        extern uint8_t vidbright, forceblnk;
        extern uint16_t scrnon;
        static int ppu_checked = 0, ppu_log = 0;
        static unsigned int ppu_frame = 0;
        static FILE* ppu_fp = NULL;
        if (!ppu_checked) {
            const char* e = getenv("PPU_STATE_LOG");
            ppu_log = (e && *e == '1') ? 1 : ((e && *e == '2') ? 2 : 0);
            if (ppu_log)
                ppu_fp = fopen("/tmp/zsnes_ppu.txt", "wb");
            ppu_checked = 1;
        }
        if (ppu_log && ppu_fp) {
            if (ppu_log == 2) {
                /* PPU_STATE_LOG=2 adds emulated machine state, so a run can be
                   compared against another build frame by frame and the first
                   divergence located. The sums are order-independent on
                   purpose - they only have to change when the memory does. */
                extern uint8_t wramdataa[65536], ram7fa[65536];
                extern uint8_t SPCRAM[];
                uint32_t w = 0, r = 0, a = 0;
                unsigned i;
                for (i = 0; i < 65536; i++) {
                    w = w * 31u + wramdataa[i];
                    r = r * 31u + ram7fa[i];
                    a = a * 31u + SPCRAM[i];
                }
                {
                    extern u1* spcPCRam;
                    extern u1 spcA, spcX, spcY, spcP, spcNZ;
                    extern uint32_t spcS, spcCycle;
                    fprintf(ppu_fp,
                        "%u bright=%u blank=%02x scrnon=%04x wram=%08x ram7f=%08x spc=%08x "
                        "spcpc=%04x a=%02x x=%02x y=%02x p=%02x nz=%08x s=%04x cyc=%08x\n",
                        ppu_frame, vidbright, forceblnk, scrnon, w, r, a,
                        (unsigned)(spcPCRam - SPCRAM), (unsigned)spcA,
                        (unsigned)spcX, (unsigned)spcY,
                        (unsigned)spcP, (unsigned)spcNZ,
                        (unsigned)(spcS & 0xFFFF), (unsigned)spcCycle);
                }
                {
                    /* PPU_DUMP_FRAME=N writes work RAM at frame N, so two
                       builds can be diffed byte for byte at the frame the
                       checksums first disagree. */
                    static int dump_at = -2;
                    if (dump_at == -2) {
                        const char* d = getenv("PPU_DUMP_FRAME");
                        dump_at = d ? atoi(d) : -1;
                    }
                    if (dump_at >= 0 && (int)ppu_frame == dump_at) {
                        FILE* wf = fopen("/tmp/zsnes_wram.bin", "wb");
                        if (wf) {
                            fwrite(wramdataa, 1, 65536, wf);
                            fclose(wf);
                        }
                    }
                }
            } else {
                fprintf(ppu_fp, "%u bright=%u blank=%02x scrnon=%04x\n",
                    ppu_frame, vidbright, forceblnk, scrnon);
            }
            fflush(ppu_fp);
            {
                extern unsigned char ngd_ms[32][2], ngd_ma[32], ngd_fb[32], ngd_all[32], ngd_ch[32][4];
                extern unsigned char ngd_fs[32], ngd_win[32][4], ngd_mos[32];
                extern unsigned short ngd_sy[32][4], ngd_sx[32][4], ngd_pt[32][4], ngd_pty[32][4];
                extern unsigned ngd_pal[32], ngd_start, ngd_end, ngd_res, ngd_resl;
                unsigned i;
                fprintf(ppu_fp, "%u LINES start=%u end=%u res=%u resl=%u\n",
                    ppu_frame, ngd_start, ngd_end, ngd_res, ngd_resl);
                {
                    extern unsigned char vrama[65536];
                    extern unsigned char bgmode, bgtilesz;
                    extern unsigned short bg1ptr, bg1ptrb, bg1ptrc, bg1ptrd;
                    extern unsigned bg1ptrx[4], bg1ptry[4];
                    extern unsigned short bg1objptr, bg2objptr;
                    extern unsigned short bg1scroly[4], bg1scrolx[4];
                    static int vd = -2;
                    if (vd == -2) {
                        char const* e = getenv("VRAM_DUMP");
                        vd = e ? atoi(e) : -1;
                    }
                    if (vd >= 0 && (int)ppu_frame == vd) {
                        FILE* vf = fopen("/tmp/zsnes_vram.bin", "wb");
                        if (vf) {
                            fwrite(vrama, 1, 65536, vf);
                            fclose(vf);
                        }
                        fprintf(ppu_fp, "%u VINFO mode=%u tilesz=%02x ptr=%04x,%04x,%04x,%04x "
                                        "ptrx=%x ptry=%x obj1=%04x obj2=%04x sy=%04x sx=%04x\n",
                            ppu_frame, bgmode, bgtilesz, bg1ptr, bg1ptrb, bg1ptrc, bg1ptrd,
                            bg1ptrx[0], bg1ptry[0], bg1objptr, bg2objptr,
                            bg1scroly[0], bg1scrolx[0]);
                    }
                }
                {
                    extern unsigned dbg_regn;
                    extern unsigned short dbg_rega[8192], dbg_regy[8192];
                    extern unsigned char dbg_regv[8192];
                    unsigned k;
                    fprintf(ppu_fp, "%u WREG n=%u:", ppu_frame, dbg_regn);
                    for (k = 0; k < dbg_regn && k < 400; k++)
                        fprintf(ppu_fp, " %04x@%u=%02x", dbg_rega[k], dbg_regy[k], dbg_regv[k]);
                    fputc('\n', ppu_fp);
                    dbg_regn = 0;
                    {
                        extern unsigned dbg_hn;
                        extern void* dbg_hp[16384];
                        extern unsigned short dbg_hy[16384];
                        extern unsigned char dbg_hv[16384];
                        unsigned j;
                        fprintf(ppu_fp, "%u HDMA n=%u:", ppu_frame, dbg_hn);
                        for (j = 0; j < dbg_hn && j < 300; j++)
                            fprintf(ppu_fp, " %p@%u=%02x", dbg_hp[j], dbg_hy[j], dbg_hv[j]);
                        fputc('\n', ppu_fp);
                        dbg_hn = 0;
                    }
                }
                for (i = 0; i < 32; i++) {
                    fprintf(ppu_fp,
                        "%u L%02u ms=%02x/%02x ma=%u fb=%u all=%u fs=%02x mos=%02x pal=%08x "
                        "ch=%u%u%u%u win=%02x%02x%02x%02x sy=%04x,%04x,%04x,%04x "
                        "sx=%04x,%04x,%04x,%04x pt=%04x,%04x,%04x,%04x pty=%04x,%04x,%04x,%04x\n",
                        ppu_frame, i, ngd_ms[i][0], ngd_ms[i][1], ngd_ma[i], ngd_fb[i],
                        ngd_all[i], ngd_fs[i], ngd_mos[i], ngd_pal[i],
                        ngd_ch[i][0], ngd_ch[i][1], ngd_ch[i][2], ngd_ch[i][3],
                        ngd_win[i][0], ngd_win[i][1], ngd_win[i][2], ngd_win[i][3],
                        ngd_sy[i][0], ngd_sy[i][1], ngd_sy[i][2], ngd_sy[i][3],
                        ngd_sx[i][0], ngd_sx[i][1], ngd_sx[i][2], ngd_sx[i][3],
                        ngd_pt[i][0], ngd_pt[i][1], ngd_pt[i][2], ngd_pt[i][3],
                        ngd_pty[i][0], ngd_pty[i][1], ngd_pty[i][2], ngd_pty[i][3]);
                }
            }
        }
        {
            /* ZST_ROUNDTRIP=N: check the save-state path once, at frame N. */
            static int zst_at = -2;
            if (zst_at == -2) {
                char const* z = getenv("ZST_ROUNDTRIP");
                zst_at = z ? atoi(z) : -1;
            }
            if (zst_at >= 0 && (int)ppu_frame == zst_at) {
                extern void zst_roundtrip_check(void);
                zst_roundtrip_check();
            }
        }
        ppu_frame++;
    }
#endif

    if (SNESRumble && !MultiTap) {
        DoRumble();
    } else {
        // Stop vibration
        SDL_RumbleJoystick(JoystickInput[0], 0, 0, 1);
    }

    if (sound_sdl) {
        SoundWrite_sdl();
    }
}

void clearwin(void)
{
    /* If we're vid_null and we get here, there's a problem */
    /* elsewhere - DDOI */
    if (sdl_state == vid_none) {
        return;
    }

#ifdef __OPENGL__
    if (UseOpenGL) {
        gl_clearwin();
    } else
#endif
    {
        sw_clearwin();
    }
}

void drawscreenwin(void)
{
#if defined(__LIBAO__) || defined(__PIPEWIRE__)
    extern bool RawDumpInProgress;
    if (!sound_sdl && !GUIOn2 && !GUIOn && !EMUPause && !RawDumpInProgress) {
#ifdef __PIPEWIRE__
        if (sound_pipewire) {
            SoundWrite_pipewire();
        }
#endif
#ifdef __LIBAO__
#ifdef __PIPEWIRE__
        else
#endif
            SoundWrite_ao();
#endif
    }
#endif

    /* Just in case - DDOI */
    if (sdl_state == vid_none) {
        return;
    }

#ifdef __OPENGL__
    if (UseOpenGL) {
        gl_drawwin();
    } else
#endif
    {
        sw_drawwin();
    }
}

void UnloadSDL()
{
    DeinitSound();
    sem_sleep_die(); // Shutdown semaphore
    if (sdl_state == vid_soft) {
        sw_end();
    }
#ifdef __OPENGL__
    else if (sdl_state == vid_gl) {
        gl_end();
    }
#endif
    if (sdl_state != vid_null && sdl_window) {
        SDL_SetWindowMouseGrab(sdl_window, false);
    }
    SDL_Quit();
}

s4 GetMouseX(void)
{
    return ((int)MouseX);
}
s4 GetMouseY(void)
{
    return ((int)MouseY);
}

s4 GetMouseMoveX(void)
{
    float fx = 0.0f, fy = 0.0f;
    SDL_GetRelativeMouseState(&fx, &fy);
    MouseMove2X = (int)fx;
    MouseMove2Y = (int)fy;
    return (MouseMove2X);
}

s4 GetMouseMoveY(void)
{
    return (MouseMove2Y);
}

s4 GetMouseButton(void)
{
    return ((int)MouseButton);
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

static float sem_GetTicks(void)
{
    struct timeval now;
    float ticks;

    gettimeofday(&now, NULL);
    ticks = ((float)(now.tv_sec - sem_start.tv_sec)) * 1000.f + ((float)(now.tv_usec - sem_start.tv_usec)) * .001f;
    return (ticks);
}

void LaunchBrowser(char* browser, char* url)
{
    char* const arglist[] = { browser, url, 0 };
    execvp(browser, arglist);
}

void LaunchURL(char* url)
{
    if (safe_fork(0, 0)) // If fork failed, or we are the parent
    {
        MouseX = 0;
        MouseY = 0;
        return;
    }

    // We are now the child proccess

    // If any of these LaunchBrowser() calls return that means it failed and we should try the next one
    LaunchBrowser("mozilla", url);
    LaunchBrowser("mozilla-firefox", url);
    LaunchBrowser("firefox", url);
    LaunchBrowser("konqueror", url);
    LaunchBrowser("opera", url);
    LaunchBrowser("lynx", url);
    LaunchBrowser("links", url);

    _exit(0); // All browser launches failed, oh well
}

void ZsnesPage(void)
{
    LaunchURL("http://www.zsnes.com/");
}

void DocsPage(void)
{
    LaunchURL("http://zsnes-docs.sourceforge.net/");
}
