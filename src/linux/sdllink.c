/* Copyright (C) 1997-2002 ZSNES Team
* 
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later
* version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*/

#include "gblhdr.h"
#include "sw_draw.h"
#include "gl_draw.h"

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef Uint32 UINT32;
typedef long long _int64;
typedef long long LARGE_INTEGER;

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

/* NETWORK RELATED VARIABLES */
extern int packettimeleft[256];
extern int PacketCounter;
extern int CounterA;
extern int CounterB;

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
extern BYTE GUIWFVID[];
extern unsigned char cvidmode;

/* JOYSTICK AND KEYBOARD INPUT */
SDL_Joystick *JoystickInput[5];
int shiftptr = 0;
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

extern BYTE GUIOn2;
static BYTE IsActivated = 1;

/* TIMER VARIABLES/MACROS */
#define UPDATE_TICKS_GAME (1000.0/59.95)// milliseconds per world update
#define UPDATE_TICKS_GAMEPAL (20)	// milliseconds per world update
#define UPDATE_TICKS_GUI (1000.0/36.0)	// milliseconds per world update
#define UPDATE_TICKS_UDP (1000.0/60.0)	// milliseconds per world update

int T60HZEnabled = 0;
int T36HZEnabled = 0;
short SystemTimewHour;
short SystemTimewMinute;
short SystemTimewSecond;
Uint32 end, end2;
double start, start2;
double update_ticks_pc, update_ticks_pc2;

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
	int j;
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
			    if (event.jhat.hat == 0) // only support the first POV hat for now
			    {
				    switch (event.jhat.value)
				    {
				        case SDL_HAT_CENTERED:
						    pressed[0x100 + event.jhat.which * 32 + 0] = 0;
							pressed[0x100 + event.jhat.which * 32 + 1] = 0;
							pressed[0x100 + event.jhat.which * 32 + 2] = 0;
							pressed[0x100 + event.jhat.which * 32 + 3] = 0;
							break;
					    case SDL_HAT_UP:
						    pressed[0x100 + event.jhat.which * 32 + 3] = 1;
							pressed[0x100 + event.jhat.which * 32 + 2] = 0;
							break;
					    case SDL_HAT_RIGHTUP:
						    pressed[0x100 + event.jhat.which * 32 + 0] = 1;
							pressed[0x100 + event.jhat.which * 32 + 3] = 1;
							pressed[0x100 + event.jhat.which * 32 + 1] = 0;
							pressed[0x100 + event.jhat.which * 32 + 2] = 0;
							break;
					    case SDL_HAT_RIGHT:
						    pressed[0x100 + event.jhat.which * 32 + 0] = 1;
							pressed[0x100 + event.jhat.which * 32 + 1] = 0;
							break;
					    case SDL_HAT_RIGHTDOWN:
						    pressed[0x100 + event.jhat.which * 32 + 0] = 1;
							pressed[0x100 + event.jhat.which * 32 + 2] = 1;
							pressed[0x100 + event.jhat.which * 32 + 1] = 0;
							pressed[0x100 + event.jhat.which * 32 + 3] = 0;
							break;
					    case SDL_HAT_DOWN:
						    pressed[0x100 + event.jhat.which * 32 + 2] = 1;
							pressed[0x100 + event.jhat.which * 32 + 3] = 0;
							break;
					    case SDL_HAT_LEFTDOWN:
						    pressed[0x100 + event.jhat.which * 32 + 1] = 1;
							pressed[0x100 + event.jhat.which * 32 + 2] = 1;
							pressed[0x100 + event.jhat.which * 32 + 0] = 0;
							pressed[0x100 + event.jhat.which * 32 + 3] = 0;
							break;
					    case SDL_HAT_LEFT:
						    pressed[0x100 + event.jhat.which * 32 + 1] = 1;
							pressed[0x100 + event.jhat.which * 32 + 0] = 0;
							break;
					    case SDL_HAT_LEFTUP:
						    pressed[0x100 + event.jhat.which * 32 + 1] = 1;
							pressed[0x100 + event.jhat.which * 32 + 3] = 1;
							pressed[0x100 + event.jhat.which * 32 + 0] = 0;
							pressed[0x100 + event.jhat.which * 32 + 2] = 0;
							break;
					}
				}
				break;

			/*
			   joystick trackball code untested; change the test values
			   if the motion is too sensitive (or not sensitive enough);
			   only the first trackball is supported for now. we could get
			   really general here, but this may break the format of 'pressed'
			 */
			case SDL_JOYBALLMOTION:
			    //CurrentJoy = event.jball.which;
				if (event.jball.ball == 0)
				{
					if (event.jball.xrel < -100)
					{
						pressed[0x100 + event.jball.which * 32 + 6] = 0;
						pressed[0x100 +	event.jball.which * 32 + 7] = 1;
					}
					if (event.jball.xrel > 100)
					{
						pressed[0x100 + event.jball.which * 32 + 6] = 1;
						pressed[0x100 +	event.jball.which * 32 + 7] = 0;
					}
					if (event.jball.yrel < -100)
					{
						pressed[0x100 + event.jball.which * 32 + 8] = 0;
						pressed[0x100 + event.jball.which * 32 + 9] = 1;
					}
					if (event.jball.yrel > 100)
					{
						pressed[0x100 + event.jball.which * 32 + 8] = 1;
						pressed[0x100 + event.jball.which * 32 + 9] = 0;
				}
				}
				break;

			case SDL_JOYAXISMOTION:
				for (j = 0; j < 4; j++)
				{
					if (event.jaxis.axis == j)
					{
						if (event.jaxis.value < -16384)
						{
							pressed[0x100 +
								event.jaxis.which *	32 + 2 * j + 1] = 1;
							pressed[0x100 +
								event.jaxis.which * 32 + 2 * j + 0] = 0;
						}
						else if (event.jaxis.value > 16384)
						{
							pressed[0x100 +
								event.jaxis.which * 32 + 2 * j + 0] = 1;
							pressed[0x100 +
								event.jaxis.which * 32 + 2 * j + 1] = 0;
						}
						else
						{
							pressed[0x100 +
								event.jaxis.which * 32 + 2 * j + 0] = 0;
							pressed[0x100 +
								event.jaxis.which * 32 + 2 * j + 1] = 0;
						}
					}
				}
				break;

			case SDL_JOYBUTTONDOWN:
				pressed[0x100 + event.jbutton.which * 32 + 16 +
					event.jbutton.button] = 1;
				break;

			case SDL_JOYBUTTONUP:
				pressed[0x100 + event.jbutton.which * 32 + 16 +
					event.jbutton.button] = 0;
				break;
			case SDL_QUIT:
				LinuxExit();
				break;
