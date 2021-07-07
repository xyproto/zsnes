#ifndef UI_H
#define UI_H

#include <stdbool.h>

#include "types.h"

#define REGPTR(x) (regptra[(x)-0x2000])
#define REGPTW(x) (regptwa[(x)-0x2000])

void cycleinputdevice1(void);
bool cycleinputdevice2(void);

void DisplayBatteryStatus(void);

extern eop* dspRptr[256];
extern eop* dspWptr[256];
extern eop* regptra[0x3000];
extern eop* regptwa[0x3000];
extern u1 DSPDisable; // Disable DSP emulation
extern u1 MusicVol;
extern u1 V8Mode; // Vegetable mode! =) (Greyscale mode)
extern u1 cbitmode; // bit mode, 0=8bit, 1=16bit
extern u1 debugdisble; // debugger disable.  0 = no, 1 = yes
extern u1 gammalevel16b; // gamma level (16-bit engine)
extern u1 mode7tab[65536];
extern u1 newgfx16b;
extern u1 romispal; // 0 = NTSC, 1 = PAL
extern u1* romdata; // rom data  (4MB = 4194304)
extern u1* sfxramdata; // SuperFX Ram Data
extern u1* spcBuffera;
extern u1* spritetablea;
extern u1* sram; // sram = 65536 * 2 = 131072
extern u1* vbufdptr;
extern u1* vcache2b; // 2-bit video cache
extern u1* vcache4b; // 4-bit video cache
extern u1* vcache8b; // 8-bit video cache
extern u1* vidbuffer; // video buffer (1024x239 = 244736)
extern u1* vidbufferofsa; // offset 1
extern u1* vidbufferofsb; // offset 2
extern u1* vram; // vram = 65536
extern u1* wramdata; // stack (64K = 65536)
extern u2 VolumeConvTable[32768];
extern u2 fulladdtab[65536];
extern u2 selcA000;

#ifndef __MSDOS__
void MultiMouseProcess(void);

extern s4 MouseCount;
extern u1 mouse;
extern u2 MouseButtons[2];
extern u2 MouseMoveX[2];
extern u2 MouseMoveY[2];
#endif

#endif
