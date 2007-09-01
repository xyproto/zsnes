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


#ifdef __UNIXSDL__
#include "gblhdr.h"
#include "zdir.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#endif

#include "asm_call.h"
#include "cfg.h"
#include "input.h"
#include "mmlib/mm.h"
#include "zpath.h"

#define BIT(x) (1 << (x))

extern unsigned int xa, MessageOn, maxromspace;
extern unsigned char FPSOn, spcon, device1, device2;
extern char *Msgptr, CSStatus[], CSStatus2[], CSStatus3[];

unsigned short selc0040, selcA000, selcB800;

unsigned char *vidbuffer;           //  video buffer (1024x239 = 244736)
unsigned char *ngwinptr;
unsigned char *vidbufferofsa;       // offset 1
unsigned char *vidbufferofsb;       // offset 2
unsigned char *headdata;
unsigned char *romdata;             // rom data  (4MB = 4194304)
unsigned char *sfxramdata;          // SuperFX Ram Data
unsigned char *setaramdata;         // Seta ST010/ST011 SRam Data
unsigned char *wramdata;            // stack (64K = 65536)
unsigned char *ram7f;               // ram @ 7f = 65536
unsigned char *vram;                // vram = 65536
unsigned char *sram;                // sram = 65536*2 = 131072
unsigned char regptra[49152];
unsigned char regptwa[49152];
unsigned char *regptr = regptra;
unsigned char *regptw = regptwa;
unsigned char *vcache2b;            // 2-bit video cache
unsigned char *vcache4b;            // 4-bit video cache
unsigned char *vcache8b;            // 8-bit video cache
unsigned char *SPC7110PackPtr;
unsigned char *SPC7110IndexPtr;
unsigned char romispal;             // 0 = NTSC, 1 = PAL
unsigned char newgfx16b;

unsigned char previdmode;           // previous video mode
unsigned char cbitmode;             // bit mode, 0=8bit, 1=16bit

unsigned char opexec268     = 162;  // # of opcodes/scanline in 2.68Mhz mode
unsigned char opexec358     = 181;  // # of opcodes/scanline in 3.58Mhz mode (228/180)
unsigned char opexec268cph  = 42;   // # of opcodes/hblank in 2.68Mhz mode
unsigned char opexec358cph  = 45;   // # of opcodes/hblank in 3.58Mhz mode (56/50)
unsigned char opexec268b    = 162;  // # of opcodes/scanline in 2.68Mhz mode
unsigned char opexec358b    = 181;  // # of opcodes/scanline in 3.58Mhz mode (228/180)
unsigned char opexec268cphb = 42;   // # of opcodes/hblank in 2.68Mhz mode
unsigned char opexec358cphb = 45;   // # of opcodes/hblank in 3.58Mhz mode (56/50)
unsigned char debugdisble   = 1;    // debugger disable.  0 = no, 1 = yes
unsigned char gammalevel16b = 0;    // gamma level (16-bit engine)
unsigned char AddSub256     = 0;    // screen add/sub in 256 colors
unsigned char dmadeddis     = 0;    // DMA deduction
unsigned char OldStyle      = 1;    // Old style joystick on
unsigned char SecondPort    = 0;    // Secondary Joystick Port Enabled (209h) (DOS port only)

unsigned char Doublevbuf    = 1;    // Double video buffer
unsigned char V8Mode        = 0;    // Vegetable mode! =) (Greyscale mode)
unsigned char fastmemptr    = 0;
unsigned char ForcePal      = 0;    // 1 = NTSC, 2 = PAL
unsigned char finterleave   = 0;
unsigned char DSPDisable    = 0;    // Disable DSP emulation
unsigned char MusicVol      = 0;
unsigned char MMXextSupport = 0;

void init(), WaitForKey(), MMXCheck(), InitSPC(), DosExit();
void SystemInit(), StartUp(), MultiMouseInit();

void zexit(), zexit_error();

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

static bool device2_wrap = false;

