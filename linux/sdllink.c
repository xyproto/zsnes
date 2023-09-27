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

#include <SDL_thread.h>
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
#include "../types.h"
#include "../link.h"
#include "../ui.h"
#include "../video/newgfx16.h"
#include "../video/procvidc.h"
#include "audio.h"
#include "sdllink.h"
#include "gl_draw.h"

#ifdef QT_DEBUGGER
#include "debugger/load.h"
#endif

#ifndef _WIN32
#include <X11/Xlib.h>
#endif

void zexit_error();

typedef enum vidstate_e {
	vid_null,
	vid_none,
	vid_gl
} vidstate_t;

/* VIDEO VARIABLES */
SDL_Surface *surface;
static uint32_t WindowWidth = 256;
static uint32_t WindowHeight = 224;
static uint32_t FullScreen = 0;
static vidstate_t sdl_state = vid_null;
static const int BitDepth = 16;
static bool ScreenSaverSuspended = false;
static uint32_t FirstVid = 1;
u1 blinit;

/* JOYSTICK AND KEYBOARD INPUT */
static SDL_Joystick *JoystickInput[5];
static unsigned int AxisOffset[5] = {256 + 128 + 64}; // per joystick offsets in
static unsigned int ButtonOffset[5] = {448};		  // pressed. We have 128 + 64
static unsigned int HatOffset[5] = {448};			  // bytes for all joysticks. We
static unsigned int BallOffset[5] = {448};			  // can control all 5 players.
static int shiftptr = 0;
static int offset;
uint32_t numlockptr;

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
static SDL_sem *sem_frames = NULL;
static struct timeval sem_start;

void Game60hzcall();
u8 copymaskRB = UINT64_C(0x001FF800001FF800);
u8 copymaskG = UINT64_C(0x0000FC000000FC00);
u8 copymagic = UINT64_C(0x0008010000080100);

static void adjustMouseXScale() {
	MouseXScale = (MouseMaxX - MouseMinX) / ((float)WindowWidth);
}

static void adjustMouseYScale() {
	MouseYScale = (MouseMaxY - MouseMinY) / ((float)WindowHeight);
}

static unsigned int sdl_keysym_to_pc_scancode(int sym);
static void ProcessKeyBuf(int scancode);

