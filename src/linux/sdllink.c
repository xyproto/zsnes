/*
Copyright (C) 1997-2005 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "gblhdr.h"
#include "sw_draw.h"
#include "gl_draw.h"

#include <SDL_thread.h>

#include <sys/time.h>
#include <time.h>
#include <dirent.h>

#include <sys/param.h>
#include <paths.h>
#include <grp.h>

#ifndef OPEN_MAX
#define OPEN_MAX 256
#endif

//C++ style code in C
#define bool unsigned char
#define true 1
#define false 0


typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef Uint32 UINT32;
typedef long long _int64;
typedef long long LARGE_INTEGER;
#define QueryPerformanceCounter(x) asm volatile("rdtsc" : "=a"(((unsigned int *)(x))[0]),"=d"(((unsigned int *)x)[1]))


typedef enum { FALSE = 0, TRUE = 1 } BOOL;
typedef enum vidstate_e { vid_null, vid_none, vid_soft, vid_gl } vidstate_t;

// SOUND RELATED VARIABLES
SDL_AudioSpec audiospec;
int SoundEnabled = 1;
BYTE PrevStereoSound;
DWORD PrevSoundQuality;
Uint8 *Buffer = NULL;
int Buffer_len = 0, Buffer_fill = 0;
int Buffer_head = 0, Buffer_tail = 0;

extern BYTE StereoSound;
extern DWORD SoundQuality;
extern int DSPBuffer[];

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

extern void SwitchFullScreen (void);
extern unsigned char cvidmode;
DWORD SMode=0;
DWORD DSMode=0;
DWORD prevHQMode=-1;

extern DWORD converta;
extern DWORD *BitConv32Ptr;
extern DWORD *RGBtoYUVPtr;
extern unsigned char hqFilter;

extern BYTE GUIWFVID[];
extern BYTE GUISMODE[];
extern BYTE GUIDSMODE[];
extern BYTE GUIHQ2X[];
extern BYTE GUIHQ3X[];
extern BYTE GUIHQ4X[];
extern BYTE GUIRESIZE[];

/* JOYSTICK AND KEYBOARD INPUT */
SDL_Joystick *JoystickInput[5];
unsigned int AxisOffset[5] = {256 + 128 + 64};	// per joystick offsets in
unsigned int ButtonOffset[5] = {448};		// pressed. We have 128 + 64
unsigned int HatOffset[5] = {448};		// bytes for all joysticks. We
unsigned int BallOffset[5] = {448};		// can control all 5 players.
int shiftptr = 0;
int offset;
DWORD numlockptr;

extern unsigned char pressed[];
extern int CurKeyPos;
extern int CurKeyReadPos;
extern int KeyBuffer[16];

/* MOUSE INPUT */
float MouseMinX = 0;
float MouseMaxX = 256;
float MouseMinY = 0;
float MouseMaxY = 223;
int MouseX, MouseY;
float MouseMoveX, MouseMoveY;
int MouseMove2X, MouseMove2Y;
Uint8 MouseButton;
float MouseXScale = 1.0;
float MouseYScale = 1.0;
DWORD LastUsedPos = 0;
DWORD CurMode = -1;

extern BYTE GUIOn;
extern BYTE GUIOn2;
extern BYTE EMUPause;
static BYTE IsActivated = 1;

/* TIMER VARIABLES/MACROS */
#define UPDATE_TICKS_GAME (1000.855001760297741789468390082/60.0)	// milliseconds per world update
#define UPDATE_TICKS_GAMEPAL (20)	// milliseconds per world update
#define UPDATE_TICKS_GUI (1000.0/36.0)	// milliseconds per world update
#define UPDATE_TICKS_UDP (1000.0/60.0)	// milliseconds per world update

int T60HZEnabled = 0;
int T36HZEnabled = 0;
short SystemTimewHour;
short SystemTimewMinute;
short SystemTimewSecond;
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
void ProcessKeyBuf(int scancode);
void LinuxExit(void);
void UpdateSound(void *userdata, Uint8 * stream, int len);