void cycleinputdevice2()
{
  for (;;)
  {
    device2++;
    if (device2 >= 5)
    {
      device2_wrap = true;
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

#ifndef __MSDOS__
unsigned char *BitConv32Ptr = 0;
unsigned char *RGBtoYUVPtr = 0;
#endif
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

unsigned char mode7tab[65536];

unsigned short fulladdtab[65536];
unsigned short VolumeConvTable[32768];
unsigned int dspWptr[256];
unsigned int dspRptr[256];

#define deallocmemhelp(p) if (p) { free(p); }

void deallocmem()
{
#ifndef __MSDOS__
  deallocmemhelp(BitConv32Ptr);
  deallocmemhelp(RGBtoYUVPtr);
#endif
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
  deallocmemhelp(vcache2b);
  deallocmemhelp(vcache4b);
  deallocmemhelp(vcache8b);
  deallocmemhelp(sram);
  deallocmemhelp(SPC7110PackPtr);
  deallocmemhelp(SPC7110IndexPtr);
}

#define AllocmemFail(ptr, size) if (!(ptr = malloc(size))) { outofmemory(); }

static void allocmem()
{
#ifndef __MSDOS__
  AllocmemFail(BitConv32Ptr, 4096+65536*16);
  AllocmemFail(RGBtoYUVPtr,65536*4+4096);
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
  AllocmemFail(sram,65536*2);
  AllocmemFail(vcache2b,262144+256);
  AllocmemFail(vcache4b,131072+256);
  AllocmemFail(vcache8b,65536+256);

  newgfx16b = 1;
  if ((romaptr = malloc(0x600000+32768*2+4096)))
  {
    maxromspace = 0x600000;
  }
  else
  {
    if ((romaptr = malloc(0x400000+32768*2+4096)))
    {
      maxromspace = 0x400000;
    }
    else
    {
      if ((romaptr = malloc(0x200000+32768*2+4096)))
      {
        maxromspace = 0x200000;
      }
      else
      {
        outofmemory();
      }
    }
  }

  // Set up memory values
  vidbuffer = vbufaptr;
  vidbufferofsa = vbufaptr;
  ngwinptr = ngwinptrb;
  vidbufferofsb = vbufeptr;

  headdata = romaptr;
  romdata = romaptr;
  sfxramdata = romaptr+0x400000;
  setaramdata = romaptr+0x400000;

  // Puts this ASM after the end of the ROM:
  //         CLI
  // here:   BRA here
  // But why?
  romdata[maxromspace+0] = 0x58;
  romdata[maxromspace+1] = 0x80;
  romdata[maxromspace+2] = 0xFE;

  wramdata = wramdataa;
  ram7f = ram7fa;
  vram = vrama;

  regptr -= 0x8000;
  regptw -= 0x8000;
}

const unsigned int versionNumber = 0x00000098; // 1.51
char *ZVERSION = "1.52";
unsigned char txtfailedalignd[] = "Data Alignment Failure : ";
unsigned char txtfailedalignc[] = "Code Alignment Failure : ";

void zstart()
{
  unsigned int ptr;

  asm_call(MMXCheck);
  asm_call(StartUp);

  // Print welcome message.
  printf("ZSNES v%s, (c) 1997-2007, ZSNES Team\n", ZVERSION);
  puts("Be sure to check http://www.zsnes.com/ for the latest version.\n");
  puts("ZSNES is written by the ZSNES Team (See AUTHORS.TXT)");
  puts("ZSNES comes with ABSOLUTELY NO WARRANTY.  This is free software,");
  puts("and you are welcome to redistribute it under certain conditions;");
  puts("please read 'LICENSE.TXT' thoroughly before doing so.\n");
  puts("Use ZSNES -? for command line definitions.\n");

#ifndef __RELEASE__
  puts("This is a work in progress build. It contains code which");
  puts("May or may not be complete\n");
#ifdef __UNIXSDL__
  puts("If this is supposed to be an official release, you forgot to");
  puts("run configure with --enable-release, go rebuild.\n");
#endif
#endif

#ifdef __UNIXSDL__
  MultiMouseInit();
#endif

  asm_call(SystemInit);

  if (guioff && !*ZCartName)
  {
    puts("Will not start without a GUI unless a filename is supplied.");
    zexit();
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

void MultiMouseShutdown()
{
   MouseCount = 0;
   ManyMouse_Quit();
}

void MultiMouseInit()
{
#ifdef __linux__
  DIR *input_dir;

  puts("Starting Mouse detection.");
  input_dir = opendir("/dev/input");
  if (input_dir)
  {
    struct dirent_info *entry;
    while ((entry = readdir_info(input_dir)))
    {
      if (!strncasecmp(entry->name, "event", strlen("event")))
      {
        if (dirent_access(entry, R_OK))
        {
          printf("Unable to poll /dev/input/%s. Make sure you have read permissions to it.\n", entry->name);
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

char panickeyp[] = "ALL SWITCHES NORMAL\0";
char mztrtr0[] = "LOAD MZT MODE - OFF\0";
char mztrtr1[] = "LOAD MZT MODE - RECORD\0";
char mztrtr2[] = "LOAD MZT MODE - REPLAY\0";
char snesdevicemsg[] = "P1:          P2:               \0";
char windissw[] = "WINDOWING DISABLED\0";
char winenasw[] = "WINDOWING ENABLED\0";
char ofsdissw[] = "OFFSET MODE DISABLED\0";
char ofsenasw[] = "OFFSET MODE ENABLED\0";
char ngena[] = "NEW GFX ENGINE ENABLED\0";
char ngdis[] = "NEW GFX ENGINE DISABLED\0";
char vollv[] = "VOLUME LEVEL :    \0";
char frlev[] = "FRAME SKIP SET TO  \0";
char frlv0[] = "AUTO FRAMERATE ENABLED\0";
char pluse1234en[] = "USE PLAYER 1/2 with 3/4 ON\0";
char pluse1234dis[] = "USE PLAYER 1/2 with 3/4 OFF\0";
char sndchena[] = "SOUND CH   ENABLED\0";
char sndchdis[] = "SOUND CH   DISABLED\0";
char sprlayena[] = "SPRITE LAYER ENABLED\0";
char sprlaydis[] = "SPRITE LAYER DISABLED\0";
char bglayermsg[] = "BG  LAYER DISABLED\0";
char gammamsg[] = "GAMMA LEVEL:   \0";

extern unsigned int MsgCount, MessageOn;
extern unsigned char pressed[];
extern unsigned char scrndis, disableeffects, osm2dis, snesinputdefault1, snesinputdefault2;
extern unsigned char mousexloc, mouseyloc, t1cc, current_zst;
extern unsigned char Voice0Disable, Voice1Disable, Voice2Disable, Voice3Disable;
extern unsigned char Voice4Disable, Voice5Disable, Voice6Disable, Voice7Disable;
extern unsigned char Voice0Status, Voice1Status, Voice2Status, Voice3Status;
extern unsigned char Voice4Status, Voice5Status, Voice6Status, Voice7Status;
void set_state_message(char *, char *);

void adjbglayermsg(char num, char toggleon)
{
    if(toggleon)
      memcpy(&bglayermsg[10], "ENABLED ",8);
    else
      memcpy(&bglayermsg[10], "DISABLED",8);

    memcpy(&bglayermsg[2], &num, 1);
    Msgptr = bglayermsg;
    MessageOn = MsgCount;
}

void adjgammamsg()
{
    gammalevel16b = gammalevel >> 1;
    if(gammalevel < 10)
      gammamsg[13] = ' ';
    else
      gammamsg[13] = '1';
    gammamsg[14] = gammalevel%10+48;
    Msgptr = gammamsg;
    MessageOn = MsgCount;
}

void adjsoundchmsg(char *soundch, char *soundstatus, char num)
{
    *soundch ^= 0x01;
    *soundstatus = 0;
    sndchena[9] = num;
    sndchdis[9] = num;
    if(*soundch == 0x01)
      Msgptr = sndchena;
    else
      Msgptr = sndchdis;
    MessageOn = MsgCount;
}

static void cycleinputdevicemsg()
{
  if(!device1)
  {
    memcpy(&snesdevicemsg[4],"GAMEPAD",7);
  }
  else
  {
    memcpy(&snesdevicemsg[4],"MOUSE  ",7);
  }

  switch(device2)
  {
    case 1:   memcpy(&snesdevicemsg[17], "MOUSE         ",14);
              break;
    case 2:   memcpy(&snesdevicemsg[17], "SUPER SCOPE   ",14);
              break;
    case 3:   memcpy(&snesdevicemsg[17], "1 JUSTIFIER ",14);
              break;
    case 4:   memcpy(&snesdevicemsg[17], "2 JUSTIFIERS",14);
              break;
    default:  memcpy(&snesdevicemsg[17], "GAMEPAD       ",14);
  }
}

static void cycleinputs(bool input1, bool input2)
{
  if (input2)
  {
    cycleinputdevice2();
    if (input1 && device2_wrap) { cycleinputdevice1(); }
    if(device2 == 2)
    {
      mousexloc = 128;
      mouseyloc = 112;
    }
    device2_wrap = false;
  }
  else if (input1)
  {
    cycleinputdevice1();
  }

  cycleinputdevicemsg();
  Msgptr = snesdevicemsg;
  MessageOn = MsgCount;
  asm_call(Get_MousePositionDisplacement);
}

#define PRESSED(key) ((pressed[(key)] == 1) && (pressed[(key)]=2))
#define SCREEN_FLIP(num) adjbglayermsg((num)+'1', !((scrndis ^= BIT(num)) & BIT(num)))
#define STATE_SELECT(num) current_zst = (current_zst/10)*10+num; set_state_message("STATE SLOT ", " SELECTED.");
#define KEY_HANDLE(key_base, action, num) if (PRESSED(key_base ## num)) { action(num); }

void QuickKeyCheck()
{
    // disable all necessary backgrounds

    KEY_HANDLE(KeyBGDisble, SCREEN_FLIP, 0)
    KEY_HANDLE(KeyBGDisble, SCREEN_FLIP, 1)
    KEY_HANDLE(KeyBGDisble, SCREEN_FLIP, 2)
    KEY_HANDLE(KeyBGDisble, SCREEN_FLIP, 3)
    if (PRESSED(KeySprDisble))
    {
      scrndis ^= 0x10;
      if(scrndis & 0x10)
        Msgptr = sprlaydis;
      else
        Msgptr = sprlayena;
      MessageOn = MsgCount;
    }

    if (PRESSED(KeyEmuSpeedDown))
    {
      if(EmuSpeed)
        EmuSpeed--;
    }

    if (PRESSED(KeyEmuSpeedUp))
    {
      if(EmuSpeed < 58)
        EmuSpeed++;
    }

    if (PRESSED(KeyResetSpeed))
    {
      EmuSpeed = 29;
    }

    if (PRESSED(KeyResetAll))
    {
      Voice0Disable = 1;
      Voice1Disable = 1;
      Voice2Disable = 1;
      Voice3Disable = 1;
      Voice4Disable = 1;
      Voice5Disable = 1;
      Voice6Disable = 1;
      Voice7Disable = 1;
      scrndis = 0;
      disableeffects = 0;
      osm2dis = 0;
      EmuSpeed = 29;
      device1 = snesinputdefault1;
      device2 = snesinputdefault2;
      Msgptr = panickeyp;
      MessageOn = MsgCount;
    }

    if (PRESSED(KeyRTRCycle))
    {
      MZTForceRTR++;
      switch(MZTForceRTR)
      {
        case 1:   Msgptr = mztrtr1;
                  break;
        case 2:   Msgptr = mztrtr2;
                  break;
        default:  Msgptr = mztrtr0;
                  MZTForceRTR = 0;                 
      }

      MessageOn = MsgCount;
    }

    if (PRESSED(KeyExtraEnab1))
    {
      cycleinputs(true, false);
    }

    if (PRESSED(KeyExtraEnab2))
    {
      cycleinputs(false, true);
    }

    if (PRESSED(KeyExtraRotate))
    {
      cycleinputs(true, true);
    }

    if (PRESSED(KeyWinDisble))
    {
      disableeffects ^= 1;
      if(disableeffects)
        Msgptr = windissw;
      else
        Msgptr = winenasw;
      MessageOn = MsgCount;
    }

    if (PRESSED(KeyOffsetMSw))
    {
      osm2dis ^= 1;
      if(osm2dis)
        Msgptr = ofsdissw;
      else
        Msgptr = ofsenasw;
      MessageOn = MsgCount;
    }

    if (PRESSED(KeyFRateUp))
    {
      if(frameskip < 10)
      {
        FPSOn = 0;
        frameskip++;
        frlev[18] = frameskip+47;
        Msgptr = frlev;
        MessageOn = MsgCount;
      }
    }

    if (PRESSED(KeyFRateDown))
    {
      if(frameskip)
      {
        frameskip--;
        if(frameskip)
        {
          frlev[18] = frameskip+47;
          Msgptr = frlev;
        }
        else
        {
          Msgptr = frlv0;
          t1cc = 0;
        }

        MessageOn = MsgCount;
      }
    }

    if (PRESSED(KeyDisplayBatt))
    {
      DisplayBatteryStatus();
    }

    if (PRESSED(KeyIncreaseGamma))
    {
      if(gammalevel < 15)
      {
        gammalevel++;
        adjgammamsg();
      }
    }

    if (PRESSED(KeyDecreaseGamma))
    {
      if(gammalevel)
      {
        gammalevel--;
        adjgammamsg();
      }
    }

    if (PRESSED(KeyDisplayFPS))
    {
      if(!frameskip)
        FPSOn ^= 1;
    }

    // do state selects
    KEY_HANDLE(KeyStateSlc, STATE_SELECT, 0)
    KEY_HANDLE(KeyStateSlc, STATE_SELECT, 1)
    KEY_HANDLE(KeyStateSlc, STATE_SELECT, 2)
    KEY_HANDLE(KeyStateSlc, STATE_SELECT, 3)
    KEY_HANDLE(KeyStateSlc, STATE_SELECT, 4)
    KEY_HANDLE(KeyStateSlc, STATE_SELECT, 5)
    KEY_HANDLE(KeyStateSlc, STATE_SELECT, 6)
    KEY_HANDLE(KeyStateSlc, STATE_SELECT, 7)
    KEY_HANDLE(KeyStateSlc, STATE_SELECT, 8)
    KEY_HANDLE(KeyStateSlc, STATE_SELECT, 9)

    if (PRESSED(KeyIncStateSlot))
    {
      current_zst = (current_zst+1)%100;
      set_state_message("STATE SLOT ", " SELECTED.");
    }

    if (PRESSED(KeyDecStateSlot))
    {
      current_zst = (current_zst+99)%100;
      set_state_message("STATE SLOT ", " SELECTED.");
    }

    if (PRESSED(KeyUsePlayer1234))
    {
      pl12s34 ^= 1;
      if(pl12s34)
        Msgptr = pluse1234en;
      else
        Msgptr = pluse1234dis;
      MessageOn = MsgCount;
    }

    if (PRESSED(KeyDisableSC0))
    {
      adjsoundchmsg(&Voice0Disable, &Voice0Status, '1');
    }

    if (PRESSED(KeyDisableSC1))
    {
      adjsoundchmsg(&Voice1Disable, &Voice1Status, '2');
    }

    if (PRESSED(KeyDisableSC2))
    {
      adjsoundchmsg(&Voice2Disable, &Voice2Status, '3');
    }

    if (PRESSED(KeyDisableSC3))
    {
      adjsoundchmsg(&Voice3Disable, &Voice3Status, '4');
    }

    if (PRESSED(KeyDisableSC4))
    {
      adjsoundchmsg(&Voice4Disable, &Voice4Status, '5');
    }

    if (PRESSED(KeyDisableSC5))
    {
      adjsoundchmsg(&Voice5Disable, &Voice5Status, '6');
    }

    if (PRESSED(KeyDisableSC6))
    {
      adjsoundchmsg(&Voice6Disable, &Voice6Status, '7');
    }

    if (PRESSED(KeyDisableSC7))
    {
      adjsoundchmsg(&Voice7Disable, &Voice7Status, '8');
    }
}
