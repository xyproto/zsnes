#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include "SDL.h"

#define BYTE   unsigned char
#define WORD   unsigned short
#define DWORD  unsigned long

DWORD WindowWidth = 256;
DWORD WindowHeight = 224;
DWORD FullScreen = 0;
BYTE TestID1[]={""};
DWORD Moving= 0;
DWORD SoundBufferSize=1024*18;
DWORD FirstSound=1;
int AllowDefault=0;
int SoundEnabled=1;

#ifdef __LINUX__ // AH
typedef enum {FALSE, TRUE} BOOL;
typedef Uint32 UINT32;
typedef long long _int64;
typedef long long LARGE_INTEGER;
#define STUB_FUNCTION fprintf(stderr,"STUB: %s at " __FILE__ ", line %d, thread %d\n",__FUNCTION__,__LINE__,getpid())
#define FAR
#define PASCAL
#endif // __LINUX__

SDL_Surface	*surface;
int sdl_inited = 0;

DWORD                   CurrentJoy=0;

SDL_Joystick	*JoystickInput[4];

DWORD                   BitDepth=0;
BYTE                    BackColor=0;

int DTimerCheck;
float MouseMinX=0;
float MouseMaxX=256;
float MouseMinY=0;
float MouseMaxY=223;
int MouseX;
int MouseY;
float MouseMoveX;
float MouseMoveY;
int MouseMove2X;
int MouseMove2Y;

DWORD SurfaceX=0;
DWORD SurfaceY=0;

Uint8 MouseButton;


#define UPDATE_TICKS_GAME 1000/60    // milliseconds per world update
#define UPDATE_TICKS_GAMEPAL 1000/50    // milliseconds per world update
#define UPDATE_TICKS_GUI 1000/36    // milliseconds per world update
#define UPDATE_TICKS_UDP 1000/60    // milliseconds per world update

_int64 start, end, freq, update_ticks_pc, start2, end2, update_ticks_pc2;


extern unsigned char pressed[];

void drawscreenwin(void);

//void Init_2xSaI(UINT32 BitFormat);
DWORD LastUsedPos=0;
DWORD CurMode=-1;

DWORD InputEn=0;
BOOL InputAcquire(void)
{
#ifdef __LINUX__ // AH
	STUB_FUNCTION;
   InputEn=1;
   return TRUE;
#else // __WIN32__
   if(JoystickInput[0]) JoystickInput[0]->Acquire();
   if(JoystickInput[1]) JoystickInput[1]->Acquire();
   if(JoystickInput[2]) JoystickInput[2]->Acquire();
   if(JoystickInput[3]) JoystickInput[3]->Acquire();

        if(MouseInput) MouseInput->Acquire();
   if(KeyboardInput) KeyboardInput->Acquire();
   InputEn=1;
   return TRUE;
#endif // __LINUX__

}

BOOL InputDeAcquire(void)
{
#ifdef __LINUX__ // AH
//	STUB_FUNCTION;
   InputEn=0;
   return TRUE;
#else // __WIN32__
   if(MouseInput) { MouseInput->Unacquire(); }
   if(KeyboardInput) KeyboardInput->Unacquire();
   if(JoystickInput[0]) JoystickInput[0]->Unacquire();
   if(JoystickInput[1]) JoystickInput[1]->Unacquire();
   if(JoystickInput[2]) JoystickInput[2]->Unacquire();
   if(JoystickInput[3]) JoystickInput[3]->Unacquire();
   InputEn=0;
   return TRUE;
#endif // __LINUX__
}

unsigned char keyboardhit=0;
void initwinvideo();
extern BYTE StereoSound;
extern DWORD SoundQuality;
extern int CurKeyPos;
extern int CurKeyReadPos;
extern int KeyBuffer[16];


BYTE PrevStereoSound;
DWORD PrevSoundQuality;

int shiftptr = 0;
void ProcessKeyBuf(int scancode);
void LinuxExit(void);