extern int GUI36hzcall(void);
extern int Game60hzcall(void);
extern void SoundProcess();
_int64 copymaskRB = 0x001FF800001FF800LL;
_int64 copymaskG = 0x0000FC000000FC00LL;
_int64 copymagic = 0x0008010000080100LL;
_int64 coef = 0x0066009a0066009aLL;
#ifdef __OPENGL__
extern void gl_clearwin(void);
#endif

static void adjustMouseXScale(void)
{
	MouseXScale = (MouseMaxX - MouseMinX) / ((float) WindowWidth);
}

static void adjustMouseYScale(void)
{
	MouseYScale = (MouseMaxY - MouseMinY) / ((float) WindowHeight);
}

int Main_Proc(void)
{
	SDL_Event event;

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
				if (event.key.keysym.scancode - 8 >= 0)
				{
					//if (pressed[event.key.keysym.scancode - 8] != 2)
					pressed[event.key.keysym.scancode - 8] = 1;
					ProcessKeyBuf(event.key.keysym.sym);
				}
				break;

			case SDL_KEYUP:
				if (event.key.keysym.sym == SDLK_LSHIFT ||
				    event.key.keysym.sym == SDLK_RSHIFT)
					shiftptr = 0;
				if (event.key.keysym.scancode - 8 >= 0)
					pressed[event.key.keysym.scancode - 8] = 0;
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
					case 2:
						ProcessKeyBuf(SDLK_RETURN);
						// Yes, this is intentional - DDOI
					default:
						MouseButton = MouseButton | event.button.button;
						break;
				}
				break;

			case SDL_MOUSEBUTTONUP:
				MouseButton =
					MouseButton & ~event.button.button;
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
//				printf("DEBUG axis offset: %d\n", offset);
				if (event.jaxis.value < -16384)
				{
					pressed[offset + 1] = 1;
					pressed[offset + 0] = 0;
				}
				else if (event.jaxis.value > 16384)
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
//				printf("DEBUG button offset: %d\n", offset);
				if (offset >= (256 + 128 + 64)) break;
				pressed[offset] = 1;
				break;

			case SDL_JOYBUTTONUP:
				offset = ButtonOffset[event.jbutton.which];
				offset += event.jbutton.button;
//				printf("DEBUG button offset: %d\n", offset);
				if (offset >= (256 + 64)) break;
				pressed[offset] = 0;
				break;
			case SDL_QUIT:
				LinuxExit();
				break;
#ifdef __OPENGL__
			case SDL_VIDEORESIZE:
				if(!GUIRESIZE[cvidmode]) {
				    surface = SDL_SetVideoMode(WindowWidth, WindowHeight,
					    BitDepth, surface->flags & ~SDL_RESIZABLE);
					adjustMouseXScale();
					adjustMouseYScale();
				    break;
				}
				WindowWidth = SurfaceX = event.resize.w;
				WindowHeight = SurfaceY = event.resize.h;
				surface = SDL_SetVideoMode(WindowWidth,
				    WindowHeight, BitDepth, surface->flags);
				adjustMouseXScale();
				adjustMouseYScale();
				glViewport(0,0, WindowWidth, WindowHeight);
				glFlush();
				gl_clearwin();
				break;
#endif
			default:
				break;
		}
	}

	return TRUE;
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
		}		// end no-numlock
	}			// end testing of keypad
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

int InitSound(void)
{
	SDL_AudioSpec wanted;
	const int samptab[7] = { 1, 1, 2, 4, 2, 4, 4 };
	const int freqtab[7] = { 8000, 11025, 22050, 44100, 16000, 32000, 48000 };

	SDL_CloseAudio();

	if (!SoundEnabled)
	{
		return FALSE;
	}

	if (Buffer)
		free(Buffer);
	Buffer = NULL;
	Buffer_len = 0;

	PrevSoundQuality = SoundQuality;
	PrevStereoSound = StereoSound;

	if (SoundQuality > 6)
		SoundQuality = 1;
	wanted.freq = freqtab[SoundQuality];

	if (StereoSound)
	{
		wanted.channels = 2;
	}
	else
	{
		wanted.channels = 1;
	}

	wanted.samples = samptab[SoundQuality] * 128 * wanted.channels;

	wanted.format = AUDIO_S16LSB;
	wanted.userdata = NULL;
	wanted.callback = UpdateSound;

	if (SDL_OpenAudio(&wanted, &audiospec) < 0)
	{
		fprintf(stderr, "Sound init failed!\n");
		fprintf(stderr, "freq: %d, channels: %d, samples: %d\n",
			wanted.freq, wanted.channels, wanted.samples);
		SoundEnabled = 0;
		return FALSE;
	}
	SDL_PauseAudio(0);

	Buffer_len = (audiospec.size * 2);
	Buffer_len = (Buffer_len + 255) & ~255; /* Align to SPCSize */
	Buffer = malloc(Buffer_len);

	return TRUE;
}

