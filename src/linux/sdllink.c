#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include "SDL.h"

#include "sw_draw.h"
#include "gl_draw.h"

#ifdef __OPENGL__
#include <GL/gl.h>
#endif

#define BYTE  unsigned char
#define WORD  unsigned short
#define DWORD unsigned long

typedef enum
{ FALSE = 0, TRUE = !FALSE }
BOOL;
typedef Uint32 UINT32;
typedef long long _int64;
typedef long long LARGE_INTEGER;

#define STUB_FUNCTION fprintf(stderr,"STUB: %s at " __FILE__ ", line %d, thread %d\n",__FUNCTION__,__LINE__,getpid())

// SOUND RELATED VARIABLES
DWORD SoundBufferSize = 1024 * 18;
DWORD FirstSound = 1;
int AllowDefault = 0;
int SoundEnabled = 1;
BYTE PrevStereoSound;
DWORD PrevSoundQuality;
extern BYTE StereoSound;
extern DWORD SoundQuality;

// SDL VIDEO VARIABLES
SDL_Surface *surface;
int SurfaceLocking = 0;
int SurfaceX, SurfaceY;

// VIDEO VARIABLES
static DWORD WindowWidth = 256;
static DWORD WindowHeight = 224;
static DWORD FullScreen = 0;
static int sdl_inited = 0;
static int UseOpenGL = 0;
extern unsigned char cvidmode;
DWORD BitDepth = 0;	// Do NOT change this for ANY reason

// JOYSTICK AND KEYBOARD INPUT
SDL_Joystick *JoystickInput[5];
//DWORD CurrentJoy = 0;
unsigned char keyboardhit = 0;
int shiftptr = 0;
DWORD numlockptr;
extern unsigned char pressed[];
extern int CurKeyPos;
extern int CurKeyReadPos;
extern int KeyBuffer[16];

// MOUSE INPUT
float MouseMinX = 0;
float MouseMaxX = 256;
float MouseMinY = 0;
float MouseMaxY = 223;
int MouseX, MouseY;
float MouseMoveX, MouseMoveY;
int MouseMove2X, MouseMove2Y;
Uint8 MouseButton;

DWORD LastUsedPos = 0;
DWORD CurMode = -1;
extern BYTE GUIOn2;
static BYTE IsActivated;

#define UPDATE_TICKS_GAME (1000/60.0)	// milliseconds per world update
#define UPDATE_TICKS_GAMEPAL (1000/50.0)// milliseconds per world update
#define UPDATE_TICKS_GUI (1000/36.0)	// milliseconds per world update
#define UPDATE_TICKS_UDP (1000/60.0)	// milliseconds per world update

Uint32 end, end2;
double start, start2;
double update_ticks_pc, update_ticks_pc2;

void drawscreenwin(void);
void initwinvideo();
void ProcessKeyBuf(int scancode);
void LinuxExit(void);

#ifdef __OPENGL__
extern void gl_clearwin();
#endif

int Main_Proc(void)
{
	int j;
	SDL_Event event;
	//Uint8 JoyButton;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_ACTIVEEVENT:
				IsActivated = event.active.gain;
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_LSHIFT ||
				    event.key.keysym.sym == SDLK_RSHIFT)
					shiftptr = 1;
				if (event.key.keysym.mod & KMOD_NUM)
					numlockptr = 1;
				else
					numlockptr = 0;
				if (event.key.keysym.scancode - 8 >= 0)
				{
					if (pressed[event.key.keysym.scancode - 8] != 2)
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
				MouseX = event.motion.x;
				MouseY = event.motion.y;
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
			    //CurrentJoy = event.jaxis.which;
				for (j = 0; j < 3; j++)
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
			    //CurrentJoy = event.jbutton.which;
			    //JoyButton = event.jbutton.button;
				pressed[0x100 + event.jbutton.which * 32 + 16 +
					event.jbutton.button] = 1;
				break;

			case SDL_JOYBUTTONUP:
			    //CurrentJoy = event.jbutton.which;
			    //JoyButton = event.jbutton.button;
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
				    break;
				}
				WindowWidth = SurfaceX = event.resize.w;
				WindowHeight = SurfaceY = event.resize.h;
				surface = SDL_SetVideoMode(WindowWidth,
				    WindowHeight, BitDepth, surface->flags);
				glViewport(0,0, WindowWidth, WindowHeight);
				glFlush();
				gl_clearwin();
				break;