void Change_Resolution_Mode() {
	printf("Updating window size\n");

	SetGLAttributes();
	surface = SDL_SetVideoMode(WindowWidth, WindowHeight, BitDepth, surface->flags);
	adjustMouseXScale();
	adjustMouseYScale();
	glViewport(0, 0, WindowWidth, WindowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// 4:3 Mode
	if (cvidmode & 1) {
		if (3 * WindowWidth > 4 * WindowHeight && WindowHeight) {
			glOrtho(-((float)3 * WindowWidth) / ((float)4 * WindowHeight), ((float)3 * WindowWidth) / ((float)4 * WindowHeight), -1, 1, -1, 1);
		} else if (3 * WindowWidth < 4 * WindowHeight && WindowWidth) {
			glOrtho(-1, 1, -((float)4 * WindowHeight) / ((float)3 * WindowWidth), ((float)4 * WindowHeight) / ((float)3 * WindowWidth), -1, 1);
		} else {
			glOrtho(-1, 1, -1, 1, -1, 1);
		}
	} else { // 8:7 SNES Mode
		if (224 * WindowWidth > 256 * WindowHeight && WindowHeight) {
			glOrtho(-((float)224 * WindowWidth) / ((float)256 * WindowHeight), ((float)224 * WindowWidth) / ((float)256 * WindowHeight), -1, 1, -1, 1);
		} else if (224 * WindowWidth < 256 * WindowHeight && WindowWidth) {
			glOrtho(-1, 1, -((float)256 * WindowHeight) / ((float)224 * WindowWidth), ((float)256 * WindowHeight) / ((float)224 * WindowWidth), -1, 1);
		} else {
			glOrtho(-1, 1, -1, 1, -1, 1);
		}
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);
	glFlush();
}

int Main_Proc() {
	SDL_Event event;
	unsigned int key;

#ifdef QT_DEBUGGER
	if (debugger_quit) {
		debug_exit(0);
	}
#endif

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_ACTIVEEVENT:
			IsActivated = event.active.gain;
			break;
		case SDL_KEYDOWN:
			if ((event.key.keysym.sym == SDLK_RETURN) && (event.key.keysym.mod & KMOD_ALT)) {
				SwitchFullScreen();
				break;
			}
			if (event.key.keysym.sym == SDLK_LSHIFT || event.key.keysym.sym == SDLK_RSHIFT) {
				shiftptr = 1;
			}
			if (event.key.keysym.mod & KMOD_NUM) {
				numlockptr = 1;
			} else {
				numlockptr = 0;
			}

			key = sdl_keysym_to_pc_scancode(event.key.keysym.sym);
			if (key < 448) {
				pressed[key] = 1;
				ProcessKeyBuf(event.key.keysym.sym);
			}
			break;

		case SDL_KEYUP:
			if (event.key.keysym.sym == SDLK_LSHIFT || event.key.keysym.sym == SDLK_RSHIFT) {
				shiftptr = 0;
			}
			key = sdl_keysym_to_pc_scancode(event.key.keysym.sym);
			if (key < 448) {
				pressed[key] = 0;
			}
			break;

		case SDL_MOUSEMOTION:
			if (FullScreen) {
				MouseX += event.motion.xrel;
				MouseY += event.motion.yrel;
			} else {
				MouseX = ((int)((float)event.motion.x) * MouseXScale);
				MouseY = ((int)((float)event.motion.y) * MouseYScale);
			}

			if (MouseX < MouseMinX) { MouseX = MouseMinX; }
			if (MouseX > MouseMaxX) { MouseX = MouseMaxX; }
			if (MouseY < MouseMinY) { MouseY = MouseMinY; }
			if (MouseY > MouseMaxY) { MouseY = MouseMaxY; }
			break;

		case SDL_MOUSEBUTTONDOWN:
			/*
		   button 2 = enter (i.e. select)
		   button 4 = mouse wheel up (treat as "up" key)
		   button 5 = mouse wheel down (treat as "down" key)
		 */
			switch (event.button.button) {
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
			switch (event.button.button) {
			case 1:
			case 2:
				MouseButton &= ~event.button.button;
				break;

			case 3:
				MouseButton &= ~2;
				break;
			}
			break;

		case SDL_JOYHATMOTION:
			// POV hats act as direction pad
			offset = HatOffset[event.jhat.which];
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

			/*
		   joystick trackball code untested; change the test
		   values if the motion is too sensitive (or not
		   sensitive enough)
		 */
		case SDL_JOYBALLMOTION:
			offset = BallOffset[event.jball.which];
			offset += event.jball.ball;
			if (offset >= (256 + 128 + 64)) {
				break;
			}
			if (event.jball.xrel < -100) {
				pressed[offset] = 0;
				pressed[offset + 1] = 1;
			}
			if (event.jball.xrel > 100) {
				pressed[offset] = 1;
				pressed[offset + 1] = 0;
			}
			if (event.jball.yrel < -100) {
				pressed[offset + 2] = 0;
				pressed[offset + 3] = 1;
			}
			if (event.jball.yrel > 100) {
				pressed[offset + 2] = 1;
				pressed[offset + 3] = 0;
			}
			break;

		case SDL_JOYAXISMOTION:
			offset = AxisOffset[event.jaxis.which];
			offset += event.jaxis.axis * 2;
			if (offset >= (256 + 128 + 64)) {
				break;
			}
			// printf("DEBUG axis offset: %d\n", offset);
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

		case SDL_JOYBUTTONDOWN:
			offset = ButtonOffset[event.jbutton.which];
			offset += event.jbutton.button;
			// printf("DEBUG button offset: %d\n", offset);
			if (offset >= (256 + 128 + 64)) {
				break;
			}
			pressed[offset] = 1;
			break;

		case SDL_JOYBUTTONUP:
			offset = ButtonOffset[event.jbutton.which];
			offset += event.jbutton.button;
			// printf("DEBUG button offset: %d\n", offset);
			if (offset >= (256 + 128 + 64)) {
				break;
			}
			pressed[offset] = 0;
			break;
		case SDL_QUIT:
			zexit();
			break;
		case SDL_VIDEORESIZE:
			WindowWidth = event.resize.w;
			WindowHeight = event.resize.h;
			Change_Resolution_Mode();
			break;
		default:
			break;
		}
	}

	return 1;
}