int ReInitSound(void)
{
	return InitSound();
}

BOOL InitJoystickInput(void)
{
	int i, max_num_joysticks;
	int num_axes, num_buttons, num_hats, num_balls;
	int js_fail = 0;

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

		if (js_fail)
		{
			printf("Warning: Joystick won't work.\n");
			continue;
		}
		if (i == 0)
		{
			AxisOffset[0] = 256;		// After key data
			ButtonOffset[0] = AxisOffset[0] + num_axes * 2;
//			printf("ButtonOffset %d\n", ButtonOffset[0]);
			HatOffset[0] = ButtonOffset[0] + num_buttons;
//			printf("HatOffset %d\n", HatOffset[0]);
			BallOffset[0] = HatOffset[0] + num_hats * 4;
//			printf("BallOffset %d\n", BallOffset[0]);
			if ((BallOffset[0] + num_balls * 4) >= (256 + 128 + 64))
				js_fail = 1;
		}
		else
		{
			AxisOffset[i] = BallOffset[i - 1] +
				SDL_JoystickNumBalls(JoystickInput[i - 1]);
			ButtonOffset[i] = AxisOffset[i] + num_axes * 2;
			HatOffset[i] = ButtonOffset[i] + num_buttons;
			BallOffset[i] = HatOffset[i] + num_hats * 4;
			if ((BallOffset[i] + num_balls * 4) >= (256 + 128 + 64))
				js_fail = 1;

		}
		if (js_fail)
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

int startgame(void)
{
	int status;

	if (sdl_state != vid_null)
	{
		if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER |
	        SDL_INIT_VIDEO) < 0)
		{
			fprintf(stderr, "Could not initialize SDL: %s", SDL_GetError());
			return FALSE;
		}
		sdl_state = vid_none;
	}

        // Start semaphore code so ZSNES multitasks nicely :)
	sem_sleep_rdy();

	if (sdl_state == vid_soft) sw_end();
#ifdef __OPENGL__
	else if (sdl_state == vid_gl) gl_end();

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

void LinuxExit(void)
{
	if (sdl_state != vid_null)
	{
		SDL_WM_GrabInput(SDL_GRAB_OFF);	// probably redundant
		sem_sleep_die(); // Shutdown semaphore
		SDL_Quit();
	}
	exit(0);
}