#endif
			default:
				break;
		}
	}
	/* TODO - fix this later
	   if(pressed[0x38]!=0&&pressed[0x3E]!=0)
	   LinuxExit();
	   if(pressed[0x38]!=0&&pressed[0x1c]!=0)
	   SwitchFullScreen();
	 */
	return TRUE;
}

#define true 1

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
		accept = true;
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
			accept = true;
			vkeyval = scancode - SDLK_KP0 + '0';
		}
		else
		{

			switch (scancode)
			{
				case SDLK_KP9: vkeyval = 256 + 73; accept = true; break;
				case SDLK_KP8: vkeyval = 256 + 72; accept = true; break;
				case SDLK_KP7: vkeyval = 256 + 71; accept = true; break;
				case SDLK_KP6: vkeyval = 256 + 77; accept = true; break;
				case SDLK_KP5: vkeyval = 256 + 76; accept = true; break;
				case SDLK_KP4: vkeyval = 256 + 75; accept = true; break;
				case SDLK_KP3: vkeyval = 256 + 81; accept = true; break;
				case SDLK_KP2: vkeyval = 256 + 80; accept = true; break;
				case SDLK_KP1: vkeyval = 256 + 79; accept = true; break;
			}
		}		// end no-numlock
	}			// end testing of keypad
	if (!shiftptr)
	{
		switch (scancode)
		{
			case SDLK_MINUS:        vkeyval = '-'; accept = true; break;
			case SDLK_EQUALS:       vkeyval = '='; accept = true; break;
			case SDLK_LEFTBRACKET:  vkeyval = '['; accept = true; break;
			case SDLK_RIGHTBRACKET: vkeyval = ']'; accept = true; break;
			case SDLK_SEMICOLON:    vkeyval = ';'; accept = true; break;
				// ??? - DDOI
				//case 222: vkeyval=39; accept = true; break;
				//case 220: vkeyval=92; accept = true; break;
			case SDLK_COMMA:        vkeyval = ','; accept = true; break;
			case SDLK_PERIOD:       vkeyval = '.'; accept = true; break;
			case SDLK_SLASH:        vkeyval = '/'; accept = true; break;
			case SDLK_QUOTE:        vkeyval = '`'; accept = true; break;
		}
	}
	else
	{
		switch (scancode)
		{
			case SDLK_MINUS:        vkeyval = '_'; accept = true; break;
			case SDLK_EQUALS:       vkeyval = '+'; accept = true; break;
			case SDLK_LEFTBRACKET:  vkeyval = '{'; accept = true; break;
			case SDLK_RIGHTBRACKET: vkeyval = '}'; accept = true; break;
			case SDLK_SEMICOLON:    vkeyval = ':'; accept = true; break;
			case SDLK_QUOTE:        vkeyval = '"'; accept = true; break;
			case SDLK_COMMA:        vkeyval = '<'; accept = true; break;
			case SDLK_PERIOD:       vkeyval = '>'; accept = true; break;
			case SDLK_SLASH:        vkeyval = '?'; accept = true; break;
			case SDLK_BACKQUOTE:    vkeyval = '~'; accept = true; break;
			case SDLK_BACKSLASH:    vkeyval = '|'; accept = true; break;
		}
	}
	// TODO Figure out what the rest these are supposed to be - DDOI
	switch (scancode)
	{
		case SDLK_PAGEUP:      vkeyval = 256 + 73; accept = true; break;
		case SDLK_UP:          vkeyval = 256 + 72; accept = true; break;
		case SDLK_HOME:        vkeyval = 256 + 71; accept = true; break;
		case SDLK_RIGHT:       vkeyval = 256 + 77; accept = true; break;
			//case 12: vkeyval = 256+76; accept = true; break;
		case SDLK_LEFT:        vkeyval = 256 + 75; accept = true; break;
		case SDLK_PAGEDOWN:    vkeyval = 256 + 81; accept = true; break;
		case SDLK_DOWN:        vkeyval = 256 + 80; accept = true; break;
		case SDLK_END:         vkeyval = 256 + 79; accept = true; break;
		case SDLK_KP_PLUS:     vkeyval = '+'; accept = true; break;
		case SDLK_KP_MINUS:    vkeyval = '-'; accept = true; break;
		case SDLK_KP_MULTIPLY: vkeyval = '*'; accept = true; break;
		case SDLK_KP_DIVIDE:   vkeyval = '/'; accept = true; break;
		case SDLK_KP_PERIOD:   vkeyval = '.'; accept = true; break;
	}

	if (accept)
	{
		KeyBuffer[CurKeyPos] = vkeyval;
		CurKeyPos++;
		if (CurKeyPos == 16)
			CurKeyPos = 0;
	}
}