#ifdef __OPENGL__
			case SDL_VIDEORESIZE:
				if(cvidmode != 16) {
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
		printf("Joystick %i (%i Buttons): %s\n", i,
		       SDL_JoystickNumButtons(JoystickInput[i]),
		       SDL_JoystickName(i));
	}

	return TRUE;
}

BOOL InitInput()
{
	InitJoystickInput();
	return TRUE;
}

int saybitdepth()
{
  int MyBitsPerPixel;
  const SDL_VideoInfo *info;
  SDL_Init(SDL_INIT_VIDEO);
  info = SDL_GetVideoInfo();
  MyBitsPerPixel = info->vfmt->BitsPerPixel;
  switch (MyBitsPerPixel)
    {
    case 0: printf("Cannot detect bitdepth. On fbcon and svgalib this is normal.\nTrying to force 16 bpp.\n\n"); break;
    case 16: break;
    default: printf("You are running in %d bpp, but ZSNES is forcing 16 bpp.\nYou may experience poor performance and/or crashing.\n\n", MyBitsPerPixel); break;
    }
  return 0;
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

	if (sdl_state == vid_soft) sw_end();
#ifdef __OPENGL__
	else if (sdl_state == vid_gl) gl_end();
	saybitdepth();
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

	start = SDL_GetTicks();
	start2 = SDL_GetTicks();
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

	start = SDL_GetTicks();
	start2 = SDL_GetTicks();
	T60HZEnabled = 0;
	T36HZEnabled = 1;
}

void Stop36HZ(void)
{
	T36HZEnabled = 0;
}

void initwinvideo(void)
{
	DWORD newmode = 0;

	if (CurMode != cvidmode)
	{
		CurMode = cvidmode;
		newmode = 1;
		WindowWidth = 256;
		WindowHeight = 224;

		FullScreen = GUIWFVID[cvidmode];
#ifdef __OPENGL__
		UseOpenGL = 0;
		if (cvidmode > 3)
		   UseOpenGL = 1;
#else
		if (cvidmode > 3)
		  cvidmode = 2; // set it to the default 512x448 W
#endif

		switch (cvidmode)
		{
			//case 0:
			default:
				WindowWidth = 256;
				WindowHeight = 224;
				break;
			case 1:
				WindowWidth = 320;
				WindowHeight = 240;
				break;
			case 2:
			case 5:
			case 13:
				WindowWidth = 512;
				WindowHeight = 448;
				break;
			case 3:
			case 6:
			case 16:
				WindowWidth = 640;
				WindowHeight = 480;
				break;
			case 7:
				WindowWidth = 640;
				WindowHeight = 576;
				break;
			case 8:
				WindowWidth = 768;
				WindowHeight = 672;
				break;
			case 9:
				WindowWidth = 896;
				WindowHeight = 784;
				break;
			case 10:
				WindowWidth = 1024;
				WindowHeight = 896;
				break;
			case 11:
			case 14:
				WindowWidth = 800;
				WindowHeight = 600;
				break;
			case 12:
			case 15:
				WindowWidth = 1024;
				WindowHeight = 768;
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
	int i;

	//QueryPerformanceCounter((LARGE_INTEGER*)&end2);
	end2 = SDL_GetTicks();

	while ((end2 - start2) >= update_ticks_pc2)
	{
		if (CounterA > 0)
			CounterA--;
		if (CounterB > 0)
			CounterB--;
		if (PacketCounter)
		{
			for (i = 0; i < 256; i++)
			{
				if (packettimeleft[i] > 0)
					packettimeleft[i]--;
			}
		}
		start2 += update_ticks_pc2;
	}

	if (T60HZEnabled)
{
		//QueryPerformanceCounter((LARGE_INTEGER*)&end);
		end = SDL_GetTicks();

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
			start += update_ticks_pc;
		}
	}

	if (T36HZEnabled)
	{
		//QueryPerformanceCounter((LARGE_INTEGER*)&end);
		end = SDL_GetTicks();

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

void UpdateSound(void *userdata, Uint8 * stream, int len)
{
	int left;

	left = Buffer_len - Buffer_head;

	if (left <= len) {
		memcpy(stream, &Buffer[Buffer_head], left);
		stream += left;
		len -= left;
		Buffer_head = 0;
		Buffer_fill -= left;
	}

	if (len) {
		memcpy(stream, &Buffer[Buffer_head], len);
		Buffer_head += len;
		Buffer_fill -= len;
	}
}

void UpdateVFrame(void)
{
	const int SPCSize = 256;
	int i;

	/* rcg06172001 get menu animations running correctly... */
	/*if (GUIOn2 == 1 && IsActivated == 0) SDL_WaitEvent(NULL);*/
	if (GUIOn2 == 1 && IsActivated == 0)
		SDL_Delay(100);  /* spare the CPU a little. */

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