void endgame()
{
	LinuxExit();
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

void initwinvideo(void)
{
	DWORD newmode = 0;

	init_hqNx();

	if (CurMode != cvidmode)
	{
		CurMode = cvidmode;
		newmode = 1;
		WindowWidth = 256;
		WindowHeight = 224;

		FullScreen = GUIWFVID[cvidmode];
#ifdef __OPENGL__
		UseOpenGL = 0;
		if (cvidmode > 5)
		   UseOpenGL = 1;
#else
		if (cvidmode > 5)
		  cvidmode = 2; // set it to the default 512x448 W
#endif

		switch (cvidmode)
		{
			//case 0:
			//case 4;
			case 1:
			default:
				WindowWidth = 256;
				WindowHeight = 224;
				break;
			case 2:
			case 3:
			case 7:
			case 21:					// Variable
				WindowWidth = 512;
				WindowHeight = 448;
				break;
			case 4:
			case 8:
			case 9:
				WindowWidth = 640;
				WindowHeight = 480;
				break;
			case 10:
				WindowWidth = 640;
				WindowHeight = 576;
				break;
			case 11:
				WindowWidth = 768;
				WindowHeight = 672;
				break;
			case 5:
			case 12:
			case 13:
				WindowWidth = 800;
				WindowHeight = 600;
				break;
			case 14:
				WindowWidth = 896;
				WindowHeight = 784;
				break;
			case 15:
			case 16:
				WindowWidth = 1024;
				WindowHeight = 768;
				break;
			case 17:
				WindowWidth = 1024;
				WindowHeight = 896;
				break;
			case 18:
				WindowWidth = 1280;
				WindowHeight = 960;
				break;
			case 19:
				WindowWidth = 1280;
				WindowHeight = 1024;
				break;
			case 20:
				WindowWidth = 1600;
				WindowHeight = 1200;
				break;
		}
		adjustMouseXScale();
		adjustMouseYScale();
	}

	if (startgame() != TRUE)
	{
		/* Exit zsnes if SDL could not be initialized */
		if (sdl_state == vid_null)
			LinuxExit ();
		else
			return;
	}

	if (newmode == 1)
		clearwin();

	if (FirstVid == 1)
	{
		FirstVid = 0;

		InitSound();
		InitInput();
	}

	if (((PrevStereoSound != StereoSound)
	     || (PrevSoundQuality != SoundQuality)))
		ReInitSound();
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
			/*
			   _asm{
			   pushad
			   call Game60hzcall
			   popad
			   }
			 */
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
			/*
			   _asm{
			   pushad
			   call GUI36hzcall
			   popad
			   }
			 */
			GUI36hzcall();
			start += update_ticks_pc;
		}
	}
}