static unsigned int sdl_keysym_to_pc_scancode(int sym) {
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
	case SDLK_q:
		return 0x10;
	case SDLK_w:
		return 0x11;
	case SDLK_e:
		return 0x12;
	case SDLK_r:
		return 0x13;
	case SDLK_t:
		return 0x14;
	case SDLK_y:
		return 0x15;
	case SDLK_u:
		return 0x16;
	case SDLK_i:
		return 0x17;
	case SDLK_o:
		return 0x18;
	case SDLK_p:
		return 0x19;
	case SDLK_LEFTBRACKET:
		return 0x1a;
	case SDLK_RIGHTBRACKET:
		return 0x1b;
	case SDLK_RETURN:
		return 0x1c;
	case SDLK_LCTRL:
		return 0x1d;
	case SDLK_a:
		return 0x1e;
	case SDLK_s:
		return 0x1f;
	case SDLK_d:
		return 0x20;
	case SDLK_f:
		return 0x21;
	case SDLK_g:
		return 0x22;
	case SDLK_h:
		return 0x23;
	case SDLK_j:
		return 0x24;
	case SDLK_k:
		return 0x25;
	case SDLK_l:
		return 0x26;
	case SDLK_SEMICOLON:
		return 0x27;
	case SDLK_QUOTE:
		return 0x28;
	case SDLK_BACKQUOTE:
	case SDLK_HASH:
		return 0x29;
	case SDLK_LSHIFT:
		return 0x2a;
	case SDLK_BACKSLASH:
		return 0x2b;
	case SDLK_z:
		return 0x2c;
	case SDLK_x:
		return 0x2d;
	case SDLK_c:
		return 0x2e;
	case SDLK_v:
		return 0x2f;
	case SDLK_b:
		return 0x30;
	case SDLK_n:
		return 0x31;
	case SDLK_m:
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
	case SDLK_NUMLOCK:
		return 0x45;
	case SDLK_SCROLLOCK:
		return 0x46;
	case SDLK_KP7:
		return 0x47;
	case SDLK_KP8:
		return 0x48;
	case SDLK_KP9:
		return 0x49;
	case SDLK_KP_MINUS:
		return 0x4a;
	case SDLK_KP4:
		return 0x4b;
	case SDLK_KP5:
		return 0x4c;
	case SDLK_KP6:
		return 0x4d;
	case SDLK_KP_PLUS:
		return 0x4e;
	case SDLK_KP1:
		return 0x4f;
	case SDLK_KP2:
		return 0x50;
	case SDLK_KP3:
		return 0x51;
	case SDLK_KP0:
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
	}
	return (0x64 + sym);
}

static void ProcessKeyBuf(int scancode) {
	int accept = 0;
	int vkeyval = 0;

	if (((scancode >= 'A') && (scancode <= 'Z')) || ((scancode >= 'a') && (scancode <= 'z')) || (scancode == SDLK_ESCAPE) || (scancode == SDLK_SPACE) || (scancode == SDLK_BACKSPACE) || (scancode == SDLK_RETURN) || (scancode == SDLK_TAB)) {
		accept = 1;
		vkeyval = scancode;
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
	if ((scancode >= SDLK_KP0) && (scancode <= SDLK_KP9)) {
		if (numlockptr) {
			accept = 1;
			vkeyval = scancode - SDLK_KP0 + '0';
		} else {
			switch (scancode) {
			case SDLK_KP9:
				vkeyval = 256 + 73;
				accept = 1;
				break;
			case SDLK_KP8:
				vkeyval = 256 + 72;
				accept = 1;
				break;
			case SDLK_KP7:
				vkeyval = 256 + 71;
				accept = 1;
				break;
			case SDLK_KP6:
				vkeyval = 256 + 77;
				accept = 1;
				break;
			case SDLK_KP5:
				vkeyval = 256 + 76;
				accept = 1;
				break;
			case SDLK_KP4:
				vkeyval = 256 + 75;
				accept = 1;
				break;
			case SDLK_KP3:
				vkeyval = 256 + 81;
				accept = 1;
				break;
			case SDLK_KP2:
				vkeyval = 256 + 80;
				accept = 1;
				break;
			case SDLK_KP1:
				vkeyval = 256 + 79;
				accept = 1;
				break;
			}
		} // end no-numlock
	}	  // end testing of keypad
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
		case SDLK_QUOTE:
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
		case SDLK_QUOTE:
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
		case SDLK_BACKQUOTE:
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
	case SDLK_KP_PERIOD:
		vkeyval = '.';
		accept = 1;
		break;
	}

	if (accept) {
		KeyBuffer[CurKeyPos] = vkeyval;
		CurKeyPos++;
		if (CurKeyPos == 16) {
			CurKeyPos = 0;
		}
	}
}

