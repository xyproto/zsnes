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

#ifdef __UNIXSDL__
#include "gblhdr.h"
#include "zdir.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#endif

#include "c_init.h"
#include "c_intrf.h"
#include "cfg.h"
#include "cpu/c_dspproc.h"
#include "input.h"
#include "mmlib/mm.h"
#include "ui.h"
#include "video/procvid.h"
#include "zpath.h"

#define BIT(x) (1 << (x))

extern uint8_t *SA1RAMArea;
extern unsigned int xa, maxromspace;
extern unsigned char spcon, device1, device2;
extern char CSStatus[], CSStatus2[], CSStatus3[], CSStatus4[];

u2 selcA000;

u1* vidbuffer;
unsigned char* ngwinptr;
u1* vidbufferofsa;
u1* vidbufferofsb;
unsigned char* headdata;
u1* romdata;
u1* sfxramdata;
unsigned char* setaramdata; // Seta ST010/ST011 SRam Data
u1* wramdata;
unsigned char* ram7f; // ram @ 7f = 65536
u1* vram;
u1* sram;
eop* regptra[0x3000];
eop* regptwa[0x3000];
u1* vcache2b;
u1* vcache4b;
u1* vcache8b;
u1 romispal;
u1 newgfx16b;

u1 cbitmode;

unsigned char opexec268 = 162; // # of opcodes/scanline in 2.68Mhz mode
unsigned char opexec358 = 181; // # of opcodes/scanline in 3.58Mhz mode (228/180)
unsigned char opexec268cph = 42; // # of opcodes/hblank in 2.68Mhz mode
unsigned char opexec358cph = 45; // # of opcodes/hblank in 3.58Mhz mode (56/50)
unsigned char opexec268b = 162; // # of opcodes/scanline in 2.68Mhz mode
unsigned char opexec358b = 181; // # of opcodes/scanline in 3.58Mhz mode (228/180)
unsigned char opexec268cphb = 42; // # of opcodes/hblank in 2.68Mhz mode
unsigned char opexec358cphb = 45; // # of opcodes/hblank in 3.58Mhz mode (56/50)
u1 debugdisble = 1;
u1 gammalevel16b = 0;
unsigned char AddSub256 = 0; // screen add/sub in 256 colors
unsigned char dmadeddis = 0; // DMA deduction
unsigned char OldStyle = 1; // Old style joystick on
unsigned char SecondPort = 0; // Secondary Joystick Port Enabled (209h) (DOS port only)

unsigned char Doublevbuf = 1; // Double video buffer
u1 V8Mode = 0;
unsigned char fastmemptr = 0;
unsigned char ForcePal = 0; // 1 = NTSC, 2 = PAL
unsigned char finterleave = 0;
u1 DSPDisable = 0;
u1 MusicVol = 0;

void init();
void MultiMouseInit();

void zexit(), zexit_error();

extern bool input1gp;
extern bool input1mouse;
extern bool input2gp;
extern bool input2mouse;
extern bool input2scope;
extern bool input2just;

void cycleinputdevice1(void)
{
    for (;;) {
        device1++;
        if (device1 >= 2) {
            device1 = 0;
        }
        if (device1 == 0) {
            if (input1gp) {
                return;
            }
            device1++;
        }
        if (device1 == 1) {
            if (input1mouse) {
                return;
            }
        }
    }
}

bool cycleinputdevice2(void)
{
    bool wrap = false;
    for (;;) {
        device2++;
        if (device2 >= 5) {
            wrap = true;
            device2 = 0;
        }
        if (device2 == 0) {
            if (input2gp)
                break;
            device2++;
        }
        if (device2 == 1) {
            if (input2mouse)
                break;
            device2++;
        }
        if (device2 == 2) {
            if (input2scope)
                break;
            device2++;
        }
        if (device2 == 3) {
            if (input2just)
                break;
            device2++;
        }
        if (device2 == 4) {
            if (input2just)
                break;
        }
    }
    return wrap;
}

unsigned char NoiseData[32768];
const unsigned char samplenoise[128] = {
    27, 232, 234, 138, 187, 246, 176, 81, 25, 241, 1, 127, 154, 190, 195, 103,
    231, 165, 220, 238, 232, 189, 57, 201, 123, 75, 63, 143, 145, 159, 13, 236,
    191, 142, 56, 164, 222, 80, 88, 13, 148, 118, 162, 212, 157, 146, 176, 0,
    241, 88, 244, 238, 51, 235, 149, 50, 77, 212, 186, 241, 88, 32, 23, 206,
    1, 24, 48, 244, 248, 210, 253, 77, 19, 100, 83, 222, 108, 68, 11, 58,
    152, 161, 223, 245, 4, 105, 3, 82, 15, 130, 171, 242, 141, 2, 172, 218,
    152, 97, 223, 157, 93, 75, 83, 238, 104, 238, 131, 70, 22, 252, 180, 82,
    110, 123, 106, 133, 183, 209, 48, 230, 157, 205, 27, 21, 107, 63, 85, 164
};

