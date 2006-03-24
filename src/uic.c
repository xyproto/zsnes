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


#ifdef __UNIXSDL__
#include "gblhdr.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "asm_call.h"
#include "mmlib/mm.h"

//C++ style code in C
#define bool unsigned char
#define true 1
#define false 0

extern unsigned char *vbufaptr;
extern unsigned char *vbufeptr;
extern unsigned char *ngwinptrb;
extern unsigned char *vbufdptr;
extern unsigned char *romaptr;
extern unsigned char mydebug[2];
extern unsigned char outofmem[51];
extern unsigned char YesMMX[34];

// Global Variables ported from ASM

unsigned int per2exec = 100; // percentage of opcodes to execute

#ifdef __MSDOS__
unsigned char cvidmode = 4; // video mode
#else
unsigned char cvidmode = 1;
#endif

unsigned char string[512];
unsigned char fname[512];
unsigned char fnames[512];	// sram filename
unsigned char fnamest[512];	// state filename

unsigned short selc0040;
unsigned short selcA000;
unsigned short selcB800;
unsigned char filefound;	// Parameter String Found
unsigned char frameskip;	// 0 = Auto, 1-10 = Skip 0 .. 9
unsigned char *vidbuffer;	//  video buffer (1024x239 = 244736)
unsigned char *ngwinptr;
unsigned char *vidbufferm;	// video buffer mirror
unsigned char *vidbufferofsa;	// offset 1
unsigned char *vidbufferofsb;	// offset 2
unsigned char *vidbufferofsc;	// offset 3
unsigned char *vidbufferofsmos;	// mosaic offset for new graphics engine
unsigned char *headdata;
unsigned char *romdata;		// rom data  (4MB = 4194304)
unsigned char *sfxramdata;	// SuperFX Ram Data
unsigned char *setaramdata;	// Seta ST010/ST011 SRam Data
unsigned char *wramdata;	// stack (64K = 32768)
unsigned char *ram7f;		// ram @ 7f = 65536
unsigned char *vram;		// vram = 65536
unsigned char *sram;		// sram = 32768
unsigned char *spritetablea;
unsigned char *spcBuffera;
unsigned char *debugbuf;	// debug buffer = 38x1000 = 38000
void (**regptr)();		// pointer to registers
void (**regptw)();		// pointer to registers
unsigned char *vcache2b;	// 2-bit video cache
unsigned char *vcache4b;	// 4-bit video cache
unsigned char *vcache8b;	// 8-bit video cache
unsigned char *vcache2bs;	// 2-bit video secondary cache
unsigned char *vcache4bs;	// 4-bit video secondary cache
unsigned char *vcache8bs;	// 8-bit video secondary cache
unsigned char romispal;		// 0 = NTSC, 1 = PAL
unsigned char enterpress;	// if enter is to be issued (0 = yes)
unsigned char newgfx16b;
unsigned char *BitConv32Ptr;
unsigned char *RGBtoYUVPtr;

unsigned char previdmode;	// previous video mode
unsigned char cbitmode;		// bit mode, 0=8bit, 1=16bit