//Why in the world did someone make this use signed values??? -Nach
void UpdateSound(void *userdata, Uint8 * stream, int len)
{
  int left = Buffer_len - Buffer_head;

  if (left < 0)
  {
    return;
  }

  if (left <= len)
  {
    memcpy(stream, &Buffer[Buffer_head], left);
    stream += left;
    len -= left;
    Buffer_head = 0;
    Buffer_fill -= left;
  }

  if (len)
  {
    memcpy(stream, &Buffer[Buffer_head], len);
    Buffer_head += len;
    Buffer_fill -= len;
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
	const int SPCSize = 256;
	int i;

	//Quick fix for GUI CPU usage
	if (GUIOn || GUIOn2 || EMUPause) usleep(6000);

	CheckTimers();
	Main_Proc();

	/* Process sound */

	/* take care of the things we left behind last time */
	SDL_LockAudio();
	while (Buffer_fill < Buffer_len) {
		short *ptr = (short*)&Buffer[Buffer_tail];

		SoundProcess();

		for (i = 0; i < SPCSize; i++, ptr++)
		{
			if (T36HZEnabled)
			{
				*ptr = 0;
			}
			else
			{
				if (DSPBuffer[i] > 32767)
					*ptr = 32767;
				else if (DSPBuffer[i] < -32767)
					*ptr = -32767;
				else
					*ptr = DSPBuffer[i];
			}
		}

		Buffer_fill += SPCSize * 2;
		Buffer_tail += SPCSize * 2;
		if (Buffer_tail >= Buffer_len) Buffer_tail = 0;
	}
	SDL_UnlockAudio();
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
	/* Just in case - DDOI */
	if (sdl_state == vid_none) return;

#ifdef __OPENGL__
	if (UseOpenGL)
		gl_drawwin();
	else
#endif
		sw_drawwin();
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

void GetLocalTime()
{
	time_t current;
	struct tm *timeptr;

	time(&current);
	timeptr = localtime(&current);
	SystemTimewHour = timeptr->tm_hour;
	SystemTimewMinute = timeptr->tm_min;
	SystemTimewSecond = timeptr->tm_sec;
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



//Introducing the secure browser opener for POSIX systems ;) -Nach

//Taken from the secure programming cookbook, slightly modified
bool spc_drop_privileges() {
  gid_t newgid = getgid(), oldgid = getegid();
  uid_t newuid = getuid(), olduid = geteuid();

  /* If root privileges are to be dropped, be sure to pare down the ancillary
   * groups for the process before doing anything else because the setgroups()
   * system call requires root privileges.  Drop ancillary groups regardless of
   * whether privileges are being dropped temporarily or permanently.
   */
  if (!olduid) setgroups(1, &newgid);

  if (newgid != oldgid) {
#if !defined(linux)
    setegid(newgid);
    if (setgid(newgid) == -1) return(false);
#else
    if (setregid(newgid, newgid) == -1) return(false);
#endif
  }

  if (newuid != olduid) {
#if !defined(linux)
    seteuid(newuid);
    if (setuid(newuid) == -1) return(false);
#else
    if (setregid(newuid, newuid) == -1) return(false);
#endif
  }

  /* verify that the changes were successful */

  if (newgid != oldgid && (setegid(oldgid) != -1 || getegid() != newgid))
    return(false);
  if (newuid != olduid && (seteuid(olduid) != -1 || geteuid() != newuid))
    return(false);

  return(true);
}

static int open_devnull(int fd) {
  FILE *f = 0;

  if (!fd) f = freopen(_PATH_DEVNULL, "rb", stdin);
  else if (fd == 1) f = freopen(_PATH_DEVNULL, "wb", stdout);
  else if (fd == 2) f = freopen(_PATH_DEVNULL, "wb", stderr);
  return (f && fileno(f) == fd);
}

void spc_sanitize_files() {
  int         fd, fds;
  struct stat st;

  /* Make sure all open descriptors other than the standard ones are closed */
  if ((fds = getdtablesize()) == -1) fds = OPEN_MAX;
  for (fd = 3;  fd < fds;  fd++) close(fd);

  /* Verify that the standard descriptors are open.  If they're not, attempt to
   * open them using /dev/null.  If any are unsuccessful, abort.
   */
  for (fd = 0;  fd < 3;  fd++)
    if (fstat(fd, &st) == -1 && (errno != EBADF || !open_devnull(fd))) abort();
}

pid_t spc_fork() {
  pid_t childpid;

  if ((childpid = fork()) == -1) return -1;

  //If this us the parent proccess nothing more to do
  if (childpid != 0) return childpid;

  //This is the child proccess

  spc_sanitize_files();

  /*
  There actually is a bug here which I submitted to the authors of the book -Nach

  The bug is as follows:
  The parent returns the child proccess ID in event of success.
  The child returns 0 on success if and only if it's spc_drop_privileges() call is successful.
  It is possible that the parent will return with a pid > 0, while the child never returns
  from spc_fork thus causing a programming error.

  The function should be rewritten that the parent doesn't return till it knows if the
  child is able to return or not. And then return -1 or the child pid.

  For out purposes in ZSNES to launch a browser, this bug does not effect us. But
  be careful if you copy this code to use somewhere else.
  */
  if (!spc_drop_privileges()) //Failed to drop special privleges
  {
    _exit(0);
  }

  return 0;
}

void LaunchBrowser(char *browser)
{
  char *arglist[] = { browser, "http://www.zsnes.com/", 0 };
  execvp(browser, arglist);
}

void ZsnesPage()
{
  if (spc_fork()) //If fork failed, or we are the parent
  {
    MouseX = 0;
    MouseY = 0;
    return;
  }

  //We are now the child proccess

  //If any of these LaunchBrowser() calls return that means it failed and we should try the next one
  LaunchBrowser("mozilla");
  LaunchBrowser("mozilla-firefox");
  LaunchBrowser("konqueror");
  LaunchBrowser("lynx");
  LaunchBrowser("links");

  _exit(0); //All browser launches failed, oh well
}


/*
Functions for battery power for Linux by Nach
I believe Linux 2.4.x+ is needed for ACPI support
but it'll compile fine for older versions too

Special thanks David Lee Lambert for most of the code here
*/

#ifdef linux
int CheckBattery()
{
  int battery = -1; //No battery / Can't get info
  const char *ac = "/proc/acpi/ac_adapter/";

  //Check ac adapter
  DIR *ac_dir = opendir(ac);
  if (ac_dir)
  {
    char fnbuf[40]; // longer than len(ac)+len(HEXDIGIT*4)+len({state|info})
    FILE *fp;
    const char *pattern = " %39[^:]: %39[ -~]"; // for sscanf
    char line[80], key[40], arg[40];

    struct dirent *ent;
    while ((ent = readdir(ac_dir)))
    {
      if (ent->d_name[0] == '.') { continue; }

      snprintf(fnbuf, 40, "%s%s/state", ac, ent->d_name);
      fp = fopen(fnbuf, "r");
      if (fp)
      {
        while (fgets(line, 80, fp) && 2 == sscanf(line, pattern, key, arg))
        {
          if (!strcmp(key, "state"))
          {
            if (!strcmp(arg, "on-line"))
            {
              battery = 0;
            }
            else if (!strcmp(arg, "off-line"))
            {
              battery = 1;
              break;
            }
          }
        }
        fclose(fp);
      }
    }
    closedir(ac_dir);
  }
  return(battery);
}

static int BatteryLifeTime;
static int BatteryLifePercent;

static void update_battery_info()
{
  const char *batt = "/proc/acpi/battery/";

  //Check batteries
  DIR *batt_dir = opendir(batt);
  if (batt_dir)
  {
    char fnbuf[40]; // longer than len(ac)+len(HEXDIGIT*4)+len({state|info})
    FILE *fp;
    const char *pattern = " %39[^:]: %39[ -~]"; // for sscanf
    char line[80], key[40], arg[40];

    float x, design_capacity = 0.0f, remaining_capacity = 0.0f, present_rate = 0.0f, full_capacity = 0.0f;

    struct dirent *ent;
    while ((ent = readdir(batt_dir)))
    {
      if (ent->d_name[0] == '.') { continue; }
      snprintf(fnbuf, 40, "%s%s/info", batt, ent->d_name);
      fp = fopen(fnbuf, "r");
      if (fp)
      {
        while (fgets(line, 80, fp) && 2 == sscanf(line, pattern, key, arg))
        {
          if (strcmp(key, "design capacity") == 0 && sscanf(arg, "%g", &x) == 1)
          {
            design_capacity += x;
          }
          else if (strcmp(key, "last full capacity") == 0 && sscanf(arg, "%g", &x) == 1)
          {
            full_capacity += x;
          }
        }
        fclose(fp);
      }
      snprintf(fnbuf, 40, "%s%s/state", batt, ent->d_name);
      fp = fopen(fnbuf, "r");
      if (fp)
      {
        int charging = 0;
        while (fgets(line, 80, fp) && 2 == sscanf(line, pattern, key, arg))
        {
          if (strcmp(key, "charging state") == 0)
          {
            if (strcmp(arg, "discharging") == 0)
            {
              charging = -1;
            }
            else if (strcmp(arg, "charging") == 0)
            {
              charging = 1;
            }
          }
          else if (strcmp(key, "present rate") == 0 && sscanf(arg, "%g", &x) == 1)
          {
            present_rate += charging * x;
            charging = 0;
          }
          else if (strcmp(key, "remaining capacity") == 0 && sscanf(arg, "%g:", &x) == 1)
          {
            remaining_capacity += x;
            charging = 0;
          }
        }
        fclose(fp);
      }
    }
    if (design_capacity > 0.0f)
    {
      BatteryLifePercent = (int)floorf(remaining_capacity / ((full_capacity > 0.0f) ? full_capacity : design_capacity) * 100.0);
      if (present_rate < 0.0f)
      {
        // Linux specifies rates in mWh or mAh
        BatteryLifeTime = (int)floorf(remaining_capacity / (-present_rate) * 3600.0);
      }
    }
    closedir(batt_dir);
  }
}

int CheckBatteryTime()
{
  BatteryLifeTime = -1;
  update_battery_info();
  return(BatteryLifeTime);
}

int CheckBatteryPercent()
{
  BatteryLifePercent = -1;
  update_battery_info();
  return(BatteryLifePercent);
}

#else //Non Linux OSs

int CheckBattery()
{
  return(-1);
}

int CheckBatteryTime()
{
  return(-1);
}

int CheckBatteryPercent()
{
  return(-1);
}

#endif