void setnoise()
{
    unsigned short ctr1, ctr2, ptr1 = 0;
    unsigned char ptr2 = 0, ptr3 = 0;

    for (ctr1 = 256; ctr1 > 0; ctr1--) {
        for (ctr2 = 128; ctr2 > 0; ctr2--, ptr1++) {
            NoiseData[ptr1] = (samplenoise[ptr2] + samplenoise[ptr3]);
            ptr2 = (ptr2 + 1) & 0x7f;
            ptr3 = (ptr3 - 1) & 0x7f;
        }
        ptr3 = (ptr3 - 1) & 0x7f;
    }
}

static void outofmemory()
{
    puts("You don't have enough memory to run this program!");
    DosExit();
}

extern unsigned char wramdataa[65536], ram7fa[65536];

#ifndef __MSDOS__
unsigned char* BitConv32Ptr = 0;
unsigned char* RGBtoYUVPtr = 0;
#endif
u1* spcBuffera = 0;
u1* spritetablea = 0;
unsigned char* vbufaptr = 0;
unsigned char* vbufeptr = 0;
unsigned char* ngwinptrb = 0;
u1* vbufdptr = 0;
unsigned char* romaptr = 0;
unsigned char* vcache2bs = 0; // 2-bit video secondary cache
unsigned char* vcache4bs = 0; // 4-bit video secondary cache
unsigned char* vcache8bs = 0; // 8-bit video secondary cache

unsigned char vrama[65536];

u1 mode7tab[65536];

u2 fulladdtab[65536];
u2 VolumeConvTable[32768];
eop* dspWptr[256];
eop* dspRptr[256];

#define deallocmemhelp(p) \
    if (p) {              \
        free(p);          \
    }

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
}

#define AllocmemFail(ptr, size)  \
    if (!(ptr = malloc(size))) { \
        outofmemory();           \
    }

static void allocmem()
{
#ifndef __MSDOS__
    AllocmemFail(BitConv32Ptr, 4096 + 65536 * 16);
    AllocmemFail(RGBtoYUVPtr, 65536 * 4 + 4096);
#endif
    AllocmemFail(spcBuffera, 65536 * 4 + 4096);
    AllocmemFail(spritetablea, 256 * 512 + 4096);
    AllocmemFail(vbufaptr, 512 * 296 * 4 + 4096 + 512 * 296);
    AllocmemFail(vbufeptr, 288 * 2 * 256 + 4096);
    AllocmemFail(ngwinptrb, 256 * 224 + 4096);
    AllocmemFail(vbufdptr, 1024 * 296);
    AllocmemFail(vcache2bs, 65536 * 4 * 4 + 4096);
    AllocmemFail(vcache4bs, 65536 * 4 * 2 + 4096);
    AllocmemFail(vcache8bs, 65536 * 4 + 4096);
    AllocmemFail(sram, 65536 * 2);
    AllocmemFail(vcache2b, 262144 + 256);
    AllocmemFail(vcache4b, 131072 + 256);
    AllocmemFail(vcache8b, 65536 + 256);
    AllocmemFail(SA1RAMArea, 131072);
    AllocmemFail(romaptr, 0x1000000);

    newgfx16b = 1;
    maxromspace = 0xC00000;
    
    // Set up memory values
    vidbuffer = vbufaptr;
    vidbufferofsa = vbufaptr;
    ngwinptr = ngwinptrb;
    vidbufferofsb = vbufeptr;

    headdata = romaptr;
    romdata = romaptr;
    sfxramdata = romaptr + 0xE00000;
    setaramdata = romaptr + 0xE00000;

    // Puts this ASM after the end of the ROM:
    //         CLI
    // here:   BRA here
    // But why?
    romdata[maxromspace + 0] = 0x58;
    romdata[maxromspace + 1] = 0x80;
    romdata[maxromspace + 2] = 0xFE;

    wramdata = wramdataa;
    ram7f = ram7fa;
    vram = vrama;
}

unsigned char txtfailedalignd[] = "Data Alignment Failure : ";
unsigned char txtfailedalignc[] = "Code Alignment Failure : ";

void zstart()
{
    unsigned int ptr;

    MMXCheck();
    StartUp();

#ifdef __UNIXSDL__
    MultiMouseInit();
#endif

    SystemInit();

    if (guioff && !*ZCartName) {
        puts("Will not start without a GUI unless a filename is supplied.");
        zexit();
    } else {
        extern bool romloadskip;
        romloadskip = true;
    }

    setnoise();
    InitSPC();

    allocmem();

    if (!(spcon = !SPCDisable)) {
        soundon = 0;
    }
    DSPDisable = !soundon;

    if (!frameskip) {
        FPSOn = FPSAtStart;
    }

    gammalevel16b = gammalevel >> 1;

    ptr = (unsigned int)&xa;
    if ((ptr & 3)) {
        printf("%s%d", txtfailedalignd, (ptr & 0x1F));
        WaitForKey();
    }

    init();
}