void InitInput() {
	int i, max_num_joysticks;
	int num_axes, num_buttons, num_hats, num_balls;
	int next_offset = 256;

	for (i = 0; i < 5; i++) {
		JoystickInput[i] = NULL;
	}

	// If it is possible to use SDL_NumJoysticks
	// before initialising SDL_INIT_JOYSTICK then
	// this call can be replaced with SDL_InitSubSystem
	SDL_InitSubSystem(SDL_INIT_JOYSTICK);
	max_num_joysticks = SDL_NumJoysticks();
	if (!max_num_joysticks) {
		printf("No joysticks found.\n");
		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
		return 0;
	}
	SDL_JoystickEventState(SDL_ENABLE);

	if (max_num_joysticks > 5) {
		max_num_joysticks = 5;
	}

	for (i = 0; i < max_num_joysticks; i++) {
		JoystickInput[i] = SDL_JoystickOpen(i);
		num_axes = SDL_JoystickNumAxes(JoystickInput[i]);
		num_buttons = SDL_JoystickNumButtons(JoystickInput[i]);
		num_hats = SDL_JoystickNumHats(JoystickInput[i]);
		num_balls = SDL_JoystickNumBalls(JoystickInput[i]);
		printf("Device %i %s\n", i, SDL_JoystickName(i));
		printf("  %i axis, %i buttons, %i hats, %i balls\n", num_axes, num_buttons, num_hats, num_balls);

		if (next_offset >= 448) {
			printf("Warning: Joystick won't work.\n");
			continue;
		}

		AxisOffset[i] = next_offset;
		ButtonOffset[i] = AxisOffset[i] + num_axes * 2;
		HatOffset[i] = ButtonOffset[i] + num_buttons;
		BallOffset[i] = HatOffset[i] + num_hats * 4;
		next_offset = BallOffset[i] + num_balls * 4;

		if (next_offset > (256 + 128 + 64)) {
			printf("Warning: Too many buttons, axes, hats and/or Balls!\n");
			printf("Warning: Joystick won't work fully.\n");
		}
	}
}

static void sem_sleep_rdy(void);

int startgame() {
	static bool ranonce = false;
	int status;

	if (!ranonce) {
		ranonce = true;

		gettimeofday(&sem_start, NULL);

		// Start semaphore code so ZSNES multitasks nicely :)
		sem_sleep_rdy();
	}

	if (sdl_state != vid_null) {
		if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_VIDEO) < 0) {
			fprintf(stderr, "Could not initialize SDL: %s", SDL_GetError());
			return 0;
		}
		sdl_state = vid_none;
	}

	gl_end();
	SDL_Init(SDL_INIT_VIDEO);

	status = gl_start(WindowWidth, WindowHeight, BitDepth, FullScreen);
	if (!status) {
		return 0;
	}
	sdl_state = vid_gl;
	return 1;
}

static float sem_GetTicks(void);