int Main_Proc(void)
{
	int j;
	SDL_Event event;
	Uint8 JoyButton;
	//STUB_FUNCTION;
	while (SDL_PollEvent(&event)) {
		switch(event.type)
		{
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_LSHIFT ||
			    event.key.keysym.sym == SDLK_RSHIFT)
				shiftptr = 1;
			if (event.key.keysym.scancode-8 >= 0) {
				if (pressed[event.key.keysym.scancode-8]!=2)
					pressed[event.key.keysym.scancode-8]=1;
				ProcessKeyBuf(event.key.keysym.sym);
			}
			break;

		case SDL_KEYUP:
			if (event.key.keysym.sym == SDLK_LSHIFT ||
			    event.key.keysym.sym == SDLK_RSHIFT)
				shiftptr = 0;
			if (event.key.keysym.scancode-8 >= 0)
				pressed[event.key.keysym.scancode-8]=0;
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
	int vkeyval;

    if (((scancode>='A') && (scancode<='Z')) ||
	((scancode>='a') && (scancode<='z')) || (scancode==27) ||
	(scancode==32) || (scancode==8) || (scancode==13) || (scancode==9)) {
      accept=true; vkeyval=scancode;
    }
    if ((scancode>='0') && (scancode<='9')) {
      accept=1; vkeyval=scancode;
      if (shiftptr) {
	switch (scancode) {
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
    if ((scancode>=SDLK_KP0) && (scancode<=SDLK_KP9)) {
      accept=true; vkeyval=scancode-SDLK_KP0+'0';
    }
    if (!shiftptr){
      switch (scancode) {
	case SDLK_MINUS: vkeyval='-'; accept=true; break;
	case SDLK_EQUALS: vkeyval='='; accept=true; break;
	case SDLK_LEFTBRACKET: vkeyval='['; accept=true; break;
	case SDLK_RIGHTBRACKET: vkeyval=']'; accept=true; break;
	case SDLK_SEMICOLON: vkeyval=';'; accept=true; break;
	// ??? - DDOI
        //case 222: vkeyval=39; accept=true; break;
	//case 220: vkeyval=92; accept=true; break;
	case SDLK_COMMA: vkeyval=','; accept=true; break;
	case SDLK_PERIOD: vkeyval='.'; accept=true; break;
	case SDLK_SLASH: vkeyval='/'; accept=true; break;
	case SDLK_QUOTE: vkeyval='`'; accept=true; break;
      }
    } else {
      switch (scancode) {
	case SDLK_MINUS: vkeyval='_'; accept=true; break;
	case SDLK_EQUALS: vkeyval='+'; accept=true; break;
	case SDLK_LEFTBRACKET: vkeyval='{'; accept=true; break;
	case SDLK_RIGHTBRACKET: vkeyval='}'; accept=true; break;
	case SDLK_SEMICOLON: vkeyval=':'; accept=true; break;
	case SDLK_QUOTE: vkeyval='"'; accept=true; break;
	case SDLK_COMMA: vkeyval='<'; accept=true; break;
	case SDLK_PERIOD: vkeyval='>'; accept=true; break;
	case SDLK_SLASH: vkeyval='?'; accept=true; break;
	case SDLK_BACKQUOTE: vkeyval='~'; accept=true; break;
	case SDLK_BACKSLASH: vkeyval='|'; accept=true; break;
      }
    }
    // TODO Figure out what the rest these are supposed to be - DDOI
    switch (scancode) {
      case SDLK_PAGEUP: vkeyval=256+73; accept=true; break;
      case SDLK_UP: vkeyval=256+72; accept=true; break;
      case SDLK_HOME: vkeyval=256+71; accept=true; break;
      case SDLK_RIGHT: vkeyval=256+77; accept=true; break;
      //case 12: vkeyval=256+76; accept=true; break;
      case SDLK_LEFT: vkeyval=256+75; accept=true; break;
      case SDLK_PAGEDOWN: vkeyval=256+81; accept=true; break;
      case SDLK_DOWN: vkeyval=256+80; accept=true; break;
      case SDLK_END: vkeyval=256+79; accept=true; break;
      case SDLK_KP_PLUS: vkeyval='+'; accept=true; break;
      case SDLK_KP_MINUS: vkeyval='-'; accept=true; break;
      case SDLK_KP_MULTIPLY: vkeyval='*'; accept=true; break;
      case SDLK_KP_DIVIDE: vkeyval='/'; accept=true; break;
      case SDLK_KP_PERIOD: vkeyval='.'; accept=true; break;
    }

    if (accept){
      KeyBuffer[CurKeyPos]=vkeyval;
      CurKeyPos++;
      if (CurKeyPos==16) CurKeyPos=0;
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
   	fprintf(stderr, "freq: %d, channels: %d, samples: %d\n", wanted.freq, wanted.channels, wanted.samples);
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

#ifdef __LINUX__ // AH
BOOL InitJoystickInput(void)
{
	int i;

//   	STUB_FUNCTION;
   	
   	for (i=0; i<4; i++)
   		JoystickInput[i]=NULL;
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
   		JoystickInput[i]=SDL_JoystickOpen(i);
		printf ("Joystick %i (%i Buttons): %s\n", i, 
			SDL_JoystickNumButtons(JoystickInput[i]), 
			SDL_JoystickName(i));
	}

   	return TRUE;
}
#endif // __LINUX__

void endgame()
{

#ifdef __LINUX__ // AH
   STUB_FUNCTION;
   SDL_Quit();
#endif // __LINUX__
}

BOOL InitInput()
{
#ifdef __LINUX__ // AH	
	InitJoystickInput();
	//InputAcquire();
#endif // __LINUX__
   return TRUE;

}

int SurfaceLocking = 0;

void TestJoy()
{
   STUB_FUNCTION;
}

DWORD converta;
unsigned int BitConv32Ptr;

int startgame(void)
{
   unsigned int color32,ScreenPtr2;
   int i;
   Uint32 flags = SDL_SWSURFACE | SDL_HWPALETTE;
   DWORD GBitMask;
   
   //STUB_FUNCTION;
   ScreenPtr2=BitConv32Ptr;
   for(i=0;i<65536;i++)
   {
      color32=((i&0xF800)<<8)+
	      ((i&0x07E0)<<5)+
	      ((i&0x001F)<<3)+0xFF000000;
      (*(unsigned int *)(ScreenPtr2))=color32;
      ScreenPtr2+=4;
   }

   if (sdl_inited == 0) {
	   if (SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO|SDL_INIT_JOYSTICK) < 0) {
		 fprintf(stderr, "Could not initialize SDL!\n");
		 return FALSE;
	   } else {
		   sdl_inited = 1;
	   }
   }

   flags |= ( FullScreen ? SDL_FULLSCREEN : 0);

   surface = SDL_SetVideoMode(WindowWidth, WindowHeight, BitDepth, flags);
   if (surface == NULL) {
	fprintf (stderr, "Could not set %ldx%ld video mode.\n",WindowWidth,
			WindowHeight);
	return FALSE;
   }

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

   if(BitDepth==16 && GBitMask!=0x07E0)
   {
      converta=1;
//      Init_2xSaI(555);
   }
   else
   {
      converta=0;
   }
   return TRUE;
}

BYTE* SurfBuf;

DWORD LockSurface(void)
{
    // Lock SDL surface, return surface pitch
    if(SurfaceLocking) SDL_LockSurface(surface);
    return(surface->pitch);
}

void UnlockSurface(void)
{
    if (SurfaceLocking) SDL_UnlockSurface(surface);
    SurfBuf = surface->pixels;
}

void WinUpdateDevices();

int Running=0;

unsigned char Noise[]={
27,232,234,138,187,246,176,81,25,241,1,127,154,190,195,103,231,165,220,238,
232,189,57,201,123,75,63,143,145,159,13,236,191,142,56,164,222,80,88,13,
148,118,162,212,157,146,176,0,241,88,244,238,51,235,149,50,77,212,186,241,
88,32,23,206,1,24,48,244,248,210,253,77,19,100,83,222,108,68,11,58,
152,161,223,245,4,105,3,82,15,130,171,242,141,2,172,218,152,97,223,157,
93,75,83,238,104,238,131,70,22,252,180,82,110,123,106,133,183,209,48,230,
157,205,27,21,107,63,85,164};


int X, Y;
DWORD Temp1;
DWORD SurfBufD;
int count, x,count2;
int i;
short *Sound;
DWORD CurrentPos;
DWORD WritePos;
DWORD SoundBufD;
DWORD SoundBufD2;



DWORD T60HZEnabled=0;
DWORD T36HZEnabled=0;

extern unsigned char romispal;

void Start60HZ(void)
{
	freq = 1000;
   update_ticks_pc2 = UPDATE_TICKS_UDP * freq / 1000;
   if(romispal==1)
   {
   update_ticks_pc = UPDATE_TICKS_GAMEPAL * freq / 1000;
   }
   else
   {
   update_ticks_pc = UPDATE_TICKS_GAME * freq / 1000;
   }

   start = SDL_GetTicks();
   start2 = SDL_GetTicks();
   //   QueryPerformanceCounter((LARGE_INTEGER*)&start);
//   QueryPerformanceCounter((LARGE_INTEGER*)&start2);
   T36HZEnabled=0;
   T60HZEnabled=1;
}

void Stop60HZ(void)
{
   T60HZEnabled=0;
}

void Start36HZ(void)
{
	freq = 1000;
   update_ticks_pc2 = UPDATE_TICKS_UDP * freq / 1000;
   update_ticks_pc = UPDATE_TICKS_GUI * freq / 1000;
//   QueryPerformanceCounter((LARGE_INTEGER*)&start);
//   QueryPerformanceCounter((LARGE_INTEGER*)&start2);

   start = SDL_GetTicks();
   start2 = SDL_GetTicks();
   T60HZEnabled=0;
   T36HZEnabled=1;
}

void Stop36HZ(void)
{
   T36HZEnabled=0;
}

char WinMessage[256];
extern unsigned char cvidmode;
DWORD FirstVid=1;
DWORD FirstFull=1;
extern BYTE GUIWFVID[];
void clearwin();

void initwinvideo(void)
{
#ifdef __LINUX__
   //RECT zwindowrect;
   //     WINDOWPLACEMENT wndpl;
   //RECT rc1, swrect;
   DWORD newmode=0;

   //STUB_FUNCTION;

   if(CurMode!=cvidmode)
   {
      CurMode=cvidmode;
      newmode=1;
      SurfaceX=256;
      SurfaceY=224;
      X=0;
      Y=0;
      FullScreen=GUIWFVID[cvidmode];

      switch(cvidmode)
      {
      case 0:
         WindowWidth=256;
         WindowHeight=224;
         break;
      case 1:
         //WindowWidth=640;
         //WindowHeight=480 ;
         WindowWidth=320;
         WindowHeight=240 ;
         SurfaceX=320;
         SurfaceY=240;
         break;
      case 2:
         WindowWidth=512;
         WindowHeight=448;
         SurfaceX=512;
         SurfaceY=448;
         break;
      case 3:
         WindowWidth=640;
         WindowHeight=480;
         SurfaceX=640;
         SurfaceY=480;
         break;
      default:
         WindowWidth=256;
         WindowHeight=224;
         break;
      }
   }

   if(startgame()!=TRUE) {return; }
   if(newmode==1) clearwin();
   
   	if (FirstVid == 1) {
		FirstVid = 0;
		
		InitSound();
		InitInput();
	}
	if(((PrevStereoSound!=StereoSound)||(PrevSoundQuality!=SoundQuality)))
	      ReInitSound();	
#endif // __LINUX__
}

extern unsigned int vidbuffer;
extern void SoundProcess();
extern int DSPBuffer[];
DWORD ScreenPtr;
DWORD ScreenPtr2;
extern int GUI36hzcall(void);
extern int Game60hzcall(void);
extern int packettimeleft[256];
extern int PacketCounter;
extern int CounterA;
extern int CounterB;


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

   if(T60HZEnabled)
   {
   //QueryPerformanceCounter((LARGE_INTEGER*)&end);
	   end = SDL_GetTicks();

   while ((end - start) >= update_ticks_pc)
      {
//         _asm{
//         pushad
//         call Game60hzcall
//         popad
//         }
	      DTimerCheck = 1;
	      Game60hzcall();
         start += update_ticks_pc;
      }
   }

   if(T36HZEnabled)
   {
   //QueryPerformanceCounter((LARGE_INTEGER*)&end);
	   end = SDL_GetTicks();

   while ((end - start) >= update_ticks_pc)
      {
//         _asm{
//         pushad
//         call GUI36hzcall
//         popad
//         }
	      GUI36hzcall();
	      DTimerCheck = 1;
         start += update_ticks_pc;
      }
   }
}

/* should we clear these on sound reset? */
DWORD BufferLeftOver=0;
short Buffer[1800*2];

#ifdef __LINUX__
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
				Buffer[i]=0;
			} else {
				if(DSPBuffer[i]>32767)Buffer[i]=32767;
				else if(DSPBuffer[i]<-32767)Buffer[i]=-32767;
				else Buffer[i]=DSPBuffer[i];
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
				if(DSPBuffer[i]>32767)Buffer[i]=32767;
				else if(DSPBuffer[i]<-32767)Buffer[i]=-32767;
				else Buffer[i]=DSPBuffer[i];
			}
		}
		
		memcpy(ptr, &Buffer[0], DataNeeded*2);
	}
}
#endif

