#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include "SDL.h"
#ifdef __OPENGL__
#include <GL/gl.h>
#endif

#define BYTE  unsigned char
#define WORD  unsigned short
#define DWORD unsigned long

typedef enum {FALSE, TRUE} BOOL;
typedef Uint32 UINT32;
typedef long long _int64;
typedef long long LARGE_INTEGER;
#define STUB_FUNCTION fprintf(stderr,"STUB: %s at " __FILE__ ", line %d, thread %d\n",__FUNCTION__,__LINE__,getpid())
#define FAR
#define PASCAL

BYTE    TestID1[] = { "" };
DWORD   Moving = 0;

// SOUND RELATED VARIABLES
DWORD   SoundBufferSize = 1024*18;
DWORD   FirstSound = 1;
int     AllowDefault = 0;
int     SoundEnabled = 1;
BYTE    PrevStereoSound;
DWORD   PrevSoundQuality;
extern  BYTE StereoSound;
extern  DWORD SoundQuality;

// SDL VIDEO VARIOABLES
SDL_Surface *surface;
DWORD   WindowWidth = 256;
DWORD   WindowHeight = 224;
DWORD   SurfaceX = 0;
DWORD   SurfaceY = 0;
DWORD   FullScreen = 0;
int     sdl_inited = 0;
DWORD   vidbuff_w, vidbuff_h;
BYTE    BackColor = 0;
DWORD   BitDepth = 0;	// Do NOT change this for ANY reason
extern  unsigned char cvidmode;

// OPENGL VARIABLES
#ifdef __OPENGL__
int     gl_inited = 0;
unsigned short *glvidbuffer = 0;
int     glvbtexture[1];
float   ratiox = 1.0;
int     UseOpenGL = 0;
int     glfilters = GL_NEAREST;
extern  Uint8 BilinearFilter;
extern  Uint8 FilteredGUI;
extern  Uint8 GUIOn2;
#endif


// JOYSTICK AND KEYBOARD INPUT
SDL_Joystick	*JoystickInput[4];
DWORD   CurrentJoy = 0;
unsigned char keyboardhit = 0;
int     shiftptr = 0;
DWORD   numlockptr;
extern  unsigned char pressed[];
extern  int CurKeyPos;
extern  int CurKeyReadPos;
extern  int KeyBuffer[16];

// MOUSE INPUT
float   MouseMinX = 0;
float   MouseMaxX = 256;
float   MouseMinY = 0;
float   MouseMaxY = 223;
int     MouseX, MouseY;
float   MouseMoveX, MouseMoveY;
int     MouseMove2X, MouseMove2Y;
Uint8   MouseButton;

DWORD   LastUsedPos = 0;
DWORD   CurMode = -1;

#define UPDATE_TICKS_GAME 1000/60     // milliseconds per world update
#define UPDATE_TICKS_GAMEPAL 1000/50  // milliseconds per world update
#define UPDATE_TICKS_GUI 1000/36      // milliseconds per world update
#define UPDATE_TICKS_UDP 1000/60      // milliseconds per world update

_int64 start, end, freq, update_ticks_pc, start2, end2, update_ticks_pc2;

void drawscreenwin(void);
void initwinvideo();
void ProcessKeyBuf(int scancode);
void LinuxExit(void);