unsigned char opexec268     = 155;	// # of opcodes/scanline in 2.68Mhz mode
unsigned char opexec358     = 142;	// # of opcodes/scanline in 3.58Mhz mode (228/180)
unsigned char opexec268cph  = 42;	// # of opcodes/hblank in 2.68Mhz mode
unsigned char opexec358cph  = 45;	// # of opcodes/hblank in 3.58Mhz mode (56/50)
unsigned char opexec268b    = 142;	// # of opcodes/scanline in 2.68Mhz mode
unsigned char opexec358b    = 155;	// # of opcodes/scanline in 3.58Mhz mode (228/180)
unsigned char opexec268cphb = 42;	// # of opcodes/hblank in 2.68Mhz mode
unsigned char opexec358cphb = 45;	// # of opcodes/hblank in 3.58Mhz mode (56/50)
unsigned char debugdisble   = 1;	// debugger disable.  0 = no, 1 = yes
unsigned char gammalevel    = 0;	// gamma level (8-bit engine)
unsigned char gammalevel16b = 0;	// gamma level (16-bit engine)
unsigned char scanlines     = 0;	// scanlines on/off
unsigned char vsyncon       = 0;	// vsync on/off
unsigned char guioff        = 0;	// gui on/off (1 = off)
unsigned char AddSub256     = 0;	// screen add/sub in 256 colors
unsigned char Sup48mbit     = 1;	// Support 48mbit roms
unsigned char Sup16mbit     = 0;	// Support 16mbit roms
unsigned char dmadeddis     = 0;	// DMA deduction
unsigned char antienab      = 0;	// Interpolation Enabled
unsigned char device1       = 0;  // Device in port 1? 0 = Gamepad
unsigned char device2       = 0;  // Device in port 2?
unsigned char OldStyle      = 1;	// Old style joystick on
unsigned char SecondPort    = 0;	// Secondary Joystick Port Enabled (209h) (DOS port only)

// New Variables
unsigned char ForcePal      = 0;	// 1 = NTSC, 2 = PAL
unsigned char Force8b       = 0;	// Force 8-bit sound on
unsigned char Doublevbuf    = 1;	// Double video buffer
unsigned char V8Mode        = 0;	// Vegetable mode! =) (Greyscale mode)
unsigned char fastmemptr    = 0;
unsigned char showallext    = 0;	// Show all extensions in GUI load dialog
unsigned char finterleave   = 0;
unsigned char DSPDisable    = 0;	// Disable DSP emulation
unsigned char Palette0      = 0;
unsigned char DisplayS      = 0;
unsigned char *spc7110romptr;

unsigned char MusicRelVol   = 75;
unsigned char MusicVol      = 0;
unsigned char MMXextSupport = 0;
extern char *Msgptr;

void outofmemory();
void init();
void WaitForKey();
void MMXCheck();
void allocmem();
void InitSPC();
void SystemInit();
void StartUp();
void MultiMouseInit();

void *doMemAlloc(size_t size)
{
  void *ptr = NULL;
  ptr = malloc(size);
  if (!ptr)
  {
    asm_call(outofmemory);
  }
  return(ptr);
}

void allocspc7110()
{
  spc7110romptr = (unsigned char *)doMemAlloc(8192*1024+4096);
}

extern bool input1gp;
extern bool input1mouse;
extern bool input2gp;
extern bool input2mouse;
extern bool input2scope;
extern bool input2just;
void cycleinputdevice1()
{
  for (;;)
  {
    device1++;
    if (device1 >= 2)
    {
      device1 = 0;
    }
    if (device1 == 0)
    {
      if (input1gp) { return; }
      device1++;
    }
    if (device1 == 1)
    {
      if (input1mouse) { return; }
    }
  }
}
void cycleinputdevice2()
{
  for (;;)
  {
    device2++;
    if (device2 >= 5)
    {
      device2 = 0;
    }
    if (device2 == 0)
    {
      if (input2gp) { return; }
      device2++;
    }
    if (device2 == 1)
    {
      if (input2mouse) { return; }
      device2++;
    }
    if (device2 == 2)
    {
      if (input2scope) { return; }
      device2++;
    }
    if (device2 == 3)
    {
      if (input2just) { return; }
      device2++;
    }
    if (device2 == 4)
    {
      if (input2just) { return; }
    }
  }
}

unsigned char NoiseData[32768];
const unsigned char samplenoise[128] = {
     27,232,234,138,187,246,176, 81, 25,241,  1,127,154,190,195,103,
    231,165,220,238,232,189, 57,201,123, 75, 63,143,145,159, 13,236,
    191,142, 56,164,222, 80, 88, 13,148,118,162,212,157,146,176,  0,
    241, 88,244,238, 51,235,149, 50, 77,212,186,241, 88, 32, 23,206,
      1, 24, 48,244,248,210,253, 77, 19,100, 83,222,108, 68, 11, 58,
    152,161,223,245,  4,105,  3, 82, 15,130,171,242,141,  2,172,218,
    152, 97,223,157, 93, 75, 83,238,104,238,131, 70, 22,252,180, 82,
    110,123,106,133,183,209, 48,230,157,205, 27, 21,107, 63, 85,164};