void UpdateVFrame(void)
{
#ifdef __LINUX__
   //STUB_FUNCTION;
   Main_Proc();

   WinUpdateDevices();
   CheckTimers();

   if (DTimerCheck == 1)
   {
	   SDL_UpdateRect(surface,0,0,0,0);
	   DTimerCheck = 0;
   }
#else
   int DataNeeded;
   int SPCSize=256;
   
   if(StereoSound==1)SPCSize=256;

   while (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
   {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }

   WinUpdateDevices();
   CheckTimers();

   if (!SoundEnabled) return;

   SoundBuffer->GetCurrentPosition(&CurrentPos,&WritePos);

   if(LastUsedPos <= CurrentPos)
   {
      DataNeeded=CurrentPos-LastUsedPos;
   }
   else
   {
      DataNeeded=SoundBufferSize- LastUsedPos + CurrentPos;
   }

   DataNeeded/=(SPCSize*2);
   DataNeeded*=(SPCSize*2);

   while(DataNeeded>0)
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
         if(DSPBuffer1[i]>32767)Buffer[i]=32767;
         if(DSPBuffer1[i]<-32767)Buffer[i]=-32767;
         if(T36HZEnabled)Buffer[i]=0;
      }

      if(DS_OK!=SoundBuffer->Lock(LastUsedPos,
                                  SPCSize*2, &lpvPtr1,
                                  &dwBytes1, &lpvPtr2,
                                  &dwBytes2, 0))
      {
         return;
      }

      Sound=(short *)lpvPtr1;

      CopyMemory(lpvPtr1, &Buffer[0], dwBytes1);

      if(NULL != lpvPtr2)
      {
         CopyMemory(lpvPtr2, &Buffer[0]+dwBytes1, dwBytes2);
      }

      if(DS_OK  != SoundBuffer->Unlock(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2))
      {
         return;
      }

      LastUsedPos+=SPCSize*2;
      if(LastUsedPos==SoundBufferSize) LastUsedPos=0;
      DataNeeded-=(SPCSize*2);
   }