void Start60HZ(void) {
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

void Stop60HZ(void) {
	T60HZEnabled = 0;
}

void Start36HZ(void) {
	update_ticks_pc2 = UPDATE_TICKS_UDP;
	update_ticks_pc = UPDATE_TICKS_GUI;

	// Restore timer data from semaphore data
	start = sem_GetTicks();
	start2 = sem_GetTicks();
	T60HZEnabled = 0;
	T36HZEnabled = 1;
}

void Stop36HZ() {
	T36HZEnabled = 0;
}

unsigned char changeRes = 1;
static unsigned char prevsync = 0;

void initwinvideo(void) {
	uint32_t newmode = 0;
	if ((CurMode != cvidmode) || changeRes || (prevsync != vsyncon)) {
		CurMode = cvidmode;
		newmode = 1;
		changeRes = 0;
		prevsync = vsyncon;
		FullScreen = cvidmode & 2;

		// This should set the new window mode, window size should be updated only if we are fullscreening or if this is the first initialization.
		if (FullScreen || FirstVid) {
			WindowWidth = CustomResX;
			WindowHeight = CustomResY;
		}
	}

	if (startgame() != 1) {
		/* Exit zsnes if SDL could not be initialized */
		if (sdl_state == vid_null) {
			zexit_error();
		} else {
			return;
		}
	}

	if (newmode) {
		Change_Resolution_Mode();
	}

	if (FirstVid) {
		FirstVid = 0;
		InitSound();
		InitInput();
	}

	if (((PrevStereoSound != StereoSound) || (PrevSoundQuality != SoundQuality))) {
		InitSound();
	}
}

void DrawScreen(void) {
	if (sdl_state == vid_gl) {
		gl_drawwin();
	}
	if (blinit == 1) {
		initwinvideo();
		blinit = 0;
	}
}

void CheckTimers(void) {
	end2 = sem_GetTicks();

	while ((end2 - start2) >= update_ticks_pc2) {
		start2 += update_ticks_pc2;
	}

	if (T60HZEnabled) {
		end = sem_GetTicks();
		while ((end - start) >= update_ticks_pc) {
			Game60hzcall();
			SDL_SemPost(sem_frames);
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

void sem_sleep(void) {
	end = update_ticks_pc - (sem_GetTicks() - start) - .2f;
	if (end > 0.f) {
		SDL_SemWaitTimeout(sem_frames, (int)end);
	}
}

static SDL_Thread *sem_threadid = NULL;
static int sem_threadrun;

int sem_thread(void *param) {
	while (sem_threadrun) {
		if (T60HZEnabled) {
			SDL_SemPost(sem_frames);
			usleep(romispal ? 2000 : 1000);
		} else {
			usleep(20000);
		}
	}
	return (0);
}

static void sem_sleep_rdy(void) {
	if (sem_frames) {
		return;
	}
	sem_frames = SDL_CreateSemaphore(0);
	sem_threadrun = 1;
	sem_threadid = SDL_CreateThread(sem_thread, 0);
}

static void sem_sleep_die() {
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

void UpdateVFrame(void) {
	// Quick fix for GUI CPU usage
	if (GUIOn || GUIOn2 || EMUPause) {
		usleep(6000);
	}

	CheckTimers();
	Main_Proc();
}

void UnloadSDL() {
	DeinitSound();
	sem_sleep_die(); // Shutdown semaphore
	if (sdl_state == vid_gl) {
		gl_end();
	}
	SDL_Quit();
}

s4 GetMouseX(void) {
	return ((int)MouseX);
}
s4 GetMouseY(void) {
	return ((int)MouseY);
}

s4 GetMouseMoveX(void) {
	//   InputRead();
	// SDL_GetRelativeMouseState(&MouseMove2X, NULL);
#if SDL_VERSION_ATLEAST(1, 3, 0)
	SDL_GetRelativeMouseState(0, &MouseMove2X, &MouseMove2Y);
#else
	SDL_GetRelativeMouseState(&MouseMove2X, &MouseMove2Y);
#endif
	return (MouseMove2X);
}

s4 GetMouseMoveY(void) {
	return (MouseMove2Y);
}

s4 GetMouseButton(void) {
	return ((int)MouseButton);
}

void SetMouseMinX(int MinX) {
	MouseMinX = MinX;
	adjustMouseXScale();
}
void SetMouseMaxX(int MaxX) {
	MouseMaxX = MaxX;
	adjustMouseXScale();
}
void SetMouseMinY(int MinY) {
	MouseMinY = MinY;
	adjustMouseYScale();
}
void SetMouseMaxY(int MaxY) {
	MouseMaxY = MaxY;
	adjustMouseYScale();
}
void SetMouseX(int X) {
	MouseX = X;
}
void SetMouseY(int Y) {
	MouseY = Y;
}

static float sem_GetTicks(void) {
	struct timeval now;
	float ticks;

	gettimeofday(&now, NULL);
	ticks = ((float)(now.tv_sec - sem_start.tv_sec)) * 1000.f + ((float)(now.tv_usec - sem_start.tv_usec)) * .001f;
	return (ticks);
}

void ZsnesPage(void) {
}

void DocsPage(void) {
}