void UpdateSound(void *userdata, Uint8 * stream, int len);

int InitSound(void)
{
	SDL_AudioSpec wanted;
	const int freqtab[7] =
		{ 8000, 11025, 22050, 44100, 16000, 32000, 48000 };
	const int samptab[7] = { 64, 64, 128, 256, 128, 256, 256 };

	SDL_CloseAudio();

	if (!SoundEnabled)
	{
		return FALSE;
	}

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

	//wanted.samples = (wanted.freq / 60) * 2 * wanted.channels;
	wanted.samples = samptab[SoundQuality] * 2 * wanted.channels;
	wanted.format = AUDIO_S16LSB;
	wanted.userdata = NULL;
	wanted.callback = UpdateSound;

	if (SDL_OpenAudio(&wanted, NULL) < 0)
	{
		fprintf(stderr, "Sound init failed!\n");
		fprintf(stderr, "freq: %d, channels: %d, samples: %d\n",
			wanted.freq, wanted.channels, wanted.samples);
		SoundEnabled = 0;
		return FALSE;
	}
	SDL_PauseAudio(0);

	return TRUE;
}

int ReInitSound(void)
{
	return InitSound();
}

BOOL InitJoystickInput(void)
{
    int i; int max_num_joysticks;

	for (i = 0; i < 5; i++)
		JoystickInput[i] = NULL;
	// If it is possible to use SDL_NumJoysticks
	// before initialising SDL_INIT_JOYSTICK then
	// this call can be replaced with SDL_InitSubSystem
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

int startgame(void)
{
	int status;

	if (!sdl_inited)
	{
		if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER |
	        SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
		{
			fprintf(stderr, "Could not initialize SDL!\n");
			return FALSE;
		}
		sdl_inited = -1;
	}

	BitDepth = (UseOpenGL ? 16 : 0);

	if (sdl_inited == 1) sw_end();
#ifdef __OPENGL__
	else if (sdl_inited == 2) gl_end();

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
	sdl_inited = UseOpenGL + 1;
	return TRUE;
}

void LinuxExit(void)
{
	if (sdl_inited)
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

int i;
int count, x, count2;
DWORD Temp1;
DWORD SurfBufD;
DWORD CurrentPos;
DWORD WritePos;
DWORD SoundBufD;
DWORD SoundBufD2;
DWORD T60HZEnabled = 0;
DWORD T36HZEnabled = 0;
short *Sound;

//void WinUpdateDevices(); function removed since it was empty
extern unsigned char romispal;

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
	//   QueryPerformanceCounter((LARGE_INTEGER*)&start);
	//   QueryPerformanceCounter((LARGE_INTEGER*)&start2);
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
	//   QueryPerformanceCounter((LARGE_INTEGER*)&start);
	//   QueryPerformanceCounter((LARGE_INTEGER*)&start2);

	start = SDL_GetTicks();
	start2 = SDL_GetTicks();
	T60HZEnabled = 0;
	T36HZEnabled = 1;
}

void Stop36HZ(void)
{
	T36HZEnabled = 0;
}

DWORD FirstVid = 1;
DWORD FirstFull = 1;
extern BYTE GUIWFVID[];
extern BYTE BlackAndWhite;
extern BYTE V8Mode;
void clearwin();

void initwinvideo(void)
{
	DWORD newmode = 0;

	if (BlackAndWhite == 1)
		V8Mode = 1;
	else
		V8Mode = 0;

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
	}

	if (startgame() != TRUE)
	{
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

extern int DSPBuffer[];
extern int packettimeleft[256];
extern int PacketCounter;
extern int CounterA;
extern int CounterB;
extern void SoundProcess();
extern int GUI36hzcall(void);
extern int Game60hzcall(void);

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

/* should we clear these on sound reset? */
DWORD BufferLeftOver = 0;
short Buffer[1800 * 2];

void UpdateSound(void *userdata, Uint8 * stream, int len)
{
	const int SPCSize = 256;
	int DataNeeded;
	int i;
	Uint8 *ptr;

	len /= 2;		/* only 16bit here */

	ptr = stream;
	DataNeeded = len;

	/* take care of the things we left behind last time */
	if (BufferLeftOver)
	{
		DataNeeded -= BufferLeftOver;

		memcpy(ptr, &Buffer[BufferLeftOver],
		       (SPCSize - BufferLeftOver) * 2);

		ptr += (SPCSize - BufferLeftOver) * 2;
		BufferLeftOver = 0;
	}

	if (len & 255)
	{			/* we'll save the rest first */
		DataNeeded -= 256;
	}

	while (DataNeeded > 0)
	{
		SoundProcess();

		for (i = 0; i < SPCSize; i++)
		{
			if (T36HZEnabled)
			{
				Buffer[i] = 0;
			}
			else
			{
				if (DSPBuffer[i] > 32767)
					Buffer[i] = 32767;
				else if (DSPBuffer[i] < -32767)
					Buffer[i] = -32767;
				else
					Buffer[i] = DSPBuffer[i];
			}
		}

		memcpy(ptr, &Buffer[0], SPCSize * 2);
		ptr += SPCSize * 2;

		DataNeeded -= SPCSize;
	}

	if (DataNeeded)
	{
		DataNeeded += 256;
		BufferLeftOver = DataNeeded;

		SoundProcess();

		for (i = 0; i < SPCSize; i++)
		{
			if (T36HZEnabled)
			{
				Buffer[i] = 0;
			}
			else
			{
				if (DSPBuffer[i] > 32767)
					Buffer[i] = 32767;
				else if (DSPBuffer[i] < -32767)
					Buffer[i] = -32767;
				else
					Buffer[i] = DSPBuffer[i];
			}
		}

		memcpy(ptr, &Buffer[0], DataNeeded * 2);
	}
}

void UpdateVFrame(void)
{
	if (GUIOn2 == 1 && IsActivated == 0) SDL_WaitEvent(NULL);
	Main_Proc();
	CheckTimers();
}

void clearwin()
{
    if (!sdl_inited) return;

#ifdef __OPENGL__
	if (UseOpenGL)
		gl_clearwin();
	else
#endif
		sw_clearwin();
}

void drawscreenwin(void)
{
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

// we can probably get rid of these functions since they are no
// longer called in sdlintrf.asm
void SetMouseX(int X)
{				/* MouseX=X; */
}
void SetMouseY(int Y)
{				/* MouseY=Y; */
}

void ZsnesPage()
{
	system("netscape -remote 'openURL(http://www.zsnes.com/)'");
}

short SystemTimewHour;
short SystemTimewMinute;
short SystemTimewSecond;

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