#endif // __LINUX__
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
   DWORD *SURFDW;

   Temp1=LockSurface();
   if(Temp1==0) { return; }

   SurfBufD=(DWORD) &SurfBuf[0];
   SURFDW=(DWORD *) &SurfBuf[0];

   if (SurfBufD == 0) { UnlockSurface(); return; }

   switch(BitDepth)
   {
      case 16:
	      //STUB_FUNCTION;
	__asm__ __volatile__ ("
		pushw %%es
		movw %%ds, %%ax
		movw %%ax, %%es
		xorl %%eax, %%eax
		movl SurfBufD, %%edi
		xorl %%ebx, %%ebx
	Blank2:
		movl SurfaceX, %%ecx
		rep
		stosw
		addl Temp1, %%edi
		subl SurfaceX, %%edi
		subl SurfaceX, %%edi
		addl $1, %%ebx
		cmpl SurfaceY, %%ebx
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
               movl SurfaceX, %%ecx
               rep
               stosl
               addl Temp1, %%edi
               subl SurfaceX, %%edi
               subl SurfaceX, %%edi
               subl SurfaceX, %%edi
               subl SurfaceX, %%edi
               addl $1, %%ebx
               cmpl SurfaceY, %%ebx
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

   NGNoTransp = 0;              // Set this value to 1 within the appropriate
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

   ScreenPtr=vidbuffer;
   ScreenPtr+=16*2+32*2+256*2;
   SurfBufD=(DWORD) &SurfBuf[0];
   SURFDW=(DWORD *) &SurfBuf[0];

   if (SurfBufD == 0) {
         UnlockSurface();
         return;
   }

   if(SurfaceX==256&&SurfaceY==224)
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
   
   else if(SurfaceX==320&&SurfaceY==240)
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
               addl $564, %%esi
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
					  (((*(WORD *)(ScreenPtr))&0x001F)<<3)+0x7F000000;
		                  SURFDW[i]=color32;
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

   else if(SurfaceX==512&&SurfaceY==448)
   {
	   switch(BitDepth)
	   {
		   case 16:
			   AddEndBytes=Temp1-1024;
			   NumBytesPerLine=Temp1;
			   WinVidMemStart=&SurfBuf[0];
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
   else if (SurfaceX==640 && SurfaceY==480)
   {
   	switch(BitDepth)
   	{
   		case 16:
   			AddEndBytes=Temp1-1024;
			NumBytesPerLine=Temp1;
			WinVidMemStart=&SurfBuf[16*640*2+64*2];
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

   UnlockSurface();
}

extern char fulladdtab[65536*2];

extern void SwitchFullScreen(void);

void WinUpdateDevices()
{
#ifdef __LINUX__
#else
   int i,j;
   unsigned char * keys;
   unsigned char keys2[256];
   HRESULT hRes;

   for (i=0;i<256;i++)
      keys2[i]=0;
   keys=(unsigned char *)&pressed;

   if(KeyboardInput&&InputEn==1)
   {
      KeyboardInput->GetDeviceState(256, keys2);
   }
   else
   {
      return;
   }
   if(keys2[0x38]!=0&&keys2[0x3E]!=0) exit(0);
   if(keys2[0x38]!=0&&keys2[0x1c]!=0)
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
      if(keys2[i]==0) keys[i]=0;
      if(keys2[i]!=0&&keys[i]==0) keys[i]=1;
   }
//   keys[1]=keys[16];
   keys[0]=0;

   for(i=0;i<4;i++)
   {
      if(JoystickInput[i])
      {
         for(j=0;j<32;j++)
         {
            keys[0x100+i*32+j]=0;
         }

//         memset(&js[i], 0, sizeof(DIJOYSTATE));
         JoystickInput[i]->Poll();
        
if(IDirectInputDevice7_GetDeviceState(JoystickInput[i],sizeof(DIJOYSTATE),
&js[i])==DIERR_INPUTLOST)
         {
            if(JoystickInput[i]) JoystickInput[i]->Acquire();
           
if(FAILED(IDirectInputDevice7_GetDeviceState(JoystickInput[i],sizeof(DIJOYSTATE),
&js[i]))) return;
         }

         if(!X1Disable[i])
         {
            if(js[i].lX>0) keys[0x100+i*32+0]=1;
         }

         if(!X2Disable[i])
         {
            if(js[i].lX<0) keys[0x100+i*32+1]=1;
         }


         if(!Y1Disable[i])
         {
            if(js[i].lY>0) keys[0x100+i*32+2]=1;
         }

         if(!Y2Disable[i])
         {
            if(js[i].lY<0) keys[0x100+i*32+3]=1;
         }

         if(!Z1Disable[i])
         {
            if(js[i].lZ>0) keys[0x100+i*32+4]=1;
         }

         if(!Z2Disable[i])
         {
            if(js[i].lZ<0) keys[0x100+i*32+5]=1;
         }
         if(!RY1Disable[i])
         {
            if(js[i].lRy>0) keys[0x100+i*32+6]=1;
         }
         if(!RY2Disable[i])
         {
            if(js[i].lRy<0) keys[0x100+i*32+7]=1;
         }

         if(!RZ1Disable[i])
         {
            if(js[i].lRz>0) keys[0x100+i*32+8]=1;
         }
         if(!RZ2Disable[i])
         {
            if(js[i].lRz<0) keys[0x100+i*32+9]=1;
         }
         if(!S01Disable[i])
         {
            if(js[i].rglSlider[0]>0) keys[0x100+i*32+10]=1;
         }
         if(!S02Disable[i])
         {
            if(js[i].rglSlider[0]<0) keys[0x100+i*32+11]=1;
         }
         if(!S11Disable[i])
         {
            if(js[i].rglSlider[1]>0) keys[0x100+i*32+12]=1;
         }
         if(!S12Disable[i])
         {
            if(js[i].rglSlider[1]<0) keys[0x100+i*32+13]=1;
         }
         if(js[i].rgbButtons[0]) keys[0x100+i*32+16]=1;
         if(js[i].rgbButtons[1]) keys[0x100+i*32+17]=1;
         if(js[i].rgbButtons[2]) keys[0x100+i*32+18]=1;
         if(js[i].rgbButtons[3]) keys[0x100+i*32+19]=1;
         if(js[i].rgbButtons[4]) keys[0x100+i*32+20]=1;
         if(js[i].rgbButtons[5]) keys[0x100+i*32+21]=1;
         if(js[i].rgbButtons[6]) keys[0x100+i*32+22]=1;
         if(js[i].rgbButtons[7]) keys[0x100+i*32+23]=1;
         if(js[i].rgbButtons[8]) keys[0x100+i*32+24]=1;
         if(js[i].rgbButtons[9]) keys[0x100+i*32+25]=1;
         if(js[i].rgbButtons[10]) keys[0x100+i*32+26]=1;
         if(js[i].rgbButtons[11]) keys[0x100+i*32+27]=1;
         if(js[i].rgbButtons[12]) keys[0x100+i*32+28]=1;
         if(js[i].rgbButtons[13]) keys[0x100+i*32+29]=1;
         if(js[i].rgbButtons[14]) keys[0x100+i*32+30]=1;
         if(js[i].rgbButtons[15]) keys[0x100+i*32+31]=1;
      }
      else
      {
         for(j=0;j<32;j++)
         {
            keys[0x100+i*32+j]=0;
         }
      }
   }
#endif // __LINUX__
}


int GetMouseX(void)
{
   return((int)MouseX);
}

int GetMouseY(void)
{
   return((int)MouseY);
}

int GetMouseMoveX(void)
{
//   InputRead();
   //SDL_GetRelativeMouseState(&MouseMove2X, NULL);
   SDL_GetRelativeMouseState(&MouseMove2X, &MouseMove2Y);
   return(MouseMove2X);
}

int GetMouseMoveY(void)
{
   return(MouseMove2Y);
}

int GetMouseButton(void)
{
   //STUB_FUNCTION;
   return((int)MouseButton);
}

void SetMouseMinX(int MinX)
{
//   MinX&=0xFFF;
//   char message1[256];
//   sprintf(message1,"MinX %d",MinX);
//   MessageBox (NULL, message1, "Init", MB_ICONERROR );
   MouseMinX=MinX;
}

void SetMouseMaxX(int MaxX)
{
//   MaxX&=0xFFF;
//   char message1[256];
//   sprintf(message1,"MaxX %d",MaxX);
//   MessageBox (NULL, message1, "Init", MB_ICONERROR );
   MouseMaxX=MaxX;
}

void SetMouseMinY(int MinY)
{
//   MinY&=0xFFF;
//   char message1[256];
//   sprintf(message1,"MinY %d",MinY);
//   MessageBox (NULL, message1, "Init", MB_ICONERROR );
   MouseMinY=MinY;
}

void SetMouseMaxY(int MaxY)
{
//   MaxY&=0xFFF;
//   char message1[256];
//   sprintf(message1,"MaxY %d",MaxY);
//   MessageBox (NULL, message1, "Init", MB_ICONERROR );
   MouseMaxY=MaxY;
}

void SetMouseX(int X)
{
//   MouseX=X;
}

void SetMouseY(int Y)
{
//   MouseY=Y;
}

void ZsnesPage()
{
#ifdef __LINUX__
	system("netscape -remote 'openURL(http://www.zsnes.com/)'");
#else
     ShellExecute(NULL, NULL, "http://www.zsnes.com", NULL, NULL, 0);
#endif // __LINUX__
}

#ifdef __LINUX__
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
#endif