static char* seconds_to_asc(unsigned int seconds)
{
    static char buffer[50];
    char* p = buffer;
    unsigned int hours, minutes;

    minutes = seconds / 60;
    seconds -= minutes * 60;
    hours = minutes / 60;
    minutes -= hours * 60;
    *buffer = 0;

    if (hours) {
        sprintf(p, "%u hours ", hours);
        p += strlen(p);
    }
    if (minutes) {
        sprintf(p, "%u min ", minutes);
        p += strlen(p);
    }
    if (seconds) {
        sprintf(p, "%u sec", seconds);
        p += strlen(p);
    }
    if (!*buffer) {
        strcpy(buffer, "0 sec");
    }
    return (buffer);
}

void DisplayBatteryStatus(void)
{
#ifndef __MSDOS__
    int CheckBattery();
    int CheckBatteryTime();
    int CheckBatteryPercent();

    *CSStatus2 = 0;
    *CSStatus3 = 0;
    *CSStatus4 = 0;

    switch (CheckBattery()) {
    case -1: // No battery
        strcpy(CSStatus, "No battery present");
        break;

    case 0: // Plugged in
    {
        int percent = CheckBatteryPercent();

        strcpy(CSStatus, "PC is plugged in");
        if (percent > 0) {
            sprintf(CSStatus2, "%d%% charged", percent);
        }
    } break;

    case 1: // Not plugged in
    {
        int percent = CheckBatteryPercent();
        int battery_time = CheckBatteryTime();

        strcpy(CSStatus, "PC is running off of battery");
        if (battery_time > 0) {
            sprintf(CSStatus2, "Time remaining: %s", seconds_to_asc(battery_time));
        }
        if (percent > 0) {
            sprintf(CSStatus3, "%d%% remaining", percent);
        }
    } break;
    }

    Msgptr = CSStatus;
    MessageOn = 100;
#endif
}

// Make use of multiple mice.

#ifndef __MSDOS__

s4 MouseCount = 0;

u2 MouseMoveX[2];
u2 MouseMoveY[2];
u2 MouseButtons[2];

static bool MouseWaiting[2];

void MultiMouseShutdown()
{
    MouseCount = 0;
    ManyMouse_Quit();
}

void MultiMouseInit()
{
#ifdef __linux__
    DIR* input_dir;

    input_dir = opendir("/dev/input");
    if (input_dir) {
        struct dirent_info* entry;
        while ((entry = readdir_info(input_dir))) {
            if (!strncasecmp(entry->name, "event", strlen("event"))) {
                if (dirent_access(entry, R_OK)) {
                    // printf("no read access: /dev/input/%s\n", entry->name);
                }
            }
        }
        closedir(input_dir);
    } else {
        puts("mouse detection: /dev/input does not exist or is inaccessable");
    }
#endif
    MouseCount = ManyMouse_Init();
    if (MouseCount > 0) {
        printf("ManyMouse: %d mice detected.\n", MouseCount);
    }

    if (MouseCount > 1) {
        MouseMoveX[0] = MouseMoveX[1] = 0;
        MouseMoveY[0] = MouseMoveY[1] = 0;
        MouseButtons[0] = MouseButtons[1] = 0;
        MouseWaiting[0] = MouseWaiting[1] = false;
        atexit(MultiMouseShutdown);

        printf("Using ManyMouse for:\nMouse 0: %s\nMouse 1: %s\n", ManyMouse_DeviceName(0), ManyMouse_DeviceName(1));
    } else {
        strcpy(CSStatus, "Dual mice not detected");
        strcpy(CSStatus2, "");
        strcpy(CSStatus3, "");
        strcpy(CSStatus4, "");
        Msgptr = CSStatus;
        MessageOn = 100;

        MultiMouseShutdown();
    }
}

#define MOUSE_BUTTON_HANDLE(mouse, bit, value) \
    if (value) {                               \
        mouse |= BIT(bit);                     \
    } else {                                   \
        mouse &= ~BIT(bit);                    \
    }

u1 mouse;

void MultiMouseProcess(void)
{
    ManyMouseEvent event;
    if (MouseWaiting[mouse]) {
        MouseWaiting[mouse] = false;
    } else {
        MouseMoveX[mouse] = 0;
        MouseMoveY[mouse] = 0;

        while (ManyMouse_PollEvent(&event)) {
            if (event.device != 0 && event.device != 1) {
                continue;
            }

            // printf("Device: %d; Type: %d; Item: %d; Value: %d\n", event.device, event.type, event.item, event.value);

            if ((event.device == (mouse ^ 1)) && !MouseWaiting[event.device]) {
                MouseMoveX[event.device] = 0;
                MouseMoveY[event.device] = 0;
                MouseWaiting[event.device] = true;
            }

            if (event.type == MANYMOUSE_EVENT_RELMOTION) {
                if (event.item == 0) {
                    MouseMoveX[event.device] = event.value;
                } else {
                    MouseMoveY[event.device] = event.value;
                }
            } else if (event.type == MANYMOUSE_EVENT_BUTTON) {
                if (event.item == 0) {
                    MOUSE_BUTTON_HANDLE(MouseButtons[event.device], 0, event.value);
                } else if (event.item == 1) {
                    MOUSE_BUTTON_HANDLE(MouseButtons[event.device], 1, event.value);
                }
            }
        }
    }
}

#endif
