/*
Copyright (C) 1997-2006 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes

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


#ifdef __UNIXSDL__
#include "gblhdr.h"
#include <dirent.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "asm_call.h"
#include "mmlib/mm.h"
#include "zpath.h"

//C++ style code in C
#define bool unsigned char
#define true 1
#define false 0

extern unsigned int per2exec, xa, MessageOn;
extern unsigned char cvidmode, frameskip, scanlines, vsyncon, guioff, antienab;
extern unsigned char Force8b, MusicRelVol, soundon, SPCDisable, spcon, FPSOn;
extern unsigned char FPSAtStart;
extern char *Msgptr, CSStatus[], CSStatus2[], CSStatus3[];

unsigned short selc0040, selcA000, selcB800;

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
unsigned char *debugbuf;	// debug buffer = 38x1000 = 38000
unsigned char regptra[49152];
unsigned char regptwa[49152];
unsigned char *regptr = regptra;
unsigned char *regptw = regptwa;
unsigned char *vcache2b;	// 2-bit video cache
unsigned char *vcache4b;	// 4-bit video cache
unsigned char *vcache8b;	// 8-bit video cache
unsigned char romispal;		// 0 = NTSC, 1 = PAL
unsigned char newgfx16b;

unsigned char previdmode;	// previous video mode
unsigned char cbitmode;		// bit mode, 0=8bit, 1=16bit

unsigned char opexec268     = 155;	// # of opcodes/scanline in 2.68Mhz mode
unsigned char opexec358     = 142;	// # of opcodes/scanline in 3.58Mhz mode (228/180)
unsigned char opexec268cph  = 42;	// # of opcodes/hblank in 2.68Mhz mode
unsigned char opexec358cph  = 45;	// # of opcodes/hblank in 3.58Mhz mode (56/50)
unsigned char opexec268b    = 155;	// # of opcodes/scanline in 2.68Mhz mode
unsigned char opexec358b    = 142;	// # of opcodes/scanline in 3.58Mhz mode (228/180)
unsigned char opexec268cphb = 42;	// # of opcodes/hblank in 2.68Mhz mode
unsigned char opexec358cphb = 45;	// # of opcodes/hblank in 3.58Mhz mode (56/50)
unsigned char debugdisble   = 1;	// debugger disable.  0 = no, 1 = yes
unsigned char gammalevel    = 0;	// gamma level (8-bit engine)
unsigned char gammalevel16b = 0;	// gamma level (16-bit engine)
unsigned char AddSub256     = 0;	// screen add/sub in 256 colors
unsigned char Sup48mbit     = 1;	// Support 48mbit roms
unsigned char Sup16mbit     = 0;	// Support 16mbit roms
unsigned char dmadeddis     = 0;	// DMA deduction
unsigned char device1       = 0;  // Device in port 1?
unsigned char device2       = 0;  // Device in port 2?
unsigned char OldStyle      = 1;	// Old style joystick on
unsigned char SecondPort    = 0;	// Secondary Joystick Port Enabled (209h) (DOS port only)

unsigned char Doublevbuf    = 1;	// Double video buffer
unsigned char V8Mode        = 0;	// Vegetable mode! =) (Greyscale mode)
unsigned char fastmemptr    = 0;
unsigned char ForcePal      = 0;    // 1 = NTSC, 2 = PAL
unsigned char finterleave   = 0;
unsigned char DSPDisable    = 0;	// Disable DSP emulation
unsigned char MusicVol      = 0;
unsigned char MMXextSupport = 0;

void init(), WaitForKey(), MMXCheck(), InitSPC();
void SystemInit(), StartUp(), MultiMouseInit();

void *alloc_ptr;
unsigned int alloc_size;

void alloc_help()
{
  alloc_ptr=malloc(alloc_size);
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

static void outofmemory()
{
  puts("You don't have enough memory to run this program!");
  asm_call(DosExit);
}

extern unsigned char wramdataa[65536], ram7fa[65536];

unsigned char *BitConv32Ptr = 0;
unsigned char *RGBtoYUVPtr = 0;
unsigned char *spcBuffera = 0;
unsigned char *spritetablea = 0;
unsigned char *vbufaptr = 0;
unsigned char *vbufeptr = 0;
unsigned char *ngwinptrb = 0;
unsigned char *vbufdptr = 0;
unsigned char *romaptr = 0;
unsigned char *vcache2bs = 0; // 2-bit video secondary cache
unsigned char *vcache4bs = 0; // 4-bit video secondary cache
unsigned char *vcache8bs = 0; // 8-bit video secondary cache


unsigned char vrama[65536];
unsigned char srama[65536*2];
unsigned char debugbufa[80000];
unsigned char vcache2ba[262144+256];
unsigned char vcache4ba[131072+256];
unsigned char vcache8ba[65536+256];

unsigned char mode7tab[65536];
unsigned char *wramreadptr, wramwriteptr;


unsigned short fulladdtab[65536];
unsigned char spcRamcmp[65536];
unsigned short VolumeConvTable[32768];
unsigned int dspWptr[256];
unsigned int dspRptr[256];

#define deallocmemhelp(p) if (p) { free(p); }

void deallocmem()
{
  deallocmemhelp(BitConv32Ptr);
  deallocmemhelp(RGBtoYUVPtr);
  deallocmemhelp(spcBuffera);
  deallocmemhelp(spritetablea);
  deallocmemhelp(vbufaptr);
  deallocmemhelp(vbufeptr);
  deallocmemhelp(ngwinptrb);
  deallocmemhelp(vbufdptr);
  deallocmemhelp(romaptr);
  deallocmemhelp(vcache2bs);
  deallocmemhelp(vcache4bs);
  deallocmemhelp(vcache8bs);
}

#define AllocmemFail(ptr, size) if (!(ptr = malloc(size))) { outofmemory(); }

static void allocmem()
{
#ifndef __MSDOS__
  AllocmemFail(BitConv32Ptr, 4096+65536*16);
#endif
  AllocmemFail(spcBuffera,65536*4+4096);
  AllocmemFail(spritetablea,256*512+4096);
  AllocmemFail(vbufaptr,512*296*4+4096+512*296);
  AllocmemFail(vbufeptr,288*2*256+4096);
  AllocmemFail(ngwinptrb,256*224+4096);
  AllocmemFail(vbufdptr,1024*296);
  AllocmemFail(vcache2bs,65536*4*4+4096);
  AllocmemFail(vcache4bs,65536*4*2+4096);
  AllocmemFail(vcache8bs,65536*4+4096);
  AllocmemFail(RGBtoYUVPtr,65536*4+4096);

  newgfx16b = 1;
  if (!(romaptr = malloc(0x600000+32768*2+4096)))
  {
    Sup48mbit = 0;
    if (!(romaptr = malloc(0x400000+32768*2+4096)))
    {
      Sup16mbit = 1;
      if (!(romaptr = malloc(0x200000+32768*2+4096)))
      {
        outofmemory();
      }
    }
  }

  // Set up memory values
  vidbuffer = vbufaptr;
  vidbufferofsa = vbufaptr;
  vidbufferofsmos = vidbuffer+75036;
  ngwinptr = ngwinptrb;
  vidbufferofsb = vbufeptr;
  vidbufferofsc = vbufdptr;

  headdata = romaptr;
  romdata = romaptr;
  sfxramdata = romaptr+0x400000;
  setaramdata = romaptr+0x400000;

  if (Sup48mbit)
  {
    romdata[0x600000] = 0x58;
    romdata[0x600001] = 0x80;
    romdata[0x600002] = 0xFE;
  }
  else if (Sup16mbit)
  {
    romdata[0x200000] = 0x58;
    romdata[0x200001] = 0x80;
    romdata[0x200002] = 0xFE;
  }
  else
  {
    romdata[0x400000] = 0x58;
    romdata[0x400001] = 0x80;
    romdata[0x400002] = 0xFE;
  }

  wramdata = wramdataa;
  ram7f = ram7fa;
  vram = vrama;
  sram = srama;
  debugbuf = debugbufa;

  regptr -= 0x8000;
  regptw -= 0x8000;

  vcache2b = vcache2ba;
  vcache4b = vcache4ba;
  vcache8b = vcache8ba;
}

const unsigned int versionNumber = 0x0000008F; // 1.43
char *ZVERSION = "Pre 1.43";
unsigned char txtfailedalignd[] = "Data Alignment Failure : ";
unsigned char txtfailedalignc[] = "Code Alignment Failure : ";

void zstart()
{
  unsigned int ptr;

  asm_call(StartUp);

  // Print welcome message.
  printf("ZSNES v%s, (c) 1997-2006, ZSNES Team\n", ZVERSION);
  puts("Be sure to check http://www.zsnes.com/ for the latest version.\n");
  puts("ZSNES is written by the ZSNES Team (See AUTHORS.TXT)");
  puts("ZSNES comes with ABSOLUTELY NO WARRANTY.  This is free software,");
  puts("and you are welcome to redistribute it under certain conditions;");
  puts("please read 'LICENSE.TXT' thoroughly before doing so.\n");
  puts("Use ZSNES -? for command line definitions.\n");

#ifndef __RELEASE__
  puts("This is a work in progress build. It contains code which");
  puts("May or may not be complete");
#endif

#ifdef __UNIXSDL__
  MultiMouseInit();
#endif

  asm_call(SystemInit);

  if (guioff && !*ZCartName)
  {
    puts("Will not start without a GUI unless a filename is supplied.");
    exit(0);
  }
  else
  {
    extern bool romloadskip;
    romloadskip = true;
  }

#ifdef OPENSPC
  OSPC_Init();
#else
  setnoise();
  asm_call(InitSPC);
#endif

  allocmem();

  if (!(spcon = !SPCDisable)) { soundon = 0; }
  DSPDisable = !soundon;

  if (!frameskip)
  {
    FPSOn = FPSAtStart;
  }

  gammalevel16b = gammalevel >> 1;

  asm_call(MMXCheck);

  ptr = (unsigned int)&init;
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

extern unsigned int statefileloc;
void GetFilename();
extern unsigned char firstsaveinc,LatestSave,newestfileloc;
void DetermineNew();

extern time_t newestfiledate;

void determinenewhelp(char ext)
{
    ZStateName[statefileloc] = ext;
    DetermineNew();
}

void zst_init()
{
  statefileloc = strlen(ZStateName)-1;
  firstsaveinc = 1;

  if(LatestSave)
  {
    newestfiledate = 0;
    newestfileloc = 0;

    determinenewhelp('t');
    determinenewhelp('1');
    determinenewhelp('2');
    determinenewhelp('3');
    determinenewhelp('4');
    determinenewhelp('5');
    determinenewhelp('6');
    determinenewhelp('7');
    determinenewhelp('8');
    determinenewhelp('9');

    if (!newestfileloc)
    {
      ZStateName[statefileloc] = 't';
    }
    else
    {
      ZStateName[statefileloc] = newestfileloc+'0';
    }
  }
}

static char *seconds_to_asc(unsigned int seconds)
{
  static char buffer[50];
  char *p = buffer;
  unsigned int hours, minutes;

  minutes = seconds/60;
  seconds -= minutes*60;
  hours = minutes/60;
  minutes -= hours*60;
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

unsigned short MouseMoveX[2];
unsigned short MouseMoveY[2];
unsigned short MouseButtons[2];

static bool MouseWaiting[2];

void MultiMouseShutdown(void)
{
   MouseCount = 0;
   ManyMouse_Quit();
}

void MultiMouseInit()
{
#ifdef linux
  DIR *input_dir;

  puts("Starting Mouse detection.");
  input_dir = opendir("/dev/input");
  if (input_dir)
  {
    struct dirent *ent;
    while ((ent = readdir(input_dir)))
    {
      if (!strncasecmp(ent->d_name, "event", strlen("event")))
      {
        char buffer[32];
        if (((size_t)snprintf(buffer, sizeof(buffer), "/dev/input/%s", ent->d_name) < sizeof(buffer)) &&
            access(buffer, R_OK))
        {
          printf("Unable to poll %s. Make sure you have read permissions to it.\n", buffer);
        }
      }
    }
    closedir(input_dir);
  }
  else
  {
    puts("/dev/input does not exist or is inaccessable");
  }
#endif
  MouseCount = ManyMouse_Init();
  printf("ManyMouse: %d mice detected.\n", MouseCount);

  if (MouseCount > 1)
  {
    MouseMoveX[0] = MouseMoveX[1] = 0;
    MouseMoveY[0] = MouseMoveY[1] = 0;
    MouseButtons[0] = MouseButtons[1] = 0;
    MouseWaiting[0] = MouseWaiting[1] = false;
    atexit(MultiMouseShutdown);

    printf("Using ManyMouse for:\nMouse 0: %s\nMouse 1: %s\n", ManyMouse_DeviceName(0), ManyMouse_DeviceName(1));
  }
  else
  {
    strcpy(CSStatus, "Dual mice not detected");
    strcpy(CSStatus2, "");
    strcpy(CSStatus3, "");
    Msgptr = CSStatus;
    MessageOn = 100;

    MultiMouseShutdown();
  }
}

#define BIT(x) (1 << (x))
#define MOUSE_BUTTON_HANDLE(mouse, bit, value) \
  if (value) { mouse |= BIT(bit); } \
  else { mouse &= ~BIT(bit); }

unsigned char mouse;
void MultiMouseProcess()
{
  ManyMouseEvent event;
  if (MouseWaiting[mouse])
  {
    MouseWaiting[mouse] = false;
  }
  else
  {
    MouseMoveX[mouse] = 0;
    MouseMoveY[mouse] = 0;

    while (ManyMouse_PollEvent(&event))
    {
      if (event.device != 0 && event.device != 1)
      {
        continue;
      }

      //printf("Device: %d; Type: %d; Item: %d; Value: %d\n", event.device, event.type, event.item, event.value);

      if ((event.device == (mouse^1)) && !MouseWaiting[event.device])
      {
        MouseMoveX[event.device] = 0;
        MouseMoveY[event.device] = 0;
        MouseWaiting[event.device] = true;
      }

      if (event.type == MANYMOUSE_EVENT_RELMOTION)
      {
        if (event.item == 0) { MouseMoveX[event.device] = event.value; }
        else { MouseMoveY[event.device] = event.value; }
      }
      else if (event.type == MANYMOUSE_EVENT_BUTTON)
      {
        if (event.item == 0) { MOUSE_BUTTON_HANDLE(MouseButtons[event.device], 0, event.value); }
        else if (event.item == 1) { MOUSE_BUTTON_HANDLE(MouseButtons[event.device], 1, event.value); }
      }
    }
  }
}

#endif