void setnoise()
{
  unsigned short ctr1, ctr2, ptr1=0;
  unsigned char ptr2=0, ptr3=0;

  for(ctr1=256;ctr1>0;ctr1--)
  {
    for(ctr2=128;ctr2>0;ctr2--,ptr1++)
    {
      NoiseData[ptr1] = (samplenoise[ptr2] + samplenoise[ptr3]);
      ptr2=(ptr2+1)&0x7f;
      ptr3=(ptr3-1)&0x7f;
    }
    ptr3=(ptr3-1)&0x7f;
  }
}


extern unsigned int xa;
extern unsigned char soundon, SPCDisable, spcon, FPSOn, FPSAtStart;

const unsigned int versionNumber = 0x0000008F; // 1.43
char *ZVERSION = "Pre 1.43";
unsigned char txtfailedalignd[25] = "Data Alignment Failure : ";
unsigned char txtfailedalignc[25] = "Code Alignment Failure : ";

void zstart ()
{
  unsigned int ptr;

  asm_call(StartUp);

  printf("%s", mydebug);

  // Print welcome message.
  printf("ZSNES v%s, (c) 1997-2005, ZSNES Team\n", ZVERSION);
  puts("Be sure to check http://www.zsnes.com/ for the latest version.");
  puts("Please report crashes to zsnes-devel@lists.sourceforge.net.\n");
  puts("ZSNES is written by the ZSNES Team (See AUTHORS.TXT)");
  puts("ZSNES comes with ABSOLUTELY NO WARRANTY.  This is free software,");
  puts("and you are welcome to redistribute it under certain conditions;");
  puts("please read 'LICENSE.TXT' thoroughly before doing so.\n");
  puts("Use ZSNES -? for command line definitions.\n");

#ifndef __RELEASE__
  puts("This is a work in progress build. It contains code which");
  puts("May or may not be complete");
#endif

#ifndef __MSDOS__
  MultiMouseInit();
#endif

  asm_call(SystemInit);


#ifdef OPENSPC
  OSPC_Init();
#else
  setnoise();
  asm_call(InitSPC);
#endif

  asm_call(allocmem);

  if (!soundon && (SPCDisable != 1))
  {
    soundon = 1;
    spcon = 1;
    DSPDisable = 1;
  }

  if (SPCDisable)
  {
    soundon = 0;
    spcon = 0;
  }

  if (!frameskip)
  {
    FPSOn = FPSAtStart;
  }

  gammalevel16b = gammalevel >> 1;

  asm_call(MMXCheck);

  ptr = (unsigned int)&outofmemory;

  if ((ptr & 3))
  {
    printf("%s%d", txtfailedalignc, (ptr & 0x1F));

    asm_call(WaitForKey);
  }

  ptr = (unsigned int)&xa;

  if ((ptr & 3))
  {
    printf("%s%d", txtfailedalignd, (ptr & 0x1F));

    asm_call(WaitForKey);
  }

  asm_call(init);
}

static char *seconds_to_asc(unsigned int seconds)
{
  static char buffer[50];
  char *p = buffer;
  unsigned int hours, minutes;

  hours = seconds/3600;
  seconds -= hours*3600;
  minutes = seconds/60;
  seconds -= minutes*60;
  *buffer = 0;

  if (hours)
  {
    sprintf(p, "%u hours ", hours);
    p += strlen(p);
  }
  if (minutes)
  {
    sprintf(p, "%u min ", minutes);
    p += strlen(p);
  }
  if (seconds)
  {
    sprintf(p, "%u sec", seconds);
    p += strlen(p);
  }
  if (!*buffer)
  {
    strcpy(buffer, "0 sec");
  }
  return(buffer);
}