int Main_Proc(void)
{
    int j;
    SDL_Event event;
    Uint8 JoyButton;

    while (SDL_PollEvent(&event)) {
	switch(event.type)
	    {
	    case SDL_KEYDOWN:
		if (event.key.keysym.sym == SDLK_LSHIFT ||
		    event.key.keysym.sym == SDLK_RSHIFT)
		    shiftptr = 1;
		if (event.key.keysym.mod & KMOD_NUM)
		    numlockptr = 1;
		else
		    numlockptr = 0;			
		if (event.key.keysym.scancode-8 >= 0) {
		    if (pressed[event.key.keysym.scancode-8] != 2)
			pressed[event.key.keysym.scancode-8] = 1;
		    ProcessKeyBuf(event.key.keysym.sym);
		}
		break;
		
	    case SDL_KEYUP:
		if (event.key.keysym.sym == SDLK_LSHIFT ||
		    event.key.keysym.sym == SDLK_RSHIFT)
		    shiftptr = 0;
		if (event.key.keysym.scancode-8 >= 0)
		    pressed[event.key.keysym.scancode-8] = 0;
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
		    MouseButton = MouseButton & ~event.button.button;
		    break;
		    
	    /*
	      joystick trackball code untested; change the test values
	      if the motion is too sensitive (or not sensitive enough);
	      only the first trackball is supported for now. we could get
	      really general here, but this may break the format of 'pressed'
	    */
	    case SDL_JOYBALLMOTION:
		CurrentJoy = event.jball.which;
		if (event.jball.ball == 0) {
		    if (event.jball.xrel < -100) {
			pressed[0x100 + CurrentJoy*32 + 6] = 0;
			pressed[0x100 + CurrentJoy*32 + 7] = 1;
		    }
		    if (event.jball.xrel > 100) {
			pressed[0x100 + CurrentJoy*32 + 6] = 1;
			pressed[0x100 + CurrentJoy*32 + 7] = 0;
		    }
		    if (event.jball.yrel < -100) {
			pressed[0x100 + CurrentJoy*32 + 8] = 0;
			pressed[0x100 + CurrentJoy*32 + 9] = 1;
		    }
		    if (event.jball.yrel > 100) {
			pressed[0x100 + CurrentJoy*32 + 8] = 1;
			pressed[0x100 + CurrentJoy*32 + 9] = 0;
		    }
		}
		break;
		
	    case SDL_JOYAXISMOTION:
		CurrentJoy = event.jaxis.which;
		for (j=0; j<3; j++) {
		    if (event.jaxis.axis == j) {
			if (event.jaxis.value < -16384) {
			    pressed[0x100 + CurrentJoy*32 + 2*j + 1] = 1;
			    pressed[0x100 + CurrentJoy*32 + 2*j + 0] = 0;
			} else if (event.jaxis.value > 16384) {
			    pressed[0x100 + CurrentJoy*32 + 2*j + 0] = 1;
			    pressed[0x100 + CurrentJoy*32 + 2*j + 1] = 0;
			} else {
			    pressed[0x100 + CurrentJoy*32 + 2*j + 0] = 0;
			    pressed[0x100 + CurrentJoy*32 + 2*j + 1] = 0;
			}
		    }
		}						
		break;
		
	    case SDL_JOYBUTTONDOWN:
		CurrentJoy = event.jbutton.which;
		JoyButton = event.jbutton.button;
		pressed[0x100 + CurrentJoy*32 + 16 + JoyButton] = 1; 
		break;
		
	    case SDL_JOYBUTTONUP: 
		CurrentJoy = event.jbutton.which;
		JoyButton = event.jbutton.button;
		pressed[0x100 + CurrentJoy*32 + 16 + JoyButton] = 0; 
		break;    			
	    case SDL_QUIT:
		LinuxExit();
		break;
	    default: break;
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
	(scancode == SDLK_TAB)) {
	accept = true; vkeyval = scancode;
    }
    if ((scancode >= '0') && (scancode <= '9')) {
	accept = 1; vkeyval = scancode;
	if (shiftptr) {
	    switch (scancode) {
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
    if ((scancode >= SDLK_KP0) && (scancode <= SDLK_KP9)) {
	
	if (numlockptr) {
	    accept = true; vkeyval = scancode - SDLK_KP0 + '0';
	} else {
	    
	    switch (scancode)
		{
		case SDLK_KP9: vkeyval = 256+73; accept=true; break;
		case SDLK_KP8: vkeyval = 256+72; accept=true; break;
		case SDLK_KP7: vkeyval = 256+71; accept=true; break;
		case SDLK_KP6: vkeyval = 256+77; accept=true; break;
		case SDLK_KP5: vkeyval = 256+76; accept=true; break;
		case SDLK_KP4: vkeyval = 256+75; accept=true; break;
		case SDLK_KP3: vkeyval = 256+81; accept=true; break;
		case SDLK_KP2: vkeyval = 256+80; accept=true; break;
		case SDLK_KP1: vkeyval = 256+79; accept=true; break;
		}
	} // end no-numlock
    } // end testing of keypad
    if (!shiftptr){
	switch (scancode) {
	case SDLK_MINUS: vkeyval = '-'; accept = true; break;
	case SDLK_EQUALS: vkeyval = '='; accept = true; break;
	case SDLK_LEFTBRACKET: vkeyval = '['; accept = true; break;
	case SDLK_RIGHTBRACKET: vkeyval = ']'; accept = true; break;
	case SDLK_SEMICOLON: vkeyval = ';'; accept = true; break;
	// ??? - DDOI
	//case 222: vkeyval=39; accept = true; break;
	//case 220: vkeyval=92; accept = true; break;
	case SDLK_COMMA: vkeyval = ','; accept = true; break;
	case SDLK_PERIOD: vkeyval = '.'; accept = true; break;
	case SDLK_SLASH: vkeyval = '/'; accept = true; break;
	case SDLK_QUOTE: vkeyval = '`'; accept = true; break;
	}
    } else {
	switch (scancode) {
	case SDLK_MINUS: vkeyval = '_'; accept = true; break;
	case SDLK_EQUALS: vkeyval = '+'; accept = true; break;
	case SDLK_LEFTBRACKET: vkeyval = '{'; accept = true; break;
	case SDLK_RIGHTBRACKET: vkeyval = '}'; accept = true; break;
	case SDLK_SEMICOLON: vkeyval = ':'; accept = true; break;
	case SDLK_QUOTE: vkeyval = '"'; accept = true; break;
	case SDLK_COMMA: vkeyval = '<'; accept = true; break;
	case SDLK_PERIOD: vkeyval = '>'; accept = true; break;
	case SDLK_SLASH: vkeyval = '?'; accept = true; break;
	case SDLK_BACKQUOTE: vkeyval = '~'; accept = true; break;
	case SDLK_BACKSLASH: vkeyval = '|'; accept = true; break;
	}
    }
    // TODO Figure out what the rest these are supposed to be - DDOI
    switch (scancode) {
    case SDLK_PAGEUP: vkeyval = 256+73; accept = true; break;
    case SDLK_UP: vkeyval = 256+72; accept = true; break;
    case SDLK_HOME: vkeyval = 256+71; accept = true; break;
    case SDLK_RIGHT: vkeyval = 256+77; accept = true; break;
    //case 12: vkeyval = 256+76; accept = true; break;
    case SDLK_LEFT: vkeyval = 256+75; accept = true; break;
    case SDLK_PAGEDOWN: vkeyval = 256+81; accept = true; break;
    case SDLK_DOWN: vkeyval = 256+80; accept = true; break;
    case SDLK_END: vkeyval = 256+79; accept = true; break;
    case SDLK_KP_PLUS: vkeyval = '+'; accept = true; break;
    case SDLK_KP_MINUS: vkeyval = '-'; accept = true; break;
    case SDLK_KP_MULTIPLY: vkeyval = '*'; accept = true; break;
    case SDLK_KP_DIVIDE: vkeyval = '/'; accept = true; break;
    case SDLK_KP_PERIOD: vkeyval = '.'; accept = true; break;
    }
    
    if (accept){
	KeyBuffer[CurKeyPos] = vkeyval;
	CurKeyPos++;
	if (CurKeyPos == 16) CurKeyPos = 0;
    }
}

void UpdateSound(void *userdata, Uint8 *stream, int len);

int InitSound (void)
{
    SDL_AudioSpec wanted;
    const int freqtab[7] = { 8000, 11025, 22050, 44100, 16000, 32000, 48000 };
    const int samptab[7] = { 64, 64, 128, 256, 128, 256, 256 };
    
    SDL_CloseAudio();
    
    if (!SoundEnabled) {
	return FALSE;
    }
    
    PrevSoundQuality = SoundQuality;
    PrevStereoSound = StereoSound;
    
    if (SoundQuality > 6)
   	SoundQuality = 1;
    wanted.freq = freqtab[SoundQuality];
    
    if (StereoSound) {
   	wanted.channels = 2;
    } else {
   	wanted.channels = 1;
    }
    
    //wanted.samples = (wanted.freq / 60) * 2 * wanted.channels;
    wanted.samples = samptab[SoundQuality] * 2 * wanted.channels;
    wanted.format = AUDIO_S16LSB;
    wanted.userdata = NULL;
    wanted.callback = UpdateSound;
    
   if (SDL_OpenAudio(&wanted, NULL) < 0) {
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
    int i;
    
    for (i=0; i<4; i++)
	JoystickInput[i] = NULL;
    // If it is possible to use SDL_NumJoysticks
    // before initialising SDL_INIT_JOYSTICK then
    // this call can be replaced with SDL_InitSubSystem
    if (!SDL_NumJoysticks()) {
	printf ("ZSNES could not find any joysticks.\n");
	SDL_QuitSubSystem(SDL_INIT_JOYSTICK);	       
	return FALSE;		
    }
    SDL_JoystickEventState(SDL_ENABLE);
    for (i=0; i<SDL_NumJoysticks(); i++) {
	JoystickInput[i] = SDL_JoystickOpen(i);
	printf ("Joystick %i (%i Buttons): %s\n", i, 
		SDL_JoystickNumButtons(JoystickInput[i]), 
		SDL_JoystickName(i));
    }
    
    return TRUE;
}


void endgame()
{
    STUB_FUNCTION;
    SDL_Quit();
}

BOOL InitInput()
{
    InitJoystickInput();
    return TRUE;
    
}


int SurfaceLocking = 0;
DWORD converta;
unsigned int BitConv32Ptr;

int startgame(void)
{
    unsigned int color32,ScreenPtr2;
    int i;
    Uint32 flags = SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE;
    DWORD GBitMask;
    
    ScreenPtr2=BitConv32Ptr;
    for(i=0; i<65536; i++)
	{
	    color32 = ((i&0xF800)<<8)+
		((i&0x07E0)<<5)+
		((i&0x001F)<<3)+0xFF000000;
	    (*(unsigned int *)(ScreenPtr2)) = color32;
	    ScreenPtr2+=4;
	}
    
    if (sdl_inited == 0) {
	if (SDL_Init(SDL_INIT_AUDIO|SDL_INIT_TIMER|SDL_INIT_VIDEO|SDL_INIT_JOYSTICK) < 0) {
	    fprintf(stderr, "Could not initialize SDL!\n");
	    return FALSE;
	} else {
	    sdl_inited = 1;
	}
    }
    
    flags |= ( FullScreen ? SDL_FULLSCREEN : 0);
#ifdef __OPENGL__
    if (UseOpenGL) {
	flags |= SDL_OPENGL;
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    }
#endif
    
    surface = SDL_SetVideoMode(WindowWidth, WindowHeight, BitDepth, flags);
    if (surface == NULL) {
	fprintf (stderr, "Could not set %ldx%ld video mode.\n",WindowWidth,
		 WindowHeight);
	return FALSE;
    }
    
#ifdef __OPENGL__
    if (UseOpenGL) {
	
     if (SurfaceY) glViewport(0,0, SurfaceX, SurfaceY);
     else glViewport(0,0, SurfaceX, 1); 
     glMatrixMode(GL_PROJECTION);
     glLoadIdentity();
     glMatrixMode(GL_MODELVIEW);
     glFlush();
     
     if (!gl_inited) {
	 glGenTextures(1,&glvbtexture[0]);
	 gl_inited = 1;
	 glvidbuffer = malloc(256*256*2);
	 // the *2 in 256*256*2 may need to be changed later on to account for the bitdepth
     }
    } else {
	if (gl_inited) {
	    free(glvidbuffer);
	    gl_inited = 0;
	}
    }

    BitDepth = (UseOpenGL ? 16 : 0);
#endif
    
    SurfaceLocking = SDL_MUSTLOCK(surface);
    SDL_WarpMouse(SurfaceX/4,SurfaceY/4);

    // Grab mouse in fullscreen mode
    FullScreen ? SDL_WM_GrabInput(SDL_GRAB_ON) : SDL_WM_GrabInput(SDL_GRAB_OFF);

    /* Need to handle situations where BPP is not what we can handle */
    SDL_WM_SetCaption ("ZSNES Linux","ZSNES");
    SDL_ShowCursor(0);

    BitDepth = surface->format->BitsPerPixel;
    // Check hardware for 565/555
    GBitMask = surface->format->Gmask;

#ifdef __OPENGL__
    if (!UseOpenGL) {
#endif
	if(BitDepth == 16 && GBitMask != 0x07E0) {
	    converta=1;
	    //      Init_2xSaI(555);
	} else {
	    converta=0;
	}

#ifdef __OPENGL__
    }
#endif

    return TRUE;
}


BYTE* SurfBuf;

DWORD LockSurface(void)
{
    // Lock SDL surface, return surface pitch
    if(SurfaceLocking) SDL_LockSurface(surface);
#ifdef __OPENGL__
    if (!UseOpenGL) {
#endif
	SurfBuf = surface->pixels;
	return(surface->pitch);
#ifdef __OPENGL__
    } else {
	return (512);
    }
#endif
}

void UnlockSurface(void)
{
    if (SurfaceLocking) SDL_UnlockSurface(surface);
#ifdef __OPENGL__
    if (!UseOpenGL) {
#endif
	SDL_Flip(surface);
#ifdef __OPENGL__
    } else {
	SDL_GL_SwapBuffers();
    }
#endif
}


int Running=0;
unsigned char Noise[]={
    27,232,234,138,187,246,176,81,25,241,1,127,154,190,195,103,231,165,220,238,
    232,189,57,201,123,75,63,143,145,159,13,236,191,142,56,164,222,80,88,13,
    148,118,162,212,157,146,176,0,241,88,244,238,51,235,149,50,77,212,186,241,
    88,32,23,206,1,24,48,244,248,210,253,77,19,100,83,222,108,68,11,58,
    152,161,223,245,4,105,3,82,15,130,171,242,141,2,172,218,152,97,223,157,
    93,75,83,238,104,238,131,70,22,252,180,82,110,123,106,133,183,209,48,230,
    157,205,27,21,107,63,85,164
};
int X, Y;
int i;
int count, x, count2;
DWORD Temp1;
DWORD SurfBufD;
DWORD CurrentPos;
DWORD WritePos;
DWORD SoundBufD;
DWORD SoundBufD2;
DWORD T60HZEnabled=0;
DWORD T36HZEnabled=0;
short *Sound;
//void WinUpdateDevices(); function removed since it was empty
extern unsigned char romispal;


void Start60HZ(void)
{
    update_ticks_pc2 = UPDATE_TICKS_UDP;
    if(romispal == 1) {
	update_ticks_pc = UPDATE_TICKS_GAMEPAL;
    } else {
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
    T36HZEnabled=0;
}


DWORD FirstVid = 1;
DWORD FirstFull = 1;
extern BYTE GUIWFVID[];
extern BYTE BlackAndWhite;
extern BYTE V8Mode;
void clearwin();

void initwinvideo(void)
{
    DWORD newmode=0;
    
    if (BlackAndWhite == 1) V8Mode = 1;
    else V8Mode = 0;

    if(CurMode != cvidmode) {
	CurMode = cvidmode;
	newmode = 1;
	SurfaceX = 256;
	SurfaceY = 224;
	X = 0; Y=0;
	FullScreen=GUIWFVID[cvidmode];
#ifdef __OPENGL__
	UseOpenGL = 0;
	if (cvidmode > 3) UseOpenGL = 1;
#endif
	
	switch(cvidmode)
	    {
		// case 0 is removed since it is the default
	    case 1:
		WindowWidth = 320;
		WindowHeight = 240 ;
		SurfaceX = 320;
		SurfaceY = 240;
		break;
	    case 2: case 5:
		WindowWidth = 512;
		WindowHeight = 448;
		SurfaceX = 512;
		SurfaceY = 448;
		break;
	    case 3: case 6:
		WindowWidth = 640;
		WindowHeight = 480;
		SurfaceX = 640;
		SurfaceY = 480;
		break;
	    case 7:
		WindowWidth = 640;
		WindowHeight = 576;
		SurfaceX = 640;
		SurfaceY = 576;
		break;
	    case 8:
		WindowWidth = 768;
		WindowHeight = 672;
		SurfaceX = 768;
		SurfaceY = 672;
		break;
	    case 9:
		WindowWidth = 896;
		WindowHeight = 784;
		SurfaceX = 896;
		SurfaceY = 784;
		break;
	    case 10:
		WindowWidth = 1024;
		WindowHeight = 896;
		SurfaceX = 1024;
		SurfaceY = 896;
		break;
	    case 11:
		WindowWidth = 800;
		WindowHeight = 600;
		SurfaceX = 800;
		SurfaceY = 600;
		break;
	    case 12:
		WindowWidth = 1024;
		WindowHeight = 768;
		SurfaceX = 1024;
		SurfaceY = 768;
		break;
	    default:
		WindowWidth = 256;
		WindowHeight = 224;
		break;
	    }
    }
    
    if(startgame() != TRUE) {return; }
    if(newmode==1) clearwin();
    
    if (FirstVid == 1) {
	FirstVid = 0;
	
	InitSound();
	InitInput();
    }

    if(((PrevStereoSound!=StereoSound)||(PrevSoundQuality!=SoundQuality)))
	ReInitSound();	
}


DWORD ScreenPtr;
DWORD ScreenPtr2;
extern unsigned int vidbuffer;
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
	    if (CounterA>0) CounterA--;
	    if (CounterB>0) CounterB--;
	    if (PacketCounter){
		for (i=0;i<256;i++){
		    if (packettimeleft[i]>0)
			packettimeleft[i]--;
		}
	    }
	    start2 += update_ticks_pc2;
	}
    
    if(T60HZEnabled) {
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
    
    if(T36HZEnabled) {
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
DWORD BufferLeftOver=0;
short Buffer[1800*2];

void UpdateSound(void *userdata, Uint8 *stream, int len)
{
    const int SPCSize = 256;
    int DataNeeded;
    int i;
    Uint8 *ptr;
    
    len /= 2; /* only 16bit here */
    
    ptr = stream;
    DataNeeded = len;
    
    /* take care of the things we left behind last time */
    if (BufferLeftOver) {
	DataNeeded -= BufferLeftOver;

	memcpy(ptr, &Buffer[BufferLeftOver], (SPCSize-BufferLeftOver)*2);
		
	ptr += (SPCSize-BufferLeftOver)*2;
	BufferLeftOver = 0;
    }
	
    if (len & 255) { /* we'll save the rest first */
	DataNeeded -= 256;
    }
		
    while (DataNeeded > 0) {
	SoundProcess();
	
	for (i = 0; i < SPCSize; i++) {
	    if (T36HZEnabled) {
		Buffer[i] = 0;
	    } else {
		if(DSPBuffer[i]>32767)Buffer[i] = 32767;
		else if(DSPBuffer[i]<-32767)Buffer[i] = -32767;
		else Buffer[i] = DSPBuffer[i];
	    }
	}
	
	memcpy(ptr, &Buffer[0], SPCSize*2);
	ptr += SPCSize*2;

	DataNeeded -= SPCSize;
    }
    
    if (DataNeeded) {
	DataNeeded += 256;
	BufferLeftOver = DataNeeded;
	
	SoundProcess();
	
	for (i = 0; i < SPCSize; i++) {
	    if (T36HZEnabled) {
		Buffer[i] = 0;
	    } else {
		if(DSPBuffer[i]>32767)Buffer[i] = 32767;
		else if(DSPBuffer[i]<-32767)Buffer[i] = -32767;
		else Buffer[i] = DSPBuffer[i];
	    }
	}
	
	memcpy(ptr, &Buffer[0], DataNeeded*2);
    }
}

void UpdateVFrame(void)
{
    Main_Proc();
    // WinUpdateDevices(); removed since it is an empty function
    CheckTimers();
}

extern DWORD AddEndBytes;
extern DWORD NumBytesPerLine;
extern unsigned char * WinVidMemStart;
extern unsigned char curblank;
extern unsigned char FPUCopy;
extern unsigned char NGNoTransp;
extern unsigned char newengen;
extern void copy640x480x16bwin(void);

void clearwin()
{
    DWORD *SURFDW;
    
    Temp1 = LockSurface();
    if(Temp1 == 0) { return; }
    vidbuff_w = SurfaceX; vidbuff_h = SurfaceY;

#ifdef __OPENGL__
    if (!UseOpenGL) {
#endif
	SurfBufD = (DWORD) &SurfBuf[0];
	SURFDW = (DWORD *) &SurfBuf[0];
#ifdef __OPENGL__
    } else {
	SurfBufD = (DWORD) &glvidbuffer[0];
	SURFDW = (DWORD *) &glvidbuffer[0];
	vidbuff_w = 256; vidbuff_h = 224;
    }
#endif

    if (SurfBufD == 0) { UnlockSurface(); return; }

    switch(BitDepth)
	{
	case 16:
	    __asm__ __volatile__ ("
		pushw %%es
		movw %%ds, %%ax
		movw %%ax, %%es
		xorl %%eax, %%eax
		movl SurfBufD, %%edi
		xorl %%ebx, %%ebx
        Blank2:
		movl vidbuff_w, %%ecx
		rep
		stosw
		addl Temp1, %%edi
		subl vidbuff_w, %%edi
		subl vidbuff_w, %%edi
		addl $1, %%ebx
		cmpl vidbuff_h, %%ebx
		jne Blank2
		popw %%es
	" : : : "cc", "memory", "eax", "ebx", "ecx", "edi");
	    break;
	case 32:
	    __asm__ __volatile__ ("
                pushw %%es
                movw %%ds, %%ax
                movw %%ax, %%es
                xorl %%eax, %%eax
                movl SurfBufD, %%edi
                xorl %%ebx, %%ebx
        Blank3:
                movl vidbuff_w, %%ecx
                rep
                stosl
                addl Temp1, %%edi
                subl vidbuff_w, %%edi
                subl vidbuff_w, %%edi
                subl vidbuff_w, %%edi
                subl vidbuff_w, %%edi
                addl $1, %%ebx
                cmpl vidbuff_h, %%ebx
                jne Blank3
                popw %%es       
        " : : : "cc", "memory", "eax", "ebx", "ecx","edi");
	    break;
	}
    UnlockSurface();
}

void LinuxExit (void)
{
    if (sdl_inited)
	{
	    SDL_WM_GrabInput(SDL_GRAB_OFF);	// probably redundant
	    SDL_Quit();
	}
    exit(0);
}

void drawscreenwin(void)
{
    DWORD i,j,color32;
    DWORD *SURFDW;

    NGNoTransp = 0;             // Set this value to 1 within the appropriate
                                // video mode if you want to add a custom
                                // transparency routine or hardware
                                // transparency.  This only works if
                                // the value of newengen is equal to 1.
                                // (see ProcessTransparencies in newgfx16.asm
                                //  for ZSNES' current transparency code)
    UpdateVFrame();
    if(curblank!=0) return;

    Temp1=LockSurface();
    if(Temp1==0) { return; }
    
#ifdef __OPENGL__
    if (UseOpenGL) {
	
	for (j = 0; j<224; j++) {
	    memcpy(glvidbuffer + 256*j, (short *) (vidbuffer) + 16 + 288 + (256+32)*j, 256*2);
	}
	if (BilinearFilter) {
	    glfilters = GL_LINEAR;
	    if (GUIOn2 && !FilteredGUI) glfilters = GL_NEAREST;
	} else {
	    glfilters = GL_NEAREST;
	}

	if (FullScreen)
	    ratiox = 0.875;
	else
	    ratiox = 1.0;
    
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,glfilters);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,glfilters);
	glBindTexture(GL_TEXTURE_2D, *glvbtexture);
	glColor3f(1.0f,1.0f,1.0f);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 256, 0,
		     GL_RGB, GL_UNSIGNED_SHORT_5_6_5, glvidbuffer);
	glBegin(GL_QUADS);
	    glTexCoord2f(0.0f, 0.0f);         glVertex3f(-ratiox,  1.0f, -1.0f);
	    glTexCoord2f(1.0f, 0.0f);         glVertex3f( ratiox,  1.0f, -1.0f);
	    glTexCoord2f(1.0f, 2.240/2.560);  glVertex3f( ratiox, -1.0f, -1.0f);
	    glTexCoord2f(0.0f, 2.240/2.560);  glVertex3f(-ratiox, -1.0f, -1.0f);
	glEnd();

    } else {
#endif

	ScreenPtr = vidbuffer;
	ScreenPtr += 16*2+32*2+256*2;
	SurfBufD = (DWORD) &SurfBuf[0];
	SURFDW = (DWORD *) &SurfBuf[0];

	if (SurfBufD == 0) {
	    UnlockSurface();
	    return;
	}

	if(SurfaceX == 256 && SurfaceY == 224)
	    {
		switch(BitDepth)
		    {
		    case 16:
			if (FPUCopy){
			    __asm__ __volatile__ ("
		            pushw %%es
		            movw %%ds, %%ax
		            movw %%ax, %%es
		            xorl %%eax, %%eax
		            movl ScreenPtr, %%esi
		            movl SurfBufD, %%edi
	            Copying3:
		            movl $32, %%ecx
	            CopyLoop:
		            movq (%%esi), %%mm0
		            movq 8(%%esi), %%mm1
		            movq %%mm0, (%%edi)
		            movq %%mm1, 8(%%edi)
		            addl $16, %%esi
		            addl $16, %%edi
		            decl %%ecx
		            jnz CopyLoop
		            incl %%eax
		            addl Temp1, %%edi
		            subl $512, %%edi
		            addl $64, %%esi
		            cmpl $223, %%eax
		            jne Copying3

		            xorl %%eax, %%eax
		            movl $128, %%ecx
		            rep
	                    stosl
		            popw %%es
		            emms
	            " : : : "cc", "memory", "eax", "ebx", "ecx","edi", "esi");
			} else {
			    // Doesn't seem to work - DDOI
			    __asm__ __volatile__ ("
		            pushw %%es
		            movw %%ds, %%ax
		            movw %%ax, %%es
		            xorl %%eax, %%eax
		            movl ScreenPtr, %%esi
		            movl SurfBufD, %%edi
	            Copying:
		            movl $128, %%ecx
		            rep
		            movsl
		            incl %%eax
		            addl Temp1, %%edi
		            subl $512, %%edi
		            addl $64, %%esi
		            cmpl $223, %%eax
		            jne Copying
		            xorl %%eax, %%eax
		            movl $128, %%ecx
		            rep
		            stosl
		            popw %%es
	            " : : : "cc", "memory", "eax", "ebx", "ecx","edi", "esi");
			}
			break;

		    case 32:
			__asm__ __volatile__ ("
                        pushw %%es
		        movw %%ds, %%ax
		        movw %%ax, %%es
		        xorl %%eax, %%eax
		        movl BitConv32Ptr, %%ebx
		        movl ScreenPtr, %%esi
		        movl SurfBufD, %%edi
                Copying32b:
		        movl $256, %%ecx
		        pushl %%eax
		        xorl %%eax, %%eax
                CopyLoop32b:
		        movw (%%esi), %%ax
		        addl $2, %%esi
		        movl (%%ebx, %%eax, 4), %%edx
		        movl %%edx, (%%edi)
		        addl $4, %%edi
		        loop CopyLoop32b
		        popl %%eax
		        incl %%eax
		        addl Temp1, %%edi
		        subl $1024, %%edi
		        addl $64, %%esi
		        cmpl $223, %%eax
		        jne Copying32b
                        popw %%es
	        " : : : "cc", "memory", "eax", "ebx", "ecx","edi", "esi");
			SURFDW = (DWORD *) &SurfBuf[222*Temp1];
			color32 = 0x7F000000;
		
			for(i=0; i<256; i++)
			    {
				SURFDW[i] = color32;
			    }
			
			SURFDW=(DWORD *) &SurfBuf[223*Temp1];
			color32=0x7F000000;
			
			for(i=0; i<256; i++)
			    {
				SURFDW[i] = color32;
			    }
			break;

		    case 24:
			fprintf (stderr, "Sorry, this mode does not work in 24 bit color\n");
			LinuxExit();
			/*
			  cvidmode=3;
			  initwinvideo();
			  sleep(1);
			  drawscreenwin();
			*/
			break;
		    default:
			UnlockSurface();
			fprintf(stderr, "Mode only available in 16 and 32 bit color.\n");
			LinuxExit();
			/*
			  cvidmode=2;
			  initwinvideo();
			  sleep(1);
			  drawscreenwin();
			*/
			break;
		    } // switch (BitDepth)
	    } // if(SurfaceX==256&&SurfaceY==224)
   
	else if(SurfaceX == 320 && SurfaceY == 240)
	    {
		switch(BitDepth)
		    {
		    case 16:
			if (FPUCopy) {
			    __asm__ __volatile__ ("
                            pushw %%es
                            movw %%ds, %%ax
                            movw %%ax, %%es
                            xor %%eax, %%eax
                            xor %%ebx, %%ebx
                            movl ScreenPtr, %%esi
                            movl SurfBufD, %%edi
                    Blank1MMX:
                            mov $160, %%ecx
                            rep
                            stosl
                            subl $160, %%edi
                            addl Temp1, %%edi
                            addl $1, %%ebx
                            cmpl $8, %%ebx
                            jne Blank1MMX
                            xor %%ebx, %%ebx
                            pxor %%mm0, %%mm0
                    Copying2MMX:
                            mov $4, %%ecx
                    MMXLoopA:
                            movq %%mm0, 0(%%edi)
                            movq %%mm0, 8(%%edi)
                            addl $16, %%edi
                            dec %%ecx
                            jnz MMXLoopA
                            mov $32, %%ecx
                    MMXLoopB:
                            movq 0(%%esi), %%mm1
                            movq 8(%%esi), %%mm2
                            movq %%mm1, 0(%%edi)
                            movq %%mm2, 8(%%edi)
                            addl $16, %%esi
                            addl $16, %%edi
                            decl %%ecx
                            jnz MMXLoopB
                            mov $4, %%ecx
                    MMXLoopC:
                            movq %%mm0, 0(%%edi)
                            movq %%mm0, 8(%%edi)
                            addl $16, %%edi
                            decl %%ecx
                            jnz MMXLoopC
                            incl %%ebx
                            addl Temp1, %%edi
                            subl $640, %%edi
                            addl $64, %%esi
                            cmpl $223, %%ebx
                            jne Copying2MMX
                            
                            movl $128, %%ecx
                            rep
                            stosl
                            pop %%es
                            emms
                    " : : : "cc", "memory", "eax", "ebx", "ecx","edi", "esi");

			} else {
			    __asm__ __volatile__ ("
                            push %%es
                            movw %%ds, %%ax
                            movw %%ax, %%es
                            xorl %%eax, %%eax
                            xorl %%ebx, %%ebx
                            movl ScreenPtr, %%esi
                            movl SurfBufD, %%edi
                    Blank1:
                            movl $160, %%ecx
                            rep
                            stosl
                            subl $640, %%edi
                            addl Temp1, %%edi
                            addl $1, %%ebx
                            cmpl $8, %%ebx 
                            jne Blank1
                            xor %%ebx, %%ebx
                    Copying2:
                            movl $16, %%ecx
                            rep
                            stosl
                            movl $128, %%ecx
                            rep
                            movsl
                            movl $16, %%ecx
                            rep
                            stosl
                            incl %%ebx
                            addl Temp1, %%edi
                            subl $640, %%edi
                            addl $64, %%esi
                            cmpl $223, %%ebx
                            jne Copying2
                            
                            movl $128, %%ecx
                            rep
                            stosl
                            pop %%es
                    " : : : "cc", "memory", "eax", "ebx", "ecx","edi", "esi");

			}
			break;

		    case 32:
			for(j=0; j<8; j++)
			    {
				SURFDW = (DWORD *) &SurfBuf[j*Temp1];
				color32 = 0x7F000000;
				
				for(i=0; i<320; i++)
				    {
					SURFDW[i] = color32;
				    }
			    }
			
			for(j=8; j<223+8; j++)
			    {
				color32 = 0x7F000000;
				for(i=0; i<32; i++)
				    {
					SURFDW[i]=color32;
				    }
				
				for(i=32; i<(256+32); i++)
				    {
					color32 = (((*(WORD *)(ScreenPtr))&0xF800)<<8)+
					    (((*(WORD *)(ScreenPtr))&0x07E0)<<5)+
					    (((*(WORD *)(ScreenPtr))&0x001F)<<3)+0x7F000000;
					SURFDW[i] = color32;
					ScreenPtr += 2;
				    }
				
				color32 = 0x7F000000;
				for(i=(256+32); i<320; i++)
				    {
					SURFDW[i] = color32;
				    }
				
				ScreenPtr = ScreenPtr+576-512;
				SURFDW=(DWORD *) &SurfBuf[(j)*Temp1];
			    }
			
			for(j=(223+8);j<240;j++)
			    {
				SURFDW=(DWORD *) &SurfBuf[j*Temp1];
				
				color32 = 0x7F000000;
				for(i=0; i<320; i++)
				    {
					SURFDW[i] = color32;
				    }
			    }
			break;
		    default:
			UnlockSurface();
			fprintf(stderr, "Mode only available in 16 and 32 bit color.\n");
			LinuxExit();
			/*
			  cvidmode=2;
			  initwinvideo();
			  sleep(1);
			  drawscreenwin();
			*/
			break;
		    } // switch
	    } // if
	
	else if(SurfaceX == 512 && SurfaceY == 448)
	    {
		switch(BitDepth)
		    {
		    case 16:
			AddEndBytes = Temp1-1024;
			NumBytesPerLine = Temp1;
			WinVidMemStart = &SurfBuf[0];
			copy640x480x16bwin();
			break;

		    case 32:
			__asm__ __volatile__ ("
                        pushw %%es
		        movw %%ds, %%ax
		        movw %%ax, %%es
		        xorl %%eax, %%eax
		        movl BitConv32Ptr, %%ebx
		        movl ScreenPtr, %%esi
		        movl SurfBufD, %%edi
                Copying32c:
		        movl $256, %%ecx
		        pushl %%eax
		        xorl %%eax, %%eax
                CopyLoop32c:
		        movw (%%esi), %%ax
		        addl $2, %%esi
		        movl (%%ebx, %%eax, 4), %%edx
		        movl %%edx, (%%edi)
		        movl %%edx, 4(%%edi)
		        addl $8, %%edi
		        loop CopyLoop32c
		        pushl %%esi
		        movl %%edi, %%esi
		        subl Temp1, %%esi
		        movl $512, %%ecx
		        rep
		        movsl
		        popl %%esi
		        popl %%eax
		        incl %%eax
		        addl $64, %%esi
		        cmpl $223, %%eax
		        jne Copying32c
                        popw %%es
	        " : : : "cc", "memory","eax","ebx","ecx","edx","edi","esi");
			break;
			/*
			  addl Temp1, %%edi
			  subl $2048, %%edi
			*/
		    default:
			UnlockSurface();
			fprintf(stderr, "Mode only available in 16 or 32 bit color.\n");
			LinuxExit();
			/*
			  cvidmode=2;
			  initwinvideo();
			  sleep(1);
			  drawscreenwin();
			*/
			break;
		    } // switch
	    } // if
	else if (SurfaceX == 640 && SurfaceY == 480)
	    {
		switch(BitDepth)
		    {
		    case 16:
   			AddEndBytes = Temp1-1024;
			NumBytesPerLine = Temp1;
			WinVidMemStart = &SurfBuf[16*640*2+64*2];
			copy640x480x16bwin();
			break;

		    case 32:
			__asm__ __volatile__ ("
                        pushw %%es
		        movw %%ds, %%ax
		        movw %%ax, %%es
		        xorl %%eax, %%eax
		        movl BitConv32Ptr, %%ebx
		        movl ScreenPtr, %%esi
		        movl SurfBufD, %%edi
		        addl $20608, %%edi
                Copying32d:
		        movl $256, %%ecx
		        pushl %%eax
		        xorl %%eax, %%eax
                CopyLoop32d:
		        movw (%%esi), %%ax
		        addl $2, %%esi
		        movl (%%ebx, %%eax, 4), %%edx
		        movl %%edx, (%%edi)
		        movl %%edx, 4(%%edi)
		        addl $8, %%edi
		        loop CopyLoop32d
		        addl $512, %%edi
		        pushl %%esi
		        movl %%edi, %%esi
		        subl Temp1, %%esi
		        movl $512, %%ecx
		        rep
		        movsl
		        popl %%esi
		        popl %%eax
		        incl %%eax
		        addl $512, %%edi
		        addl $64, %%esi
		        cmpl $223, %%eax
		        jne Copying32d
                        popw %%es
	        " : : : "cc", "memory","eax","ebx","ecx","edx","edi","esi");
			break;
			/*
			  addl Temp1, %%edi
			  subl $2048, %%edi
			*/

		    default:
			UnlockSurface();
			fprintf(stderr, "Mode only available in 16 or 32 bit color.\n");
			LinuxExit();
			/*
			  cvidmode=2;
			  initwinvideo();
			  sleep(1);
			  drawscreenwin();
			*/
			break;
		    }
	    }
	
#ifdef __OPENGL__
    }
#endif
    UnlockSurface();
}

extern char fulladdtab[65536*2];
extern void SwitchFullScreen(void);

int GetMouseX(void) { return((int)MouseX); }
int GetMouseY(void) { return((int)MouseY); }

int GetMouseMoveX(void)
{
    //   InputRead();
    //SDL_GetRelativeMouseState(&MouseMove2X, NULL);
    SDL_GetRelativeMouseState(&MouseMove2X, &MouseMove2Y);
    return(MouseMove2X);
}

int GetMouseMoveY(void) { return(MouseMove2Y); }
int GetMouseButton(void) { return((int) MouseButton); }

void SetMouseMinX(int MinX) { MouseMinX = MinX; }
void SetMouseMaxX(int MaxX) { MouseMaxX = MaxX; }
void SetMouseMinY(int MinY) { MouseMinY = MinY; }
void SetMouseMaxY(int MaxY) { MouseMaxY = MaxY; }

// we can probably get rid of these functions since they are no
// longer called in sdlintrf.asm
void SetMouseX(int X) { /* MouseX=X; */ }
void SetMouseY(int Y) { /* MouseY=Y; */ }

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
    time (&current);
    timeptr = localtime(&current);
    SystemTimewHour = timeptr->tm_hour;
    SystemTimewMinute = timeptr->tm_min;
    SystemTimewSecond = timeptr->tm_sec;
}