extern unsigned int MessageOn;
extern unsigned int MsgCount;
extern char CSStatus[70];
extern char CSStatus2[70];
extern char CSStatus3[70];

void DisplayBatteryStatus()
{
#ifndef __MSDOS__
   int CheckBattery();
   int CheckBatteryTime();
   int CheckBatteryPercent();

   *CSStatus2 = 0;
   *CSStatus3 = 0;

   switch (CheckBattery())
   {
     case -1: //No battery
       strcpy(CSStatus, "No battery present");
       break;

     case 0: //Plugged in
       {
         int percent = CheckBatteryPercent();

         strcpy(CSStatus, "PC is plugged in");
         if (percent > 0)
         {
           sprintf(CSStatus2, "%d%% charged", percent);
         }
       }
       break;

     case 1: //Not plugged in
       {
         int percent = CheckBatteryPercent();
         int battery_time = CheckBatteryTime();

         strcpy(CSStatus, "PC is running off of battery");
         if (battery_time > 0)
         {
           sprintf(CSStatus2, "Time remaining: %s", seconds_to_asc(battery_time));
         }
         if (percent > 0)
         {
           sprintf(CSStatus3, "%d%% remaining", percent);
         }
       }
       break;
   }

   Msgptr = CSStatus;
   MessageOn = 100;

#endif
}

// Make use of multiple mice.

#ifndef __MSDOS__

int MouseCount = 0;
int Mouse1MoveX = 0;
int Mouse2MoveX = 0;
int Mouse1MoveY = 0;
int Mouse2MoveY = 0;

void MultiMouseShutdown()
{
   MouseCount = 0;
   ManyMouse_Quit();
}

void MultiMouseInit()
{
   MouseCount = ManyMouse_Init();
   printf("ManyMouse: %d mice detected.\n", MouseCount);
   atexit(MultiMouseShutdown);
}

bool Mouse1Waiting = false, Mouse2Waiting = false;
ManyMouseEvent Mouse1Event,Mouse2Event;


void MultiMouseProcess(unsigned int mouse)
{
  ManyMouseEvent event;
  event.device = ~0;

  Mouse1MoveX = Mouse1MoveY = Mouse2MoveX = Mouse2MoveY = 0;

  if (mouse == 1)
  {
    if (Mouse1Waiting)
    {
      if (Mouse1Event.type == MANYMOUSE_EVENT_RELMOTION)
      {
        if (Mouse1Event.item == 0) { Mouse1MoveX = Mouse1Event.value; } else { Mouse1MoveY = Mouse1Event.value; }
      }
      Mouse1Waiting = false;
      return;
    }
  }

  if (mouse == 2)
  {
    if (Mouse2Waiting)
    {
      if (Mouse2Event.type == MANYMOUSE_EVENT_RELMOTION)
      {
        if (Mouse2Event.item == 1) { Mouse2MoveX = Mouse2Event.value; } else { Mouse2MoveY = Mouse2Event.value; }
      }
      Mouse2Waiting = false;
      return;
    }
  }

  while ((event.device != 0) && (event.device != 1))
  {
    if (!ManyMouse_PollEvent(&event))
    {
      return;
    }

    if ((mouse == 1) && event.device == 1)
    {
      Mouse2Event = event;
      Mouse2Waiting = true;
      event.device = ~0;
    }
    if ((mouse == 2) && event.device == 0)
    {
      Mouse1Event = event;
      Mouse1Waiting = true;
      event.device = ~0;
    }
  }

  if (event.type == MANYMOUSE_EVENT_RELMOTION)
  {
    if (event.device == 0)
    {
      if (event.item == 0) { Mouse1MoveX = event.value; } else { Mouse1MoveY = event.value; }
      return;
    }
    if (event.device == 1)
    {
      if (event.item == 0) { Mouse2MoveX = event.value; } else { Mouse2MoveY = event.value; }
    }
  }
}

#endif

